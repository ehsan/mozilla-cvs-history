/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "plstr.h"
#include "prmem.h"
#include "nsIServiceManager.h"
#include "nsCookieManager.h"
#include "nsCRT.h"
#include "nsCookies.h"
#include "nsCookie.h"
#include "nsIGenericFactory.h"
#include "nsXPIDLString.h"
#include "nsIScriptGlobalObject.h"

////////////////////////////////////////////////////////////////////////////////

class nsCookieEnumerator : public nsISimpleEnumerator
{
    public:

        NS_DECL_ISUPPORTS

        nsCookieEnumerator() : mCookieCount(0) 
        {
        }

        NS_IMETHOD HasMoreElements(PRBool *result) 
        {
            *result = COOKIE_Count() > mCookieCount;
            return NS_OK;
        }

        NS_IMETHOD GetNext(nsISupports **result) 
        {
          nsCAutoString name;
          nsCAutoString value;
          PRBool isDomain;
          nsCAutoString host;
          nsCAutoString path;
          PRBool isSecure;
          PRUint64 expires;
          nsCookieStatus status;
          nsCookiePolicy policy;
          nsresult rv = COOKIE_Enumerate
            (mCookieCount++, name, value, &isDomain, host, path, &isSecure, &expires,
              &status, &policy);
          if (NS_SUCCEEDED(rv)) {
            nsICookie *cookie =
              new nsCookie(name, value, isDomain, host, path, isSecure, expires,
                           status, policy);
            *result = cookie;
            NS_ADDREF(*result);
          } else {
            *result = nsnull;
          }
          return rv;
        }

        virtual ~nsCookieEnumerator() 
        {
        }

    protected:
        PRInt32 mCookieCount;
};

NS_IMPL_ISUPPORTS1(nsCookieEnumerator, nsISimpleEnumerator);


////////////////////////////////////////////////////////////////////////////////
// nsCookieManager Implementation

NS_IMPL_ISUPPORTS3(nsCookieManager, nsICookieManager, nsICookieManager2, nsISupportsWeakReference);

nsCookieManager::nsCookieManager()
{
}

nsCookieManager::~nsCookieManager(void)
{
}

nsresult nsCookieManager::Init()
{
  COOKIE_Read();
  return NS_OK;
}

NS_IMETHODIMP nsCookieManager::RemoveAll(void) {
  ::COOKIE_RemoveAll();
  ::COOKIE_Write(nsnull);
  return NS_OK;
}

NS_IMETHODIMP nsCookieManager::GetEnumerator(nsISimpleEnumerator * *entries)
{
    *entries = nsnull;

    nsCookieEnumerator* cookieEnum = new nsCookieEnumerator();
    if (cookieEnum == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(cookieEnum);
    *entries = cookieEnum;
    return NS_OK;
}

NS_IMETHODIMP nsCookieManager::Add(const nsACString &aDomain,
                                   const nsACString &aPath,
                                   const nsACString &aName,
                                   const nsACString &aValue,
                                   PRBool aSecure,
                                   PRInt32 aExpires)
{
  // nasty COOKIE method requires caller to hand off ownership of strings.
  /* nulls aren't allowed (cookie code is full of checks as if they were
     but see COOKIE_Write) */
  char *domainCopy = PL_strdup(PromiseFlatCString(aDomain).get()),
       *pathCopy = PL_strdup(PromiseFlatCString(aPath).get()),
       *nameCopy = PL_strdup(PromiseFlatCString(aName).get()),
       *valueCopy = PL_strdup(PromiseFlatCString(aValue).get());

  if (domainCopy && pathCopy && nameCopy && valueCopy)
    return ::COOKIE_AddCookie(domainCopy, pathCopy, nameCopy, valueCopy,
                aSecure, PR_TRUE, aExpires,
                nsICookie::STATUS_UNKNOWN, nsICookie::POLICY_UNKNOWN);

  if (domainCopy) PL_strfree(domainCopy);
  if (pathCopy) PL_strfree(pathCopy);
  if (nameCopy) PL_strfree(nameCopy);
  if (valueCopy) PL_strfree(valueCopy);
  return NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP nsCookieManager::Remove
  (const nsACString& host, const nsACString& name, const nsACString& path, PRBool blocked) {
//  (const nsAUTF8String& host, const nsACString& name, const nsAUTF8String& path, PRBool blocked) {
// using nsACString above instead of nsAUTF8String because the latter doesn't exist yet

  ::COOKIE_Remove(PromiseFlatCString(host).get(),
                  PromiseFlatCString(name).get(),
                  PromiseFlatCString(path).get(), blocked);
  return NS_OK;
}
