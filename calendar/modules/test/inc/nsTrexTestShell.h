/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef _nsTrexTestShell_h__
#define _nsTrexTestShell_h__

#include "nsITrexTestShell.h"
#include "nsIFactory.h"
#include "nsRepository.h"
#include "nsIShellInstance.h"
#include "nsIWidget.h"
#include "nsGUIEvent.h"
#include "nsRect.h"
#include "nsApplicationManager.h"
#include "nsIContentViewer.h"
#include "nsCRT.h"
#include "plstr.h"
#include "nsIWidget.h"
#include "nsITextWidget.h"
#include "nsITextAreaWidget.h"
#include "nspr.h"
#include "jsapi.h"
#include "nsIStreamListener.h"

#define TCP_MESG_SIZE                     1024
#define TCP_SERVER_PORT                   666
#define NUM_TCP_CLIENTS                   10
#define NUM_TCP_CONNECTIONS_PER_CLIENT    5
#define NUM_TCP_MESGS_PER_CONNECTION      10
#define SERVER_MAX_BIND_COUNT             100

typedef struct buffer {
    char    data[TCP_MESG_SIZE * 2];
} buffer;

class nsIURL;

class nsTrexTestShell : public nsITrexTestShell,
                        public nsIStreamListener 
{
public:
  nsTrexTestShell();
  ~nsTrexTestShell();

  NS_DECL_ISUPPORTS

  NS_IMETHOD Init();

  // nsIAppShell interfaces
  NS_IMETHOD Create(int* argc, char ** argv) ;
  NS_IMETHOD SetDispatchListener(nsDispatchListener* aDispatchListener) ;
  NS_IMETHOD Exit();
  virtual nsresult Run();
  virtual void* GetNativeData(PRUint32 aDataType) ;

  NS_IMETHOD_(nsEventStatus) HandleEvent(nsGUIEvent *aEvent)  ;
  NS_IMETHOD GetWebViewerContainer(nsIWebViewerContainer ** aWebViewerContainer) ;
  NS_IMETHOD StartCommandServer();

  NS_IMETHOD ReceiveCallback(nsICollectedData& aReply);
  NS_IMETHOD SendCommand(nsString& aCommand);
  NS_IMETHOD ExecuteJS();

private:
  NS_METHOD RegisterFactories();
  NS_IMETHOD SendJS(nsString& aCommand);
  NS_IMETHOD LoadScript(nsString& aScript);
  NS_IMETHOD ReceiveCommand(nsString& aCommand, nsString& aReply);
  NS_METHOD ParseCommandLine();
  NS_IMETHOD LoadUI();
  NS_IMETHOD InitNetwork();

private:
  nsIShellInstance * mShellInstance ;
  nsITextAreaWidget * mDisplay;
  nsITextWidget * mInput;

public:
  NS_IMETHOD    RunThread();
  NS_IMETHOD    ExitThread();

private:
  PRMonitor *   mExitMon;       /* monitor to signal on exit            */
  PRInt32 *     mExitCounter;   /* counter to decrement, before exit        */
  PRInt32       mDatalen;       /* bytes of data transfered in each read/write    */
  PRInt32       mNumThreads;
  PRMonitor *   mClientMon;
  PRNetAddr     mClientAddr;
  nsString      mCommand;
  JSRuntime *   mJSRuntime ;
  JSContext *   mJSContext;
  JSObject *    mJSGlobal;
  JSObject *    mJSZuluObject;
  nsString      mScript;
  nsIURL *      mURL;
  nsIStreamListener * mListener;
  nsString      mJSData;
  PRBool        mQuiet;
  PRBool        mVerbose;
  PRBool        mStdOut;

  // nsIStreamListener interfaces
public:
  NS_IMETHOD GetBindInfo(nsIURL* aURL);
  NS_IMETHOD OnDataAvailable(nsIURL* aURL, nsIInputStream *aIStream, PRInt32 aLength)  ;
  NS_IMETHOD OnStartBinding(nsIURL* aURL, const char *aContentType);
  NS_IMETHOD OnProgress(nsIURL* aURL, PRInt32 aProgress, PRInt32 aProgressMax);
  NS_IMETHOD OnStatus(nsIURL* aURL, const nsString &aMsg);
  NS_IMETHOD OnStopBinding(nsIURL* aURL, PRInt32 aStatus, const nsString &aMsg);


};

#endif
