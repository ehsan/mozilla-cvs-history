/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */

#ifndef nsUCvTW2CID_h___
#define nsUCvTW2CID_h___

#include "nsISupports.h"

// Class ID for our EUCTWToUnicode charset converter
// {379C2771-EC77-11d2-8AAC-00600811A836}
NS_DECLARE_ID(kEUCTWToUnicodeCID,
  0x379c2771, 0xec77, 0x11d2, 0x8a, 0xac, 0x0, 0x60, 0x8, 0x11, 0xa8, 0x36);

// Class ID for our UnicodeToEUCTW charset converter
// {379C2776-EC77-11d2-8AAC-00600811A836}
NS_DECLARE_ID(kUnicodeToEUCTWCID, 
  0x379c2776, 0xec77, 0x11d2, 0x8a, 0xac, 0x0, 0x60, 0x8, 0x11, 0xa8, 0x36);


// Class ID for our UnicodeToCNS11643p1 charset converter
// {BA615197-1DFA-11d3-B3BF-00805F8A6670}
NS_DECLARE_ID(kUnicodeToCNS11643p1CID, 
0xba615197, 0x1dfa, 0x11d3, 0xb3, 0xbf, 0x0, 0x80, 0x5f, 0x8a, 0x66, 0x70);

// Class ID for our UnicodeToCNS11643p2 charset converter
// {BA615198-1DFA-11d3-B3BF-00805F8A6670}
NS_DECLARE_ID(kUnicodeToCNS11643p2CID, 
0xba615198, 0x1dfa, 0x11d3, 0xb3, 0xbf, 0x0, 0x80, 0x5f, 0x8a, 0x66, 0x70);

#endif /* nsUCvTW2CID_h___ */
