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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsSVGPathGeometryFrame.h"
#include "nsIDOMSVGAnimatedLength.h"
#include "nsIDOMSVGLength.h"
#include "nsIDOMSVGImageElement.h"
#include "nsIDOMSVGElement.h"
#include "nsISVGRendererPathBuilder.h"
#include "nsISVGRendererSurface.h"
#include "nsISVGRendererCanvas.h"
#include "nsISVGRenderer.h"
#include "nsIDOMSVGMatrix.h"
#include "nsIDOMSVGAnimPresAspRatio.h"
#include "nsIDOMSVGPresAspectRatio.h"
#include "imgIDecoderObserver.h"
#include "imgIContainerObserver.h"
#include "nsIImageLoadingContent.h"
#include "imgIContainer.h"
#include "gfxIImageFrame.h"
#include "imgIRequest.h"

#define NS_GET_BIT(rowptr, x) (rowptr[(x)>>3] &  (1<<(7-(x)&0x7)))

class nsSVGImageFrame;

class nsSVGImageListener : public imgIDecoderObserver
{
public:
  nsSVGImageListener(nsSVGImageFrame *aFrame);
  virtual ~nsSVGImageListener();

  NS_DECL_ISUPPORTS
  NS_DECL_IMGIDECODEROBSERVER
  NS_DECL_IMGICONTAINEROBSERVER

  void SetFrame(nsSVGImageFrame *frame) { mFrame = frame; }

private:
  nsSVGImageFrame *mFrame;
};


class nsSVGImageFrame : public nsSVGPathGeometryFrame
{
protected:
  friend nsresult
  NS_NewSVGImageFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsIFrame** aNewFrame);

  virtual ~nsSVGImageFrame();
  virtual nsresult Init();

public:
  // nsISVGValueObserver interface:
  NS_IMETHOD DidModifySVGObservable(nsISVGValue* observable);

  // nsISVGPathGeometrySource interface:
  NS_IMETHOD ConstructPath(nsISVGRendererPathBuilder *pathBuilder);

  // nsISVGChildFrame interface:
  NS_IMETHOD Paint(nsISVGRendererCanvas* canvas, const nsRect& dirtyRectTwips);

private:
  nsCOMPtr<nsIDOMSVGLength> mX;
  nsCOMPtr<nsIDOMSVGLength> mY;
  nsCOMPtr<nsIDOMSVGLength> mWidth;
  nsCOMPtr<nsIDOMSVGLength> mHeight;
  nsCOMPtr<nsIDOMSVGPreserveAspectRatio> mPreserveAspectRatio;

  nsCOMPtr<imgIDecoderObserver> mListener;
  nsCOMPtr<nsISVGRendererSurface> mSurface;

  nsresult ConvertFrame(gfxIImageFrame *aNewFrame);

  friend class nsSVGImageListener;
  PRBool mSurfaceInvalid;
};

//----------------------------------------------------------------------
// Implementation

nsresult
NS_NewSVGImageFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsIFrame** aNewFrame)
{
  *aNewFrame = nsnull;

  nsCOMPtr<nsIDOMSVGImageElement> Rect = do_QueryInterface(aContent);
  if (!Rect) {
#ifdef DEBUG
    printf("warning: trying to construct an SVGImageFrame for a content element that doesn't support the right interfaces\n");
#endif
    return NS_ERROR_FAILURE;
  }

  nsSVGImageFrame* it = new (aPresShell) nsSVGImageFrame;
  if (nsnull == it)
    return NS_ERROR_OUT_OF_MEMORY;

  *aNewFrame = it;
  return NS_OK;
}

nsSVGImageFrame::~nsSVGImageFrame()
{
  nsCOMPtr<nsISVGValue> value;
  if (mX && (value = do_QueryInterface(mX)))
      value->RemoveObserver(this);
  if (mY && (value = do_QueryInterface(mY)))
      value->RemoveObserver(this);
  if (mWidth && (value = do_QueryInterface(mWidth)))
      value->RemoveObserver(this);
  if (mHeight && (value = do_QueryInterface(mHeight)))
      value->RemoveObserver(this);
  if (mPreserveAspectRatio && (value = do_QueryInterface(mPreserveAspectRatio)))
      value->RemoveObserver(this);

  // set the frame to null so we don't send messages to a dead object.
  if (mListener) {
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    if (imageLoader) {
      imageLoader->RemoveObserver(mListener);
    }
    NS_REINTERPRET_CAST(nsSVGImageListener*, mListener.get())->SetFrame(nsnull);
  }
  mListener = nsnull;
}

nsresult nsSVGImageFrame::Init()
{
  nsresult rv = nsSVGPathGeometryFrame::Init();
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsIDOMSVGImageElement> Rect = do_QueryInterface(mContent);
  NS_ASSERTION(Rect,"wrong content element");

  {
    nsCOMPtr<nsIDOMSVGAnimatedLength> length;
    Rect->GetX(getter_AddRefs(length));
    length->GetBaseVal(getter_AddRefs(mX));
    NS_ASSERTION(mX, "no x");
    if (!mX) return NS_ERROR_FAILURE;
    nsCOMPtr<nsISVGValue> value = do_QueryInterface(mX);
    if (value)
      value->AddObserver(this);
  }

  {
    nsCOMPtr<nsIDOMSVGAnimatedLength> length;
    Rect->GetY(getter_AddRefs(length));
    length->GetBaseVal(getter_AddRefs(mY));
    NS_ASSERTION(mY, "no y");
    if (!mY) return NS_ERROR_FAILURE;
    nsCOMPtr<nsISVGValue> value = do_QueryInterface(mY);
    if (value)
      value->AddObserver(this);
  }

  {
    nsCOMPtr<nsIDOMSVGAnimatedLength> length;
    Rect->GetWidth(getter_AddRefs(length));
    length->GetBaseVal(getter_AddRefs(mWidth));
    NS_ASSERTION(mWidth, "no width");
    if (!mWidth) return NS_ERROR_FAILURE;
    nsCOMPtr<nsISVGValue> value = do_QueryInterface(mWidth);
    if (value)
      value->AddObserver(this);
  }

  {
    nsCOMPtr<nsIDOMSVGAnimatedLength> length;
    Rect->GetHeight(getter_AddRefs(length));
    length->GetBaseVal(getter_AddRefs(mHeight));
    NS_ASSERTION(mHeight, "no height");
    if (!mHeight) return NS_ERROR_FAILURE;
    nsCOMPtr<nsISVGValue> value = do_QueryInterface(mHeight);
    if (value)
      value->AddObserver(this);
  }

  {
    nsCOMPtr<nsIDOMSVGAnimatedPreserveAspectRatio> ratio;
    Rect->GetPreserveAspectRatio(getter_AddRefs(ratio));
    ratio->GetBaseVal(getter_AddRefs(mPreserveAspectRatio));
    NS_ASSERTION(mHeight, "no preserveAspectRatio");
    if (!mPreserveAspectRatio) return NS_ERROR_FAILURE;
    nsCOMPtr<nsISVGValue> value = do_QueryInterface(mPreserveAspectRatio);
    if (value)
      value->AddObserver(this);
  }

  mSurface = nsnull;
  mSurfaceInvalid = PR_TRUE;

  mListener = new nsSVGImageListener(this);
  if (!mListener) return NS_ERROR_OUT_OF_MEMORY;
  nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
  NS_ENSURE_TRUE(imageLoader, NS_ERROR_UNEXPECTED);
  imageLoader->AddObserver(mListener);

  return NS_OK; 
}

//----------------------------------------------------------------------
// nsISVGValueObserver methods:

NS_IMETHODIMP
nsSVGImageFrame::DidModifySVGObservable(nsISVGValue* observable)
{
  nsCOMPtr<nsIDOMSVGLength> l = do_QueryInterface(observable);
  if (l && (mX==l || mY==l || mWidth==l || mHeight==l)) {
    UpdateGraphic(nsISVGPathGeometrySource::UPDATEMASK_PATH);
    return NS_OK;
  }
  // else
  return nsSVGPathGeometryFrame::DidModifySVGObservable(observable);
}

//----------------------------------------------------------------------
// nsISVGPathGeometrySource methods:

/* void constructPath (in nsISVGRendererPathBuilder pathBuilder); */
/* For the purposes of the update/invalidation logic pretend to
   be a rectangle. */
NS_IMETHODIMP nsSVGImageFrame::ConstructPath(nsISVGRendererPathBuilder* pathBuilder)
{
  float x, y, width, height;

  mX->GetValue(&x);
  mY->GetValue(&y);
  mWidth->GetValue(&width);
  mHeight->GetValue(&height);

  /* In a perfect world, this would be handled by the DOM, and 
     return a DOM exception. */
  if (width == 0 || height == 0)
    return NS_OK;

  pathBuilder->Moveto(x, y);
  pathBuilder->Lineto(x+width, y);
  pathBuilder->Lineto(x+width, y+height);
  pathBuilder->Lineto(x, y+height);
  pathBuilder->ClosePath(&x, &y);

  return NS_OK;
}

//----------------------------------------------------------------------
// nsISVGChildFrame methods:
NS_IMETHODIMP
nsSVGImageFrame::Paint(nsISVGRendererCanvas* canvas, const nsRect& dirtyRectTwips)
{
  if (mSurfaceInvalid) {
    nsCOMPtr<imgIRequest> currentRequest;
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    if (imageLoader)
      imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                              getter_AddRefs(currentRequest));

    nsCOMPtr<imgIContainer> currentContainer;
    if (currentRequest)
      currentRequest->GetImage(getter_AddRefs(currentContainer));

    nsCOMPtr<gfxIImageFrame> currentFrame;
    if (currentContainer) 
      currentContainer->GetCurrentFrame(getter_AddRefs(currentFrame));

    if (currentFrame) {
      ConvertFrame(currentFrame);
      mSurfaceInvalid = PR_FALSE;
    } else {
      return NS_OK;
    }
  }

  if (mSurface) {
    nsCOMPtr<nsIDOMSVGMatrix> ctm;
    GetCanvasTM(getter_AddRefs(ctm));

    float width, height;
    mWidth->GetValue(&width);
    mHeight->GetValue(&height);
    PRUint32 nativeWidth, nativeHeight;
    mSurface->GetWidth(&nativeWidth);
    mSurface->GetHeight(&nativeHeight);

    PRUint16 align, meetOrSlice;
    mPreserveAspectRatio->GetAlign(&align);
    mPreserveAspectRatio->GetMeetOrSlice(&meetOrSlice);

    // default to the defaults
    if (align == nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_UNKNOWN)
      align = nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMID;
    if (meetOrSlice == nsIDOMSVGPreserveAspectRatio::SVG_MEETORSLICE_UNKNOWN)
      align = nsIDOMSVGPreserveAspectRatio::SVG_MEETORSLICE_MEET;
    
    float a, d, e, f;
    a = width/nativeWidth;
    d = height/nativeHeight;
    e = 0.0f;
    f = 0.0f;

    if (align != nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_NONE &&
        a != d) {
      if (meetOrSlice == nsIDOMSVGPreserveAspectRatio::SVG_MEETORSLICE_MEET &&
          a < d ||
          meetOrSlice == nsIDOMSVGPreserveAspectRatio::SVG_MEETORSLICE_SLICE &&
          d < a) {
        d = a;
        switch (align) {
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMINYMIN:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMIN:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMAXYMIN:
            break;
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMINYMID:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMID:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMAXYMID:
            f = (height - a * nativeHeight) / 2.0f;
            break;
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMINYMAX:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMAX:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMAXYMAX:
            f = height - a * nativeHeight;
            break;
          default:
            NS_NOTREACHED("Unknown value for align");
        }
      }
      else if (
          meetOrSlice == nsIDOMSVGPreserveAspectRatio::SVG_MEETORSLICE_MEET &&
          d < a ||
          meetOrSlice == nsIDOMSVGPreserveAspectRatio::SVG_MEETORSLICE_SLICE &&
          a < d) {
        a = d;
        switch (align) {
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMINYMIN:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMINYMID:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMINYMAX:
            break;
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMIN:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMID:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMIDYMAX:
            e = (width - a * nativeWidth) / 2.0f;
            break;
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMAXYMIN:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMAXYMID:
          case nsIDOMSVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_XMAXYMAX:
            e = width - a * nativeWidth;
            break;
          default:
            NS_NOTREACHED("Unknown value for align");
        }
      }
      else NS_NOTREACHED("Unknown value for meetOrSlice");
    }

    float x, y;
    mX->GetValue(&x);
    mY->GetValue(&y);
    nsCOMPtr<nsIDOMSVGMatrix> trans;
    ctm->Translate(x + e, y + f, getter_AddRefs(trans));

    nsCOMPtr<nsIDOMSVGMatrix> fini;
    trans->ScaleNonUniform(a, d, getter_AddRefs(fini));

    canvas->CompositeSurfaceMatrix(mSurface,
                                   fini,
                                   mStyleContext->GetStyleDisplay()->mOpacity);
  }

  return NS_OK;
}

nsresult
nsSVGImageFrame::ConvertFrame(gfxIImageFrame *aNewFrame)
{
  PRInt32 width, height;
  aNewFrame->GetWidth(&width);
  aNewFrame->GetHeight(&height);
  
  nsISVGOuterSVGFrame *outerSVGFrame = GetOuterSVGFrame();
  nsCOMPtr<nsISVGRenderer> renderer;
  outerSVGFrame->GetRenderer(getter_AddRefs(renderer));
  renderer->CreateSurface(width, height, getter_AddRefs(mSurface));
  
  PRUint8 *data, *target;
  PRUint32 length;
  PRInt32 stride;
  mSurface->Lock();
  mSurface->GetData(&data, &length, &stride);
#ifdef XP_WIN
  stride = -stride;
#endif

  aNewFrame->LockImageData();
  aNewFrame->LockAlphaData();
  
  PRUint8 *rgb, *alpha = nsnull;
  PRUint32 bpr, abpr;
  aNewFrame->GetImageData(&rgb, &length);
  aNewFrame->GetImageBytesPerRow(&bpr);
  
  aNewFrame->GetAlphaData(&alpha, &length);
  aNewFrame->GetAlphaBytesPerRow(&abpr);
  
  if (!alpha) {
    for (PRInt32 y=0; y<height; y++) {
      if (stride > 0)
        target = data + stride * y;
      else
        target = data + stride * (1 - height) + stride * y;
      for (PRInt32 x=0; x<width; x++) {
#ifdef XP_WIN
        *target++ = rgb[y*bpr + 3*x];
        *target++ = rgb[y*bpr + 3*x + 1];
        *target++ = rgb[y*bpr + 3*x + 2];
#else
        *target++ = rgb[y*bpr + 3*x + 2];
        *target++ = rgb[y*bpr + 3*x + 1];
        *target++ = rgb[y*bpr + 3*x];
#endif
        *target++ = 255;
      }
    }
  } else {
    if (abpr >= width) {
      /* 8-bit alpha */
      for (PRInt32 y=0; y<height; y++) {
        if (stride > 0)
          target = data + stride * y;
        else
          target = data + stride * (1 - height) + stride * y;
        for (PRInt32 x=0; x<width; x++) {
          PRUint32 a = alpha[y*abpr + x];
#ifdef XP_WIN
          FAST_DIVIDE_BY_255(*target++, rgb[y*bpr + 3*x] * a);
          FAST_DIVIDE_BY_255(*target++, rgb[y*bpr + 3*x + 1] * a);
          FAST_DIVIDE_BY_255(*target++, rgb[y*bpr + 3*x + 2] * a);
#else
          FAST_DIVIDE_BY_255(*target++, rgb[y*bpr + 3*x + 2] * a);
          FAST_DIVIDE_BY_255(*target++, rgb[y*bpr + 3*x + 1] * a);
          FAST_DIVIDE_BY_255(*target++, rgb[y*bpr + 3*x] * a);
#endif
          *target++ = a;
        }
      }
    } else {
      /* 1-bit alpha */
      for (PRInt32 y=0; y<height; y++) {
        if (stride > 0)
          target = data + stride * y;
        else
          target = data + stride * (1 - height) + stride * y;
        PRUint8 *alphaRow = alpha + y*abpr;
        
        for (PRUint32 x=0; x<width; x++) {
          if (NS_GET_BIT(alphaRow, x)) {
#ifdef XP_WIN
            *target++ = rgb[y*bpr + 3*x];
            *target++ = rgb[y*bpr + 3*x + 1];
            *target++ = rgb[y*bpr + 3*x + 2];
#else
            *target++ = rgb[y*bpr + 3*x + 2];
            *target++ = rgb[y*bpr + 3*x + 1];
            *target++ = rgb[y*bpr + 3*x];
#endif
            *target++ = 255;
          } else {
            *target++ = 0;
            *target++ = 0;
            *target++ = 0;
            *target++ = 0;
          }
        }
      }
    }
  }
  
  mSurface->Unlock();
  aNewFrame->UnlockImageData();
  aNewFrame->UnlockAlphaData();
  
  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGImageListener implementation

NS_IMPL_ISUPPORTS2(nsSVGImageListener,
                   imgIDecoderObserver,
                   imgIContainerObserver)

nsSVGImageListener::nsSVGImageListener(nsSVGImageFrame *aFrame) :  mFrame(aFrame)
{
}

nsSVGImageListener::~nsSVGImageListener()
{
}

NS_IMETHODIMP nsSVGImageListener::OnStartDecode(imgIRequest *aRequest)
{
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnStartContainer(imgIRequest *aRequest,
                                                   imgIContainer *aImage)
{
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnStartFrame(imgIRequest *aRequest,
                                            gfxIImageFrame *aFrame)
{
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnDataAvailable(imgIRequest *aRequest,
                                                  gfxIImageFrame *aFrame,
                                                  const nsRect *aRect)
{
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnStopFrame(imgIRequest *aRequest,
                                              gfxIImageFrame *aFrame)
{
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnStopContainer(imgIRequest *aRequest,
                                                  imgIContainer *aImage)
{
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnStopDecode(imgIRequest *aRequest,
                                               nsresult status,
                                               const PRUnichar *statusArg)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  mFrame->mSurfaceInvalid = PR_TRUE;
  mFrame->UpdateGraphic(nsISVGPathGeometrySource::UPDATEMASK_ALL);
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::FrameChanged(imgIContainer *aContainer,
                                               gfxIImageFrame *newframe,
                                               nsRect * dirtyRect)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  mFrame->mSurfaceInvalid = PR_TRUE;
  mFrame->UpdateGraphic(nsISVGPathGeometrySource::UPDATEMASK_ALL);
  return NS_OK;
}

