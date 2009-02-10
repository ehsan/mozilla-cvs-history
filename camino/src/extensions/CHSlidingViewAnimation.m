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
 * Sean Murphy
 * Portions created by the Initial Developer are Copyright (C) 2008
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

#import "CHSlidingViewAnimation.h"

static const NSTimeInterval kDefaultAnimationDuration = 0.5;

@interface CHSlidingViewAnimation (Private)

- (void)setNewPosition:(NSValue*)newPoint;

@end

@implementation CHSlidingViewAnimation

- (id)initWithAnimationTarget:(id)targetObject
{
  if ((self = [super initWithDuration:kDefaultAnimationDuration animationCurve:NSAnimationEaseInOut]))
    [self setAnimationTarget:targetObject];

  return self;
}

- (void)dealloc
{
  [mAnimationTarget release];
  [super dealloc];
}

- (void)setCurrentProgress:(NSAnimationProgress)progress
{
  // We need to call super to update the progress value.
  [super setCurrentProgress:progress];

  // Calculate a new position based on the current progress.
  NSPoint newPosition = 
    NSMakePoint(floor(mStartLocation.x + ((mEndLocation.x - mStartLocation.x) * progress)),
                floor(mStartLocation.y + ((mEndLocation.y - mStartLocation.y) * progress)));

  // If the animation in running on a secondary thread, we should update the UI from the main thread.
  if ([self animationBlockingMode] == NSAnimationNonblockingThreaded) {
    [self performSelectorOnMainThread:@selector(setNewPosition:) 
                           withObject:[NSValue valueWithPoint:newPosition] 
                        waitUntilDone:NO];
  }
  else {
    [self setNewPosition:[NSValue valueWithPoint:newPosition]];
  }
}

- (void)setNewPosition:(NSValue*)newPoint
{
  // If the animation target is a NSView subclass, setFrameOrigin does not automatically mark
  // the view or its superview as needing display.
  if ([mAnimationTarget isKindOfClass:[NSView class]]) {
    [[mAnimationTarget superview] setNeedsDisplayInRect:[mAnimationTarget frame]];
    [mAnimationTarget setFrameOrigin:[newPoint pointValue]];
    [mAnimationTarget setNeedsDisplay:YES];
  }
  else {
    [mAnimationTarget setFrameOrigin:[newPoint pointValue]];
  }
}

- (void)stopAnimation
{
  if ([self isAnimating])
    [super stopAnimation];

  // NSAnimation does not reset progress when the animation is stopped.
  [super setCurrentProgress:0.0];
}

#pragma mark -

- (id)animationTarget
{
  return [[mAnimationTarget retain] autorelease]; 
}

- (void)setAnimationTarget:(id)newTargetObject
{
  if (mAnimationTarget == newTargetObject)
    return;

  if (![newTargetObject respondsToSelector:@selector(setFrameOrigin:)]) {
    [NSException raise:@"CHSlidingViewAnimationInvalidAnimationTarget" 
                format:@"The animation target must be either an NSWindow or NSView object"];
    mAnimationTarget = nil;
    return;
  }

  [mAnimationTarget release];
  mAnimationTarget = [newTargetObject retain];
}

- (NSPoint)startLocation
{
  return mStartLocation;
}

- (void)setStartLocation:(NSPoint)newStartLocation
{
  mStartLocation = newStartLocation;
}

- (NSPoint)endLocation
{
  return mEndLocation;
}

- (void)setEndLocation:(NSPoint)newEndLocation
{
  mEndLocation = newEndLocation;
}

@end
