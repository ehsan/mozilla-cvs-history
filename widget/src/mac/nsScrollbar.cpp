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

#include "nsScrollbar.h"
#include "nsToolkit.h"
#include "nsGUIEvent.h"
#include "nsUnitConversion.h"
#include "nsIDeviceContext.h"

NS_IMPL_ADDREF(nsScrollbar)
NS_IMPL_RELEASE(nsScrollbar)

/**-------------------------------------------------------------------------------
 * nsScrollbar Constructor
 *  @update  dc 10/31/98
 * @param aIsVertical -- Tells if the scrollbar had a vertical or horizontal orientation
 */
nsScrollbar::nsScrollbar(PRBool aIsVertical)
{
    strcpy(gInstanceClassName, "nsScrollbar");
    mIsVertical  = aIsVertical;
    mLineIncrement = 0;

}

/**-------------------------------------------------------------------------------
 * The create method for a scrollbar, using a nsIWidget as the parent
 * @update  dc 08/31/98
 * @param  aParent -- the widget which will be this widgets parent in the tree
 * @param  aRect -- The bounds in parental coordinates of this widget
 * @param  aHandleEventFunction -- Procedures to be executed for this widget
 * @param  aContext -- device context to be used by this widget
 * @param  aAppShell -- 
 * @param  aToolkit -- toolkit to be used by this widget
 * @param  aInitData -- Initialization data used by frames
 * @return -- NS_OK if everything was created correctly
 */ 
NS_IMETHODIMP nsScrollbar::Create(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
  mParent = aParent;
  aParent->AddChild(this);
	
	WindowPtr window = nsnull;

  if (aParent) {
    window = (WindowPtr) aParent->GetNativeData(NS_NATIVE_WIDGET);
  } else if (aAppShell) {
   		window = (WindowPtr) aAppShell->GetNativeData(NS_NATIVE_SHELL);
  }

  mIsMainWindow = PR_FALSE;
  mWindowMadeHere = PR_TRUE;
	mWindowRecord = (WindowRecord*)window;
	mWindowPtr = (WindowPtr)window;
  
  NS_ASSERTION(window!=nsnull,"The WindowPtr for the widget cannot be null")
	if (window){
	  InitToolkit(aToolkit, aParent);
	  // InitDeviceContext(aContext, parentWidget);
		
		// Set the bounds to the local rect
		SetBounds(aRect);
		
		// Convert to macintosh coordinates		
		Rect r;
		nsRectToMacRect(aRect,r);
				
		mWindowRegion = NewRgn();
		SetRectRgn(mWindowRegion,aRect.x,aRect.y,aRect.x+aRect.width,aRect.y+aRect.height);		 

	  // save the event callback function
	  mEventCallback = aHandleEventFunction;
	  
	  mMouseDownInScroll = PR_FALSE;
	  mWidgetArmed = PR_FALSE;

	  //InitCallbacks("nsButton");
	  InitDeviceContext(mContext, (nsNativeWidget)mWindowPtr);
	}
	return NS_OK;
}

/**-------------------------------------------------------------------------------
 * The create method for a button, using a nsNativeWidget as the parent
 * @update  dc 08/31/98
 * @param  aParent -- the widget which will be this widgets parent in the tree
 * @param  aRect -- The bounds in parental coordinates of this widget
 * @param  aHandleEventFunction -- Procedures to be executed for this widget
 * @param  aContext -- device context to be used by this widget
 * @param  aAppShell -- 
 * @param  aToolkit -- toolkit to be used by this widget
 * @param  aInitData -- Initialization data used by frames
 * @return -- NS_OK if everything was created correctly
 */ 
NS_IMETHODIMP nsScrollbar::Create(nsNativeWidget aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
nsWindow		*theNsWindow=nsnull;
nsRefData		*theRefData;

	if(0!=aParent){
		theRefData = (nsRefData*)(((WindowPeek)aParent)->refCon);
		theNsWindow = (nsWindow*)theRefData->GetCurWidget();
	}
		
	if(nsnull!=theNsWindow){
		Create(theNsWindow, aRect,aHandleEventFunction, aContext, aAppShell, aToolkit, aInitData);
	}

	//NS_ERROR("This Widget must not use this Create method");
	return NS_OK;

}

/**-------------------------------------------------------------------------------
 * Destuctor for the nsScrollbar
 * @update  dc 10/31/98
 */ 
nsScrollbar::~nsScrollbar()
{
}

/**-------------------------------------------------------------------------------
 * Implement the standard QueryInterface for NS_IWIDGET_IID and NS_ISUPPORTS_IID
 * @update  dc 08/31/98
 * @param aIID The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. NS_ISUPPORTS_IID)
 */ 
nsresult nsScrollbar::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  if (NULL == aInstancePtr) {
      return NS_ERROR_NULL_POINTER;
  }

  static NS_DEFINE_IID(kIScrollbarIID, NS_ISCROLLBAR_IID);
  if (aIID.Equals(kIScrollbarIID)) {
      *aInstancePtr = (void*) ((nsIScrollbar*)this);
      AddRef();
      return NS_OK;
  }

  return nsWindow::QueryInterface(aIID,aInstancePtr);
}

/**-------------------------------------------------------------------------------
 * DispatchMouseEvent handle an event for this scrollbar
 * @update  dc 08/31/98
 * @Param aEvent -- The mouse event to respond to for this button
 * @return -- True if the event was handled, PR_FALSE if we did not handle it.
 */ 
PRBool 
nsScrollbar::DispatchMouseEvent(nsMouseEvent &aEvent)
{
PRBool 	result;

	switch (aEvent.message)
		{
		case NS_MOUSE_LEFT_BUTTON_DOWN:
			mMouseDownInScroll = PR_TRUE;
			DrawWidget();
			result = nsWindow::DispatchMouseEvent(aEvent);
			break;
		case NS_MOUSE_LEFT_BUTTON_UP:
			mMouseDownInScroll = PR_FALSE;
			DrawWidget();
			if(mWidgetArmed==PR_TRUE)
				result = nsWindow::DispatchMouseEvent(aEvent);
			break;
		case NS_MOUSE_EXIT:
			if( mMouseDownInScroll )
				{
				DrawWidget();
				mWidgetArmed = PR_FALSE;
				}
			result = nsWindow::DispatchMouseEvent(aEvent);
			break;
		case NS_MOUSE_ENTER:
			if( mMouseDownInScroll )
				{
				DrawWidget();
				//mWidgetArmed = PR_TRUE;
				mWidgetArmed = PR_FALSE;
				}
			result = nsWindow::DispatchMouseEvent(aEvent);
			break;
		case NS_MOUSE_MOVE:
			if(mWidgetArmed)
				{
				
				//this->SetPosition();
				this->DrawWidget();
				}
			break;
		}
		
	return result;
}

/**-------------------------------------------------------------------------------
 *  Draw in the scrollbar and thumb
 *  @update  dc 10/16/98
 *  @param   aMouseInside -- A boolean indicating if the mouse is inside the control
 *  @return  nothing is returned
 */
void
nsScrollbar::DrawWidget()
{
PRInt32							offx,offy;
nsRect							theRect,tr;
Rect								macRect;
GrafPtr							theport;
RGBColor						blackcolor = {0,0,0};
RGBColor						redcolor = {255<<8,0,0};
RgnHandle						thergn;


	CalcOffset(offx,offy);
	GetPort(&theport);
	::SetPort(mWindowPtr);
	::SetOrigin(-offx,-offy);
	GetBounds(theRect);
	nsRectToMacRect(theRect,macRect);
	thergn = ::NewRgn();
	::GetClip(thergn);
	::ClipRect(&macRect);
	::PenNormal();
	::RGBForeColor(&blackcolor);
	// Frame the general scrollbar
	::FrameRect(&macRect);

	::RGBForeColor(&blackcolor);
	::PenSize(1,1);
	::SetClip(thergn);
	::SetOrigin(0,0);
	::SetPort(theport);

	DrawThumb(PR_FALSE);
			
}

/**-------------------------------------------------------------------------------
 *  Draw or clear the thumb area of the scrollbar
 *  @update  dc 10/31/98
 *  @param   aClear -- A boolean indicating if it will be erased instead of painted
 *  @return  nothing is returned
 */
void
nsScrollbar::DrawThumb(PRBool	aClear)
{
PRInt32							offx,offy;
nsRect							frameRect,thumbRect;
Rect								macFrameRect,macThumbRect;
GrafPtr							theport;
RGBColor						blackcolor = {0,0,0};
RGBColor						redcolor = {255<<8,0,0};
RgnHandle						thergn;


	CalcOffset(offx,offy);
	GetPort(&theport);
	::SetPort(mWindowPtr);
	::SetOrigin(-offx,-offy);
	GetBounds(frameRect);
	nsRectToMacRect(frameRect,macFrameRect);
	::PenNormal();
	::RGBForeColor(&blackcolor);
	
	// draw or clear the thumb
	if(mIsVertical)
		{	
		thumbRect.width = frameRect.width;
		thumbRect.height = (frameRect.height*mThumbSize)/mMaxRange;
		thumbRect.x = frameRect.x;
		thumbRect.y = frameRect.y+(frameRect.height*mPosition)/mMaxRange;
		}
	else
		{
		thumbRect.width = (frameRect.width*mThumbSize)/mMaxRange;
		thumbRect.height = frameRect.height;
		thumbRect.y = frameRect.y+(frameRect.width*mPosition)/mMaxRange;
		thumbRect.y = frameRect.y;
		}

	// clip only at the thumb
	thergn = ::NewRgn();
	::GetClip(thergn);
	nsRectToMacRect(thumbRect,macThumbRect);
	::ClipRect(&macThumbRect);
	
	if(aClear == PR_TRUE)
		::EraseRect(&macThumbRect);
	else
		{
		::RGBForeColor(&blackcolor);
		::PaintRect(&macThumbRect);
		}
	
	// Frame the general scrollbar
	::FrameRect(&macFrameRect);

	::RGBForeColor(&blackcolor);
	::PenSize(1,1);
	::SetClip(thergn);
	::SetOrigin(0,0);
	::SetPort(theport);
}



/**-------------------------------------------------------------------------------
 *	set the maximum range of a scroll bar
 *  @update  dc 09/16/98
 *  @param  aMaxRange -- the maximum to set this to
 *  @return -- If a good size was returned
 */
NS_METHOD nsScrollbar::SetMaxRange(PRUint32 aEndRange)
{
    mMaxRange = aEndRange;
		return (NS_OK);
}

/**-------------------------------------------------------------------------------
 *	get the maximum range of a scroll bar
 *  @update  dc 09/16/98
 *  @param  aMaxRange -- The current maximum this slider can be
 *  @return -- If a good size was returned
 */
NS_METHOD nsScrollbar::GetMaxRange(PRUint32& aMaxRange)
{
	aMaxRange = mMaxRange;
	return (NS_OK);
}

/**-------------------------------------------------------------------------------
 *	Set the current position of the slider
 *  @update  dc 09/16/98
 *  @param  aMaxRange -- The current value to set the slider position to.
 *  @return -- NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::SetPosition(PRUint32 aPos)
{
	if(aPos>=0)
		{
		// erase the old position
		
		
		DrawThumb(PR_TRUE);
		mPosition = aPos;
		DrawThumb(PR_FALSE);
		//this->DrawWidget();
  	return (NS_OK);
  	}
  else
  	return(NS_ERROR_FAILURE);
}


/**-------------------------------------------------------------------------------
 *	Get the current position of the slider
 *  @update  dc 09/16/98
 *  @param  aMaxRange -- The current slider position.
 *  @return -- NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::GetPosition(PRUint32& aPos)
{
  aPos = mPosition;
  return (NS_OK);
}

/**-------------------------------------------------------------------------------
 *	Set the hieght of a vertical, or width of a horizontal scroll bar thumb control
 *  @update  dc 09/16/98
 *  @param  aSize -- the size to set the thumb control to
 *  @return -- NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::SetThumbSize(PRUint32 aSize)
{

    if (aSize <= 0) 
  		aSize = 1;
    
  	mThumbSize = aSize;
  	this->DrawWidget();				// ??? is this necessary - DWC
  	return(NS_OK);
}

/**-------------------------------------------------------------------------------
 *	get the height of a vertical, or width of a horizontal scroll bar thumb control
 *  @update  dc 09/16/98
 *  @param  aSize -- the size to set the thumb control to
 *  @return -- NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::GetThumbSize(PRUint32& aSize)
{

	aSize = mThumbSize;
	return(NS_OK);
}

/**-------------------------------------------------------------------------------
 *	Set the increment of the scroll bar
 *  @update  dc 09/16/98
 *  @param  aLineIncrement -- the control increment
 *  @return -- NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::SetLineIncrement(PRUint32 aLineIncrement)
{
  if (aLineIncrement > 0) 
  	{
    mLineIncrement = aLineIncrement;
  	}
	return(NS_OK);
}


/**-------------------------------------------------------------------------------
 *	Get the increment of the scroll bar
 *  @update  dc 09/16/98
 *  @param aLineIncrement -- the control increment
 *  @return NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::GetLineIncrement(PRUint32& aLineIncrement)
{
		aLineIncrement = mLineIncrement;
    return(NS_OK);
}


/**-------------------------------------------------------------------------------
 *	Set all the scrollbar parameters
 *  @update  dc 09/16/98
 *  @param aMaxRange -- max range of the scrollbar in relative units
 *  @param aThumbSize -- thumb size, in relative units
 *  @param aPosition -- the thumb position in relative units
 *  @param aLineIncrement -- the increment levelof the scrollbar
 *  @return NS_OK if the position is valid
 */
NS_METHOD nsScrollbar::SetParameters(PRUint32 aMaxRange, PRUint32 aThumbSize,
                                PRUint32 aPosition, PRUint32 aLineIncrement)
{

	this->SetThumbSize(aThumbSize);
	
	mMaxRange  = (((int)aMaxRange) > 0?aMaxRange:10);
	mLineIncrement = (((int)aLineIncrement) > 0?aLineIncrement:1);

	mPosition    = ((int)aPosition) > mMaxRange ? mMaxRange-1 : ((int)aPosition);

	 return(NS_OK);
}


/**-------------------------------------------------------------------------------
 * The onPaint handleer for a button -- this may change, inherited from windows
 * @param aEvent -- The paint event to respond to
 * @return -- PR_TRUE if painted, false otherwise
 */ 
//-------------------------------------------------------------------------
PRBool nsScrollbar::OnPaint(nsPaintEvent & aEvent)
{
	DrawWidget();
  return PR_FALSE;
}


/**-------------------------------------------------------------------------------
 * Resizes the button, currently handles by nsWindow
 * @update  dc 08/31/98
 * @Param aEvent -- The event for this resize
 * @return -- True if the event was handled, PR_FALSE is always return for now
 */ 
PRBool nsScrollbar::OnResize(nsSizeEvent &aEvent)
{
  return nsWindow::OnResize(aEvent);
}

/**-------------------------------------------------------------------------------
 * Resizes the Resizes the bounds
 * @update  dc 10/23/98
 * @Param aWidth -- The new width
 * @Param aHeight -- The new width
 * @Param aRepaint -- a boolean which tells if we need to repaint
 * @return -- True if the event was handled, NS_OK is always return for now
 */ 
NS_IMETHODIMP nsScrollbar::Resize(PRUint32 aWidth, PRUint32 aHeight, PRBool aRepaint)
{

  mBounds.width  = aWidth;
  mBounds.height = aHeight;
  
   if(nsnull!=mWindowRegion)
  	::DisposeRgn(mWindowRegion);
	mWindowRegion = NewRgn();
	SetRectRgn(mWindowRegion,mBounds.x,mBounds.y,mBounds.x+mBounds.width,mBounds.y+mBounds.height);		 
 
  if (aRepaint){
  	UpdateVisibilityFlag();
  	UpdateDisplay();
  	}
  return(NS_OK);
}

    
/**-------------------------------------------------------------------------------
 * Resizes the Resizes the bounds
 * @update  dc 10/23/98
 * @Param aX -- the left position of the bounds
 * @Param aY -- the top position of the bounds
 * @Param aWidth -- The new width
 * @Param aHeight -- The new width
 * @Param aRepaint -- a boolean which tells if we need to repaint
 * @return -- True if the event was handled, NS_OK is always return for now
 */ 
NS_IMETHODIMP nsScrollbar::Resize(PRUint32 aX, PRUint32 aY, PRUint32 aWidth, PRUint32 aHeight, PRBool aRepaint)
{
nsSizeEvent 	event;

  mBounds.x      = aX;
  mBounds.y      = aY;
  mBounds.width  = aWidth;
  mBounds.height = aHeight;
  if(nsnull!=mWindowRegion)
  	::DisposeRgn(mWindowRegion);
	mWindowRegion = NewRgn();
	SetRectRgn(mWindowRegion,mBounds.x,mBounds.y,mBounds.x+mBounds.width,mBounds.y+mBounds.height);

  if (aRepaint){
  	UpdateVisibilityFlag();
  	UpdateDisplay();
  }
 	return(NS_OK);
}

/**-------------------------------------------------------------------------------
 * Set the scrollbar position
 * @update  dc 10/31/98
 * @Param aPosition -- position in relative units
 * @return -- return the position
 */ 
int nsScrollbar::AdjustScrollBarPosition(int aPosition) 
{
int maxRange=0,cap,sliderSize=0;

	cap = maxRange - sliderSize;
  return aPosition > cap ? cap : aPosition;
}

/**-------------------------------------------------------------------------------
 * Deal with scrollbar messages (actually implemented only in nsScrollbar)
 * @update  dc 08/31/98
 * @Param aEvent -- the event to handle
 * @Param cPos -- the current position
 * @return -- True if the event was handled, PR_FALSE if we did not handle it.
 */ 
PRBool nsScrollbar::OnScroll(nsScrollbarEvent & aEvent, PRUint32 cPos)
{
PRBool 				result = PR_TRUE;
PRUint32 			newPosition=0;
PRUint32			range;

	switch (aEvent.message) {
	  // scroll one line right or down
	  case NS_SCROLLBAR_LINE_NEXT: {
	    newPosition += mLineIncrement;
	    PRUint32 max = GetMaxRange(range) - GetThumbSize(range);
	    if (newPosition > (int)max) 
	        newPosition = (int)max;

	    // if an event callback is registered, give it the chance
	    // to change the increment
	    if (mEventCallback) {
	      aEvent.position = newPosition;
	      result = ConvertStatus((*mEventCallback)(&aEvent));
	      newPosition = aEvent.position;
	    }
	    break;
	  }

    // scroll one line left or up
    case NS_SCROLLBAR_LINE_PREV: {
      newPosition -= mLineIncrement;
      if (newPosition < 0) 
          newPosition = 0;

      // if an event callback is registered, give it the chance
      // to change the decrement
      if (mEventCallback) {
        aEvent.position = newPosition;
        result = ConvertStatus((*mEventCallback)(&aEvent));
        newPosition = aEvent.position;
      }
      break;
    }

    // Scrolls one page right or down
    case NS_SCROLLBAR_PAGE_NEXT: {
      PRUint32 max = GetMaxRange(range) - GetThumbSize(range);
      if (newPosition > (int)max) 
          newPosition = (int)max;

      // if an event callback is registered, give it the chance
      // to change the increment
      if (mEventCallback) {
        aEvent.position = newPosition;
        result = ConvertStatus((*mEventCallback)(&aEvent));
        newPosition = aEvent.position;
      }
      break;
    }

      // Scrolls one page left or up.
    case NS_SCROLLBAR_PAGE_PREV: {
      //XtVaGetValues(mWidget, XmNvalue, &newPosition, nsnull);
      if (newPosition < 0) 
          newPosition = 0;

      // if an event callback is registered, give it the chance
      // to change the increment
      if (mEventCallback) {
        aEvent.position = newPosition;
        result = ConvertStatus((*mEventCallback)(&aEvent));
        newPosition = aEvent.position;
      }

    break;
    }

      // Scrolls to the absolute position. The current position is specified by 
      // the cPos parameter.
    case NS_SCROLLBAR_POS: {
        newPosition = cPos;

        // if an event callback is registered, give it the chance
        // to change the increment
        if (mEventCallback) {
          aEvent.position = newPosition;
          result = ConvertStatus((*mEventCallback)(&aEvent));
          newPosition = aEvent.position;
        }
     break;
     }
  }
  return result;
}


