/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
/* 
 * DO NOT EDIT THIS DOCUMENT MANUALLY !!!
 * THIS FILE IS AUTOMATICALLY GENERATED BY THE TOOLS UNDER
 *    mozilla/intl/chardet/tools/
 * Please contact ftang@netscape.com or mozilla-i18n@mozilla.org
 * if you have any question. Thanks
 */
#include "nsVerifier.h"
static PRUint32 SJIS_cls [ 256 / 8 ] = {
PCK4BITS(0,1,1,1,1,1,1,1),  // 00 - 07 
PCK4BITS(1,1,1,1,1,1,1,1),  // 08 - 0f 
PCK4BITS(1,1,1,1,1,1,1,1),  // 10 - 17 
PCK4BITS(1,1,1,0,1,1,1,1),  // 18 - 1f 
PCK4BITS(1,1,1,1,1,1,1,1),  // 20 - 27 
PCK4BITS(1,1,1,1,1,1,1,1),  // 28 - 2f 
PCK4BITS(1,1,1,1,1,1,1,1),  // 30 - 37 
PCK4BITS(1,1,1,1,1,1,1,1),  // 38 - 3f 
PCK4BITS(2,2,2,2,2,2,2,2),  // 40 - 47 
PCK4BITS(2,2,2,2,2,2,2,2),  // 48 - 4f 
PCK4BITS(2,2,2,2,2,2,2,2),  // 50 - 57 
PCK4BITS(2,2,2,2,2,2,2,2),  // 58 - 5f 
PCK4BITS(2,2,2,2,2,2,2,2),  // 60 - 67 
PCK4BITS(2,2,2,2,2,2,2,2),  // 68 - 6f 
PCK4BITS(2,2,2,2,2,2,2,2),  // 70 - 77 
PCK4BITS(2,2,2,2,2,2,2,1),  // 78 - 7f 
PCK4BITS(3,3,3,3,3,3,3,3),  // 80 - 87 
PCK4BITS(3,3,3,3,3,3,3,3),  // 88 - 8f 
PCK4BITS(3,3,3,3,3,3,3,3),  // 90 - 97 
PCK4BITS(3,3,3,3,3,3,3,3),  // 98 - 9f 
PCK4BITS(4,2,2,2,2,2,2,2),  // a0 - a7 
PCK4BITS(2,2,2,2,2,2,2,2),  // a8 - af 
PCK4BITS(2,2,2,2,2,2,2,2),  // b0 - b7 
PCK4BITS(2,2,2,2,2,2,2,2),  // b8 - bf 
PCK4BITS(2,2,2,2,2,2,2,2),  // c0 - c7 
PCK4BITS(2,2,2,2,2,2,2,2),  // c8 - cf 
PCK4BITS(2,2,2,2,2,2,2,2),  // d0 - d7 
PCK4BITS(2,2,2,2,2,2,2,2),  // d8 - df 
PCK4BITS(5,5,5,5,5,5,5,5),  // e0 - e7 
PCK4BITS(5,5,5,5,5,5,5,5),  // e8 - ef 
PCK4BITS(5,5,5,5,5,5,5,5),  // f0 - f7 
PCK4BITS(5,5,5,5,5,0,0,0)   // f8 - ff 
};


static PRUint32 SJIS_st [ 3] = {
PCK4BITS(eError,eStart,eStart,     3,eError,     3,eError,eError),//00-07 
PCK4BITS(eError,eError,eError,eError,eItsMe,eItsMe,eItsMe,eItsMe),//08-0f 
PCK4BITS(eItsMe,eItsMe,eError,eError,eStart,eError,eStart,eStart) //10-17 
};


static nsVerifier nsSJISVerifier = {
     "Shift_JIS",
    {
       eIdxSft4bits, 
       eSftMsk4bits, 
       eBitSft4bits, 
       eUnitMsk4bits, 
       SJIS_cls 
    },
    6,
    {
       eIdxSft4bits, 
       eSftMsk4bits, 
       eBitSft4bits, 
       eUnitMsk4bits, 
       SJIS_st 
    }
};
