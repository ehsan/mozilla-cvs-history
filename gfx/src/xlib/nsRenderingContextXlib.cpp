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
 *   Peter Hartshorn <peter@igelaus.com.au>
 *   Ken Faulkner <faulkner@igelaus.com.au>
 *   Tony Tsui <tony@igelaus.com.au>
 *   Tomi Leppikangas <tomi.leppikangas@oulu.fi>
 */

#include "nsRenderingContextXlib.h"
#include "nsFontMetricsXlib.h"
#include "nsICharRepresentable.h"
#include "xlibrgb.h"
#include "prprf.h"
#include "prmem.h"
#include "nsGCCache.h"

NS_IMPL_THREADSAFE_ISUPPORTS1(nsRenderingContextXlib, nsIRenderingContext)

static PRLogModuleInfo * RenderingContextXlibLM = PR_NewLogModule("RenderingContextXlib");

static nsGCCache *gcCache = nsnull;

class GraphicsState
{
public:
  GraphicsState();
  ~GraphicsState();

  nsTransform2D  *mMatrix;
  nsCOMPtr<nsIRegion> mClipRegion;
  nscolor         mColor;
  nsLineStyle     mLineStyle;
  nsIFontMetrics *mFontMetrics;
};

MOZ_DECL_CTOR_COUNTER(GraphicsState)

GraphicsState::GraphicsState()
{
  MOZ_COUNT_CTOR(GraphicsState);
  mMatrix = nsnull;
  mColor = NS_RGB(0, 0, 0);
  mLineStyle = nsLineStyle_kSolid;
  mFontMetrics = nsnull;
}

GraphicsState::~GraphicsState()
{
  MOZ_COUNT_DTOR(GraphicsState);
  NS_IF_RELEASE(mFontMetrics);
}

nsRenderingContextXlib::nsRenderingContextXlib()
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::nsRenderingContextXlib()\n"));
  NS_INIT_REFCNT();
  mOffscreenSurface = nsnull;
  mRenderingSurface = nsnull;
  mContext = nsnull;
  mFontMetrics = nsnull;
  mTranMatrix = nsnull;
  mP2T = 1.0f;
  mStateCache = new nsVoidArray();
  mCurrentFont = nsnull;
  mCurrentLineStyle = nsLineStyle_kSolid;
  mCurrentColor = NS_RGB(0, 0, 0);

  mDisplay = nsnull;
  mScreen = nsnull;
  mVisual = nsnull;
  mDepth = 0;
  mGC = nsnull;

  mFunction = GXcopy;

  PushState();
}

nsRenderingContextXlib::~nsRenderingContextXlib()
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::~nsRenderingContextXlib()\n"));
  NS_IF_RELEASE(mContext);
  NS_IF_RELEASE(mFontMetrics);
  if (mStateCache) {
    PRInt32 cnt = mStateCache->Count();

    while (--cnt >= 0) {
      PRBool clipstate;
      PopState(clipstate);
    }
    delete mStateCache;
    mStateCache = nsnull;
  }

  if (mTranMatrix)
    delete mTranMatrix;
  NS_IF_RELEASE(mOffscreenSurface);
  NS_IF_RELEASE(mFontMetrics);
  NS_IF_RELEASE(mContext);
  
  if(nsnull != mGC)
    mGC->Release();
}

NS_IMETHODIMP
nsRenderingContextXlib::Init(nsIDeviceContext* aContext, nsIWidget *aWindow)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::Init(DeviceContext, Widget)\n"));
  mContext = aContext;
  NS_IF_ADDREF(mContext);

  NS_ASSERTION(nsnull != mContext,"Device context is null.");

  mDisplay = ((nsDeviceContextXlib *) mContext)->GetDisplay();
  mScreen = ((nsDeviceContextXlib *) mContext)->GetScreen();
  mVisual = ((nsDeviceContextXlib *) mContext)->GetVisual();
  mDepth = ((nsDeviceContextXlib *) mContext)->GetDepth();

  mRenderingSurface = (nsDrawingSurfaceXlib *)new nsDrawingSurfaceXlib();

  if (mRenderingSurface) {

    Drawable win = (Drawable)aWindow->GetNativeData(NS_NATIVE_WINDOW);
    xGC *gc       = (xGC*)aWindow->GetNativeData(NS_NATIVE_GRAPHIC);

    mRenderingSurface->Init(mDisplay, 
                            mScreen, 
                            mVisual,
                            mDepth,
                            (PRUint32)win, 
                            gc);

    mOffscreenSurface = mRenderingSurface;
    NS_ADDREF(mRenderingSurface);
  }
  return (CommonInit());
}

/*static*/ nsresult
nsRenderingContextXlib::Shutdown()
{
  delete gcCache;
  return NS_OK;
}                  

NS_IMETHODIMP
nsRenderingContextXlib::Init(nsIDeviceContext* aContext, nsDrawingSurface aSurface)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContxtXlib::Init(DeviceContext, DrawingSurface)\n"));

  mContext = aContext;
  NS_IF_ADDREF(mContext);

  NS_ASSERTION(nsnull != mContext,"Device context is null.");

  mDisplay = ((nsDeviceContextXlib *) mContext)->GetDisplay();
  mScreen = ((nsDeviceContextXlib *) mContext)->GetScreen();
  mVisual = ((nsDeviceContextXlib *) mContext)->GetVisual();
  mDepth = ((nsDeviceContextXlib *) mContext)->GetDepth();

  mRenderingSurface = (nsDrawingSurfaceXlib *)aSurface;

  if (nsnull != mRenderingSurface) {
    NS_ADDREF(mRenderingSurface);
  }

  return CommonInit();
}

nsresult nsRenderingContextXlib::CommonInit(void)
{
  // put common stuff in here.
  int x, y;
  unsigned int width, height, border, depth;
  Window root_win;

  Drawable drawable = mRenderingSurface->GetDrawable();

#ifdef XLIB_GFX_NOISY
  printf("XGetGeometry(display=%p,drawable=%p)\n",
         (void *) mDisplay,
         (void *) drawable);
#endif

  XGetGeometry(mDisplay, 
               drawable, 
               &root_win,
               &x, 
               &y, 
               &width, 
               &height, 
               &border, 
               &depth);

  mClipRegion = do_QueryInterface(new nsRegionXlib());
  if (!mClipRegion) return NS_ERROR_OUT_OF_MEMORY;
  mClipRegion->Init();
  mClipRegion->SetTo(0, 0, width, height);

  mContext->GetDevUnitsToAppUnits(mP2T);
  float app2dev;
  mContext->GetAppUnitsToDevUnits(app2dev);
  mTranMatrix->AddScale(app2dev, app2dev);
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::Reset(void)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContext::Reset()\n"));
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetDeviceContext(nsIDeviceContext *&aContext)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContext::GetDeviceContext()\n"));
  NS_IF_ADDREF(mContext);
  aContext = mContext;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::LockDrawingSurface(PRInt32 aX, PRInt32 aY,
                                           PRUint32 aWidth, PRUint32 aHeight,
                                           void **aBits,
                                           PRInt32 *aStride,
                                           PRInt32 *aWidthBytes,
                                           PRUint32 aFlags)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::LockDrawingSurface()\n"));
  PushState();
  return mRenderingSurface->Lock(aX, aY, aWidth, aHeight,
                                 aBits, aStride, aWidthBytes, aFlags);
}

NS_IMETHODIMP
nsRenderingContextXlib::UnlockDrawingSurface(void)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::UnlockDrawingSurface()\n"));
  PRBool clipstate;
  PopState(clipstate);
  mRenderingSurface->Unlock();
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::SelectOffScreenDrawingSurface(nsDrawingSurface aSurface)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SelectOffScreenDrawingSurface()\n"));
  if (nsnull == aSurface)
    mRenderingSurface = mOffscreenSurface;
  else 
    mRenderingSurface = (nsDrawingSurfaceXlib *)aSurface;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetDrawingSurface(nsDrawingSurface *aSurface)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetDrawingSurface()\n"));
  *aSurface = mRenderingSurface;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetHints(PRUint32& aResult)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetHints()\n"));
  PRUint32 result = 0;
  // Most X servers implement 8 bit text rendering alot faster than
  // XChar2b rendering. In addition, we can avoid the PRUnichar to
  // XChar2b conversion. So we set this bit...
  result |= NS_RENDERING_HINT_FAST_8BIT_TEXT;
  aResult = result;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::PushState(void)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::PushState()\n"));
  GraphicsState *state = new GraphicsState();

  if (state == 0) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  state->mMatrix = mTranMatrix;
  
  mStateCache->AppendElement(state);
  
  if (nsnull == mTranMatrix)
    mTranMatrix = new nsTransform2D();
  else
    mTranMatrix = new nsTransform2D(mTranMatrix);
  
  if (mClipRegion) {
    state->mClipRegion = mClipRegion;
    mClipRegion = do_QueryInterface(new nsRegionXlib());
    if (!mClipRegion) return NS_ERROR_OUT_OF_MEMORY;
    mClipRegion->Init();
    mClipRegion->SetTo(*state->mClipRegion);
  }
  
  NS_IF_ADDREF(mFontMetrics);
  state->mFontMetrics = mFontMetrics;
  state->mColor = mCurrentColor;
  state->mLineStyle = mCurrentLineStyle;
  
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::PopState(PRBool &aClipState)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::PopState()\n"));

  PRUint32 cnt = mStateCache->Count();
  GraphicsState *state;
  
  if (cnt > 0) {
    state = (GraphicsState *)mStateCache->ElementAt(cnt - 1);
    mStateCache->RemoveElementAt(cnt - 1);
    
    if (mTranMatrix)
      delete mTranMatrix;
    mTranMatrix = state->mMatrix;
    
    mClipRegion = state->mClipRegion;
    if (mFontMetrics != state->mFontMetrics)
      SetFont(state->mFontMetrics);

    if (state->mColor != mCurrentColor)
      SetColor(state->mColor);
    if (state->mLineStyle != mCurrentLineStyle)
      SetLineStyle(state->mLineStyle);

    delete state;
  }

  if (mClipRegion)
    aClipState = mClipRegion->IsEmpty();
  else 
    aClipState = PR_TRUE;

  return NS_OK;
}
void nsRenderingContextXlib::UpdateGC()
{
   XGCValues values;
   unsigned long valuesMask;
 
   if (mGC)
     mGC->Release();
 
   memset(&values, 0, sizeof(XGCValues));
 
   unsigned long color;
 
   color = xlib_rgb_xpixel_from_rgb (NS_RGB(NS_GET_B(mCurrentColor),
                                            NS_GET_G(mCurrentColor),
                                            NS_GET_R(mCurrentColor)));

   values.foreground  =color;
   valuesMask = GCForeground;
 
   if (mCurrentFont) {
     valuesMask |= GCFont;
     values.font = mCurrentFont->fid;
   }
 
   values.line_style = mLineStyle;
   valuesMask |= GCLineStyle;
 
   values.function = mFunction;
   valuesMask |= GCFunction;

   Region rgn = 0;
   if (mClipRegion) { 
     mClipRegion->GetNativeRegion((void*&)rgn);
   }
 
   if (!gcCache) {
     gcCache = new nsGCCache();
     if (!gcCache) return;
   }

   mGC = gcCache->GetGC(mDisplay, mRenderingSurface->GetDrawable(), 
                        valuesMask, &values,
                        rgn);
}

NS_IMETHODIMP
nsRenderingContextXlib::IsVisibleRect(const nsRect& aRect, PRBool &aVisible)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::IsVisibleRect()\n"));
  aVisible = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::SetClipRect(const nsRect& aRect, nsClipCombine aCombine, PRBool &aClipState)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SetClipRect()\n"));
  nsRect trect = aRect;
  Region rgn;
  mTranMatrix->TransformCoord(&trect.x, &trect.y,
                           &trect.width, &trect.height);
  switch(aCombine) {
  case nsClipCombine_kIntersect:
    mClipRegion->Intersect(trect.x,trect.y,trect.width,trect.height);
    break;
  case nsClipCombine_kUnion:
    mClipRegion->Union(trect.x,trect.y,trect.width,trect.height);
    break;
  case nsClipCombine_kSubtract:
    mClipRegion->Subtract(trect.x,trect.y,trect.width,trect.height);
    break;
  case nsClipCombine_kReplace:
    mClipRegion->SetTo(trect.x,trect.y,trect.width,trect.height);
    break;
  }
  aClipState = mClipRegion->IsEmpty();

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetClipRect(nsRect &aRect, PRBool &aClipState)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetClipRext()\n"));
  PRInt32 x, y, w, h;
  if (!mClipRegion->IsEmpty()) {
    mClipRegion->GetBoundingBox(&x,&y,&w,&h);
    aRect.SetRect(x,y,w,h);
    aClipState = PR_TRUE;
  } else {
    aRect.SetRect(0,0,0,0);
    aClipState = PR_FALSE;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::SetClipRegion(const nsIRegion& aRegion, nsClipCombine aCombine, PRBool &aClipState)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SetClipRegion()\n"));
  Region rgn;
  switch(aCombine)
  {
    case nsClipCombine_kIntersect:
      mClipRegion->Intersect(aRegion);
      break;
    case nsClipCombine_kUnion:
      mClipRegion->Union(aRegion);
      break;
    case nsClipCombine_kSubtract:
      mClipRegion->Subtract(aRegion);
      break;
    case nsClipCombine_kReplace:
      mClipRegion->SetTo(aRegion);
      break;
  }

  aClipState = mClipRegion->IsEmpty();

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::CopyClipRegion(nsIRegion &aRegion)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::CopyClipRegion()\n"));
  aRegion.SetTo(*mClipRegion);
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetClipRegion(nsIRegion **aRegion)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetClipRegion()\n"));
  nsresult  rv = NS_OK;
  
  NS_ASSERTION(!(nsnull == aRegion), "no region ptr");
  
  if (*aRegion) {
    (*aRegion)->SetTo(*mClipRegion);
  }
  return rv;
}

NS_IMETHODIMP
nsRenderingContextXlib::SetLineStyle(nsLineStyle aLineStyle)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SetLineStyle()\n"));
  if (aLineStyle != mCurrentLineStyle) {
    /* XXX this isnt done in gtk, copy from there when ready
    switch(aLineStyle)
      { 
      case nsLineStyle_kSolid:
        mLineStyle = LineSolid;
        mDashes = 0;
        break;
      case nsLineStyle_kDashed:
          static char dashed[2] = {4,4};
        mDashList = dashed;
        mDashes = 2;
        break;
      case nsLineStyle_kDotted:
          static char dotted[2] = {3,1};
        mDashList = dotted;
        mDashes = 2;
        break;
    default:
        break;

    }
    */
    mCurrentLineStyle = aLineStyle ;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetLineStyle(nsLineStyle &aLineStyle)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetLineStyle()\n"));
  aLineStyle = mCurrentLineStyle;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::SetColor(nscolor aColor)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SetColor(nscolor)\n"));
  if (nsnull == mContext)
    return NS_ERROR_FAILURE;

  mCurrentColor = aColor;

  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("Setting color to %d %d %d\n", NS_GET_R(aColor), NS_GET_G(aColor), NS_GET_B(aColor)));
  return NS_OK;
}


NS_IMETHODIMP
nsRenderingContextXlib::GetColor(nscolor &aColor) const
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetColor()\n"));
  aColor = mCurrentColor;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::SetFont(const nsFont& aFont)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SetFont(nsFont)\n"));
  NS_IF_RELEASE(mFontMetrics);
  mContext->GetMetricsFor(aFont, mFontMetrics);
  return SetFont(mFontMetrics);
}

NS_IMETHODIMP
nsRenderingContextXlib::SetFont(nsIFontMetrics *aFontMetrics)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::SetFont(nsIFontMetrics)\n"));
  NS_IF_RELEASE(mFontMetrics);
  mFontMetrics = aFontMetrics;
  NS_IF_ADDREF(mFontMetrics);

  if (mFontMetrics)
  {
    nsFontHandle  fontHandle;
    mFontMetrics->GetFontHandle(fontHandle);
    mCurrentFont = (XFontStruct *)fontHandle;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetFontMetrics(nsIFontMetrics *&aFontMetrics)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetFontMetrics()\n"));
  NS_IF_ADDREF(mFontMetrics);
  aFontMetrics = mFontMetrics;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::Translate(nscoord aX, nscoord aY)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::Translate()\n"));
  mTranMatrix->AddTranslation((float)aX,(float)aY);
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::Scale(float aSx, float aSy)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::Scale()\n"));
  mTranMatrix->AddScale(aSx, aSy);
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetCurrentTransform(nsTransform2D *&aTransform)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetCurrentTransform()\n"));
  aTransform = mTranMatrix;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::CreateDrawingSurface(nsRect *aBounds, PRUint32 aSurfFlags, nsDrawingSurface &aSurface)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::CreateDrawingSurface()\n"));
  if (nsnull == mRenderingSurface) {
    aSurface = nsnull;
    return NS_ERROR_FAILURE;
  }
 
  nsDrawingSurfaceXlib *surf = new nsDrawingSurfaceXlib();

  if (surf) {
    NS_ADDREF(surf);
    if (!mGC)
      UpdateGC();
    surf->Init(mDisplay, 
               mScreen, 
               mVisual,
               mDepth,
               mGC,
               aBounds->width, 
               aBounds->height, 
               aSurfFlags);
  }
  
  aSurface = (nsDrawingSurface)surf;

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DestroyDrawingSurface(nsDrawingSurface aDS)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DestroyDrawingSurface()\n"));
  nsDrawingSurfaceXlib *surf = (nsDrawingSurfaceXlib *) aDS;

  NS_IF_RELEASE(surf);

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawLine(nscoord aX0, nscoord aY0, nscoord aX1, nscoord aY1)
{
  nscoord diffX, diffY;

  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawLine()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface)
    return NS_ERROR_FAILURE;

  mTranMatrix->TransformCoord(&aX0,&aY0);
  mTranMatrix->TransformCoord(&aX1,&aY1);
  
  diffX = aX1-aX0;
  diffY = aY1-aY0;

  if (0!=diffX) {
    diffX = (diffX>0?1:-1);
  }
  if (0!=diffY) {
    diffY = (diffY>0?1:-1);
  }

  UpdateGC();

  ::XDrawLine(mDisplay, mRenderingSurface->GetDrawable(),
              *mGC, aX0, aY0, aX1 - diffX, aY1 - diffY);

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawStdLine(nscoord aX0, nscoord aY0, nscoord aX1, nscoord aY1)
{
  nscoord diffX, diffY;

  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawStdLine()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface)
    return NS_ERROR_FAILURE;

  mTranMatrix->TransformCoord(&aX0,&aY0);
  mTranMatrix->TransformCoord(&aX1,&aY1);
  
  diffX = aX1-aX0;
  diffY = aY1-aY0;

  if (0!=diffX) {
    diffX = (diffX>0?1:-1);
  }
  if (0!=diffY) {
    diffY = (diffY>0?1:-1);
  }

  UpdateGC();
  ::XDrawLine(mDisplay, mRenderingSurface->GetDrawable(),
              *mGC, aX0, aY0, aX1 - diffX, aY1 - diffY);

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawPolyline(const nsPoint aPoints[], PRInt32 aNumPoints)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawPolyLine()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  PRInt32 i ;
  XPoint * xpoints;
  XPoint * thispoint;

  xpoints = (XPoint *) PR_Malloc(sizeof(XPoint) * aNumPoints);

  for (i = 0; i < aNumPoints; i++){
    thispoint = (xpoints+i);
    thispoint->x = aPoints[i].x;
    thispoint->y = aPoints[i].y;
    mTranMatrix->TransformCoord((PRInt32*)&thispoint->x,(PRInt32*)&thispoint->y);
  }

  UpdateGC();
  ::XDrawLines(mDisplay,
               mRenderingSurface->GetDrawable(),
               *mGC,
               xpoints, aNumPoints, CoordModeOrigin);

  PR_Free((void *)xpoints);

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawRect(const nsRect& aRect)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawRext()\n"));
  return DrawRect(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawRect()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }

  nscoord x,y,w,h; 
  
  x = aX;
  y = aY; 
  w = aWidth;
  h = aHeight;
    
  mTranMatrix->TransformCoord(&x,&y,&w,&h);
    
  // After the transform, if the numbers are huge, chop them, because
  // they're going to be converted from 32 bit to 16 bit.
  // It's all way off the screen anyway.
  ConditionRect(x,y,w,h);
   
  // Don't draw empty rectangles; also, w/h are adjusted down by one
  // so that the right number of pixels are drawn.
  if (w && h) 
  {
    UpdateGC();
    ::XDrawRectangle(mDisplay,
                     mRenderingSurface->GetDrawable(),
                     *mGC,
                     x,
                     y,
                     w - 1,
                     h - 1);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::FillRect(const nsRect& aRect)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillRect()\n"));
  return FillRect(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP
nsRenderingContextXlib::FillRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillRect()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;
  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  // After the transform, if the numbers are huge, chop them, because
  // they're going to be converted from 32 bit to 16 bit.
  // It's all way off the screen anyway.
  ConditionRect(x,y,w,h);

  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("About to fill window 0x%lxd with rect %d %d %d %d\n",
                                                mRenderingSurface->GetDrawable(), x, y, w, h));
  UpdateGC();
  ::XFillRectangle(mDisplay,
                   mRenderingSurface->GetDrawable(),
                   *mGC,
                   x,y,w,h);
  
  return NS_OK;
}

NS_IMETHODIMP 
nsRenderingContextXlib :: InvertRect(const nsRect& aRect)
{
  return InvertRect(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP 
nsRenderingContextXlib :: InvertRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) 
    return NS_ERROR_FAILURE;
  
  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  // After the transform, if the numbers are huge, chop them, because
  // they're going to be converted from 32 bit to 16 bit.
  // It's all way off the screen anyway.
  ConditionRect(x,y,w,h);

  mFunction = GXxor;
  
  UpdateGC();
  
  ::XFillRectangle(mDisplay,
                   mRenderingSurface->GetDrawable(),
                   *mGC,
                   x,
                   y,
                   w,
                   h);
  
  mFunction = GXcopy;

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawPolygon(const nsPoint aPoints[], PRInt32 aNumPoints)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawPolygon()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  PRInt32 i ;
  XPoint * xpoints;
  XPoint * thispoint;
  
  xpoints = (XPoint *) PR_Malloc(sizeof(XPoint) * aNumPoints);
  
  for (i = 0; i < aNumPoints; i++){
    thispoint = (xpoints+i);
    thispoint->x = aPoints[i].x;
    thispoint->y = aPoints[i].y;
    mTranMatrix->TransformCoord((PRInt32*)&thispoint->x,(PRInt32*)&thispoint->y);
  }
  
  UpdateGC();
  ::XDrawLines(mDisplay,
               mRenderingSurface->GetDrawable(),
               *mGC,
               xpoints, aNumPoints, CoordModeOrigin);

  PR_Free((void *)xpoints);
  
  return NS_OK;    
}

NS_IMETHODIMP
nsRenderingContextXlib::FillPolygon(const nsPoint aPoints[], PRInt32 aNumPoints)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillPolygon()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  PRInt32 i ;
  XPoint * xpoints;
  XPoint * thispoint;
  nscoord x,y;
   
  xpoints = (XPoint *) PR_Malloc(sizeof(XPoint) * aNumPoints);
  
  for (i = 0; i < aNumPoints; i++){
    thispoint = (xpoints+i);
    x = aPoints[i].x;
    y = aPoints[i].y;
    mTranMatrix->TransformCoord(&x,&y);
    thispoint->x = x;
    thispoint->y = y;
  } 
    
  UpdateGC();
  ::XFillPolygon(mDisplay,
                 mRenderingSurface->GetDrawable(),
                 *mGC,
                 xpoints, aNumPoints, Convex, CoordModeOrigin);
               
  PR_Free((void *)xpoints);

  return NS_OK; 
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawEllipse(const nsRect& aRect)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawEllipse()\n"));
  return DrawEllipse(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawEllipse(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawEllipse()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;
   
  x = aX; 
  y = aY;
  w = aWidth; 
  h = aHeight;
    
  mTranMatrix->TransformCoord(&x,&y,&w,&h);
    
  UpdateGC();
  ::XDrawArc(mDisplay,
             mRenderingSurface->GetDrawable(),
             *mGC,
             x,y,w,h, 0, 360 * 64);
  
  return NS_OK;  
}

NS_IMETHODIMP
nsRenderingContextXlib::FillEllipse(const nsRect& aRect)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillEllipse()\n"));
  return FillEllipse(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP
nsRenderingContextXlib::FillEllipse(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillEllipse()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;
   
  x = aX; 
  y = aY;
  w = aWidth; 
  h = aHeight;
    
  mTranMatrix->TransformCoord(&x,&y,&w,&h);
    
  UpdateGC();

  ::XFillArc(mDisplay,
             mRenderingSurface->GetDrawable(),
             *mGC,
             x,y,w,h, 0, 360 * 64);
  
  return NS_OK;  
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawArc(const nsRect& aRect,
                                float aStartAngle, float aEndAngle)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawArc()\n"));
  return DrawArc(aRect.x,aRect.y,aRect.width,aRect.height,aStartAngle,aEndAngle); 
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawArc(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight,
                                float aStartAngle, float aEndAngle)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawArc()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;
   
  x = aX; 
  y = aY;
  w = aWidth; 
  h = aHeight;
    
  mTranMatrix->TransformCoord(&x,&y,&w,&h);
    
  UpdateGC();
  ::XDrawArc(mDisplay,
             mRenderingSurface->GetDrawable(),
             *mGC,
             x,y,w,h, NSToIntRound(aStartAngle * 64.0f),
             NSToIntRound(aEndAngle * 64.0f));
  
  return NS_OK;  
}

NS_IMETHODIMP
nsRenderingContextXlib::FillArc(const nsRect& aRect,
                                float aStartAngle, float aEndAngle)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillArc()\n"));
  return FillArc(aRect.x, aRect.y, aRect.width, aRect.height, aStartAngle, aEndAngle);
}

NS_IMETHODIMP
nsRenderingContextXlib::FillArc(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight,
                                float aStartAngle, float aEndAngle)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::FillArc()\n"));
  if (nsnull == mTranMatrix || nsnull == mRenderingSurface) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;
   
  x = aX; 
  y = aY;
  w = aWidth; 
  h = aHeight;
    
  mTranMatrix->TransformCoord(&x,&y,&w,&h);
    
  UpdateGC();
  ::XFillArc(mDisplay,
             mRenderingSurface->GetDrawable(),
             *mGC,
             x,y,w,h, NSToIntRound(aStartAngle * 64.0f),
             NSToIntRound(aEndAngle * 64.0f));
  
  return NS_OK;  
}

NS_IMETHODIMP
nsRenderingContextXlib::GetWidth(char aC, nscoord& aWidth)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetWidth()\n"));
  // Check for the very common case of trying to get the width of a single
  // space.
  if ((aC == ' ') && (nsnull != mFontMetrics)) {
    nsFontMetricsXlib* fontMetricsXlib = (nsFontMetricsXlib*)mFontMetrics;
    return fontMetricsXlib->GetSpaceWidth(aWidth);
  }
  return GetWidth(&aC, 1, aWidth);
}

NS_IMETHODIMP
nsRenderingContextXlib::GetWidth(PRUnichar aC, nscoord& aWidth,
                                 PRInt32 *aFontID)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetWidth()\n"));
  return GetWidth(&aC, 1, aWidth, aFontID);
}

NS_IMETHODIMP
nsRenderingContextXlib::GetWidth(const nsString& aString, nscoord& aWidth,
                                 PRInt32 *aFontID)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetWidth()\n"));
  return GetWidth(aString.GetUnicode(), aString.Length(), aWidth, aFontID);
}

NS_IMETHODIMP
nsRenderingContextXlib::GetWidth(const char* aString, nscoord& aWidth)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetWidth()\n"));
  return GetWidth(aString, strlen(aString), aWidth);
}

NS_IMETHODIMP
nsRenderingContextXlib::GetWidth(const char* aString, PRUint32 aLength, nscoord& aWidth)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetWidth()\n"));
  if (0 == aLength) {
    aWidth = 0;
  }
  else {
   
    int rawWidth;

    if ((mCurrentFont->min_byte1 == 0) && (mCurrentFont->max_byte1 == 0))
      rawWidth = XTextWidth(mCurrentFont, (char *)aString, aLength);
    else
      rawWidth = XTextWidth16(mCurrentFont, (XChar2b *)aString, aLength / 2);

    aWidth = NSToCoordRound(rawWidth * mP2T);
  }  
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetWidth(const PRUnichar* aString, PRUint32 aLength,
                                 nscoord& aWidth, PRInt32 *aFontID)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::GetWidth()\n"));
  if (0 == aLength) {
    aWidth = 0;
  }
  else {
    nsFontMetricsXlib* metrics = (nsFontMetricsXlib*) mFontMetrics;
    nsFontXlib *prevFont = nsnull;
    int rawWidth = 0;
    PRUint32 start = 0;
    PRUint32 i;
    for (i = 0; i < aLength; i++) {
      PRUnichar c = aString[i];
      nsFontXlib* currFont = nsnull;
      nsFontXlib** font = metrics->mLoadedFonts;
      nsFontXlib** end = &metrics->mLoadedFonts[metrics->mLoadedFontsCount];
      while (font < end) {
        if (IS_REPRESENTABLE((*font)->mMap, c)) {
          currFont = *font;
          goto FoundFont; // for speed -- avoid "if" statement
        }
        font++;
      }
      currFont = metrics->FindFont(c);
FoundFont:
      // XXX avoid this test by duplicating code -- erik
      if (prevFont) {
        if (currFont != prevFont) {
          rawWidth += prevFont->GetWidth(&aString[start], i - start);
          prevFont = currFont;
          start = i;
        }
      }
      else {
        prevFont = currFont;
        start = i;
      }
    }

    if (prevFont) {
      //                                              
      rawWidth += prevFont->GetWidth(&aString[start], i - start);
    }

    aWidth = NSToCoordRound(rawWidth * mP2T);
  }
  if (nsnull != aFontID)
    *aFontID = 0;


  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawString(const char *aString, PRUint32 aLength,
                                   nscoord aX, nscoord aY,
                                   const nscoord* aSpacing)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawString()\n"));
  if (0 != aLength) {
    if (mTranMatrix == nsnull)
      return NS_ERROR_FAILURE;
    if (mRenderingSurface == nsnull)
      return NS_ERROR_FAILURE;
    if (aString == nsnull)
      return NS_ERROR_FAILURE;
    
    nscoord x = aX;
    nscoord y = aY;
    
    // Substract xFontStruct ascent since drawing specifies baseline
    if (mFontMetrics) {
      mFontMetrics->GetMaxAscent(y);
      y += aY;
    }

    UpdateGC();

    if (nsnull != aSpacing) {
      // Render the string, one character at a time...
      const char* end = aString + aLength;
      while (aString < end) {
        char ch = *aString++;
        nscoord xx = x;
        nscoord yy = y;
        mTranMatrix->TransformCoord(&xx, &yy);
        ::XDrawString(mDisplay,
                    mRenderingSurface->GetDrawable(),
                    *mGC,
                    xx, yy, &ch, 1);
        x += *aSpacing++;
      }
    }
    else {
      mTranMatrix->TransformCoord(&x, &y);
      XDrawString(mDisplay,
                  mRenderingSurface->GetDrawable(),
                  *mGC,
                  x, y, aString, aLength);
    }
  }

#if 0
  //this is no longer to be done by this API, but another
  //will take it's place that will need this code again. MMP
  if (mFontMetrics)
  {
    const nsFont *font;
    mFontMetrics->GetFont(font);
    PRUint8 deco = font->decorations;

    if (deco & NS_FONT_DECORATION_OVERLINE)
      DrawLine(aX, aY, aX + aWidth, aY);

    if (deco & NS_FONT_DECORATION_UNDERLINE)
    {
      nscoord ascent,descent;

      mFontMetrics->GetMaxAscent(ascent);
      mFontMetrics->GetMaxDescent(descent);

      DrawLine(aX, aY + ascent + (descent >> 1),
               aX + aWidth, aY + ascent + (descent >> 1));
    }

    if (deco & NS_FONT_DECORATION_LINE_THROUGH)
    {
      nscoord height;

	  mFontMetrics->GetHeight(height);

      DrawLine(aX, aY + (height >> 1), aX + aWidth, aY + (height >> 1));
    }
  }
#endif


  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawString(const PRUnichar *aString, PRUint32 aLength,
                                   nscoord aX, nscoord aY,
                                   PRInt32 aFontID,
                                   const nscoord* aSpacing)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawString()\n"));
    if (aLength && mFontMetrics) {
      if (mTranMatrix == nsnull)
        return NS_ERROR_FAILURE;
      if (mRenderingSurface == nsnull)
        return NS_ERROR_FAILURE;
      if (aString == nsnull)
        return NS_ERROR_FAILURE;

    nscoord x = aX;
    nscoord y;

    // Substract xFontStruct ascent since drawing specifies baseline
    mFontMetrics->GetMaxAscent(y);
    y += aY;
    aY = y;

    mTranMatrix->TransformCoord(&x, &y);

    nsFontMetricsXlib* metrics = (nsFontMetricsXlib*) mFontMetrics;
    nsFontXlib* prevFont = nsnull;
    PRUint32 start = 0;
    PRUint32 i;
    for (i = 0; i < aLength; i++) {
      PRUnichar c = aString[i];
      nsFontXlib* currFont = nsnull;
      nsFontXlib** font = metrics->mLoadedFonts;
      nsFontXlib** lastFont = &metrics->mLoadedFonts[metrics->mLoadedFontsCount];
      while (font < lastFont) {
        if (IS_REPRESENTABLE((*font)->mMap, c)) {
	  currFont = *font;
	  goto FoundFont; // for speed -- avoid "if" statement
	}
	font++;
      }
      currFont = metrics->FindFont(c);
FoundFont:
      // XXX avoid this test by duplicating code -- erik
      if (prevFont) {
	if (currFont != prevFont) {
	  if (aSpacing) {
	    const PRUnichar* str = &aString[start];
	    const PRUnichar* end = &aString[i];
      UpdateGC();
	    while (str < end) {
	      x = aX;
	      y = aY;
              mTranMatrix->TransformCoord(&x, &y);
        prevFont->DrawString(this, mRenderingSurface, x, y, str, 1);
	      aX += *aSpacing++;
	      str++;
	    }
	  }
	  else {
      UpdateGC();
      prevFont->DrawString(this, mRenderingSurface, x, y, &aString[start], i - start);
              x += prevFont->GetWidth(&aString[start], i -start);

	  }
	  prevFont = currFont;
	  start = i;
	}
      }
      else {
        prevFont = currFont;
	start = i;
      }
    }

    if (prevFont) {
    UpdateGC();
      if (aSpacing) {
	const PRUnichar* str = &aString[start];
	const PRUnichar* end = &aString[i];
	while (str < end) {
	  x = aX;
	  y = aY;
          mTranMatrix->TransformCoord(&x, &y);
          prevFont->DrawString(this, mRenderingSurface, x, y, str, 1);
	  aX += *aSpacing++;
	  str++;
	}
      }
      else {
        prevFont->DrawString(this, mRenderingSurface, x, y, &aString[start], i - start);
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP nsRenderingContextXlib::DrawString(const nsString& aString, nscoord aX, nscoord aY,
                                                 PRInt32 aFontID,
                                                 const nscoord* aSpacing)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawString()\n"));
  return DrawString(aString.GetUnicode(), aString.Length(),
                    aX, aY, aFontID, aSpacing);
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawImage(nsIImage *aImage, nscoord aX, nscoord aY)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawImage()\n"));
  nscoord width, height;

  // we have to do this here because we are doing a transform below
  width = NSToCoordRound(mP2T * aImage->GetWidth());
  height = NSToCoordRound(mP2T * aImage->GetHeight());
  
  return DrawImage(aImage, aX, aY, width, height);
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawImage(nsIImage *aImage, nscoord aX, nscoord aY,
                                  nscoord aWidth, nscoord aHeight)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawImage()\n"));
  nsRect tr;
  
  tr.x = aX;
  tr.y = aY;
  tr.width = aWidth;
  tr.height = aHeight;
  return DrawImage(aImage, tr);
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawImage(nsIImage *aImage, const nsRect& aRect)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawImage()\n"));

  nsRect tr;
  tr = aRect;
  mTranMatrix->TransformCoord(&tr.x, &tr.y, &tr.width, &tr.height);
  UpdateGC();
  return aImage->Draw(*this, mRenderingSurface, tr.x, tr.y, tr.width, tr.height);
}

NS_IMETHODIMP
nsRenderingContextXlib::DrawImage(nsIImage *aImage, const nsRect& aSRect, const nsRect& aDRect)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::DrawImage()\n"));
  nsRect	sr,dr;
  
  sr = aSRect;
  mTranMatrix->TransformCoord(&sr.x, &sr.y,
                           &sr.width, &sr.height);
  sr.x = aSRect.x;
  sr.y = aSRect.y;
  mTranMatrix->TransformNoXLateCoord(&sr.x, &sr.y);
  
  dr = aDRect;
  mTranMatrix->TransformCoord(&dr.x, &dr.y,
                           &dr.width, &dr.height);
  UpdateGC();
  
  return aImage->Draw(*this, mRenderingSurface,
                      sr.x, sr.y,
                      sr.width, sr.height,
                      dr.x, dr.y,
                      dr.width, dr.height);
  return NS_OK;
}

#if 0
// in nsRenderingContextImpl
/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 *	@update 3/16/00 dwc
 */
NS_IMETHODIMP 
nsRenderingContextXlib::DrawTile(nsIImage *aImage,nscoord aX0,nscoord aY0,nscoord aX1,nscoord aY1,
                                                    nscoord aWidth,nscoord aHeight)
{

  return NS_OK;
}
#endif

// this method of DrawTile is not implemented in nsRenderingContextImpl

NS_IMETHODIMP
nsRenderingContextXlib::DrawTile(nsIImage *aImage,
                                nscoord aSrcXOffset, nscoord aSrcYOffset,
                                const nsRect &aTileRect)
{
  nsRect tileRect(aTileRect);
  nsRect srcRect(0, 0, aSrcXOffset, aSrcYOffset);
  mTranMatrix->TransformCoord(&srcRect.x, &srcRect.y, &srcRect.width,
                           &srcRect.height);
  mTranMatrix->TransformCoord(&tileRect.x, &tileRect.y,
                           &tileRect.width, &tileRect.height);

  if((tileRect.width > 0) && (tileRect.height > 0))
    ((nsImageXlib*)aImage)->DrawTile(*this, mRenderingSurface, srcRect.width, srcRect.height, tileRect);

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::CopyOffScreenBits(nsDrawingSurface aSrcSurf, PRInt32 aSrcX, PRInt32 aSrcY,
                                          const nsRect &aDestBounds, PRUint32 aCopyFlags)
{
  PR_LOG(RenderingContextXlibLM, PR_LOG_DEBUG, ("nsRenderingContextXlib::CopyOffScreenBits()\n"));
  PRInt32               srcX = aSrcX;
  PRInt32               srcY = aSrcY;
  nsRect                drect = aDestBounds;
  nsDrawingSurfaceXlib  *destsurf;

  if (aSrcSurf == nsnull)
    return NS_ERROR_FAILURE;
  if (mTranMatrix == nsnull)
    return NS_ERROR_FAILURE;
  if (mRenderingSurface == nsnull)
    return NS_ERROR_FAILURE;

  if (aCopyFlags & NS_COPYBITS_TO_BACK_BUFFER) {
    NS_ASSERTION(!(nsnull == mRenderingSurface), "no back buffer");
    destsurf = mRenderingSurface;
  }
  else
    destsurf = mOffscreenSurface;
  
  if (aCopyFlags & NS_COPYBITS_XFORM_SOURCE_VALUES)
    mTranMatrix->TransformCoord(&srcX, &srcY);
  
  if (aCopyFlags & NS_COPYBITS_XFORM_DEST_VALUES)
    mTranMatrix->TransformCoord(&drect.x, &drect.y, &drect.width, &drect.height);
  
  //XXX flags are unused. that would seem to mean that there is
  //inefficiency somewhere... MMP

  UpdateGC();

  XCopyArea(mDisplay,
            ((nsDrawingSurfaceXlib *)aSrcSurf)->GetDrawable(),
            destsurf->GetDrawable(),
            *mGC,
            srcX, srcY,
            drect.width, drect.height,
            drect.x, drect.y);

  return NS_OK;
}

NS_IMETHODIMP nsRenderingContextXlib::RetrieveCurrentNativeGraphicData(PRUint32 * ngd)
{
  return NS_OK;
}

#ifdef MOZ_MATHML
#ifdef FONT_SWITCHING

NS_IMETHODIMP
nsRenderingContextXlib::GetBoundingMetrics(const char*        aString, 
                                           PRUint32           aLength,
                                           nsBoundingMetrics& aBoundingMetrics)
{
  aBoundingMetrics.Clear();
  if (aString && 0 < aLength) {

    XCharStruct overall;
    int direction, font_ascent, font_descent;

    if ((mCurrentFont->min_byte1 == 0) && (mCurrentFont->max_byte1 == 0))
      XTextExtents(mCurrentFont, aString, aLength,
                    &direction, &font_ascent, &font_descent,
                    &overall);
    else
      XTextExtents16(mCurrentFont, (XChar2b *)aString, aLength/2,
                    &direction, &font_ascent, &font_descent,
                    &overall);

    aBoundingMetrics.leftBearing  =  overall.lbearing;
    aBoundingMetrics.rightBearing =  overall.rbearing;
    aBoundingMetrics.width        =  overall.width;
    aBoundingMetrics.ascent       =  overall.ascent;
    aBoundingMetrics.descent      =  overall.descent;

    aBoundingMetrics.leftBearing = NSToCoordRound(aBoundingMetrics.leftBearing * mP2T);
    aBoundingMetrics.rightBearing = NSToCoordRound(aBoundingMetrics.rightBearing * mP2T);
    aBoundingMetrics.width = NSToCoordRound(aBoundingMetrics.width * mP2T);
    aBoundingMetrics.ascent = NSToCoordRound(aBoundingMetrics.ascent * mP2T);
    aBoundingMetrics.descent = NSToCoordRound(aBoundingMetrics.descent * mP2T);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXlib::GetBoundingMetrics(const PRUnichar*   aString, 
                                           PRUint32           aLength,
                                           nsBoundingMetrics& aBoundingMetrics,
                                           PRInt32*           aFontID)
{
  aBoundingMetrics.Clear(); 
  if (aString && 0 < aLength) {

    nsFontMetricsXlib* metrics = (nsFontMetricsXlib*) mFontMetrics;
    nsFontXlib* prevFont = nsnull;

    nsBoundingMetrics rawbm;
    PRBool firstTime = PR_TRUE;
    PRUint32 start = 0;
    PRUint32 i;
    for (i = 0; i < aLength; i++) {
      PRUnichar c = aString[i];
      nsFontXlib* currFont = nsnull;
      nsFontXlib** font = metrics->mLoadedFonts;
      nsFontXlib** end = &metrics->mLoadedFonts[metrics->mLoadedFontsCount];
      while (font < end) {
        if (IS_REPRESENTABLE((*font)->mMap, c)) {
          currFont = *font;
          goto FoundFont; // for speed -- avoid "if" statement
        }
        font++;
      }
      currFont = metrics->FindFont(c);
    FoundFont:
      // XXX avoid this test by duplicating code -- erik
      if (prevFont) {
        if (currFont != prevFont) {
          prevFont->GetBoundingMetrics((const PRUnichar*) &aString[start],
                                       i - start, rawbm);
          if (firstTime) {
            firstTime = PR_FALSE;
            aBoundingMetrics = rawbm;
          } 
          else {
            aBoundingMetrics += rawbm;
          }
          prevFont = currFont;
          start = i;
        }
      }
      else {
        prevFont = currFont;
        start = i;
      }
    }
    
    if (prevFont) {
      prevFont->GetBoundingMetrics((const PRUnichar*) &aString[start],
                                   i - start, rawbm);
      if (firstTime) {
        aBoundingMetrics = rawbm;
      }
      else {
        aBoundingMetrics += rawbm;
      }
    }
    // convert to app units
    aBoundingMetrics.leftBearing = NSToCoordRound(aBoundingMetrics.leftBearing * mP2T);
    aBoundingMetrics.rightBearing = NSToCoordRound(aBoundingMetrics.rightBearing * mP2T);
    aBoundingMetrics.width = NSToCoordRound(aBoundingMetrics.width * mP2T);
    aBoundingMetrics.ascent = NSToCoordRound(aBoundingMetrics.ascent * mP2T);
    aBoundingMetrics.descent = NSToCoordRound(aBoundingMetrics.descent * mP2T);
  }
  if (nsnull != aFontID)
    *aFontID = 0;

  return NS_OK;
}
#endif /* FONT_SWITCHING */
#endif /* MOZ_MATHML */
