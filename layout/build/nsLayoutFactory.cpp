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
#include "nslayout.h"
#include "nsLayoutModule.h"
#include "nsIFactory.h"
#include "nsISupports.h"
#include "nsLayoutCID.h"
#include "nsIDocument.h"
#include "nsIHTMLContent.h"
#include "nsITextContent.h"
#include "nsIPresShell.h"
#include "nsIPresContext.h"
#include "nsIDOMSelection.h"
#include "nsIFrameUtil.h"

#include "nsHTMLAtoms.h"
#include "nsHTMLParts.h"
#include "nsDOMCID.h"
#include "nsIServiceManager.h"
#include "nsICSSParser.h"
#include "nsIHTMLStyleSheet.h"
#include "nsIHTMLCSSStyleSheet.h"
#include "nsICSSLoader.h"
#include "nsIDOMRange.h"
#include "nsIContentIterator.h"
#include "nsINameSpaceManager.h"
#include "nsIScriptNameSetRegistry.h"
#include "nsIScriptNameSpaceManager.h"
#include "nsIScriptExternalNameSet.h"
#include "nsIEventListenerManager.h"
#include "nsILayoutDebugger.h"
#include "nsIHTMLElementFactory.h"
#include "nsIXMLElementFactory.h"
#include "nsIDocumentEncoder.h"
#include "nsCOMPtr.h"
#include "nsIFrameSelection.h"

class nsIDocumentLoaderFactory;

static NS_DEFINE_CID(kComponentManagerCID, NS_COMPONENTMANAGER_CID);

static NS_DEFINE_IID(kHTMLDocumentCID, NS_HTMLDOCUMENT_CID);
static NS_DEFINE_IID(kXMLDocumentCID, NS_XMLDOCUMENT_CID);
static NS_DEFINE_CID(kXMLElementFactoryCID, NS_XML_ELEMENT_FACTORY_CID);
static NS_DEFINE_IID(kImageDocumentCID, NS_IMAGEDOCUMENT_CID);
static NS_DEFINE_IID(kCSSParserCID,     NS_CSSPARSER_CID);
static NS_DEFINE_CID(kHTMLStyleSheetCID, NS_HTMLSTYLESHEET_CID);
static NS_DEFINE_CID(kHTMLCSSStyleSheetCID, NS_HTML_CSS_STYLESHEET_CID);
static NS_DEFINE_CID(kCSSLoaderCID, NS_CSS_LOADER_CID);
static NS_DEFINE_IID(kHTMLImageElementCID, NS_HTMLIMAGEELEMENT_CID);
static NS_DEFINE_IID(kHTMLOptionElementCID, NS_HTMLOPTIONELEMENT_CID);

static NS_DEFINE_CID(kSelectionCID, NS_SELECTION_CID);
static NS_DEFINE_IID(kFrameSelectionCID, NS_FRAMESELECTION_CID);
static NS_DEFINE_IID(kRangeCID,     NS_RANGE_CID);
static NS_DEFINE_IID(kContentIteratorCID, NS_CONTENTITERATOR_CID);
static NS_DEFINE_IID(kSubtreeIteratorCID, NS_SUBTREEITERATOR_CID);

static NS_DEFINE_CID(kPresShellCID,  NS_PRESSHELL_CID);
static NS_DEFINE_CID(kTextNodeCID,   NS_TEXTNODE_CID);
static NS_DEFINE_CID(kNameSpaceManagerCID,  NS_NAMESPACEMANAGER_CID);
static NS_DEFINE_CID(kFrameUtilCID,  NS_FRAME_UTIL_CID);
static NS_DEFINE_CID(kEventListenerManagerCID, NS_EVENTLISTENERMANAGER_CID);
static NS_DEFINE_CID(kPrintPreviewContextCID, NS_PRINT_PREVIEW_CONTEXT_CID);

static NS_DEFINE_CID(kLayoutDocumentLoaderFactoryCID, NS_LAYOUT_DOCUMENT_LOADER_FACTORY_CID);
static NS_DEFINE_CID(kLayoutDebuggerCID, NS_LAYOUT_DEBUGGER_CID);
static NS_DEFINE_CID(kHTMLElementFactoryCID, NS_HTML_ELEMENT_FACTORY_CID);
static NS_DEFINE_CID(kTextEncoderCID, NS_TEXT_ENCODER_CID);

extern nsresult NS_NewRangeList(nsIFrameSelection** aResult);
extern nsresult NS_NewRange(nsIDOMRange** aResult);
extern nsresult NS_NewContentIterator(nsIContentIterator** aResult);
extern nsresult NS_NewContentSubtreeIterator(nsIContentIterator** aResult);
extern nsresult NS_NewFrameUtil(nsIFrameUtil** aResult);

extern nsresult NS_NewLayoutDocumentLoaderFactory(nsIDocumentLoaderFactory** aResult);
extern nsresult NS_NewLayoutDebugger(nsILayoutDebugger** aResult);
extern nsresult NS_NewHTMLElementFactory(nsIHTMLElementFactory** aResult);
extern nsresult NS_NewHTMLEncoder(nsIDocumentEncoder** aResult);
extern nsresult NS_NewTextEncoder(nsIDocumentEncoder** aResult);

//----------------------------------------------------------------------

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIFactoryIID, NS_IFACTORY_IID);

nsLayoutFactory::nsLayoutFactory(const nsCID &aClass)
{
  mRefCnt = 0;
  mClassID = aClass;
#if 0
  char* cs = aClass.ToString();
  printf("+++ Creating layout factory for %s\n", cs);
  nsCRT::free(cs);
#endif
}

nsLayoutFactory::~nsLayoutFactory()
{
  NS_ASSERTION(mRefCnt == 0, "non-zero refcnt at destruction");
#if 0
  char* cs = mClassID.ToString();
  printf("+++ Destroying layout factory for %s\n", cs);
  nsCRT::free(cs);
#endif
}

nsresult
nsLayoutFactory::QueryInterface(const nsIID &aIID, void **aResult)
{
  if (aResult == NULL) {
    return NS_ERROR_NULL_POINTER;
  }

  // Always NULL result, in case of failure
  *aResult = NULL;

  if (aIID.Equals(kISupportsIID)) {
    *aResult = (void *)(nsISupports*)this;
  } else if (aIID.Equals(kIFactoryIID)) {
    *aResult = (void *)(nsIFactory*)this;
  }

  if (*aResult == NULL) {
    return NS_NOINTERFACE;
  }

  AddRef(); // Increase reference count for caller
  return NS_OK;
}

nsrefcnt
nsLayoutFactory::AddRef()
{
  return ++mRefCnt;
}

nsrefcnt
nsLayoutFactory::Release()
{
  if (--mRefCnt == 0) {
    delete this;
    return 0; // Don't access mRefCnt after deleting!
  }
  return mRefCnt;
}

#ifdef DEBUG
#define LOG_NEW_FAILURE(_msg,_ec)                                           \
  printf("nsLayoutFactory::CreateInstance failed for %s: error=%d(0x%x)\n", \
         _msg, _ec, _ec)
#else
#define LOG_NEW_FAILURE(_msg,_ec)
#endif

nsresult
nsLayoutFactory::CreateInstance(nsISupports *aOuter,
                                const nsIID &aIID,
                                void **aResult)
{
  nsresult res;
  PRBool refCounted = PR_TRUE;

  if (aResult == NULL) {
    return NS_ERROR_NULL_POINTER;
  }

  *aResult = NULL;

  nsISupports *inst = nsnull;

  // XXX ClassID check happens here
  if (mClassID.Equals(kHTMLDocumentCID)) {
    res = NS_NewHTMLDocument((nsIDocument **)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewHTMLDocument", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kXMLDocumentCID)) {
    res = NS_NewXMLDocument((nsIDocument **)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewXMLDocument", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kImageDocumentCID)) {
    res = NS_NewImageDocument((nsIDocument **)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewImageDocument", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
#if 1
// XXX replace these with nsIHTMLElementFactory calls
  else if (mClassID.Equals(kHTMLImageElementCID)) {
    res = NS_NewHTMLImageElement((nsIHTMLContent**)&inst, nsHTMLAtoms::img);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewHTMLImageElement", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kHTMLOptionElementCID)) {
    res = NS_NewHTMLOptionElement((nsIHTMLContent**)&inst, nsHTMLAtoms::option);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewHTMLOptionElement", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
// XXX why the heck is this exported???? bad bad bad bad
  else if (mClassID.Equals(kPresShellCID)) {
    res = NS_NewPresShell((nsIPresShell**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewPresShell", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
#endif
  else if (mClassID.Equals(kFrameSelectionCID)) {
    res = NS_NewRangeList((nsIFrameSelection**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewRangeList", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kRangeCID)) {
    res = NS_NewRange((nsIDOMRange **)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewRange", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kContentIteratorCID)) {
    res = NS_NewContentIterator((nsIContentIterator **)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewContentIterator", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kSubtreeIteratorCID)) {
    res = NS_NewContentSubtreeIterator((nsIContentIterator **)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewContentSubtreeIterator", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kCSSParserCID)) {
    res = NS_NewCSSParser((nsICSSParser**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewCSSParser", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kHTMLStyleSheetCID)) {
    res = NS_NewHTMLStyleSheet((nsIHTMLStyleSheet**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewHTMLStyleSheet", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kHTMLCSSStyleSheetCID)) {
    res = NS_NewHTMLCSSStyleSheet((nsIHTMLCSSStyleSheet**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewHTMLCSSStyleSheet", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kCSSLoaderCID)) {
    res = NS_NewCSSLoader((nsICSSLoader**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewCSSLoader", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kTextNodeCID)) {
    res = NS_NewTextNode((nsIContent**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewTextNode", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kNameSpaceManagerCID)) {
    res = NS_NewNameSpaceManager((nsINameSpaceManager**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewNameSpaceManager", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kFrameUtilCID)) {
    res = NS_NewFrameUtil((nsIFrameUtil**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewFrameUtil", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kEventListenerManagerCID)) {
    res = NS_NewEventListenerManager((nsIEventListenerManager**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewEventListenerManager", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kPrintPreviewContextCID)) {
    res = NS_NewPrintPreviewContext((nsIPresContext**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewPrintPreviewContext", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kLayoutDocumentLoaderFactoryCID)) {
    res = NS_NewLayoutDocumentLoaderFactory((nsIDocumentLoaderFactory**)&inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewLayoutDocumentLoaderFactory", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kLayoutDebuggerCID)) {
    res = NS_NewLayoutDebugger((nsILayoutDebugger**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewLayoutDebugger", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kHTMLElementFactoryCID)) {
    res = NS_NewHTMLElementFactory((nsIHTMLElementFactory**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewHTMLElementFactory", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kXMLElementFactoryCID)) {
    res = NS_NewXMLElementFactory((nsIXMLElementFactory**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewXMLElementFactory", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else if (mClassID.Equals(kTextEncoderCID)) {
    res = NS_NewTextEncoder((nsIDocumentEncoder**) &inst);
    if (NS_FAILED(res)) {
      LOG_NEW_FAILURE("NS_NewTextEncoder", res);
      return res;
    }
    refCounted = PR_TRUE;
  }
  else {
    return NS_NOINTERFACE;
  }

  if (inst == NULL) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  res = inst->QueryInterface(aIID, aResult);

  if (refCounted) {
    NS_RELEASE(inst);
  }
  else if (res != NS_OK) {
    // We didn't get the right interface, so clean up
    LOG_NEW_FAILURE("final QueryInterface", res);
    delete inst;
  }

  return res;
}

nsresult nsLayoutFactory::LockFactory(PRBool aLock)
{
  // Not implemented in simplest case.
  return NS_OK;
}
