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
 * The Original Code is the mozilla.org LDAP XPCOM SDK.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s): Dan Mosedale <dmose@mozilla.org>
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

#include "nsError.h"
#include "nspr.h"

#define NS_ERROR_LDAP_OPERATIONS_ERROR \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_LDAP, LDAP_OPERATIONS_ERROR)

#define NS_ERROR_LDAP_ENCODING_ERROR \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_LDAP, LDAP_ENCODING_ERROR)

#define NS_ERROR_LDAP_SERVER_DOWN \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_LDAP, LDAP_SERVER_DOWN)

#define NS_ERROR_LDAP_NOT_SUPPORTED \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_LDAP, LDAP_NOT_SUPPORTED)

#define NS_ERROR_LDAP_DECODING_ERROR \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_LDAP, LDAP_DECODING_ERROR)

#ifdef PR_LOGGING
extern PRLogModuleInfo *gLDAPLogModule;	   // defn in nsLDAPProtocolModule.cpp
#endif
