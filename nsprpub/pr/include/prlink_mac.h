/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "NPL"); you may not use this file except in
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


#ifndef prlink_mac_h___
#define prlink_mac_h___

#ifdef XP_MAC

#include <Files.h>
#include "prtypes.h"

PR_BEGIN_EXTERN_C

/*
** Doc me!
*/
PR_EXTERN(PRLibrary*) PR_LoadNamedFragment(const FSSpec *fileSpec, const char* fragmentName);

/*
** Doc me!
*/
PR_EXTERN(PRLibrary*) PR_LoadIndexedFragment(const FSSpec *fileSpec, PRUint32 fragIndex);

PR_END_EXTERN_C

#endif


#endif /* prlink_mac_h___ */
