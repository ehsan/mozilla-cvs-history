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
 * The Original Code is RaptorCanvas.
 *
 * The Initial Developer of the Original Code is Kirk Baker and
 * Ian Wilkinson. Portions created by Kirk Baker and Ian Wilkinson are
 * Copyright (C) 1999 Kirk Baker and Ian Wilkinson. All
 * Rights Reserved.
 *
 * Contributor(s): Kirk Baker <kbaker@eb.com>
 *               Ian Wilkinson <iw@ennoble.com>
 *               Mark Lin <mark.lin@eng.sun.com>
 *               Mark Goddard
 *               Ed Burns <edburns@acm.org>
 *               Ann Sunhachawee
 */

#include "WrapperFactoryImpl.h"

#include "nsIServiceManager.h"  // for NS_InitXPCOM
#include "nsAppShellCIDs.h" // for NS_SESSIONHISTORY_CID
#include "nsCRT.h" // for nsCRT::strcmp

static NS_DEFINE_CID(kSessionHistoryCID, NS_SESSIONHISTORY_CID);

#ifdef XP_PC

// All this stuff is needed to initialize the history

#define APPSHELL_DLL "appshell.dll"
#define BROWSER_DLL  "nsbrowser.dll"
#define EDITOR_DLL "ender.dll"

#else

#ifdef XP_MAC

#define APPSHELL_DLL "APPSHELL_DLL"
#define EDITOR_DLL  "ENDER_DLL"

#else

// XP_UNIX || XP_BEOS
#define APPSHELL_DLL  "libnsappshell"MOZ_DLL_SUFFIX
#define APPCORES_DLL  "libappcores"MOZ_DLL_SUFFIX
#define EDITOR_DLL  "libender"MOZ_DLL_SUFFIX

#endif // XP_MAC

#endif // XP_PC


static nsFileSpec gBinDir; 

const char *gImplementedInterfaces[] = {
        "webclient.WindowControl",
        "webclient.Navigation",
        "webclient.CurrentPage",
        "webclient.History",
        "webclient.EventRegistration",
        "webclient.Bookmarks",
        NULL
        };

extern "C" void NS_SetupRegistry();
extern nsresult NS_AutoregisterComponents();

JNIEXPORT void JNICALL 
Java_org_mozilla_webclient_wrapper_1native_WrapperFactoryImpl_nativeInitialize(
										JNIEnv *env, jobject obj, jstring verifiedBinDirAbsolutePath)
{
  JNIEnv		*	pEnv = env;
  jobject			jobj = obj;
  static PRBool	gFirstTime = PR_TRUE;
  if (gFirstTime) {
    const char *nativePath = (const char *) env->GetStringUTFChars(verifiedBinDirAbsolutePath, 0);
    gBinDir = nativePath;
    
    NS_InitXPCOM(NULL, &gBinDir);
    NS_SetupRegistry();
    nsComponentManager::RegisterComponentLib(kSessionHistoryCID, NULL, 
					     NULL, APPSHELL_DLL, 
					     PR_FALSE, PR_FALSE);
    NS_AutoregisterComponents();
    gFirstTime = PR_FALSE;
    env->ReleaseStringUTFChars(verifiedBinDirAbsolutePath, nativePath);

  }
}

JNIEXPORT void JNICALL 
Java_org_mozilla_webclient_wrapper_1native_WrapperFactoryImpl_nativeTerminate
(JNIEnv *, jobject)
{

}

JNIEXPORT jboolean JNICALL 
Java_org_mozilla_webclient_wrapper_1native_WrapperFactoryImpl_nativeDoesImplement
(JNIEnv *env, jobject obj, jstring interfaceName)
{
    const char *iName = (const char *) env->GetStringUTFChars(interfaceName, 
                                                              0);
    jboolean result = JNI_FALSE;
    
    int i = 0;

    if (NULL == iName) {
        return result;
    }

    while (NULL != gImplementedInterfaces[i]) {
        if (0 == nsCRT::strcmp(gImplementedInterfaces[i++], iName)) {
            result = JNI_TRUE;
            break;
        }
    }
    env->ReleaseStringUTFChars(interfaceName, iName);
    
    return result;
}
