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

#include "nsRegionMac.h"

static NS_DEFINE_IID(kRegionIID, NS_IREGION_IID);

//---------------------------------------------------------------------

nsRegionMac :: nsRegionMac()
{
  NS_INIT_REFCNT();
	mRegion = nsnull;
  mRegionType = eRegionComplexity_empty;
}

//---------------------------------------------------------------------

nsRegionMac :: ~nsRegionMac()
{
  if (mRegion)
    ::DisposeRgn(mRegion);
  mRegion = nsnull;
}

NS_IMPL_QUERY_INTERFACE(nsRegionMac, kRegionIID)
NS_IMPL_ADDREF(nsRegionMac)
NS_IMPL_RELEASE(nsRegionMac)

//---------------------------------------------------------------------

nsresult nsRegionMac :: Init(void)
{
	mRegion = ::NewRgn();
  mRegionType = eRegionComplexity_empty;
  return NS_OK;
}

//---------------------------------------------------------------------

void nsRegionMac :: SetTo(const nsIRegion &aRegion)
{
	nsRegionMac* pRegion = (nsRegionMac*)&aRegion;
	::CopyRgn(pRegion->mRegion, mRegion);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: SetTo(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
	::SetRectRgn(mRegion, aX, aY, aX + aWidth, aY + aHeight);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: Intersect(const nsIRegion &aRegion)
{
  nsRegionMac* pRegion = (nsRegionMac*)&aRegion;
  ::SectRgn(mRegion, pRegion->mRegion, mRegion);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: Intersect(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
	RgnHandle rectRgn = ::NewRgn();
	::SetRectRgn(rectRgn, aX, aY, aX + aWidth, aY + aHeight);
  ::SectRgn(mRegion, rectRgn, mRegion);
  ::DisposeRgn(rectRgn);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: Union(const nsIRegion &aRegion)
{
	nsRegionMac* pRegion = (nsRegionMac*)&aRegion;
  ::UnionRgn(mRegion, pRegion->mRegion, mRegion);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: Union(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
	RgnHandle rectRgn = ::NewRgn();
	::SetRectRgn(rectRgn, aX, aY, aX + aWidth, aY + aHeight);
  ::UnionRgn(mRegion, rectRgn, mRegion);
  ::DisposeRgn(rectRgn);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: Subtract(const nsIRegion &aRegion)
{
	nsRegionMac* pRegion = (nsRegionMac*)&aRegion;
  ::DiffRgn(mRegion, pRegion->mRegion, mRegion);
  SetRegionType();
}

//---------------------------------------------------------------------

void nsRegionMac :: Subtract(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
	RgnHandle rectRgn = ::NewRgn();
	::SetRectRgn(rectRgn, aX, aY, aX + aWidth, aY + aHeight);
  ::DiffRgn(mRegion, rectRgn, mRegion);
  ::DisposeRgn(rectRgn);
  SetRegionType();
}

//---------------------------------------------------------------------

PRBool nsRegionMac :: IsEmpty(void)
{
  if (mRegionType == eRegionComplexity_empty)
    return PR_TRUE;
	else
  	return PR_FALSE;
}

//---------------------------------------------------------------------

PRBool nsRegionMac :: IsEqual(const nsIRegion &aRegion)
{
  nsRegionMac* pRegion = (nsRegionMac*)&aRegion;
  return(::EqualRgn(mRegion, pRegion->mRegion));
}

//---------------------------------------------------------------------

void nsRegionMac :: GetBoundingBox(PRInt32 *aX, PRInt32 *aY, PRInt32 *aWidth, PRInt32 *aHeight)
{
  Rect macRect = (**mRegion).rgnBBox;

  *aX = macRect.left;
  *aY = macRect.top;
  *aWidth  = macRect.right - macRect.left;
  *aHeight = macRect.bottom - macRect.top;
}

//---------------------------------------------------------------------

void nsRegionMac :: Offset(PRInt32 aXOffset, PRInt32 aYOffset)
{
  ::OffsetRgn(mRegion, aXOffset, aYOffset);
}

//---------------------------------------------------------------------

PRBool nsRegionMac :: ContainsRect(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
	Rect macRect;
	::SetRect(&macRect, aX, aY, aX + aWidth, aY + aHeight);
	return(::RectInRgn(&macRect, mRegion));
}

//---------------------------------------------------------------------

PRBool nsRegionMac :: ForEachRect(nsRectInRegionFunc *func, void *closure)
{
  return PR_FALSE;
}

//---------------------------------------------------------------------


NS_IMETHODIMP nsRegionMac :: GetNativeRegion(void *&aRegion) const
{
  aRegion = (void *)mRegion;
  return NS_OK;
}

//---------------------------------------------------------------------

NS_IMETHODIMP nsRegionMac :: GetRegionComplexity(nsRegionComplexity &aComplexity) const
{
  aComplexity = mRegionType;
  return NS_OK;
}

//---------------------------------------------------------------------


void nsRegionMac :: SetRegionType()
{
  if (::EmptyRgn(mRegion) == PR_TRUE)
    mRegionType = eRegionComplexity_empty;
  else
    mRegionType = eRegionComplexity_rect;
}


//---------------------------------------------------------------------


void nsRegionMac :: SetRegionEmpty()
{
  ::SetEmptyRgn(mRegion);
}

//---------------------------------------------------------------------


RgnHandle nsRegionMac :: CreateRectRegion(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
	RgnHandle rectRgn = ::NewRgn();
	::SetRectRgn(rectRgn, aX, aY, aX + aWidth, aY + aHeight);
  return rectRgn;
}








