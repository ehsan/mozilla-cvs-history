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
 * The Original Code is Minimo.
 *
 * The Initial Developer of the Original Code is
 * Doug Turner <dougt@meer.net>.
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsSimpleChromeRegistry.h"
#include "nsNetUtil.h"
#include "nsLayoutCID.h"


static NS_DEFINE_CID(kCSSLoaderCID, NS_CSS_LOADER_CID);

NS_IMPL_ISUPPORTS1(nsSimpleChromeRegistry, nsIChromeRegistry)

nsSimpleChromeRegistry::nsSimpleChromeRegistry()
{
  nsCOMPtr<nsISupportsArray> foo;
  GetAgentSheets(nsnull, getter_AddRefs(foo));
}

nsSimpleChromeRegistry::~nsSimpleChromeRegistry()
{
}

NS_IMETHODIMP 
nsSimpleChromeRegistry::Canonify(nsIURI *aChromeURL)
{
    return NS_OK;
}

NS_IMETHODIMP 
nsSimpleChromeRegistry::ConvertChromeURL(nsIURI *aChromeURL, nsACString & _retval)
{
    return aChromeURL->GetSpec(_retval);
}

NS_IMETHODIMP 
nsSimpleChromeRegistry::GetStyleSheets(nsIURI *aChromeURL, nsISupportsArray **_retval)
{
    *_retval = nsnull;
    return NS_OK;
}

NS_IMETHODIMP 
nsSimpleChromeRegistry::GetUserSheets(PRBool useChromeSheets, nsISupportsArray **_retval)
{
    *_retval = nsnull;
    return NS_OK;
}


nsresult 
nsSimpleChromeRegistry::LoadStyleSheet(nsICSSStyleSheet** aSheet, const nsACString& aURL)
{
  *aSheet = nsnull;

  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aURL);
  if (NS_FAILED(rv)) return rv;
  
  if (!mCSSLoader) {
    mCSSLoader = do_CreateInstance(kCSSLoaderCID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  
  if (mCSSLoader)
    rv = mCSSLoader->LoadAgentSheet(uri, aSheet);
    
#ifdef DEBUG
  if (NS_FAILED(rv)) {
    nsXPIDLCString spec;
    uri->GetSpec(spec);
    printf("chrome: failed to load: %s\n", spec.get());   
  }
#endif

  return rv;
}

NS_IMETHODIMP 
nsSimpleChromeRegistry::GetAgentSheets(nsIDocShell *docShell, nsISupportsArray **_retval)
{
    nsresult rv = NS_NewISupportsArray(_retval);
    if (NS_FAILED(rv))
      return rv;

    // Determine the agent sheets that should be loaded.
    if (!mScrollbarSheet)
      LoadStyleSheet(getter_AddRefs(mScrollbarSheet), NS_LITERAL_CSTRING("chrome://global/skin/scrollbars.css"));
    
    if (!mFormSheet)
      LoadStyleSheet(getter_AddRefs(mFormSheet), NS_LITERAL_CSTRING("resource:/res/platform-forms.css"));
    
    
    if (mScrollbarSheet) {
      (*_retval)->AppendElement(mScrollbarSheet);
    }
    if (mFormSheet) {
      (*_retval)->AppendElement(mFormSheet);
    }

    return NS_OK;
}

NS_IMETHODIMP nsSimpleChromeRegistry::CheckForNewChrome()
{
    return NS_OK;
}

