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

#ifndef nsIPrivateDOMEvent_h__
#define nsIPrivateDOMEvent_h__

#include "nsGUIEvent.h"
#include "nsISupports.h"

class nsIPresContext;

/*
 * Event listener manager interface.
 */
#define NS_IPRIVATEDOMEVENT_IID \
{ /* 80a98c80-2036-11d2-bd89-00805f8ae3f4 */ \
0x80a98c80, 0x2036, 0x11d2, \
{0xbd, 0x89, 0x00, 0x80, 0x5f, 0x8a, 0xe3, 0xf4} }

class nsIDOMNode;

class nsIPrivateDOMEvent : public nsISupports {

public:
  static const nsIID& GetIID() { static nsIID iid = NS_IPRIVATEDOMEVENT_IID; return iid; }

  NS_IMETHOD DuplicatePrivateData() = 0;
  NS_IMETHOD SetTarget(nsIDOMNode* aNode) = 0;
};

extern nsresult NS_NewDOMEvent(nsIDOMEvent** aInstancePtrResult, nsIPresContext& aPresContext, nsEvent *aEvent);
extern nsresult NS_NewDOMUIEvent(nsIDOMEvent** aInstancePtrResult, nsIPresContext& aPresContext, nsEvent *aEvent);

#endif // nsIPrivateDOMEvent_h__
