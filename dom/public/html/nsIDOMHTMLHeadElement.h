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

#ifndef nsIDOMHTMLHeadElement_h__
#define nsIDOMHTMLHeadElement_h__

#include "nsISupports.h"
#include "nsString.h"
#include "nsIScriptContext.h"
#include "nsIDOMHTMLElement.h"


#define NS_IDOMHTMLHEADELEMENT_IID \
 { 0xa6cf9087, 0x15b3, 0x11d2, \
  { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 } } 

class nsIDOMHTMLHeadElement : public nsIDOMHTMLElement {
public:

  NS_IMETHOD    GetProfile(nsString& aProfile)=0;
  NS_IMETHOD    SetProfile(const nsString& aProfile)=0;
};


#define NS_DECL_IDOMHTMLHEADELEMENT   \
  NS_IMETHOD    GetProfile(nsString& aProfile);  \
  NS_IMETHOD    SetProfile(const nsString& aProfile);  \



#define NS_FORWARD_IDOMHTMLHEADELEMENT(_to)  \
  NS_IMETHOD    GetProfile(nsString& aProfile) { return _to##GetProfile(aProfile); } \
  NS_IMETHOD    SetProfile(const nsString& aProfile) { return _to##SetProfile(aProfile); } \


extern nsresult NS_InitHTMLHeadElementClass(nsIScriptContext *aContext, void **aPrototype);

extern "C" NS_DOM nsresult NS_NewScriptHTMLHeadElement(nsIScriptContext *aContext, nsISupports *aSupports, nsISupports *aParent, void **aReturn);

#endif // nsIDOMHTMLHeadElement_h__
