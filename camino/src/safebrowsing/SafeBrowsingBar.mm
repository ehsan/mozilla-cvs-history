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
 * The Original Code is Camino code.
 *
 * The Initial Developer of the Original Code is
 * Sean Murphy.
 * Portions created by the Initial Developer are Copyright (C) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sean Murphy <murph@seanmurph.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#import "SafeBrowsingBar.h"
#import "CHGradient.h"
#import "RolloverImageButton.h"

@implementation SafeBrowsingBar

- (void)awakeFromNib
{
  [mCloseButton setImage:[NSImage imageNamed:@"popup_close"]];
  [mCloseButton setAlternateImage:[NSImage imageNamed:@"popup_close_pressed"]];
  [mCloseButton setHoverImage:[NSImage imageNamed:@"popup_close_hover"]];
  
  // The warning label's text color is set to black in the nib so it's visible
  // when editing. Revert it to the actual white color we display it in on the bar.
  [mWarningLabelTextField setTextColor:[NSColor whiteColor]];

  [self setLastKeySubview:mCloseButton];
}

- (void)drawRect:(NSRect)rect
{
  NSColor* startColor = [NSColor colorWithCalibratedRed:0.654902f green:0.101961f blue:0.094118f 
                                                  alpha:1.0f];
  NSColor* endColor = [NSColor colorWithCalibratedRed:0.349020f green:0.015686f blue:0.015686f 
                                                alpha:1.0f];

  CHGradient* backgroundGradient =
    [[[CHGradient alloc] initWithStartingColor:startColor
                                   endingColor:endColor] autorelease];
  
  [backgroundGradient drawInRect:[self bounds] angle:270.0];

  [[NSColor colorWithCalibratedWhite:0.70 alpha:1.0] set];
  [NSBezierPath strokeLineFromPoint:NSMakePoint(0, 0) toPoint:NSMakePoint(NSMaxX([self bounds]), 0)];
}

- (BOOL)isReplaceable
{
  return NO;
}

@end
