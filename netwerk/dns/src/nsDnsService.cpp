/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
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
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#include "nsCOMPtr.h"
#include "nsDnsService.h"
#include "nsIDNSListener.h"
#include "nsIRequest.h"
#include "nsISupportsArray.h"
#include "nsError.h"
#include "prnetdb.h"
#include "nsString2.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "netCore.h"

static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);

////////////////////////////////////////////////////////////////////////////////
// Platform specific defines and includes
////////////////////////////////////////////////////////////////////////////////
// PC
#if defined(XP_PC)
#define WM_DNS_SHUTDOWN         (WM_USER + 1)
#endif /* XP_PC */

// MAC
#if defined(XP_MAC)
#include "pprthred.h"

#if TARGET_CARBON

#define nsDNS_NOTIFIER_ROUTINE	nsDnsServiceNotifierRoutineUPP
#define INIT_OPEN_TRANSPORT()	InitOpenTransport(mClientContext, kInitOTForExtensionMask)
#define OT_OPEN_INTERNET_SERVICES(config, flags, err)	OTOpenInternetServices(config, flags, err, mClientContext)
#define OT_OPEN_ENDPOINT(config, flags, info, err)		OTOpenEndpoint(config, flags, info, err, mClientContext)

#else

#define nsDNS_NOTIFIER_ROUTINE	nsDnsServiceNotifierRoutine
#define INIT_OPEN_TRANSPORT()	InitOpenTransport()
#define OT_OPEN_INTERNET_SERVICES(config, flags, err)	OTOpenInternetServices(config, flags, err)
#define OT_OPEN_ENDPOINT(config, flags, info, err)		OTOpenEndpoint(config, flags, info, err)
#endif /* TARGET_CARBON */

typedef struct nsInetHostInfo {
    InetHostInfo    hostInfo;
    nsDNSLookup *   lookup;
} nsInetHostInfo;

typedef struct nsLookupElement {
	QElem	         qElem;
	nsDNSLookup *    lookup;
	
} nsLookupElement;
#endif /* XP_MAC */

// UNIX
// XXX needs implementation

class nsDNSLookup
{
public:
    nsDNSLookup(nsISupports * clientContext, const char * hostName, nsIDNSListener* listener);
    virtual ~nsDNSLookup(void);

    nsresult            FinishHostEntry(void);
    nsresult            CallOnFound(void);
    const char *        HostName(void)  { return mHostName; }
    nsresult            InitiateDNSLookup(nsDNSService * dnsService);

protected:
    // Input when creating a nsDNSLookup
    nsCOMPtr<nsIDNSListener>    mListener;
    nsCOMPtr<nsISupports>       mContext;
    char *                      mHostName;
    // Result of the DNS Lookup
    nsHostEnt                   mHostEntry;
    nsresult                    mResult;
    PRBool                      mComplete;

    // Platform specific portions
#if defined(XP_MAC)
    friend pascal void nsDnsServiceNotifierRoutine(void * contextPtr, OTEventCode code, OTResult result, void * cookie);
    nsLookupElement             mLookupElement;
    nsInetHostInfo              mInetHostInfo;
#endif

#if defined(XP_PC)
    friend static LRESULT CALLBACK nsDNSEventProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HANDLE                      mLookupHandle;
#endif
};


////////////////////////////////////////////////////////////////////////////////
// utility routines:
////////////////////////////////////////////////////////////////////////////////
//
// Allocate space from the buffer, aligning it to "align" before doing
// the allocation. "align" must be a power of 2.
// NOTE: this code was taken from NSPR.
static char *BufAlloc(PRIntn amount, char **bufp, PRIntn *buflenp, PRIntn align) {
	char *buf = *bufp;
	PRIntn buflen = *buflenp;

	if (align && ((long)buf & (align - 1))) {
		PRIntn skip = align - ((ptrdiff_t)buf & (align - 1));
		if (buflen < skip) {
			return 0;
		}
		buf += skip;
		buflen -= skip;
	}
	if (buflen < amount) {
		return 0;
	}
	*bufp = buf + amount;
	*buflenp = buflen - amount;
	return buf;
}


////////////////////////////////////////////////////////////////////////////////
// nsDNSLookup methods:
////////////////////////////////////////////////////////////////////////////////

nsDNSLookup::nsDNSLookup(nsISupports * clientContext, const char * hostName, nsIDNSListener* listener)
{
    // Store input into member variables
    mHostName = new char [PL_strlen(hostName) + 1];
    PL_strcpy(mHostName, hostName);
    mContext = clientContext;
    mListener = listener;

    // Initialize result holders
    mHostEntry.bufLen = PR_NETDB_BUF_SIZE;
    mHostEntry.bufPtr = mHostEntry.buffer;
    mComplete = PR_FALSE;
    mResult = NS_OK;

    // Platform specific initializations
#if defined(XP_MAC)
    mInetHostInfo.lookup = this;
    mLookupElement.lookup = this;
#endif
}


nsDNSLookup::~nsDNSLookup(void)
{
    if (mHostName)
    	delete [] mHostName;
}


nsresult
nsDNSLookup::FinishHostEntry(void)
{
#if defined(XP_MAC)
    PRIntn  len, count, i;

    // convert InetHostInfo to PRHostEnt
    
    // copy name
    len = nsCRT::strlen(mInetHostInfo.hostInfo.name);
    mHostEntry.hostEnt.h_name = BufAlloc(len, &mHostEntry.bufPtr, &mHostEntry.bufLen, 0);
    NS_ASSERTION(nsnull != mHostEntry.hostEnt.h_name,"nsHostEnt buffer full.");
    if (mHostEntry.hostEnt.h_name == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    nsCRT::memcpy(mHostEntry.hostEnt.h_name, mInetHostInfo.hostInfo.name, len);
    
    // set aliases to nsnull
    mHostEntry.hostEnt.h_aliases = (char **)BufAlloc(sizeof(char *), &mHostEntry.bufPtr, &mHostEntry.bufLen, sizeof(char *));
    NS_ASSERTION(nsnull != mHostEntry.hostEnt.h_aliases,"nsHostEnt buffer full.");
    if (mHostEntry.hostEnt.h_aliases == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    *mHostEntry.hostEnt.h_aliases = nsnull;
    
    // fill in address type
    mHostEntry.hostEnt.h_addrtype = AF_INET;
    
    // set address length
    mHostEntry.hostEnt.h_length = sizeof(long);
    
    // count addresses and allocate storage for the pointers
    for (count = 1; count < kMaxHostAddrs && mInetHostInfo.hostInfo.addrs[count] != nsnull; count++){;}
    mHostEntry.hostEnt.h_addr_list = (char **)BufAlloc(count * sizeof(char *), &mHostEntry.bufPtr, &mHostEntry.bufLen, sizeof(char *));
    NS_ASSERTION(nsnull != mHostEntry.hostEnt.h_addr_list,"nsHostEnt buffer full.");
    if (mHostEntry.hostEnt.h_addr_list == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;

    // copy addresses one at a time
    len = mHostEntry.hostEnt.h_length;
    for (i = 0; i < kMaxHostAddrs && mInetHostInfo.hostInfo.addrs[i] != nsnull; i++)
    {
        mHostEntry.hostEnt.h_addr_list[i] = BufAlloc(len, &mHostEntry.bufPtr, &mHostEntry.bufLen, len);
        NS_ASSERTION(nsnull != mHostEntry.hostEnt.h_addr_list[i],"nsHostEnt buffer full.");
        if (mHostEntry.hostEnt.h_addr_list[i] == nsnull)
            return NS_ERROR_OUT_OF_MEMORY;
        *(InetHost *)mHostEntry.hostEnt.h_addr_list[i] = mInetHostInfo.hostInfo.addrs[i];
    }
#endif

    mComplete = PR_TRUE;

    return NS_OK;
}


nsresult
nsDNSLookup::CallOnFound(void)
{
    nsresult result;
    // iterate through request queue calling listeners
    
    // but for now just do this
    if (NS_SUCCEEDED(mResult))
    {
        result = mListener->OnFound(mContext, mHostName, &mHostEntry);
    }
    result = mListener->OnStopLookup(mContext, mHostName, mResult);

    mListener = 0;
    mContext = 0;
	
    return result;
}


nsresult
nsDNSLookup::InitiateDNSLookup(nsDNSService * dnsService)
{
	nsresult rv = NS_OK;
	
#if defined(XP_MAC)
	OSErr   err;
	
    err = OTInetStringToAddress(dnsService->mServiceRef, (char *)mHostName, (InetHostInfo *)&mInetHostInfo);
    if (err != noErr)
        rv = NS_ERROR_UNEXPECTED;
#endif /* XP_MAC */

#if defined(XP_PC)
    mLookupHandle = WSAAsyncGetHostByName(dnsService->mDNSWindow, dnsService->mMsgFoundDNS,
                                          mHostName, (char *)&mHostEntry.hostEnt, PR_NETDB_BUF_SIZE);
    // check for error conditions
    if (mLookupHandle == nsnull) {
        rv = NS_ERROR_UNEXPECTED;  // or call WSAGetLastError() for details;
    }
#endif /* XP_PC */

#ifdef XP_UNIX
    // temporary SYNC version
    PRStatus status = PR_GetHostByName(mHostName, mHostEntry.buffer, 
                                       PR_NETDB_BUF_SIZE, 
                                       &(mHostEntry.hostEnt));
    if (PR_SUCCESS != status) rv = NS_ERROR_UNKNOWN_HOST;
    mResult = rv;

    CallOnFound();
#endif /* XP_UNIX */

    return rv;
}


////////////////////////////////////////////////////////////////////////////////
// Platform specific helper routines
////////////////////////////////////////////////////////////////////////////////

#if defined(XP_MAC)
pascal void  nsDnsServiceNotifierRoutine(void * contextPtr, OTEventCode code, OTResult result, void * cookie);

pascal void  nsDnsServiceNotifierRoutine(void * contextPtr, OTEventCode code, OTResult result, void * cookie)
{
#pragma unused(contextPtr)

	if (code == T_DNRSTRINGTOADDRCOMPLETE) {
        nsDNSService *  dnsService = (nsDNSService *)contextPtr;
        nsDNSLookup *   dnsLookup = ((nsInetHostInfo *)cookie)->lookup;
        PRThread *      thread;
        
        if (result != kOTNoError)
            dnsLookup->mResult = NS_ERROR_UNKNOWN_HOST;
        
        // queue result & wake up dns service thread
        Enqueue((QElem *)&dnsLookup->mLookupElement, &dnsService->mCompletionQueue);

        dnsService->mThread->GetPRThread(&thread);
        if (thread)
	        PR_Mac_PostAsyncNotify(thread);
    }
	// or else we don't handle the event
}
#endif /* XP_MAC */

#if defined(XP_PC)

static LRESULT CALLBACK
nsDNSEventProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = nsnull;
    int     error = nsnull;

    nsDNSService *  dnsService = (nsDNSService *)GetWindowLong(hWnd, GWL_USERDATA);

    if ((dnsService != nsnull) && (uMsg == dnsService->mMsgFoundDNS)) {
        // dns lookup complete - get error code
        error = WSAGETASYNCERROR(lParam);
        
        // which lookup completed?  fetch lookup for this (HANDLE)wParam
        PRInt32 index;
        nsDNSLookup * lookup;
        nsresult    rv;
        
        index = dnsService->mCompletionQueue.Count();
        while (index) {
            lookup = (nsDNSLookup *)dnsService->mCompletionQueue.ElementAt(index-1);
            if (lookup->mLookupHandle == (HANDLE)wParam) {
                result = dnsService->mCompletionQueue.RemoveElement(lookup);
                // check result
                lookup->mComplete = PR_TRUE;
                
                if (error != 0) {
                    lookup->mResult = NS_ERROR_UNKNOWN_HOST;
                }
                rv = lookup->CallOnFound();
                delete lookup;  // for now, until we implement a dns cache
                break;
            }
            index--;
        }

        result = 0;
    }
    else if (uMsg == WM_DNS_SHUTDOWN) {
        // dispose DNS EventHandler Window
        (void) DestroyWindow(dnsService->mDNSWindow);
        PostQuitMessage(0);
        result = 0;
    }
    else {
        result = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return result;
}
#endif /* XP_PC */

#ifdef XP_UNIX

#endif /* XP_UNIX */

////////////////////////////////////////////////////////////////////////////////
// nsDNSService methods:
////////////////////////////////////////////////////////////////////////////////

nsDNSService::nsDNSService()
{
    NS_INIT_REFCNT();
    
    mThread = nsnull;

#if defined(XP_MAC)
    mThreadRunning = PR_FALSE;
    mServiceRef = nsnull;

    mCompletionQueue.qFlags = 0;
    mCompletionQueue.qHead = nsnull;
    mCompletionQueue.qTail = nsnull;
    
#if TARGET_CARBON
    mClientContext = nsnull;
#endif /* TARGET_CARBON */
#endif /* defined(XP_MAC) */
}


nsresult
nsDNSService::Init()
{
    nsresult rv = NS_OK;
//    initialize DNS cache (persistent?)
#if defined(XP_MAC)
//    create Open Transport Service Provider for DNS Lookups
    OSStatus    errOT;

#if TARGET_CARBON
    nsDnsServiceNotifierRoutineUPP	=  NewOTNotifyUPP(nsDnsServiceNotifierRoutine);

    errOT = OTAllocClientContext((UInt32)0, &clientContext);
    NS_ASSERTION(err == kOTNoError, "error allocating OTClientContext.");
#endif /* TARGET_CARBON */

    errOT = INIT_OPEN_TRANSPORT();
    NS_ASSERTION(errOT == kOTNoError, "InitOpenTransport failed.");

    mServiceRef = OT_OPEN_INTERNET_SERVICES(kDefaultInternetServicesPath, NULL, &errOT);
    if (errOT != kOTNoError) return NS_ERROR_UNEXPECTED;    /* no network -- oh well */
    NS_ASSERTION((mServiceRef != NULL) && (errOT == kOTNoError), "error opening OT service.");

    /* Install notify function for DNR Address To String completion */
    errOT = OTInstallNotifier(mServiceRef, nsDNS_NOTIFIER_ROUTINE, this);
    NS_ASSERTION(errOT == kOTNoError, "error installing dns notification routine.");

    /* Put us into async mode */
    if (errOT == kOTNoError) {
        errOT = OTSetAsynchronous(mServiceRef);
        NS_ASSERTION(errOT == kOTNoError, "error setting service to async mode.");
    } else {
        // if either of the two previous calls failed then dealloc service ref and return NS_ERROR_UNEXPECTED
        OSStatus status = OTCloseProvider((ProviderRef)mServiceRef);
        return NS_ERROR_UNEXPECTED;
    }
#endif

#if defined(XP_PC)
    // sync with DNS thread to allow it to create the DNS window
    PRMonitor * monitor;
    PRStatus    status;

    monitor = PR_CEnterMonitor(this);
#endif

#if defined(XP_MAC) || defined(XP_PC)
    // create DNS thread
    rv = NS_NewThread(&mThread, this, 0, PR_JOINABLE_THREAD);
#endif


#if defined(XP_PC)
    status = PR_CWait(this, PR_INTERVAL_NO_TIMEOUT);
    status = PR_CExitMonitor(this);
#endif

	return rv;
}


nsDNSService::~nsDNSService()
{
    Shutdown();
}


NS_IMPL_ISUPPORTS2(nsDNSService, nsIDNSService, nsIRunnable);


NS_METHOD
nsDNSService::Create(nsISupports* aOuter, const nsIID& aIID, void* *aResult)
{
    nsresult rv;
    
    if (aOuter != nsnull)
    	return NS_ERROR_NO_AGGREGATION;
    
    nsDNSService* dnsService = new nsDNSService();
    if (dnsService == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(dnsService);
    rv = dnsService->Init();
    if (NS_SUCCEEDED(rv)) {
        rv = dnsService->QueryInterface(aIID, aResult);
    }
    NS_RELEASE(dnsService);
    return rv;
}

nsresult
nsDNSService::InitDNSThread(void)
{
#if defined(XP_PC)
    WNDCLASS    wc;
    char *      windowClass = "Mozilla:DNSWindowClass";

    // allocate message id for event handler
    mMsgFoundDNS = RegisterWindowMessage("Mozilla:DNSMessage");

    // register window class for DNS event receiver window
    memset(&wc, 0, sizeof(wc));
    wc.style            = 0;
    wc.lpfnWndProc      = nsDNSEventProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = NULL;
    wc.hIcon            = NULL;
    wc.hbrBackground    = (HBRUSH) NULL;
    wc.lpszMenuName     = (LPCSTR) NULL;
    wc.lpszClassName    = windowClass;
    RegisterClass(&wc);

    // create DNS event receiver window
    mDNSWindow = CreateWindow(windowClass, "Mozilla:DNSWindow",
                              0, 0, 0, 10, 10, NULL, NULL, NULL, NULL);

    (void) SetWindowLong(mDNSWindow, GWL_USERDATA, (long)this);

    // sync with Create thread
    PRMonitor * monitor;
    PRStatus    status;

    monitor = PR_CEnterMonitor(this);
    mState = NS_OK;
    status = PR_CNotify(this);
    status = PR_CExitMonitor(this);
#endif /* XP_PC */

    return NS_OK;
}

//
// --------------------------------------------------------------------------
// nsIRunnable implementation...
// --------------------------------------------------------------------------
//
NS_IMETHODIMP
nsDNSService::Run(void)
{
    nsresult            rv = NS_OK;

#if defined(XP_PC)
    MSG msg;
    
    InitDNSThread();

    while(GetMessage(&msg, mDNSWindow, 0, 0)) {
        // no TranslateMessage() because we're not expecting input
        DispatchMessage(&msg);
    }
#endif /* XP_PC */

#if defined(XP_MAC)
    OSErr			    err;
    nsLookupElement *   lookupElement;
    nsDNSLookup *       lookup;
    
    mThreadRunning = PR_TRUE;
	
    while (mThreadRunning) {
		
        PR_Mac_WaitForAsyncNotify(PR_INTERVAL_NO_TIMEOUT);
        // check queue for completed DNS lookups
        while ((lookupElement = (nsLookupElement *)mCompletionQueue.qHead) != nsnull) {

            err = Dequeue((QElemPtr)lookupElement, &mCompletionQueue);
            if (err)
        	    continue;		// assert
        
            lookup = lookupElement->lookup;
            // convert InetHostInfo to nsHostEnt
            rv = lookup->FinishHostEntry();
            if (NS_SUCCEEDED(rv)) {
                // put lookup in cache
            }
            
            // issue callbacks
            rv = lookup->CallOnFound();
            
            delete lookup;  // until we start caching them
            
        }
	}
#endif /* XP_MAC */

    return rv;
}

    
////////////////////////////////////////////////////////////////////////////////
// nsIDNSService methods:
////////////////////////////////////////////////////////////////////////////////


NS_IMETHODIMP
nsDNSService::Lookup(nsISupports*    clientContext,
                     const char*     hostName,
                     nsIDNSListener* listener,
                     nsIRequest*     *DNSRequest)
{
    nsresult	rv = NS_OK;

    NS_WITH_SERVICE(nsIIOService, ios, kIOServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;
    PRBool offline;
    rv = ios->GetOffline(&offline);
    if (NS_FAILED(rv)) return rv;
    if (offline) return NS_ERROR_OFFLINE;

    PRStatus    status = PR_SUCCESS;

    PRBool numeric = PR_TRUE;
    for (const char *hostCheck = hostName; *hostCheck; hostCheck++) {
        if (!nsString2::IsDigit(*hostCheck) && (*hostCheck != '.') ) {
            numeric = PR_FALSE;
            break;
        }
    }

    if (numeric) {
        nsHostEnt*  hostentry = new nsHostEnt;
        if (!hostentry) return NS_ERROR_OUT_OF_MEMORY;

        (void)listener->OnStartLookup(clientContext, hostName);

        // If it is numeric then try to convert it into an IP-Address
        PRNetAddr *netAddr = (PRNetAddr*)nsAllocator::Alloc(sizeof(PRNetAddr));
        if (!netAddr) {
            delete hostentry;
            return NS_ERROR_OUT_OF_MEMORY;
        }
        status = PR_StringToNetAddr(hostName, netAddr);
        if (PR_SUCCESS == status) {
            // slam the IP in and move on.
            PRHostEnt *ent = &(hostentry->hostEnt);
            PRIntn bufLen = hostentry->bufLen = PR_NETDB_BUF_SIZE;
            char *buffer = hostentry->buffer;
            ent->h_name = (char*)BufAlloc(PL_strlen(hostName) + 1,
                                          &buffer,
                                          &bufLen,
                                          0);
            memcpy(ent->h_name, hostName, PL_strlen(hostName) + 1);

            ent->h_aliases = (char**)BufAlloc(1 * sizeof(char*),
                                              &buffer,
                                              &bufLen,
                                              sizeof(char **));
            ent->h_aliases[0] = '\0';

            ent->h_addrtype = 2;
            ent->h_length = 4;
            ent->h_addr_list = (char**)BufAlloc(2 * sizeof(char*),
                                                &buffer,
                                                &bufLen,
                                                sizeof(char **));
            ent->h_addr_list[0] = (char*)BufAlloc(ent->h_length,
                                                  &buffer,
                                                  &bufLen,
                                                  0);
            memcpy(ent->h_addr_list[0], &netAddr->inet.ip, ent->h_length);
            ent->h_addr_list[1] = '\0';

            (void)listener->OnFound(clientContext, hostName, hostentry);
            
            delete hostentry;
            // XXX: The hostentry should really be reference counted so the 
            //      listener does not need to copy it...
	        
            return listener->OnStopLookup(clientContext, hostName, NS_OK);
        }
    }

    // Incomming hostname is not a numeric ip address. Need to do the actual
    // dns lookup.

    // create nsDNSLookup
    nsDNSLookup * lookup = new nsDNSLookup(clientContext, hostName, listener);
    if (!lookup) {
        return NS_ERROR_OUT_OF_MEMORY;
    }

#if defined(XP_PC)
    // save on outstanding lookup queue
    mCompletionQueue.AppendElement(lookup);
#endif /* XP_PC */

    (void)listener->OnStartLookup(clientContext, hostName);

    // initiate async lookup
    return lookup->InitiateDNSLookup(this);	
}

NS_IMETHODIMP
nsDNSService::Shutdown()
{
    nsresult rv = NS_OK;

    // XXX clean up outstanding requests

    // XXX deallocate cache

#if defined(XP_MAC)

    mThreadRunning = PR_FALSE;

    // deallocate Open Transport Service Provider
    OSStatus status = OTCloseProvider((ProviderRef)mServiceRef);
    CloseOpenTransport();           // should be moved to terminate routine
    PRThread* dnsServiceThread;
    rv = mThread->GetPRThread(&dnsServiceThread);
    if (dnsServiceThread)
        PR_Mac_PostAsyncNotify(dnsServiceThread);
    rv = mThread->Join();

#elif defined(XP_PC)

    SendMessage(mDNSWindow, WM_DNS_SHUTDOWN, 0, 0);
    rv = mThread->Join();

#elif defined(XP_UNIX)
    // XXXX - ?
#endif

    return rv;
}
