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

#include "nsScrollingView.h"
#include "nsIWidget.h"
#include "nsUnitConversion.h"
#include "nsIViewManager.h"
#include "nsIPresContext.h"
#include "nsIScrollbar.h"
#include "nsIDeviceContext.h"
#include "nsGUIEvent.h"
#include "nsWidgetsCID.h"
#include "nsViewsCID.h"
#include "nsIScrollableView.h"
#include "nsIFrame.h"

static NS_DEFINE_IID(kIScrollbarIID, NS_ISCROLLBAR_IID);
static NS_DEFINE_IID(kIScrollableViewIID, NS_ISCROLLABLEVIEW_IID);
static NS_DEFINE_IID(kWidgetCID, NS_CHILD_CID);

class ScrollBarView : public nsView
{
public:
  ScrollBarView(nsScrollingView *aScrollingView);
  ~ScrollBarView();
  nsEventStatus HandleEvent(nsGUIEvent *aEvent, PRUint32 aEventFlags);
  void SetPosition(nscoord x, nscoord y);
  void SetDimensions(nscoord width, nscoord height);

public:
  nsScrollingView *mScrollingView;
};

ScrollBarView :: ScrollBarView(nsScrollingView *aScrollingView)
{
  mScrollingView = aScrollingView;
}

ScrollBarView :: ~ScrollBarView()
{
}

nsEventStatus ScrollBarView :: HandleEvent(nsGUIEvent *aEvent, PRUint32 aEventFlags)
{
  nsEventStatus retval = nsEventStatus_eIgnore;

  switch (aEvent->message)
  {
    case NS_SCROLLBAR_POS:
    case NS_SCROLLBAR_PAGE_NEXT:
    case NS_SCROLLBAR_PAGE_PREV:
    case NS_SCROLLBAR_LINE_NEXT:
    case NS_SCROLLBAR_LINE_PREV:
      NS_ASSERTION((nsnull != mScrollingView), "HandleEvent() called after the ScrollingView has been destroyed.");
      if (nsnull != mScrollingView)
        mScrollingView->HandleScrollEvent(aEvent, aEventFlags);
      retval = nsEventStatus_eConsumeNoDefault;
      break;

    default:
      break;
  }

  return retval;
}

void ScrollBarView :: SetPosition(nscoord x, nscoord y)
{
  mBounds.MoveTo(x, y);

  if (nsnull != mWindow)
  {
    nsIPresContext  *px = mViewManager->GetPresContext();
    float           twipToPix = px->GetTwipsToPixels();
    nscoord         parx = 0, pary = 0;
    nsIWidget       *pwidget = nsnull;
  
    pwidget = GetOffsetFromWidget(&parx, &pary);
    NS_IF_RELEASE(pwidget);
    
    mWindow->Move(NSTwipsToIntPixels((x + parx), twipToPix),
                  NSTwipsToIntPixels((y + pary), twipToPix));

    NS_RELEASE(px);
  }
}

void ScrollBarView :: SetDimensions(nscoord width, nscoord height)
{
  mBounds.SizeTo(width, height);

  if (nsnull != mWindow)
  {
    nsIPresContext  *px = mViewManager->GetPresContext();
    float           t2p = px->GetTwipsToPixels();
  
    mWindow->Resize(NSTwipsToIntPixels(width, t2p), NSTwipsToIntPixels(height, t2p),
                    PR_TRUE);

    NS_RELEASE(px);
  }
}

#if 0
class nsICornerWidget : public nsISupports {
public:
  NS_IMETHOD Init(nsIWidget* aParent, const nsRect& aBounds) = 0;
  NS_IMETHOD MoveTo(PRInt32 aX, PRInt32 aY) = 0;
  NS_IMETHOD Show() = 0;
  NS_IMETHOD Hide() = 0;
  NS_IMETHOD Start() = 0;
  NS_IMETHOD Stop() = 0;
};
#endif

class CornerView : public nsView
{
public:
  CornerView();
  ~CornerView();
  void ShowQuality(PRBool aShow);
  void SetQuality(nsContentQuality aQuality);
  void Show(PRBool aShow);
  PRBool Paint(nsIRenderingContext& rc, const nsRect& rect,
               PRUint32 aPaintFlags, nsIView *aBackstop = nsnull);

  PRBool            mShowQuality;
  nsContentQuality  mQuality;
  PRBool            mShow;
};

CornerView :: CornerView()
{
  mShowQuality = PR_FALSE;
  mQuality = nsContentQuality_kGood;
  mShow = PR_FALSE;
}

CornerView :: ~CornerView()
{
}

void CornerView :: ShowQuality(PRBool aShow)
{
  if (mShowQuality != aShow)
  {
    mShowQuality = aShow;

    if (mShow == PR_FALSE)
    {
      if (mVis == nsViewVisibility_kShow)
        mViewManager->SetViewVisibility(this, nsViewVisibility_kHide);
      else
        mViewManager->SetViewVisibility(this, nsViewVisibility_kShow);

      nscoord dimx, dimy;

      //this will force the scrolling view to recalc the scrollbar sizes... MMP

      mParent->GetDimensions(&dimx, &dimy);
      mParent->SetDimensions(dimx, dimy);
    }

    mViewManager->UpdateView(this, nsnull, NS_VMREFRESH_IMMEDIATE);
  }
}

void CornerView :: SetQuality(nsContentQuality aQuality)
{
  if (mQuality != aQuality)
  {
    mQuality = aQuality;

    if (mVis == nsViewVisibility_kShow)
      mViewManager->UpdateView(this, nsnull, NS_VMREFRESH_IMMEDIATE);
  }
}

void CornerView :: Show(PRBool aShow)
{
  if (mShow != aShow)
  {
    mShow = aShow;

    if (mShow == PR_TRUE)
      mViewManager->SetViewVisibility(this, nsViewVisibility_kShow);
    else if (mShowQuality == PR_FALSE)
      mViewManager->SetViewVisibility(this, nsViewVisibility_kHide);

    nscoord dimx, dimy;

    //this will force the scrolling view to recalc the scrollbar sizes... MMP

    mParent->GetDimensions(&dimx, &dimy);
    mParent->SetDimensions(dimx, dimy);
  }
}

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

PRBool CornerView :: Paint(nsIRenderingContext& rc, const nsRect& rect,
                           PRUint32 aPaintFlags, nsIView *aBackstop)
{
  PRBool  clipres = PR_FALSE;

  if (mVis == nsViewVisibility_kShow)
  {
    nscoord xoff, yoff;
    nsRect  brect;

    rc.PushState();

    GetScrollOffset(&xoff, &yoff);
    rc.Translate(xoff, yoff);

    GetBounds(brect);

    clipres = rc.SetClipRect(brect, nsClipCombine_kIntersect);

    if (clipres == PR_FALSE)
    {
      rc.SetColor(NS_RGB(192, 192, 192));
      rc.FillRect(brect);

      if (PR_TRUE == mShowQuality)
      {
        nscolor tcolor, bcolor;

        //display quality indicator

        rc.Translate(brect.x, brect.y);

        rc.SetColor(NS_RGB(0, 0, 0));

        rc.FillEllipse(NSToCoordFloor(brect.width * 0.15f),
                       NSToCoordFloor(brect.height * 0.15f),
                       NSToCoordRound(brect.width * 0.7f),    // XXX should use NSToCoordCeil ??
                       NSToCoordRound(brect.height * 0.7f));  // XXX should use NSToCoordCeil ??

        if (mQuality == nsContentQuality_kGood)
          rc.SetColor(NS_RGB(0, 255, 0));
        else if (mQuality == nsContentQuality_kFair)
          rc.SetColor(NS_RGB(255, 176, 0));
        else
          rc.SetColor(NS_RGB(255, 0, 0));

        //hey, notice that these numbers don't add up... that's because
        //something funny happens on windows when the *right* numbers are
        //used. MMP

        rc.FillEllipse(NSToCoordRound(brect.width * 0.23f),  // XXX should use NSToCoordCeil ??
                       NSToCoordRound(brect.height * 0.23f), // XXX should use NSToCoordCeil ??
                       nscoord(brect.width * 0.46f),
                       nscoord(brect.height * 0.46f));

        bcolor = tcolor = rc.GetColor();

        //this is inefficient, but compact...

        tcolor = NS_RGB((int)min(NS_GET_R(bcolor) + 40, 255), 
                        (int)min(NS_GET_G(bcolor) + 40, 255),
                        (int)min(NS_GET_B(bcolor) + 40, 255));

        rc.SetColor(tcolor);

        rc.FillEllipse(NSToCoordRound(brect.width * 0.34f),  // XXX should use NSToCoordCeil ??
                       NSToCoordRound(brect.height * 0.34f), // XXX should use NSToCoordCeil ??
                       nscoord(brect.width * 0.28f),
                       nscoord(brect.height * 0.28f));

        tcolor = NS_RGB((int)min(NS_GET_R(bcolor) + 120, 255), 
                        (int)min(NS_GET_G(bcolor) + 120, 255),
                        (int)min(NS_GET_B(bcolor) + 120, 255));

        rc.SetColor(tcolor);

        rc.FillEllipse(NSToCoordRound(brect.width * 0.32f),  // XXX should use NSToCoordCeil ??
                       NSToCoordRound(brect.height * 0.32f), // XXX should use NSToCoordCeil ??
                       nscoord(brect.width * 0.17f),
                       nscoord(brect.height * 0.17f));
      }
    }

    clipres = rc.PopState();

    if (clipres == PR_FALSE)
    {
      nsRect  xrect = brect;

      xrect.x += xoff;
      xrect.y += yoff;

      clipres = rc.SetClipRect(xrect, nsClipCombine_kSubtract);
    }
  }

  return clipres;
}

static NS_DEFINE_IID(kIViewIID, NS_IVIEW_IID);

nsScrollingView :: nsScrollingView()
{
  mSizeX = mSizeY = 0;
  mOffsetX = mOffsetY = 0;
  mVScrollBarView = nsnull;
  mHScrollBarView = nsnull;
  mCornerView = nsnull;
  mScrollPref = nsScrollPreference_kAuto;
  mClipX = mClipY = 0;
  mScrollingTimer = nsnull;
}

nsScrollingView :: ~nsScrollingView()
{
  if (nsnull != mVScrollBarView)
  {
    // Clear the back-pointer from the scrollbar...
    ((ScrollBarView*)mVScrollBarView)->mScrollingView = nsnull;
  }

  if (nsnull != mHScrollBarView)
  {
    // Clear the back-pointer from the scrollbar...
    ((ScrollBarView*)mHScrollBarView)->mScrollingView = nsnull;
  }

  if (nsnull != mCornerView)
  {
    mCornerView = nsnull;
  }
}

nsresult nsScrollingView :: QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  if (nsnull == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  
  static NS_DEFINE_IID(kClassIID, NS_ISCROLLABLEVIEW_IID);

  if (aIID.Equals(kClassIID)) {
    *aInstancePtr = (void*)(nsIScrollableView*)this;
    return NS_OK;
  }

  return nsView::QueryInterface(aIID, aInstancePtr);
}

nsrefcnt nsScrollingView :: AddRef()
{
  NS_WARNING("not supported for views");
  return 1;
}

nsrefcnt nsScrollingView :: Release()
{
  NS_WARNING("not supported for views");
  return 1;
}

nsresult nsScrollingView :: Init(nsIViewManager* aManager,
					const nsRect &aBounds,
					nsIView *aParent,
					const nsIID *aWindowIID,
          nsWidgetInitData *aWidgetInitData,
					nsNativeWidget aNative,
					PRInt32 aZIndex,
					const nsViewClip *aClip,
					float aOpacity,
					nsViewVisibility aVisibilityFlag)
{
  nsresult rv;

  mClipX = aBounds.width;
  mClipY = aBounds.height;
  
  rv = nsView :: Init(aManager, aBounds, aParent, aWindowIID, aWidgetInitData, aNative, aZIndex, aClip, aOpacity, aVisibilityFlag);

  if (rv == NS_OK)
  {
    nsIPresContext    *cx = mViewManager->GetPresContext();
    nsIDeviceContext  *dx = cx->GetDeviceContext();

    // Create a view for a corner cover

    mCornerView = new CornerView();

    if (nsnull != mCornerView)
    {
      nsRect trect;

      trect.width = NSToCoordRound(dx->GetScrollBarWidth());
      trect.x = aBounds.x + aBounds.XMost() - trect.width;
      trect.height = NSToCoordRound(dx->GetScrollBarHeight());
      trect.y = aBounds.y + aBounds.YMost() - trect.height;

      rv = mCornerView->Init(mViewManager, trect, this, nsnull, nsnull, nsnull, -1, nsnull, 1.0f, nsViewVisibility_kHide);

      mViewManager->InsertChild(this, mCornerView, -1);
    }

    // Create a view for a vertical scrollbar

    mVScrollBarView = new ScrollBarView(this);

    if (nsnull != mVScrollBarView)
    {
      nsRect trect = aBounds;

      trect.width = NSToCoordRound(dx->GetScrollBarWidth());
      trect.x += aBounds.XMost() - trect.width;
      trect.height -= NSToCoordRound(dx->GetScrollBarHeight());

      static NS_DEFINE_IID(kCScrollbarIID, NS_VERTSCROLLBAR_CID);

      rv = mVScrollBarView->Init(mViewManager, trect, this, &kCScrollbarIID, nsnull, aNative, -3);

      mViewManager->InsertChild(this, mVScrollBarView, -3);
    }

    // Create a view for a horizontal scrollbar

    mHScrollBarView = new ScrollBarView(this);

    if (nsnull != mHScrollBarView)
    {
      nsRect trect = aBounds;

      trect.height = NSToCoordRound(dx->GetScrollBarHeight());
      trect.y += aBounds.YMost() - trect.height;
      trect.width -= NSToCoordRound(dx->GetScrollBarWidth());

      static NS_DEFINE_IID(kCHScrollbarIID, NS_HORZSCROLLBAR_CID);

      rv = mHScrollBarView->Init(mViewManager, trect, this, &kCHScrollbarIID, nsnull, aNative, -3);

      mViewManager->InsertChild(this, mHScrollBarView, -3);
    }

    NS_RELEASE(dx);
    NS_RELEASE(cx);
  }

  return rv;
}

void nsScrollingView :: SetDimensions(nscoord width, nscoord height)
{
  nsRect            trect;
  nsIPresContext    *cx = mViewManager->GetPresContext();
  nsIDeviceContext  *dx = cx->GetDeviceContext();
  nscoord           showHorz = 0, showVert = 0;
  nscoord           scrollWidth = NSToCoordRound(dx->GetScrollBarWidth());
  nscoord           scrollHeight = NSToCoordRound(dx->GetScrollBarHeight());

  if (nsnull != mCornerView)
  {
    mCornerView->GetDimensions(&trect.width, &trect.height);

    trect.y = height - scrollHeight;
    trect.x = width - scrollWidth;

    mCornerView->SetBounds(trect);
  }

  if (mHScrollBarView && (mHScrollBarView->GetVisibility() == nsViewVisibility_kShow))
    showHorz = scrollHeight;

  if (mVScrollBarView && (mVScrollBarView->GetVisibility() == nsViewVisibility_kShow))
    showVert = scrollWidth;

//  nsView :: SetDimensions(width, height);

  mBounds.SizeTo(width, height);

  if (nsnull != mWindow)
  {
    float t2p = cx->GetTwipsToPixels();

    mClipX = width - showVert;
    mClipY = height - showHorz;
  
    mWindow->Resize(NSTwipsToIntPixels((width - showVert), t2p),
                    NSTwipsToIntPixels((height - showHorz), t2p),
                    PR_TRUE);
  }
  else
  {
    mClipX = width;
    mClipY = height;
  }

  if (nsnull != mVScrollBarView)
  {
    mVScrollBarView->GetDimensions(&trect.width, &trect.height);

    trect.height = height;

    if (showHorz || (mCornerView && (mCornerView->GetVisibility() == nsViewVisibility_kShow)))
      trect.height -= scrollHeight;

    trect.x = width - scrollWidth;
    trect.y = 0;

    mVScrollBarView->SetBounds(trect);
  }

  if (nsnull != mHScrollBarView)
  {
    mHScrollBarView->GetDimensions(&trect.width, &trect.height);

    trect.width = width;

    if (showVert || (mCornerView && (mCornerView->GetVisibility() == nsViewVisibility_kShow)))
      trect.width -= scrollWidth;

    trect.y = height - scrollHeight;
    trect.x = 0;

    mHScrollBarView->SetBounds(trect);
  }

  //this will fix the size of the thumb when we resize the root window,
  //but unfortunately it will also cause scrollbar flashing. so long as
  //all resize operations happen through the viewmanager, this is not
  //an issue. we'll see. MMP
//  ComputeContainerSize();

  NS_RELEASE(dx);
  NS_RELEASE(cx);
}

void nsScrollingView :: SetPosition(nscoord aX, nscoord aY)
{
  nsIPresContext  *px = mViewManager->GetPresContext();
  nsIWidget       *thiswin = GetWidget();

  if (nsnull == thiswin)
    thiswin = GetOffsetFromWidget(nsnull, nsnull);

  if (nsnull != thiswin)
    thiswin->BeginResizingChildren();

  nsView::SetPosition(aX, aY);

  AdjustChildWidgets(this, this, 0, 0, px->GetTwipsToPixels());

  if (nsnull != thiswin)
  {
    thiswin->EndResizingChildren();
    NS_RELEASE(thiswin);
  }

  NS_RELEASE(px);
}

PRBool nsScrollingView :: Paint(nsIRenderingContext& rc, const nsRect& rect,
                                PRUint32 aPaintFlags, nsIView *aBackstop)
{
  PRBool  clipres = PR_FALSE;
  nsRect  brect;

  rc.PushState();

  GetBounds(brect);

  if (mVis == nsViewVisibility_kShow)
    clipres = rc.SetClipRect(brect, nsClipCombine_kIntersect);

  if (clipres == PR_FALSE)
  {
    rc.Translate(-mOffsetX, -mOffsetY);
    clipres = nsView::Paint(rc, rect, aPaintFlags | NS_VIEW_FLAG_CLIP_SET, aBackstop);
  }

  clipres = rc.PopState();

  if ((clipres == PR_FALSE) && (mVis == nsViewVisibility_kShow) && (nsnull == mWindow))
    clipres = rc.SetClipRect(brect, nsClipCombine_kSubtract);

  return clipres;
}

void nsScrollingView :: HandleScrollEvent(nsGUIEvent *aEvent, PRUint32 aEventFlags)
{
  nsIView         *scview = nsView::GetViewFor(aEvent->widget);
  nsIPresContext  *px = mViewManager->GetPresContext();
  float           scale = px->GetTwipsToPixels();
  nscoord         dx = 0, dy = 0;
  nsRect          bounds;

  GetBounds(bounds);

  if ((nsnull != mVScrollBarView) && (scview == mVScrollBarView))
  {
    nscoord oy = mOffsetY;
    nscoord newpos;

    //now, this horrible thing makes sure that as we scroll
    //the document a pixel at a time, we keep the logical position of
    //our scroll bar at the top edge of the same pixel that
    //is displayed.

    newpos = ((nsScrollbarEvent *)aEvent)->position;

    if ((newpos + bounds.height) > mSizeY)
      newpos = mSizeY - bounds.height;

    mOffsetY = NSIntPixelsToTwips(NSTwipsToIntPixels(newpos, scale), px->GetPixelsToTwips());

    dy = NSTwipsToIntPixels((oy - mOffsetY), scale);

    if (dy != 0)
    {
      nscoord sx, sy;

      mVScrollBarView->GetDimensions(&sx, &sy);

      if ((nsnull != mHScrollBarView) && (mHScrollBarView->GetVisibility() == nsViewVisibility_kShow))
        mHScrollBarView->GetDimensions(&sx, &sy);
      else
        sy = 0;

      mViewManager->ClearDirtyRegion();

      nsIWidget *thiswin = GetWidget();

      if (nsnull == thiswin)
        thiswin = GetOffsetFromWidget(nsnull, nsnull);

      if (nsnull != thiswin)
        thiswin->BeginResizingChildren();

      //and now we make sure that the scrollbar thumb is in sync with the
      //numbers we came up with here, but only if we actually moved at least
      //a full pixel. if didn't adjust the thumb only if the delta is non-zero,
      //very slow scrolling would never actually work.

      ((nsScrollbarEvent *)aEvent)->position = mOffsetY;

      if (dy != 0)
      {
        if (nsnull != mWindow)
          mWindow->Scroll(0, dy, nsnull);
        else
          mViewManager->UpdateView(this, nsnull, 0);
      }

      if (nsnull != thiswin)
      {
        thiswin->EndResizingChildren();
        NS_RELEASE(thiswin);
      }
    }
  }
  else if ((nsnull != mHScrollBarView) && (scview == mHScrollBarView))
  {
    nscoord ox = mOffsetX;
    nscoord newpos;

    //now, this horrible thing makes sure that as we scroll
    //the document a pixel at a time, we keep the logical position of
    //our scroll bar at the top edge of the same pixel that
    //is displayed.

    newpos = ((nsScrollbarEvent *)aEvent)->position;

    if ((newpos + bounds.width) > mSizeX)
      newpos = mSizeX - bounds.width;

    mOffsetX = NSIntPixelsToTwips(NSTwipsToIntPixels(newpos, scale), px->GetPixelsToTwips());

    dx = NSTwipsToIntPixels((ox - mOffsetX), scale);

    if (dx != 0)
    {
      nscoord sx, sy;

      if ((nsnull != mVScrollBarView) && (mVScrollBarView->GetVisibility() == nsViewVisibility_kShow))
        mVScrollBarView->GetDimensions(&sx, &sy);
      else
        sx = 0;

      mHScrollBarView->GetDimensions(&sx, &sy);

      mViewManager->ClearDirtyRegion();

      nsIWidget *thiswin = GetWidget();

      if (nsnull == thiswin)
        thiswin = GetOffsetFromWidget(nsnull, nsnull);

      if (nsnull != thiswin)
        thiswin->BeginResizingChildren();

      //and now we make sure that the scrollbar thumb is in sync with the
      //numbers we came up with here, but only if we actually moved at least
      //a full pixel. if didn't adjust the thumb only if the delta is non-zero,
      //very slow scrolling would never actually work.

      ((nsScrollbarEvent *)aEvent)->position = mOffsetX;

      if (dx != 0)
      {
        if (nsnull != mWindow)
          mWindow->Scroll(dx, 0, nsnull);
        else
          mViewManager->UpdateView(this, nsnull, 0);
      }

      if (nsnull != thiswin)
      {
        thiswin->EndResizingChildren();
        NS_RELEASE(thiswin);
      }
    }
  }

  NS_RELEASE(px);
}

void nsScrollingView :: Notify(nsITimer * aTimer)
{
  nscoord xoff, yoff;
  nsIView *view = GetScrolledView();

  // First do the scrolling of the view

  view->GetScrollOffset(&xoff, &yoff);

  nscoord newPos = yoff + mScrollingDelta;

  if (newPos < 0)
    newPos = 0;

  ScrollTo(0, newPos, 0);

  // Now fake a mouse event so the frames can process the selection event

  nsRect        rect;
  nsGUIEvent    event;
  nsEventStatus retval;

  event.message = NS_MOUSE_MOVE;

  nsIPresContext  *cx   = mViewManager->GetPresContext();

  GetBounds(rect);

  event.point.x = rect.x;
  event.point.y = (mScrollingDelta > 0) ? (rect.height - rect.y - 1) : 135;

  //printf("timer %d %d\n", event.point.x, event.point.y);

  mFrame->HandleEvent(*cx, &event, retval);

  NS_RELEASE(cx);
  NS_RELEASE(mScrollingTimer);

  if (NS_OK == NS_NewTimer(&mScrollingTimer))
    mScrollingTimer->Init(this, 25);
}

nsEventStatus nsScrollingView :: HandleEvent(nsGUIEvent *aEvent, PRUint32 aEventFlags)
{
  switch (aEvent->message)
  {
    case NS_MOUSE_LEFT_BUTTON_DOWN:
    case NS_MOUSE_MIDDLE_BUTTON_DOWN:
    case NS_MOUSE_RIGHT_BUTTON_DOWN: 
    {
      nsIWidget *win = GetWidget();

      if (nsnull != win) 
      {
        win->SetFocus();
        NS_RELEASE(win);
      }

      break;
    }

    case NS_KEY_DOWN:
    {
      nsKeyEvent * keyEvent = (nsKeyEvent *)aEvent;
      switch (keyEvent->keyCode) {
        case NS_VK_PAGE_DOWN : 
        case NS_VK_PAGE_UP   : {
          nsIScrollbar  *scrollv = nsnull, *scrollh = nsnull;
          nsIWidget     *win = mVScrollBarView->GetWidget();

          if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollv))
          {
            PRUint32  oldpos = scrollv->GetPosition();
            nsRect rect;
            GetBounds(rect);
            nscoord newPos = 0;
            if (keyEvent->keyCode == NS_VK_PAGE_DOWN) {
              newPos = oldpos+rect.height;
            } else {
              newPos = oldpos-rect.height;
              newPos = (newPos < 0 ? 0 : newPos);
            }
            ScrollTo(0, newPos, 0);
          }

        } break;

        case NS_VK_DOWN : 
        case NS_VK_UP   : {
          nsIScrollbar  *scrollv = nsnull, *scrollh = nsnull;
          nsIWidget     *win = mVScrollBarView->GetWidget();

          if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollv))
          {
            PRUint32  oldpos  = scrollv->GetPosition();
            PRUint32  lineInc = scrollv->GetLineIncrement();
            nscoord newPos = 0;
            if (keyEvent->keyCode == NS_VK_DOWN) {
              newPos = oldpos+lineInc;
            } else {
              newPos = oldpos-lineInc;
              newPos = (newPos < 0 ? 0 : newPos);
            }
            ScrollTo(0, newPos, 0);
          }

        } break;

        default:
          break;

      } // switch
    } break;

    case NS_MOUSE_MOVE:
    {
      nsRect  trect;
      nscoord lx, ly;

      GetBounds(trect);

      lx = aEvent->point.x - trect.x;
      ly = aEvent->point.y - trect.y;

      //nscoord         xoff, yoff;
      //GetScrolledView()->GetScrollOffset(&xoff, &yoff);
      //printf("%d %d   %d\n", trect.y, trect.height, yoff);
      //printf("mouse %d %d \n", aEvent->point.x, aEvent->point.y);

      if (!trect.Contains(lx, ly))
      {
        if (mScrollingTimer == nsnull)
        {
          if (nsnull != mFrame)
          {
            if (ly < 0 || ly > trect.y)
            {
              mScrollingDelta = ly < 0 ? -100 : 100;
              NS_NewTimer(&mScrollingTimer);
              mScrollingTimer->Init(this, 25);
            }
          }
        }
      }
      else if (mScrollingTimer != nsnull)
      {
        mScrollingTimer->Cancel();
        NS_RELEASE(mScrollingTimer);
      }
      break;
    }

    case NS_MOUSE_LEFT_BUTTON_UP:
    case NS_MOUSE_MIDDLE_BUTTON_UP:
    case NS_MOUSE_RIGHT_BUTTON_UP: 
    {
      if (mScrollingTimer != nsnull)
      {
        mScrollingTimer->Cancel();
        NS_RELEASE(mScrollingTimer);
        mScrollingTimer = nsnull;
      }

      nsRect  trect;
      nscoord lx, ly;

      GetBounds(trect);

      lx = aEvent->point.x - trect.x;
      ly = aEvent->point.y - trect.y;

      if (!trect.Contains(lx, ly))
      {
        nsEventStatus retval;

        if (nsnull != mFrame)
        {
          nsIPresContext  *cx = mViewManager->GetPresContext();
          nscoord         xoff, yoff;

          GetScrollOffset(&xoff, &yoff);

          aEvent->point.x += xoff;
          aEvent->point.y += yoff;

          mFrame->HandleEvent(*cx, aEvent, retval);

          aEvent->point.x -= xoff;
          aEvent->point.y -= yoff;

          NS_RELEASE(cx);
        }
      }
      break;
    }

    default:
      break;
  }

  return nsView::HandleEvent(aEvent, aEventFlags);
}

void nsScrollingView :: ComputeContainerSize()
{
  nsIView       *scrollview = GetScrolledView();
  nsIScrollbar  *scrollv = nsnull, *scrollh = nsnull;
  nsIWidget     *win;

  if (nsnull != scrollview)
  {
    nscoord         dx = 0, dy = 0;
    nsIPresContext  *px = mViewManager->GetPresContext();
    nscoord         hwidth, hheight;
    nscoord         vwidth, vheight;
    PRUint32        oldsizey = mSizeY, oldsizex = mSizeX;
    nsRect          area(0, 0, 0, 0);
    nscoord         offx, offy;
    float           scale = px->GetTwipsToPixels();

    ComputeScrollArea(scrollview, area, 0, 0);

    mSizeY = area.YMost();
    mSizeX = area.XMost();

    if (nsnull != mHScrollBarView)
    {
      mHScrollBarView->GetDimensions(&hwidth, &hheight);
      win = mHScrollBarView->GetWidget();

      if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollh))
      {
        if (((mSizeX > mBounds.width) &&
            (mScrollPref != nsScrollPreference_kNeverScroll)) ||
            (mScrollPref == nsScrollPreference_kAlwaysScroll))
          scrollh->Release(); //DO NOT USE NS_RELEASE()! MMP
        else
          NS_RELEASE(scrollh); //MUST USE NS_RELEASE()! MMP
      }

      NS_RELEASE(win);
    }

    if (nsnull != mVScrollBarView)
    {
      mVScrollBarView->GetDimensions(&vwidth, &vheight);
      offy = mOffsetY;

      win = mVScrollBarView->GetWidget();

      if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollv))
      {
        if ((mSizeY > mBounds.height) && (mScrollPref != nsScrollPreference_kNeverScroll))
        {
          //we need to be able to scroll

          mVScrollBarView->SetVisibility(nsViewVisibility_kShow);
          win->Enable(PR_TRUE);

          //now update the scroller position for the new size

          PRUint32  oldpos = scrollv->GetPosition();

          mOffsetY = NSIntPixelsToTwips(NSTwipsToIntPixels(nscoord(((float)oldpos * mSizeY) / oldsizey), scale), px->GetPixelsToTwips());

          dy = NSTwipsToIntPixels((offy - mOffsetY), scale);

          scrollv->SetParameters(mSizeY, mBounds.height - ((nsnull != scrollh) ? hheight : 0),
                                 mOffsetY, NSIntPointsToTwips(12));
        }
        else
        {
          mOffsetY = 0;
          dy = NSTwipsToIntPixels(offy, scale);

          if (mScrollPref == nsScrollPreference_kAlwaysScroll)
          {
            mVScrollBarView->SetVisibility(nsViewVisibility_kShow);
            win->Enable(PR_FALSE);
          }
          else
          {
            mVScrollBarView->SetVisibility(nsViewVisibility_kHide);
            win->Enable(PR_TRUE);
            NS_RELEASE(scrollv);
          }
        }

        //don't release the vertical scroller here because if we need to
        //create a horizontal one, it will need to know that there is a vertical one
//        //create a horizontal one, it will need to tweak the vertical one
      }

      NS_RELEASE(win);
    }

    if (nsnull != mHScrollBarView)
    {
      offx = mOffsetX;

      win = mHScrollBarView->GetWidget();

      if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollh))
      {
        if ((mSizeX > mBounds.width) && (mScrollPref != nsScrollPreference_kNeverScroll))
        {
          //we need to be able to scroll

          mHScrollBarView->SetVisibility(nsViewVisibility_kShow);
          win->Enable(PR_TRUE);

          //now update the scroller position for the new size

          PRUint32  oldpos = scrollh->GetPosition();

          mOffsetX = NSIntPixelsToTwips(NSTwipsToIntPixels(nscoord(((float)oldpos * mSizeX) / oldsizex), scale), px->GetPixelsToTwips());

          dx = NSTwipsToIntPixels((offx - mOffsetX), scale);

          scrollh->SetParameters(mSizeX, mBounds.width - ((nsnull != scrollv) ? vwidth : 0),
                                 mOffsetX, NSIntPointsToTwips(12));

//          //now make the vertical scroll region account for this scrollbar
//
//          if (nsnull != scrollv)
//            scrollv->SetParameters(mSizeY, mBounds.height - hheight, mOffsetY, NSIntPointsToTwips(12));
        }
        else
        {
          mOffsetX = 0;
          dx = NSTwipsToIntPixels(offx, scale);

          if (mScrollPref == nsScrollPreference_kAlwaysScroll)
          {
            mHScrollBarView->SetVisibility(nsViewVisibility_kShow);
            win->Enable(PR_FALSE);
          }
          else
          {
            mHScrollBarView->SetVisibility(nsViewVisibility_kHide);
            win->Enable(PR_TRUE);
          }
        }

        NS_RELEASE(scrollh);
      }

      NS_RELEASE(win);
    }

    if (mCornerView)
    {
      if ((mHScrollBarView && (mHScrollBarView->GetVisibility() == nsViewVisibility_kShow)) &&
          (mVScrollBarView && (mVScrollBarView->GetVisibility() == nsViewVisibility_kShow)))
        ((CornerView *)mCornerView)->Show(PR_TRUE);
      else
        ((CornerView *)mCornerView)->Show(PR_FALSE);
    }

    // now we can release the vertical scroller if there was one...

    NS_IF_RELEASE(scrollv);

//    if ((dx != 0) || (dy != 0))
//      AdjustChildWidgets(this, this, 0, 0, px->GetTwipsToPixels());

    NS_RELEASE(px);
  }
  else
  {
    if (nsnull != mHScrollBarView)
    {
      mHScrollBarView->SetVisibility(nsViewVisibility_kHide);

      win = mHScrollBarView->GetWidget();

      if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollh))
      {
        scrollh->SetParameters(0, 0, 0, 0);
        NS_RELEASE(scrollh);
      }

      NS_RELEASE(win);
    }

    if (nsnull != mVScrollBarView)
    {
      mVScrollBarView->SetVisibility(nsViewVisibility_kHide);

      win = mVScrollBarView->GetWidget();

      if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollv))
      {
        scrollv->SetParameters(0, 0, 0, 0);
        NS_RELEASE(scrollv);
      }

      NS_RELEASE(win);
    }

    if (nsnull != mCornerView)
      ((CornerView *)mCornerView)->Show(PR_FALSE);

    mOffsetX = mOffsetY = 0;
    mSizeX = mSizeY = 0;
  }
}

void nsScrollingView :: GetContainerSize(nscoord *aWidth, nscoord *aHeight)
{
  *aWidth = mSizeX;
  *aHeight = mSizeY;
}

void nsScrollingView :: SetVisibleOffset(nscoord aOffsetX, nscoord aOffsetY)
{
  mOffsetX = aOffsetX;
  mOffsetY = aOffsetY;
}

void nsScrollingView :: GetVisibleOffset(nscoord *aOffsetX, nscoord *aOffsetY)
{
  *aOffsetX = mOffsetX;
  *aOffsetY = mOffsetY;
}

void nsScrollingView :: ShowQuality(PRBool aShow)
{
  ((CornerView *)mCornerView)->ShowQuality(aShow);
}

PRBool nsScrollingView :: GetShowQuality(void)
{
  return ((CornerView *)mCornerView)->mShowQuality;
}

void nsScrollingView :: SetQuality(nsContentQuality aQuality)
{
  ((CornerView *)mCornerView)->SetQuality(aQuality);
}

void nsScrollingView :: SetScrollPreference(nsScrollPreference aPref)
{
  mScrollPref = aPref;
  ComputeContainerSize();
}

nsScrollPreference nsScrollingView :: GetScrollPreference(void)
{
  return mScrollPref;
}

// XXX This doesn't do X scrolling yet

// XXX This doesn't let the scrolling code slide the bits on the
// screen and damage only the appropriate area

// XXX doesn't smooth scroll

NS_IMETHODIMP
nsScrollingView :: ScrollTo(nscoord aX, nscoord aY, PRUint32 aUpdateFlags)
{
  nsIPresContext  *px = mViewManager->GetPresContext();
  float           t2p = px->GetTwipsToPixels();
  float           p2t = px->GetPixelsToTwips();
  NS_RELEASE(px);

  nsIWidget*      win;
  win = mVScrollBarView->GetWidget();
  if (nsnull != win)
  {
    nsIScrollbar* scrollv;
    if (NS_OK == win->QueryInterface(kIScrollbarIID, (void **)&scrollv))
    {
      // Clamp aY
      nsRect r;
      GetBounds(r);
      if (aY + r.height > mSizeY) {
        aY = mSizeY - r.height;
        if (aY < 0) {
          aY = 0;
        }
      }

      // Move the scrollbar's thumb

      PRUint32  oldpos = mOffsetY;
      nscoord dy;

      PRUint32 newpos =
        NSIntPixelsToTwips(NSTwipsToIntPixels(aY, t2p), p2t);
      scrollv->SetPosition(newpos);

      dy = oldpos - newpos;

      // Update offsets
      SetVisibleOffset(aX, aY);

      AdjustChildWidgets(this, this, 0, 0, t2p);

      // Damage the updated area
      r.x = 0;
      r.y = aY;
      nsIView* scrolledView = GetScrolledView();
      if (nsnull != scrolledView)
      {
        mViewManager->UpdateView(scrolledView, r, aUpdateFlags);
      }

      NS_RELEASE(scrollv);
    }
    NS_RELEASE(win);
  }
  return NS_OK;
}

NS_IMETHODIMP nsScrollingView :: GetClipSize(nscoord *aX, nscoord *aY)
{
  *aX = mClipX;
  *aY = mClipY;

  return NS_OK;
}

void nsScrollingView :: AdjustChildWidgets(nsScrollingView *aScrolling, nsIView *aView, nscoord aDx, nscoord aDy, float scale)
{
  PRInt32           numkids = aView->GetChildCount();
  nsIScrollableView *scroller;
  nscoord           offx, offy;
  PRBool            isscroll = PR_FALSE;

  if (aScrolling == aView)
  {
    nsIWidget *widget = aScrolling->GetOffsetFromWidget(&aDx, &aDy);
    nsIView   *parview = aScrolling->GetParent();

    while (nsnull != parview)
    {
      nsIWidget *parwidget = parview->GetWidget();

      if (NS_OK == parview->QueryInterface(kIScrollableViewIID, (void **)&scroller))
      {
        scroller->GetVisibleOffset(&offx, &offy);

        aDx -= offx;
        aDy -= offy;
      }

      if (parwidget == widget)
      {
        NS_IF_RELEASE(parwidget);
        break;
      }

      NS_IF_RELEASE(parwidget);

      parview = parview->GetParent();
    }

    NS_IF_RELEASE(widget);
  }

  aView->GetPosition(&offx, &offy);

  aDx += offx;
  aDy += offy;

  if (NS_OK == aView->QueryInterface(kIScrollableViewIID, (void **)&scroller))
  {
    scroller->GetVisibleOffset(&offx, &offy);

    aDx -= offx;
    aDy -= offy;

    isscroll = PR_TRUE;
  }

  for (PRInt32 cnt = 0; cnt < numkids; cnt++)
  {
    nsIView   *kid = aView->GetChild(cnt);
    nsIWidget *win = kid->GetWidget();

    if (nsnull != win)
    {
      nsRect  bounds;

      win->BeginResizingChildren();
      kid->GetBounds(bounds);

      if (!isscroll ||
          (isscroll &&
          (kid != ((nsScrollingView *)aView)->mVScrollBarView) &&
          (kid != ((nsScrollingView *)aView)->mHScrollBarView)))
        win->Move(NSTwipsToIntPixels((bounds.x + aDx), scale), NSTwipsToIntPixels((bounds.y + aDy), scale));
      else
        win->Move(NSTwipsToIntPixels((bounds.x + aDx + offx), scale), NSTwipsToIntPixels((bounds.y + aDy + offy), scale));
    }

    AdjustChildWidgets(aScrolling, kid, aDx, aDy, scale);

    if (nsnull != win)
    {
      win->EndResizingChildren();
      NS_RELEASE(win);
    }
  }
}

nsIView * nsScrollingView :: GetScrolledView(void)
{
  PRInt32 numkids;
  nsIView *retview = nsnull;

  numkids = GetChildCount();

  for (PRInt32 cnt = 0; cnt < numkids; cnt++)
  {
    retview = GetChild(cnt);

    if ((retview != mVScrollBarView) &&
        (retview != mHScrollBarView) &&
        (retview != mCornerView))
      break;
    else
      retview = nsnull;
  }

  return retview;
}

void nsScrollingView :: ComputeScrollArea(nsIView *aView, nsRect &aRect,
                                          nscoord aOffX, nscoord aOffY)
{
  nsRect  trect, vrect;

  aView->GetBounds(vrect);

  aOffX += vrect.x;
  aOffY += vrect.y;

  trect.x = aOffX;
  trect.y = aOffY;
  trect.width = vrect.width;
  trect.height = vrect.height;

  if (aRect.IsEmpty() == PR_TRUE)
    aRect = trect;
  else
    aRect.UnionRect(aRect, trect);

  PRInt32 numkids = aView->GetChildCount();
  
  for (PRInt32 cnt = 0; cnt < numkids; cnt++)
  {
    nsIView *view = aView->GetChild(cnt);
    ComputeScrollArea(view, aRect, aOffX, aOffY);
  }
}
