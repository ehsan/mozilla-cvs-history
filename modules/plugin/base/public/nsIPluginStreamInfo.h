/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifndef nsIPluginStreamInfo_h___
#define nsIPluginStreamInfo_h___

#include "nsplugindefs.h"
#include "nsISupports.h"

/**
 * nsIPluginStreamInfo
 *
 * @status DEPRECATED
 *
 * Originally published XPCOM Plugin API is now deprecated
 * Developers are welcome to use NPAPI, please refer to:
 * http://mozilla.org/projects/plugins/
 */

#define NS_IPLUGINSTREAMINFO_IID                     \
{ /* ed7d4ca0-b005-11d2-adaa-00805f6dec49 */         \
    0xed7d4ca0,                                      \
    0xb005,                                          \
    0x11d2,                                          \
    {0xad, 0xaa, 0x00, 0x80, 0x5f, 0x6d, 0xec, 0x49} \
}

class nsIPluginStreamInfo : public nsISupports {
public:

	NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPLUGINSTREAMINFO_IID)

	NS_IMETHOD
	GetContentType(nsMIMEType* result) = 0;

	NS_IMETHOD
	IsSeekable(PRBool* result) = 0;

	NS_IMETHOD
	GetLength(PRUint32* result) = 0;

	NS_IMETHOD
	GetLastModified(PRUint32* result) = 0;

	NS_IMETHOD
	GetURL(const char** result) = 0;

	NS_IMETHOD
	RequestRead(nsByteRange* rangeList) = 0;

        NS_IMETHOD
        GetStreamOffset(PRInt32 *aStreamOffset) = 0;

        NS_IMETHOD
        SetStreamOffset(PRInt32 aStreamOffset) = 0;
};

////////////////////////////////////////////////////////////////////////////////

#endif /* nsIPluginStreamInfo_h___ */
