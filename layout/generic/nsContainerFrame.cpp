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
 */
#include "nsContainerFrame.h"
#include "nsIContent.h"
#include "nsIPresContext.h"
#include "nsIRenderingContext.h"
#include "nsISpaceManager.h"
#include "nsIStyleContext.h"
#include "nsRect.h"
#include "nsPoint.h"
#include "nsGUIEvent.h"
#include "nsStyleConsts.h"
#include "nsIView.h"
#include "nsVoidArray.h"
#include "nsISizeOfHandler.h"
#include "nsIReflowCommand.h"
#include "nsHTMLIIDs.h"
#include "nsHTMLContainerFrame.h"
#include "nsIFrameManager.h"
#include "nsIPresShell.h"
#include "nsCOMPtr.h"
#include "nsLayoutAtoms.h"
#include "nsIViewManager.h"

#ifdef NS_DEBUG
#undef NOISY
#else
#undef NOISY
#endif

nsContainerFrame::nsContainerFrame()
{
}

nsContainerFrame::~nsContainerFrame()
{
}

NS_IMETHODIMP
nsContainerFrame::SetInitialChildList(nsIPresContext& aPresContext,
                                      nsIAtom*        aListName,
                                      nsIFrame*       aChildList)
{
//  NS_PRECONDITION(mFrames.IsEmpty(), "already initialized");

  nsresult  result;
  if (!mFrames.IsEmpty()) {
    // We already have child frames which means we've already been
    // initialized
    result = NS_ERROR_UNEXPECTED;
  } else if (aListName) {
    // All we know about is the unnamed principal child list
    result = NS_ERROR_INVALID_ARG;
  } else {
#ifdef NS_DEBUG
    nsFrame::VerifyDirtyBitSet(aChildList);
#endif
    mFrames.SetFrames(aChildList);
    result = NS_OK;
  }
  return result;
}

NS_IMETHODIMP
nsContainerFrame::Destroy(nsIPresContext& aPresContext)
{
  // Prevent event dispatch during destruction
  nsIView* view;
  GetView(&aPresContext, &view);
  if (nsnull != view) {
    view->SetClientData(nsnull);
  }

  // Delete the primary child list
  mFrames.DestroyFrames(aPresContext);

  // Base class will destroy the frame
  return nsFrame::Destroy(aPresContext);
}

NS_IMETHODIMP
nsContainerFrame::DidReflow(nsIPresContext& aPresContext,
                            nsDidReflowStatus aStatus)
{
  NS_FRAME_TRACE_MSG(NS_FRAME_TRACE_CALLS,
                     ("enter nsContainerFrame::DidReflow: status=%d",
                      aStatus));
  // Let nsFrame position and size our view (if we have one), and clear
  // the NS_FRAME_IN_REFLOW bit
  nsresult  result = nsFrame::DidReflow(aPresContext, aStatus);

  // XXX TROY
#if 0
  if (NS_FRAME_REFLOW_FINISHED == aStatus) {
    // Apply DidReflow to each and every list that this frame implements
    nsIAtom* listName = nsnull;
    PRInt32 listIndex = 0;
    do {
      nsIFrame* kid;
      FirstChild(listName, &kid);
      while (nsnull != kid) {
        kid->DidReflow(aPresContext, aStatus);
        kid->GetNextSibling(&kid);
      }
      NS_IF_RELEASE(listName);
      GetAdditionalChildListName(listIndex++, &listName);
    } while(nsnull != listName);
  }
#endif

  NS_FRAME_TRACE_OUT("nsContainerFrame::DidReflow");
  return result;
}

/////////////////////////////////////////////////////////////////////////////
// Child frame enumeration

NS_IMETHODIMP
nsContainerFrame::FirstChild(nsIAtom* aListName, nsIFrame** aFirstChild) const
{
  NS_PRECONDITION(nsnull != aFirstChild, "null OUT parameter pointer");
  // We only know about the unnamed principal child list
  if (nsnull == aListName) {
    *aFirstChild = mFrames.FirstChild();
    return NS_OK;
  } else {
    *aFirstChild = nsnull;
    return NS_ERROR_INVALID_ARG;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Painting/Events

NS_IMETHODIMP
nsContainerFrame::Paint(nsIPresContext&      aPresContext,
                        nsIRenderingContext& aRenderingContext,
                        const nsRect&        aDirtyRect,
                        nsFramePaintLayer    aWhichLayer)
{
  PaintChildren(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer);
  return NS_OK;
}

// Paint the children of a container, assuming nothing about the
// childrens spatial arrangement. Given relative positioning, negative
// margins, etc, that's probably a good thing.
//
// Note: aDirtyRect is in our coordinate system (and of course, child
// rect's are also in our coordinate system)
void
nsContainerFrame::PaintChildren(nsIPresContext&      aPresContext,
                                nsIRenderingContext& aRenderingContext,
                                const nsRect&        aDirtyRect,
                                nsFramePaintLayer    aWhichLayer)
{
  const nsStyleDisplay* disp = (const nsStyleDisplay*)
    mStyleContext->GetStyleData(eStyleStruct_Display);

  // Child elements have the opportunity to override the visibility property
  // of their parent and display even if the parent is hidden
  PRBool clipState;

  // If overflow is hidden then set the clip rect so that children
  // don't leak out of us
  if (NS_STYLE_OVERFLOW_HIDDEN == disp->mOverflow) {
    aRenderingContext.PushState();
    aRenderingContext.SetClipRect(nsRect(0, 0, mRect.width, mRect.height),
                                  nsClipCombine_kIntersect, clipState);
  }

  nsIFrame* kid = mFrames.FirstChild();
  while (nsnull != kid) {
    PaintChild(aPresContext, aRenderingContext, aDirtyRect, kid, aWhichLayer);
    kid->GetNextSibling(&kid);
  }

  if (NS_STYLE_OVERFLOW_HIDDEN == disp->mOverflow) {
    aRenderingContext.PopState(clipState);
  }
}

// Paint one child frame
void
nsContainerFrame::PaintChild(nsIPresContext&      aPresContext,
                             nsIRenderingContext& aRenderingContext,
                             const nsRect&        aDirtyRect,
                             nsIFrame*            aFrame,
                             nsFramePaintLayer    aWhichLayer)
{
  nsIView *pView;
  aFrame->GetView(&aPresContext, &pView);
  if (nsnull == pView) {
    nsRect kidRect;
    aFrame->GetRect(kidRect);
    nsFrameState state;
    aFrame->GetFrameState(&state);

    // Compute the constrained damage area; set the overlap flag to
    // PR_TRUE if any portion of the child frame intersects the
    // dirty rect.
    nsRect damageArea;
    PRBool overlap;
    if (NS_FRAME_OUTSIDE_CHILDREN & state) {
      // If the child frame has children that leak out of our box
      // then we don't constrain the damageArea to just the childs
      // bounding rect.
      damageArea = aDirtyRect;
      overlap = PR_TRUE;
    }
    else {
      // Compute the intersection of the dirty rect and the childs
      // rect (both are in our coordinate space). This limits the
      // damageArea to just the portion that intersects the childs
      // rect.
      overlap = damageArea.IntersectRect(aDirtyRect, kidRect);
#ifdef NS_DEBUG
      if (!overlap && (0 == kidRect.width) && (0 == kidRect.height)) {
        overlap = PR_TRUE;
      }
#endif
    }

    if (overlap) {
      // Translate damage area into the kids coordinate
      // system. Translate rendering context into the kids
      // coordinate system.
      damageArea.x -= kidRect.x;
      damageArea.y -= kidRect.y;
      aRenderingContext.PushState();
      aRenderingContext.Translate(kidRect.x, kidRect.y);

      // Paint the kid
      aFrame->Paint(aPresContext, aRenderingContext, damageArea, aWhichLayer);
      PRBool clipState;
      aRenderingContext.PopState(clipState);

#ifdef NS_DEBUG
      // Draw a border around the child
      if (nsIFrameDebug::GetShowFrameBorders() && !kidRect.IsEmpty()) {
        aRenderingContext.SetColor(NS_RGB(255,0,0));
        aRenderingContext.DrawRect(kidRect);
      }
#endif
    }
  }
}

NS_IMETHODIMP
nsContainerFrame::GetFrameForPoint(nsIPresContext* aPresContext,
                                   const nsPoint& aPoint, 
                                   nsIFrame**     aFrame)
{
  return GetFrameForPointUsing(aPresContext, aPoint, nsnull, aFrame);
}

nsresult
nsContainerFrame::GetFrameForPointUsing(nsIPresContext* aPresContext,
                                        const nsPoint& aPoint,
                                        nsIAtom*       aList,
                                        nsIFrame**     aFrame)
{
  nsIFrame* kid;
  nsRect kidRect;
  nsPoint tmp;
  *aFrame = this;

  // Attempt to find the first child that contains the desired
  // point. We try to use a quick check on the child frames bbox to
  // avoid a potentially expensive recursion into the child frames
  // GetFrameForPoint method.
  FirstChild(aList, &kid);
  while (nsnull != kid) {
    kid->GetRect(kidRect);
    // Do a quick check and see if the child frame contains the point
    if (kidRect.Contains(aPoint)) {
      // The child frame contains the point. Now see if it really
      // contains the point.
      tmp.MoveTo(aPoint.x - kidRect.x, aPoint.y - kidRect.y);

#ifdef KIPPS_FIX_FOR_BUG_1413

      nsresult rv = kid->GetFrameForPoint(aPresContext, tmp, aFrame);
      if (NS_SUCCEEDED(rv)) {
        // We found the target frame somewhere in the child frame.
        return rv;
      }

      // We failed to find in the child frame the target frame. We
      // need to break out of this loop and look elsewhere so that
      // situations where overlap occurs (e.g. floaters overlapping
      // the background of a block element) find the floater.
      break;

#else

      // XXX: The following code backs out Kipp's fix for 1413 temporarily
      //      so we could prevent bug #18002 and #18006 from happening
      //      and get M11 out the door. I will reenable the code above after
      //      M11 branches, and make sure that #18002 and #18006 are properly
      //      fixed. -- kin@netscape.com

      return kid->GetFrameForPoint(aPresContext, tmp, aFrame);

#endif
    }
    kid->GetNextSibling(&kid);
  }

  // Try again, this time looking only inside child frames that have
  // outside children.
  FirstChild(aList, &kid);
  while (nsnull != kid) {
    nsFrameState state;
    kid->GetFrameState(&state);
    if (NS_FRAME_OUTSIDE_CHILDREN & state) {
      kid->GetRect(kidRect);
      tmp.MoveTo(aPoint.x - kidRect.x, aPoint.y - kidRect.y);
      if (NS_OK == kid->GetFrameForPoint(aPresContext, tmp, aFrame)) {
        return NS_OK;
      }
      else {
        *aFrame = this;
      }
    }
    kid->GetNextSibling(&kid);
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsContainerFrame::ReplaceFrame(nsIPresContext& aPresContext,
                               nsIPresShell&   aPresShell,
                               nsIAtom*        aListName,
                               nsIFrame*       aOldFrame,
                               nsIFrame*       aNewFrame)
{
  nsIFrame* prevFrame;
  nsIFrame* firstChild;
  nsresult  rv;

  // Get the old frame's previous sibling frame
  FirstChild(aListName, &firstChild);
  nsFrameList frames(firstChild);
  NS_ASSERTION(frames.ContainsFrame(aOldFrame), "frame is not a valid child frame");
  prevFrame = frames.GetPrevSiblingFor(aOldFrame);

  // Default implementation treats it like two separate operations
  rv = RemoveFrame(aPresContext, aPresShell, aListName, aOldFrame);
  if (NS_SUCCEEDED(rv)) {
    rv = InsertFrames(aPresContext, aPresShell, aListName, prevFrame, aNewFrame);
  }

  return rv;
}

/////////////////////////////////////////////////////////////////////////////
// Helper member functions

void
nsContainerFrame::PositionFrameView(nsIPresContext* aPresContext,
                                    nsIFrame*       aKidFrame,
                                    nsIView*        aView)
{
  if (aView) {
    // Position view relative to its parent, not relative to aKidFrame's
    // frame which may not have a view
    nsIView*  containingView;
    nsPoint   origin;
    nsIView*  parentView;

    aView->GetParent(parentView);
    aKidFrame->GetOffsetFromView(aPresContext, origin, &containingView);
    
    if (containingView == parentView) {
      nsIViewManager *vm;
      aView->GetViewManager(vm);
      vm->MoveViewTo(aView, origin.x, origin.y);
      NS_RELEASE(vm);
    }
  }
}

void
nsContainerFrame::SyncFrameViewAfterReflow(nsIPresContext* aPresContext,
                                           nsIFrame*       aFrame,
                                           nsIView*        aView,
                                           nsRect*         aCombinedArea,
                                           PRUint32        aFlags)
{
  if (aView) {
    nsIViewManager  *vm;
    nsFrameState    kidState;
    nsSize          frameSize;
    
    aView->GetViewManager(vm);
    aFrame->GetFrameState(&kidState);
    aFrame->GetSize(frameSize);
    
    // Make sure the view is sized and positioned correctly
    if (0 == (aFlags & NS_FRAME_NO_MOVE_VIEW)) {
      nsIView*  containingView;
      nsPoint   origin;
      nsIView*  parentView;

      aView->GetParent(parentView);
      aFrame->GetOffsetFromView(aPresContext, origin, &containingView);
      
      if (containingView == parentView) {
        vm->MoveViewTo(aView, origin.x, origin.y);
      }
    }

    if (0 == (aFlags & NS_FRAME_NO_SIZE_VIEW)) {
      // If the frame has child frames that stick outside the content
      // area, then size the view large enough to include those child
      // frames
      if ((kidState & NS_FRAME_OUTSIDE_CHILDREN) && aCombinedArea) {
        vm->ResizeView(aView, aCombinedArea->XMost(), aCombinedArea->YMost());

      } else {
        vm->ResizeView(aView, frameSize.width, frameSize.height);
      }
    }
  
    const nsStyleColor* color;
    const nsStyleDisplay* display;
    aFrame->GetStyleData(eStyleStruct_Color, (const nsStyleStruct*&)color);
    aFrame->GetStyleData(eStyleStruct_Display, (const nsStyleStruct*&)display);

    // Set the view's opacity
    vm->SetViewOpacity(aView, color->mOpacity);

    // See if the view should be hidden or visible
    PRBool  viewIsVisible = PR_TRUE;
    PRBool  viewHasTransparentContent = (color->mBackgroundFlags &
              NS_STYLE_BG_COLOR_TRANSPARENT) == NS_STYLE_BG_COLOR_TRANSPARENT;

    if (NS_STYLE_VISIBILITY_COLLAPSE == display->mVisible) {
      viewIsVisible = PR_FALSE;
    }
    else if (NS_STYLE_VISIBILITY_HIDDEN == display->mVisible) {
      // If it has a widget, hide the view because the widget can't deal with it
      nsIWidget* widget = nsnull;
      aView->GetWidget(widget);
      if (widget) {
        viewIsVisible = PR_FALSE;
        NS_RELEASE(widget);
      }
      else {
        // If it's a scroll frame, then hide the view. This means that
        // child elements can't override their parent's visibility, but
        // it's not practical to leave it visible in all cases because
        // the scrollbars will be showing
        nsIAtom*  frameType;
        aFrame->GetFrameType(&frameType);

        if (frameType == nsLayoutAtoms::scrollFrame) {
          viewIsVisible = PR_FALSE;

        } else {
          // If we're a container element, then leave the view visible, but
          // mark it as having transparent content. The reason we need to
          // do this is that child elements can override their parent's
          // hidden visibility and be visible anyway
          nsIFrame* firstChild;

          aFrame->FirstChild(nsnull, &firstChild);
          if (firstChild) {
            // Not a left frame, so the view needs to be visible, but marked
            // as having transparent content
            viewHasTransparentContent = PR_TRUE;
          } else {
            // Leaf frame so go ahead and hide the view
            viewIsVisible = PR_FALSE;
          }
        }
        NS_IF_RELEASE(frameType);
      }
    }

    // If the frame has visible content that overflows the content area, then we
    // need the view marked as having transparent content
    if (NS_STYLE_OVERFLOW_VISIBLE == display->mOverflow) {
      if (kidState & NS_FRAME_OUTSIDE_CHILDREN) {
        viewHasTransparentContent = PR_TRUE;
      }
    }

    // Make sure visibility is correct
    vm->SetViewVisibility(aView, viewIsVisible ? nsViewVisibility_kShow :
                          nsViewVisibility_kHide);

    // Make sure content transparency is correct
    if (viewIsVisible) {
      vm->SetViewContentTransparency(aView, viewHasTransparentContent);
    }

    // Clip applies to block-level and replaced elements with overflow
    // set to other than 'visible'
    if (display->IsBlockLevel()) {
      if (display->mOverflow == NS_STYLE_OVERFLOW_HIDDEN) {
        nscoord left, top, right, bottom;

        // Start with the 'auto' values and then factor in user
        // specified values
        left = top = 0;
        right = frameSize.width;
        bottom = frameSize.height;

        if (0 == (NS_STYLE_CLIP_TOP_AUTO & display->mClipFlags)) {
          top += display->mClip.top;
        }
        if (0 == (NS_STYLE_CLIP_RIGHT_AUTO & display->mClipFlags)) {
          right -= display->mClip.right;
        }
        if (0 == (NS_STYLE_CLIP_BOTTOM_AUTO & display->mClipFlags)) {
          bottom -= display->mClip.bottom;
        }
        if (0 == (NS_STYLE_CLIP_LEFT_AUTO & display->mClipFlags)) {
          left += display->mClip.left;
        }
        aView->SetClip(left, top, right, bottom);

      } else {
        // Make sure no clip is set
        aView->SetClip(0, 0, 0, 0);
      }
    }

    NS_RELEASE(vm);
  }
}

/**
 * Invokes the WillReflow() function, positions the frame and its view (if
 * requested), and then calls Reflow(). If the reflow succeeds and the child
 * frame is complete, deletes any next-in-flows using DeleteChildsNextInFlow()
 */
nsresult
nsContainerFrame::ReflowChild(nsIFrame*                aKidFrame,
                              nsIPresContext&          aPresContext,
                              nsHTMLReflowMetrics&     aDesiredSize,
                              const nsHTMLReflowState& aReflowState,
                              nscoord                  aX,
                              nscoord                  aY,
                              PRUint32                 aFlags,
                              nsReflowStatus&          aStatus)
{
  NS_PRECONDITION(aReflowState.frame == aKidFrame, "bad reflow state");

  nsresult  result;

#ifdef DEBUG
  nsSize* saveMaxElementSize = aDesiredSize.maxElementSize;
#ifdef REALLY_NOISY_MAX_ELEMENT_SIZE
  if (nsnull != aDesiredSize.maxElementSize) {
    aDesiredSize.maxElementSize->width = nscoord(0xdeadbeef);
    aDesiredSize.maxElementSize->height = nscoord(0xdeadbeef);
  }
#endif
#endif

  // Send the WillReflow() notification, and position the child frame
  // and its view if requested
  aKidFrame->WillReflow(aPresContext);

  if (0 == (aFlags & NS_FRAME_NO_MOVE_FRAME)) {
    aKidFrame->MoveTo(&aPresContext, aX, aY);
  }

  if (0 == (aFlags & NS_FRAME_NO_MOVE_VIEW)) {
    nsIView*  view;
    aKidFrame->GetView(&aPresContext, &view);
    if (view) {
      PositionFrameView(&aPresContext, aKidFrame, view);
    }
  }

  // Reflow the child frame
  result = aKidFrame->Reflow(aPresContext, aDesiredSize, aReflowState,
                             aStatus);

#ifdef DEBUG
  if (saveMaxElementSize != aDesiredSize.maxElementSize) {
    printf("nsContainerFrame: ");
    nsFrame::ListTag(stdout, aKidFrame);
    printf(" changed the maxElementSize *pointer* (baaaad boy!)\n");
  }
#ifdef REALLY_NOISY_MAX_ELEMENT_SIZE
  if ((nsnull != aDesiredSize.maxElementSize) &&
      ((nscoord(0xdeadbeef) == aDesiredSize.maxElementSize->width) ||
       (nscoord(0xdeadbeef) == aDesiredSize.maxElementSize->height))) {
    printf("nsContainerFrame: ");
    nsFrame::ListTag(stdout, aKidFrame);
    printf(" didn't set max-element-size!\n");
    aDesiredSize.maxElementSize->width = 0;
    aDesiredSize.maxElementSize->height = 0;
  }
#endif
#endif

  // If the reflow was successful and the child frame is complete, delete any
  // next-in-flows
  if (NS_SUCCEEDED(result) && NS_FRAME_IS_COMPLETE(aStatus)) {
    nsIFrame* kidNextInFlow;
    aKidFrame->GetNextInFlow(&kidNextInFlow);
    if (nsnull != kidNextInFlow) {
      // Remove all of the childs next-in-flows. Make sure that we ask
      // the right parent to do the removal (it's possible that the
      // parent is not this because we are executing pullup code)
      nsIFrame* parent;
      aKidFrame->GetParent(&parent);
      ((nsContainerFrame*)parent)->DeleteChildsNextInFlow(aPresContext,
                                                          aKidFrame);
    }
  }
  return result;
}

void
nsContainerFrame::PositionChildViews(nsIPresContext* aPresContext,
                                     nsIFrame*       aFrame)
{
  nsIAtom*  childListName = nsnull;
  PRInt32   childListIndex = 0;

  do {
    // Recursively walk aFrame's child frames
    nsIFrame* childFrame;
    aFrame->FirstChild(childListName, &childFrame);
    while (childFrame) {
      nsIView*  view;

      // See if the child frame has a view
      childFrame->GetView(aPresContext, &view);

      if (view) {
        // Position the view. Because any child views are relative to their
        // parent, there's no need to recurse
        PositionFrameView(aPresContext, childFrame, view);

      } else {
        // Recursively examine its child frames
        PositionChildViews(aPresContext, childFrame);
      }

      // Get the next sibling child frame
      childFrame->GetNextSibling(&childFrame);
    }

    NS_IF_RELEASE(childListName);
    aFrame->GetAdditionalChildListName(childListIndex++, &childListName);
  } while (childListName);
}

/**
 * The second half of frame reflow. Does the following:
 * - sets the frame's bounds
 * - sizes and positions (if requested) the frame's view. If the frame's final
 *   position differs from the current position and the frame itself does not
 *   have a view, then any child frames with views are positioned so they stay
 *   in sync
 * - sets the view's visibility, opacity, content transparency, and clip
 * - invoked the DidReflow() function
 *
 * Flags:
 * NS_FRAME_NO_MOVE_FRAME - don't move the frame. aX and aY are ignored in this
 *    case. Also implies NS_FRAME_NO_MOVE_VIEW
 * NS_FRAME_NO_MOVE_VIEW - don't position the frame's view. Set this if you
 *    don't want to automatically sync the frame and view
 * NS_FRAME_NO_SIZE_VIEW - don't size the frame's view
 * NS_FRAME_NO_MOVE_CHILD_VIEWS - don't move child views. This is for the case
 *    where the frame's new position differs from its current position and the
 *    frame itself doesn't have a view, so moving the frame would cause any child
 *    views to be out of sync
*/
nsresult
nsContainerFrame::FinishReflowChild(nsIFrame*            aKidFrame,
                                    nsIPresContext&      aPresContext,
                                    nsHTMLReflowMetrics& aDesiredSize,
                                    nscoord              aX,
                                    nscoord              aY,
                                    PRUint32             aFlags)
{
  nsPoint curOrigin;
  nsRect  bounds(aX, aY, aDesiredSize.width, aDesiredSize.height);

  aKidFrame->GetOrigin(curOrigin);
  aKidFrame->SetRect(&aPresContext, bounds);

  nsIView*  view;
  aKidFrame->GetView(&aPresContext, &view);
  if (view) {
    // Make sure the frame's view is properly sized and positioned and has
    // things like opacity correct
    SyncFrameViewAfterReflow(&aPresContext, aKidFrame, view,
                             &aDesiredSize.mCombinedArea,
                             aFlags);

  } else if (0 == (aFlags & NS_FRAME_NO_MOVE_CHILD_VIEWS)) {
    // If the frame has moved, then we need to make sure any child views are
    // correctly positioned
    if ((curOrigin.x != aX) || (curOrigin.y != aY)) {
      PositionChildViews(&aPresContext, aKidFrame);
    }
  }
  
  return aKidFrame->DidReflow(aPresContext, NS_FRAME_REFLOW_FINISHED);
}

/**
 * Remove and delete aChild's next-in-flow(s). Updates the sibling and flow
 * pointers
 *
 * @param   aChild child this child's next-in-flow
 * @return  PR_TRUE if successful and PR_FALSE otherwise
 */
void
nsContainerFrame::DeleteChildsNextInFlow(nsIPresContext& aPresContext,
                                         nsIFrame* aChild)
{
  NS_PRECONDITION(mFrames.ContainsFrame(aChild), "bad geometric parent");

  nsIFrame*         nextInFlow;
  nsContainerFrame* parent;
   
  aChild->GetNextInFlow(&nextInFlow);
  NS_PRECONDITION(nsnull != nextInFlow, "null next-in-flow");
  nextInFlow->GetParent((nsIFrame**)&parent);

  // If the next-in-flow has a next-in-flow then delete it, too (and
  // delete it first).
  nsIFrame* nextNextInFlow;

  nextInFlow->GetNextInFlow(&nextNextInFlow);
  if (nsnull != nextNextInFlow) {
    parent->DeleteChildsNextInFlow(aPresContext, nextInFlow);
  }

  // Disconnect the next-in-flow from the flow list
  nsSplittableFrame::BreakFromPrevFlow(nextInFlow);

  // Take the next-in-flow out of the parent's child list
  PRBool  result = parent->mFrames.RemoveFrame(nextInFlow);
  NS_ASSERTION(result, "failed to remove frame");

  // Delete the next-in-flow frame
  nextInFlow->Destroy(aPresContext);

#ifdef NS_DEBUG
  aChild->GetNextInFlow(&nextInFlow);
  NS_POSTCONDITION(nsnull == nextInFlow, "non null next-in-flow");
#endif
}

nsIFrame*
nsContainerFrame::GetOverflowFrames(nsIPresContext* aPresContext,
                                    PRBool          aRemoveProperty)
{
  nsCOMPtr<nsIPresShell>     presShell;
  aPresContext->GetShell(getter_AddRefs(presShell));

  if (presShell) {
    nsCOMPtr<nsIFrameManager>  frameManager;
    presShell->GetFrameManager(getter_AddRefs(frameManager));
  
    if (frameManager) {
      PRUint32  options = 0;
      void*     value;
  
      if (aRemoveProperty) {
        options |= NS_IFRAME_MGR_REMOVE_PROP;
      }
      frameManager->GetFrameProperty(this, nsLayoutAtoms::overflowProperty,
                                     options, &value);
      return (nsIFrame*)value;
    }
  }

  return nsnull;
}

// Destructor function for the overflow frame property
static void
DestroyOverflowFrames(nsIPresContext* aPresContext,
                      nsIFrame*       aFrame,
                      nsIAtom*        aPropertyName,
                      void*           aPropertyValue)
{
  if (aPropertyValue) {
    nsFrameList frames((nsIFrame*)aPropertyValue);

    frames.DestroyFrames(*aPresContext);
  }
}

nsresult
nsContainerFrame::SetOverflowFrames(nsIPresContext* aPresContext,
                                    nsIFrame*       aOverflowFrames)
{
  nsCOMPtr<nsIPresShell>     presShell;
  nsresult                   rv = NS_ERROR_FAILURE;

  aPresContext->GetShell(getter_AddRefs(presShell));
  if (presShell) {
    nsCOMPtr<nsIFrameManager>  frameManager;
    presShell->GetFrameManager(getter_AddRefs(frameManager));
  
    if (frameManager) {
      rv = frameManager->SetFrameProperty(this, nsLayoutAtoms::overflowProperty,
                                          aOverflowFrames, DestroyOverflowFrames);

      // Verify that we didn't overwrite an existing overflow list
      NS_ASSERTION(rv != NS_IFRAME_MGR_PROP_OVERWRITTEN,
                   "existing overflow list");
    }
  }

  return rv;
}

/**
 * Push aFromChild and its next siblings to the next-in-flow. Change the
 * geometric parent of each frame that's pushed. If there is no next-in-flow
 * the frames are placed on the overflow list (and the geometric parent is
 * left unchanged).
 *
 * Updates the next-in-flow's child count. Does <b>not</b> update the
 * pusher's child count.
 *
 * @param   aFromChild the first child frame to push. It is disconnected from
 *            aPrevSibling
 * @param   aPrevSibling aFromChild's previous sibling. Must not be null. It's
 *            an error to push a parent's first child frame
 */
void
nsContainerFrame::PushChildren(nsIPresContext* aPresContext,
                               nsIFrame*       aFromChild,
                               nsIFrame*       aPrevSibling)
{
  NS_PRECONDITION(nsnull != aFromChild, "null pointer");
  NS_PRECONDITION(nsnull != aPrevSibling, "pushing first child");
#ifdef NS_DEBUG
  nsIFrame* prevNextSibling;
  aPrevSibling->GetNextSibling(&prevNextSibling);
  NS_PRECONDITION(prevNextSibling == aFromChild, "bad prev sibling");
#endif

  // Disconnect aFromChild from its previous sibling
  aPrevSibling->SetNextSibling(nsnull);

  if (nsnull != mNextInFlow) {
    // XXX This is not a very good thing to do. If it gets removed
    // then remove the copy of this routine that doesn't do this from
    // nsInlineFrame.
    nsContainerFrame* nextInFlow = (nsContainerFrame*)mNextInFlow;
    // When pushing and pulling frames we need to check for whether any
    // views need to be reparented.
    for (nsIFrame* f = aFromChild; f; f->GetNextSibling(&f)) {
      nsHTMLContainerFrame::ReparentFrameView(aPresContext, f, this, mNextInFlow);
    }
    nextInFlow->mFrames.InsertFrames(mNextInFlow, nsnull, aFromChild);
  }
  else {
    // Add the frames to our overflow list
    SetOverflowFrames(aPresContext, aFromChild);
  }
}

/**
 * Moves any frames on the overflwo lists (the prev-in-flow's overflow list and
 * the receiver's overflow list) to the child list.
 *
 * Updates this frame's child count and content mapping.
 *
 * @return  PR_TRUE if any frames were moved and PR_FALSE otherwise
 */
PRBool
nsContainerFrame::MoveOverflowToChildList(nsIPresContext* aPresContext)
{
  PRBool result = PR_FALSE;

  // Check for an overflow list with our prev-in-flow
  nsContainerFrame* prevInFlow = (nsContainerFrame*)mPrevInFlow;
  if (nsnull != prevInFlow) {
    nsIFrame* prevOverflowFrames = prevInFlow->GetOverflowFrames(aPresContext,
                                                                 PR_TRUE);
    if (prevOverflowFrames) {
      NS_ASSERTION(mFrames.IsEmpty(), "bad overflow list");
      // When pushing and pulling frames we need to check for whether any
      // views need to be reparented.
      for (nsIFrame* f = prevOverflowFrames; f; f->GetNextSibling(&f)) {
        nsHTMLContainerFrame::ReparentFrameView(aPresContext, f, prevInFlow, this);
      }
      mFrames.InsertFrames(this, nsnull, prevOverflowFrames);
      result = PR_TRUE;
    }
  }

  // It's also possible that we have an overflow list for ourselves
  nsIFrame* overflowFrames = GetOverflowFrames(aPresContext, PR_TRUE);
  if (overflowFrames) {
    NS_ASSERTION(mFrames.NotEmpty(), "overflow list w/o frames");
    mFrames.AppendFrames(nsnull, overflowFrames);
    result = PR_TRUE;
  }
  return result;
}

/////////////////////////////////////////////////////////////////////////////
// Debugging

#ifdef NS_DEBUG
NS_IMETHODIMP
nsContainerFrame::List(nsIPresContext* aPresContext, FILE* out, PRInt32 aIndent) const
{
  IndentBy(out, aIndent);
  ListTag(out);
  nsIView* view;
  GetView(aPresContext, &view);
  if (nsnull != view) {
    fprintf(out, " [view=%p]", view);
  }
  if (nsnull != mNextSibling) {
    fprintf(out, " next=%p", mNextSibling);
  }
  if (nsnull != mPrevInFlow) {
    fprintf(out, " prev-in-flow=%p", mPrevInFlow);
  }
  if (nsnull != mNextInFlow) {
    fprintf(out, " next-in-flow=%p", mNextInFlow);
  }
  fprintf(out, " {%d,%d,%d,%d}", mRect.x, mRect.y, mRect.width, mRect.height);
  if (0 != mState) {
    fprintf(out, " [state=%08x]", mState);
  }
  fprintf(out, " sc=%p", mStyleContext);

  // Output the children
  nsIAtom* listName = nsnull;
  PRInt32 listIndex = 0;
  PRBool outputOneList = PR_FALSE;
  do {
    nsIFrame* kid;
    FirstChild(listName, &kid);
    if (nsnull != kid) {
      if (outputOneList) {
        IndentBy(out, aIndent);
      }
      outputOneList = PR_TRUE;
      nsAutoString tmp;
      if (nsnull != listName) {
        listName->ToString(tmp);
        fputs(tmp, out);
      }
      fputs("<\n", out);
      while (nsnull != kid) {
        nsIFrameDebug*  frameDebug;

        if (NS_SUCCEEDED(kid->QueryInterface(nsIFrameDebug::GetIID(), (void**)&frameDebug))) {
          frameDebug->List(aPresContext, out, aIndent + 1);
        }
        kid->GetNextSibling(&kid);
      }
      IndentBy(out, aIndent);
      fputs(">\n", out);
    }
    NS_IF_RELEASE(listName);
    GetAdditionalChildListName(listIndex++, &listName);
  } while(nsnull != listName);

  if (!outputOneList) {
    fputs("<>\n", out);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsContainerFrame::SizeOf(nsISizeOfHandler* aHandler, PRUint32* aResult) const
{
  if (!aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  *aResult = sizeof(*this);
  return NS_OK;
}
#endif
