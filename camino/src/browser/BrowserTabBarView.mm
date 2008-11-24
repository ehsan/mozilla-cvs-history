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
 * The Original Code is tab UI for Camino.
 *
 * The Initial Developer of the Original Code is
 * Geoff Beier.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Geoff Beier <me@mollyandgeoff.com>
 *   Aaron Schulman <aschulm@gmail.com>
 *   Desmond Elliott <d.elliott@inf.ed.ac.uk>
 *   Ian Leue <froodian@gmail.com>
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

#include <Carbon/Carbon.h>

#import "BrowserTabBarView.h"
#import "BrowserTabView.h"
#import "BrowserTabViewItem.h"
#import "TabButtonView.h"
#import "RolloverImageButton.h"
#import "BrowserWrapper.h"

#import "NSArray+Utils.h"
#import "NSImage+Utils.h"
#import "NSMenu+Utils.h"
#import "NSPasteboard+Utils.h"
#import "NSView+Utils.h"
#import "CHSlidingViewAnimation.h"

@interface BrowserTabBarView(TabBarViewPrivate)

-(void)layoutButtonsPreservingVisibility:(BOOL)preserveVisibility;
-(void)loadImages;
-(void)drawTabBarBackgroundInRect:(NSRect)rect withActiveTabRect:(NSRect)tabRect;
-(void)drawTabBarBackgroundHiliteRectInRect:(NSRect)rect;
-(TabButtonView*)buttonAtPoint:(NSPoint)clickPoint;
-(void)registerTabButtonsForTracking;
-(void)unregisterTabButtonsForTracking;
-(void)ensureOverflowButtonsInitted;
-(NSRect)tabsRect;
-(NSRect)tabsRectWithOverflow:(BOOL)overflowing;
-(BrowserTabViewItem *)tabViewItemUnderMouse;
-(NSString*)view:(NSView*)view stringForToolTip:(NSToolTipTag)tag point:(NSPoint)point userData:(void*)userData;
-(NSButton*)newOverflowButtonForImageNamed:(NSString*)imageName;
-(void)setLeftmostVisibleTabIndex:(int)index;
-(NSButton*)scrollButtonAtPoint:(NSPoint)clickPoint;
-(BOOL)tabIndexIsVisible:(int)index;
-(void)setOverflowButtonsVisible:(BOOL)visible;
-(float)verticalOriginForButtonWithHeight:(float)height;
-(void)scrollLeft:(id)sender;
-(void)scrollRight:(id)sender;
-(int)rightmostVisibleTabIndex;
-(void)slideTabButtonsAroundCurrentDragPosition;
-(void)saveTabButtonFramesForDragging;
-(TabButtonView*)tabButtonAtIndex:(int)index;
-(NSWindow*)newFloatingWindowForDraggedTab;
-(void)slidingTabAnimationEnded:(NSNotification*)aNotification;
-(void)calculateOverflowTabValues;
-(BOOL)isReorderingTabViewItemsDuringDrag;
-(void)setReorderingTabViewItemsDuringDrag:(BOOL)flag;
-(void)currentlyDraggingTabClosed;

@end

static const float kTabBarDefaultHeight = 22.0;
static const float kTabBottomPad = 4.0;           // height of the padding below tabs

@implementation BrowserTabBarView

static const float kTabBarMargin = 5.0;           // left/right margin for tab bar
static const float kMinTabWidth = 100.0;          // the smallest tabs that will be drawn
static const float kMaxTabWidth = 175.0;          // the widest tabs that will be drawn

static const int kTabDragThreshold = 3;           // distance a drag must go before we start dnd

static const float kScrollButtonDelay = 0.4;      // how long a button must be held before we start scrolling
static const float kScrollButtonInterval = 0.15;  // time (in seconds) between firing scroll actions

const int kEscapeKeyCode = 53;

-(id)initWithFrame:(NSRect)frame 
{
  self = [super initWithFrame:frame];
  if (self) {
    mOverflowTabs = NO;
    // this will not likely have any result here
    [self rebuildTabBar];
    [self registerForDraggedTypes:[NSArray arrayWithObjects: kCaminoBookmarkListPBoardType,
                                                             kWebURLsWithTitlesPboardType,
                                                             NSStringPboardType,
                                                             NSFilenamesPboardType,
                                                             NSURLPboardType,
                                                             nil]];
  }
  return self;
}

-(void)awakeFromNib
{
  // The last subview of the bar will have its next key view set as the "next external key view" of
  // the BrowserTabBarView, which was set from within Interface Builder.  The next key view of the
  // BrowserTabBarView itself is changed to instead be the first subview item inside the tab bar.
  mNextExternalKeyView = [self nextKeyView];
  // start off with the tabs hidden, and allow our controller to show or hide as appropriate.
  [self setVisible:NO];
  // this needs to be called again since our tabview should be non-nil now
  [self rebuildTabBar];
  // When background tabs are finished sliding around the dragging one, we need to perform
  // certain actions like updating the divider images.
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(slidingTabAnimationEnded:) 
                                               name:kSlidingTabAnimationFinishedNotification
                                             object:nil];
}

-(void)dealloc
{
  [mTrackingCells release];
  [mOverflowRightButton release];
  [mOverflowLeftButton release];
  [mOverflowMenuButton release];

  [mBackgroundImage release];
  [mButtonDividerImage release];
  [mDraggingTab release];
  [mDraggingTabWindow release];
  [mSavedTabFrames release];
  [mCurrentlySlidingTabs release];

  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [super dealloc];
}

-(void)drawRect:(NSRect)rect 
{
  // Determine the active tab rect, and draw the tab background in the rest of the bar.
  NSRect activeTabRect;
  if ([self tabIsCurrentlyDragging]) {
    activeTabRect = NSZeroRect;
  }
  else {
    TabButtonView* activeTabButton = [(BrowserTabViewItem *)[mTabView selectedTabViewItem] buttonView];
    activeTabRect = [activeTabButton superview] ? [activeTabButton frame]
                                                : NSMakeRect(0, 0, 0, 0);
  }
  [self drawTabBarBackgroundInRect:[self bounds] withActiveTabRect:activeTabRect];

  NSRect tabsRect = [self tabsRect];
  // Draw the leftmost button image divider (right sides are drawn by the buttons themselves).
  // A divider is not needed if the leftmost button is selected.
  TabButtonView* firstTabButton = [(BrowserTabViewItem *)[mTabView tabViewItemAtIndex:mLeftMostVisibleTabIndex] buttonView];
  if ([mTabView indexOfTabViewItem:[mTabView selectedTabViewItem]] != mLeftMostVisibleTabIndex &&
      ([firstTabButton slideAnimationDirection] == eSlideAnimationDirectionNone)) {
    [mButtonDividerImage compositeToPoint:NSMakePoint(NSMinX(tabsRect), 0)
                                operation:NSCompositeSourceOver];
  }

  // Draw a divider to the left of the overflow menu button, if it's showing
  if (mOverflowTabs)
    [mButtonDividerImage compositeToPoint:NSMakePoint(NSMaxX(tabsRect) +
                                                      [mOverflowRightButton frame].size.width, 0)
                                operation:NSCompositeSourceOver];

  if (mDragOverBar)
    [self drawTabBarBackgroundHiliteRectInRect:rect];
}

-(void)setFrame:(NSRect)frameRect
{
  [super setFrame:frameRect];
  // tab buttons probably need to be resized if the frame changes
  [self unregisterTabButtonsForTracking];
  [self layoutButtonsPreservingVisibility:YES];
  [self registerTabButtonsForTracking];
}

-(NSMenu*)menuForEvent:(NSEvent*)theEvent
{
  return [self menu];
}

-(void)mouseDown:(NSEvent*)theEvent
{
  NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
  // If the click fell through a disabled scroll button, ignore it.
  if ([self scrollButtonAtPoint:clickPoint])
    return;

  // We get a double-click even if the first click was actually on a tab's
  // close button, so try to make sure this was really a tab bar double-click.
  NSTimeInterval doubleClickTime = ::GetDblTime() / 60.0;
  NSTimeInterval elapsedTime = [theEvent timestamp] - mLastClickTime;
  int clickCount = [theEvent clickCount];
  // If there was no corresponding first click for a double-click, then we
  // presumably lost a click to a tab close, so treat it as a first click.
  if (clickCount == 2 && elapsedTime > doubleClickTime)
    clickCount = 1;

  if (clickCount == 1) {
    mLastClickTime = [theEvent timestamp];
  }
  else if (elapsedTime < doubleClickTime) {
    mLastClickTime = 0;
    [[NSNotificationCenter defaultCenter] postNotificationName:kTabBarBackgroundDoubleClickedNotification
                                                        object:mTabView];
  }
}

- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
  // We don't want our invisible dragged tab tracking image to indicate anything on screen
  // to show the cursor is dragging something other than the moving tab.
  return NSDragOperationNone;
}

- (void)mouseDragged:(NSEvent*)theEvent
{
  if ([self tabIsCurrentlyDragging])
    return;

  NSPoint clickLocation = [self convertPoint:[theEvent locationInWindow] fromView:nil];
  TabButtonView* activeTabButton = [self buttonAtPoint:clickLocation];

  // When a tab is dragged to the edge, the tab is not allowed to move off the bar, but the mouse
  // keeps moving. We want to prevent moving the tab again until the mouse gets back to the original
  // location on the tab where the drag started.
  NSPoint pointOnDraggingTab = [activeTabButton convertPoint:clickLocation fromView:self];
  mHorizontalGrabOffset = pointOnDraggingTab.x;

  // To have more control over the dragged tab image (e.g. restricting the drag to horiz movement)
  // we actually have Cocoa drag an empty image, and then move a floating window around based upon
  // the NSDraggingSource methods.
  [self dragImage:[[[NSImage alloc] initWithSize:NSMakeSize(1, 1)] autorelease]
               at:clickLocation
           offset:NSZeroSize 
            event:theEvent 
       pasteboard:nil
           source:self 
        slideBack:NO];
}

- (TabButtonView*)tabButtonAtIndex:(int)index
{
  return [(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:index] buttonView];
}

// Returns the scroll button at the specified point, if there is one.
-(NSButton*)scrollButtonAtPoint:(NSPoint)clickPoint
{
  if (NSPointInRect(clickPoint, [mOverflowLeftButton frame]))
    return mOverflowLeftButton;
  if (NSPointInRect(clickPoint, [mOverflowRightButton frame]))
    return mOverflowRightButton;
  if (NSPointInRect(clickPoint, [mOverflowMenuButton frame]))
    return mOverflowMenuButton;
  return nil;
}

// returns the tab at the specified point (in tab bar view coordinates)
-(TabButtonView*)buttonAtPoint:(NSPoint)clickPoint
{
  BrowserTabViewItem *tab = nil;
  NSArray *tabItems = [mTabView tabViewItems];
  NSEnumerator *tabEnumerator = [tabItems objectEnumerator];
  while ((tab = [tabEnumerator nextObject])) {
    TabButtonView *button = [tab buttonView];
    if ([button superview] && NSPointInRect(clickPoint, [button frame]))
      return button;
  }
  return nil;
}

-(void)drawTabBarBackgroundInRect:(NSRect)rect withActiveTabRect:(NSRect)tabRect
{
  // draw tab bar background, omitting the selected Tab
  NSRect barFrame = [self bounds];
  NSPoint patternOrigin = [self convertPoint:NSMakePoint(0.0f, 0.0f) toView:[[self window] contentView]];
  NSRect fillRect;

  // first, fill to the left of the active tab
  fillRect = NSMakeRect(barFrame.origin.x, barFrame.origin.y, 
                        (tabRect.origin.x - barFrame.origin.x), barFrame.size.height);
  if (NSIntersectsRect(fillRect, rect)) {
    // make sure we're not drawing to the left or right of the actual rectangle we were asked to draw
    if (fillRect.origin.x < NSMinX(rect)) {
      fillRect.size.width -= NSMinX(rect) - fillRect.origin.x;
      fillRect.origin.x = NSMinX(rect);
    }

    if (NSMaxX(fillRect) > NSMaxX(rect))
      fillRect.size.width -= NSMaxX(fillRect) - NSMaxX(rect);

    [mBackgroundImage drawTiledInRect:fillRect origin:patternOrigin operation:NSCompositeSourceOver];
  }

  // then fill to the right
  fillRect = NSMakeRect(NSMaxX(tabRect), barFrame.origin.y, 
                        (NSMaxX(barFrame) - NSMaxX(tabRect)), barFrame.size.height);
  if (NSIntersectsRect(fillRect,rect)) {
      // make sure we're not drawing to the left or right of the actual rectangle we were asked to draw
      if (fillRect.origin.x < NSMinX(rect)) {
        fillRect.size.width -= NSMinX(rect) - fillRect.origin.x;
        fillRect.origin.x = NSMinX(rect);
      }

      if (NSMaxX(fillRect) > NSMaxX(rect))
        fillRect.size.width -= NSMaxX(fillRect) - NSMaxX(rect);
        
      [mBackgroundImage drawTiledInRect:fillRect origin:patternOrigin operation:NSCompositeSourceOver];
   }
}

-(void)drawDragHiliteInRect:(NSRect)rect
{
  NSRect fillRect;
  NSRect junk;
  NSDivideRect(rect, &junk, &fillRect, kTabBottomPad, NSMinYEdge);

  NSGraphicsContext* gc = [NSGraphicsContext currentContext];
  [gc saveGraphicsState];
  [[[NSColor colorForControlTint:NSDefaultControlTint] colorWithAlphaComponent:0.3] set];
  NSRectFillUsingOperation(fillRect, NSCompositeSourceOver);
  [gc restoreGraphicsState];
}


-(void)drawTabBarBackgroundHiliteRectInRect:(NSRect)rect
{
  NSRect barBounds = [self bounds];

  BrowserTabViewItem* thisTab        = [[mTabView tabViewItems] firstObject];
  TabButtonView*      tabButton      = [thisTab buttonView];
  NSRect              tabButtonFrame = [tabButton frame];

  NSRect junk;
  NSRect backgroundRect;
  NSDivideRect(barBounds, &backgroundRect, &junk, NSMinX(tabButtonFrame), NSMinXEdge);
  if (NSIntersectsRect(backgroundRect, rect))
    [self drawDragHiliteInRect:backgroundRect];

  thisTab         = [[mTabView tabViewItems] lastObject];
  tabButton       = [thisTab buttonView];
  tabButtonFrame  = [tabButton frame];

  NSDivideRect(barBounds, &junk, &backgroundRect, NSMaxX(tabButtonFrame), NSMinXEdge);
  if (!NSIsEmptyRect(backgroundRect) && NSIntersectsRect(backgroundRect, rect))
    [self drawDragHiliteInRect:backgroundRect];
}

-(void)loadImages
{
  if (!mBackgroundImage)
    mBackgroundImage = [[NSImage imageNamed:@"tab_bar_bg"] retain];
  if (!mButtonDividerImage)
    mButtonDividerImage = [[NSImage imageNamed:@"tab_button_divider"] retain];
}

// construct the tab bar based on the current state of mTabView;
// should be called when tabs are first shown.
-(void)rebuildTabBar
{
  if ([self tabIsCurrentlyDragging])
    return;

  [self loadImages];

  [self unregisterTabButtonsForTracking];
  [self layoutButtonsPreservingVisibility:NO];
  [self registerTabButtonsForTracking];
  [self updateKeyViewLoop];
}

- (void)viewWillMoveToWindow:(NSWindow*)window
{
  [self unregisterTabButtonsForTracking];
}

// allows tab button cells to react to mouse events
-(void)registerTabButtonsForTracking
{
  if ([self window] && ![self isHidden]) {
    NSArray* tabItems = [mTabView tabViewItems];
    if (mTrackingCells)
      [self unregisterTabButtonsForTracking];
    mTrackingCells = [[NSMutableArray alloc] initWithCapacity:[tabItems count]];
    NSEnumerator* tabEnumerator = [tabItems objectEnumerator];
    
    BrowserTabViewItem* tab = nil;
    while ((tab = [tabEnumerator nextObject])) {
      TabButtonView* tabButton = [tab buttonView];
      if (tabButton) {
        // only track tabs that are visible
        if (![tabButton superview])
          continue;

        [mTrackingCells addObject:tabButton];
        NSRect trackingRect = [tabButton frame];
        // TODO: now that tab are views, they should probably update their
        // own tracking rects as necessary instead.
        [tabButton addTrackingRect];
      }
    }
  }
}

// causes tab buttons to stop reacting to mouse events
-(void)unregisterTabButtonsForTracking
{
  if (mTrackingCells) {
    NSEnumerator *tabEnumerator = [mTrackingCells objectEnumerator];
    TabButtonView *tab = nil;
    while ((tab = (TabButtonView*)[tabEnumerator nextObject]))
      [tab removeTrackingRect];
    [mTrackingCells release];
    mTrackingCells = nil;
  }
}

- (void)viewDidMoveToWindow
{
  // setup the tab bar to recieve notifications of key window changes
  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
  [nc removeObserver:self name:NSWindowDidBecomeKeyNotification object:[self window]];
  [nc removeObserver:self name:NSWindowDidResignKeyNotification object:[self window]];
  if ([self window]) {
    [nc addObserver:self selector:@selector(handleWindowIsKey:)
          name:NSWindowDidBecomeKeyNotification object:[self window]];
    [nc addObserver:self selector:@selector(handleWindowResignKey:)
          name:NSWindowDidResignKeyNotification object:[self window]];
  }
}

- (void)handleWindowIsKey:(NSWindow *)inWindow
{
  // the mouse isn't tracked when the window isn't key, so update the tab hover
  // state manually if the mouse is in a tab
  BrowserTabViewItem *tab = [self tabViewItemUnderMouse];
  if (tab)
    [[tab buttonView] updateHoverState:YES];
}

- (void)handleWindowResignKey:(NSWindow *)inWindow
{
  // the mouse isn't tracked when the window isn't key, so update the tab hover
  // state manually if the mouse is in a tab
  BrowserTabViewItem *tab = [self tabViewItemUnderMouse];
  if (tab)
    [[tab buttonView] updateHoverState:NO];
}
  
// returns the height the tab bar should be if drawn
-(float)tabBarHeight
{
  // this will be constant for now
  return kTabBarDefaultHeight;
}

// finds the tab under the given point (in window coordinates), if any
-(BrowserTabViewItem *)tabViewItemAtPoint:(NSPoint)location
{
  NSPoint mousePointInView = [self convertPoint:location fromView:nil];
  // Don't bother checking each tab's frame if the point isn't in the tab bar
  if (!NSMouseInRect(mousePointInView, [self bounds], NO))
    return nil;
  return [[self buttonAtPoint:mousePointInView] tabViewItem];
}

// finds the tab currently under the mouse, if any
-(BrowserTabViewItem *)tabViewItemUnderMouse
{
  NSPoint mousePointInWindow = [[self window] convertScreenToBase:[NSEvent mouseLocation]];
  return [self tabViewItemAtPoint:mousePointInWindow];
}

// Computes sizes and positions of the currently visible tabs.
// If |preserveVisibility| is YES, then the currently selected tab is forced to
// remain visible in the new layout if it was previously. If it is NO, then the
// tab may or may not stay visible in the new layout.
-(void)layoutButtonsPreservingVisibility:(BOOL)preserveVisibility
{
  // before changing anything, get information about the current state
  BrowserTabViewItem* selectedTab = (BrowserTabViewItem*)[mTabView selectedTabViewItem];
  int selectedTabIndex = selectedTab ? [mTabView indexOfTabViewItem:selectedTab]
                                     : -1;
  // if we aren't currently overflowing, or we were asked to preserve the
  // visibility of the current tab, make sure the current tab stays visible.
  BOOL keepCurrentTabVisible = !mOverflowTabs ||
                               (preserveVisibility && [self tabIndexIsVisible:selectedTabIndex]);

  int numberOfTabs = [mTabView numberOfTabViewItems];

  // check to see whether or not the tabs will fit without the overflows
  float widthOfATab = floor(NSWidth([self tabsRectWithOverflow:NO]) / numberOfTabs);
  mOverflowTabs = widthOfATab < kMinTabWidth;

  if (mOverflowTabs) {
    float widthOfTabBar = NSWidth([self tabsRect]);
    mNumberOfVisibleTabs = (int)floor(widthOfTabBar / kMinTabWidth);
    widthOfATab = floor(widthOfTabBar / mNumberOfVisibleTabs);
    if (mNumberOfVisibleTabs + mLeftMostVisibleTabIndex > numberOfTabs)
      [self setLeftmostVisibleTabIndex:(numberOfTabs - mNumberOfVisibleTabs)];
    if (keepCurrentTabVisible && selectedTab)
      [self scrollTabIndexToVisible:selectedTabIndex];
  }
  else {
    mLeftMostVisibleTabIndex = 0;
    mNumberOfVisibleTabs = numberOfTabs;
    widthOfATab = (widthOfATab > kMaxTabWidth ? kMaxTabWidth : widthOfATab);
  }

  [self removeAllSubviews];
  [self setOverflowButtonsVisible:mOverflowTabs];

  NSRect tabsRect = [self tabsRect];
  float extraWidth = 0.0;
  if (widthOfATab < kMaxTabWidth)
    extraWidth = NSWidth(tabsRect) - widthOfATab * mNumberOfVisibleTabs;
  float nextTabXOrigin  = NSMinX(tabsRect);
  for (int i = 0; i < numberOfTabs; i++) {
    TabButtonView* tabButton = [(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:i] buttonView];

    // Don't do anything with offscreen tabs
    if (i < mLeftMostVisibleTabIndex || i >= mLeftMostVisibleTabIndex + mNumberOfVisibleTabs)
      continue;

    [self addSubview:tabButton];
    NSRect tabRect = NSMakeRect(nextTabXOrigin, 0, widthOfATab, [self tabBarHeight]);
    // spread the extra width from rounding tab sizes over the leftmost tabs.
    if (extraWidth > 0.5) {
      extraWidth -= 1.0;
      tabRect.size.width += 1.0;
    }
    [tabButton setFrame:tabRect];
    [tabButton setDrawsLeftDivider:NO];
    [tabButton setDrawsRightDivider:YES];
    nextTabXOrigin += NSWidth(tabRect);
  }

  if (selectedTab) {
    [[selectedTab buttonView] setDrawsRightDivider:NO];
    if (selectedTabIndex >= 1 && [self tabIndexIsVisible:selectedTabIndex])
      [[(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:(selectedTabIndex - 1)] buttonView] setDrawsRightDivider:NO];
  }

  [self setNeedsDisplay:YES];
}

// Determines whether or not the specified tab index is in the currently visible
// tab bar.
-(BOOL)tabIndexIsVisible:(int)tabIndex
{
  return (mLeftMostVisibleTabIndex <= tabIndex && tabIndex < mNumberOfVisibleTabs + mLeftMostVisibleTabIndex);
}

// A helper method that returns an NSButton ready for use as one of our overflow buttons
-(NSButton*)newOverflowButtonForImageNamed:(NSString*)imageName
{
  NSImage* buttonImage = [NSImage imageNamed:imageName];
  NSButton* button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, [buttonImage size].width, [buttonImage size].height)];
  [button setImage:buttonImage];
  [button setImagePosition:NSImageOnly];
  [button setBezelStyle:NSShadowlessSquareBezelStyle];
  [button setButtonType:NSToggleButton];
  [button setBordered:NO];
  [button setTarget:self];
  return button;
}

-(void)ensureOverflowButtonsInitted
{
  if (!mOverflowLeftButton) {
    mOverflowLeftButton = [self newOverflowButtonForImageNamed:@"tab_scroll_button_left"];
    [[mOverflowLeftButton cell] setContinuous:YES];
    [mOverflowLeftButton setPeriodicDelay:kScrollButtonDelay interval:kScrollButtonInterval];
    [mOverflowLeftButton setAction:@selector(scrollLeft:)];
  }
  if (!mOverflowRightButton) {
    mOverflowRightButton = [self newOverflowButtonForImageNamed:@"tab_scroll_button_right"];
    [[mOverflowRightButton cell] setContinuous:YES];
    [mOverflowRightButton setPeriodicDelay:kScrollButtonDelay interval:kScrollButtonInterval];
    [mOverflowRightButton setAction:@selector(scrollRight:)];
  }
  if (!mOverflowMenuButton) {
    mOverflowMenuButton = [self newOverflowButtonForImageNamed:@"tab_menu_button"];
    [mOverflowMenuButton setAction:@selector(showOverflowMenu:)];
    [mOverflowMenuButton sendActionOn:NSLeftMouseDownMask];
  }
}

-(void)setOverflowButtonsVisible:(BOOL)visible
{
  if (visible) {
    [self ensureOverflowButtonsInitted];

    NSRect rect = [self tabsRect];

    [mOverflowLeftButton setFrameOrigin:NSMakePoint(0, kTabBottomPad)];
    [mOverflowLeftButton setEnabled:(mLeftMostVisibleTabIndex != 0)];
    [self addSubview:mOverflowLeftButton];

    [mOverflowRightButton setFrameOrigin:NSMakePoint(NSMaxX(rect), kTabBottomPad)];
    [mOverflowRightButton setEnabled:(mLeftMostVisibleTabIndex + mNumberOfVisibleTabs != [mTabView numberOfTabViewItems])];
    [self addSubview:mOverflowRightButton];

    [mOverflowMenuButton setFrameOrigin:NSMakePoint(NSMaxX(rect) +
                                                    [mOverflowRightButton frame].size.width +
                                                    [mButtonDividerImage size].width,
                                                    kTabBottomPad)];
    [self addSubview:mOverflowMenuButton];
  }
  else {
    [mOverflowLeftButton removeFromSuperview];
    [mOverflowRightButton removeFromSuperview];
    [mOverflowMenuButton removeFromSuperview];
  }
}

- (void)showOverflowMenu:(id)sender
{
  NSMenu* overflowMenu = [[[NSMenu alloc] init] autorelease];
  int numberOfTabs = [mTabView numberOfTabViewItems];

  for (int i = 0; i < numberOfTabs; i++)
    [overflowMenu addItem:[(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:i] menuItem]];

  // Insert the separators from right to left, so we don't mess up the index numbers as we go
  if (mLeftMostVisibleTabIndex + mNumberOfVisibleTabs < numberOfTabs)
    [overflowMenu insertItem:[NSMenuItem separatorItem] atIndex:(mLeftMostVisibleTabIndex + mNumberOfVisibleTabs)];
  if (mLeftMostVisibleTabIndex > 0)
    [overflowMenu insertItem:[NSMenuItem separatorItem] atIndex:mLeftMostVisibleTabIndex];

  NSPopUpButtonCell* popupCell = [[[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:NO] autorelease];
  [popupCell setAltersStateOfSelectedItem:YES];
  [popupCell setMenu:overflowMenu];
  [popupCell trackMouse:[NSApp currentEvent] inRect:[sender bounds] ofView:sender untilMouseUp:YES];
}

-(void)scrollWheel:(NSEvent*)theEvent {
  // Treat vertical scrolling as horizontal (with down == right), since there's
  // no other meaning for the tab bar, and many mice are vertical-only.
  float scrollIncrement = 0.0;
  if ([theEvent deltaX])
    scrollIncrement = -[theEvent deltaX];
  else if ([theEvent deltaY])
    scrollIncrement = -[theEvent deltaY];

  // We don't use the accellation; just scroll one tab per event.
  if (scrollIncrement > 0.0)
    [self scrollRight:nil];
  else if (scrollIncrement < 0.0)
    [self scrollLeft:nil];
}

-(void)scrollLeft:(id)aSender
{
  int numberOfTabsToScroll = 1;
  // We can safely scroll up to the number of tabs hidden to the left
  int tabsHiddenToTheLeft = mLeftMostVisibleTabIndex;

  // If option's down and we're being called from a button-click
  if (([[NSApp currentEvent] modifierFlags] & NSAlternateKeyMask) && [aSender isKindOfClass:[NSButton class]])
    // Scroll up to a window's width (if possible)
    numberOfTabsToScroll = MIN(tabsHiddenToTheLeft, mNumberOfVisibleTabs);

  if (tabsHiddenToTheLeft > 0)
    [self setLeftmostVisibleTabIndex:(mLeftMostVisibleTabIndex - numberOfTabsToScroll)];
}

-(void)scrollRight:(id)aSender
{
  int numberOfTabsToScroll = 1;
  // We can safely scroll up to the the number of tabs hidden to the right
  int tabsHiddenToTheRight = [mTabView numberOfTabViewItems] - (mLeftMostVisibleTabIndex + mNumberOfVisibleTabs);
  
  // If option's down and we're being called from a button-click
  if (([[NSApp currentEvent] modifierFlags] & NSAlternateKeyMask) && [aSender isKindOfClass:[NSButton class]])
    // Scroll up to a window's width (if possible)
    numberOfTabsToScroll = MIN(tabsHiddenToTheRight, mNumberOfVisibleTabs);

  if (tabsHiddenToTheRight > 0)
    [self setLeftmostVisibleTabIndex:(mLeftMostVisibleTabIndex + numberOfTabsToScroll)];
}

// Scrolls the tab bar to make index visible
-(void)scrollTabIndexToVisible:(int)index
{
  // if it's to the left of screen, make it leftmost
  if (index < mLeftMostVisibleTabIndex)
    [self setLeftmostVisibleTabIndex:index];
  // if it's to the right of screen, make it rightmost
  else if (index >= mLeftMostVisibleTabIndex + mNumberOfVisibleTabs)
    [self setLeftmostVisibleTabIndex:(index - mNumberOfVisibleTabs + 1)];
}

// Sets the left most visible tab index to the value specified and rebuilds the 
// tab bar. Should not be called before performing necessary sanity checks.
-(void)setLeftmostVisibleTabIndex:(int)index
{
  if (index != mLeftMostVisibleTabIndex) {
    mLeftMostVisibleTabIndex = index;
    [self rebuildTabBar];
  }
}

// returns an NSRect of the area where tabs may currently be drawn
- (NSRect)tabsRect
{
  return [self tabsRectWithOverflow:mOverflowTabs];
}

// returns an NSRect of the available area to draw tabs with or without overflowing
-(NSRect)tabsRectWithOverflow:(BOOL)overflowing
{
  NSRect rect = [self frame];

  if (overflowing) {
    // Makes sure the buttons exist before getting their frame information.
    [self ensureOverflowButtonsInitted];

    float overflowLeftButtonWidth = [mOverflowLeftButton frame].size.width;
    rect.origin.x += overflowLeftButtonWidth;
    rect.size.width -= overflowLeftButtonWidth +
                       [mOverflowRightButton frame].size.width +
                       [mButtonDividerImage size].width +
                       [mOverflowMenuButton frame].size.width;
  }
  // If there aren't overflows, give ourselves a little margin around the tabs
  // to make them look nicer.
  else {
    rect.origin.x += kTabBarMargin;
    rect.size.width -= 2 * kTabBarMargin;
  }

  return rect;
}

-(BOOL)isVisible
{
  return ![self isHidden];
}

// show or hide tabs- should be called if this view will be hidden, to give it a chance to register or
// unregister tracking rects as appropriate.
//
// Does not actually remove the view from the hierarchy; simply hides it.
-(void)setVisible:(BOOL)show
{
  if ([self tabIsCurrentlyDragging])
    return;

  // only change anything if the new state is different from the current state
  if (show && [self isHidden]) {
    [self setHidden:NO];
    [self rebuildTabBar];
    // set up tracking rects
    [self registerTabButtonsForTracking];
  } else if (!show && ![self isHidden]) { // being hidden
    [self setHidden:YES];
    // destroy tracking rects
    [self unregisterTabButtonsForTracking];
  }
}    

- (int)rightmostVisibleTabIndex
{
  return mLeftMostVisibleTabIndex + mNumberOfVisibleTabs - 1;
}

- (void)updateKeyViewLoop
{
  // Connects the key view loop among all subviews of the BrowserTabBarView.

  int numberOfTabs = [mTabView numberOfTabViewItems];

  if (numberOfTabs <= 0)
    return;

  TabButtonView* firstTabButton = [(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:0] buttonView];
  // If we don't have a tab button yet, just keep the current nextKeyView.
  if (!firstTabButton)
    return;  

  // Set the next key view of the BrowserTabBarView itself to the close button of the first tab.
  // If there are overflow buttons, insert them before the first close button. 
  if (mOverflowTabs) {
    [self setNextKeyView:mOverflowLeftButton];
    [mOverflowLeftButton setNextKeyView:[firstTabButton closeButton]];
  }
  else {
    [self setNextKeyView:[firstTabButton closeButton]];
  }

  // Other than the last tab button, set the nextKeyView of each to the following tab's close
  // button.  Make the tab itself the next key view of its own close button.
  for (int currentButtonIndex = 0; currentButtonIndex < (numberOfTabs - 1); currentButtonIndex++) {
    TabButtonView* currentTabButton = 
    [(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:currentButtonIndex] buttonView];
    TabButtonView* nextTabButton = 
    [(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:(currentButtonIndex + 1)] buttonView];
    
    [[currentTabButton closeButton] setNextKeyView:currentTabButton];
    [currentTabButton setNextKeyView:[nextTabButton closeButton]];
  }

  // For the last tab, account for the right overflow items and set the next key view of
  // the last tab bar item to the next external key view of the tab bar itself.
  TabButtonView* lastTabButton = 
  [(BrowserTabViewItem*)[mTabView tabViewItemAtIndex:(numberOfTabs - 1)] buttonView];
  [[lastTabButton closeButton] setNextKeyView:lastTabButton];
  if (mOverflowTabs) {
    [lastTabButton setNextKeyView:mOverflowRightButton];
    [mOverflowRightButton setNextKeyView:mOverflowMenuButton];
    [mOverflowMenuButton setNextKeyView:mNextExternalKeyView];
  }
  else {
    [lastTabButton setNextKeyView:mNextExternalKeyView];
  }
}

- (void)keyDown:(NSEvent *)theEvent
{
  // If the spacebar key was pressed with either the left or right overflow buttons highlighted,
  // restore focus on them after the key event is processed.  (They will normally lose
  // focus when carrying out their action, and it makes sense to leave it on them for
  // further action).
  [self ensureOverflowButtonsInitted];
  NSResponder *firstResponder = [[self window] firstResponder];
  [super keyDown:theEvent];
  if ([[theEvent characters] isEqualToString:@" "] &&
      (firstResponder == mOverflowLeftButton ||
       firstResponder == mOverflowRightButton))
  {
    if ([firstResponder acceptsFirstResponder])
      [[self window] makeFirstResponder:firstResponder];
  }
}

#pragma mark -

// NSDraggingDestination destination methods
-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
  if (![mTabView shouldAcceptDrag:sender])
    return NSDragOperationNone;

  mDragOverBar = YES;
  [self setNeedsDisplay:YES];

  if ([sender draggingSourceOperationMask] & NSDragOperationCopy)
    return NSDragOperationCopy;

  return NSDragOperationGeneric;
}

-(void)draggingExited:(id <NSDraggingInfo>)sender
{
  mDragOverBar = NO;
  [self setNeedsDisplay:YES];
}

-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
  return YES;
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
  mDragOverBar = NO;
  [self setNeedsDisplay:YES];

  return [mTabView handleDrop:sender onTab:nil];
}

# pragma mark -

- (void)draggedImage:(NSImage *)anImage beganAt:(NSPoint)aPoint
{
  [mDraggingTab release];
  mDraggingTab = [[(BrowserTabViewItem *)[mTabView selectedTabViewItem] buttonView] retain];
  int indexOfDraggingTab = [mTabView indexOfTabViewItem:[mTabView selectedTabViewItem]];

  // Represent the dragged tab with a floating borderless child window.
  [mDraggingTabWindow release];
  mDraggingTabWindow = [self newFloatingWindowForDraggedTab];
  [[self window] addChildWindow:mDraggingTabWindow ordered:NSWindowAbove];

  mTabIsCurrentlyDragging = YES;
  mEmptyTabPlaceholderIndex = indexOfDraggingTab;
  mDraggingTabOriginalIndex = indexOfDraggingTab;

  if (!mCurrentlySlidingTabs)
    mCurrentlySlidingTabs = [[NSMutableArray alloc] init];
  else
    [mCurrentlySlidingTabs removeAllObjects];

  [mDraggingTab removeFromSuperview];

  // When the drag begins, set the dividers on surrounding tabs we need immediately.
  if (indexOfDraggingTab > 0)
    [[self tabButtonAtIndex:(indexOfDraggingTab - 1)] setDrawsRightDivider:YES];
  if (indexOfDraggingTab < ([self rightmostVisibleTabIndex]))
    [[self tabButtonAtIndex:(indexOfDraggingTab + 1)] setDrawsLeftDivider:YES];

  [self saveTabButtonFramesForDragging];
  [self unregisterTabButtonsForTracking];
}

// Create a borderless floating window, with an image of the dragged tab as its content view
// to move around on screen representing the dragged tab.
// The caller owns the reference to the returned window.
- (NSWindow*)newFloatingWindowForDraggedTab;
{
  NSRect draggingTabButtonFrame = [mDraggingTab frame];

  // Create an image representation of the dragged tab.
  [self lockFocus];
  NSBitmapImageRep* tabImageRep = 
    [[[NSBitmapImageRep alloc] initWithFocusedViewRect:draggingTabButtonFrame] autorelease];
  [self unlockFocus];
  NSImage* draggedTabImage = [[[NSImage alloc] initWithSize:[tabImageRep size]] autorelease];
  [draggedTabImage addRepresentation:tabImageRep];

  // Convert the tab's frame to screen coordinates.
  NSPoint tabOriginInWindowCoords = [self convertPoint:draggingTabButtonFrame.origin toView:nil];
  NSPoint tabOriginInScreenCoords = [[self window] convertBaseToScreen:tabOriginInWindowCoords];
  NSRect draggedTabWindowFrame = {tabOriginInScreenCoords, draggingTabButtonFrame.size};

  NSWindow* draggedTabWindow = [[NSWindow alloc] initWithContentRect:draggedTabWindowFrame
                                                   styleMask:NSBorderlessWindowMask
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];

  // Embed an image view in the window, containing the image of the dragging tab.
  NSRect imageViewFrame = {NSZeroPoint, draggingTabButtonFrame.size};
  NSImageView* imageView = [[[NSImageView alloc] initWithFrame:imageViewFrame] autorelease];
  [imageView setImage:draggedTabImage];

  [draggedTabWindow setContentView:imageView];

  return draggedTabWindow;
}

- (void)draggedImage:(NSImage *)draggedImage movedTo:(NSPoint)screenPoint
{
  // The dragged tab is represented by a floating window; move the window
  // to reflect the new location of the dragged image.

  // If the dragging tab closed, the invisible drag image is still moving around since
  // there's no way to force AppKit to end a drag.
  if (!mTabIsCurrentlyDragging)
    return;

  float deltaX = [[NSApp currentEvent] deltaX];
  if (deltaX == 0.0)
    return;

  NSPoint currentTabWindowLocation = [mDraggingTabWindow frame].origin;

  NSRect tabsRect = [self tabsRect];
  NSPoint leftmostTabBarLocationInWindowCoords = [self convertPoint:tabsRect.origin toView:nil];
  NSPoint rightmostTabBarLocationInWindowCoords = [self convertPoint:NSMakePoint(NSMaxX(tabsRect), NSMaxY(tabsRect)) toView:nil];

  // Since we're manipulating the dragged tab window's origin, retrieve the tab bar boundaries
  // in screen coordinates.
  float minAllowedTabXLocation = [[self window] convertBaseToScreen:leftmostTabBarLocationInWindowCoords].x;
  float maxAllowedTabXLocation = [[self window] convertBaseToScreen:rightmostTabBarLocationInWindowCoords].x;
  maxAllowedTabXLocation -= [mDraggingTabWindow frame].size.width;

  // If the dragged tab is at either end of tab bar, and the mouse kept moving far away from
  // where the tab was stopped, wait to drag again until the mouse is under the original location
  // in the tab where the drag started.  
  if ((currentTabWindowLocation.x == minAllowedTabXLocation &&
       screenPoint.x < (minAllowedTabXLocation + mHorizontalGrabOffset)) ||
      (currentTabWindowLocation.x == maxAllowedTabXLocation &&
       screenPoint.x > (maxAllowedTabXLocation + mHorizontalGrabOffset)))
  {
    return;
  }

  // Calculate the dragged tab window's new position.
  NSPoint newDragggedTabLocation = NSMakePoint(currentTabWindowLocation.x + deltaX, currentTabWindowLocation.y);

  // Ensure the tab is not dragged out of the tab bar's boundaries.
  if (newDragggedTabLocation.x <= minAllowedTabXLocation)
    newDragggedTabLocation.x = minAllowedTabXLocation;
  else if (newDragggedTabLocation.x > maxAllowedTabXLocation)
    newDragggedTabLocation.x = maxAllowedTabXLocation;
  [mDraggingTabWindow setFrameOrigin:newDragggedTabLocation];

  [self slideTabButtonsAroundCurrentDragPosition];
}

- (void)slideTabButtonsAroundCurrentDragPosition
{
  // Convert the dragged tab window's origin to the tab bar's coordinate system.
  NSPoint draggedTabWindowOrigin = [mDraggingTabWindow frame].origin;
  draggedTabWindowOrigin = [[self window] convertScreenToBase:draggedTabWindowOrigin];
  draggedTabWindowOrigin = [self convertPoint:draggedTabWindowOrigin fromView:nil];
  NSRect draggedTabFrameInTabBarCoords = {draggedTabWindowOrigin, [mDraggingTabWindow frame].size};

  // Determine which tab button is under the middle of the dragged tab.
  NSPoint midDraggedTab = NSMakePoint(NSMidX(draggedTabFrameInTabBarCoords), NSMidY(draggedTabFrameInTabBarCoords));
  TabButtonView* tabButtonUnderDrag = [self buttonAtPoint:midDraggedTab];

  // If there's nothing under the drag, we have to first make sure we're not on the right
  // after all of the tabs.  Otherwise, we're under the empty spot.
  TabButtonView* lastVisibleTabButton = [self tabButtonAtIndex:[self rightmostVisibleTabIndex]];
  if (!tabButtonUnderDrag && NSMinX(draggedTabFrameInTabBarCoords) > NSMaxX([lastVisibleTabButton frame]))
    tabButtonUnderDrag = lastVisibleTabButton;
  else if (!tabButtonUnderDrag)
    return;

  BOOL draggedTabMovingRight = ([[NSApp currentEvent] deltaX] > 0);

  // Check if the tab we're under is already moving in the correct direction.
  ESlideAnimationDirection tabSlideDirection = [tabButtonUnderDrag slideAnimationDirection];
  if (draggedTabMovingRight && tabSlideDirection == eSlideAnimationDirectionLeft)
    return;
  else if (!draggedTabMovingRight && tabSlideDirection == eSlideAnimationDirectionRight)
    return;

  int indexOfTabUnderDrag = [mTabView indexOfTabViewItem:[tabButtonUnderDrag tabViewItem]];

  if (indexOfTabUnderDrag == mEmptyTabPlaceholderIndex)
    return;

  // Find all tabs between the one we're over and the available spot and slide them out of the way.
  int currentTabIndex = indexOfTabUnderDrag;
  while ((draggedTabMovingRight && currentTabIndex > mEmptyTabPlaceholderIndex) || 
         (!draggedTabMovingRight && currentTabIndex < mEmptyTabPlaceholderIndex))
  {
    TabButtonView* tabButton = [self tabButtonAtIndex:currentTabIndex];

    // If we're moving to the right, tell the tab to our right to draw its divider.
    if (draggedTabMovingRight && (currentTabIndex < [self rightmostVisibleTabIndex])) {
      TabButtonView* nextTabButton = [self tabButtonAtIndex:(currentTabIndex + 1)];
      [nextTabButton setDrawsLeftDivider:YES];
    }

    // Determine where this tab will move to.
    int destinationTabIndex = draggedTabMovingRight ? currentTabIndex - 1 : currentTabIndex + 1;
    NSPoint newTabLocation = [[mSavedTabFrames objectAtIndex:destinationTabIndex] rectValue].origin;

    [mCurrentlySlidingTabs addObject:tabButton];

    [tabButton slideToLocation:newTabLocation];

    if (draggedTabMovingRight)
      currentTabIndex--;
    else
      currentTabIndex++;
  }

  mEmptyTabPlaceholderIndex = indexOfTabUnderDrag;

  // Update the BrowserTabView to reflect where the dragged image currently will be dropped.
  BrowserTabViewItem* draggedTabViewItem = [[[mDraggingTab tabViewItem] retain] autorelease];
  [self setReorderingTabViewItemsDuringDrag:YES];
  [mTabView removeTabViewItem:draggedTabViewItem];
  [mTabView insertTabViewItem:draggedTabViewItem atIndex:mEmptyTabPlaceholderIndex];
  [mTabView selectTabViewItem:draggedTabViewItem];
  [self setReorderingTabViewItemsDuringDrag:NO];
}

- (void)draggedImage:(NSImage *)anImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)operation
{
  // If the dragged tab closed during the drag, it has already been removed and cleaned up.
  if (![self tabIsCurrentlyDragging])
    return;

  // Calculate the destination frame in Tab Bar coords, then convert it to screen coords
  // to position the dragged tab window.
  NSRect draggedTabDestinationFrame = NSZeroRect;

  // If the drag ended by the escape key being pressed, make the tab go back to the index
  // where it originally started dragging from instead.
  NSEvent *currentEvent = [NSApp currentEvent];
  if ([currentEvent type] == NSKeyDown && [currentEvent keyCode] == kEscapeKeyCode) {
    draggedTabDestinationFrame = [[mSavedTabFrames objectAtIndex:mDraggingTabOriginalIndex] rectValue];
    BrowserTabViewItem* draggedTabViewItem = [[[mDraggingTab tabViewItem] retain] autorelease];
    [self setReorderingTabViewItemsDuringDrag:YES];
    [mTabView removeTabViewItem:draggedTabViewItem];
    [mTabView insertTabViewItem:draggedTabViewItem atIndex:mDraggingTabOriginalIndex];
    [mTabView selectTabViewItem:draggedTabViewItem];
    [self setReorderingTabViewItemsDuringDrag:NO];
  }
  else {
    draggedTabDestinationFrame = [[mSavedTabFrames objectAtIndex:mEmptyTabPlaceholderIndex] rectValue];
  }

  NSPoint destOriginInWindowCoords = [self convertPoint:draggedTabDestinationFrame.origin
                                                 toView:nil];
  NSPoint destOriginInScreenCoords = [[self window] convertBaseToScreen:destOriginInWindowCoords];
  draggedTabDestinationFrame.origin = destOriginInScreenCoords;

  // Animate the dragged tab into its proper position.
  CHSlidingViewAnimation *slidingWindowAnimation = [[CHSlidingViewAnimation alloc] initWithAnimationTarget:mDraggingTabWindow];
  [slidingWindowAnimation setDuration:0.2];
  [slidingWindowAnimation setFrameRate:0.0];
  [slidingWindowAnimation setAnimationBlockingMode:NSAnimationNonblocking];
  [slidingWindowAnimation setAnimationCurve:NSAnimationEaseInOut];
  [slidingWindowAnimation setDelegate:self];
  [slidingWindowAnimation setStartLocation:[mDraggingTabWindow frame].origin];
  [slidingWindowAnimation setEndLocation:draggedTabDestinationFrame.origin];
  [slidingWindowAnimation startAnimation];
  // The animation retains itself, will perform a release when the animation is finished.
  [slidingWindowAnimation release];
}

// Called when a background tab has finished sliding out of the way of the
// dragging tab.
- (void)slidingTabAnimationEnded:(NSNotification*)aNotification
{
  TabButtonView* animatedTab = (TabButtonView*)[aNotification object];
  int animatedTabIndex = [mTabView indexOfTabViewItem:[animatedTab tabViewItem]];

  [mCurrentlySlidingTabs removeObjectIdenticalTo:animatedTab];

  // Update the tab divider images.

  // If there is a tab to the right of the animated one and we finished animating (meaning we sit 
  // right next to it), take away its left divider.
  if (animatedTabIndex < [self rightmostVisibleTabIndex] &&
      [[[aNotification userInfo] valueForKey:kSlidingTabAnimationFinishedCompletelyKey] boolValue] == YES) {
    TabButtonView* nextTabButton = [self tabButtonAtIndex:(animatedTabIndex + 1)];
    [nextTabButton setDrawsLeftDivider:NO];
  }

  // Draw a left divider if this tab is to the right of the available tab spot.
  // (The drag may have ended before this tab's animation did, so also ensure there
  // is a tab dragging.)
  if ([self tabIsCurrentlyDragging] && animatedTabIndex == (mEmptyTabPlaceholderIndex + 1))
    [animatedTab setDrawsLeftDivider:YES];
  else
    [animatedTab setDrawsLeftDivider:NO];

  if ([mCurrentlySlidingTabs count] == 0 && ![self tabIsCurrentlyDragging])
    [self performSelectorOnMainThread:@selector(rebuildTabBar) withObject:nil waitUntilDone:YES];
}

// Called when the dragging tab has been released and is finished animating into its
// proper location.
- (void)animationDidEnd:(NSAnimation *)animation
{
  mTabIsCurrentlyDragging = NO;

  // We need to rebuild the tab bar after the drag is over, but if there are still background
  // tabs animating, just throw only the dragged tab button back on the tab bar and when
  // the background tab animation is finished it will rebuild the bar.
  if ([mCurrentlySlidingTabs count] > 0) {
    [mDraggingTab setFrame:[[mSavedTabFrames objectAtIndex:mEmptyTabPlaceholderIndex] rectValue]];
    [self addSubview:mDraggingTab];
  }
  else {
    [self rebuildTabBar];
  }

  [[self window] removeChildWindow:mDraggingTabWindow];
  // To prevent a rare crash when clicking on the dragged tab window right when it's being removed,
  // do not immediately release it.  (The crash is because of Gecko's NSWindow -sendEvent method swizzling
  // calling firstResponder on our deallocated floating tab window.)
  [mDraggingTabWindow performSelector:@selector(autorelease) withObject:nil afterDelay:2.0];
  [mDraggingTabWindow orderOut:self];
  mDraggingTabWindow = nil;
  [mDraggingTab release];
  mDraggingTab = nil;

  // If this is the only tab button, have the tab view perform the logic about showing and hiding
  // the bar and enabling the close button.
  if ([mTabView numberOfTabViewItems] == 1)
    [mTabView showOrHideTabsAsAppropriate];
}

- (void)saveTabButtonFramesForDragging
{
  // Our tabs do not all have a consistent size (when scrolling tabs, some extra space is spread 
  // out over a few of the leftmost ones).  We need to save the actual frames of the tabs to
  // reference later, rather than just calculating based on index and width.

  if (!mSavedTabFrames)
    mSavedTabFrames = [[NSMutableArray alloc] init];

  [mSavedTabFrames removeAllObjects];

  NSEnumerator* tabViewItemEnum = [[mTabView tabViewItems] objectEnumerator];
  BrowserTabViewItem* tabViewItem;

  while ((tabViewItem = [tabViewItemEnum nextObject])) {
    TabButtonView* currentTabButton = (TabButtonView*)[tabViewItem buttonView];
    NSValue* currentFrameValue = [NSValue valueWithRect:[currentTabButton frame]];
    [mSavedTabFrames addObject:currentFrameValue];
  }
}

- (void)currentlyDraggingTabClosed
{
  mTabIsCurrentlyDragging = NO;

  [mCurrentlySlidingTabs makeObjectsPerformSelector:@selector(stopSliding)];

  [[self window] removeChildWindow:mDraggingTabWindow];

  [mDraggingTab release];
  mDraggingTab = nil;
  [mDraggingTabWindow release];
  mDraggingTabWindow = nil;

  [self performSelector:@selector(rebuildTabBar) withObject:nil afterDelay:0.0];
}

-(void)tabViewClosedTabViewItem:(BrowserTabViewItem*)closedTabViewItem atIndex:(int)indexOfClosedItem
{
  // If we are currently dragging a tab, we need to know right away about a closed tab item
  // and move the others around appropriately.

  if (![self tabIsCurrentlyDragging] || [self isReorderingTabViewItemsDuringDrag])
    return;

  if ([closedTabViewItem isEqual:[mDraggingTab tabViewItem]]) {
    [self currentlyDraggingTabClosed];
    return;
  }

  if (indexOfClosedItem == mEmptyTabPlaceholderIndex)
    return;

  if ([self tabIndexIsVisible:indexOfClosedItem])
    mNumberOfVisibleTabs--;

  if (indexOfClosedItem < mEmptyTabPlaceholderIndex)
    mEmptyTabPlaceholderIndex--;

  // Find all visible tabs to the right of the removed one, and slide them to the left to fill empty spots.
  for (int currentTabIndex = indexOfClosedItem; currentTabIndex <= [self rightmostVisibleTabIndex]; currentTabIndex++) {
    if (currentTabIndex == mEmptyTabPlaceholderIndex) {
      continue;
    }
    else if (currentTabIndex == (mEmptyTabPlaceholderIndex + 1)) {
      // Slide the tab after the empty spot two spaces to the left, because the empty needs to remain
      // in the same location.
      TabButtonView* tabButtonAfterPlaceholder = [self tabButtonAtIndex:currentTabIndex];
      [tabButtonAfterPlaceholder slideToLocation:[[mSavedTabFrames objectAtIndex:currentTabIndex - 1] rectValue].origin];
      continue;
    }

    TabButtonView* tabButton = [self tabButtonAtIndex:currentTabIndex];
    [tabButton slideToLocation:[[mSavedTabFrames objectAtIndex:currentTabIndex] rectValue].origin];
  }

  // If there was a tab to the right of the empty placeholder, we need to move the empty index since
  // it was actually slid to before the empty spot.
  if (indexOfClosedItem <= mEmptyTabPlaceholderIndex &&
      mEmptyTabPlaceholderIndex < (mNumberOfVisibleTabs - 1))
  {
    mEmptyTabPlaceholderIndex++;
  }

  // Update the BrowserTabView to reflect where the dragged image currently will be dropped.
  BrowserTabViewItem* draggedTabViewItem = [[[mDraggingTab tabViewItem] retain] autorelease];
  [self setReorderingTabViewItemsDuringDrag:YES];
  [mTabView removeTabViewItem:draggedTabViewItem];
  [mTabView insertTabViewItem:draggedTabViewItem atIndex:mEmptyTabPlaceholderIndex];
  [mTabView selectTabViewItem:draggedTabViewItem];
  [self setReorderingTabViewItemsDuringDrag:NO];
}

-(void)tabViewAddedTabViewItem:(BrowserTabViewItem*)addedTabViewItem
{
  if (![self tabIsCurrentlyDragging])
    return;

  // If we're dragging, add the new tab rect to the saved frames collection.
  NSRect lastTabButtonFrame = [[mSavedTabFrames lastObject] rectValue];
  NSRect frameForNewButton = lastTabButtonFrame;
  frameForNewButton.origin.x += lastTabButtonFrame.size.width;
  [mSavedTabFrames addObject:[NSValue valueWithRect:frameForNewButton]];
}

- (BOOL)isReorderingTabViewItemsDuringDrag
{
  return mIsReorganizingTabViewItems;
}

- (void)setReorderingTabViewItemsDuringDrag:(BOOL)flag
{
  mIsReorganizingTabViewItems = flag;
}

-(BOOL)tabIsCurrentlyDragging
{
  return mTabIsCurrentlyDragging;
}

@end
