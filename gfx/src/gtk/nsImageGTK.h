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

#ifndef nsImageGTK_h___
#define nsImageGTK_h___

#include "nsIImage.h"

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include <gdk/gdk.h>

#undef Bool

class nsImageGTK : public nsIImage
{
public:
  nsImageGTK();
  virtual ~nsImageGTK();

  NS_DECL_ISUPPORTS

  /**
  @see nsIImage.h
  */
  virtual PRInt32     GetBytesPix()       { return 0; }  // XXXX this need to be filled in
  virtual PRInt32     GetHeight()         { return mHeight; }
  virtual PRInt32     GetWidth()          { return mWidth; }
  virtual PRUint8*    GetBits()           { return mImageBits; }
  virtual void*       GetBitInfo()        { return nsnull; }
  virtual PRInt32     GetLineStride()     { return mRowBytes; }
  virtual nsColorMap* GetColorMap()       { return nsnull; }
  NS_IMETHOD Draw(nsIRenderingContext &aContext, nsDrawingSurface aSurface, PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);
  NS_IMETHOD Draw(nsIRenderingContext &aContext, nsDrawingSurface aSurface, PRInt32 aSX, PRInt32 aSY, PRInt32 aSWidth, PRInt32 aSHeight,
                  PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight);
  virtual void ImageUpdated(nsIDeviceContext *aContext, PRUint8 aFlags, nsRect *aUpdateRect);
  virtual nsresult    Init(PRInt32 aWidth, PRInt32 aHeight, PRInt32 aDepth, nsMaskRequirements aMaskRequirements);
  virtual PRBool      IsOptimized()       { return PR_TRUE; }

  virtual nsresult    Optimize(nsIDeviceContext* aContext);
  virtual PRUint8*    GetAlphaBits()      { return mAlphaBits; }
  virtual PRInt32     GetAlphaWidth()     { return mAlphaWidth; }
  virtual PRInt32     GetAlphaHeight()    { return mAlphaHeight; }
  virtual PRInt32     GetAlphaXLoc()      { return 0; }
  virtual PRInt32     GetAlphaYLoc()      { return 0; }
  virtual PRInt32     GetAlphaLineStride(){ return mAlphaRowBytes; }
  virtual void        CompositeImage(nsIImage *aTheImage,nsPoint *aULLocation,nsBlendQuality aQuality);
  virtual nsIImage*   DuplicateImage() {return(nsnull);}

  /**
   * Calculate the number of bytes spaned for this image for a given width
   * @param aWidth is the width to calculate the number of bytes for
   * @return the number of bytes in this span
   */
  PRInt32  CalcBytesSpan(PRUint32  aWidth);
  PRBool  SetAlphaMask(nsIImage *aTheMask);
  virtual void  SetAlphaLevel(PRInt32 /* aAlphaLevel */) {}
  virtual PRInt32 GetAlphaLevel() {return(0);}
  virtual void  MoveAlphaMask(PRInt32 /* aX */, PRInt32 /* aY */) {}

private:
  /**
   * Calculate the amount of memory needed for the initialization of the image
   */
  void ComputMetrics();
  void ComputePaletteSize(PRIntn nBitCount);


private:
  PRInt32    mWidth;
  PRInt32    mHeight;
  PRInt32    mDepth;       // bits per pixel
  PRInt32    mRowBytes;
  PRUint8    *mImageBits;
  PRUint8    *mConvertedBits;
  PRInt32    mSizeImage;

  PRInt8      mNumBytesPixel;

 // alpha layer members
  PRUint8    *mAlphaBits;
  GdkPixmap  *mAlphaPixmap;
  PRInt8     mAlphaDepth;        // alpha layer depth
  PRInt16    mAlphaRowBytes;     // alpha bytes per row
  PRInt16    mAlphaWidth;        // alpha layer width
  PRInt16    mAlphaHeight;       // alpha layer height
  nsPoint    mLocation;          // alpha mask location


};

#endif
