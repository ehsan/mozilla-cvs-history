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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Original Author: Eric J. Burley (ericb@neoplanet.com)
 *
 * Contributor(s): 
 * based heavily on David Hyatt's work on nsButtonBoxFrame
 */

#include "nsCOMPtr.h"
#include "nsResizerFrame.h"
#include "nsIContent.h"
#include "nsIDocument.h"
#include "nsIDOMXULDocument.h"
#include "nsIDOMNodeList.h"
#include "nsHTMLAtoms.h"
#include "nsINameSpaceManager.h"

#include "nsIWidget.h"
#include "nsIPresContext.h"
#include "nsPIDOMWindow.h"	
#include "nsIScriptGlobalObject.h"
#include "nsIViewManager.h"
#include "nsXULAtoms.h"

//
// NS_NewXULButtonFrame
//
// Creates a new Button frame and returns it in |aNewFrame|
//
nsresult
NS_NewResizerFrame( nsIPresShell* aPresShell, nsIFrame** aNewFrame )
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsTitleBarFrame* it = new (aPresShell) nsResizerFrame(aPresShell);
  if (nsnull == it)
    return NS_ERROR_OUT_OF_MEMORY;

  // it->SetFlags(aFlags);
  *aNewFrame = it;
  return NS_OK;
  
} // NS_NewTitleBarFrame

nsResizerFrame::nsResizerFrame(nsIPresShell* aPresShell)
:nsTitleBarFrame(aPresShell) 
{
	mDirection = topleft; // by default...
}

NS_IMETHODIMP  nsResizerFrame::Init(nsIPresContext*  aPresContext,
                nsIContent*      aContent,
                nsIFrame*        aParent,
                nsIStyleContext* aContext,
                nsIFrame*        asPrevInFlow)
{
	nsresult rv = nsTitleBarFrame::Init(aPresContext, aContent, aParent, aContext, asPrevInFlow);

	GetInitialDirection(mDirection);

	return rv;
}


NS_IMETHODIMP
nsResizerFrame::HandleEvent(nsIPresContext* aPresContext, 
                                      nsGUIEvent* aEvent,
                                      nsEventStatus* aEventStatus)
{


  PRBool doDefault = PR_TRUE;

  switch (aEvent->message) {

	 case NS_MOUSE_LEFT_BUTTON_DOWN:	{
			 
			 // we're tracking.
			 mTrackingMouseMove = PR_TRUE;
			 
			 // start capture.		 
			 aEvent->widget->CaptureMouse(PR_TRUE);
			 CaptureMouseEvents(aPresContext,PR_TRUE);


			 
			 // remember current mouse coordinates.
			 mLastPoint = aEvent->refPoint;
			 aEvent->widget->GetScreenBounds(mWidgetRect);

			 nsRect bounds;
			 aEvent->widget->GetBounds(bounds);

			 *aEventStatus = nsEventStatus_eConsumeNoDefault;
			 doDefault = PR_FALSE;
		 }
		 break;
		 

	 case NS_MOUSE_LEFT_BUTTON_UP: {

			 if(mTrackingMouseMove)
			 {
				 // we're done tracking.
				 mTrackingMouseMove = PR_FALSE;
				 
				 // end capture
				 aEvent->widget->CaptureMouse(PR_FALSE);				 
				 CaptureMouseEvents(aPresContext,PR_FALSE);

				 *aEventStatus = nsEventStatus_eConsumeNoDefault;
				 doDefault = PR_FALSE;
			 }
		 }
		 break;

	 case NS_MOUSE_MOVE: {
			 if(mTrackingMouseMove)
			 {				 				 
			   // get the document and the global script object - should this be cached?
			   nsCOMPtr<nsIPresShell> presShell;
			   aPresContext->GetShell(getter_AddRefs(presShell));
			   nsCOMPtr<nsIDocument> document;
			   presShell->GetDocument(getter_AddRefs(document));
			   nsCOMPtr<nsIScriptGlobalObject> scriptGlobalObject;
			   document->GetScriptGlobalObject(getter_AddRefs(scriptGlobalObject));
			   nsCOMPtr<nsPIDOMWindow> window(do_QueryInterface(scriptGlobalObject));


				 nsPoint nsMoveBy(0,0),nsSizeBy(0,0);
				 nsPoint nsMouseMove(aEvent->refPoint - mLastPoint);
				 				 

				 switch(mDirection)
				 {
						case topleft:
							nsMoveBy = nsMouseMove;
							nsSizeBy -= nsMouseMove;
							break;
						case top:
							nsMoveBy.y = nsMouseMove.y;
							nsSizeBy.y = - nsMouseMove.y;
							break;
						case topright:
							nsMoveBy.y = nsMouseMove.y;
							nsSizeBy.x = nsMouseMove.x;
							mLastPoint.x += nsMouseMove.x;
							nsSizeBy.y = -nsMouseMove.y;
							break;
						case left:
							nsMoveBy.x = nsMouseMove.x;
							nsSizeBy.x = -nsMouseMove.x;
							break;						
						case right:
							nsSizeBy.x = nsMouseMove.x;							
							mLastPoint.x += nsMouseMove.x;
							break;
						case bottomleft:
							nsMoveBy.x = nsMouseMove.x;
							nsSizeBy.y = nsMouseMove.y;
							nsSizeBy.x = -nsMouseMove.x;
							mLastPoint.y += nsMouseMove.y;							
							break;
						case bottom:													
							nsSizeBy.y = nsMouseMove.y;							
							mLastPoint.y += nsMouseMove.y;
							break;
						case bottomright:							
							nsSizeBy = nsMouseMove;		
							mLastPoint += nsMouseMove;							
							break;
				 }



				 PRInt32 x,y,cx,cy;
				 window->GetPositionAndSize(&x,&y,&cx,&cy);

				 x+=nsMoveBy.x;
				 y+=nsMoveBy.y;
				 cx+=nsSizeBy.x;
				 cy+=nsSizeBy.y;

				 window->SetPositionAndSize(x,y,cx,cy,PR_TRUE); // do the repaint.

				 /*
				 if(nsSizeBy.x || nsSizeBy.y)
				 {
					window->ResizeBy(nsSizeBy.x,nsSizeBy.y);
				 }

				 if(nsMoveBy.x || nsMoveBy.y)
				 {
					window->MoveBy(nsMoveBy.x,nsMoveBy.y);
				 }	*/
				 
				 
				 
				 *aEventStatus = nsEventStatus_eConsumeNoDefault;				
				 
				 doDefault = PR_FALSE;
			 }
		 }
		 break;



    case NS_MOUSE_LEFT_CLICK:
      MouseClicked(aPresContext);
      break;
  }
  
  if ( doDefault )  
	  return nsTitleBarFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
  else
	  return NS_OK;
}



/* returns true if aText represented a valid direction 
 */
PRBool 
nsResizerFrame::EvalDirection(nsAutoString& aText,eDirection& aDir)
{
	PRBool aResult = PR_TRUE;
	
	if( aText.EqualsIgnoreCase("topleft") )
	{
		aDir = topleft;
	}
	else if( aText.EqualsIgnoreCase("top") )
	{
		aDir = top;
	}
	else if( aText.EqualsIgnoreCase("topright") )
	{
		aDir = topright;
	}
	else if( aText.EqualsIgnoreCase("left") )
	{
		aDir = left;
	}	
	else if( aText.EqualsIgnoreCase("right") )
	{
		aDir = right;
	}
	else if( aText.EqualsIgnoreCase("bottomleft") )
	{
		aDir = bottomleft;
	}
	else if( aText.EqualsIgnoreCase("bottom") )
	{
		aDir = bottom;
	}
	else if( aText.EqualsIgnoreCase("bottomright") )
	{
		aDir = bottomright;
	}
	else
	{
		aResult = PR_FALSE;
	}
	
	return aResult;
}


/* Returns true if it was set.
 */
PRBool
nsResizerFrame::GetInitialDirection(eDirection& aDirection)
{
 // see what kind of resizer we are.
  nsAutoString value;

  nsCOMPtr<nsIContent> content;
  GetContentOf(getter_AddRefs(content));

  if (!content)
     return PR_FALSE;

  if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsXULAtoms::dir, value))
  {
	   return EvalDirection(value,aDirection); 
  }  

  return PR_FALSE;
}


NS_IMETHODIMP
nsResizerFrame::AttributeChanged(nsIPresContext* aPresContext,
                               nsIContent* aChild,
                               PRInt32 aNameSpaceID,
                               nsIAtom* aAttribute,
                               PRInt32 aHint)
{
    nsresult rv = nsTitleBarFrame::AttributeChanged(aPresContext, aChild,
                                              aNameSpaceID, aAttribute, aHint);

    if (aAttribute == nsXULAtoms::dir ) 
	 {
	 
        GetInitialDirection(mDirection);
    }
  
  return rv;
}



void 
nsResizerFrame::MouseClicked (nsIPresContext* aPresContext) 
{
  // Execute the oncommand event handler.
  nsEventStatus status = nsEventStatus_eIgnore;
  nsMouseEvent event;
  event.eventStructType = NS_EVENT;
  event.message = NS_MENU_ACTION;
  event.isShift = PR_FALSE;
  event.isControl = PR_FALSE;
  event.isAlt = PR_FALSE;
  event.isMeta = PR_FALSE;
  event.clickCount = 0;
  event.widget = nsnull;
  mContent->HandleDOMEvent(aPresContext, &event, nsnull, NS_EVENT_FLAG_INIT, &status);
}
