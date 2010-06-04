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
 * The Original Code is Growl notifications for Camino.
 *
 * The Initial Developer of the Original Code is
 * Ben Willmore.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ben Willmore <bdeb@willmore.eu>
 *   Ilya Sherman <isherman@cs.stanford.edu>
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


#import "GrowlController.h"
#import "ProgressDlgController.h"
#import "ProgressViewController.h"

// Names of keys for objects passed in these notifications
static NSString* const kGrowlNotificationNameKey = @"GrowlNotificationNameKey";
static NSString* const kGrowlNotificationObjectKey = @"GrowlNotificationObjectKey";
static NSString* const kGrowlNotificationUserInfoKey = @"GrowlNotificationUserInfoKey";


// Download duration (in seconds) less than which a download is considered 'short'
static NSTimeInterval const kShortDownloadInterval = 15.0;


@interface GrowlController (Private)

- (void)growlForNotification:(NSNotification*)notification;

@end

@implementation GrowlController

- (id)init
{
  if ((self = [super init])) {
    NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
    [notificationCenter addObserver:self
                           selector:@selector(growlForNotification:)
                               name:kDownloadStartedNotificationName
                             object:nil];
    [notificationCenter addObserver:self
                           selector:@selector(growlForNotification:)
                               name:kDownloadFailedNotificationName
                             object:nil];
    [notificationCenter addObserver:self
                           selector:@selector(growlForNotification:)
                               name:kDownloadCompleteNotificationName
                             object:nil];
  }
  return self;
}

- (void)dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  if (mGrowlIsInitialized) {
    // Note this is never acutally called, since Growl retains its delegate.
    [GrowlApplicationBridge setGrowlDelegate:nil];
  }
  [super dealloc];
}

- (NSDictionary*)registrationDictionaryForGrowl
{
  NSArray* allNotifications = [NSArray arrayWithObjects:
                               NSLocalizedString(@"GrowlDownloadStarted", nil),
                               NSLocalizedString(@"GrowlDownloadFailed", nil),
                               NSLocalizedString(@"GrowlShortDownloadComplete", nil),
                               NSLocalizedString(@"GrowlDownloadComplete", nil),
                               nil];

  NSDictionary* notificationDictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                          allNotifications, GROWL_NOTIFICATIONS_DEFAULT,
                                          allNotifications, GROWL_NOTIFICATIONS_ALL,
                                          nil];

  return notificationDictionary;
}

- (void)growlForNotification:(NSNotification*)notification
{
  if (!mGrowlIsInitialized) {
    mGrowlIsInitialized = YES;
    [GrowlApplicationBridge setGrowlDelegate:self];
  }
  NSString* notificationName = [notification name];
  ProgressViewController* download = [notification object];
  NSNumber* downloadIdentifier = [NSNumber numberWithUnsignedInt:[download uniqueIdentifier]];
  NSDictionary* userInfo = [notification userInfo];

  NSString* title = nil;
  NSString* name = nil;

  if ([notificationName isEqual:kDownloadStartedNotificationName]) {
    name = NSLocalizedString(@"GrowlDownloadStarted", nil);
  }
  else if ([notificationName isEqual:kDownloadFailedNotificationName]) {
    name = NSLocalizedString(@"GrowlDownloadFailed", nil);
  }
  else if ([notificationName isEqual:kDownloadCompleteNotificationName]) {
    double timeElapsed = [[userInfo objectForKey:kDownloadNotificationTimeElapsedKey] doubleValue];
    if (timeElapsed < kShortDownloadInterval) {
      name = NSLocalizedString(@"GrowlShortDownloadComplete", nil);
      title = NSLocalizedString(@"GrowlDownloadComplete", nil);
    }
    else {
      name = NSLocalizedString(@"GrowlDownloadComplete", nil);
    }
  }
  if (!title)
    title = name;

  NSString* description = [[userInfo objectForKey:kDownloadNotificationFilenameKey] lastPathComponent];
  NSDictionary* context = [NSDictionary dictionaryWithObjectsAndKeys:
                           notificationName, kGrowlNotificationNameKey,
                           downloadIdentifier, kGrowlNotificationObjectKey,
                           userInfo, kGrowlNotificationUserInfoKey,
                           nil];

  [GrowlApplicationBridge notifyWithTitle:title
                              description:description
                         notificationName:name
                                 iconData:nil
                                 priority:0
                                 isSticky:0
                             clickContext:context];
}

- (void)growlNotificationWasClicked:(id)clickContext
{
  NSString* notificationName = [clickContext objectForKey:kGrowlNotificationNameKey];

  if ([notificationName isEqual:kDownloadStartedNotificationName] ||
      [notificationName isEqual:kDownloadFailedNotificationName])
  {
    ProgressDlgController* progressWindowController = [ProgressDlgController existingSharedDownloadController];
    if (progressWindowController) {
      [progressWindowController showWindow:self];

      unsigned int downloadIdentifier = [[clickContext objectForKey:kGrowlNotificationObjectKey] unsignedIntValue];
      ProgressViewController* downloadInstance = [progressWindowController downloadWithIdentifier:downloadIdentifier];
      // If downloadInstance is |nil|, this will clear the selection, which is what we want
      [progressWindowController updateSelectionOfDownload:downloadInstance
                                             withBehavior:DownloadSelectExclusively];
    }
  }
  else if ([notificationName isEqual:kDownloadCompleteNotificationName]) {
    // Reveal the file directly rather than asking the progressViewController to
    // reveal it because users can ask Camino to automatically remove downloads
    // upon completion.
    NSString* filename = [[clickContext objectForKey:kGrowlNotificationUserInfoKey] objectForKey:kDownloadNotificationFilenameKey];
    if (filename && [[NSFileManager defaultManager] fileExistsAtPath:filename]) {
      [[NSWorkspace sharedWorkspace] selectFile:filename
                       inFileViewerRootedAtPath:[filename stringByDeletingLastPathComponent]];
    }
  }
}

@end // GrowlController
