/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef nsIApplicationShell_h___
#define nsIApplicationShell_h___

#include "nscore.h"
#include "nsshell.h"
#include "nsISupports.h"
#include "nsIShellInstance.h"

#define NS_IAPPLICATIONSHELL_IID      \
 { 0xaf9a93e0, 0xdebc, 0x11d1, \
   {0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

#define NS_IAPPLICATIONSHELL_CID      \
 { 0x2293d960, 0xdeff, 0x11d1, \
   {0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }


extern "C" nsresult NS_RegisterApplicationShellFactory();

// Application Shell Interface
class nsIApplicationShell : public nsISupports 
{
public:

  /**
   * Initialize the ApplicationShell
   * @result The result of the initialization, NS_OK if no errors
   */
  NS_IMETHOD Init() = 0;

  /**
   * Start the Shell's Event Loop.  
   * @result The result of the event loop execution, NS_Ok if appropriate Exit Message occured
   */
  NS_IMETHOD Run() = 0;


};

#endif /* nsIApplicationShell_h___ */
