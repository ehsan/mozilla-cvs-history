/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
/* AUTO-GENERATED. DO NOT EDIT!!! */

#include "jsapi.h"
#include "nsJSUtils.h"
#include "nsDOMError.h"
#include "nscore.h"
#include "nsIServiceManager.h"
#include "nsIScriptContext.h"
#include "nsIScriptSecurityManager.h"
#include "nsIJSScriptObject.h"
#include "nsIScriptObjectOwner.h"
#include "nsIScriptGlobalObject.h"
#include "nsCOMPtr.h"
#include "nsDOMPropEnums.h"
#include "nsString.h"
#include "nsIDOMHTMLAnchorElement.h"


static NS_DEFINE_IID(kIScriptObjectOwnerIID, NS_ISCRIPTOBJECTOWNER_IID);
static NS_DEFINE_IID(kIJSScriptObjectIID, NS_IJSSCRIPTOBJECT_IID);
static NS_DEFINE_IID(kIScriptGlobalObjectIID, NS_ISCRIPTGLOBALOBJECT_IID);
static NS_DEFINE_IID(kIHTMLAnchorElementIID, NS_IDOMHTMLANCHORELEMENT_IID);

//
// HTMLAnchorElement property ids
//
enum HTMLAnchorElement_slots {
  HTMLANCHORELEMENT_ACCESSKEY = -1,
  HTMLANCHORELEMENT_CHARSET = -2,
  HTMLANCHORELEMENT_COORDS = -3,
  HTMLANCHORELEMENT_HREF = -4,
  HTMLANCHORELEMENT_HREFLANG = -5,
  HTMLANCHORELEMENT_NAME = -6,
  HTMLANCHORELEMENT_REL = -7,
  HTMLANCHORELEMENT_REV = -8,
  HTMLANCHORELEMENT_SHAPE = -9,
  HTMLANCHORELEMENT_TABINDEX = -10,
  HTMLANCHORELEMENT_TARGET = -11,
  HTMLANCHORELEMENT_TYPE = -12
};

/***********************************************************************/
//
// HTMLAnchorElement Properties Getter
//
PR_STATIC_CALLBACK(JSBool)
GetHTMLAnchorElementProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsIDOMHTMLAnchorElement *a = (nsIDOMHTMLAnchorElement*)nsJSUtils::nsGetNativeThis(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_PROGID, &rv);
    if (NS_FAILED(rv)) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
    }
    switch(JSVAL_TO_INT(id)) {
      case HTMLANCHORELEMENT_ACCESSKEY:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_ACCESSKEY, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetAccessKey(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_CHARSET:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_CHARSET, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetCharset(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_COORDS:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_COORDS, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetCoords(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_HREF:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_HREF, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetHref(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_HREFLANG:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_HREFLANG, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetHreflang(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_NAME:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_NAME, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetName(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_REL:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_REL, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetRel(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_REV:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_REV, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetRev(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_SHAPE:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_SHAPE, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetShape(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_TABINDEX:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_TABINDEX, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        PRInt32 prop;
        nsresult result = NS_OK;
        result = a->GetTabIndex(&prop);
        if (NS_SUCCEEDED(result)) {
          *vp = INT_TO_JSVAL(prop);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_TARGET:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_TARGET, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetTarget(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      case HTMLANCHORELEMENT_TYPE:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_TYPE, PR_FALSE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsresult result = NS_OK;
        result = a->GetType(prop);
        if (NS_SUCCEEDED(result)) {
          nsJSUtils::nsConvertStringToJSVal(prop, cx, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      default:
        return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, obj, id, vp);
    }
  }
  else {
    return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, obj, id, vp);
  }

  return PR_TRUE;
}

/***********************************************************************/
//
// HTMLAnchorElement Properties Setter
//
PR_STATIC_CALLBACK(JSBool)
SetHTMLAnchorElementProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsIDOMHTMLAnchorElement *a = (nsIDOMHTMLAnchorElement*)nsJSUtils::nsGetNativeThis(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_PROGID, &rv);
    if (NS_FAILED(rv)) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
    }
    switch(JSVAL_TO_INT(id)) {
      case HTMLANCHORELEMENT_ACCESSKEY:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_ACCESSKEY, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetAccessKey(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_CHARSET:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_CHARSET, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetCharset(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_COORDS:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_COORDS, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetCoords(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_HREF:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_HREF, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetHref(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_HREFLANG:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_HREFLANG, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetHreflang(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_NAME:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_NAME, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetName(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_REL:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_REL, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetRel(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_REV:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_REV, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetRev(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_SHAPE:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_SHAPE, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetShape(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_TABINDEX:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_TABINDEX, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        PRInt32 prop;
        int32 temp;
        if (JSVAL_IS_NUMBER(*vp) && JS_ValueToInt32(cx, *vp, &temp)) {
          prop = (PRInt32)temp;
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_NOT_NUMBER_ERR);
        }
      
        a->SetTabIndex(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_TARGET:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_TARGET, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetTarget(prop);
        
        break;
      }
      case HTMLANCHORELEMENT_TYPE:
      {
        PRBool ok = PR_FALSE;
        secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_TYPE, PR_TRUE, &ok);
        if (!ok) {
          return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
        }
        nsAutoString prop;
        nsJSUtils::nsConvertJSValToString(prop, cx, *vp);
      
        a->SetType(prop);
        
        break;
      }
      default:
        return nsJSUtils::nsCallJSScriptObjectSetProperty(a, cx, obj, id, vp);
    }
  }
  else {
    return nsJSUtils::nsCallJSScriptObjectSetProperty(a, cx, obj, id, vp);
  }

  return PR_TRUE;
}


//
// HTMLAnchorElement finalizer
//
PR_STATIC_CALLBACK(void)
FinalizeHTMLAnchorElement(JSContext *cx, JSObject *obj)
{
  nsJSUtils::nsGenericFinalize(cx, obj);
}


//
// HTMLAnchorElement enumerate
//
PR_STATIC_CALLBACK(JSBool)
EnumerateHTMLAnchorElement(JSContext *cx, JSObject *obj)
{
  return nsJSUtils::nsGenericEnumerate(cx, obj);
}


//
// HTMLAnchorElement resolve
//
PR_STATIC_CALLBACK(JSBool)
ResolveHTMLAnchorElement(JSContext *cx, JSObject *obj, jsval id)
{
  return nsJSUtils::nsGenericResolve(cx, obj, id);
}


//
// Native method Blur
//
PR_STATIC_CALLBACK(JSBool)
HTMLAnchorElementBlur(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLAnchorElement *nativeThis = (nsIDOMHTMLAnchorElement*)nsJSUtils::nsGetNativeThis(cx, obj);
  nsresult result = NS_OK;
  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  {

  *rval = JSVAL_NULL;

  {
    PRBool ok;
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_PROGID, &rv);
    if (NS_FAILED(rv)) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
    }
    secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_BLUR, PR_FALSE, &ok);
    if (!ok) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
    }
  }


    result = nativeThis->Blur();
    if (NS_FAILED(result)) {
      return nsJSUtils::nsReportError(cx, obj, result);
    }

    *rval = JSVAL_VOID;
  }

  return JS_TRUE;
}


//
// Native method Focus
//
PR_STATIC_CALLBACK(JSBool)
HTMLAnchorElementFocus(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsIDOMHTMLAnchorElement *nativeThis = (nsIDOMHTMLAnchorElement*)nsJSUtils::nsGetNativeThis(cx, obj);
  nsresult result = NS_OK;
  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  {

  *rval = JSVAL_NULL;

  {
    PRBool ok;
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_PROGID, &rv);
    if (NS_FAILED(rv)) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
    }
    secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLANCHORELEMENT_FOCUS, PR_FALSE, &ok);
    if (!ok) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECURITY_ERR);
    }
  }


    result = nativeThis->Focus();
    if (NS_FAILED(result)) {
      return nsJSUtils::nsReportError(cx, obj, result);
    }

    *rval = JSVAL_VOID;
  }

  return JS_TRUE;
}


/***********************************************************************/
//
// class for HTMLAnchorElement
//
JSClass HTMLAnchorElementClass = {
  "HTMLAnchorElement", 
  JSCLASS_HAS_PRIVATE | JSCLASS_PRIVATE_IS_NSISUPPORTS,
  JS_PropertyStub,
  JS_PropertyStub,
  GetHTMLAnchorElementProperty,
  SetHTMLAnchorElementProperty,
  EnumerateHTMLAnchorElement,
  ResolveHTMLAnchorElement,
  JS_ConvertStub,
  FinalizeHTMLAnchorElement
};


//
// HTMLAnchorElement class properties
//
static JSPropertySpec HTMLAnchorElementProperties[] =
{
  {"accessKey",    HTMLANCHORELEMENT_ACCESSKEY,    JSPROP_ENUMERATE},
  {"charset",    HTMLANCHORELEMENT_CHARSET,    JSPROP_ENUMERATE},
  {"coords",    HTMLANCHORELEMENT_COORDS,    JSPROP_ENUMERATE},
  {"href",    HTMLANCHORELEMENT_HREF,    JSPROP_ENUMERATE},
  {"hreflang",    HTMLANCHORELEMENT_HREFLANG,    JSPROP_ENUMERATE},
  {"name",    HTMLANCHORELEMENT_NAME,    JSPROP_ENUMERATE},
  {"rel",    HTMLANCHORELEMENT_REL,    JSPROP_ENUMERATE},
  {"rev",    HTMLANCHORELEMENT_REV,    JSPROP_ENUMERATE},
  {"shape",    HTMLANCHORELEMENT_SHAPE,    JSPROP_ENUMERATE},
  {"tabIndex",    HTMLANCHORELEMENT_TABINDEX,    JSPROP_ENUMERATE},
  {"target",    HTMLANCHORELEMENT_TARGET,    JSPROP_ENUMERATE},
  {"type",    HTMLANCHORELEMENT_TYPE,    JSPROP_ENUMERATE},
  {0}
};


//
// HTMLAnchorElement class methods
//
static JSFunctionSpec HTMLAnchorElementMethods[] = 
{
  {"blur",          HTMLAnchorElementBlur,     0},
  {"focus",          HTMLAnchorElementFocus,     0},
  {0}
};


//
// HTMLAnchorElement constructor
//
PR_STATIC_CALLBACK(JSBool)
HTMLAnchorElement(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  return JS_FALSE;
}


//
// HTMLAnchorElement class initialization
//
extern "C" NS_DOM nsresult NS_InitHTMLAnchorElementClass(nsIScriptContext *aContext, void **aPrototype)
{
  JSContext *jscontext = (JSContext *)aContext->GetNativeContext();
  JSObject *proto = nsnull;
  JSObject *constructor = nsnull;
  JSObject *parent_proto = nsnull;
  JSObject *global = JS_GetGlobalObject(jscontext);
  jsval vp;

  if ((PR_TRUE != JS_LookupProperty(jscontext, global, "HTMLAnchorElement", &vp)) ||
      !JSVAL_IS_OBJECT(vp) ||
      ((constructor = JSVAL_TO_OBJECT(vp)) == nsnull) ||
      (PR_TRUE != JS_LookupProperty(jscontext, JSVAL_TO_OBJECT(vp), "prototype", &vp)) || 
      !JSVAL_IS_OBJECT(vp)) {

    if (NS_OK != NS_InitHTMLElementClass(aContext, (void **)&parent_proto)) {
      return NS_ERROR_FAILURE;
    }
    proto = JS_InitClass(jscontext,     // context
                         global,        // global object
                         parent_proto,  // parent proto 
                         &HTMLAnchorElementClass,      // JSClass
                         HTMLAnchorElement,            // JSNative ctor
                         0,             // ctor args
                         HTMLAnchorElementProperties,  // proto props
                         HTMLAnchorElementMethods,     // proto funcs
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
// Method for creating a new HTMLAnchorElement JavaScript object
//
extern "C" NS_DOM nsresult NS_NewScriptHTMLAnchorElement(nsIScriptContext *aContext, nsISupports *aSupports, nsISupports *aParent, void **aReturn)
{
  NS_PRECONDITION(nsnull != aContext && nsnull != aSupports && nsnull != aReturn, "null argument to NS_NewScriptHTMLAnchorElement");
  JSObject *proto;
  JSObject *parent;
  nsIScriptObjectOwner *owner;
  JSContext *jscontext = (JSContext *)aContext->GetNativeContext();
  nsresult result = NS_OK;
  nsIDOMHTMLAnchorElement *aHTMLAnchorElement;

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

  if (NS_OK != NS_InitHTMLAnchorElementClass(aContext, (void **)&proto)) {
    return NS_ERROR_FAILURE;
  }

  result = aSupports->QueryInterface(kIHTMLAnchorElementIID, (void **)&aHTMLAnchorElement);
  if (NS_OK != result) {
    return result;
  }

  // create a js object for this class
  *aReturn = JS_NewObject(jscontext, &HTMLAnchorElementClass, proto, parent);
  if (nsnull != *aReturn) {
    // connect the native object to the js object
    JS_SetPrivate(jscontext, (JSObject *)*aReturn, aHTMLAnchorElement);
  }
  else {
    NS_RELEASE(aHTMLAnchorElement);
    return NS_ERROR_FAILURE; 
  }

  return NS_OK;
}
