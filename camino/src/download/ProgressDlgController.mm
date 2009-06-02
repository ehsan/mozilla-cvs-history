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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Calum Robinson <calumr@mac.com>
 *   Josh Aas <josh@mozilla.com>
 *   Nick Kreeger <nick.kreeger@park.edu>
 *   Bruce Davidson <mozilla@transoceanic.org.uk>
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

#include "BrowserWindowController.h"

#include "nsNetError.h"

#import "NSString+Utils.h"
#import "NSView+Utils.h"
#import "nsAlertController.h"

#import "ProgressDlgController.h"

#import "PreferenceManager.h"
#import "ProgressViewController.h"
#import "ProgressView.h"

static NSString* const kProgressWindowFrameSaveName = @"ProgressWindow";

@interface ProgressDlgController(PrivateProgressDlgController)

-(void)showErrorSheetForDownload:(id <CHDownloadProgressDisplay>)progressDisplay withStatus:(nsresult)inStatus;
-(void)rebuildViews;
-(NSArray*)selectedProgressViewControllers;
-(ProgressViewController*)progressViewControllerAtIndex:(unsigned)inIndex;
-(void)deselectDownloads:(NSArray*)downloads;
-(void)removeSelectedDownloads:(BOOL)byDeleting;
-(void)scrollIntoView:(ProgressViewController*)controller;
-(void)killDownloadTimer;
-(void)setupDownloadTimer;
-(void)maybeCloseWindow;
-(void)removeSuccessfulDownloads;
-(BOOL)shouldRemoveDownloadsOnQuit;
-(NSString*)downloadsPlistPath;
-(void)setToolTipForToolbarItem:(NSToolbarItem*)theItem;
-(BOOL)isDestructiveAction:(SEL)action;
-(BOOL)shouldAllowAction:(SEL)action;
-(void)saveProgressViewControllers;
-(void)loadProgressViewControllers;

@end

#pragma mark -

@implementation ProgressDlgController

static id gSharedProgressController = nil;

+(ProgressDlgController *)sharedDownloadController
{
  if (gSharedProgressController == nil) {
    gSharedProgressController = [[ProgressDlgController alloc] init];
  }

  return gSharedProgressController;
}

+(ProgressDlgController *)existingSharedDownloadController
{
  return gSharedProgressController;
}

-(id)init
{
  if ((self = [super initWithWindowNibName:@"ProgressDialog"])) {
    // Load the saved instances to mProgressViewControllers array.
    mProgressViewControllers = [[NSMutableArray alloc] init];

    mSelectionPivotIndex = -1;
    [self loadProgressViewControllers];

    mFileChangeWatcher = [[FileChangeWatcher alloc] init];
  }
  return self;
}

-(void)awakeFromNib
{
  mDefaultWindowSize = [[self window] frame].size;
  // it would be nice if we could get the frame from the name, and then
  // mess with it before setting it.
  [[self window] setFrameUsingName:kProgressWindowFrameSaveName];
  
  // we "know" that the superview of the stack view is a CHFlippedShrinkWrapView
  // (it has to be, because NSScrollViews have to contain a flipped view)
  if ([[mStackView superview] respondsToSelector:@selector(setNoIntrinsicPadding)])
    [(CHShrinkWrapView*)[mStackView superview] setNoIntrinsicPadding];
  
  // We provide the views for the stack view, from mProgressViewControllers
  [mStackView setShowsSeparators:YES];
  [mStackView setDataSource:self];
  
  NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"dlmanager1"]; // so pause/resume button will show
  [toolbar setDelegate:self];
  [toolbar setAllowsUserCustomization:YES];
  [toolbar setAutosavesConfiguration:YES];
  [[self window] setToolbar:toolbar];

  // Scroll to last selected download, or to the end if no downloads are selected.
  NSArray* currentlySelectedDownloads = [self selectedProgressViewControllers];
  if ([currentlySelectedDownloads count] > 0) {
    [self scrollIntoView:[currentlySelectedDownloads lastObject]];
  }
  else if ([mProgressViewControllers count] > 0) {
    [self scrollIntoView:[mProgressViewControllers lastObject]];
  }

  [self rebuildViews];
}

-(void)dealloc
{
  if (self == gSharedProgressController) {
    gSharedProgressController = nil;
  }
  [mProgressViewControllers release];
  [mFileChangeWatcher release];
  [self killDownloadTimer];
  [super dealloc];
}

// cancel all selected instances
-(IBAction)cancel:(id)sender
{
  [[self selectedProgressViewControllers] makeObjectsPerformSelector:@selector(cancel:) withObject:sender];
}

// reveal all selected instances in the Finder
-(IBAction)reveal:(id)sender
{
  [[self selectedProgressViewControllers] makeObjectsPerformSelector:@selector(reveal:) withObject:sender];
}

// open all selected instances
-(IBAction)open:(id)sender
{
  if ([self shouldAllowAction:@selector(open:)])
    [[self selectedProgressViewControllers] makeObjectsPerformSelector:@selector(open:) withObject:sender];
  else
    NSBeep();
}

// Removes downloads from the view; also deletes corresponding files if |byDeleting|.
-(void)removeSelectedDownloads:(BOOL)byDeleting
{
  NSArray* selected = [self selectedProgressViewControllers];
  unsigned int selectedCount = [selected count];
  if (selectedCount == 0) return;

  // Prepare to select download following the last selected download.
  // We add 1 to get following download; then subtract |selectedCount| since
  // this many downloads preceding the to-be-selected one have been removed.
  mSelectionPivotIndex = [mProgressViewControllers indexOfObject:[selected lastObject]] + 1 - selectedCount;

  // now remove stuff
  for (unsigned int i = 0; i < selectedCount; i++)
  {
    ProgressViewController* progressController = [selected objectAtIndex:i];
    // If we are not deleting, then we need to check if the download is active.
    // Otherwise, we don't need to check if active; the toolbar/menu validates.
    // Note that if we are deleting and the file was moved without the controller
    // noticing, the move to trash will fail, but cause it to notice that the
    // file is missing.  Leave it in the list (now showing missing) so the user
    // doesn't think it was successfully trashed.
    if ((byDeleting && [progressController deleteFile]) ||
        (!byDeleting && ![progressController isActive])) {
      [self removeDownload:progressController suppressRedraw:YES];
    }
  }

  // Select the pivot download.
  if ([mProgressViewControllers count] != 0) {
    // Ensure that that pivot index is in the range of the array.
    mSelectionPivotIndex = MIN(mSelectionPivotIndex, (int)[mProgressViewControllers count] - 1);
    [(ProgressViewController*)[mProgressViewControllers objectAtIndex:mSelectionPivotIndex] setSelected:YES];
  }
  else {
    mSelectionPivotIndex = -1;
  }

  [self rebuildViews];
  [self saveProgressViewControllers];
}

// remove all selected instances, don't remove anything that is active as a guard against bad things
-(IBAction)remove:(id)sender
{
  [self removeSelectedDownloads:NO];
}

// delete the selected download(s), moving files to trash and clearing them from the list
-(IBAction)deleteDownloads:(id)sender
{
  [self removeSelectedDownloads:YES];
}

-(IBAction)pause:(id)sender
{
  [[self selectedProgressViewControllers] makeObjectsPerformSelector:@selector(pause:) withObject:sender];
  [self rebuildViews];  // because we swap in a different progress view
}

-(IBAction)resume:(id)sender
{
  [[self selectedProgressViewControllers] makeObjectsPerformSelector:@selector(resume:) withObject:sender];
  [self rebuildViews];  // because we swap in a different progress view
  [self setupDownloadTimer];
}

// Remove all inactive instances.
-(IBAction)cleanUpDownloads:(id)sender
{
  unsigned int pivotShift = 0;
  // We loop over the downloads in reverse order so that removing a download
  // does not affect the indices of the downloads we haven't yet looked at.
  for (int i = [mProgressViewControllers count] - 1; i >= 0; i--)
  {
    ProgressViewController* curProgressViewController = [mProgressViewControllers objectAtIndex:i];
    if ((![curProgressViewController isActive]) || [curProgressViewController isCanceled]) {
      [self removeDownload:curProgressViewController suppressRedraw:YES];

      if (mSelectionPivotIndex == i) {
        mSelectionPivotIndex = -1;
      }
      else if (i < mSelectionPivotIndex) {
        pivotShift++;
      }
    }
  }

  mSelectionPivotIndex -= pivotShift;

  [self rebuildViews];
  [self saveProgressViewControllers];
}

// remove all downloads, cancelling if necessary
// this is used for the browser reset function
-(void)clearAllDownloads
{
  // We loop over the downloads in reverse order so that removing a download
  // does not affect the indices of the downloads we haven't yet looked at.
  for (int i = [mProgressViewControllers count] - 1; i >= 0; i--)
  {
    ProgressViewController* curProgressViewController = [mProgressViewControllers objectAtIndex:i];
    // the ProgressViewController method "cancel:" has a sanity check, so its ok to call on anything
    // make sure downloads are not active before removing them
    [curProgressViewController cancel:self];
    [self removeDownload:curProgressViewController suppressRedraw:YES];
  }
  mSelectionPivotIndex = -1;

  [self rebuildViews];
  [self saveProgressViewControllers];
}

-(ProgressViewController*)downloadWithIdentifier:(unsigned int)identifier
{
  NSEnumerator* downloadsEnum = [mProgressViewControllers objectEnumerator];
  ProgressViewController* curController;
  while ((curController = [downloadsEnum nextObject])) {
    if ([curController uniqueIdentifier] == identifier)
      return curController;
  }
  return nil;
}

-(void)updateSelectionOfDownload:(ProgressViewController*)selectedDownload
                    withBehavior:(DownloadSelectionBehavior)behavior
{
  int indexOfSelectedDownload = (int)[mProgressViewControllers indexOfObjectIdenticalTo:selectedDownload];
  NSArray* currentlySelectedDownloads = [self selectedProgressViewControllers];

  if (indexOfSelectedDownload == NSNotFound) {
    if (behavior == DownloadSelectExclusively) {
      [self deselectDownloads:currentlySelectedDownloads];
      mSelectionPivotIndex = -1;
    }

    return;
  }

  switch (behavior) {
    case DownloadSelectExclusively:
      [self deselectDownloads:currentlySelectedDownloads];
      [selectedDownload setSelected:YES];
      mSelectionPivotIndex = indexOfSelectedDownload;
      break;

    // Select all downloads between clicked download and pivot download (inclusive).
    case DownloadSelectByExtending:
      if (mSelectionPivotIndex == -1 || [currentlySelectedDownloads count] == 0) {
        mSelectionPivotIndex = indexOfSelectedDownload;
      }
      else {
        [self deselectDownloads:currentlySelectedDownloads];
        int minSelectedIndex = MIN(indexOfSelectedDownload, mSelectionPivotIndex);
        int maxSelectedIndex = MAX(indexOfSelectedDownload, mSelectionPivotIndex);
        for (int i = minSelectedIndex; i <= maxSelectedIndex; i++) {
          [(ProgressViewController*)[mProgressViewControllers objectAtIndex:i] setSelected:YES];
        }
      }
      break;

    case DownloadSelectByInverting:
      if ([selectedDownload isSelected]) {
        // If this was at the pivot index, clear the pivot index.
        if (indexOfSelectedDownload == mSelectionPivotIndex)
          mSelectionPivotIndex = -1;
      }
      else {
        if ([currentlySelectedDownloads count] == 0)
          mSelectionPivotIndex = indexOfSelectedDownload;
      }
      [selectedDownload setSelected:(![selectedDownload isSelected])];
      break;
  }
}

-(void)keyDown:(NSEvent *)theEvent
{
  // we don't care about anything if no downloads exist
  if ([mProgressViewControllers count] == 0)
  {
    NSBeep();
    return;
  }

  int instanceToSelect = -1;
  BOOL shiftKeyDown = (([theEvent modifierFlags] & NSShiftKeyMask) != 0);

  if ([[theEvent characters] length] < 1)
    return;
  unichar key = [[theEvent characters] characterAtIndex:0];
  switch (key)
  {
    case NSUpArrowFunctionKey:
      {
        // find the first selected item
        int i; // we use this outside the loop so declare it here
        for (i = 0; i < (int)[mProgressViewControllers count]; i++) {
          if ([[mProgressViewControllers objectAtIndex:i] isSelected]) {
            break;
          }
        }
        // deselect everything if the shift key isn't a modifier
        if (!shiftKeyDown)
          [self deselectDownloads:[self selectedProgressViewControllers]];

        if (i == (int)[mProgressViewControllers count]) // if nothing was selected select the first item
          instanceToSelect = 0;
        else if (i == 0) // if selection was already at the top leave it there
          instanceToSelect = 0;
        else // select the next highest instance
          instanceToSelect = i - 1;

        // select and make sure its visible
        if (instanceToSelect != -1)
        {
          ProgressViewController* dlToSelect = [mProgressViewControllers objectAtIndex:instanceToSelect];
          [dlToSelect setSelected:YES];
          [self scrollIntoView:dlToSelect];
          if (!shiftKeyDown)
            mSelectionPivotIndex = instanceToSelect;
        }
      }
      break;

    case NSDownArrowFunctionKey:
      {
        // find the last selected item
        int i; // we use this outside the coming loop so declare it here
        for (i = [mProgressViewControllers count] - 1; i >= 0 ; i--) {
          if ([[mProgressViewControllers objectAtIndex:i] isSelected])
            break;
        }

        // deselect everything if the shift key isn't a modifier
        if (!shiftKeyDown)
          [self deselectDownloads:[self selectedProgressViewControllers]];

        if (i < 0) // if nothing was selected select the first item
          instanceToSelect = ([mProgressViewControllers count] - 1);
        else if (i == ((int)[mProgressViewControllers count] - 1)) // if selection was already at the bottom leave it there
          instanceToSelect = ([mProgressViewControllers count] - 1);
        else // select the next lowest instance
          instanceToSelect = i + 1;

        if (instanceToSelect != -1)
        {
          ProgressViewController* dlToSelect = [mProgressViewControllers objectAtIndex:instanceToSelect];
          [dlToSelect setSelected:YES];
          [self scrollIntoView:dlToSelect];
          if (!shiftKeyDown)
            mSelectionPivotIndex = instanceToSelect;
        }
      }
      break;

    case NSDeleteFunctionKey:
    case NSDeleteCharacter:
      { // delete or fwd-delete key - remove all selected items unless an active one is selected
        if ([self shouldAllowAction:@selector(remove:)])
          [self remove:self];
        else
          NSBeep();
      }
      break;

    case NSPageUpFunctionKey:
      if ([mProgressViewControllers count] > 0) {
        // make the first instance completely visible
        [self scrollIntoView:((ProgressViewController*)[mProgressViewControllers objectAtIndex:0])];
      }
      break;

    case NSPageDownFunctionKey:
      if ([mProgressViewControllers count] > 0) {
        // make the last instance completely visible
        [self scrollIntoView:((ProgressViewController*)[mProgressViewControllers lastObject])];
      }
      break;

    default:
      NSBeep();
      break;
  }
}

// Selects all of the downlaods in the window.
-(IBAction)selectAll:(id)sender
{
  unsigned int count = [mProgressViewControllers count];
  for (unsigned int i = 0; i < count; i++) {
    [(ProgressViewController*)[mProgressViewControllers objectAtIndex:i] setSelected:YES];
  }
}

-(void)deselectDownloads:(NSArray*)downloads
{
  unsigned count = [downloads count];
  for (unsigned i = 0; i < count; i++) {
    [(ProgressViewController*)[downloads objectAtIndex:i] setSelected:NO];
  }
}

// Returns the currently-selected download view controllers.
-(NSArray*)selectedProgressViewControllers
{
  NSMutableArray *selectedArray = [[NSMutableArray alloc] init];
  unsigned selectedCount = [mProgressViewControllers count];
  for (unsigned i = 0; i < selectedCount; i++) {
    if ([[mProgressViewControllers objectAtIndex:i] isSelected])
      [selectedArray addObject:[mProgressViewControllers objectAtIndex:i]];
  }
  [selectedArray autorelease];
  return selectedArray;
}

-(ProgressViewController*)progressViewControllerAtIndex:(unsigned)inIndex
{
  return (ProgressViewController*) [mProgressViewControllers objectAtIndex:inIndex];
}

-(void)scrollIntoView:(ProgressViewController*)controller
{
  NSView* dlView = [controller view];
  NSRect instanceFrame = [[mScrollView contentView] convertRect:[dlView bounds] fromView:dlView];
  NSRect visibleRect = [[mScrollView contentView] documentVisibleRect];

  if (!NSContainsRect(visibleRect, instanceFrame)) { // if instance isn't completely visible
    if (instanceFrame.origin.y < visibleRect.origin.y) { // if the dl instance is at least partly above visible rect
      // just go to the instance's frame origin point
      [[mScrollView contentView] scrollToPoint:instanceFrame.origin];
    }
    else { // if the dl instance is at least partly below visible rect
      // take instance's frame origin y, subtract content view height,
      // add instance view height, no parenthesizing
      NSPoint adjustedPoint = NSMakePoint(0, (instanceFrame.origin.y - NSHeight([[mScrollView contentView] frame])) + NSHeight(instanceFrame));
      [[mScrollView contentView] scrollToPoint:adjustedPoint];
    }
    [mScrollView reflectScrolledClipView:[mScrollView contentView]];
  }
}

-(void)didStartDownload:(ProgressViewController*)progressDisplay
{
  BOOL gotPref;
  NSWindow* downloadManagerWindow = [self window];
  BOOL shouldOpenManager = [[PreferenceManager sharedInstance] getBooleanPref:kGeckoPrefOpenDownloadManagerOnDownload
                                                                  withSuccess:&gotPref];
  if (shouldOpenManager || !gotPref) {
    // A common cause of user confusion is the window being visible but behind other
    // windows. They have no idea the download was successful, and click the link
    // two or three times before looking around to see what happened.
    // This ensures the download manager is open and visible, and has the side effect of
    // making it key and front (i.e., focusing it).
    // Some people don't want the manager to be stealing focus on every download,
    // so we support a (hidden) pref to allow the manager not to be focused each time.
    BOOL bringToFront = [[PreferenceManager sharedInstance] getBooleanPref:kGeckoPrefFocusDownloadManagerOnDownload
                                                               withSuccess:&gotPref];
    if (bringToFront || !gotPref) {
      [self showWindow:nil];
    }
    else if (![downloadManagerWindow isVisible]) {
      // If the hidden pref is set to false and the manager isn't already open,
      // open it and send it to the back of all browser windows. This is sort of
      // arbitrary, but it beats |orderBack| (which includes visible windows
      // from other apps and can thus look weird).
      // NB: We include popups and view-source windows in this definition of
      // "browser window", though we generally don't elsewhere.

      // Store the current key window before we stomp it.
      NSWindow* storedKeyWindow = [NSApp keyWindow];

      [self showWindow:nil];

      NSEnumerator* windowEnum = [[NSApp orderedWindows] reverseObjectEnumerator];
      NSWindow* rearmostBrowserWindow = nil;

      NSWindow* curWindow;
      while ((curWindow = [windowEnum nextObject])) {
        if ([[curWindow windowController] isMemberOfClass:[BrowserWindowController class]]) {
          rearmostBrowserWindow = curWindow;
          break;
        }
      }

      if (rearmostBrowserWindow) {
        [downloadManagerWindow orderWindow:NSWindowBelow relativeTo:[rearmostBrowserWindow windowNumber]];
        // Restore the stored key/main window.
        [storedKeyWindow makeKeyAndOrderFront:self];
      }
    }
  }

  [self rebuildViews];
  [self setupDownloadTimer];

  // Downloads should be individually selected when initiated.
  [self updateSelectionOfDownload:progressDisplay withBehavior:DownloadSelectExclusively];

  // Make sure the new download is visible.
  [self scrollIntoView:progressDisplay];
}

-(void)didEndDownload:(ProgressViewController*)progressDisplay withSuccess:(BOOL)completedOK statusCode:(nsresult)status
{
  [self rebuildViews]; // to swap in the completed view
  [[[self window] toolbar] validateVisibleItems]; // force update which doesn't always happen

  // close the window if user has set pref to close when all downloads complete
  if (completedOK)
  {
    if (!mAwaitingTermination)
      [self maybeCloseWindow];
    else
      mShouldCloseWindow = YES;
  }
  else if (NS_FAILED(status) && status != NS_BINDING_ABORTED)  // if it's an error, and not just canceled, show sheet
  {
    [self showErrorSheetForDownload:progressDisplay withStatus:status];
  }

  [self saveProgressViewControllers];
}

-(void)maybeCloseWindow
{
  // Only check if there are zero downloads running and there is no sheet
  // (e.g. toolbar customization sheet) attached.
  if ([self numDownloadsInProgress] == 0 && ![[self window] attachedSheet]) {
    BOOL gotPref;
    BOOL closeDownloadManager = [[PreferenceManager sharedInstance] getBooleanPref:kGeckoPrefCloseDownloadManagerWhenDone
                                                                       withSuccess:&gotPref];
    if (gotPref && closeDownloadManager) {
      // don't call -performClose: on the window, because we don't want Cocoa to look
      // for the option key and try to close all windows
      [self close];
    }
  }
}

-(void)showErrorSheetForDownload:(id <CHDownloadProgressDisplay>)progressDisplay withStatus:(nsresult)inStatus
{
  NSString* errorMsgFmt = NSLocalizedString(@"DownloadErrorMsgFmt", @"");
  NSString* errorExplString = nil;

  NSString* destFilePath = [progressDisplay destinationPath];
  NSString* fileName = [destFilePath displayNameOfLastPathComponent];

  NSString* errorMsg = [NSString stringWithFormat:errorMsgFmt, fileName];

  switch (inStatus)
  {
    case NS_ERROR_FILE_DISK_FULL:
    case NS_ERROR_FILE_NO_DEVICE_SPACE:
      {
        NSString* fmtString = NSLocalizedString(@"DownloadErrorNoDiskSpaceOnVolumeFmt", @"");
        errorExplString = [NSString stringWithFormat:fmtString, [destFilePath volumeNamePathComponent]];
      }
      break;

    case NS_ERROR_FILE_ACCESS_DENIED:
      {
        NSString* fmtString = NSLocalizedString(@"DownloadErrorDestinationWriteProtectedFmt", @"");
        NSString* destDirPath = [destFilePath stringByDeletingLastPathComponent];
        errorExplString = [NSString stringWithFormat:fmtString, [destDirPath displayNameOfLastPathComponent]];
      }
      break;

    case NS_ERROR_FILE_TOO_BIG:
    case NS_ERROR_FILE_READ_ONLY:
    default:
      {
        errorExplString = NSLocalizedString(@"DownloadErrorOther", @"");
        NSLog(@"Download failure code: %X", inStatus);
      }
      break;
  }

  NSBeginAlertSheet(errorMsg,
                    nil,    // default button ("OK")
                    nil,    // alt button (none)
                    nil,    // other button (nil)
                    [self window],
                    self,
                    @selector(downloadErrorSheetDidEnd:returnCode:contextInfo:),
                    nil,    // didDismissSelector
                    NULL,   // context info
                    errorExplString);
}

-(void)downloadErrorSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
}

-(void)removeDownload:(ProgressViewController*)progressDisplay suppressRedraw:(BOOL)suppressRedraw
{
  [progressDisplay displayWillBeRemoved];
  // This is sometimes called by code that thinks it can continue
  // to use |progressDisplay|. Extended the lifetime slightly as a
  // band-aid, but this logic should really be reworked.
  [[progressDisplay retain] autorelease];
  [mProgressViewControllers removeObject:progressDisplay];

  if ([mProgressViewControllers count] == 0) {
    // Stop doing stuff if there aren't any downloads going on
    [self killDownloadTimer];
  }

  if (!suppressRedraw)
    [self rebuildViews];
}

-(void)rebuildViews
{
  [mStackView adaptToSubviews];
}

-(int)numDownloadsInProgress
{
  unsigned numViews = [mProgressViewControllers count];
  int numActive = 0;

  for (unsigned int i = 0; i < numViews; i++) {
    if ([[mProgressViewControllers objectAtIndex:i] isActive]) {
      ++numActive;
    }
  }
  return numActive;
}

-(void)autosaveWindowFrame
{
  [[self window] saveFrameUsingName:kProgressWindowFrameSaveName];
}

-(void)windowWillClose:(NSNotification *)notification
{
  [self autosaveWindowFrame];
}

-(void)killDownloadTimer
{
  if (mDownloadTimer) {
    [mDownloadTimer invalidate];
    [mDownloadTimer release];
    mDownloadTimer = nil;
  }
}

// Called by our timer to refresh all the download stats.
// Kills the download timer if no active downloads remain.
- (void)setDownloadProgress:(NSTimer *)aTimer
{
  bool activeDownloadExists = NO;
  
  // For efficiency, only refresh info for active downloads.
  NSEnumerator* downloadsEnum = [mProgressViewControllers objectEnumerator];
  ProgressViewController* curController;
  while ((curController = [downloadsEnum nextObject])) {
    if ([curController isActive] && ![curController isPaused]) {
      activeDownloadExists = YES;
      [curController performSelector:@selector(refreshDownloadInfo)];
    }
  }

  // Kill the download timer if no active downloads remain.
  if (!activeDownloadExists) {
    [self killDownloadTimer];
  }
}

- (void)setupDownloadTimer
{
  [self killDownloadTimer];
  // note that this sets up a retain cycle between |self| and the timer,
  // which has to be broken out of band, before we'll be dealloc'd.
  mDownloadTimer = [[NSTimer scheduledTimerWithTimeInterval:1.0
                                                     target:self
                                                   selector:@selector(setDownloadProgress:)
                                                   userInfo:nil
                                                    repeats:YES] retain];
  // make sure it fires even when the mouse is down
  [[NSRunLoop currentRunLoop] addTimer:mDownloadTimer forMode:NSEventTrackingRunLoopMode];
}

-(NSApplicationTerminateReply)allowTerminate
{
  BOOL shouldTerminate = YES;
  BOOL downloadsInProgress = ([self numDownloadsInProgress] > 0 ? YES : NO);
	
  if (downloadsInProgress)
  {
    // set this bool to true so that if a download finishes while the modal
    // sheet is up we don't lock
    mAwaitingTermination = YES;

    // make sure the window is visible
    [self showWindow:self];

    NSString *title    = NSLocalizedString(@"QuitWithDownloadsMsg", nil);
    NSString *text     = NSLocalizedString(@"QuitWithDownloadsExpl", nil);
    NSString *dontQuit = NSLocalizedString(@"DontQuitButtonText", nil);
    NSString *quit     = NSLocalizedString(@"QuitButtonText", nil);

    nsAlertController* alertController = [nsAlertController sharedController];
    int sheetResult = [alertController confirmDestructive:[self window]
                                                    title:title
                                                     text:text
                                                  button1:dontQuit
                                                  button2:quit
                                                  button3:nil];

    if (sheetResult == NSAlertDefaultReturn)
    {
      mAwaitingTermination = NO;

      // Check to see if a request to close the download window was made while the modal sheet was running.
      // If so, close the download window according to the user's pref.
      if (mShouldCloseWindow)
      {
        [self maybeCloseWindow];
        mShouldCloseWindow = NO;
      }

      shouldTerminate = NO;
    }
  }

  return shouldTerminate ? NSTerminateNow : NSTerminateCancel;
}

// Called by MainController when the application is about to terminate.
// Either save the progress view's or remove them according to the download removal pref.
-(void)applicationWillTerminate
{
  // Check the download item removal policy here to see if downloads should be removed when Camino quits.
  if ([self shouldRemoveDownloadsOnQuit])
    [self removeSuccessfulDownloads];

  // Since the pref is not set to remove the downloads when Camino quits, save them here before the app terminates.
  else
    [self saveProgressViewControllers];
}

-(void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
  [NSApp stopModalWithCode:returnCode];
}

-(void)saveProgressViewControllers
{
  unsigned int arraySize = [mProgressViewControllers count];
  NSMutableArray* downloadArray = [[[NSMutableArray alloc] initWithCapacity:arraySize] autorelease];

  NSEnumerator* downloadsEnum = [mProgressViewControllers objectEnumerator];
  ProgressViewController* curController;
  while ((curController = [downloadsEnum nextObject])) {
    [downloadArray addObject:[curController downloadInfoDictionary]];
  }

  // Now save the array.
  [downloadArray writeToFile:[self downloadsPlistPath] atomically: YES];
}

-(void)loadProgressViewControllers
{
  NSArray* downloads = [NSArray arrayWithContentsOfFile:[self downloadsPlistPath]];

  if (downloads) {
    NSEnumerator* downloadsEnum = [downloads objectEnumerator];
    NSDictionary* downloadsDictionary;
    while((downloadsDictionary = [downloadsEnum nextObject])) {
      ProgressViewController* curController = [[ProgressViewController alloc] initWithDictionary:downloadsDictionary
                                                                             andWindowController:self];
      [mProgressViewControllers addObject:curController];
      [curController release];
    }

    NSArray* selectedDownloads = [self selectedProgressViewControllers];
    if ([selectedDownloads count] > 0) {
      mSelectionPivotIndex = [mProgressViewControllers indexOfObject:[selectedDownloads lastObject]];
    }
  }
}

-(void)addFileDelegateToWatchList:(id<WatchedFileDelegate>)aWatchedFileDelegate
{
  [mFileChangeWatcher addWatchedFileDelegate:aWatchedFileDelegate];
}

-(void)removeFileDelegateFromWatchList:(id<WatchedFileDelegate>)aWatchedFileDelegate
{
  [mFileChangeWatcher removeWatchedFileDelegate:aWatchedFileDelegate];
}

// Remove the successful downloads from the downloads list
-(void)removeSuccessfulDownloads
{
  // We loop over the downloads in reverse order so that removing a download
  // does not affect the indices of the downloads we haven't yet looked at.
  for (int i = [mProgressViewControllers count] - 1; i >= 0; i--)
  {
    // Remove successful downloads from the list.
    if ([[mProgressViewControllers objectAtIndex:i] hasSucceeded])
      [mProgressViewControllers removeObjectAtIndex:i];
  }

  [self saveProgressViewControllers];
}

// Return true if the pref is set to remove downloads when the application quits
-(BOOL)shouldRemoveDownloadsOnQuit
{
  int downloadRemovalPolicy = [[PreferenceManager sharedInstance] getIntPref:kGeckoPrefDownloadCleanupPolicy
                                                                 withSuccess:NULL];
  return (downloadRemovalPolicy == kRemoveDownloadsOnQuit);
}

// Get the downloads.plist path
-(NSString*)downloadsPlistPath
{
  return [[[PreferenceManager sharedInstance] profilePath] stringByAppendingPathComponent:@"downloads.plist"];
}

-(BOOL)isDestructiveAction:(SEL)action
{
  return
    (action == @selector(remove:) ||
     action == @selector(cancel:) ||
     action == @selector(deleteDownloads:) ||
     action == @selector(pause:));
}

// Returns YES if the action makes sense given the state of the selected downloads.
-(BOOL)shouldAllowAction:(SEL)action
{
  // Don't allow click-through for actions with potential dataloss.
  if ([self isDestructiveAction:action] && ![[self window] isKeyWindow])
    return NO;

  // Actions act on selected downloads, so if no downloads are selected, disallow the action.
  NSArray *selectedDownloads = [self selectedProgressViewControllers];
  if ([selectedDownloads count] == 0)
    return NO;

  // Return YES only if every selected download supports the action.
  NSEnumerator* progViewEnum = [selectedDownloads objectEnumerator];
  ProgressViewController* curController;
  while ((curController = [progViewEnum nextObject]))
  {
    if (![curController shouldAllowAction:action])
      return NO;
  }

  return YES;
}

-(BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
  return [self shouldAllowAction:[menuItem action]];
}

// Sets the icon, label, and action for the pause/resume toolbar button.
// We only enable the button if the download manager is the key window and either
// the pause or resume action applies to the currently selected downloads.
- (BOOL)setPauseResumeToolbarItem:(NSToolbarItem*)theItem
{
  if ([self shouldAllowAction:@selector(resume:)]) {
    [theItem setImage:[NSImage imageNamed:@"dl_resume"]];
    [theItem setLabel:NSLocalizedString(@"dlResumeButtonLabel", nil)];
    [theItem setAction:@selector(resume:)];
    
    return [[self window] isKeyWindow];
  }
  else {
    [theItem setImage:[NSImage imageNamed:@"dl_pause"]];    
    [theItem setLabel:NSLocalizedString(@"dlPauseButtonLabel", nil)];
    [theItem setAction:@selector(pause:)];

    return [self shouldAllowAction:@selector(pause:)];
  }
}

-(BOOL)validateToolbarItem:(NSToolbarItem *)theItem
{
  SEL action = [theItem action];

  // validate items not dependent on the current selection.  Must include all such items.
  if (action == @selector(showWindow:))
    return YES;
  else if (action == @selector(cleanUpDownloads:)) {
    if (![[self window] isKeyWindow])
      return NO; //XXX Get rid of me once we can resume cancelled/failed downloads

    unsigned pcControllersCount = [mProgressViewControllers count];
    for (unsigned i = 0; i < pcControllersCount; i++)
    {
      ProgressViewController* curController = [mProgressViewControllers objectAtIndex:i];
      if ((![curController isActive]) || [curController isCanceled])
        return YES;
    }
    return NO;
  }

  // validate items that depend on current selection
  [self setToolTipForToolbarItem:theItem];

  if (action == @selector(pause:) || action == @selector(resume:)) {
    return [self setPauseResumeToolbarItem:theItem];
  }
  else {
    return [self shouldAllowAction:action];
  }
}

-(NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
  NSToolbarItem *theItem = [[[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier] autorelease];
  [theItem setTarget:self];
  [theItem setEnabled:NO];
  [self setToolTipForToolbarItem:theItem];

  if ([itemIdentifier isEqualToString:@"removebutton"]) {
    [theItem setLabel:NSLocalizedString(@"dlRemoveButtonLabel", nil)];
    [theItem setPaletteLabel:NSLocalizedString(@"dlRemoveButtonLabel", nil)];
    [theItem setAction:@selector(remove:)];
    [theItem setImage:[NSImage imageNamed:@"dl_remove"]];
  }
  else if ([itemIdentifier isEqualToString:@"cancelbutton"]) {
    [theItem setLabel:NSLocalizedString(@"dlCancelButtonLabel", nil)];
    [theItem setPaletteLabel:NSLocalizedString(@"dlCancelButtonLabel", nil)];
    [theItem setAction:@selector(cancel:)];
    [theItem setImage:[NSImage imageNamed:@"dl_cancel"]];
  }
  else if ([itemIdentifier isEqualToString:@"revealbutton"]) {
    [theItem setLabel:NSLocalizedString(@"dlRevealButtonLabel", nil)];
    [theItem setPaletteLabel:NSLocalizedString(@"dlRevealButtonLabel", nil)];
    [theItem setAction:@selector(reveal:)];
    [theItem setImage:[NSImage imageNamed:@"dl_reveal"]];
  }
  else if ([itemIdentifier isEqualToString:@"openbutton"]) {
    [theItem setLabel:NSLocalizedString(@"dlOpenButtonLabel", nil)];
    [theItem setPaletteLabel:NSLocalizedString(@"dlOpenButtonLabel", nil)];
    [theItem setAction:@selector(open:)];
    [theItem setImage:[NSImage imageNamed:@"dl_open"]];
  }
  else if ([itemIdentifier isEqualToString:@"cleanupbutton"]) {
    [theItem setLabel:NSLocalizedString(@"dlCleanUpButtonLabel", nil)];
    [theItem setPaletteLabel:NSLocalizedString(@"dlCleanUpButtonLabel", nil)];
    [theItem setAction:@selector(cleanUpDownloads:)];
    [theItem setImage:[NSImage imageNamed:@"dl_clearall"]];
  }
  else if ([itemIdentifier isEqualToString:@"movetotrashbutton"]) {
    [theItem setLabel:NSLocalizedString(@"dlTrashButtonLabel", nil)];
    [theItem setPaletteLabel:NSLocalizedString(@"dlTrashButtonLabel", nil)];
    [theItem setAction:@selector(deleteDownloads:)];
    [theItem setImage:[NSImage imageNamed:@"dl_trash"]];
  }
  else if ([itemIdentifier isEqualToString:@"pauseresumebutton"]) {
    // If the button is being requested for the toolbar, choose the pause or
    // resume version as usual, depending on the state of the selected downloads.
    // Otherwise (if it is being requested for the toolbar customization sheet),
    // always display the pause version.
    if (flag) {
      [self setPauseResumeToolbarItem:theItem];
    }
    else {
      [theItem setLabel:NSLocalizedString(@"dlPauseButtonLabel", nil)];
      [theItem setPaletteLabel:NSLocalizedString(@"dlPauseButtonLabel", nil)];
      [theItem setImage:[NSImage imageNamed:@"dl_pause"]];
    }
  }
  else {
    return nil;
  }
  return theItem;
}

-(NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
  return [NSArray arrayWithObjects:@"cleanupbutton",
                                   @"removebutton",
                                   @"cancelbutton",
                                   @"pauseresumebutton",
                                   @"openbutton",
                                   @"revealbutton",
                                   @"movetotrashbutton",
                                   NSToolbarCustomizeToolbarItemIdentifier,
                                   NSToolbarFlexibleSpaceItemIdentifier,
                                   NSToolbarSpaceItemIdentifier,
                                   NSToolbarSeparatorItemIdentifier,
                                   nil];
}

-(NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
  return [NSArray arrayWithObjects:@"cleanupbutton",
                                   @"removebutton",
                                   @"cancelbutton",
                                   @"pauseresumebutton",
                                   @"openbutton",
                                   NSToolbarFlexibleSpaceItemIdentifier,
                                   @"revealbutton",
                                   nil];
}

- (void)setToolTipForToolbarItem:(NSToolbarItem *)theItem
{
  NSString* toolTip = nil;
  NSString* itemIdentifier = [theItem itemIdentifier];
  BOOL plural = ([[self selectedProgressViewControllers] count] > 1);

  if ([itemIdentifier isEqualToString:@"cleanupbutton"])
    toolTip = NSLocalizedString(@"dlCleanUpButtonTooltip", nil);
  else if ([itemIdentifier isEqualToString:@"removebutton"])
    toolTip = NSLocalizedString((plural ? @"dlRemoveButtonTooltipPlural" : @"dlRemoveButtonTooltip"), nil);
  else if ([itemIdentifier isEqualToString:@"cancelbutton"])
    toolTip = NSLocalizedString((plural ? @"dlCancelButtonTooltipPlural" : @"dlCancelButtonTooltip"), nil);
  else if ([itemIdentifier isEqualToString:@"revealbutton"])
    toolTip = NSLocalizedString((plural ? @"dlRevealButtonTooltipPlural" : @"dlRevealButtonTooltip"), nil);
  else if ([itemIdentifier isEqualToString:@"openbutton"])
    toolTip = NSLocalizedString((plural ? @"dlOpenButtonTooltipPlural" : @"dlOpenButtonTooltip"), nil);
  else if ([itemIdentifier isEqualToString:@"movetotrashbutton"])
    toolTip = NSLocalizedString((plural ? @"dlTrashButtonTooltipPlural" : @"dlTrashButtonTooltip"), nil);
  else if ([itemIdentifier isEqualToString:@"pauseresumebutton"]) {
    if ([self shouldAllowAction:@selector(resume:)])
      toolTip = NSLocalizedString((plural ? @"dlResumeButtonTooltipPlural" : @"dlResumeButtonTooltip"), nil);
    else
      toolTip = NSLocalizedString((plural ? @"dlPauseButtonTooltipPlural" : @"dlPauseButtonTooltip"), nil);
  }

  if (toolTip)
    [theItem setToolTip:toolTip];
}

#pragma mark -

// zoom to fit contents, but don't go under minimum size
-(NSRect)windowWillUseStandardFrame:(NSWindow *)sender defaultFrame:(NSRect)defaultFrame
{
  NSRect windowFrame = [[self window] frame];
  NSSize curScrollFrameSize = [mScrollView frame].size;
  NSSize scrollFrameSize = [NSScrollView frameSizeForContentSize:[mStackView bounds].size
                                         hasHorizontalScroller:NO hasVerticalScroller:YES borderType:NSNoBorder];
  float frameDelta = (curScrollFrameSize.height - scrollFrameSize.height);

  // don't get vertically smaller than the default window size
  if ((windowFrame.size.height - frameDelta) < mDefaultWindowSize.height) {
    frameDelta = windowFrame.size.height - mDefaultWindowSize.height;
  }

  windowFrame.size.height -= frameDelta;
  windowFrame.origin.y    += frameDelta; // maintain top
  windowFrame.size.width  = mDefaultWindowSize.width;

  // cocoa will ensure that the window fits onscreen for us
  return windowFrame;
}

#pragma mark -

/*
 CHStackView datasource methods
 */

- (NSArray*)subviewsForStackView:(CHStackView *)stackView
{
  NSMutableArray* viewsArray = [NSMutableArray arrayWithCapacity:[mProgressViewControllers count]];

  unsigned int numViews = [mProgressViewControllers count];
  for (unsigned int i = 0; i < numViews; i ++)
    [viewsArray addObject:[((ProgressViewController*)[mProgressViewControllers objectAtIndex:i]) view]];

  return viewsArray;
}

#pragma mark -

/*
 Just create a progress view, but don't display it (otherwise the URL fields etc.
                                                    are just blank)
 */
-(id <CHDownloadProgressDisplay>)createProgressDisplay
{
  ProgressViewController* newController = [[[ProgressViewController alloc] initWithWindowController:self] autorelease];
  [mProgressViewControllers addObject:newController];

  return newController;
}

@end
