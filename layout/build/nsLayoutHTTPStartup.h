/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Alec Flett <alecf@netscape.com>
 */

#include "nsIObserver.h"

#define NS_LAYOUTHTTPSTARTUP_CONTRACTID \
  "@mozilla.org/layout/http-startup;1"

/* c2f6ef7e-afd5-4be4-a1f5-c824efa4231b */
#define NS_LAYOUTHTTPSTARTUP_CID \
{ 0xc2f6ef7e, 0xafd5, 0x4be4, \
    {0xa1, 0xf5, 0xc8, 0x24, 0xef, 0xa4, 0x23, 0x1b} }

class nsLayoutHTTPStartup : public nsIObserver
{
public:
    nsLayoutHTTPStartup() {}
    virtual ~nsLayoutHTTPStartup() {}
  
    NS_DECL_ISUPPORTS
    NS_DECL_NSIOBSERVER

public:
    static nsresult RegisterHTTPStartup();
    static nsresult UnregisterHTTPStartup();
  
private:
    nsresult setUserAgent();
  
};
