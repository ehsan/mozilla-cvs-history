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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Peter Van der Beken <peterv@netscape.com>
 *
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

#ifndef nsIAttribute_h___
#define nsIAttribute_h___

#include "nsISupports.h"
#include "nsINodeInfo.h"

class nsIContent;

#define NS_IATTRIBUTE_IID  \
 {0xa6cf90dd, 0x15b3, 0x11d2,        \
 {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32}}

class nsIAttribute : public nsISupports
{
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IATTRIBUTE_IID)

  void SetContent(nsIContent* aContent)
  {
    mContent = aContent;
  }

  nsIContent *GetContent()
  {
    return mContent;
  }

  nsINodeInfo *NodeInfo()
  {
    return mNodeInfo;
  }

protected:
  nsIAttribute(nsIContent *aContent, nsINodeInfo *aNodeInfo)
    : mContent(aContent), mNodeInfo(aNodeInfo)
  {
  }

  nsIContent *mContent; // WEAK
  nsCOMPtr<nsINodeInfo> mNodeInfo; // STRONG

private:
  nsIAttribute(); // Not to be implemented.
};

#endif /* nsIAttribute_h___ */
