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
/* AUTO-GENERATED. DO NOT EDIT!!! */

#include "jsapi.h"
#include "nsJSUtils.h"
#include "nscore.h"
#include "nsIScriptContext.h"
#include "nsIScriptSecurityManager.h"
#include "nsIJSScriptObject.h"
#include "nsIScriptObjectOwner.h"
#include "nsIScriptGlobalObject.h"
#include "nsIPtr.h"
#include "nsString.h"
#include "nsIDOMElement.h"
#include "nsIDOMHTMLElement.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMNSHTMLDocument.h"
#include "nsIDOMHTMLCollection.h"
#include "nsIDOMNodeList.h"


static NS_DEFINE_IID(kIScriptObjectOwnerIID, NS_ISCRIPTOBJECTOWNER_IID);
static NS_DEFINE_IID(kIJSScriptObjectIID, NS_IJSSCRIPTOBJECT_IID);
static NS_DEFINE_IID(kIScriptGlobalObjectIID, NS_ISCRIPTGLOBALOBJECT_IID);
static NS_DEFINE_IID(kIElementIID, NS_IDOMELEMENT_IID);
static NS_DEFINE_IID(kIHTMLElementIID, NS_IDOMHTMLELEMENT_IID);
static NS_DEFINE_IID(kIHTMLDocumentIID, NS_IDOMHTMLDOCUMENT_IID);
static NS_DEFINE_IID(kINSHTMLDocumentIID, NS_IDOMNSHTMLDOCUMENT_IID);
static NS_DEFINE_IID(kIHTMLCollectionIID, NS_IDOMHTMLCOLLECTION_IID);
static NS_DEFINE_IID(kINodeListIID, NS_IDOMNODELIST_IID);

NS_DEF_PTR(nsIDOMElement);
NS_DEF_PTR(nsIDOMHTMLElement);
NS_DEF_PTR(nsIDOMHTMLDocument);
NS_DEF_PTR(nsIDOMNSHTMLDocument);
NS_DEF_PTR(nsIDOMHTMLCollection);
NS_DEF_PTR(nsIDOMNodeList);

//
// HTMLDocument property ids
//
enum HTMLDocument_slots {
  HTMLDOCUMENT_TITLE = -1,
  HTMLDOCUMENT_REFERRER = -2,
  HTMLDOCUMENT_DOMAIN = -3,
  HTMLDOCUMENT_URL = -4,
  HTMLDOCUMENT_BODY = -5,
  HTMLDOCUMENT_IMAGES = -6,
  HTMLDOCUMENT_APPLETS = -7,
  HTMLDOCUMENT_LINKS = -8,
  HTMLDOCUMENT_FORMS = -9,
  HTMLDOCUMENT_ANCHORS = -10,
  HTMLDOCUMENT_COOKIE = -11,
  NSHTMLDOCUMENT_ALINKCOLOR = -12,
  NSHTMLDOCUMENT_LINKCOLOR = -13,
  NSHTMLDOCUMENT_VLINKCOLOR = -14,
  NSHTMLDOCUMENT_BGCOLOR = -15,
  NSHTMLDOCUMENT_FGCOLOR = -16,
  NSHTMLDOCUMENT_LASTMODIFIED = -17,
  NSHTMLDOCUMENT_EMBEDS = -18,
  NSHTMLDOCUMENT_LAYERS = -19,
  NSHTMLDOCUMENT_PLUGINS = -20
};

/***********************************************************************/
//
// HTMLDocument Properties Getter
//
PR_STATIC_CALLBACK(JSBool)
GetHTMLDocumentProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsIDOMHTMLDocument *a = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
    nsIScriptSecurityManager *secMan;
    PRBool ok;
    if (NS_OK != scriptCX->GetSecurityManager(&secMan)) {
      return JS_FALSE;
    }
    switch(JSVAL_TO_INT(id)) {
      case HTMLDOCUMENT_TITLE:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.title", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        if (NS_OK == a->GetTitle(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_REFERRER:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.referrer", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        if (NS_OK == a->GetReferrer(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_DOMAIN:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.domain", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        if (NS_OK == a->GetDomain(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_URL:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.url", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        if (NS_OK == a->GetURL(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_BODY:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.body", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLElement* prop;
        if (NS_OK == a->GetBody(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_IMAGES:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.images", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        if (NS_OK == a->GetImages(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_APPLETS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.applets", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        if (NS_OK == a->GetApplets(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_LINKS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.links", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        if (NS_OK == a->GetLinks(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_FORMS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.forms", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        if (NS_OK == a->GetForms(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_ANCHORS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.anchors", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        if (NS_OK == a->GetAnchors(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case HTMLDOCUMENT_COOKIE:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.cookie", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        if (NS_OK == a->GetCookie(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_ALINKCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.alinkcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetAlinkColor(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_LINKCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.linkcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetLinkColor(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_VLINKCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.vlinkcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetVlinkColor(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_BGCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.bgcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetBgColor(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_FGCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.fgcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetFgColor(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_LASTMODIFIED:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.lastmodified", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetLastModified(prop)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_EMBEDS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.embeds", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetEmbeds(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_LAYERS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.layers", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetLayers(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      case NSHTMLDOCUMENT_PLUGINS:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.plugins", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLCollection* prop;
        nsIDOMNSHTMLDocument* b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          if(NS_OK == b->GetPlugins(&prop)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
            NS_RELEASE(b);
          }
          else {
            NS_RELEASE(b);
            return JS_FALSE;
          }
        }
        else {
          JS_ReportError(cx, "Object must be of type NSHTMLDocument");
          return JS_FALSE;
        }
        break;
      }
      default:
        return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, id, vp);
    }
    NS_RELEASE(secMan);
  }
  else if (JSVAL_IS_STRING(id)) {
    nsIDOMElement* prop;
    nsIDOMNSHTMLDocument* b;
    nsAutoString name;

    JSString *jsstring = JS_ValueToString(cx, id);
    if (nsnull != jsstring) {
      name.SetString(JS_GetStringChars(jsstring));
    }
    else {
      name.SetString("");
    }

    if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
      if (NS_OK == b->NamedItem(name, &prop)) {
        NS_RELEASE(b);
        if (NULL != prop) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, vp);
        }
        else {
          return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, id, vp);
        }
      }
      else {
        NS_RELEASE(b);
        return JS_FALSE;
      }
    }
    else {
      JS_ReportError(cx, "Object must be of type NSHTMLDocument");
      return JS_FALSE;
    }
  }
  else {
    return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, id, vp);
  }

  return PR_TRUE;
}

/***********************************************************************/
//
// HTMLDocument Properties Setter
//
PR_STATIC_CALLBACK(JSBool)
SetHTMLDocumentProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsIDOMHTMLDocument *a = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
    nsIScriptSecurityManager *secMan;
    PRBool ok;
    if (NS_OK != scriptCX->GetSecurityManager(&secMan)) {
      return JS_FALSE;
    }
    switch(JSVAL_TO_INT(id)) {
      case HTMLDOCUMENT_TITLE:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.title", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetTitle(prop);
        
        break;
      }
      case HTMLDOCUMENT_BODY:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.body", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsIDOMHTMLElement* prop;
        if (PR_FALSE == nsJSUtils::nsConvertJSValToObject((nsISupports **)&prop,
                                                kIHTMLElementIID, "HTMLElement",
                                                cx, *vp)) {
          return JS_FALSE;
        }
      
        a->SetBody(prop);
        NS_IF_RELEASE(prop);
        break;
      }
      case HTMLDOCUMENT_COOKIE:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.cookie", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetCookie(prop);
        
        break;
      }
      case NSHTMLDOCUMENT_ALINKCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.alinkcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        nsIDOMNSHTMLDocument *b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          b->SetAlinkColor(prop);
          NS_RELEASE(b);
        }
        else {
           
           JS_ReportError(cx, "Object must be of type NSHTMLDocument");
           return JS_FALSE;
        }
        
        break;
      }
      case NSHTMLDOCUMENT_LINKCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.linkcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        nsIDOMNSHTMLDocument *b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          b->SetLinkColor(prop);
          NS_RELEASE(b);
        }
        else {
           
           JS_ReportError(cx, "Object must be of type NSHTMLDocument");
           return JS_FALSE;
        }
        
        break;
      }
      case NSHTMLDOCUMENT_VLINKCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.vlinkcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        nsIDOMNSHTMLDocument *b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          b->SetVlinkColor(prop);
          NS_RELEASE(b);
        }
        else {
           
           JS_ReportError(cx, "Object must be of type NSHTMLDocument");
           return JS_FALSE;
        }
        
        break;
      }
      case NSHTMLDOCUMENT_BGCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.bgcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        nsIDOMNSHTMLDocument *b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          b->SetBgColor(prop);
          NS_RELEASE(b);
        }
        else {
           
           JS_ReportError(cx, "Object must be of type NSHTMLDocument");
           return JS_FALSE;
        }
        
        break;
      }
      case NSHTMLDOCUMENT_FGCOLOR:
      {
        secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.fgcolor", &ok);
        if (!ok) {
          //Need to throw error here
          return JS_FALSE;
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        nsIDOMNSHTMLDocument *b;
        if (NS_OK == a->QueryInterface(kINSHTMLDocumentIID, (void **)&b)) {
          b->SetFgColor(prop);
          NS_RELEASE(b);
        }
        else {
           
           JS_ReportError(cx, "Object must be of type NSHTMLDocument");
           return JS_FALSE;
        }
        
        break;
      }
      default:
        return nsJSUtils::nsCallJSScriptObjectSetProperty(a, cx, id, vp);
    }
    NS_RELEASE(secMan);
  }
  else {
    return nsJSUtils::nsCallJSScriptObjectSetProperty(a, cx, id, vp);
  }

  return PR_TRUE;
}


//
// HTMLDocument finalizer
//
PR_STATIC_CALLBACK(void)
FinalizeHTMLDocument(JSContext *cx, JSObject *obj)
{
  nsJSUtils::nsGenericFinalize(cx, obj);
}


//
// HTMLDocument enumerate
//
PR_STATIC_CALLBACK(JSBool)
EnumerateHTMLDocument(JSContext *cx, JSObject *obj)
{
  return nsJSUtils::nsGenericEnumerate(cx, obj);
}


//
// HTMLDocument resolve
//
PR_STATIC_CALLBACK(JSBool)
ResolveHTMLDocument(JSContext *cx, JSObject *obj, jsval id)
{
  return nsJSUtils::nsGenericResolve(cx, obj, id);
}


//
// Native method Open
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocumentOpen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *nativeThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  JSBool rBool = JS_FALSE;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.open", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 0) {

    if (NS_OK != nativeThis->Open(cx, argv+0, argc-0)) {
      return JS_FALSE;
    }

    *rval = JSVAL_VOID;
  }
  else {
    JS_ReportError(cx, "Function open requires 0 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method Close
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocumentClose(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *nativeThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  JSBool rBool = JS_FALSE;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.close", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 0) {

    if (NS_OK != nativeThis->Close()) {
      return JS_FALSE;
    }

    *rval = JSVAL_VOID;
  }
  else {
    JS_ReportError(cx, "Function close requires 0 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method Write
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocumentWrite(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *nativeThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  JSBool rBool = JS_FALSE;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.write", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 0) {

    if (NS_OK != nativeThis->Write(cx, argv+0, argc-0)) {
      return JS_FALSE;
    }

    *rval = JSVAL_VOID;
  }
  else {
    JS_ReportError(cx, "Function write requires 0 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method Writeln
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocumentWriteln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *nativeThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  JSBool rBool = JS_FALSE;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.writeln", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 0) {

    if (NS_OK != nativeThis->Writeln(cx, argv+0, argc-0)) {
      return JS_FALSE;
    }

    *rval = JSVAL_VOID;
  }
  else {
    JS_ReportError(cx, "Function writeln requires 0 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method GetElementById
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocumentGetElementById(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *nativeThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  JSBool rBool = JS_FALSE;
  nsIDOMElement* nativeRet;
  nsAutoString b0;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.getelementbyid", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 1) {

    nsJSUtils::nsConvertJSValToString(b0, cx, argv[0]);

    if (NS_OK != nativeThis->GetElementById(b0, &nativeRet)) {
      return JS_FALSE;
    }

    nsJSUtils::nsConvertObjectToJSVal(nativeRet, cx, rval);
  }
  else {
    JS_ReportError(cx, "Function getElementById requires 1 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method GetElementsByName
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocumentGetElementsByName(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *nativeThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  JSBool rBool = JS_FALSE;
  nsIDOMNodeList* nativeRet;
  nsAutoString b0;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "htmldocument.getelementsbyname", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 1) {

    nsJSUtils::nsConvertJSValToString(b0, cx, argv[0]);

    if (NS_OK != nativeThis->GetElementsByName(b0, &nativeRet)) {
      return JS_FALSE;
    }

    nsJSUtils::nsConvertObjectToJSVal(nativeRet, cx, rval);
  }
  else {
    JS_ReportError(cx, "Function getElementsByName requires 1 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method GetSelection
//
PR_STATIC_CALLBACK(JSBool)
NSHTMLDocumentGetSelection(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *privateThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  nsIDOMNSHTMLDocument *nativeThis = nsnull;
  if (NS_OK != privateThis->QueryInterface(kINSHTMLDocumentIID, (void **)&nativeThis)) {
    JS_ReportError(cx, "Object must be of type NSHTMLDocument");
    return JS_FALSE;
  }

  JSBool rBool = JS_FALSE;
  nsAutoString nativeRet;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.getselection", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 0) {

    if (NS_OK != nativeThis->GetSelection(nativeRet)) {
      return JS_FALSE;
    }

    nsJSUtils::nsConvertStringToJSVal(nativeRet, cx, rval);
  }
  else {
    JS_ReportError(cx, "Function getSelection requires 0 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method NamedItem
//
PR_STATIC_CALLBACK(JSBool)
NSHTMLDocumentNamedItem(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLDocument *privateThis = (nsIDOMHTMLDocument*)JS_GetPrivate(cx, obj);
  nsIDOMNSHTMLDocument *nativeThis = nsnull;
  if (NS_OK != privateThis->QueryInterface(kINSHTMLDocumentIID, (void **)&nativeThis)) {
    JS_ReportError(cx, "Object must be of type NSHTMLDocument");
    return JS_FALSE;
  }

  JSBool rBool = JS_FALSE;
  nsIDOMElement* nativeRet;
  nsAutoString b0;

  *rval = JSVAL_NULL;

  nsIScriptContext *scriptCX = (nsIScriptContext *)JS_GetContextPrivate(cx);
  nsIScriptSecurityManager *secMan;
  if (NS_OK == scriptCX->GetSecurityManager(&secMan)) {
    PRBool ok;
    secMan->CheckScriptAccess(scriptCX, obj, "nshtmldocument.nameditem", &ok);
    if (!ok) {
      //Need to throw error here
      return JS_FALSE;
    }
    NS_RELEASE(secMan);
  }
  else {
    return JS_FALSE;
  }

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 1) {

    nsJSUtils::nsConvertJSValToString(b0, cx, argv[0]);

    if (NS_OK != nativeThis->NamedItem(b0, &nativeRet)) {
      return JS_FALSE;
    }

    nsJSUtils::nsConvertObjectToJSVal(nativeRet, cx, rval);
  }
  else {
    JS_ReportError(cx, "Function namedItem requires 1 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


/***********************************************************************/
//
// class for HTMLDocument
//
JSClass HTMLDocumentClass = {
  "HTMLDocument", 
  JSCLASS_HAS_PRIVATE | JSCLASS_PRIVATE_IS_NSISUPPORTS,
  JS_PropertyStub,
  JS_PropertyStub,
  GetHTMLDocumentProperty,
  SetHTMLDocumentProperty,
  EnumerateHTMLDocument,
  ResolveHTMLDocument,
  JS_ConvertStub,
  FinalizeHTMLDocument
};


//
// HTMLDocument class properties
//
static JSPropertySpec HTMLDocumentProperties[] =
{
  {"title",    HTMLDOCUMENT_TITLE,    JSPROP_ENUMERATE},
  {"referrer",    HTMLDOCUMENT_REFERRER,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"domain",    HTMLDOCUMENT_DOMAIN,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"URL",    HTMLDOCUMENT_URL,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"body",    HTMLDOCUMENT_BODY,    JSPROP_ENUMERATE},
  {"images",    HTMLDOCUMENT_IMAGES,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"applets",    HTMLDOCUMENT_APPLETS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"links",    HTMLDOCUMENT_LINKS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"forms",    HTMLDOCUMENT_FORMS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"anchors",    HTMLDOCUMENT_ANCHORS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"cookie",    HTMLDOCUMENT_COOKIE,    JSPROP_ENUMERATE},
  {"alinkColor",    NSHTMLDOCUMENT_ALINKCOLOR,    JSPROP_ENUMERATE},
  {"linkColor",    NSHTMLDOCUMENT_LINKCOLOR,    JSPROP_ENUMERATE},
  {"vlinkColor",    NSHTMLDOCUMENT_VLINKCOLOR,    JSPROP_ENUMERATE},
  {"bgColor",    NSHTMLDOCUMENT_BGCOLOR,    JSPROP_ENUMERATE},
  {"fgColor",    NSHTMLDOCUMENT_FGCOLOR,    JSPROP_ENUMERATE},
  {"lastModified",    NSHTMLDOCUMENT_LASTMODIFIED,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"embeds",    NSHTMLDOCUMENT_EMBEDS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"layers",    NSHTMLDOCUMENT_LAYERS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {"plugins",    NSHTMLDOCUMENT_PLUGINS,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {0}
};


//
// HTMLDocument class methods
//
static JSFunctionSpec HTMLDocumentMethods[] = 
{
  {"open",          HTMLDocumentOpen,     0},
  {"close",          HTMLDocumentClose,     0},
  {"write",          HTMLDocumentWrite,     0},
  {"writeln",          HTMLDocumentWriteln,     0},
  {"getElementById",          HTMLDocumentGetElementById,     1},
  {"getElementsByName",          HTMLDocumentGetElementsByName,     1},
  {"getSelection",          NSHTMLDocumentGetSelection,     0},
  {"namedItem",          NSHTMLDocumentNamedItem,     1},
  {0}
};


//
// HTMLDocument constructor
//
PR_STATIC_CALLBACK(JSBool)
HTMLDocument(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  return JS_FALSE;
}


//
// HTMLDocument class initialization
//
extern "C" NS_DOM nsresult NS_InitHTMLDocumentClass(nsIScriptContext *aContext, void **aPrototype)
{
  JSContext *jscontext = (JSContext *)aContext->GetNativeContext();
  JSObject *proto = nsnull;
  JSObject *constructor = nsnull;
  JSObject *parent_proto = nsnull;
  JSObject *global = JS_GetGlobalObject(jscontext);
  jsval vp;

  if ((PR_TRUE != JS_LookupProperty(jscontext, global, "HTMLDocument", &vp)) ||
      !JSVAL_IS_OBJECT(vp) ||
      ((constructor = JSVAL_TO_OBJECT(vp)) == nsnull) ||
      (PR_TRUE != JS_LookupProperty(jscontext, JSVAL_TO_OBJECT(vp), "prototype", &vp)) || 
      !JSVAL_IS_OBJECT(vp)) {

    if (NS_OK != NS_InitDocumentClass(aContext, (void **)&parent_proto)) {
      return NS_ERROR_FAILURE;
    }
    proto = JS_InitClass(jscontext,     // context
                         global,        // global object
                         parent_proto,  // parent proto 
                         &HTMLDocumentClass,      // JSClass
                         HTMLDocument,            // JSNative ctor
                         0,             // ctor args
                         HTMLDocumentProperties,  // proto props
                         HTMLDocumentMethods,     // proto funcs
                         nsnull,        // ctor props (static)
                         nsnull);       // ctor funcs (static)
    if (nsnull == proto) {
      return NS_ERROR_FAILURE;
    }

  }
  else if ((nsnull != constructor) && JSVAL_IS_OBJECT(vp)) {
    proto = JSVAL_TO_OBJECT(vp);
  }
  else {
    return NS_ERROR_FAILURE;
  }

  if (aPrototype) {
    *aPrototype = proto;
  }
  return NS_OK;
}


//
// Method for creating a new HTMLDocument JavaScript object
//
extern "C" NS_DOM nsresult NS_NewScriptHTMLDocument(nsIScriptContext *aContext, nsISupports *aSupports, nsISupports *aParent, void **aReturn)
{
  NS_PRECONDITION(nsnull != aContext && nsnull != aSupports && nsnull != aReturn, "null argument to NS_NewScriptHTMLDocument");
  JSObject *proto;
  JSObject *parent;
  nsIScriptObjectOwner *owner;
  JSContext *jscontext = (JSContext *)aContext->GetNativeContext();
  nsresult result = NS_OK;
  nsIDOMHTMLDocument *aHTMLDocument;

  if (nsnull == aParent) {
    parent = nsnull;
  }
  else if (NS_OK == aParent->QueryInterface(kIScriptObjectOwnerIID, (void**)&owner)) {
    if (NS_OK != owner->GetScriptObject(aContext, (void **)&parent)) {
      NS_RELEASE(owner);
      return NS_ERROR_FAILURE;
    }
    NS_RELEASE(owner);
  }
  else {
    return NS_ERROR_FAILURE;
  }

  if (NS_OK != NS_InitHTMLDocumentClass(aContext, (void **)&proto)) {
    return NS_ERROR_FAILURE;
  }

  result = aSupports->QueryInterface(kIHTMLDocumentIID, (void **)&aHTMLDocument);
  if (NS_OK != result) {
    return result;
  }

  // create a js object for this class
  *aReturn = JS_NewObject(jscontext, &HTMLDocumentClass, proto, parent);
  if (nsnull != *aReturn) {
    // connect the native object to the js object
    JS_SetPrivate(jscontext, (JSObject *)*aReturn, aHTMLDocument);
  }
  else {
    NS_RELEASE(aHTMLDocument);
    return NS_ERROR_FAILURE; 
  }

  return NS_OK;
}
