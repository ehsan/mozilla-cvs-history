/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#ifndef nsIRDFXMLSource_h__
#define nsIRDFXMLSource_h__

class nsIOutputStream;

// {4DA56F10-99FE-11d2-8EBB-00805F29F370}
#define NS_IRDFXMLSOURCE_IID \
{ 0x4da56f10, 0x99fe, 0x11d2, { 0x8e, 0xbb, 0x0, 0x80, 0x5f, 0x29, 0xf3, 0x70 } }


class nsIRDFXMLSource : public nsISupports
{
public:
    NS_IMETHOD Serialize(nsIOutputStream* stream) = 0;
};

#endif // nsIRDFXMLSource_h__
