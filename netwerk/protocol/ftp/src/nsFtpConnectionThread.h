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
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#ifndef __nsftpconnectionthread__h_
#define __nsftpconnectionthread__h_

#include "nsIThread.h"
#include "nsIRunnable.h"
#include "nsISocketTransportService.h"
#include "nsIServiceManager.h"
#include "nsIStreamListener.h"
#include "nsIURI.h"
#include "prtime.h"
#include "prmon.h"
#include "nsString2.h"
#include "nsIEventQueue.h"
#include "nsPIFTPChannel.h"
#include "nsIConnectionCache.h"
#include "nsConnectionCacheObj.h"
#include "nsIProtocolHandler.h"
#include "nsCOMPtr.h"
#include "nsXPIDLString.h"
#include "nsIBufferInputStream.h"
#include "nsIBufferOutputStream.h"
#include "nsAutoLock.h"

// ftp server types
#define FTP_GENERIC_TYPE     0
#define FTP_UNIX_TYPE        1
#define FTP_DCTS_TYPE        2
#define FTP_NCSA_TYPE        3
#define FTP_PETER_LEWIS_TYPE 4
#define FTP_MACHTEN_TYPE     5
#define FTP_CMS_TYPE         6
#define FTP_TCPC_TYPE        7
#define FTP_VMS_TYPE         8
#define FTP_NT_TYPE          9
#define FTP_WEBSTAR_TYPE     10

// ftp states
typedef enum _FTP_STATE {
///////////////////////
//// Internal states
    FTP_READ_BUF,
    FTP_ERROR,
    FTP_COMPLETE,

///////////////////////
//// Command channel connection setup states
    FTP_S_USER, FTP_R_USER,
    FTP_S_PASS, FTP_R_PASS,
    FTP_S_SYST, FTP_R_SYST,
    FTP_S_ACCT, FTP_R_ACCT,
    FTP_S_MACB, FTP_R_MACB,
    FTP_S_PWD , FTP_R_PWD ,
    FTP_S_DEL_FILE, FTP_R_DEL_FILE,
    FTP_S_DEL_DIR , FTP_R_DEL_DIR ,
    FTP_S_MKDIR, FTP_R_MKDIR,
    FTP_S_MODE, FTP_R_MODE,
    FTP_S_CWD,  FTP_R_CWD,
    FTP_S_SIZE, FTP_R_SIZE,
    FTP_S_PUT,  FTP_R_PUT,
    FTP_S_RETR, FTP_R_RETR,
    FTP_S_MDTM, FTP_R_MDTM,
    FTP_S_LIST, FTP_R_LIST,
    FTP_S_TYPE, FTP_R_TYPE,

///////////////////////
//// Data channel connection setup states
    FTP_S_PASV, FTP_R_PASV
} FTP_STATE;

// higher level ftp actions
typedef enum _FTP_ACTION { GET, PUT, MKDIR, DEL} FTP_ACTION;

class nsFtpConnectionThread : public nsIRunnable,
                              public nsIRequest,
                              public nsIStreamObserver {
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIRUNNABLE
    NS_DECL_NSIREQUEST
    NS_DECL_NSISTREAMOBSERVER

    nsFtpConnectionThread();
    virtual ~nsFtpConnectionThread();

    nsresult Init(nsIProtocolHandler    *aHandler,
                  nsIChannel            *aChannel,
                  PRUint32              bufferSegmentSize,
                  PRUint32              bufferMaxSize);

    // use this to set an observer. (as in the asyncopen case)
    nsresult SetStreamObserver(nsIStreamObserver *aObserver, nsISupports *aContext);
    
    // use this to set a listener to receive data related On*() notifications
    nsresult SetStreamListener(nsIStreamListener *aListener, nsISupports *aContext=nsnull);

    // user level setup
    nsresult SetAction(FTP_ACTION aAction);

private:
    ///////////////////////////////////
    // BEGIN: STATE METHODS
    nsresult        S_user(); FTP_STATE       R_user();
    nsresult        S_pass(); FTP_STATE       R_pass();
    nsresult        S_syst(); FTP_STATE       R_syst();
    nsresult        S_acct(); FTP_STATE       R_acct();

    nsresult        S_macb(); FTP_STATE       R_macb();
    nsresult        S_pwd();  FTP_STATE       R_pwd();
    nsresult        S_mode(); FTP_STATE       R_mode();
    nsresult        S_cwd();  FTP_STATE       R_cwd();

    nsresult        S_size(); FTP_STATE       R_size();
    nsresult        S_mdtm(); FTP_STATE       R_mdtm();
    nsresult        S_list(); FTP_STATE       R_list();

    nsresult        S_retr(); FTP_STATE       R_retr();
    nsresult        S_pasv();     FTP_STATE   R_pasv();
    nsresult        S_del_file(); FTP_STATE   R_del_file();
    nsresult        S_del_dir();  FTP_STATE   R_del_dir();

    nsresult        S_mkdir();    FTP_STATE   R_mkdir();
    // END: STATE METHODS
    ///////////////////////////////////

    // internal methods
    nsresult    StopProcessing();
    void        SetSystInternals(void);
    FTP_STATE   FindActionState(void);
    FTP_STATE   FindGetState(void);
    nsresult    MapResultCodeToString(nsresult aResultCode, PRUnichar* *aOutMsg);
    void        SetDirMIMEType(nsString& aString);
    nsresult    DigestServerGreeting();
    nsresult    Process();

    ///////////////////////////////////
    // Private members

        // ****** state machine vars
    FTP_STATE           mState;             // the current state
    FTP_STATE           mNextState;         // the next state
    PRBool              mKeepRunning;       // thread event loop boolean
    PRInt32             mResponseCode;      // the last command response code
    nsCAutoString       mResponseMsg;       // the last command response text
    nsCOMPtr<nsIEventQueue> mFTPEventQueue; // the eventq for this thread

        // ****** channel/transport/stream vars 
    nsCOMPtr<nsISocketTransportService> mSTS;       // the socket transport service
    nsCOMPtr<nsIChannel>         mCPipe;            // the command channel transport
    nsCOMPtr<nsIChannel>         mDPipe;            // the data channel transport
    nsCOMPtr<nsIOutputStream>    mCOutStream;       // command channel output
    nsCOMPtr<nsIInputStream>     mCInStream;        // command channel input

        // ****** consumer vars
    nsCOMPtr<nsIStreamListener>     mListener;        // the consumer of our read events
    nsCOMPtr<nsISupports>           mListenerContext; // the context we pass through our read events
    nsCOMPtr<nsIStreamObserver>     mObserver;        // the consumer of our open events
    nsCOMPtr<nsISupports>           mObserverContext; // the context we pass through our open events
    nsCOMPtr<nsIChannel>            mChannel;         // our owning FTP channel we pass through our events

        // ****** connection cache vars
    PRInt32             mServerType;    // What kind of server are we talking to
    PRBool              mPasv;          // Should we use PASV for data channel
    PRBool              mList;          // Use LIST instead of NLST
    nsCAutoString       mCwd;           // Our current working dir.
    nsCAutoString       mCwdAttempt;    // The dir we're trying to get into.
    nsCAutoString       mCacheKey;      // the key into the cache hash.
    PRBool              mCachedConn;    // is this connection from the cache
    nsCOMPtr<nsIConnectionCache> mConnCache;// the nsISupports proxy ptr to our connection cache
    nsConnectionCacheObj         *mConn;    // The cached connection.


        // ****** protocol interpretation related state vars
    nsAutoString        mUsername;      // username
    nsAutoString        mPassword;      // password
    FTP_ACTION          mAction;        // the higher level action (GET/PUT)
    PRBool              mUsePasv;       // use a passive data connection.
    PRBool              mBin;           // transfer mode (ascii or binary)
    PRBool              mResetMode;     // have we reset the mode to ascii
    PRBool              mAnonymous;     // try connecting anonymous (default)
    PRBool              mRetryPass;     // retrying the password
    nsresult            mInternalError; // represents internal state errors

        // ****** URI vars
    nsCOMPtr<nsIURI>       mURL;        // the uri we're connecting to
    nsXPIDLCString         mURLSpec;    // raw spec of the url
    PRInt32                mPort;       // the port to connect to
    nsAutoString           mFilename;   // url filename (if any)
    PRInt32                mLength;     // length of the file
    PRTime                 mLastModified;// last modified time for file

        // ****** other vars
    PRBool                 mConnected;  // are we connected.
    PRBool                 mSentStart;  // have we sent an OnStartRequest() notification
    PRUint8                mSuspendCount;// number of times we've been suspended.
    nsCOMPtr<nsPIFTPChannel> mFTPChannel;// used to synchronize w/ our owning channel.
    PRUint32               mBufferSegmentSize;
    PRUint32               mBufferMaxSize;
    PRLock                 *mLock;
    PRMonitor              *mMonitor;
    nsCOMPtr<nsIEventQueue>mUIEventQ;
    PLEvent                *mAsyncReadEvent;
};

#define NS_FTP_BUFFER_READ_SIZE             (8*1024)
#define NS_FTP_BUFFER_WRITE_SIZE            (8*1024)

#endif //__nsftpconnectionthread__h_
