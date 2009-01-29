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
 * Sean Murphy.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sean Murphy <murph@seanmurph.com>
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

#import <Cocoa/Cocoa.h>
#include "SafeBrowsingAboutModule.h"

#include "nsCOMPtr.h"
#include "nsIChannel.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsNetCID.h"
#include "nsString.h"
#include "nsIScriptSecurityManager.h"
#include "nsIPrincipal.h"

#define BLOCKED_PAGE_CHROME_URL "chrome://global/locale/safebrowsing/blockedSite.xhtml"

NS_IMPL_ISUPPORTS1(CHSafeBrowsingAboutModule, nsIAboutModule)

NS_IMETHODIMP
CHSafeBrowsingAboutModule::NewChannel(nsIURI *aURI, nsIChannel **result)
{
  NS_ENSURE_ARG_POINTER(aURI);
  NS_ASSERTION(result, "must not be null");

  nsresult rv;

  nsCOMPtr<nsIIOService> ioService = do_GetService(NS_IOSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCAutoString blockedSitePageURL;
  blockedSitePageURL.AssignLiteral(BLOCKED_PAGE_CHROME_URL);

  nsCOMPtr<nsIChannel> blockedSiteChannel;
  rv = ioService->NewChannel(blockedSitePageURL, "UTF8", aURI, getter_AddRefs(blockedSiteChannel));
  if (NS_FAILED(rv))
    return rv;

  blockedSiteChannel->SetOriginalURI(aURI);

  nsCOMPtr<nsIScriptSecurityManager> securityManager = 
    do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIPrincipal> principal;
  rv = securityManager->GetCodebasePrincipal(aURI, getter_AddRefs(principal));
  if (NS_FAILED(rv))
    return rv;

  rv = blockedSiteChannel->SetOwner(principal);
  if (NS_FAILED(rv))
    return rv;

  *result = blockedSiteChannel;
  NS_ADDREF(*result);
  return NS_OK;
}

NS_IMETHODIMP
CHSafeBrowsingAboutModule::GetURIFlags(nsIURI *aURI, PRUint32 *result)
{
  // Since bad sites can cause this page to appear (e.g. by having an iframe pointing to 
  // another blacklisted site), we should allow URI_SAFE_FOR_UNTRUSTED_CONTENT.

  *result = nsIAboutModule::ALLOW_SCRIPT | nsIAboutModule::URI_SAFE_FOR_UNTRUSTED_CONTENT;
  return NS_OK;
}

NS_METHOD
CHSafeBrowsingAboutModule::CreateSafeBrowsingAboutModule(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  CHSafeBrowsingAboutModule *aboutModule = new CHSafeBrowsingAboutModule();
  if (aboutModule == nsnull)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(aboutModule);
  nsresult rv = aboutModule->QueryInterface(aIID, aResult);
  NS_RELEASE(aboutModule);
  return rv;
}
