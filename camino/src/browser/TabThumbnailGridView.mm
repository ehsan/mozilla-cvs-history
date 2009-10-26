/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
* The Original Code is mozilla.org code.
*
* The Initial Developer of the Original Code is
* Jeff Dlouhy.
* Portions created by the Initial Developer are Copyright (C) 2007
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*   Jeff Dlouhy <Jeff.Dlouhy@gmail.com>
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

#import "TabThumbnailGridView.h"
#import "ThumbnailView.h"

#import "MainController.h"

#import "BrowserWindowController.h"
#import "BrowserTabViewItem.h"
#import "BrowserWrapper.h"
#import "CHGradient.h"
#import "NSArray+Utils.h"
#import "NSView+Utils.h"

const int kVerticalPadding = 25;
const int kHorizontalPadding = 25;

@interface TabThumbnailGridView (Private)
- (void)updateGridSizeFor:(int)num;
- (void)layoutThumbnails;
- (void)setUpKeyViewLoopWithInitialIndex:(unsigned int)startIndex;
- (void)createThumbnailViews;
- (BOOL)validateTabExists:(ThumbnailView*)tabThumb;
@end

@implementation TabThumbnailGridView

#pragma mark NSView

- (BOOL)isFlipped
{
  return YES;
}

//
// Updates the grid when the frame size changes
//
- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
  [self layoutThumbnails];
}

- (void)drawRect:(NSRect)rect
{
  CHGradient* gradient =
    [[[CHGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:(250.0/255.0)
                                                                       alpha:1.0]
                                   endingColor:[NSColor colorWithDeviceWhite:(200.0/255.0)
                                                                       alpha:1.0]] autorelease];
  [gradient drawInRect:[self bounds] angle:90.0];
}

#pragma mark Private

//
// Creates a ThumbnailView for each and adds it as a subview, and schedules
// loading of actual thumbnails.
//
- (void)createThumbnailViews
{
  // The window isn't hooked up yet, so go through the superview
  BrowserWindowController* bwc = (BrowserWindowController*)[[[self superview] window] windowController];
  BrowserTabView* tabView = [bwc tabBrowser];
  NSArray* openTabs = [tabView tabViewItems];

  NSMutableArray* thumbViewsToLoad = [NSMutableArray arrayWithCapacity:[openTabs count]];

  NSSize browserContentSize = [[bwc browserWrapper] frame].size;
  NSImage* placeholderThumb = [[[NSImage alloc] initWithSize:browserContentSize] autorelease];
  [placeholderThumb lockFocus];
  [[NSColor whiteColor] set];
  [NSBezierPath fillRect:NSMakeRect(0, 0, browserContentSize.width, browserContentSize.height)];
  [placeholderThumb unlockFocus];

  NSEnumerator* tabEnum = [openTabs objectEnumerator];
  BrowserTabViewItem* tabViewItem;
  while ((tabViewItem = [tabEnum nextObject])) {
    ThumbnailView* curThumbView = [[[ThumbnailView alloc] init] autorelease];
    if (curThumbView) {
      [curThumbView setThumbnail:placeholderThumb];
      [curThumbView setRepresentedObject:tabViewItem];
      [curThumbView setTitle:[tabViewItem label]];
      [curThumbView setDelegate:self];
      [self addSubview:curThumbView];
      [thumbViewsToLoad addObject:curThumbView];
    }
  }

  [self layoutThumbnails];
  unsigned int selectedIndex = [tabView indexOfTabViewItem:[tabView selectedTabViewItem]];
  [self setUpKeyViewLoopWithInitialIndex:selectedIndex];

  // Start the tab thumbnailing process. We do this one tab at a time, scheduled
  // with the runloop, so that the grid view comes up immediately and the UI
  // stays responsive the whole time.
  [self performSelector:@selector(loadNextThumbnailForViews:)
             withObject:thumbViewsToLoad
             afterDelay:0];
}

//
// Wires up a key loop that cycles through all the thumbnails, starting with
// the one at the given index.
//
// This assumes that there's nothing outside this view that needs to be part of
// the loop, so if that changes in the future this will need adjusting.
//
- (void)setUpKeyViewLoopWithInitialIndex:(unsigned int)startIndex
{
  // First make a self-contained tab loop.
  NSArray* thumbViews = [self subviews];
  unsigned int thumbViewCount = [thumbViews count];
  for (unsigned int i = 0; i < thumbViewCount ; i++) {
    unsigned int nextThumbIndex = (i + 1) % thumbViewCount;
    ThumbnailView* thumbView = [thumbViews objectAtIndex:i];
    [thumbView setNextKeyView:[thumbViews objectAtIndex:nextThumbIndex]];
  }

  // Then, insert the grid view itself just before the selected tab, so that
  // keyboard selection will start there.
  [self setNextKeyView:[thumbViews objectAtIndex:startIndex]];
  unsigned int previousThumbIndex =
    (startIndex + thumbViewCount - 1) % thumbViewCount;
  [[thumbViews objectAtIndex:previousThumbIndex] setNextKeyView:self];
}

- (void)loadNextThumbnailForViews:(NSMutableArray*)thumbViewsToLoad
{
  // First, bail if the tab grid view has been dismissed.
  if (![self superview])
    return;

  // Load the next view if we aren't done yet.
  ThumbnailView* thumbView = [thumbViewsToLoad firstObject];
  if (!thumbView)
    return;
  [thumbViewsToLoad removeObjectAtIndex:0];
  BrowserTabViewItem* tabViewItem = [thumbView representedObject];
  // Ensure that the tab hasn't somehow vanished out from under us. The user
  // shouldn't be able to close it, but a site potentially could via JS.
  if (![self validateTabExists:thumbView])
    return;

  NSImage* thumb = [[[tabViewItem view] browserView] snapshot];
  if (thumb)
    [thumbView setThumbnail:thumb];
  // Schedule the next iteration.
  [self performSelector:@selector(loadNextThumbnailForViews:)
             withObject:thumbViewsToLoad
             afterDelay:0];
}

//
// Draw the views when it's added to a view
//
- (void)viewDidMoveToSuperview
{
  if ([self superview])
    [self createThumbnailViews];
  else
    [self removeAllSubviews];
}

//
// Checks that the tab this thumbnail represents still exists, and removes it
// from the grid if not.
//
- (BOOL)validateTabExists:(ThumbnailView*)tabThumb
{
  BrowserWindowController* bwc = (BrowserWindowController*)[[[self superview] window] windowController];
  if (![[[bwc tabBrowser] tabViewItems] containsObject:[tabThumb representedObject]]) {
    // Remove the now-dead tab, and re-flow
    [tabThumb removeFromSuperview];
    [self layoutThumbnails];
    return NO;
  }
  return YES;
}

//
// Change the tab to the selected ThumbnailView
//
- (void)thumbnailViewWasSelected:(ThumbnailView*)selectedView
{
  // Ensure that the tab hasn't somehow vanished out from under us. The user
  // shouldn't be able to close it, but a site potentially could via JS.
  // The user experience is odd here, but this is so unlikely that setting up
  // a notification-based live update seems like overkill. 
  if (![self validateTabExists:selectedView])
    return;

  BrowserWindowController* bwc = (BrowserWindowController*)[[self window] windowController];
  BrowserTabView* tabView = [bwc tabBrowser];

  [tabView selectTabViewItem:[selectedView representedObject]];
  [bwc toggleTabThumbnailView:self];
}

//
// Update the grid size based on the number of images
// This tries to keep the grid as square as possible with a
// max of n - 1 views on the bottom row when num is odd
//
- (void)updateGridSizeFor:(int)num
{
  mNumCols = ceilf(sqrtf(num));
  mNumRows = ceilf((float) num / mNumCols);
}

//
// Calculates where each of the subviews should be drawn
// then sets each of their frames
//
- (void)layoutThumbnails
{
  [self updateGridSizeFor:[[self subviews] count]];
  NSSize viewSize = [self bounds].size;
  float aspectRatio = viewSize.width / viewSize.height;
  float subviewWidth = (viewSize.width - (kHorizontalPadding * (mNumCols + 1))) / mNumCols;
  float subviewHeight = subviewWidth / aspectRatio;

  float newX = kHorizontalPadding;
  // Centers the grid vertically
  float newY = kVerticalPadding + ((viewSize.height - ((mNumRows * subviewHeight) + ((mNumRows + 1) * kVerticalPadding))) / 2);

  // This will allow us to center the last row if # of views is < mNumCols
  unsigned int firstItemInLastRow = ((mNumRows - 1) * mNumCols);
  unsigned int numItemsInLastRow = ([[self subviews] count] - firstItemInLastRow);

  unsigned int rowCount = 0;
  unsigned int totalCount = 0;

  while (rowCount < mNumRows) {
    for (unsigned int x = 0; x < mNumCols && totalCount < [[self subviews] count]; x++) {
      NSRect frame = NSMakeRect(newX, newY, subviewWidth, subviewHeight);
      [[[self subviews] objectAtIndex:totalCount++] setFrame:frame];
      newX += kHorizontalPadding + subviewWidth;
    }

    // Once we are done with a row we add on to the y and move on
    // The newX will be centered if the last row has less views than mNumCol
    if (totalCount == firstItemInLastRow && numItemsInLastRow != mNumCols)
      newX = kHorizontalPadding + ((viewSize.width - (kHorizontalPadding + (numItemsInLastRow * (subviewWidth + kHorizontalPadding)))) / 2);
    else
      newX = kHorizontalPadding;

    newY += kVerticalPadding + subviewHeight;
    rowCount++;
  }
}

@end
