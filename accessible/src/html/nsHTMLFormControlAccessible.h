/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Author: Eric D Vaughan (evaughan@netscape.com)
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

#ifndef _nsHTMLFormControlAccessible_H_
#define _nsHTMLFormControlAccessible_H_

#include "nsAccessible.h"
#include "nsBaseWidgetAccessible.h"

class nsICheckboxControlFrame;

/* Accessible for supporting for controls
 * supports:
 * - walking up to get name from label
 * - support basic state
 */
class nsHTMLFormControlAccessible : public nsLeafAccessible
{

public:
  nsHTMLFormControlAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  NS_IMETHOD GetAccName(nsAWritableString& _retval); 
  NS_IMETHOD GetAccState(PRUint32 *_retval); 

protected:
  NS_IMETHODIMP AppendLabelFor(nsIContent *aLookNode, nsAReadableString *aId, nsAWritableString *aLabel);
};

class nsHTMLCheckboxAccessible : public nsHTMLFormControlAccessible
{

public:
  nsHTMLCheckboxAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  NS_IMETHOD GetAccRole(PRUint32 *_retval); 
  NS_IMETHOD GetAccNumActions(PRUint8 *_retval);
  NS_IMETHOD GetAccActionName(PRUint8 index, nsAWritableString& _retval);
  NS_IMETHOD AccDoAction(PRUint8 index);
  NS_IMETHOD GetAccState(PRUint32 *_retval); 
};

class nsHTMLRadioButtonAccessible : public nsHTMLFormControlAccessible
{

public:
  nsHTMLRadioButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  NS_IMETHOD GetAccRole(PRUint32 *_retval); 
  NS_IMETHOD GetAccNumActions(PRUint8 *_retval);
  NS_IMETHOD GetAccActionName(PRUint8 index, nsAWritableString& _retval);
  NS_IMETHOD AccDoAction(PRUint8 index);
  NS_IMETHOD GetAccState(PRUint32 *_retval); 
};

class nsHTMLButtonAccessible : public nsHTMLFormControlAccessible
{

public:
  nsHTMLButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  NS_IMETHOD GetAccRole(PRUint32 *_retval); 
  NS_IMETHOD GetAccName(nsAWritableString& _retval); 
  NS_IMETHOD GetAccNumActions(PRUint8 *_retval);
  NS_IMETHOD GetAccActionName(PRUint8 index, nsAWritableString& _retval);
  NS_IMETHOD AccDoAction(PRUint8 index);
};

class nsHTML4ButtonAccessible : public nsBlockAccessible
{

public:
  nsHTML4ButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  NS_IMETHOD GetAccRole(PRUint32 *_retval); 
  NS_IMETHOD GetAccName(nsAWritableString& _retval); 
  NS_IMETHOD GetAccState(PRUint32 *_retval);
  NS_IMETHOD GetAccNumActions(PRUint8 *_retval);
  NS_IMETHOD GetAccActionName(PRUint8 index, nsAWritableString& _retval);
  NS_IMETHOD AccDoAction(PRUint8 index);
};


class nsHTMLTextFieldAccessible : public nsHTMLFormControlAccessible
{
public:
  nsHTMLTextFieldAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  NS_IMETHOD GetAccRole(PRUint32 *_retval); 
  NS_IMETHOD GetAccValue(nsAWritableString& _retval); 
  NS_IMETHOD GetAccState(PRUint32 *_retval);
};

#endif  
