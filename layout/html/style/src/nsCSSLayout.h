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
#ifndef nsCSSLayout_h___
#define nsCSSLayout_h___

#include "nsCoord.h"
class nsIFrame;
class nsIPresContext;
struct nsSize;
struct nsStyleFont;
struct nsStyleText;

class nsCSSLayout {
public:
  /**
   * Perform vertical alignment ala CSS. Return the height of the
   * line.
   */
  static nscoord VerticallyAlignChildren(nsIPresContext* aPresContext,
                                         nsIFrame* aContainer,
                                         nsStyleFont* aContainerFont,
                                         nscoord aY0,
                                         nsIFrame* aFirstChild,
                                         PRInt32 aChildCount,
                                         nscoord* aAscents,
                                         nscoord aMaxAscent);

  /**
   * Perform horizontal alignment ala CSS.
   */
  static void HorizontallyPlaceChildren(nsIPresContext* aPresContext,
                                        nsIFrame* aContainer,
                                        nsStyleText* aContainerStyle,
                                        nsIFrame* aFirstChild,
                                        PRInt32 aChildCount,
                                        nscoord aLineWidth,
                                        nscoord aMaxWidth);

  /**
   * Perform relative positioning ala CSS
   */
  static void RelativePositionChildren(nsIPresContext* aPresContext,
                                       nsIFrame* aContainer,
                                       nsIFrame* aFirstChild,
                                       PRInt32 aChildCount);

  /**
   * Get the CSS size (width & height) values for the given
   * frame. The value returned indicates which values were set
   * stylistically.
   */
  static PRIntn GetStyleSize(nsIPresContext* aPresContext,
                             nsIFrame* aFrame,
                             nsSize& aStyleSize);

// Return value from GetStyleSize
#define NS_SIZE_HAS_NONE   0x0
#define NS_SIZE_HAS_WIDTH  0x1
#define NS_SIZE_HAS_HEIGHT 0x2
#define NS_SIZE_HAS_BOTH   0x3

};

#endif /* nsCSSLayout_h___ */
