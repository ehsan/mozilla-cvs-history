/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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
 * The Original Code is Mozilla.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications.  Portions created by Netscape Communications are
 * Copyright (C) 2001 by Netscape Communications.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 *   Darin Fisher <darin@netscape.com> (original author)
 */

#include "nsHttpConnection.h"
#include "nsHttpTransaction.h"
#include "nsHttpRequestHead.h"
#include "nsHttpResponseHead.h"
#include "nsHttpHandler.h"
#include "nsISocketTransportService.h"
#include "nsISocketTransport.h"
#include "nsIServiceManager.h"
#include "nsISSLSocketControl.h"
#include "nsIStringStream.h"
#include "netCore.h"
#include "nsNetCID.h"
#include "prmem.h"
#include "plevent.h"

static NS_DEFINE_CID(kSocketTransportServiceCID, NS_SOCKETTRANSPORTSERVICE_CID);

//-----------------------------------------------------------------------------
// helpers...
//-----------------------------------------------------------------------------

static void *PR_CALLBACK
TransactionReleaseEventHandler(PLEvent *ev)
{
    nsHttpTransaction *trans = (nsHttpTransaction *) PL_GetEventOwner(ev);

    LOG(("TransactionReleaseEventHandler [trans=%x refcnt=%u] calling release...\n",
        trans, trans->RefCnt()));

    NS_RELEASE(trans);
    return 0;
}

static void PR_CALLBACK
TransactionReleaseDestroyHandler(PLEvent *ev)
{
    delete ev;
}

//-----------------------------------------------------------------------------
// nsHttpConnection <public>
//-----------------------------------------------------------------------------

nsHttpConnection::nsHttpConnection()
    : mTransaction(0)
    , mConnectionInfo(0)
    , mReuseCount(0)
    , mMaxReuseCount(0)
    , mIdleTimeout(0)
    , mLastActiveTime(0)
    , mKeepAlive(0)
{
    LOG(("Creating nsHttpConnection @%x\n", this));

    NS_INIT_ISUPPORTS();
}

nsHttpConnection::~nsHttpConnection()
{
    LOG(("Destroying nsHttpConnection @%x\n", this));
 
    NS_IF_RELEASE(mConnectionInfo);
    mConnectionInfo = 0;

    if (mTransaction) {
        ProxyReleaseTransaction(mTransaction);
        mTransaction = 0;
    }

    // warning: this call could result in OnStopRequest being called.  this
    // is why we are careful to null out mTransaction and mConnectionInfo ;-)
    if (mSocketTransport)
        mSocketTransport->SetReuseConnection(PR_FALSE);
}

nsresult
nsHttpConnection::Init(nsHttpConnectionInfo *info)
{
    LOG(("nsHttpConnection::Init [this=%x]\n"));

    NS_ENSURE_ARG_POINTER(info);
    NS_ENSURE_TRUE(!mConnectionInfo, NS_ERROR_ALREADY_INITIALIZED);

    mConnectionInfo = info;
    NS_ADDREF(mConnectionInfo);

    return NS_OK;
}

// never called from the socket thread
nsresult
nsHttpConnection::SetTransaction(nsHttpTransaction *transaction)
{
    LOG(("nsHttpConnection::SetTransaction [this=%x]\n"));

    NS_ENSURE_TRUE(!mTransaction, NS_ERROR_IN_PROGRESS);
    NS_ENSURE_ARG_POINTER(transaction);

    // take ownership of the transaction
    mTransaction = transaction;
    NS_ADDREF(mTransaction);

    // grab a reference to the calling thread's event queue.
    mEventQ = 0;
    nsCOMPtr<nsIEventQueueService> eqs;
    nsHttpHandler::get()->GetEventQueueService(getter_AddRefs(eqs));
    if (eqs)
        eqs->ResolveEventQueue(NS_CURRENT_EVENTQ, getter_AddRefs(mEventQ));

    // build a proxy for the progress event sink
    mProgressSink = 0;
    if (mTransaction->Callbacks()) {
        nsCOMPtr<nsIProgressEventSink> temp = do_GetInterface(mTransaction->Callbacks());
        if (temp) {
            nsCOMPtr<nsIProxyObjectManager> mgr;
            nsHttpHandler::get()->GetProxyObjectManager(getter_AddRefs(mgr));
            if (mgr)
                mgr->GetProxyForObject(mEventQ,
                                       NS_GET_IID(nsIProgressEventSink),
                                       temp,
                                       PROXY_ASYNC | PROXY_ALWAYS,
                                       getter_AddRefs(mProgressSink));
        }
    }

    // assign ourselves to the transaction
    mTransaction->SetConnection(this);

    return ActivateConnection();
}

// called from the socket thread
nsresult
nsHttpConnection::OnHeadersAvailable(nsHttpTransaction *trans, PRBool *reset)
{
    LOG(("nsHttpConnection::OnHeadersAvailable [this=%x trans=%x]\n",
        this, trans));

    NS_ENSURE_ARG_POINTER(trans);

    // we won't change our keep-alive policy unless the server has explicitly
    // told us to do so.

    if (!trans || !trans->ResponseHead()) {
        LOG(("trans->ResponseHead() = %x\n", trans));
        return NS_OK;
    }

    // inspect the connection headers for keep-alive info provided the
    // transaction completed successfully.
    const char *val = trans->ResponseHead()->PeekHeader(nsHttp::Connection);
    if (!val)
        val = trans->ResponseHead()->PeekHeader(nsHttp::Proxy_Connection);
    if (val) {
        // be pesimistic
        mKeepAlive = PR_FALSE;

        if (PL_strcasecmp(val, "keep-alive") == 0) {
            mKeepAlive = PR_TRUE;

            val = trans->ResponseHead()->PeekHeader(nsHttp::Keep_Alive);

            LOG(("val = [%s]\n", val));

            const char *cp = PL_strcasestr(val, "max=");
            if (cp)
                mMaxReuseCount = (PRUint32) atoi(cp + 4);
            else
                mMaxReuseCount = 100;

            cp = PL_strcasestr(val, "timeout=");
            if (cp)
                mIdleTimeout = (PRUint32) atoi(cp + 8);
            else
                mIdleTimeout = 10;
            
            LOG(("Connection can be reused [this=%x max-reuse=%u "
                 "keep-alive-timeout=%u\n", this, mMaxReuseCount, mIdleTimeout));
        }
    }

    // if we're doing an SSL proxy connect, then we need to check whether or not
    // the connect was successful.  if so, then we have to reset the transaction
    // and step-up the socket connection to SSL. finally, we have to wake up the
    // socket write request.
    if (mSSLProxyConnectStream) {
        mSSLProxyConnectStream = 0;
        if (trans->ResponseHead()->Status() == 200) {
            LOG(("SSL proxy CONNECT succeeded!\n"));
            *reset = PR_TRUE;
            ProxyStepUp();
            mWriteRequest->Resume();
        }
        else {
            LOG(("SSL proxy CONNECT failed!\n"));
            // close out the write request
            mWriteRequest->Cancel(NS_OK);
        }
    }

    return NS_OK;
}

// called from any thread
nsresult
nsHttpConnection::OnTransactionComplete(nsresult status)
{
    LOG(("nsHttpConnection::OnTransactionComplete [this=%x status=%x]\n",
        this, status));

    NS_ENSURE_TRUE(mSocketTransport, NS_ERROR_UNEXPECTED);

    // be warned: trans may not be mTransaction

    // cancel the requests... this will cause OnStopRequest to be fired
    if (mWriteRequest) {
        mWriteRequest->Cancel(status);
        mWriteRequest = 0;
    }
    if (mReadRequest) {
        mReadRequest->Cancel(status);
        mReadRequest = 0;
    }

    // break the cycle between the socket transport and this
    if (mSocketTransport)
        mSocketTransport->SetNotificationCallbacks(nsnull, 0);

    if (!mKeepAlive) {
        // if we're not going to be keeping this connection alive...
        mSocketTransport->SetReuseConnection(PR_FALSE);
        mSocketTransport = 0;
    }

    return NS_OK;
}

// not called from the socket thread
nsresult
nsHttpConnection::Resume()
{
    // XXX may require a lock to ensure thread safety

    if (mReadRequest)
        mReadRequest->Resume();

    return NS_OK;
}

// called from the socket thread
nsresult
nsHttpConnection::ProxyStepUp()
{
    nsCOMPtr<nsISupports> securityInfo;
    nsresult rv;

    LOG(("nsHttpConnection::ProxyStepUp [this=%x]\n", this));

    rv = mSocketTransport->GetSecurityInfo(getter_AddRefs(securityInfo));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISSLSocketControl> ssl = do_QueryInterface(securityInfo, &rv);
    if (NS_FAILED(rv)) return rv;

    return ssl->ProxyStepUp();
}

PRBool
nsHttpConnection::CanReuse()
{
    return (mReuseCount < mMaxReuseCount) && 
           (NowInSeconds() - mLastActiveTime < mIdleTimeout) && IsAlive();
}

PRBool
nsHttpConnection::IsAlive()
{
    if (!mSocketTransport)
        return PR_FALSE;

    PRBool isAlive = PR_FALSE;
    nsresult rv = mSocketTransport->IsAlive(0, &isAlive);
    NS_ASSERTION(NS_SUCCEEDED(rv), "IsAlive test failed");
    return isAlive;
}

void
nsHttpConnection::ReportProgress(PRUint32 progress, PRInt32 progressMax)
{
    if (mProgressSink)
        mProgressSink->OnProgress(nsnull, nsnull, progress,
                                  progressMax < 0 ? 0 : PRUint32(progressMax));
}

//-----------------------------------------------------------------------------
// nsHttpConnection <private>
//-----------------------------------------------------------------------------

nsresult
nsHttpConnection::ActivateConnection()
{
    nsresult rv;

    // if we don't have a socket transport then create a new one
    if (!mSocketTransport) {
        rv = CreateTransport();
        if (NS_FAILED(rv)) return rv;

        // need to handle SSL proxy CONNECT if this is the first time.
        if (mConnectionInfo->UsingSSL() && mConnectionInfo->ProxyHost()) {
            rv = SetupSSLProxyConnect();
            if (NS_FAILED(rv)) return rv;
        }
    }

    // allow the socket transport to call us directly on progress
    rv = mSocketTransport->
            SetNotificationCallbacks(this, nsITransport::DONT_PROXY_PROGRESS);
    if (NS_FAILED(rv)) return rv;

    // note: we pass a reference to ourselves as the context so we can
    // differentiate the OnStopRequest events.
    rv = mSocketTransport->AsyncWrite(this, (nsIStreamListener*) this,
                                      0, PRUint32(-1),
                                      nsITransport::DONT_PROXY_OBSERVER |
                                      nsITransport::DONT_PROXY_PROVIDER,
                                      getter_AddRefs(mWriteRequest));
    if (NS_FAILED(rv)) return rv;

    rv = mSocketTransport->AsyncRead(this, nsnull,
                                     0, PRUint32(-1),
                                     nsITransport::DONT_PROXY_OBSERVER |
                                     nsITransport::DONT_PROXY_LISTENER,
                                     getter_AddRefs(mReadRequest));
    if (NS_FAILED(rv)) return rv;

    return NS_OK;
}

nsresult
nsHttpConnection::CreateTransport()
{
    nsresult rv;

    NS_PRECONDITION(!mSocketTransport, "unexpected");

    nsCOMPtr<nsISocketTransportService> sts =
            do_GetService(kSocketTransportServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    // configure the socket type based on the connection type requested.
    const char *types[3] = {0,0,0};
    PRUint32 count = 0;
    if (mConnectionInfo->UsingSSL()) {
        types[0] = "ssl";
        count++;
    }
    if (!PL_strcasecmp(mConnectionInfo->ProxyType(), "socks")) {
        types[count] = "socks";
        count++;
    }

    nsCOMPtr<nsITransport> transport;
    rv = sts->CreateTransportOfTypes(count, types,
                                     mConnectionInfo->Host(),
                                     mConnectionInfo->Port(),
                                     mConnectionInfo->ProxyHost(),
                                     mConnectionInfo->ProxyPort(),
                                     NS_HTTP_SEGMENT_SIZE,
                                     NS_HTTP_BUFFER_SIZE,
                                     getter_AddRefs(transport));
    if (NS_FAILED(rv)) return rv;

    // the transport has better be a socket transport !!
    mSocketTransport = do_QueryInterface(transport, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = mSocketTransport->SetReuseConnection(PR_TRUE);
    if (NS_FAILED(rv)) return rv;

    return rv;
}

nsresult
nsHttpConnection::ProxyReleaseTransaction(nsHttpTransaction *trans)
{
    LOG(("nsHttpConnection::ProxyReleaseTransaction [this=%x trans=%x refcnt=%u]\n",
        this, trans, trans->RefCnt()));

    NS_ENSURE_TRUE(mEventQ, NS_ERROR_NOT_INITIALIZED);
    NS_ENSURE_ARG_POINTER(trans);

    PLEvent *event = new PLEvent;
    if (!event)
        return NS_ERROR_OUT_OF_MEMORY;

    PL_InitEvent(event, trans,
                 TransactionReleaseEventHandler,
                 TransactionReleaseDestroyHandler);

    return mEventQ->PostEvent(event);
}

nsresult
nsHttpConnection::SetupSSLProxyConnect()
{
    nsresult rv;

    LOG(("nsHttpConnection::SetupSSLProxyConnect [this=%x]\n", this));

    NS_ENSURE_TRUE(!mSSLProxyConnectStream, NS_ERROR_ALREADY_INITIALIZED);

    nsCAutoString buf;
    buf.Assign(mConnectionInfo->Host());
    buf.Append(':');
    buf.AppendInt(mConnectionInfo->Port());

    // CONNECT host:port HTTP/1.1
    nsHttpRequestHead request;
    request.SetMethod(nsHttp::Connect);
    request.SetVersion(nsHttpHandler::get()->DefaultVersion());
    request.SetRequestURI(buf.get());
    request.SetHeader(nsHttp::User_Agent, nsHttpHandler::get()->UserAgent());

    buf.Truncate(0);
    request.Flatten(buf);
    buf.Append("\r\n");

    nsCOMPtr<nsISupports> sup;
    rv = NS_NewCStringInputStream(getter_AddRefs(sup), buf);
    if (NS_FAILED(rv)) return rv;

    mSSLProxyConnectStream = do_QueryInterface(sup, &rv);
    return rv;
}

//-----------------------------------------------------------------------------
// nsHttpConnection::nsISupports
//-----------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ADDREF(nsHttpConnection)
NS_IMPL_THREADSAFE_RELEASE(nsHttpConnection)

NS_INTERFACE_MAP_BEGIN(nsHttpConnection)
    NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
    NS_INTERFACE_MAP_ENTRY(nsIStreamProvider)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsIRequestObserver, nsIStreamListener)
    NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
    NS_INTERFACE_MAP_ENTRY(nsIProgressEventSink)
NS_INTERFACE_MAP_END_THREADSAFE

//-----------------------------------------------------------------------------
// nsHttpConnection::nsIRequestObserver
//-----------------------------------------------------------------------------

// called on the socket transport thread
NS_IMETHODIMP
nsHttpConnection::OnStartRequest(nsIRequest *request, nsISupports *ctxt)
{
    LOG(("nsHttpConnection::OnStartRequest [this=%x]\n", this));
    return NS_OK;
}

// called on the socket transport thread
NS_IMETHODIMP
nsHttpConnection::OnStopRequest(nsIRequest *request, nsISupports *ctxt,
                                nsresult status)
{
    LOG(("nsHttpConnection::OnStopRequest [this=%x ctxt=%x status=%x]\n",
        this, ctxt, status));

    if (!mTransaction)
        return NS_OK;

    if (ctxt == (nsISupports *) (nsIStreamListener *) this)
        mWriteRequest = 0;
    else {
        // Done reading, so signal transaction complete...
        mReadRequest = 0;

        // break the cycle between the socket transport and this
        if (mSocketTransport)
            mSocketTransport->SetNotificationCallbacks(nsnull, 0);
        
        // don't need this anymore
        mProgressSink = 0;

        // make sure mTransaction is clear before calling OnStopTransaction
		nsHttpTransaction *trans = mTransaction;
		mTransaction = nsnull;

        trans->OnStopTransaction(status);

        // because this could be the last reference to the transaction and
        // because we are on the socket transport thread, it is essential that
        // this final release be proxied to the thread which called
        // SetTransaction.
        if (NS_FAILED(ProxyReleaseTransaction(trans)))
            NS_NOTREACHED("proxy release of transaction failed");
    }
    // no point in returning anything else but NS_OK
    return NS_OK;
}

//-----------------------------------------------------------------------------
// nsHttpConnection::nsIStreamProvider
//-----------------------------------------------------------------------------

// called on the socket transport thread
NS_IMETHODIMP
nsHttpConnection::OnDataWritable(nsIRequest *request, nsISupports *context,
                                 nsIOutputStream *outputStream,
                                 PRUint32 offset, PRUint32 count)
{
    if (!mTransaction) {
        LOG(("nsHttpConnection: no transaction! closing stream\n"));
        return NS_BASE_STREAM_CLOSED;
    }

    LOG(("nsHttpConnection::OnDataWritable [this=%x]\n", this));

    // if we're doing an SSL proxy connect, then we need to bypass calling
    // into the transaction.
    if (mSSLProxyConnectStream) {
        PRUint32 n;

        nsresult rv = mSSLProxyConnectStream->Available(&n);
        if (NS_FAILED(rv)) return rv;

        // if there are bytes available in the stream, then write them out.
        // otherwise, suspend the write request... it'll get restarted once
        // we get a response from the proxy server.
        if (n) {
            LOG(("writing data from proxy connect stream [count=%u]\n", n));
            return outputStream->WriteFrom(mSSLProxyConnectStream, n, &n);
        }

        LOG(("done writing proxy connect stream\n"));
        return NS_BASE_STREAM_WOULD_BLOCK;
    }

    LOG(("calling mTransaction->OnDataWritable\n"));

    // in the normal case, we just want to defer to the transaction to write
    // out the request.
    return mTransaction->OnDataWritable(outputStream);
}

//-----------------------------------------------------------------------------
// nsHttpConnection::nsIStreamListener
//-----------------------------------------------------------------------------

// called on the socket transport thread
NS_IMETHODIMP
nsHttpConnection::OnDataAvailable(nsIRequest *request, nsISupports *context,
                                  nsIInputStream *inputStream,
                                  PRUint32 offset, PRUint32 count)
{
    if (!mTransaction) {
        LOG(("nsHttpConnection: no transaction! closing stream\n"));
        return NS_BASE_STREAM_CLOSED;
    }

    mLastActiveTime = NowInSeconds();

    LOG(("nsHttpConnection::OnDataAvailable [this=%x]\n", this));

    nsresult rv = mTransaction->OnDataReadable(inputStream);

    LOG(("mTransaction->OnDataReadable() returned [rv=%x]\n", rv));
    return rv;
}

//-----------------------------------------------------------------------------
// nsHttpConnection::nsIInterfaceRequestor
//-----------------------------------------------------------------------------

// not called on the socket transport thread
NS_IMETHODIMP
nsHttpConnection::GetInterface(const nsIID &iid, void **result)
{
    if (iid.Equals(NS_GET_IID(nsIProgressEventSink)))
        return QueryInterface(iid, result);

    if (mTransaction && mTransaction->Callbacks())
        return mTransaction->Callbacks()->GetInterface(iid, result);

    return NS_ERROR_NO_INTERFACE;
}

//-----------------------------------------------------------------------------
// nsHttpConnection::nsIProgressEventSink
//-----------------------------------------------------------------------------

// called on the socket transport thread
NS_IMETHODIMP
nsHttpConnection::OnStatus(nsIRequest *req, nsISupports *ctx, nsresult status,
                           const PRUnichar *statusText)
{
    if (mProgressSink)
        mProgressSink->OnStatus(nsnull, nsnull, status, statusText);

    return NS_OK;
}

NS_IMETHODIMP
nsHttpConnection::OnProgress(nsIRequest *req, nsISupports *ctx,
                             PRUint32 progress, PRUint32 progressMax)
{
    // we ignore progress notifications from the socket transport.
    // we'll generate these ourselves from OnDataAvailable
    return NS_OK;
}

//-----------------------------------------------------------------------------
// nsHttpConnectionInfo::nsISupports
//-----------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ISUPPORTS0(nsHttpConnectionInfo)
