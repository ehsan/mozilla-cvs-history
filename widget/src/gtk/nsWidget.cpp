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

#include "nsWidget.h"
#include "nsIDeviceContext.h"
#include "nsIAppShell.h"
#include "nsGfxCIID.h"
#include "nsRepository.h"
#include <gdk/gdkx.h>


nsWidget::nsWidget()
{
  // XXX Shouldn't this be done in nsBaseWidget? 
  NS_INIT_REFCNT();
  mGC = nsnull;
  mWidget = nsnull;
  mParent = nsnull;
  mPreferredWidth  = 0;
  mPreferredHeight = 0;
  mShown = PR_FALSE;
  mBounds.x = 0;
  mBounds.y = 0;
  mBounds.width = 0;
  mBounds.height = 0;
}

nsWidget::~nsWidget()
{
}

NS_METHOD nsWidget::WidgetToScreen(const nsRect& aOldRect, nsRect& aNewRect)
{
    NS_NOTYETIMPLEMENTED("nsWidget::WidgetToScreen");
    return NS_OK;
}

NS_METHOD nsWidget::ScreenToWidget(const nsRect& aOldRect, nsRect& aNewRect)
{
    NS_NOTYETIMPLEMENTED("nsWidget::ScreenToWidget");
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Close this nsWidget
//
//-------------------------------------------------------------------------

NS_IMETHODIMP nsWidget::Destroy(void)
{
  ::gtk_widget_destroy(mWidget);
}

//-------------------------------------------------------------------------
//
// Get this nsWidget parent
//
//-------------------------------------------------------------------------

nsIWidget *nsWidget::GetParent(void)
{
//  NS_NOTYETIMPLEMENTED("nsWidget::GetParent");
  return mParent;
}

//-------------------------------------------------------------------------
//
// Hide or show this component
//
//-------------------------------------------------------------------------

NS_METHOD nsWidget::Show(PRBool bState)
{
    g_print("nsWidget::Show(%6d)    - %s %p\n", bState, mWidget->name, this);

    if (bState) {
      if (mWidget) {
        gtk_widget_show(mWidget);
      } else {
#ifdef DEBUG_shaver
        g_print("showing a NULL-be-widgeted widget @ %p\n", this);
#endif
        return NS_ERROR_NULL_POINTER;
      }
    } else {
    if (mWidget) {
      gtk_widget_hide(mWidget);
    } else {
#ifdef DEBUG_shaver
    g_print("hiding a NULL-be-widgeted widget @ %p\n", this);
#endif
    return NS_ERROR_NULL_POINTER;
    }
  }
  mShown = bState;
  return NS_OK;
}

NS_METHOD nsWidget::IsVisible(PRBool &aState)
{
    aState = mShown;
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Move this component
//
//-------------------------------------------------------------------------

NS_METHOD nsWidget::Move(PRUint32 aX, PRUint32 aY)
{
  g_print("nsWidget::Move(%3d,%3d)   - %s %p\n", aX, aY, mWidget->name, this);
  mBounds.x = aX;
  mBounds.y = aY;
  gtk_layout_move(GTK_LAYOUT(mWidget->parent), mWidget, aX, aY);
  //XtVaSetValues(mWidget, XmNx, aX, XmNy, GetYCoord(aY), nsnull);
  return NS_OK;
}

NS_METHOD nsWidget::Resize(PRUint32 aWidth, PRUint32 aHeight, PRBool aRepaint)
{
  g_print("nsWidget::Resize(%3d,%3d) - %s %p\n", aWidth, aHeight, mWidget->name, this);
  mBounds.width  = aWidth;
  mBounds.height = aHeight;
  gtk_widget_set_usize(mWidget, aWidth, aHeight);
  return NS_OK;
}

NS_METHOD nsWidget::Resize(PRUint32 aX, PRUint32 aY, PRUint32 aWidth,
			   PRUint32 aHeight, PRBool aRepaint)
{
    Resize(aWidth, aHeight, aRepaint);
    Move(aX, aY);
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Enable/disable this component
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::Enable(PRBool bState)
{
    gtk_widget_set_sensitive(mWidget, bState);
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Give the focus to this component
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::SetFocus(void)
{
    gtk_widget_grab_focus(mWidget);
    return NS_OK;
}

NS_METHOD nsWidget::GetBounds(nsRect &aRect)
{
    aRect = mBounds;
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get the foreground color
//
//-------------------------------------------------------------------------
nscolor nsWidget::GetForegroundColor(void)
{
    /* can we safely cache this? */
    return mForeground;
}

//-------------------------------------------------------------------------
//
// Set the foreground color
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::SetForegroundColor(const nscolor &aColor)
{
    mForeground = aColor;
    NS_NOTYETIMPLEMENTED("nsWidget::SetForegroundColor");
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get the background color
//
//-------------------------------------------------------------------------
nscolor nsWidget::GetBackgroundColor(void)
{
    /* can we safely cache this? */
    return mBackground;
}

//-------------------------------------------------------------------------
//
// Set the background color
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::SetBackgroundColor(const nscolor &aColor)
{
    mBackground = aColor;
    NS_NOTYETIMPLEMENTED("nsWidget::SetBackgroundColor");
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get this component font
//
//-------------------------------------------------------------------------
nsIFontMetrics *nsWidget::GetFont(void)
{
    NS_NOTYETIMPLEMENTED("nsWidget::GetFont");
    return nsnull;
}

//-------------------------------------------------------------------------
//
// Set this component font
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::SetFont(const nsFont &aFont)
{
    NS_NOTYETIMPLEMENTED("nsWidget::SetFont");
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get this component cursor
//
//-------------------------------------------------------------------------
nsCursor nsWidget::GetCursor(void)
{
    NS_NOTYETIMPLEMENTED("nsWidget::GetCursor");
    return eCursor_standard;
}

//-------------------------------------------------------------------------
//
// Set this component cursor
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::SetCursor(nsCursor aCursor)
{
  if (!mWidget || !mWidget->window)
    return NS_ERROR_FAILURE;
  
  // Only change cursor if it's changing
  if (aCursor != mCursor) {
    GdkCursor *newCursor = 0;
    
    switch(aCursor) {
	  case eCursor_select:
	    newCursor = gdk_cursor_new(GDK_XTERM);
	    break;
      
	  case eCursor_wait:
	    newCursor = gdk_cursor_new(GDK_WATCH);
	    break;
      
	  case eCursor_hyperlink:
	    newCursor = gdk_cursor_new(GDK_HAND2);
	    break;

	  case eCursor_standard:
	    newCursor = gdk_cursor_new(GDK_LEFT_PTR);
	    break;

	  case eCursor_arrow_south:
	  case eCursor_arrow_south_plus:
	    newCursor = gdk_cursor_new(GDK_BOTTOM_SIDE);
	    break;

	  case eCursor_arrow_north:
	  case eCursor_arrow_north_plus:
	    newCursor = gdk_cursor_new(GDK_TOP_SIDE);
	    break;

	  case eCursor_arrow_east:
	  case eCursor_arrow_east_plus:
	    newCursor = gdk_cursor_new(GDK_RIGHT_SIDE);
	    break;

	  case eCursor_arrow_west:
	  case eCursor_arrow_west_plus:
	    newCursor = gdk_cursor_new(GDK_LEFT_SIDE);
	    break;

	  default:
	    NS_ASSERTION(PR_FALSE, "Invalid cursor type");
	    break;
	}
    
    if (nsnull != newCursor) {
	    mCursor = aCursor;
	    gdk_window_set_cursor(mWidget->window, newCursor);
    }
  }
  return NS_OK;
}

NS_METHOD nsWidget::Invalidate(PRBool aIsSynchronous)
{
    NS_NOTYETIMPLEMENTED("nsWidget::Invalidate");
    return NS_OK;
}

NS_METHOD nsWidget::Invalidate(const nsRect & aRect, PRBool aIsSynchronous)
{
    NS_NOTYETIMPLEMENTED("nsWidget::Invalidate");
    return NS_OK;
}

NS_METHOD nsWidget::Update(void)
{
    NS_NOTYETIMPLEMENTED("nsWidget::Update");
    return NS_OK;
}

//-------------------------------------------------------------------------
//
// Return some native data according to aDataType
//
//-------------------------------------------------------------------------
void *nsWidget::GetNativeData(PRUint32 aDataType)
{
    switch(aDataType) {
      case NS_NATIVE_WINDOW:
        return (void *)mWidget->window;
      case NS_NATIVE_DISPLAY:
	return (void *)GDK_DISPLAY();
      case NS_NATIVE_WIDGET:
	return (void *)mWidget;
      case NS_NATIVE_GRAPHIC:
        {
	      void *res;
	      if (mGC) {
		      res = mGC;
	      } else {
          NS_ASSERTION(mToolkit, "unable to return NS_NATIVE_GRAPHIC");
          res = (void *)((nsToolkit *)mToolkit)->GetSharedGC();
	      }
	      NS_ASSERTION(res, "unable to return NS_NATIVE_GRAPHIC");
	      return res;
	  }
      default:
	break;
    }
    return nsnull;
}

//-------------------------------------------------------------------------
//
// Set the colormap of the window
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::SetColorMap(nsColorMap *aColorMap)
{
    return NS_OK;
}

nsIDeviceContext* nsWidget::GetDeviceContext(void)
{
    NS_NOTYETIMPLEMENTED("nsWidget::GetDeviceContext");
    return mContext;
}

nsIAppShell* nsWidget::GetAppShell(void)
{
    NS_NOTYETIMPLEMENTED("nsWidget::GetAppShell");
    return nsnull;
}

NS_METHOD nsWidget::Scroll(PRInt32 aDx, PRInt32 aDy, nsRect *aClipRect)
{
    NS_NOTYETIMPLEMENTED("nsWidget::Scroll");
    return NS_OK;
}

NS_METHOD nsWidget::BeginResizingChildren(void)
{
  return NS_OK;
}

NS_METHOD nsWidget::EndResizingChildren(void)
{
  return NS_OK;
}

NS_METHOD nsWidget::GetPreferredSize(PRInt32& aWidth, PRInt32& aHeight)
{
  aWidth  = mPreferredWidth;
  aHeight = mPreferredHeight;
  return (mPreferredWidth != 0 && mPreferredHeight != 0)?NS_OK:NS_ERROR_FAILURE;
}

NS_METHOD nsWidget::SetPreferredSize(PRInt32 aWidth, PRInt32 aHeight)
{
    mPreferredWidth  = aWidth;
    mPreferredHeight = aHeight;
    return NS_OK;
}

NS_METHOD nsWidget::SetMenuBar(nsIMenuBar * aMenuBar)
{
    NS_NOTYETIMPLEMENTED("nsWidget::SetMenuBar");
    return NS_OK;
}

nsresult nsWidget::StandardWindowCreate(nsIWidget *aParent,
		      const nsRect &aRect,
		      EVENT_CALLBACK aHandleEventFunction,
		      nsIDeviceContext *aContext,
		      nsIAppShell *aAppShell,
		      nsIToolkit *aToolkit,
		      nsWidgetInitData *aInitData,
		      nsNativeWidget aNativeParent)
{
  GtkWidget *parentWidget = nsnull;
  mBounds = aRect;

  BaseCreate(aParent, aRect, aHandleEventFunction, aContext, 
             aAppShell, aToolkit, aInitData);

  if (aParent) {
    parentWidget = GTK_WIDGET(aParent->GetNativeData(NS_NATIVE_WIDGET));

  } else if (aNativeParent) {
    parentWidget = GTK_WIDGET(aNativeParent);

  } else if(aAppShell) {
    nsNativeWidget shellWidget = aAppShell->GetNativeData(NS_NATIVE_SHELL);
    if (shellWidget)
      parentWidget = GTK_WIDGET(shellWidget);
  }

  g_print("--\n");

  CreateNative (parentWidget);

  Resize(mBounds.width, mBounds.height, TRUE);

  if (parentWidget)
  {
    gtk_layout_put(GTK_LAYOUT(parentWidget), mWidget, mBounds.x, mBounds.y);
    g_print("nsWidget::SWC(%3d,%3d)    - %s %p\n", mBounds.x, mBounds.y, mWidget->name, this);
  }

  InitCallbacks();
  CreateGC();
  
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// create with nsIWidget parent
//
//-------------------------------------------------------------------------

NS_METHOD nsWidget::Create(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
    return(StandardWindowCreate(aParent, aRect, aHandleEventFunction,
                           aContext, aAppShell, aToolkit, aInitData,
			   nsnull));
}

//-------------------------------------------------------------------------
//
// create with a native parent
//
//-------------------------------------------------------------------------
NS_METHOD nsWidget::Create(nsNativeWidget aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
    return(StandardWindowCreate(nsnull, aRect, aHandleEventFunction,
                           aContext, aAppShell, aToolkit, aInitData,
			   aParent));
}

//-------------------------------------------------------------------------
//
// Initialize all the Callbacks
//
//-------------------------------------------------------------------------
void nsWidget::InitCallbacks(char *aName)
{
    NS_NOTYETIMPLEMENTED("nsWidget::InitCallbacks");
}

nsIRenderingContext* nsWidget::GetRenderingContext()
{
    nsIRenderingContext * ctx = nsnull;

    if (GetNativeData(NS_NATIVE_WIDGET)) {

	nsresult  res;

	static NS_DEFINE_IID(kRenderingContextCID, NS_RENDERING_CONTEXT_CID);
	static NS_DEFINE_IID(kRenderingContextIID, NS_IRENDERING_CONTEXT_IID);

	res = nsRepository::CreateInstance(kRenderingContextCID, nsnull,
					   kRenderingContextIID,
					   (void **)&ctx);

	if (NS_OK == res)
	    ctx->Init(mContext, this);

	NS_ASSERTION(NULL != ctx, "Null rendering context");
    }

    return ctx;
}

void nsWidget::CreateGC()
{
  if (nsnull == mGC) {
    if (!mWidget) {
      mWidget = ::gtk_window_new(GTK_WINDOW_POPUP);
      gtk_widget_realize(mWidget);
      mGC = ::gdk_gc_new(GTK_WIDGET(mWidget)->window);
    }
    else if (GTK_IS_LAYOUT(mWidget)) {
      if (!GTK_LAYOUT(mWidget)->bin_window) {
        gtk_widget_realize(mWidget);
        mGC = ::gdk_gc_new(GTK_WIDGET(mWidget)->window);
      }
      else
        mGC = ::gdk_gc_new(GTK_WIDGET(mWidget)->window);
    }
    else if (!GTK_WIDGET(mWidget)->window) {
      gtk_widget_realize(mWidget);
      mGC = ::gdk_gc_new(GTK_WIDGET(mWidget)->window);
    }
    else
      mGC = ::gdk_gc_new(GTK_WIDGET(mWidget)->window);
  }
}

void nsWidget::ConvertToDeviceCoordinates(nscoord &aX, nscoord &aY)
{

}

PRBool nsWidget::ConvertStatus(nsEventStatus aStatus)
{
  switch(aStatus) {
    case nsEventStatus_eIgnore:
      return(PR_FALSE);
    case nsEventStatus_eConsumeNoDefault:
      return(PR_TRUE);
    case nsEventStatus_eConsumeDoDefault:
      return(PR_FALSE);
    default:
      NS_ASSERTION(0, "Illegal nsEventStatus enumeration value");
      break;
  }
  return(PR_FALSE);
}

PRBool nsWidget::DispatchWindowEvent(nsGUIEvent* event)
{
  nsEventStatus status;
  DispatchEvent(event, status);
  return ConvertStatus(status);
}

//-------------------------------------------------------------------------
//
// Invokes callback and  ProcessEvent method on Event Listener object
//
//-------------------------------------------------------------------------

NS_IMETHODIMP nsWidget::DispatchEvent(nsGUIEvent *event,
				      nsEventStatus &aStatus)
{
  NS_ADDREF(event->widget);

  aStatus = nsEventStatus_eIgnore;
  if (nsnull != mEventCallback) {
    aStatus = (*mEventCallback)(event);
  }

  // Dispatch to event listener if event was not consumed
  if ((aStatus != nsEventStatus_eIgnore) && (nsnull != mEventListener)) {
    aStatus = mEventListener->ProcessEvent(*event);
  }
  NS_RELEASE(event->widget);
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Deal with all sort of mouse event
//
//-------------------------------------------------------------------------
PRBool nsWidget::DispatchMouseEvent(nsMouseEvent& aEvent)
{
  PRBool result = PR_FALSE;
  if (nsnull == mEventCallback && nsnull == mMouseListener) {
    return result;
  }


  // call the event callback
  if (nsnull != mEventCallback) {
    result = DispatchWindowEvent(&aEvent);

    return result;
  }

  if (nsnull != mMouseListener) {
    switch (aEvent.message) {
      case NS_MOUSE_MOVE: {
        /*result = ConvertStatus(mMouseListener->MouseMoved(event));
        nsRect rect;
        GetBounds(rect);
        if (rect.Contains(event.point.x, event.point.y)) {
          if (mCurrentWindow == NULL || mCurrentWindow != this) {
            //printf("Mouse enter");
            mCurrentWindow = this;
          }
        } else {
          //printf("Mouse exit");
        }*/

      } break;

      case NS_MOUSE_LEFT_BUTTON_DOWN:
      case NS_MOUSE_MIDDLE_BUTTON_DOWN:
      case NS_MOUSE_RIGHT_BUTTON_DOWN:
        result = ConvertStatus(mMouseListener->MousePressed(aEvent));
        break;

      case NS_MOUSE_LEFT_BUTTON_UP:
      case NS_MOUSE_MIDDLE_BUTTON_UP:
      case NS_MOUSE_RIGHT_BUTTON_UP:
        result = ConvertStatus(mMouseListener->MouseReleased(aEvent));
        result = ConvertStatus(mMouseListener->MouseClicked(aEvent));
        break;
    } // switch
  }
  return result;
}
