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
#ifndef nsTextContentChangeData_h___
#define nsTextContentChangeData_h___

#include "nsITextContent.h"

class nsTextContentChangeData : public nsITextContentChangeData {
public:
  friend nsresult
    NS_NewTextContentChangeData(nsTextContentChangeData** aResult);

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsITextContentChangedData
  NS_IMETHOD GetChangeType(ChangeType* aResult);

  NS_IMETHOD GetReplaceData(PRInt32* aOffset,
                            PRInt32* aSourceLength,
                            PRInt32* aReplaceLength);

  NS_IMETHOD GetInsertData(PRInt32* aOffset,
                           PRInt32* aInsertLength);

  NS_IMETHOD GetAppendData(PRInt32* aOffset,
                           PRInt32* aAppendLength);

  void SetData(ChangeType aType, PRInt32 aOffset, PRInt32 aLength) {
    mType = aType;
    mOffset = aOffset;
    mLength = aLength;
  }

  void SetReplaceLength(PRInt32 aReplaceLength) {
    mReplaceLength = aReplaceLength;
  }

protected:
  nsTextContentChangeData();
  virtual ~nsTextContentChangeData();

  ChangeType mType;
  PRInt32 mOffset;
  PRInt32 mLength;
  PRInt32 mReplaceLength;               // only used for replace type
};

// Create a new instance of nsTextContentChangeData with a refcnt of 1
extern nsresult
NS_NewTextContentChangeData(nsTextContentChangeData** aResult);

#endif /* nsTextContentChangeData_h___ */
