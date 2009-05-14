/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License Version
* 1.1 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is Camino code.
*
* The Initial Developer of the Original Code is
* Stuart Morgan
* Portions created by the Initial Developer are Copyright (C) 2009
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*   Stuart Morgan <stuart.morgan@alumni.case.edu>
*
* Alternatively, the contents of this file may be used under the terms of
* either the GNU General Public License Version 2 or later (the "GPL"), or
* the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
* in which case the provisions of the GPL or the LGPL are applicable instead
* of those above. If you wish to allow use of your version of this file only
* under the terms of either the GPL or the LGPL, and not to allow others to
* use your version of this file under the terms of the MPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the MPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */

#import "CHCertificateOverrideManager.h"

#import "NSString+Gecko.h"

#include "nsCOMPtr.h"
#include "nsIX509Cert.h"
#include "nsICertOverrideService.h"
#include "nsICertTree.h"
#include "nsServiceManagerUtils.h"
#include "nsString.h"

const int CHCertificateOverrideFlagUntrusted =
  nsICertOverrideService::ERROR_UNTRUSTED;
const int CHCertificateOverrideFlagDomainMismatch =
  nsICertOverrideService::ERROR_MISMATCH;
const int CHCertificateOverrideFlagInvalidTime =
  nsICertOverrideService::ERROR_TIME;

@implementation CHCertificateOverrideManager

+ (CHCertificateOverrideManager*)certificateOverrideManager
{
  return [[[self alloc] init] autorelease];
}

- (NSArray*)overrideHosts
{
  NSMutableArray* overrides = [NSMutableArray array];
  // There is no way to get a list of all the overrides via the APIs,
  // other than to go via nsICertTree; see bug 467317.
  nsCOMPtr<nsICertTree> certTree =
    do_CreateInstance("@mozilla.org/security/nsCertTree;1");
  if (!certTree)
    return overrides;
  nsresult rv = certTree->LoadCerts(nsIX509Cert::SERVER_CERT);
  if (NS_FAILED(rv))
    return overrides;

  PRInt32 rowCount = 0;
  certTree->GetRowCount(&rowCount);
  for (PRInt32 i = 0; i < rowCount; ++i) {
    PRBool isOverride = PR_FALSE;
    certTree->IsHostPortOverride(i, &isOverride);
    if (!isOverride)
      continue;
    nsCOMPtr<nsICertTreeItem> treeItem;
    certTree->GetTreeItem(i, getter_AddRefs(treeItem));
    if (!treeItem)
      continue;

    nsAutoString hostPort;
    treeItem->GetHostPort(hostPort);
    [overrides addObject:[NSString stringWith_nsAString:hostPort]];
  }
  return overrides;
}

- (BOOL)addOverrideForHost:(NSString*)host
                      port:(int)port
                  withCert:(nsIX509Cert*)cert
           validationFlags:(int)validationFlags
{
  nsCOMPtr<nsICertOverrideService> overrideService =
    do_GetService(NS_CERTOVERRIDE_CONTRACTID);
  if (!overrideService)
    return NO;

  overrideService->RememberValidityOverride(
      nsDependentCString([host UTF8String]), port,
      cert, validationFlags, PR_FALSE);
  return YES;
}

- (BOOL)removeOverrideForHost:(NSString*)host port:(int)port
{
  nsCOMPtr<nsICertOverrideService> overrideService =
    do_GetService(NS_CERTOVERRIDE_CONTRACTID);
  if (!overrideService)
    return NO;

  overrideService->ClearValidityOverride(nsDependentCString([host UTF8String]),
                                         port);
  return YES;
}

@end
