/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef nsFileStream_h__
#define nsFileStream_h__

//#include "nsISupports.h"
#include "nsStream.h"
#include "xp.h" // Reqd. for xp_file.h
#include "xp_file.h" // Cuz we don't have a better choice, as yet

class nsFileStream: public nsStream
{

public:
            nsFileStream(XP_File* i_pFile);
    virtual ~nsFileStream();
/*
    NS_IMETHOD              QueryInterface(const nsIID& aIID, 
                                           void** aInstancePtr);
    NS_IMETHOD_(nsrefcnt)   AddRef(void);
    NS_IMETHOD_(nsrefcnt)   Release(void);
*/
    PRInt32     Read(void* o_Buffer, PRUint32 i_Len);
    PRInt32     Write(const void* i_Buffer, PRUint32 i_Len);

protected:

private:
    nsFileStream(const nsFileStream& o);
    nsFileStream& operator=(const nsFileStream& o);
    XP_File* m_pFile;
};

#endif // nsFileStream_h__

