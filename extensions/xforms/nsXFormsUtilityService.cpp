/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Aaron Reed <aaronr@us.ibm.com> (original author)
 *  Alexander Surkov <surkov.alexander@gmail.com>
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

#include "nsXFormsUtilityService.h"

#include "nsIXFormsDelegate.h"
#include "nsIXFormsAccessors.h"
#include "nsIXFormsRangeConditionAccessors.h"
#include "nsIXFormsRangeAccessors.h"
#include "nsIXFormsUIWidget.h"
#include "nsXFormsUtils.h"

NS_IMPL_ISUPPORTS1(nsXFormsUtilityService, nsIXFormsUtilityService)

#define GET_XFORMS_ACCESSORS \
NS_ENSURE_ARG_POINTER(aElement);\
nsCOMPtr<nsIXFormsDelegate> delegate(do_QueryInterface(aElement));\
NS_ENSURE_TRUE(delegate, NS_ERROR_FAILURE);\
nsCOMPtr<nsIXFormsAccessors> accessors;\
delegate->GetXFormsAccessors(getter_AddRefs(accessors));\
NS_ENSURE_TRUE(accessors, NS_ERROR_FAILURE);

#define GET_XFORMS_UIWIDGET \
NS_ENSURE_ARG_POINTER(aElement);\
nsCOMPtr<nsIXFormsUIWidget> widget(do_QueryInterface(aElement));\
NS_ENSURE_TRUE(widget, NS_ERROR_FAILURE);\

NS_IMETHODIMP
nsXFormsUtilityService::IsReadonly(nsIDOMNode *aElement, PRBool *aState)
{
  NS_ENSURE_ARG_POINTER(aState);

  GET_XFORMS_ACCESSORS
  return accessors->IsReadonly(aState);
}

NS_IMETHODIMP
nsXFormsUtilityService::IsRelevant(nsIDOMNode *aElement, PRBool *aState)
{
  NS_ENSURE_ARG_POINTER(aState);

  GET_XFORMS_ACCESSORS
  return accessors->IsRelevant(aState);
}

NS_IMETHODIMP
nsXFormsUtilityService::IsRequired(nsIDOMNode *aElement, PRBool *aState)
{
  NS_ENSURE_ARG_POINTER(aState);

  GET_XFORMS_ACCESSORS
  return accessors->IsRequired(aState);
}

NS_IMETHODIMP
nsXFormsUtilityService::IsValid(nsIDOMNode *aElement, PRBool *aState)
{
  NS_ENSURE_ARG_POINTER(aState);

  GET_XFORMS_ACCESSORS
  return accessors->IsValid(aState);
}

NS_IMETHODIMP
nsXFormsUtilityService::IsInRange(nsIDOMNode *aElement, PRUint32 *aState)
{
  NS_ENSURE_ARG_POINTER(aState);
  *aState = STATE_NOT_A_RANGE;

  GET_XFORMS_ACCESSORS
  nsCOMPtr<nsIXFormsRangeConditionAccessors> raccessors(
    do_QueryInterface(accessors));
  if (!raccessors)
    return NS_OK;

  PRBool isInRange = PR_FALSE;
  nsresult rv = raccessors->IsInRange(&isInRange);
  NS_ENSURE_SUCCESS(rv, rv);

  if (isInRange)
    *aState = STATE_IN_RANGE;
  else
    *aState = STATE_OUT_OF_RANGE;

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsUtilityService::GetValue(nsIDOMNode *aElement, nsAString& aValue)
{
  GET_XFORMS_UIWIDGET
  return widget->GetCurrentValue(aValue);
}

NS_IMETHODIMP
nsXFormsUtilityService::Focus(nsIDOMNode *aElement)
{
  GET_XFORMS_UIWIDGET
  PRBool advanced = PR_FALSE;
  return widget->Focus(&advanced);
}

NS_IMETHODIMP
nsXFormsUtilityService::GetRangeStart(nsIDOMNode *aElement, nsAString& aValue)
{
  GET_XFORMS_ACCESSORS

  nsCOMPtr<nsIXFormsRangeAccessors> raccessors(do_QueryInterface(accessors));
  NS_ENSURE_TRUE(raccessors, NS_ERROR_FAILURE);
  return raccessors->GetRangeStart(aValue);
}

NS_IMETHODIMP
nsXFormsUtilityService::GetRangeEnd(nsIDOMNode *aElement, nsAString& aValue)
{
  GET_XFORMS_ACCESSORS

  nsCOMPtr<nsIXFormsRangeAccessors> raccessors(do_QueryInterface(accessors));
  NS_ENSURE_TRUE(raccessors, NS_ERROR_FAILURE);
  return raccessors->GetRangeEnd(aValue);
}

NS_IMETHODIMP
nsXFormsUtilityService::GetRangeStep(nsIDOMNode *aElement, nsAString& aValue)
{
  GET_XFORMS_ACCESSORS

  nsCOMPtr<nsIXFormsRangeAccessors> raccessors(do_QueryInterface(accessors));
  NS_ENSURE_TRUE(raccessors, NS_ERROR_FAILURE);
  return raccessors->GetRangeStep(aValue);
}

