/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "nsDragService.h"
#include "nsITransferable.h"
#include "nsDataObj.h"

#include "nsWidgetsCID.h"
#include "nsNativeDragTarget.h"
#include "nsNativeDragSource.h"
#include "nsClipboard.h"

#include <OLE2.h>
#include "OLEIDL.H"


static NS_DEFINE_IID(kIDragServiceIID,   NS_IDRAGSERVICE_IID);
//static NS_DEFINE_IID(kIClipboardIID,     NS_ICLIPBOARD_IID);

//static NS_DEFINE_CID(kCClipboardCID,     NS_CLIPBOARD_CID);

NS_IMPL_ADDREF_INHERITED(nsDragService, nsBaseDragService)
NS_IMPL_RELEASE_INHERITED(nsDragService, nsBaseDragService)

//-------------------------------------------------------------------------
//
// DragService constructor
//
//-------------------------------------------------------------------------
nsDragService::nsDragService()
{
  NS_INIT_REFCNT();
  mNativeDragTarget = nsnull;
  mNativeDragSrc    = nsnull;
  mDataObject       = nsnull;
}

//-------------------------------------------------------------------------
//
// DragService destructor
//
//-------------------------------------------------------------------------
nsDragService::~nsDragService()
{
  NS_IF_RELEASE(mNativeDragSrc);
  NS_IF_RELEASE(mNativeDragTarget);
  NS_IF_RELEASE(mDataObject);
}

/**
 * @param aIID The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. NS_ISUPPORTS_IID)
 * 
*/ 
nsresult nsDragService::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{

  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }

  nsresult rv = NS_NOINTERFACE;

  if (aIID.Equals(kIDragServiceIID)) {
    *aInstancePtr = (void*) ((nsIDragService*)this);
    NS_ADDREF_THIS();
    return NS_OK;
  }

  return rv;
}

//---------------------------------------------------------
NS_IMETHODIMP nsDragService::StartDragSession (nsITransferable * aTransferable, PRUint32 aActionType)

{

  // To do the drag we need to create an object that 
  // mplements the IDataObject interface (for OLE)
  //
  // We start by getting the clipboard object,
  // casting it our known class and using utility

  NS_IF_RELEASE(mNativeDragSrc);
  mNativeDragSrc = (IDropSource *)new nsNativeDragSource();
  if (nsnull != mNativeDragSrc) {
    mNativeDragSrc->AddRef();
  }

  mTransferable = dont_QueryInterface(aTransferable);

  IDataObject * dataObj;
  nsClipboard::CreateNativeDataObject(mTransferable, &dataObj);

  nsresult result = NS_ERROR_FAILURE;
  DWORD dropRes;
  DWORD effects = DROPEFFECT_SCROLL;
  if (aActionType & DRAGDROP_ACTION_COPY) {
    effects |= DROPEFFECT_COPY;
  }
  if (aActionType & DRAGDROP_ACTION_MOVE) {
    effects |= DROPEFFECT_MOVE;
  }
  if (aActionType & DRAGDROP_ACTION_LINK) {
    effects |= DROPEFFECT_LINK;
  }

  HRESULT res = ::DoDragDrop(dataObj, mNativeDragSrc, effects, &dropRes);

  return (DRAGDROP_S_DROP == res?NS_OK:NS_ERROR_FAILURE);
}

//-------------------------------------------------------------------------
NS_IMETHODIMP nsDragService::GetData (nsITransferable * aTransferable)
{
  if (nsnull != mDataObject) {
    return nsClipboard::GetDataFromDataObject(mDataObject, nsnull, aTransferable);
  }
  return NS_ERROR_FAILURE;
}

//---------------------------------------------------------
NS_IMETHODIMP nsDragService::SetIDataObject (IDataObject * aDataObj)
{
  NS_IF_RELEASE(mDataObject);
  mDataObject = aDataObj;
  NS_ADDREF(mDataObject);
  return NS_OK;
}
