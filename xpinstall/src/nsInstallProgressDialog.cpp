/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, 
 * released March 31, 1998. 
 *
 * The Initial Developer of the Original Code is Netscape Communications 
 * Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 *
 * Contributors:
 *     Douglas Turner <dougt@netscape.com>
 */


#include "nsIXPINotifier.h"
#include "nsInstallProgressDialog.h"

#include "nsIAppShellComponentImpl.h"

#include "nsIServiceManager.h"
#include "nsIDocumentViewer.h"
#include "nsIContent.h"
#include "nsINameSpaceManager.h"
#include "nsIContentViewer.h"
#include "nsIDOMElement.h"
#include "nsIURL.h"
#ifndef NECKO
#include "nsINetService.h"
#else
#include "nsIIOService.h"
#include "nsIURI.h"
#include "nsIServiceManager.h"
static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);
#endif // NECKO
#include "nsIWebShell.h"
#include "nsIWebShellWindow.h"

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID( kAppShellServiceCID, NS_APPSHELL_SERVICE_CID );

nsInstallProgressDialog::nsInstallProgressDialog()
{
    NS_INIT_REFCNT();
    mWindow = nsnull;
    mDocument = nsnull;

}

nsInstallProgressDialog::~nsInstallProgressDialog()
{
}


NS_IMPL_ADDREF( nsInstallProgressDialog );
NS_IMPL_RELEASE( nsInstallProgressDialog );

NS_IMETHODIMP 
nsInstallProgressDialog::QueryInterface(REFNSIID aIID,void** aInstancePtr)
{
  if (aInstancePtr == NULL) {
    return NS_ERROR_NULL_POINTER;
  }

  // Always NULL result, in case of failure
  *aInstancePtr = NULL;

  if (aIID.Equals(nsIXPINotifier::GetIID())) {
    *aInstancePtr = (void*) ((nsIXPINotifier*)this);
    NS_ADDREF_THIS();
    return NS_OK;
  }
  if (aIID.Equals(nsIXPIProgressDlg::GetIID())) {
    *aInstancePtr = (void*) ((nsIXPIProgressDlg*)this);
    NS_ADDREF_THIS();
    return NS_OK;
  }
  if (aIID.Equals(nsIXULWindowCallbacks::GetIID())) {
    *aInstancePtr = (void*) ((nsIXULWindowCallbacks*)this);
    NS_ADDREF_THIS();
    return NS_OK;
  }
  if (aIID.Equals(kISupportsIID)) {
    *aInstancePtr = (void*) (nsISupports*)((nsIXPINotifier*)this);
    NS_ADDREF_THIS();
    return NS_OK;
  }

  return NS_ERROR_NO_INTERFACE;
}

NS_IMETHODIMP 
nsInstallProgressDialog::BeforeJavascriptEvaluation()
{
    return Open();
}

NS_IMETHODIMP 
nsInstallProgressDialog::AfterJavascriptEvaluation()
{
    return Close();
}

NS_IMETHODIMP 
nsInstallProgressDialog::InstallStarted(const char *UIPackageName)
{
    return SetHeading( nsString(UIPackageName).GetUnicode() );
}

NS_IMETHODIMP 
nsInstallProgressDialog::ItemScheduled(const char *message)
{
    nsresult rv = SetActionText( nsString(message).GetUnicode() );

    if (NS_SUCCEEDED(rv))
    {
        PRBool cancel;
        rv = GetCancelStatus( &cancel );

        if ( NS_SUCCEEDED(rv) && cancel)
            return NS_ERROR_FAILURE;  // XXX: Not a COM failure! change interface to return val
    }
    return rv;
}

NS_IMETHODIMP 
nsInstallProgressDialog::InstallFinalization(const char *message, PRInt32 itemNum, PRInt32 totNum)
{

    nsresult rv = SetActionText( nsString(message).GetUnicode() );

    if (NS_SUCCEEDED(rv))
        rv = SetProgress( itemNum, totNum );
    
    return rv;
}

NS_IMETHODIMP 
nsInstallProgressDialog::InstallAborted()
{
    return NS_OK;
}




// Do startup stuff from C++ side.
NS_IMETHODIMP
nsInstallProgressDialog::ConstructBeforeJavaScript(nsIWebShell *aWebShell) 
{
    nsresult rv = NS_OK;

    // Get content viewer from the web shell.
    nsCOMPtr<nsIContentViewer> contentViewer;
    rv = aWebShell ? aWebShell->GetContentViewer(getter_AddRefs(contentViewer))
                   : NS_ERROR_NULL_POINTER;

    if ( contentViewer ) {
        // Up-cast to a document viewer.
        nsCOMPtr<nsIDocumentViewer> docViewer( do_QueryInterface( contentViewer, &rv ) );
        if ( docViewer ) {
            // Get the document from the doc viewer.
            nsCOMPtr<nsIDocument> document;
            rv = docViewer->GetDocument(*getter_AddRefs(document));
            if ( document ) {
                // Upcast to XUL document.
                mDocument = do_QueryInterface( document, &rv );
                if ( ! mDocument ) 
                {
                    DEBUG_PRINTF( PR_STDOUT, "%s %d: Upcast to nsIDOMXULDocument failed, rv=0x%X\n",
                                  __FILE__, (int)__LINE__, (int)rv );
                }
            } 
            else 
            {
                DEBUG_PRINTF( PR_STDOUT, "%s %d: GetDocument failed, rv=0x%X\n",
                              __FILE__, (int)__LINE__, (int)rv );
            }
        } 
        else 
        {
            DEBUG_PRINTF( PR_STDOUT, "%s %d: Upcast to nsIDocumentViewer failed, rv=0x%X\n",
                          __FILE__, (int)__LINE__, (int)rv );
        }
    } 
    else 
    {
        DEBUG_PRINTF( PR_STDOUT, "%s %d: GetContentViewer failed, rv=0x%X\n",
                      __FILE__, (int)__LINE__, (int)rv );
    }

    return rv;
}


NS_IMETHODIMP
nsInstallProgressDialog::Open()
{
    nsresult rv = NS_OK;

    // Get app shell service.
    nsIAppShellService *appShell;
    rv = nsServiceManager::GetService( kAppShellServiceCID,
                                       nsIAppShellService::GetIID(),
                                       (nsISupports**)&appShell );

    if ( NS_SUCCEEDED( rv ) ) 
    {
        // Open "progress" dialog.
        nsIURL *url;
        char * urlStr = "resource:/res/xpinstall/progress.xul";
#ifndef NECKO
        rv = NS_NewURL( &url, urlStr );
#else
        NS_WITH_SERVICE(nsIIOService, service, kIOServiceCID, &rv);
        if (NS_FAILED(rv)) return rv;

        nsIURI *uri = nsnull;
        rv = service->NewURI(urlStr, nsnull, &uri);
        if (NS_FAILED(rv)) return rv;

        rv = uri->QueryInterface(nsIURL::GetIID(), (void**)&url);
        NS_RELEASE(uri);
#endif // NECKO
        
        if ( NS_SUCCEEDED(rv) ) 
        {
        
            nsCOMPtr<nsIWebShellWindow> newWindow;
            rv = appShell->CreateTopLevelWindow( nsnull,
                                                 url,
                                                 PR_TRUE,
                                                 *getter_AddRefs(newWindow),
                                                 nsnull,
                                                 this,  // callbacks??
                                                 0,
                                                 0 );

            if ( NS_SUCCEEDED( rv ) ) 
            {
                mWindow = newWindow;			// ownership?

                 if (mWindow != nsnull)
                    mWindow->Show(PR_TRUE);
            }
            else 
            {
                DEBUG_PRINTF( PR_STDOUT, "Error creating progress dialog, rv=0x%X\n", (int)rv );
            }
            NS_RELEASE( url );
        }
        
        nsServiceManager::ReleaseService( kAppShellServiceCID, appShell );
    } 
    else 
    {
        DEBUG_PRINTF( PR_STDOUT, "Unable to get app shell service, rv=0x%X\n", (int)rv );
    }
    return NS_OK;
}

NS_IMETHODIMP
nsInstallProgressDialog::Close()
{
    if (mWindow)
    {
        mWindow->Close();
    }
    return NS_OK;
}

NS_IMETHODIMP
nsInstallProgressDialog::SetTitle(const PRUnichar * aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsInstallProgressDialog::SetHeading(const PRUnichar * aHeading)
{
    return setDlgAttribute( "dialog.uiPackageName", "value", nsString(aHeading) );
}

NS_IMETHODIMP
nsInstallProgressDialog::SetActionText(const PRUnichar * aActionText)
{
    const PRInt32 maxChars = 40;

    nsString theMessage(aActionText);
    PRInt32 len = theMessage.Length();
    if (len > maxChars)
    {
        PRInt32 offset = (len/2) - ((len - maxChars)/2);
        PRInt32 count  = (len - maxChars);
        theMessage.Cut(offset, count);  
        theMessage.Insert(nsString("..."), offset);
    }

    return setDlgAttribute( "dialog.currentAction", "value", theMessage );
}

NS_IMETHODIMP
nsInstallProgressDialog::SetProgress(PRInt32 aValue, PRInt32 aMax)
{
    char buf[16];
    
    PR_snprintf( buf, sizeof buf, "%lu", aMax );
    nsresult rv = setDlgAttribute( "dialog.progress", "max", buf );
   
    if ( NS_SUCCEEDED(rv))
    {
        if (aMax != 0)
            PR_snprintf( buf, sizeof buf, "%lu", ((aMax-aValue)/aMax) );
        else
            PR_snprintf( buf, sizeof buf, "%lu", 0 );

        rv = setDlgAttribute( "dialog.progress", "value", buf );
    }
    return rv;
}

NS_IMETHODIMP
nsInstallProgressDialog::GetCancelStatus(PRBool *_retval)
{
    *_retval = PR_FALSE;
    return NS_OK;
}

// Utility to set element attribute.
nsresult nsInstallProgressDialog::setDlgAttribute( const char *id,
                                                   const char *name,
                                                   const nsString &value )
{
    nsresult rv = NS_OK;

    if ( mDocument ) {
        // Find specified element.
        nsCOMPtr<nsIDOMElement> elem;
        rv = mDocument->GetElementById( id, getter_AddRefs( elem ) );
        if ( elem ) {
            // Set the text attribute.
            rv = elem->SetAttribute( name, value );
            if ( NS_SUCCEEDED( rv ) ) {
            } else {
                 DEBUG_PRINTF( PR_STDOUT, "%s %d: SetAttribute failed, rv=0x%X\n",
                               __FILE__, (int)__LINE__, (int)rv );
            }
        } else {
            DEBUG_PRINTF( PR_STDOUT, "%s %d: GetElementById failed, rv=0x%X\n",
                          __FILE__, (int)__LINE__, (int)rv );
        }
    } else {
        rv = NS_ERROR_NULL_POINTER;
    }
    return rv;
}

// Utility to get element attribute.
nsresult nsInstallProgressDialog::getDlgAttribute(  const char *id,
                                                    const char *name,
                                                    nsString &value ) 
{
    nsresult rv = NS_OK;

    if ( mDocument ) {
        // Find specified element.
        nsCOMPtr<nsIDOMElement> elem;
        rv = mDocument->GetElementById( id, getter_AddRefs( elem ) );
        if ( elem ) {
            // Set the text attribute.
            rv = elem->GetAttribute( name, value );
            if ( NS_SUCCEEDED( rv ) ) {
            } else {
                 DEBUG_PRINTF( PR_STDOUT, "%s %d: GetAttribute failed, rv=0x%X\n",
                               __FILE__, (int)__LINE__, (int)rv );
            }
        } else {
            DEBUG_PRINTF( PR_STDOUT, "%s %d: GetElementById failed, rv=0x%X\n",
                          __FILE__, (int)__LINE__, (int)rv );
        }
    } else {
        rv = NS_ERROR_NULL_POINTER;
    }

    return rv;
}
