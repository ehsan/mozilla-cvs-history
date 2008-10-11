/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun Microsystems,
 * Inc. Portions created by Sun are
 * Copyright (C) 1999 Sun Microsystems, Inc. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#include "XPCOMShutdownObserver.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsIPref.h"
#include "nsIFile.h"
#include "nsFileSpec.h"

#define kPluginRegistryFilename NS_LITERAL_CSTRING("pluginreg.dat")

XPCOMShutdownObserver::XPCOMShutdownObserver() {
    NS_INIT_ISUPPORTS();

}

XPCOMShutdownObserver::~XPCOMShutdownObserver(void) {
}

NS_IMPL_ISUPPORTS1(XPCOMShutdownObserver, nsIObserver);
NS_IMETHODIMP
XPCOMShutdownObserver::Observe(nsISupports *aSubject, const char *aTopic, 
                       const PRUnichar *aData)
{
    nsresult rv;
    nsCOMPtr<nsIPref> prefs(do_GetService(NS_PREF_CONTRACTID, &rv));
    if (NS_SUCCEEDED(rv)) {
        rv = prefs->SetBoolPref("plugin.allow_alien_star_handler", PR_TRUE);
        if (NS_SUCCEEDED(rv)) {
            prefs->SetBoolPref("plugin.default_plugin_disabled", PR_FALSE);
            if (NS_SUCCEEDED(rv)) {
                prefs->SetBoolPref("plugin.override_internal_types", PR_TRUE);
            }
        }
        
        nsCOMPtr<nsIProperties> dirService
            (do_GetService(NS_DIRECTORY_SERVICE_CONTRACTID, &rv));
        nsCOMPtr<nsIFile> file;
        if (NS_SUCCEEDED(rv)) {
            rv = dirService->Get(NS_APP_USER_PROFILE_50_DIR,
                                 NS_GET_IID(nsIFile),
                                 getter_AddRefs(file));
            if (NS_SUCCEEDED(rv)) {
                rv = file->AppendNative(NS_LITERAL_CSTRING("prefs.js"));
                if (NS_SUCCEEDED(rv)) {
                    rv = prefs->SavePrefFile(file);
                }
            }
            
            // Delete the pluginreg.dat
            rv = dirService->Get(NS_APP_APPLICATION_REGISTRY_DIR, 
                                 NS_GET_IID(nsIFile),
                                 getter_AddRefs(file));
            if (NS_SUCCEEDED(rv)) {
                rv = file->AppendNative(kPluginRegistryFilename);
                if (NS_SUCCEEDED(rv)) {
                    rv = file->Remove(PR_FALSE);
                }
            }
        }
    }
    return rv;

}

