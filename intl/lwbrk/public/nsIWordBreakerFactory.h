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
#ifndef nsIWordBreakerFactory_h__
#define nsIWordBreakerFactory_h__


#include "nsISupports.h"
#include "nsIWordBreaker.h"
#include "nsString.h"

#include "nscore.h"

// {E86B337A-BF89-11d2-B3AF-00805F8A6670}
#define NS_IWORDBREAKERFACTORY_IID \
{ 0xe86b337a, 0xbf89, 0x11d2, \
    { 0xb3, 0xaf, 0x0, 0x80, 0x5f, 0x8a, 0x66, 0x70 } };


class nsIWordBreakerFactory : public nsISupports
{
public:
  NS_IMETHOD GetBreaker(nsString& aParam, nsIWordBreaker** breaker) = 0;
};


#endif  /* nsIWordBreakerFactory_h__ */
