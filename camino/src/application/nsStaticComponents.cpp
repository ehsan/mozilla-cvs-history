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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 2001 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * 	Christopher Seawood <cls@seawood.org>
 *  Chris Waterson <waterson@netscape.com>
 */

#line 26 "nsStaticComponents.cpp.in"
#define XPCOM_TRANSLATE_NSGM_ENTRY_POINT 1

#include "nsIGenericFactory.h"
#include "nsStaticComponent.h"

/**
 * Construct a unique NSGetModule entry point for a generic module.
 */
#define NSGETMODULE(_name) _name##_NSGetmodule

/**
 * Declare an NSGetModule() routine for a generic module.
 */
#define DECL_NSGETMODULE(_name)                                \
extern nsModuleInfo NSMODULEINFO(_name);                       \
extern "C" NS_EXPORT nsresult                                  \
NSGETMODULE(_name) (nsIComponentManager* aCompMgr,             \
                    nsIFile*             aLocation,            \
                    nsIModule**          aResult)              \
{                                                              \
    return NS_NewGenericModule2(&NSMODULEINFO(_name), aResult);\
}

// NSGetModule entry points
DECL_NSGETMODULE(UcharUtil) DECL_NSGETMODULE(nsUConvModule) DECL_NSGETMODULE(nsUCvJAModule) DECL_NSGETMODULE(nsUCvCnModule) DECL_NSGETMODULE(nsUCvLatinModule) DECL_NSGETMODULE(nsUCvTWModule) DECL_NSGETMODULE(nsUCvTW2Module) DECL_NSGETMODULE(nsUCvKoModule) DECL_NSGETMODULE(nsLocaleModule) DECL_NSGETMODULE(nsStringBundleModule) DECL_NSGETMODULE(nsLWBrkModule) DECL_NSGETMODULE(nsCharDetModule) DECL_NSGETMODULE(xpconnect) DECL_NSGETMODULE(cacheservice) DECL_NSGETMODULE(necko_core_and_primary_protocols) DECL_NSGETMODULE(necko_secondary_protocols) DECL_NSGETMODULE(nsURILoaderModule) DECL_NSGETMODULE(nsPrefModule) DECL_NSGETMODULE(nsCJVMManagerModule) DECL_NSGETMODULE(nsSecurityManagerModule) DECL_NSGETMODULE(nsChromeModule) DECL_NSGETMODULE(nsRDFModule) DECL_NSGETMODULE(nsParserModule) DECL_NSGETMODULE(nsGfxMacModule) DECL_NSGETMODULE(nsGfx2Module) DECL_NSGETMODULE(nsImageLib2Module) DECL_NSGETMODULE(nsPNGDecoderModule) DECL_NSGETMODULE(nsGIFModule2) DECL_NSGETMODULE(nsJPEGDecoderModule) DECL_NSGETMODULE(nsPluginModule) DECL_NSGETMODULE(javascript__protocol) DECL_NSGETMODULE(DOM_components) DECL_NSGETMODULE(nsViewModule) DECL_NSGETMODULE(nsWidgetMacModule) DECL_NSGETMODULE(nsContentModule) DECL_NSGETMODULE(nsLayoutModule) DECL_NSGETMODULE(nsMorkModule) DECL_NSGETMODULE(docshell_provider) DECL_NSGETMODULE(embedcomponents) DECL_NSGETMODULE(Browser_Embedding_Module) DECL_NSGETMODULE(nsEditorModule) DECL_NSGETMODULE(nsTransactionManagerModule) DECL_NSGETMODULE(nsTextServicesModule) DECL_NSGETMODULE(nsProfileModule) DECL_NSGETMODULE(Session_History_Module) DECL_NSGETMODULE(application) DECL_NSGETMODULE(nsCookieModule) DECL_NSGETMODULE(nsXMLExtrasModule) DECL_NSGETMODULE(BOOT) DECL_NSGETMODULE(NSS)
#line 52 "nsStaticComponents.cpp.in"

/**
 * The nsStaticModuleInfo
 */
static nsStaticModuleInfo gStaticModuleInfo[] = {
#define MODULE(_name) { (#_name), NSGETMODULE(_name) }
	MODULE(UcharUtil), MODULE(nsUConvModule), MODULE(nsUCvJAModule), MODULE(nsUCvCnModule), MODULE(nsUCvLatinModule), MODULE(nsUCvTWModule), MODULE(nsUCvTW2Module), MODULE(nsUCvKoModule), MODULE(nsLocaleModule), MODULE(nsStringBundleModule), MODULE(nsLWBrkModule), MODULE(nsCharDetModule), MODULE(xpconnect), MODULE(cacheservice), MODULE(necko_core_and_primary_protocols), MODULE(necko_secondary_protocols), MODULE(nsURILoaderModule), MODULE(nsPrefModule), MODULE(nsCJVMManagerModule), MODULE(nsSecurityManagerModule), MODULE(nsChromeModule), MODULE(nsRDFModule), MODULE(nsParserModule), MODULE(nsGfxMacModule), MODULE(nsGfx2Module), MODULE(nsImageLib2Module), MODULE(nsPNGDecoderModule), MODULE(nsGIFModule2), MODULE(nsJPEGDecoderModule), MODULE(nsPluginModule), MODULE(javascript__protocol), MODULE(DOM_components), MODULE(nsViewModule), MODULE(nsWidgetMacModule), MODULE(nsContentModule), MODULE(nsLayoutModule), MODULE(nsMorkModule), MODULE(docshell_provider), MODULE(embedcomponents), MODULE(Browser_Embedding_Module), MODULE(nsEditorModule), MODULE(nsTransactionManagerModule), MODULE(nsTextServicesModule), MODULE(nsProfileModule), MODULE(Session_History_Module), MODULE(application), MODULE(nsCookieModule), MODULE(nsXMLExtrasModule), MODULE(BOOT), MODULE(NSS),
#line 60 "nsStaticComponents.cpp.in"
};

/**
 * Our NSGetStaticModuleInfoFunc
 */
nsresult
app_getModuleInfo(nsStaticModuleInfo **info, PRUint32 *count)
{
  *info = gStaticModuleInfo;
  *count = sizeof(gStaticModuleInfo) / sizeof(gStaticModuleInfo[0]);
  return NS_OK;
}

