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

#include "nsROCSSPrimitiveValue.h"

#include "nsCOMPtr.h"
#include "nsDOMError.h"
#include "prprf.h"


nsROCSSPrimitiveValue::nsROCSSPrimitiveValue(nsISupports *aOwner, float aT2P)
  : mType(CSS_PX), mTwips(0), mString(), mOwner(aOwner), mT2P(aT2P),
    mScriptObject(nsnull)
{
  NS_INIT_REFCNT();
}


nsROCSSPrimitiveValue::~nsROCSSPrimitiveValue()
{
}


NS_IMPL_ADDREF(nsROCSSPrimitiveValue);
NS_IMPL_RELEASE(nsROCSSPrimitiveValue);


NS_INTERFACE_MAP_BEGIN(nsROCSSPrimitiveValue)
   NS_INTERFACE_MAP_ENTRY(nsIDOMCSSPrimitiveValue)
   NS_INTERFACE_MAP_ENTRY(nsIDOMCSSValue)
   NS_INTERFACE_MAP_ENTRY(nsIScriptObjectOwner)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMCSSPrimitiveValue)
NS_INTERFACE_MAP_END


// nsIScriptObjectOwner

NS_IMETHODIMP
nsROCSSPrimitiveValue::GetScriptObject(nsIScriptContext *aContext, 
                                       void** aScriptObject)
{
  nsresult res = NS_OK;

  if (!mScriptObject) {
    nsISupports *supports = NS_STATIC_CAST(nsIDOMCSSPrimitiveValue *, this);

    // XXX Should be done through factory
    res = NS_NewScriptCSSPrimitiveValue(aContext, supports, mOwner,
                                        &mScriptObject);
  }

  *aScriptObject = mScriptObject;

  return res;
}


// nsIDOMCSSValue

NS_IMETHODIMP
nsROCSSPrimitiveValue::SetScriptObject(void* aScriptObject)
{
  mScriptObject = aScriptObject;
  return NS_OK;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::GetCssText(nsString& aCssText)
{
  aCssText.Truncate();

  switch (mType) {
    case CSS_PX :
      {
        PRInt32 px = NSTwipsToIntPixels(mTwips, mT2P);
        aCssText.AppendInt(px);
        aCssText.AppendWithConversion("px");

        break;
      }
    case CSS_CM :
      {
        float val = NS_TWIPS_TO_CENTIMETERS(mTwips);
        char buf[64];
        PR_snprintf(buf, 63, "%.2fcm", val);
        aCssText.AppendWithConversion("cm");
        break;
      }
    case CSS_MM :
      {
        float val = NS_TWIPS_TO_MILLIMETERS(mTwips);
        char buf[64];
        PR_snprintf(buf, 63, "%.2fcm", val);
        aCssText.AppendWithConversion("mm");
        break;
      }
    case CSS_IN :
      {
        float val = NS_TWIPS_TO_INCHES(mTwips);
        char buf[64];
        PR_snprintf(buf, 63, "%.2fcm", val);
        aCssText.AppendWithConversion("in");
        break;
      }
    case CSS_PT :
      {
        float val = NSTwipsToFloatPoints(mTwips);
        char buf[64];
        PR_snprintf(buf, 63, "%.2fcm", val);
        aCssText.AppendWithConversion("pt");
        break;
      }
    case CSS_STRING :
      {
        aCssText.Append(mString);
        break;
      }
    case CSS_PC :
    case CSS_UNKNOWN :
    case CSS_NUMBER :
    case CSS_PERCENTAGE :
    case CSS_EMS :
    case CSS_EXS :
    case CSS_DEG :
    case CSS_RAD :
    case CSS_GRAD :
    case CSS_MS :
    case CSS_S :
    case CSS_HZ :
    case CSS_KHZ :
    case CSS_DIMENSION :
    case CSS_URI :
    case CSS_IDENT :
    case CSS_ATTR :
    case CSS_COUNTER :
    case CSS_RECT :
    case CSS_RGBCOLOR :
      return NS_ERROR_DOM_INVALID_ACCESS_ERR;
  }

  return NS_OK;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::SetCssText(const nsString& aCssText)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::GetValueType(PRUint16* aValueType)
{
  NS_ENSURE_ARG_POINTER(aValueType);
  *aValueType = nsIDOMCSSValue::CSS_PRIMITIVE_VALUE;
  return NS_OK;
}


// nsIDOMCSSPrimitiveValue

NS_IMETHODIMP
nsROCSSPrimitiveValue::GetPrimitiveType(PRUint16* aPrimitiveType)
{
  NS_ENSURE_ARG_POINTER(aPrimitiveType);
  *aPrimitiveType = mType;

  return NS_OK;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::SetFloatValue(PRUint16 aUnitType, float aFloatValue)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::GetFloatValue(PRUint16 aUnitType, float* aReturn)
{
  NS_ENSURE_ARG_POINTER(aReturn);
  *aReturn = 0;

  if (mType == CSS_STRING) {
    return NS_ERROR_DOM_INVALID_ACCESS_ERR;
  }

  switch(aUnitType) {
    case CSS_PX :
      *aReturn = NSTwipsToFloatPixels(mTwips, mT2P);
      break;
    case CSS_CM :
      *aReturn = NS_TWIPS_TO_CENTIMETERS(mTwips);
      break;
    case CSS_MM :
      *aReturn = NS_TWIPS_TO_MILLIMETERS(mTwips);
      break;
    case CSS_IN :
      *aReturn = NS_TWIPS_TO_INCHES(mTwips);
      break;
    case CSS_PT :
      *aReturn = NSTwipsToFloatPoints(mTwips);
      break;
    case CSS_PC :
    case CSS_UNKNOWN :
    case CSS_NUMBER :
    case CSS_PERCENTAGE :
    case CSS_EMS :
    case CSS_EXS :
    case CSS_DEG :
    case CSS_RAD :
    case CSS_GRAD :
    case CSS_MS :
    case CSS_S :
    case CSS_HZ :
    case CSS_KHZ :
    case CSS_DIMENSION :
    case CSS_STRING :
    case CSS_URI :
    case CSS_IDENT :
    case CSS_ATTR :
    case CSS_COUNTER :
    case CSS_RECT :
    case CSS_RGBCOLOR :
      return NS_ERROR_DOM_INVALID_ACCESS_ERR;
  }

  return NS_OK;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::SetStringValue(PRUint16 aStringType,
                                      const nsString& aStringValue)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::GetStringValue(nsString& aReturn)
{
  aReturn = mString;
  return NS_OK;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::GetCounterValue(nsIDOMCounter** aReturn)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsROCSSPrimitiveValue::GetRectValue(nsIDOMRect** aReturn)
{
  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}


NS_IMETHODIMP 
nsROCSSPrimitiveValue::GetRGBColorValue(nsIDOMRGBColor** aReturn)
{
  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}

