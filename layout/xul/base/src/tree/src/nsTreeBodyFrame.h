/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Original Author: David W. Hyatt (hyatt@netscape.com)
 *
 * Contributor(s):
 *   Dean Tessman <dean_tessman@hotmail.com>
 *   Brian Ryner <bryner@brianryner.com>
 *   Jan Varga <varga@nixcorp.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsLeafBoxFrame.h"
#include "nsITreeBoxObject.h"
#include "nsITreeView.h"
#include "nsICSSPseudoComparator.h"
#include "nsIScrollbarMediator.h"
#include "nsIDragSession.h"
#include "nsITimer.h"
#include "nsIReflowCallback.h"
#include "nsILookAndFeel.h"
#include "nsValueArray.h"
#include "nsTreeStyleCache.h"
#include "nsTreeColumns.h"
#include "nsTreeImageListener.h"
#include "nsAutoPtr.h"

// The actual frame that paints the cells and rows.
class nsTreeBodyFrame : public nsLeafBoxFrame,
                        public nsITreeBoxObject,
                        public nsICSSPseudoComparator,
                        public nsIScrollbarMediator,
                        public nsIReflowCallback
{
public:
  nsTreeBodyFrame(nsIPresShell* aPresShell);
  virtual ~nsTreeBodyFrame();

  NS_DECL_ISUPPORTS
  NS_DECL_NSITREEBOXOBJECT

  // nsIBox
  NS_IMETHOD GetMinSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD SetBounds(nsBoxLayoutState& aBoxLayoutState, const nsRect& aRect);

  // nsIReflowCallback
  NS_IMETHOD ReflowFinished(nsIPresShell* aPresShell, PRBool* aFlushFlag);

  // nsICSSPseudoComparator
  NS_IMETHOD PseudoMatches(nsIAtom* aTag, nsCSSSelector* aSelector, PRBool* aResult);

  // nsIScrollbarMediator
  NS_IMETHOD PositionChanged(PRInt32 aOldIndex, PRInt32& aNewIndex);
  NS_IMETHOD ScrollbarButtonPressed(PRInt32 aOldIndex, PRInt32 aNewIndex);
  NS_IMETHOD VisibilityChanged(PRBool aVisible) { Invalidate(); return NS_OK; };

  // Overridden from nsIFrame to cache our pres context.
  NS_IMETHOD Init(nsIPresContext* aPresContext, nsIContent* aContent,
                  nsIFrame* aParent, nsStyleContext* aContext, nsIFrame* aPrevInFlow);
  NS_IMETHOD Destroy(nsIPresContext* aPresContext);
  NS_IMETHOD GetCursor(nsIPresContext* aPresContext,
                       nsPoint& aPoint,
                       PRInt32& aCursor);

  NS_IMETHOD HandleEvent(nsIPresContext* aPresContext,
                         nsGUIEvent* aEvent,
                         nsEventStatus* aEventStatus);

  // Painting methods.
  // Paint is the generic nsIFrame paint method.  We override this method
  // to paint our contents (our rows and cells).
  NS_IMETHOD Paint(nsIPresContext*      aPresContext,
                   nsIRenderingContext& aRenderingContext,
                   const nsRect&        aDirtyRect,
                   nsFramePaintLayer    aWhichLayer,
                   PRUint32             aFlags = 0);

  // This method paints a specific column background of the tree.
  void PaintColumn(nsTreeColumn*        aColumn,
                   const nsRect&        aColumnRect,
                   nsIPresContext*      aPresContext,
                   nsIRenderingContext& aRenderingContext,
                   const nsRect&        aDirtyRect);

  // This method paints a single row in the tree.
  void PaintRow(PRInt32              aRowIndex,
                const nsRect&        aRowRect,
                nsIPresContext*      aPresContext,
                nsIRenderingContext& aRenderingContext,
                const nsRect&        aDirtyRect);

  // This method paints a single separator in the tree.
  void PaintSeparator(PRInt32              aRowIndex,
                      const nsRect&        aSeparatorRect,
                      nsIPresContext*      aPresContext,
                      nsIRenderingContext& aRenderingContext,
                      const nsRect&        aDirtyRect);

  // This method paints a specific cell in a given row of the tree.
  void PaintCell(PRInt32              aRowIndex, 
                 nsTreeColumn*        aColumn,
                 const nsRect&        aCellRect,
                 nsIPresContext*      aPresContext,
                 nsIRenderingContext& aRenderingContext,
                 const nsRect&        aDirtyRect,
                 nscoord&             aCurrX);

  // This method paints the twisty inside a cell in the primary column of an tree.
  void PaintTwisty(PRInt32              aRowIndex,
                   nsTreeColumn*        aColumn,
                   const nsRect&        aTwistyRect,
                   nsIPresContext*      aPresContext,
                   nsIRenderingContext& aRenderingContext,
                   const nsRect&        aDirtyRect,
                   nscoord&             aRemainingWidth,
                   nscoord&             aCurrX);

  // This method paints the image inside the cell of an tree.
  void PaintImage(PRInt32              aRowIndex,
                  nsTreeColumn*        aColumn,
                  const nsRect&        aImageRect,
                  nsIPresContext*      aPresContext,
                  nsIRenderingContext& aRenderingContext,
                  const nsRect&        aDirtyRect,
                  nscoord&             aRemainingWidth,
                  nscoord&             aCurrX);

  // This method paints the text string inside a particular cell of the tree.
  void PaintText(PRInt32              aRowIndex, 
                 nsTreeColumn*        aColumn,
                 const nsRect&        aTextRect,
                 nsIPresContext*      aPresContext,
                 nsIRenderingContext& aRenderingContext,
                 const nsRect&        aDirtyRect,
                 nscoord&             aCurrX);

  // This method paints the checkbox inside a particular cell of the tree.
  void PaintCheckbox(PRInt32              aRowIndex, 
                     nsTreeColumn*        aColumn,
                     const nsRect&        aCheckboxRect,
                     nsIPresContext*      aPresContext,
                     nsIRenderingContext& aRenderingContext,
                     const nsRect&        aDirtyRect);

  // This method paints the progress meter inside a particular cell of the tree.
  void PaintProgressMeter(PRInt32              aRowIndex, 
                          nsTreeColumn*        aColumn,
                          const nsRect&        aProgressMeterRect,
                          nsIPresContext*      aPresContext,
                          nsIRenderingContext& aRenderingContext,
                          const nsRect&        aDirtyRect);

  // This method paints a drop feedback of the tree.
  void PaintDropFeedback(const nsRect&        aDropFeedbackRect, 
                         nsIPresContext*      aPresContext,
                         nsIRenderingContext& aRenderingContext,
                         const nsRect&        aDirtyRect);

  // This method is called with a specific style context and rect to
  // paint the background rect as if it were a full-blown frame.
  void PaintBackgroundLayer(nsStyleContext*      aStyleContext,
                            nsIPresContext*      aPresContext, 
                            nsIRenderingContext& aRenderingContext, 
                            const nsRect&        aRect,
                            const nsRect&        aDirtyRect);

  friend nsresult NS_NewTreeBodyFrame(nsIPresShell* aPresShell, 
                                          nsIFrame** aNewFrame);

protected:
  PRInt32 GetLastVisibleRow() {
    return mTopRowIndex + mPageLength;
  };

  // An internal hit test.
  PRInt32 GetRowAt(PRInt32 aX, PRInt32 aY);

  // A helper used when hit testing.
  nsIAtom* GetItemWithinCellAt(PRInt32 aX, const nsRect& aCellRect, PRInt32 aRowIndex,
                               nsTreeColumn* aColumn);

  // An internal hit test.
  void GetCellAt(PRInt32 aX, PRInt32 aY, PRInt32* aRow, nsTreeColumn** aCol,
                 nsIAtom** aChildElt);

  // Fetch an image from the image cache.
  nsresult GetImage(PRInt32 aRowIndex, nsTreeColumn* aCol, PRBool aUseContext,
                    nsStyleContext* aStyleContext, PRBool& aAllowImageRegions, imgIContainer** aResult);

  // Returns the size of a given image.   This size *includes* border and
  // padding.  It does not include margins.
  nsRect GetImageSize(PRInt32 aRowIndex, nsTreeColumn* aCol, PRBool aUseContext, nsStyleContext* aStyleContext);

  // Returns the height of rows in the tree.
  PRInt32 GetRowHeight();

  // Returns our indentation width.
  PRInt32 GetIndentation();

  // Calculates our width/height once border and padding have been removed.
  void CalcInnerBox();

  // Looks up a style context in the style cache.  On a cache miss we resolve
  // the pseudo-styles passed in and place them into the cache.
  nsStyleContext* GetPseudoStyleContext(nsIAtom* aPseudoElement);

  // Makes |mScrollbar| non-null if at all possible, and returns it.
  nsIFrame* EnsureScrollbar();

  // Update the curpos of the scrollbar.
  void UpdateScrollbar();

  // Update the maxpos of the scrollbar.
  void InvalidateScrollbar();

  // Check vertical overflow.
  void CheckVerticalOverflow();

  // Use to auto-fill some of the common properties without the view having to do it.
  // Examples include container, open, selected, and focus.
  void PrefillPropertyArray(PRInt32 aRowIndex, nsTreeColumn* aCol);

  // Our internal scroll method, used by all the public scroll methods.
  nsresult ScrollInternal(PRInt32 aRow);
  
  // Convert pixels, probably from an event, into twips in our coordinate space.
  void AdjustEventCoordsToBoxCoordSpace(PRInt32 aX, PRInt32 aY, PRInt32* aResultX, PRInt32* aResultY);

  // Convert a border style into line style.
  nsLineStyle ConvertBorderStyleToLineStyle(PRUint8 aBorderStyle);

  // Cache the box object
  void EnsureBoxObject();

  void EnsureView();

  // Get the base element, <tree> or <select>
  nsIContent* GetBaseElement();

  void GetCellWidth(PRInt32 aRow, nsTreeColumn* aCol,
                    nsIRenderingContext* aRenderingContext,
                    nscoord& aDesiredSize, nscoord& aCurrentSize);
  nscoord CalcMaxRowWidth();

  PRBool CanAutoScroll(PRInt32 aRowIndex);

  // Calc the row and above/below/on status given where the mouse currently is hovering.
  // Also calc if we're in the region in which we want to auto-scroll the tree.
  // A positive value of |aScrollLines| means scroll down, a negative value
  // means scroll up, a zero value means that we aren't in drag scroll region.
  void ComputeDropPosition(nsGUIEvent* aEvent, PRInt32* aRow, PRInt16* aOrient,
                           PRInt16* aScrollLines);

  // Mark ourselves dirty if we're a select widget
  void MarkDirtyIfSelect();

  void InvalidateDropFeedback(PRInt32 aRow, PRInt16 aOrientation) {
    InvalidateRow(aRow);
    if (aOrientation != nsITreeView::DROP_ON)
      InvalidateRow(aRow + aOrientation);
  };

  // Create a new timer. This method is used to delay various actions like
  // opening/closing folders or tree scrolling.
  // aID is type of the action, aFunc is the function to be called when
  // the timer fires and aType is type of timer - one shot or repeating.
  nsresult CreateTimer(const nsILookAndFeel::nsMetricID aID,
                       nsTimerCallbackFunc aFunc, PRInt32 aType,
                       nsITimer** aTimer);

  static void OpenCallback(nsITimer *aTimer, void *aClosure);

  static void CloseCallback(nsITimer *aTimer, void *aClosure);

  static void LazyScrollCallback(nsITimer *aTimer, void *aClosure);

  static void ScrollCallback(nsITimer *aTimer, void *aClosure);

protected: // Data Members
  // Our cached pres context.
  nsIPresContext* mPresContext;

  // The cached box object parent.
  nsCOMPtr<nsITreeBoxObject> mTreeBoxObject;

  // Cached column information.
  nsRefPtr<nsTreeColumns> mColumns;

  // The current view for this tree widget.  We get all of our row and cell data
  // from the view.
  nsCOMPtr<nsITreeView> mView;    

  // A cache of all the style contexts we have seen for rows and cells of the tree.  This is a mapping from
  // a list of atoms to a corresponding style context.  This cache stores every combination that
  // occurs in the tree, so for n distinct properties, this cache could have 2 to the n entries
  // (the power set of all row properties).
  nsTreeStyleCache mStyleCache;

  // A hashtable that maps from URLs to image requests.  The URL is provided
  // by the view or by the style context. The style context represents
  // a resolved :-moz-tree-cell-image (or twisty) pseudo-element.
  // It maps directly to an imgIRequest.
  nsSupportsHashtable* mImageCache;

  // Our vertical scrollbar.
  nsIFrame* mScrollbar;

  // The index of the first visible row and the # of rows visible onscreen.  
  // The tree only examines onscreen rows, starting from
  // this index and going up to index+pageLength.
  PRInt32 mTopRowIndex;
  PRInt32 mPageLength;

  // Cached heights and indent info.
  nsRect mInnerBox;
  PRInt32 mRowHeight;
  PRInt32 mIndentation;
  nscoord mStringWidth;

  // A scratch array used when looking up cached style contexts.
  nsCOMPtr<nsISupportsArray> mScratchArray;

  // Whether or not we're currently focused.
  PRPackedBool mFocused;

  // Do we have a fixed number of onscreen rows?
  PRPackedBool mHasFixedRowCount;

  PRPackedBool mVerticalOverflow;

  PRPackedBool mReflowCallbackPosted;

  PRInt32 mUpdateBatchNest;

  // Cached row count.
  PRInt32 mRowCount;

  class Slots {
    public:
      Slots()
        : mValueArray(~PRInt32(0)) {
      };

      friend class nsTreeBodyFrame;

    protected:
      // If the drop is actually allowed here or not.
      PRBool                   mDropAllowed;

      // The row the mouse is hovering over during a drop.
      PRInt32                  mDropRow;

      // Where we want to draw feedback (above/on this row/below) if allowed.
      PRInt16                  mDropOrient;

      // Number of lines to be scrolled.
      PRInt16                  mScrollLines;

      nsCOMPtr<nsIDragSession> mDragSession;

      // Timer for opening/closing spring loaded folders or scrolling the tree.
      nsCOMPtr<nsITimer>       mTimer;

      // A value array used to keep track of all spring loaded folders.
      nsValueArray             mValueArray;
  };

  Slots* mSlots;
}; // class nsTreeBodyFrame
