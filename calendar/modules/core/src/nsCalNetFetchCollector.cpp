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
#include <stdio.h>
#include "nscore.h"
#include "nsCoreCIID.h"
#include "nsxpfcCIID.h"

static PRInt32 gsiID = 0;

PRInt32 nsCalNetFetchCollector::GetNextID()
{
  return gsiID++;
}

nsCalNetFetchCollector::nsCalNetFetchCollector(nsISupports* outer)
{
  NS_INIT_REFCNT();
  mpFetchList = nsnull;
}

// is this needed for a singleton?
NS_IMPL_ADDREF(nsCalNetFetchCollector)
NS_IMPL_RELEASE(nsCalNetFetchCollector)

nsCalNetFetchCollector::~nsCalNetFetchCollector()
{
	/*
	 * destroy anything in the list...
	 */
	NS_IF_RELEASE(mpFetchList);
}

nsresult nsCalNetFetchCollector::Init()
{
  static NS_DEFINE_IID(kCVectorCID, NS_ARRAY_CID);

  nsresult res = nsRepository::CreateInstance(kCVectorCID, nsnull, kCVectorCID, (void **)&mpFetchList);
  if (NS_OK != res)
    return res ;
  mpFetchList->Init();
  return (NS_OK);
}

nsresult nsCalNetFetchCollector::QueueFetchByRange(nsIUser* pUser, nsILayer* pLayer, DateTime d1, DateTime d2)
{
  return (NS_OK);
}

nsresult nsCalNetFetchCollector::FlushFetchByRange(PRInt32* pID)
{
  return (NS_OK);
}

nsresult nsCalNetFetchCollector::SetPriority(PRInt32 id, PRInt32 iPri)
{
  return (NS_OK);
}

nsresult nsCalNetFetchCollector::GetState(PRInt32 ID, eCalNetFetchState *pState)
{
  return (NS_OK);
}

nsresult nsCalNetFetchCollector::Cancel(nsILayer * aLayer)
{
  return (NS_OK);
}


