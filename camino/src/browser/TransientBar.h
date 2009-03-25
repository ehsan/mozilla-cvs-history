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

#import <Cocoa/Cocoa.h>

// The position of the bar in relation to the browser content.
typedef enum {
  eTransientBarPositionTop,
  eTransientBarPositionBottom
} ETransientBarPosition;

//
// TransientBar
//
// An abstract class for creating an information bar which can be displayed around
// browser content.
//
@interface TransientBar : NSView {
 @private
  NSView *mLastKeySubview;
}

// Bars may adjust their height inside this method to accommodate the supplied width.
// The new frame value will be re-fetched by the browser after the initial call, to
// determine any changes in height.
- (void)setFrame:(NSRect)aNewFrame;

// The last view in the bar's internal key view loop.
- (NSView *)lastKeySubview;
- (void)setLastKeySubview:(NSView *)aLastKeySubview;

// Indicates whether the bar could be replaced, by a subsequent call to insert 
// another one in the same position. Subclasses can override this method to ensure
// important bars (security, for instance) are not replaced by less urgent ones.
- (BOOL)isReplaceable;

@end
