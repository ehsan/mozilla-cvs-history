/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998-2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Hubbie Shaw
 *   Doug Turner <dougt@netscape.com>
 *   Stuart Parmenter <pavlov@netscape.com>
*/

#include "nspr.h"
#include "prlog.h"

#include "nsISecureBrowserUI.h"
#include "nsSecureBrowserUIImpl.h"
#include "nsIPSMComponent.h"
#include "nsPSMComponent.h"
#include "nsCOMPtr.h"
#include "nsIInterfaceRequestor.h"
#include "nsIServiceManager.h"

#include "nsIScriptGlobalObject.h"
#include "nsIObserverService.h"
#include "nsIDocumentLoader.h"
#include "nsCURILoader.h"
#include "nsIDocShell.h"
#include "nsIDocumentViewer.h"
#include "nsCURILoader.h"
#include "nsIDocument.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMXULDocument.h"
#include "nsIDOMElement.h"
#include "nsIDOMWindowInternal.h"
#include "nsIContent.h"
#include "nsIWebProgress.h"
#include "nsIChannel.h"
#include "nsIPSMSocketInfo.h"

#include "nsIURI.h"

#include "prmem.h"

#include "nsISecurityEventSink.h"

#include "nsINetSupportDialogService.h"
#include "nsIPrompt.h"
#include "nsICommonDialogs.h"
#include "nsIPref.h"

#include "nsIFormSubmitObserver.h"

#include "cmtcmn.h"
#include "rsrcids.h"
#include "nsSSLIOLayer.h"

static NS_DEFINE_CID(kCStringBundleServiceCID,  NS_STRINGBUNDLESERVICE_CID);
static NS_DEFINE_CID(kCommonDialogsCID,         NS_CommonDialog_CID );
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);

#define ENTER_SITE_PREF      "security.warn_entering_secure"
#define LEAVE_SITE_PREF      "security.warn_leaving_secure"
#define MIXEDCONTENT_PREF    "security.warn_viewing_mixed"
#define INSECURE_SUBMIT_PREF "security.warn_submit_insecure"

#define CERT_PREFIX_STR "Signed by "
#define CERT_PREFIX_STR_LENGTH 10

#if defined(PR_LOGGING)
//
// Log module for nsSecureBrowserUI logging...
//
// To enable logging (see prlog.h for full details):
//
//    set NSPR_LOG_MODULES=nsSecureBrowserUI:5
//    set NSPR_LOG_FILE=nspr.log
//
// this enables PR_LOG_DEBUG level information and places all output in
// the file nspr.log
//
PRLogModuleInfo* gSecureDocLog = nsnull;
#endif /* PR_LOGGING */

 
NS_IMETHODIMP
nsSecureBrowserUIImpl::Create(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
    nsresult rv;

    nsSecureBrowserUIImpl * inst;

    if (NULL == aResult) {
        rv = NS_ERROR_NULL_POINTER;
        return rv;
    }
    *aResult = NULL;
    if (NULL != aOuter) {
        rv = NS_ERROR_NO_AGGREGATION;
        return rv;
    }

    NS_NEWXPCOM(inst, nsSecureBrowserUIImpl);
    if (NULL == inst) {
        rv = NS_ERROR_OUT_OF_MEMORY;
        return rv;
    }
    NS_ADDREF(inst);
    rv = inst->QueryInterface(aIID, aResult);
    NS_RELEASE(inst);

    return rv;
}

nsSecureBrowserUIImpl::nsSecureBrowserUIImpl()
{
    NS_INIT_REFCNT();
    
#if defined(PR_LOGGING)
    if (nsnull == gSecureDocLog) {
        gSecureDocLog = PR_NewLogModule("nsSecureBrowserUI");
    }
#endif /* PR_LOGGING */


    mMixContentAlertShown = PR_FALSE;
    mSecurityState = STATE_IS_INSECURE;
    mLastPSMStatus    = nsnull;
    mCurrentURI       = nsnull;
    mSecurityButton   = nsnull;
}

nsSecureBrowserUIImpl::~nsSecureBrowserUIImpl()
{
    PR_FREEIF(mLastPSMStatus);
}

NS_IMPL_ISUPPORTS4(nsSecureBrowserUIImpl, 
                   nsSecureBrowserUI, 
                   nsIWebProgressListener, 
                   nsIFormSubmitObserver,
                   nsIObserver); 


NS_IMETHODIMP
nsSecureBrowserUIImpl::Init(nsIDOMWindowInternal *window, nsIDOMElement *button)
{
    mSecurityButton = button;
    mWindow         = window;

    nsresult rv = nsServiceManager::GetService( kPrefCID, 
                                                NS_GET_IID(nsIPref), 
                                                getter_AddRefs(mPref));  
    if (NS_FAILED(rv)) return rv;

    NS_WITH_SERVICE(nsIStringBundleService, service, kCStringBundleServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsILocale* locale = nsnull;
    rv = service->CreateBundle(SECURITY_STRING_BUNDLE_URL, locale, getter_AddRefs(mStringBundle));
    if (NS_FAILED(rv)) return rv;

    // hook up to the form post notifications:
    nsIObserverService *svc = 0;
    rv = nsServiceManager::GetService(NS_OBSERVERSERVICE_CONTRACTID, 
                                      NS_GET_IID(nsIObserverService), 
                                      (nsISupports**)&svc );
    if (NS_SUCCEEDED(rv) && svc) {
        nsString  topic; topic.AssignWithConversion(NS_FORMSUBMIT_SUBJECT);
        rv = svc->AddObserver( this, topic.GetUnicode());
        nsServiceManager::ReleaseService( NS_OBSERVERSERVICE_CONTRACTID, svc );
    }

    // hook up to the webprogress notifications.
    nsCOMPtr<nsIDocShell> docShell;

    nsCOMPtr<nsIScriptGlobalObject> sgo = do_QueryInterface(window);
    if (!sgo) return NS_ERROR_NULL_POINTER;

    sgo->GetDocShell(getter_AddRefs(docShell));
    if (!docShell) return NS_ERROR_NULL_POINTER;

    nsCOMPtr<nsIWebProgress> wp = do_GetInterface(docShell);
    if (!wp) return NS_ERROR_NULL_POINTER;

    wp->AddProgressListener(NS_STATIC_CAST(nsIWebProgressListener*,this));

    //    mInitByLocationChange = PR_TRUE;
    mSecurityState = STATE_IS_INSECURE;

    return NS_OK;
}

NS_IMETHODIMP
nsSecureBrowserUIImpl::DisplayPageInfoUI()
{
    nsresult res;
    NS_WITH_SERVICE(nsIPSMComponent, psm, PSM_COMPONENT_CONTRACTID, &res);
    if (NS_FAILED(res)) 
        return res;
    
    nsXPIDLCString host;
    if (mCurrentURI)
        mCurrentURI->GetHost(getter_Copies(host));
    
    return psm->DisplayPSMAdvisor(mLastPSMStatus, host, mWindow);
}




NS_IMETHODIMP 
nsSecureBrowserUIImpl::Observe(nsISupports*, const PRUnichar*, const PRUnichar*) 
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


static nsresult IsChildOfDomWindow(nsIDOMWindow *parent, nsIDOMWindow *child, PRBool* value)
{
    *value = PR_FALSE;

    if (parent == child)
    {
        *value = PR_TRUE;
        return NS_OK;
    }

    nsCOMPtr<nsIDOMWindow> childsParent;
    child->GetParent(getter_AddRefs(childsParent));
        
    if (childsParent && childsParent.get() != child)
        IsChildOfDomWindow(parent, childsParent, value);
    
    return NS_OK;
}

static PRInt16 GetSecurityStateFromChannel(nsIChannel* aChannel,
                                            char* *aPSMStatus = nsnull,
                                            CMT_CONTROL **aControl = nsnull)
{
    nsresult res;
    PRInt32 securityState;

    // qi for the psm information about this channel load.
    nsCOMPtr<nsISupports> info;
    aChannel->GetSecurityInfo(getter_AddRefs(info));
    nsCOMPtr<nsIPSMSocketInfo> psmInfo = do_QueryInterface(info);
    if (!psmInfo) {
        PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI: GetSecurityState:%p - no nsIPSMSocketInfo for %p\n", aChannel, (nsISupports *)info));
        return nsIWebProgressListener::STATE_IS_INSECURE;
    }
    PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI: GetSecurityState:%p - info is %p\n", aChannel, (nsISupports *)info));

    if (aPSMStatus) {
        PR_FREEIF(*aPSMStatus);
        res = psmInfo->GetPickledStatus(aPSMStatus);
    }
    
    if (aControl) {
        psmInfo->GetControlPtr(aControl);
    }

    res = psmInfo->GetSecurityState(&securityState);
    if (!NS_SUCCEEDED(res)) {
        PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI: GetSecurityState:%p - GetSecurityState failed: %d\n", aChannel, res));
        securityState = nsIWebProgressListener::STATE_IS_BROKEN;
    }

    PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI: GetSecurityState:%p - Returning %d\n", aChannel, securityState));
    return securityState;
}

NS_IMETHODIMP 
nsSecureBrowserUIImpl::Notify(nsIContent* formNode, nsIDOMWindowInternal* window, nsIURI* actionURL, PRBool* cancelSubmit)
{
    // Return NS_OK unless we want to prevent this form from submitting.
    *cancelSubmit = PR_FALSE;
    if (!window || !actionURL || !formNode) {
        return NS_OK;
    }
    
    nsCOMPtr<nsIDocument> document; 
    formNode->GetDocument(*getter_AddRefs(document)); 
    if (!document) return NS_OK; 

    nsCOMPtr<nsIScriptGlobalObject> globalObject; 
    document->GetScriptGlobalObject(getter_AddRefs(globalObject)); 
    nsCOMPtr<nsIDOMWindowInternal> postingWindow = do_QueryInterface(globalObject); 
    
    PRBool isChild;
    IsChildOfDomWindow(mWindow, postingWindow, &isChild);

    // This notify call is not for our window, ignore it.
    if (!isChild)
        return NS_OK;

    PRBool okayToPost;
    nsresult res = CheckPost(actionURL, &okayToPost);

    if (NS_SUCCEEDED(res) && !okayToPost)
        *cancelSubmit = PR_TRUE;
    
    return res;
}

//  nsIWebProgressListener
NS_IMETHODIMP 
nsSecureBrowserUIImpl::OnProgressChange(nsIWebProgress* aWebProgress,
                                        nsIRequest* aRequest, 
                                        PRInt32 aCurSelfProgress, 
                                        PRInt32 aMaxSelfProgress, 
                                        PRInt32 aCurTotalProgress, 
                                        PRInt32 aMaxTotalProgress)
{
    return NS_OK;
}

NS_IMETHODIMP 
nsSecureBrowserUIImpl::OnStateChange(nsIWebProgress* aWebProgress,
                                     nsIRequest* aRequest, 
                                     PRInt32 aProgressStateFlags,
                                     nsresult aStatus)
{
    nsresult res = NS_OK;

    if (aRequest == nsnull || !mPref)
        return NS_ERROR_NULL_POINTER;

    // Get the channel from the request...
    // If the request is not network based, then ignore it.    
    nsCOMPtr<nsIChannel> channel  = do_QueryInterface(aRequest, &res);
    if (NS_FAILED(res))
        return NS_OK;

    nsCOMPtr<nsIInterfaceRequestor> requestor;
    nsCOMPtr<nsISecurityEventSink> eventSink;
    channel->GetNotificationCallbacks(getter_AddRefs(requestor));
    if (requestor)
       eventSink = do_GetInterface(requestor);
    
#if defined(DEBUG)
    nsCOMPtr<nsIURI> loadingURI;
    res = channel->GetURI(getter_AddRefs(loadingURI));
    NS_ASSERTION(NS_SUCCEEDED(res),"GetURI failed");
	if (loadingURI) {
      nsXPIDLCString temp;
      loadingURI->GetSpec(getter_Copies(temp));
      PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI:%p: OnStateChange: %x :%s\n", this, aProgressStateFlags,(const char*)temp));
	}
#endif

    // A Document is starting to load...
    if ((aProgressStateFlags & (STATE_TRANSFERRING|STATE_REDIRECTING)) && 
        (aProgressStateFlags & STATE_IS_DOCUMENT))
    {
        // starting to load a webpage
        PR_FREEIF(mLastPSMStatus); mLastPSMStatus = nsnull;

        mMixContentAlertShown = PR_FALSE;

        res = CheckProtocolContextSwitch(eventSink, aRequest, channel);    
        return res;
    } 

    // A document has finished loading    
    if ((aProgressStateFlags & STATE_STOP) &&
        (aProgressStateFlags & STATE_IS_DOCUMENT) &&
        (mSecurityState == STATE_IS_SECURE ||
         mSecurityState == STATE_IS_BROKEN))
    {
        if (mSecurityState == STATE_IS_SECURE)
        {
            CMT_CONTROL *control;
            // XXX Shouldn't we do this even if the state is broken?
            // XXX Shouldn't we grab the pickled status at STATE_NET_TRANSFERRING?

            PR_FREEIF(mLastPSMStatus);
            if (GetSecurityStateFromChannel(channel, &mLastPSMStatus, &control) ==
                STATE_IS_SECURE) {
                // Everything looks okay.
                    PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI:%p: Icon set to lock\n", this));
                    
                    if (mSecurityButton)
                        res = mSecurityButton->SetAttribute( NS_ConvertASCIItoUCS2("level"), NS_ConvertASCIItoUCS2("high") );
                    
                    if (eventSink)
                        eventSink->OnSecurityChange(aRequest, (STATE_IS_SECURE));
                    
                    if (!mSecurityButton)
                        return res;

                    // Do we really need to look at res here? What happens if there's an error?
                    // We should still set the certificate authority display.
                    CMTItem caName;
                    CMTItem pickledResource = {0, NULL, 0};
                    CMUint32 socketStatus = 0;

                    pickledResource.len = *(int*)(mLastPSMStatus);
                    pickledResource.data = NS_REINTERPRET_POINTER_CAST(unsigned char*,nsMemory::Alloc(SSMSTRING_PADDED_LENGTH(pickledResource.len)));

                    if (! pickledResource.data) return PR_FAILURE;
                    
                    memcpy(pickledResource.data, mLastPSMStatus+sizeof(int), pickledResource.len);
                    
                    if (CMT_UnpickleResource( control, 
                          SSM_RESTYPE_SSL_SOCKET_STATUS,
                          pickledResource, 
                          &socketStatus) == CMTSuccess)
                    {
                        if (CMT_GetStringAttribute(control, socketStatus, SSM_FID_SSS_CA_NAME, &caName) == CMTSuccess)
                        {
							// If the CA name is RSA Data Security, then change the name to the real
							// name of the company i.e. VeriSign, Inc.
							if (PL_strcmp((const char*)caName.data, "RSA Data Security, Inc.") == 0) {
								free(caName.data);
								caName.data = (unsigned char*)PL_strdup("VeriSign, Inc.");
								caName.len = PL_strlen((const char*)caName.data);
							}

                            // Create space for "Signed by %s" display string
                            char *str = NS_REINTERPRET_POINTER_CAST(char*, nsMemory::Alloc(CERT_PREFIX_STR_LENGTH + 1 + caName.len));
                            if (str)
                            {
                                *str = '\0';
                                strcat(str, CERT_PREFIX_STR);
                                // will memcpy just return if size == 0?
                                memcpy(str + CERT_PREFIX_STR_LENGTH, caName.data, caName.len);
                                *(str + CERT_PREFIX_STR_LENGTH + caName.len) = '\0';
                                res = mSecurityButton->SetAttribute( NS_ConvertASCIItoUCS2("tooltiptext"), NS_ConvertASCIItoUCS2(str) );
                                nsMemory::Free(str);
                            }
                        }
                    }
                    nsMemory::Free(pickledResource.data);
                    return res;
            }
            mSecurityState = STATE_IS_BROKEN;
        }

        PR_LOG(gSecureDocLog, PR_LOG_DEBUG, ("SecureUI:%p: Icon set to broken\n", this));
        SetBrokenLockIcon(eventSink, aRequest);
        
        return res;
    }
    
    // don't need to do anything more if the page is broken or not secure...

    if (mSecurityState != STATE_IS_SECURE)
        return NS_OK;

    // A URL is starting to load...
    if ((aProgressStateFlags & (STATE_TRANSFERRING|STATE_REDIRECTING)) &&
        (aProgressStateFlags & STATE_IS_REQUEST))
    {   // check to see if we are going to mix content.
        return CheckMixedContext(eventSink, aRequest, channel);
    }

    return res;
}


NS_IMETHODIMP 
nsSecureBrowserUIImpl::OnLocationChange(nsIWebProgress* aWebProgress,
                                        nsIRequest* aRequest,
                                        nsIURI* aLocation)
{
    mCurrentURI = aLocation;
    
    //    if (mInitByLocationChange)
    //    {
    //        IsURLHTTPS(mCurrentURI, &mIsSecureDocument);
    //        mInitByLocationChange = PR_FALSE;
    //    }

    return NS_OK;
}

NS_IMETHODIMP 
nsSecureBrowserUIImpl::OnStatusChange(nsIWebProgress* aWebProgress,
                                      nsIRequest* aRequest,
                                      nsresult aStatus,
                                      const PRUnichar* aMessage)
{
    return NS_OK;
}

NS_IMETHODIMP 
nsSecureBrowserUIImpl::OnSecurityChange(nsIWebProgress *aWebProgress, 
                                        nsIRequest *aRequest, 
                                        PRInt32 state)
{
    // I am the guy that created this notification - do nothing

#if defined(DEBUG_dougt)
    nsCOMPtr<nsIChannel> channel  = do_QueryInterface(aRequest);
    if (!channel)
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsIURI> aURI;
    channel->GetURI(getter_AddRefs(aURI));

    nsXPIDLCString temp;
    aURI->GetSpec(getter_Copies(temp));
    printf("OnSecurityChange: (%x) %s\n", state, (const char*)temp);
#endif




    return NS_OK;
}

nsresult
nsSecureBrowserUIImpl::IsURLHTTPS(nsIURI* aURL, PRBool* value)
{
	*value = PR_FALSE;

	if (!aURL)
		return NS_OK;

    char* scheme;
	aURL->GetScheme(&scheme);

        // If no scheme, it's not an https url - not necessarily an error.
        // See bugs 54845 and 54966
	if (scheme == nsnull)
		return NS_OK;

    if ( PL_strncasecmp(scheme, "https",  5) == 0 )
		*value = PR_TRUE;
	
	nsMemory::Free(scheme);
	return NS_OK;
}

nsresult
nsSecureBrowserUIImpl::IsURLfromPSM(nsIURI* aURL, PRBool* value)
{
	*value = PR_FALSE;

	if (!aURL)
		return NS_OK;

	PCMT_CONTROL control;
    nsXPIDLCString host;
	aURL->GetHost(getter_Copies(host));

	// This may legitimately be null, for example a javascript: or file: url    
	// See bug 54966 and 54845
	if (host == nsnull)
		return NS_OK;

    if ( PL_strncasecmp(host, "127.0.0.1",  9) == 0 ) {
	    nsresult res;
	    NS_WITH_SERVICE(nsIPSMComponent, psm, PSM_COMPONENT_CONTRACTID, &res);
	    if (NS_FAILED(res)) 
		    return res;

        res = psm->GetControlConnection(&control);
		if (NS_FAILED(res)) {
			return res;
		}

		// Get the password 
		nsXPIDLCString password;
		aURL->GetPassword(getter_Copies(password));

		// Bug 55906: this is not guaranteed to be present
		if (password == nsnull) {
			return NS_OK;
		}

		if (PL_strncasecmp(password, (const char*)control->nonce.data, control->nonce.len) == 0) {
			*value = PR_TRUE;
		}
	}
	return NS_OK;
}

void nsSecureBrowserUIImpl::GetBundleString(const nsString& name, nsString &outString)
{
    if (mStringBundle && name.Length() > 0) {
        PRUnichar *ptrv = nsnull;
        if (NS_SUCCEEDED(mStringBundle->GetStringFromName(name.GetUnicode(), &ptrv)))
            outString = ptrv;
        else
            outString.SetLength(0);;

        nsMemory::Free(ptrv);

    } else {
        outString.SetLength(0);;
    }
}

nsresult 
nsSecureBrowserUIImpl::CheckProtocolContextSwitch( nsISecurityEventSink* eventSink, nsIRequest* aRequest, nsIChannel *aChannel)
{
    nsresult res;
    PRInt32 newSecurityState, oldSecurityState = mSecurityState;
    PRBool boolpref;

    newSecurityState = GetSecurityStateFromChannel(aChannel);
    
    // Check to see if we are going from a secure page to and insecure page
	if (newSecurityState == STATE_IS_INSECURE &&
      (oldSecurityState == STATE_IS_SECURE ||
       oldSecurityState == STATE_IS_BROKEN))
	{
        SetBrokenLockIcon(eventSink, aRequest, PR_TRUE);        

        if ((mPref->GetBoolPref(LEAVE_SITE_PREF, &boolpref) != 0))
            boolpref = PR_TRUE;
        
        if (boolpref) 
        {
            nsCOMPtr<nsIPrompt> dialog;
            mWindow->GetPrompter(getter_AddRefs(dialog));
            if (!dialog) 
                return NS_ERROR_FAILURE;

            nsAutoString windowTitle, message, dontShowAgain;

            GetBundleString(NS_ConvertASCIItoUCS2("Title"), windowTitle);
            GetBundleString(NS_ConvertASCIItoUCS2("LeaveSiteMessage"), message);
            GetBundleString(NS_ConvertASCIItoUCS2("DontShowAgain"), dontShowAgain);

            PRBool outCheckValue = PR_TRUE;
            res = dialog->AlertCheck(windowTitle.GetUnicode(), 
                                     message.GetUnicode(), 
                                     dontShowAgain.GetUnicode(), 
                                     &outCheckValue);
            if (NS_FAILED(res)) 
                    return res;
                
            if (!outCheckValue) {
                mPref->SetBoolPref(LEAVE_SITE_PREF, PR_FALSE);
                NS_WITH_SERVICE(nsIPSMComponent, psm, PSM_COMPONENT_CONTRACTID, &res);
                if (NS_FAILED(res)) 
                    return res;
                psm->PassPrefs();
            }
        }
    }
    // check to see if we are going from an insecure page to a secure one.
	else if ((newSecurityState == STATE_IS_SECURE ||
            newSecurityState == STATE_IS_BROKEN) &&
           oldSecurityState == STATE_IS_INSECURE)
    {
        if ((mPref->GetBoolPref(ENTER_SITE_PREF, &boolpref) != 0))
            boolpref = PR_TRUE;
        if (boolpref) 
        {
            nsCOMPtr<nsIPrompt> dialog;
            mWindow->GetPrompter(getter_AddRefs(dialog));
            if (!dialog) 
                return NS_ERROR_FAILURE;

            nsAutoString windowTitle, message, dontShowAgain;

            GetBundleString(NS_ConvertASCIItoUCS2("Title"), windowTitle);
            GetBundleString(NS_ConvertASCIItoUCS2("EnterSiteMessage"), message);
            GetBundleString(NS_ConvertASCIItoUCS2("DontShowAgain"), dontShowAgain);

            PRBool outCheckValue = PR_TRUE;
            res = dialog->AlertCheck(windowTitle.GetUnicode(), 
                                     message.GetUnicode(), 
                                     dontShowAgain.GetUnicode(), 
                                     &outCheckValue);
            if (NS_FAILED(res)) 
                return res;
                
            if (!outCheckValue)
            {
                mPref->SetBoolPref(ENTER_SITE_PREF, PR_FALSE);
                NS_WITH_SERVICE(nsIPSMComponent, psm, PSM_COMPONENT_CONTRACTID, &res);
                if (NS_FAILED(res)) 
                    return res;
                psm->PassPrefs();
            }
        }
    }

    mSecurityState = newSecurityState;
    return NS_OK;
}


nsresult
nsSecureBrowserUIImpl::CheckMixedContext(nsISecurityEventSink *eventSink, nsIRequest* aRequest, nsIChannel* aChannel)
{
    PRInt16 newSecurityState;
    nsresult rv;

    newSecurityState = GetSecurityStateFromChannel(aChannel);

    if ((newSecurityState == STATE_IS_INSECURE ||
         newSecurityState == STATE_IS_BROKEN) &&
        mSecurityState == STATE_IS_SECURE)
    {
        {
            // workaround bug48515
            nsCOMPtr<nsIURI> aURI;
            aChannel->GetURI(getter_AddRefs(aURI));

            nsXPIDLCString temp;
            aURI->GetSpec(getter_Copies(temp));
            
            if (!strncmp((const char *)temp, "file:", 5) ||
                !strcmp((const char*)temp, "about:layout-dummy-request")) {
                return NS_OK;
            }
        }

        mSecurityState = STATE_IS_BROKEN;
        SetBrokenLockIcon(eventSink, aRequest);

        if (!mPref) return NS_ERROR_NULL_POINTER;

        PRBool boolpref;
        if ((mPref->GetBoolPref(MIXEDCONTENT_PREF, &boolpref) != 0))
            boolpref = PR_TRUE;

        if (boolpref && !mMixContentAlertShown) 
        {
            nsCOMPtr<nsIPrompt> dialog;
            mWindow->GetPrompter(getter_AddRefs(dialog));
            if (!dialog) 
                return NS_ERROR_FAILURE;

            nsAutoString windowTitle, message, dontShowAgain;

            GetBundleString(NS_ConvertASCIItoUCS2("Title"), windowTitle);
            GetBundleString(NS_ConvertASCIItoUCS2("MixedContentMessage"), message);
            GetBundleString(NS_ConvertASCIItoUCS2("DontShowAgain"), dontShowAgain);

            PRBool outCheckValue = PR_TRUE;

            rv  = dialog->AlertCheck(windowTitle.GetUnicode(), 
                                     message.GetUnicode(), 
                                     dontShowAgain.GetUnicode(), 
                                     &outCheckValue);
            if (NS_FAILED(rv)) 
                return rv;
                

            if (!outCheckValue) {
                mPref->SetBoolPref(MIXEDCONTENT_PREF, PR_FALSE);
                NS_WITH_SERVICE(nsIPSMComponent, psm, PSM_COMPONENT_CONTRACTID, &rv);
                if (NS_FAILED(rv)) 
                    return rv;
                psm->PassPrefs();
            }

            mMixContentAlertShown = PR_TRUE;
        }
    }

  return NS_OK;
}

nsresult
nsSecureBrowserUIImpl::CheckPost(nsIURI *actionURL, PRBool *okayToPost)
{
    PRBool secure, isSecurityAdvisor;
    *okayToPost = PR_TRUE;

    nsresult rv = IsURLHTTPS(actionURL, &secure);
    if (NS_FAILED(rv))
        return rv;
    
    // if we are posting to a secure link from a secure page, all is okay.
    if (secure  && 
        (mSecurityState == STATE_IS_SECURE ||
         mSecurityState == STATE_IS_BROKEN)) {
        return NS_OK;
    }

	// If this is a Personal Security Manager (PSM) url, all is okay
	rv = IsURLfromPSM(actionURL, &isSecurityAdvisor);
	if (NS_FAILED(rv)) {
		return rv;
	}

	if (isSecurityAdvisor) {
		return NS_OK;
	}

    PRBool boolpref = PR_TRUE;    

    // posting to a non https URL.
    mPref->GetBoolPref(INSECURE_SUBMIT_PREF, &boolpref);
    
    if (boolpref) {
        nsCOMPtr<nsIPrompt> dialog;
        mWindow->GetPrompter(getter_AddRefs(dialog));
        if (!dialog) 
            return NS_ERROR_FAILURE;

        nsAutoString windowTitle, message, dontShowAgain;
        
        GetBundleString(NS_ConvertASCIItoUCS2("Title"), windowTitle);
        GetBundleString(NS_ConvertASCIItoUCS2("DontShowAgain"), dontShowAgain);

        // posting to insecure webpage from a secure webpage.
        if (!secure  && mSecurityState == STATE_IS_SECURE) {
            GetBundleString(NS_ConvertASCIItoUCS2("PostToInsecure"), message);
        } else { // anything else, post generic warning
            GetBundleString(NS_ConvertASCIItoUCS2("PostToInsecureFromInsecure"), message);
        }

        PRBool outCheckValue = PR_TRUE;
        rv  = dialog->ConfirmCheck(windowTitle.GetUnicode(), 
                                   message.GetUnicode(), 
                                   dontShowAgain.GetUnicode(), 
                                   &outCheckValue, 
                                   okayToPost);
        if (NS_FAILED(rv)) 
            return rv;
        
        if (!outCheckValue) {
            mPref->SetBoolPref(INSECURE_SUBMIT_PREF, PR_FALSE);
            NS_WITH_SERVICE(nsIPSMComponent, psm, PSM_COMPONENT_CONTRACTID, &rv);
            if (NS_FAILED(rv)) 
                return rv;
            return psm->PassPrefs();
        }
    }
  
    return NS_OK;
}

nsresult
nsSecureBrowserUIImpl::SetBrokenLockIcon(nsISecurityEventSink* eventSink, nsIRequest* aRequest, PRBool removeValue)
{
  nsresult rv = NS_OK;
  if (removeValue)
  {
      if (mSecurityButton)
          rv = mSecurityButton->RemoveAttribute( NS_ConvertASCIItoUCS2("level") );
      if (eventSink)
          (void) eventSink->OnSecurityChange(aRequest, (STATE_IS_INSECURE));
  }
  else
  {
      if (mSecurityButton)
          rv = mSecurityButton->SetAttribute( NS_ConvertASCIItoUCS2("level"), NS_ConvertASCIItoUCS2("broken") );
      if (eventSink)
          (void) eventSink->OnSecurityChange(aRequest, (STATE_IS_BROKEN));
  }
  
  nsAutoString tooltiptext;
  GetBundleString(NS_ConvertASCIItoUCS2("SecurityButtonTooltipText"), tooltiptext);
  if (mSecurityButton)
      rv = mSecurityButton->SetAttribute( NS_ConvertASCIItoUCS2("tooltiptext"), tooltiptext );
  return rv;
}





