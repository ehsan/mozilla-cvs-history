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


#include "nsSpinnerFrame.h"

#include "nsIContent.h"
#include "prtypes.h"
#include "nsIAtom.h"
#include "nsIPresContext.h"
#include "nsIStyleContext.h"
#include "nsCSSRendering.h"
#include "nsINameSpaceManager.h"


//
// NS_NewSpinnerFrame
//
// Wrapper for creating a new spinner
//
nsresult
NS_NewSpinnerFrame(nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsSpinnerFrame* it = new nsSpinnerFrame;
  if ( !it )
    return NS_ERROR_OUT_OF_MEMORY;
  *aNewFrame = it;
  return NS_OK;
}


//
// nsSpinnerFrame cntr
//
nsSpinnerFrame::nsSpinnerFrame()
{

} // cntr


//
// Paint
//
// Overidden to handle ???
//
NS_METHOD 
nsSpinnerFrame::Paint(nsIPresContext& aPresContext,
                              nsIRenderingContext& aRenderingContext,
                              const nsRect& aDirtyRect,
                              nsFramePaintLayer aWhichLayer)
{
  return nsLeafFrame::Paint(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer);
}


//
// GetDesiredSize
//
// For now, be as big as CSS wants us to be, or some small default size.
//
void
nsSpinnerFrame :: GetDesiredSize(nsIPresContext* aPresContext,
                                           const nsHTMLReflowState& aReflowState,
                                           nsHTMLReflowMetrics& aDesiredLayoutSize)
{
  const int CSS_NOTSET = -1;

  nsSize styleSize;
  if (NS_UNCONSTRAINEDSIZE != aReflowState.mComputedWidth) {
    styleSize.width = aReflowState.mComputedWidth;
  }
  else {
    styleSize.width = CSS_NOTSET;
  }
  if (NS_UNCONSTRAINEDSIZE != aReflowState.mComputedHeight) {
    styleSize.height = aReflowState.mComputedHeight;
  }
  else {
    styleSize.height = CSS_NOTSET;
  }

  // subclasses should always override this method, but if not and no css, make it small
  aDesiredLayoutSize.width  = (styleSize.width  > CSS_NOTSET) ? styleSize.width  : 200;
  aDesiredLayoutSize.height = (styleSize.height > CSS_NOTSET) ? styleSize.height : 200;
  aDesiredLayoutSize.ascent = aDesiredLayoutSize.height;
  aDesiredLayoutSize.descent = 0;
  if (aDesiredLayoutSize.maxElementSize) {
    aDesiredLayoutSize.maxElementSize->width  = aDesiredLayoutSize.width;
    aDesiredLayoutSize.maxElementSize->height = aDesiredLayoutSize.height;
  }

} // GetDesiredSize
