/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

/* local header for xpconnect tests components */

#ifndef xpctest_private_h___
#define xpctest_private_h___

#include "nsISupports.h"
#include "nsIFactory.h"
#include "nsIAllocator.h"
#include "nsIXPConnect.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsIGenericFactory.h"
#include "nscore.h"
#include "nsCOMPtr.h"
#include <stdio.h>

#include "xpctest.h"
#include "jsapi.h"

#include "nsITimer.h"
#include "nsITimerCallback.h"

// {ED132C20-EED1-11d2-BAA4-00805F8A5DD7}
#define NS_ECHO_CID \
{ 0xed132c20, 0xeed1, 0x11d2, \
    { 0xba, 0xa4, 0x0, 0x80, 0x5f, 0x8a, 0x5d, 0xd7 } }


// {0ECB3420-0D6F-11d3-BAB8-00805F8A5DD7}
#define NS_CHILD_CID \
{ 0xecb3420, 0xd6f, 0x11d3, \
    { 0xba, 0xb8, 0x0, 0x80, 0x5f, 0x8a, 0x5d, 0xd7 } }


// {FD774840-237B-11d3-9879-006008962422}
#define NS_NOISY_CID \
{ 0xfd774840, 0x237b, 0x11d3, \
    { 0x98, 0x79, 0x0, 0x60, 0x8, 0x96, 0x24, 0x22 } }

// {4DD7EC80-30D9-11d3-9885-006008962422}
#define NS_STRING_TEST_CID \
{ 0x4dd7ec80, 0x30d9, 0x11d3,\
    { 0x98, 0x85, 0x0, 0x60, 0x8, 0x96, 0x24, 0x22 } }

// {DC5FDE90-439D-11d3-988C-006008962422}
#define NS_OVERLOADED_CID \
{ 0xdc5fde90, 0x439d, 0x11d3, \
    { 0x98, 0x8c, 0x0, 0x60, 0x8, 0x96, 0x24, 0x22 } }

#define NS_XPCTESTOBJECTREADONLY_CID \
  {0x1364941e, 0x4462, 0x11d3, \
    { 0x82, 0xee, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTOBJECTREADWRITE_CID \
  {0x3b9b1d38, 0x491a, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTIN_CID \
  {0x318d6f6a, 0x5411, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTOUT_CID \
  {0x4105ae88, 0x5599, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTCONST_CID \
  {0x83f57a56, 0x4f55, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTCALLJS_CID \
  {0x38ba7d98, 0x5a54, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTSCRIPTABLE_CID \
  {0x9ede3336, 0x5fee, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTPARENTONE_CID \
  {0x5408fdcc, 0x60a3, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTPARENTTWO_CID \
  {0x63137392, 0x60a3, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTCHILD2_CID \
  {0x66bed216, 0x60a3, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTCHILD3_CID \
  {0x62353978, 0x614e, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTCHILD4_CID \
  {0xa6d22202, 0x622b, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}

#define NS_XPCTESTCHILD5_CID \
  {0xba3eef4e, 0x6250, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}
#if 0
#define NS_XPCTESTCONSTRUCTWITHARGS_CID \
  {0xbd39c75c, 0x6a05, 0x11d3, \
    { 0x82, 0xef, 0x00, 0x60, 0xb0, 0xeb, 0x59, 0x6f }}
#endif /* 0 */
// {5B9AF380-6569-11d3-989E-006008962422}
#define NS_ARRAY_CID \
{ 0x5b9af380, 0x6569, 0x11d3, \
    { 0x98, 0x9e, 0x0, 0x60, 0x8, 0x96, 0x24, 0x22 } };

// 'namespace' class
class xpctest
{
public:
  static const nsID& GetEchoCID() {static nsID cid = NS_ECHO_CID; return cid;}
  static NS_METHOD ConstructEcho(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetChildCID() {static nsID cid = NS_CHILD_CID; return cid;}
  static NS_METHOD ConstructChild(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetNoisyCID() {static nsID cid = NS_NOISY_CID; return cid;}
  static NS_METHOD ConstructNoisy(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetStringTestCID() {static nsID cid = NS_STRING_TEST_CID; return cid;}
  static NS_METHOD ConstructStringTest(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetOverloadedCID() {static nsID cid = NS_OVERLOADED_CID; return cid;}
  static NS_METHOD ConstructOverloaded(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestObjectReadOnlyCID() {static nsID cid = NS_XPCTESTOBJECTREADONLY_CID; return cid;}
  static NS_METHOD ConstructXPCTestObjectReadOnly(nsISupports *aOuter, REFNSIID aIID, void **aResult);
  
  static const nsID& GetXPCTestObjectReadWriteCID() {static nsID cid = NS_XPCTESTOBJECTREADWRITE_CID; return cid;}
  static NS_METHOD ConstructXPCTestObjectReadWrite(nsISupports *aOuter, REFNSIID aIID, void **aResult);
  
  static const nsID& GetXPCTestInCID() { static nsID cid = NS_XPCTESTIN_CID; return cid; }
  static NS_METHOD ConstructXPCTestIn(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestOutCID() { static nsID cid = NS_XPCTESTOUT_CID; return cid; }
  static NS_METHOD ConstructXPCTestOut(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestInOutCID() { static nsID cid = NS_XPCTESTOUT_CID; return cid; }
  static NS_METHOD ConstructXPCTestInOut(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestConstCID() { static nsID cid = NS_XPCTESTCONST_CID; return cid; }
  static NS_METHOD ConstructXPCTestConst(nsISupports *aOuter, REFNSIID aIID, void **aResult);
#if 0
  static const nsID& GetXPCTestConstructWithArgsCID() { static nsID cid = NS_XPCTESTCONSTRUCTWITHARGS_CID; return cid; }
  static NS_METHOD ConstructXPCTestConstructWithArgs(nsISupports *aOuter, REFNSIID aIID, void **aResult);
  static const nsID& GetXPCTestScriptableCID() { static nsID cid = NS_XPCTESTSCRIPTABLE_CID; return cid; }
  static NS_METHOD ConstructXPCTestScriptable(nsISupports *aOuter, REFNSIID aIID, void **aResult);
#endif /* 0 */

  static const nsID& GetXPCTestCallJSCID() { static nsID cid = NS_XPCTESTCALLJS_CID; return cid; }
  static NS_METHOD ConstructXPCTestCallJS(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestParentOneCID() { static nsID cid = NS_XPCTESTPARENTONE_CID; return cid; }
  static NS_METHOD ConstructXPCTestParentOne(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestParentTwoCID() { static nsID cid = NS_XPCTESTPARENTTWO_CID; return cid; }
  static NS_METHOD ConstructXPCTestParentTwo(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestChild2CID() { static nsID cid = NS_XPCTESTCHILD2_CID; return cid; }
  static NS_METHOD ConstructXPCTestChild2(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestChild3CID() { static nsID cid = NS_XPCTESTCHILD3_CID; return cid; }
  static NS_METHOD ConstructXPCTestChild3(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestChild4CID() { static nsID cid = NS_XPCTESTCHILD4_CID; return cid; }
  static NS_METHOD ConstructXPCTestChild4(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetXPCTestChild5CID() { static nsID cid = NS_XPCTESTCHILD5_CID; return cid; }
  static NS_METHOD ConstructXPCTestChild5(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static const nsID& GetArrayTestCID() {static nsID cid = NS_ARRAY_CID; return cid;}
  static NS_METHOD ConstructArrayTest(nsISupports *aOuter, REFNSIID aIID, void **aResult);

private:
    xpctest();  // not implemented
};

#endif /* xpctest_private_h___ */
