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

#include <math.h>

#include "nspr.h"
#include "il_util.h"

#include "nsDeviceContextGTK.h"
#include "../nsGfxCIID.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kDeviceContextIID, NS_IDEVICE_CONTEXT_IID);

typedef unsigned char BYTE;

nsDeviceContextGTK::nsDeviceContextGTK()
{
  NS_INIT_REFCNT();
  mTwipsToPixels = 1.0;
  mPixelsToTwips = 1.0;
  mRedMask = 0;
  mGreenMask = 0;
  mBlueMask = 0;
  mRedBits = 0;
  mGreenBits = 0;
  mBlueBits = 0;
  mRedOffset = 0;
  mGreenOffset = 0;
  mBlueOffset = 0;
  mDepth = 0 ;
  mPaletteInfo.isPaletteDevice = PR_FALSE;
  mPaletteInfo.sizePalette = 0;
  mPaletteInfo.numReserved = 0;
  mPaletteInfo.palette = NULL;
  mColormap = nsnull;
  mNumCells = 0;
}

nsDeviceContextGTK::~nsDeviceContextGTK()
{
  if (mColormap) 
    {
      gdk_colormap_unref(mColormap);
      mColormap = nsnull;
    }
}

NS_IMPL_QUERY_INTERFACE(nsDeviceContextGTK, kDeviceContextIID)
NS_IMPL_ADDREF(nsDeviceContextGTK)
NS_IMPL_RELEASE(nsDeviceContextGTK)

NS_IMETHODIMP nsDeviceContextGTK::Init(nsNativeWidget aNativeWidget)
{
  GdkVisual *vis;

  for (PRInt32 cnt = 0; cnt < 256; cnt++)
    mGammaTable[cnt] = cnt;
  
  mWidget = aNativeWidget;

// this is used for something odd.  who knows
  mTwipsToPixels = (((float)::gdk_screen_width()) /
                    ((float)::gdk_screen_width_mm()) * 25.4) /
		     (float)NSIntPointsToTwips(72);

  mPixelsToTwips = 1.0f / mTwipsToPixels;

  vis = gdk_rgb_get_visual();
  mRedMask = vis->red_mask;
  mGreenMask = vis->green_mask;
  mBlueMask = vis->blue_mask;
  mRedBits = vis->bits_per_rgb;
  mGreenBits = vis->bits_per_rgb;
  mBlueBits = vis->bits_per_rgb;
  mRedOffset = vis->red_shift;
  mGreenOffset = vis->green_shift;
  mBlueOffset = vis->blue_shift;
  mDepth = vis->depth;

  return NS_OK;
}

NS_IMETHODIMP nsDeviceContextGTK::CreateRenderingContext(nsIRenderingContext *&aContext)
{
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDeviceContextGTK::SupportsNativeWidgets(PRBool &aSupportsWidgets)
{
  //XXX it is very critical that this not lie!! MMP
  // read the comments in the mac code for this
  aSupportsWidgets = PR_TRUE;

  return NS_OK;
}

NS_IMETHODIMP nsDeviceContextGTK::GetScrollBarDimensions(float &aWidth, float &aHeight) const
{
/* 11 + 2 + 2   (11 is the default scrollbar size, 2 is the y offset from the
		 style code.  we should get this directly */
  aWidth = 15.0 * mPixelsToTwips;
  aHeight = 15.0 * mPixelsToTwips;
//  aHeight = 24.0 * mPixelsToTwips;
//  aWidth = 11.0 * mTwipsToPixels;
//  aHeight = 24.0 * mTwipsToPixels;
  return NS_OK;
}

NS_IMETHODIMP nsDeviceContextGTK::GetDrawingSurface(nsIRenderingContext &aContext, 
                                                    nsDrawingSurface &aSurface)
{
  aContext.CreateDrawingSurface(nsnull, 0, aSurface);
  return nsnull == aSurface ? NS_ERROR_OUT_OF_MEMORY : NS_OK;  
}

NS_IMETHODIMP nsDeviceContextGTK::ConvertPixel(nscolor aColor, 
                                               PRUint32 & aPixel)
{
  aPixel = ::gdk_rgb_xpixel_from_rgb ((aColor & 0xff) << 16 |
                                      (aColor & 0xff00) |
                                      ((aColor >> 16) & 0xff));

  return NS_OK;
}


NS_IMETHODIMP nsDeviceContextGTK::CheckFontExistence(const nsString& aFontName)
{
  char        **fnames = nsnull;
  PRInt32     namelen = aFontName.Length() + 1;
  char        *wildstring = (char *)PR_Malloc(namelen + 200);
  float       t2d;
  GetTwipsToDevUnits(t2d);
  PRInt32     dpi = NSToIntRound(t2d * 1440);
  int         numnames = 0;
  XFontStruct *fonts;
  nsresult    rv = NS_ERROR_FAILURE;
  
  if (nsnull == wildstring)
    return NS_ERROR_UNEXPECTED;
  
  if (abs(dpi - 75) < abs(dpi - 100))
    dpi = 75;
  else
    dpi = 100;
  
  char* fontName = aFontName.ToNewCString();
  PR_snprintf(wildstring, namelen + 200,
             "*-%s-*-*-normal--*-*-%d-%d-*-*-*",
             fontName, dpi, dpi);
  delete [] fontName;
  
  fnames = ::XListFontsWithInfo(GDK_DISPLAY(), wildstring, 1, &numnames, &fonts);
  
  if (numnames > 0)
  {
    ::XFreeFontInfo(fnames, fonts, numnames);
    rv = NS_OK;
  }
  
  PR_Free(wildstring);
  
  return rv;
}

NS_IMETHODIMP nsDeviceContextGTK::GetDeviceSurfaceDimensions(PRInt32 &aWidth, PRInt32 &aHeight)
{
  aWidth = 1;
  aHeight = 1;

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDeviceContextGTK::GetDeviceContextFor(nsIDeviceContextSpec *aDevice,
                                                      nsIDeviceContext *&aContext)
{
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDeviceContextGTK::BeginDocument(void)
{
  return NS_OK;
}

NS_IMETHODIMP nsDeviceContextGTK::EndDocument(void)
{
  return NS_OK;
}

NS_IMETHODIMP nsDeviceContextGTK::BeginPage(void)
{
  return NS_OK;
}

NS_IMETHODIMP nsDeviceContextGTK::EndPage(void)
{
  return NS_OK;
}
