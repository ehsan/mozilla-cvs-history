/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/********************************************************************************************************
 
   Interface for representing Address Book Directory
 
*********************************************************************************************************/

#ifndef nsAbDirProperty_h__
#define nsAbDirProperty_h__

#include "nsIAbDirectory.h" /* include the interface we are going to support */
#include "nsIAbCard.h"
#include "nsISupportsArray.h"
#include "nsCOMPtr.h"
#include "nsDirPrefs.h"
#include "nsIAddrDatabase.h"

 /* 
  * Address Book Directory
  */ 

class nsAbDirProperty: public nsIAbDirectory
{
public: 
	nsAbDirProperty(void);
	virtual ~nsAbDirProperty(void);

	NS_DECL_ISUPPORTS
	NS_DECL_NSIABDIRECTORY

protected:

	nsresult GetAttributeName(PRUnichar **aName, nsString& value);
	nsresult SetAttributeName(const PRUnichar *aName, nsString& arrtibute);


	nsString m_DirName;
	PRUint32 m_LastModifiedDate;

	nsString m_ListName;
	nsString m_ListNickName;
	nsString m_Description;
	PRBool   m_bIsMailList;

	nsCOMPtr<nsISupportsArray> m_AddressList;

};

#endif
