/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */
#include "nsCOMPtr.h"
#include "nsLayoutModule.h"
#include "nsLayoutCID.h"
#include "nsIComponentManager.h"
#include "nsNeckoUtil.h"
#include "nsICSSStyleSheet.h"
#include "nsICSSLoader.h"
#include "nsHTMLAtoms.h"
#include "nsCSSKeywords.h"  // to addref/release table
#include "nsCSSProps.h"     // to addref/release table
#include "nsCSSAtoms.h"     // to addref/release table
#include "nsColorNames.h"   // to addref/release table
#ifdef INCLUDE_XUL
#include "nsXULAtoms.h"
#endif
//MathML Mod - RBS
#ifdef MOZ_MATHML
#include "nsMathMLAtoms.h"
#include "nsMathMLOperators.h"
#endif
#include "nsLayoutAtoms.h"
#include "nsDOMCID.h"
#include "nsIScriptContext.h"
#include "nsINameSpaceManager.h"
#include "nsIScriptNameSetRegistry.h"
#include "nsIScriptNameSpaceManager.h"
#include "nsIScriptExternalNameSet.h"

#include "nsIDocumentEncoder.h"

// XXX
#include "nsIServiceManager.h"

// URL for the "user agent" style sheet
#define UA_CSS_URL "resource:/res/ua.css"

static nsLayoutModule *gModule = NULL;

extern "C" NS_EXPORT nsresult NSGetModule(nsIComponentManager *servMgr,
                                          nsIFileSpec* location,
                                          nsIModule** return_cobj)
{
  nsresult rv = NS_OK;

  NS_ASSERTION(return_cobj, "Null argument");
  NS_ASSERTION(gModule == NULL, "nsLayoutModule: Module already created.");

  // Create an initialize the layout module instance
  nsLayoutModule *m = new nsLayoutModule();
  if (!m) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  // Increase refcnt and store away nsIModule interface to m in return_cobj
  rv = m->QueryInterface(nsIModule::GetIID(), (void**)return_cobj);
  if (NS_FAILED(rv)) {
    delete m;
    m = nsnull;
  }
  gModule = m;                  // WARNING: Weak Reference
  return rv;
}

//----------------------------------------------------------------------


static NS_DEFINE_IID(kIScriptNameSetRegistryIID, NS_ISCRIPTNAMESETREGISTRY_IID);
static NS_DEFINE_IID(kCScriptNameSetRegistryCID, NS_SCRIPT_NAMESET_REGISTRY_CID);
static NS_DEFINE_IID(kIScriptNameSpaceManagerIID, NS_ISCRIPTNAMESPACEMANAGER_IID);
static NS_DEFINE_IID(kIScriptExternalNameSetIID, NS_ISCRIPTEXTERNALNAMESET_IID);

class LayoutScriptNameSet : public nsIScriptExternalNameSet {
public:
  LayoutScriptNameSet();
  virtual ~LayoutScriptNameSet();

  NS_DECL_ISUPPORTS
  
  NS_IMETHOD InitializeClasses(nsIScriptContext* aScriptContext);
  NS_IMETHOD AddNameSet(nsIScriptContext* aScriptContext);
};

LayoutScriptNameSet::LayoutScriptNameSet()
{
  NS_INIT_REFCNT();
}

LayoutScriptNameSet::~LayoutScriptNameSet()
{
}

NS_IMPL_ISUPPORTS(LayoutScriptNameSet, kIScriptExternalNameSetIID);

NS_IMETHODIMP 
LayoutScriptNameSet::InitializeClasses(nsIScriptContext* aScriptContext)
{
  return NS_OK;
}

NS_IMETHODIMP
LayoutScriptNameSet::AddNameSet(nsIScriptContext* aScriptContext)
{
  nsresult result = NS_OK;
  nsIScriptNameSpaceManager* manager;
  static NS_DEFINE_IID(kHTMLImageElementCID, NS_HTMLIMAGEELEMENT_CID);
  static NS_DEFINE_IID(kHTMLOptionElementCID, NS_HTMLOPTIONELEMENT_CID);

  result = aScriptContext->GetNameSpaceManager(&manager);
  if (NS_OK == result) {
    result = manager->RegisterGlobalName("HTMLImageElement",
                                         kHTMLImageElementCID,
                                         PR_TRUE);
    if (NS_FAILED(result)) {
      NS_RELEASE(manager);
      return result;
    }

    result = manager->RegisterGlobalName("HTMLOptionElement",
                                         kHTMLOptionElementCID,
                                         PR_TRUE);
    if (NS_FAILED(result)) {
      NS_RELEASE(manager);
      return result;
    }
        
    NS_RELEASE(manager);
  }
  
  return result;
}

//----------------------------------------------------------------------

static NS_DEFINE_IID(kIModuleIID, NS_IMODULE_IID);

nsICSSStyleSheet* nsLayoutModule::gUAStyleSheet;

nsIScriptNameSetRegistry* nsLayoutModule::gRegistry;

nsLayoutModule::nsLayoutModule()
  : mInitialized(PR_FALSE)
{
  NS_INIT_ISUPPORTS();
#ifdef DEBUG_kipp
  printf("*** Creating layout module %p\n", this);
#endif
}

nsLayoutModule::~nsLayoutModule()
{
  Shutdown();
#ifdef DEBUG_kipp
  printf("*** Destroying layout module %p\n", this);
#endif
}

NS_IMPL_ISUPPORTS(nsLayoutModule, kIModuleIID)

// Perform our one-time intialization for this module
nsresult
nsLayoutModule::Initialize()
{
  if (mInitialized) {
    return NS_OK;
  }

  mInitialized = PR_TRUE;
    
  // Register all of our atoms once
  nsCSSAtoms::AddRefAtoms();
  nsCSSKeywords::AddRefTable();
  nsCSSProps::AddRefTable();
  nsColorNames::AddRefTable();
  nsHTMLAtoms::AddRefAtoms();
  nsLayoutAtoms::AddRefAtoms();
#ifdef INCLUDE_XUL
  nsXULAtoms::AddRefAtoms();
#endif
//MathML Mod - RBS
#ifdef MOZ_MATHML
  nsMathMLOperators::AddRefTable();
  nsMathMLAtoms::AddRefAtoms();
#endif

  // Load the UA style sheet
  nsCOMPtr<nsIURI> uaURL;
  nsresult rv = NS_NewURI(getter_AddRefs(uaURL), UA_CSS_URL);
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsICSSLoader> cssLoader;
    rv = NS_NewCSSLoader(getter_AddRefs(cssLoader));
    if (cssLoader) {
      PRBool complete;
      rv = cssLoader->LoadAgentSheet(uaURL, gUAStyleSheet, complete,
                                     nsnull, nsnull);
    }
  }
  if (NS_FAILED(rv)) {
#ifdef DEBUG
    printf("*** open of %s failed: error=%x\n", UA_CSS_URL, rv);
#endif
    return rv;
  }

  // XXX Initialize the script name set thingy-ma-jigger
  if (!gRegistry) {
    rv = nsServiceManager::GetService(kCScriptNameSetRegistryCID,
                                      kIScriptNameSetRegistryIID,
                                      (nsISupports**) &gRegistry);
    if (NS_SUCCEEDED(rv)) {
      LayoutScriptNameSet* nameSet = new LayoutScriptNameSet();
      gRegistry->AddExternalNameSet(nameSet);
    }
  }

  return rv;
}

// Shutdown this module, releasing all of the module resources
void
nsLayoutModule::Shutdown()
{
  // Release all of our atoms
  nsColorNames::ReleaseTable();
  nsCSSProps::ReleaseTable();
  nsCSSKeywords::ReleaseTable();
  nsCSSAtoms::ReleaseAtoms();
  nsHTMLAtoms::ReleaseAtoms();
  nsLayoutAtoms::ReleaseAtoms();
#ifdef INCLUDE_XUL
  nsXULAtoms::ReleaseAtoms();
#endif
//MathML Mod - RBS
#ifdef MOZ_MATHML
  nsMathMLOperators::ReleaseTable();
  nsMathMLAtoms::ReleaseAtoms();
#endif

  NS_IF_RELEASE(gRegistry);
  NS_IF_RELEASE(gUAStyleSheet);
}

NS_IMETHODIMP
nsLayoutModule::GetClassObject(nsIComponentManager *aCompMgr,
                               const nsCID& aClass,
                               const nsIID& aIID,
                               void** r_classObj)
{
  nsresult rv;

  if (!mInitialized) {
    rv = Initialize();
    if (NS_FAILED(rv)) {
      return rv;
    }
  }

  nsIFactory* f = new nsLayoutFactory(aClass);
  if (!f) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  rv = f->QueryInterface(aIID, r_classObj);
  if (NS_FAILED(rv)) {
    delete f;
  }

  return rv;
}

//----------------------------------------

struct Components {
  const char* mDescription;
  nsID mCID;
  const char* mProgID;
};

// The list of components we register
static Components gComponents[] = {
  { "Namespace manager", NS_NAMESPACEMANAGER_CID, nsnull, },
  { "Event listener manager", NS_EVENTLISTENERMANAGER_CID, nsnull, },
  { "Frame utility", NS_FRAME_UTIL_CID, nsnull, },
  { "Print preview context", NS_PRINT_PREVIEW_CONTEXT_CID, nsnull, },
  { "Layout debugger", NS_LAYOUT_DEBUGGER_CID, nsnull, },

  { "HTML document", NS_HTMLDOCUMENT_CID, nsnull, },
  { "HTML style sheet", NS_HTMLSTYLESHEET_CID, nsnull, },
  { "HTML-CSS style sheet", NS_HTML_CSS_STYLESHEET_CID, nsnull, },

  { "XML document", NS_XMLDOCUMENT_CID, nsnull, },
  { "Image document", NS_IMAGEDOCUMENT_CID, nsnull, },

  { "CSS parser", NS_CSSPARSER_CID, nsnull, },
  { "CSS loader", NS_CSS_LOADER_CID, nsnull, },

  { "HTML element factory", NS_HTML_ELEMENT_FACTORY_CID, nsnull, },
  { "Text element", NS_TEXTNODE_CID, nsnull, },

  { "XML element factory", NS_XML_ELEMENT_FACTORY_CID, nsnull, },

  { "Selection", NS_SELECTION_CID, nsnull, },
  { "Frame selection", NS_FRAMESELECTION_CID, nsnull, },
  { "Range", NS_RANGE_CID, nsnull, },
  { "Content iterator", NS_CONTENTITERATOR_CID, nsnull, },
  { "Subtree iterator", NS_SUBTREEITERATOR_CID, nsnull, },

  // XXX ick
  { "HTML image element", NS_HTMLIMAGEELEMENT_CID, nsnull, },
  { "HTML option element", NS_HTMLOPTIONELEMENT_CID, nsnull, },
  { "Presentation shell", NS_PRESSHELL_CID, nsnull, },
  // XXX end ick

  { "HTML document encoder", NS_TEXT_ENCODER_CID,
    NS_DOC_ENCODER_PROGID_BASE "text/html", },
  { "Plaintext document encoder", NS_TEXT_ENCODER_CID,
    NS_DOC_ENCODER_PROGID_BASE "text/plain", },
  { "XIF document encoder", NS_TEXT_ENCODER_CID,
    NS_DOC_ENCODER_PROGID_BASE "text/xif", },
};
#define NUM_COMPONENTS (sizeof(gComponents) / sizeof(gComponents[0]))

NS_IMETHODIMP
nsLayoutModule::RegisterSelf(nsIComponentManager *aCompMgr,
                             nsIFileSpec* aPath,
                             const char* registryLocation,
                             const char* componentType)
{
  nsresult rv = NS_OK;

#ifdef DEBUG
  printf("*** Registering layout components\n");
#endif

  Components* cp = gComponents;
  Components* end = cp + NUM_COMPONENTS;
  while (cp < end) {
    rv = aCompMgr->RegisterComponentSpec(cp->mCID, cp->mDescription,
                                         cp->mProgID, aPath, PR_TRUE, PR_TRUE);
    if (NS_FAILED(rv)) {
#ifdef DEBUG
      printf("nsLayoutModule: unable to register %s component => %x\n",
             cp->mDescription, rv);
#endif
      break;
    }
    cp++;
  }

  rv = RegisterDocumentFactories(aCompMgr, aPath);

  return rv;
}

NS_IMETHODIMP
nsLayoutModule::UnregisterSelf(nsIComponentManager* aCompMgr,
                               nsIFileSpec* aPath,
                               const char* registryLocation)
{
#ifdef DEBUG
  printf("*** Unregistering layout components\n");
#endif
  Components* cp = gComponents;
  Components* end = cp + NUM_COMPONENTS;
  while (cp < end) {
    nsresult rv = aCompMgr->UnregisterComponentSpec(cp->mCID, aPath);
    if (NS_FAILED(rv)) {
#ifdef DEBUG
      printf("nsLayoutModule: unable to unregister %s component => %x\n",
             cp->mDescription, rv);
#endif
    }
    cp++;
  }

  UnregisterDocumentFactories(aCompMgr, aPath);

  return NS_OK;
}

NS_IMETHODIMP
nsLayoutModule::CanUnload(nsIComponentManager *aCompMgr, PRBool *okToUnload)
{
  if (!okToUnload) {
    return NS_ERROR_INVALID_POINTER;
  }
  *okToUnload = PR_FALSE;
  return NS_ERROR_FAILURE;
}
