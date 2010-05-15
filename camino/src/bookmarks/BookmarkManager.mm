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
 *   David Haas <haasd@cae.wisc.edu>
 *   Josh Aas <josh@mozilla.com>
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

#include <unistd.h>

#include "nsString.h"
#include "nsIContent.h"
#include "nsIFile.h"
#include "nsAppDirectoryServiceDefs.h"

#import "NSString+Utils.h"
#import "NSArray+Utils.h"
#import "NSThread+Utils.h"
#import "NSFileManager+Utils.h"
#import "NSWorkspace+Utils.h"
#import "NSPasteboard+Utils.h"
#import "NSMenu+Utils.h"

#import "PreferenceManager.h"
#import "BookmarkManager.h"
#import "Bookmark.h"
#import "BookmarkFolder.h"
#import "BookmarkToolbar.h"
#import "BookmarkImportDlgController.h"
#import "BookmarkOutlineView.h"
#import "BookmarkViewController.h"
#import "KindaSmartFolderManager.h"
#import "BrowserWindowController.h"
#import "MainController.h"
#import "SiteIconProvider.h"
#import "HTMLBookmarkConverter.h"
#import "OperaBookmarkConverter.h"
#import "SafariBookmarkConverter.h"

NSString* const kBookmarkManagerStartedNotification = @"BookmarkManagerStartedNotification";

// root bookmark folder identifiers (must be unique!)
NSString* const kBookmarksToolbarFolderIdentifier              = @"BookmarksToolbarFolder";
NSString* const kBookmarksMenuFolderIdentifier                 = @"BookmarksMenuFolder";

static NSString* const kTop10BookmarksFolderIdentifier         = @"Top10BookmarksFolder";
static NSString* const kRendezvousFolderIdentifier             = @"RendezvousFolder";   // aka Bonjour
static NSString* const kAddressBookFolderIdentifier            = @"AddressBookFolder";
static NSString* const kHistoryFolderIdentifier                = @"HistoryFolder";

NSString* const kBookmarkImportPathIndentifier                 = @"path";
NSString* const kBookmarkImportNewFolderNameIdentifier         = @"title";

static NSString* const kBookmarkImportStatusIndentifier        = @"flag";
static NSString* const kBookmarkImportNewFolderIdentifier      = @"folder";
static NSString* const kBookmarkImportNewFolderIndexIdentifier = @"index";

// these are suggested indices; we only use them to order the root folders, not to access them.
enum {
  kBookmarkMenuContainerIndex = 0,
  kToolbarContainerIndex = 1,
  kHistoryContainerIndex = 2,
};

@interface BookmarkManager (Private)

+ (NSString*)canonicalBookmarkURL:(NSString*)inBookmarkURL;
+ (NSString*)faviconURLForBookmark:(Bookmark*)inBookmark;

- (void)loadBookmarksThreadEntry:(id)inObject;
- (void)loadBookmarks;
- (void)setBookmarkRoot:(BookmarkFolder *)rootFolder;
- (void)setPathToBookmarkFile:(NSString *)aString;
- (void)setupSmartCollections;
- (void)delayedStartupItems;
- (void)noteBookmarksChanged;
- (void)writeBookmarks:(NSNotification *)note;
- (BookmarkFolder *)findDockMenuFolderInFolder:(BookmarkFolder *)aFolder;

// Writes Spotlight metadata for all bookmarks to disk.
- (void)writeBookmarksMetadataForSpotlight;
// Sets mMetadataPath, and creates it on disk if it doesn't already exist.
- (void)initializeMetadataDirectory;
// Updates the last time of a known complete metadata sync. This should be
// called every time the bookmark file is written.
- (void)updateMetadataSyncTime;
// Returns the path to the file used to timestamp metadata syncs.
- (NSString*)metadataSyncTimestampFile;

// Reading bookmark files
- (BOOL)readBookmarks;
- (void)showCorruptBookmarksAlert;

// Loads (not imports) the given bookmarks file.
- (BOOL)readCaminoBookmarks:(NSString *)pathToFile;

- (BookmarkFolder*)importPropertyListFile:(NSString *)pathToFile;

- (void)importBookmarksThreadReturn:(NSDictionary *)aDict;

+ (void)addItem:(id)inBookmark toURLMap:(NSMutableDictionary*)urlMap usingURL:(NSString*)inURL;
+ (void)removeItem:(id)inBookmark fromURLMap:(NSMutableDictionary*)urlMap usingURL:(NSString*)inURL;  // url may be nil, in which case exhaustive search is used
+ (NSEnumerator*)enumeratorForBookmarksInMap:(NSMutableDictionary*)urlMap matchingURL:(NSString*)inURL;

- (void)registerBookmarkForLoads:(Bookmark*)inBookmark;
- (void)unregisterBookmarkForLoads:(Bookmark*)inBookmark ignoringURL:(BOOL)inIgnoreURL;
- (void)setAndReregisterFaviconURL:(NSString*)inFaviconURL forBookmark:(Bookmark*)inBookmark;
- (void)onSiteIconLoad:(NSNotification *)inNotification;
- (void)onPageLoad:(NSNotification*)inNotification;

@end

#pragma mark -

@implementation BookmarkManager

static NSString* const    kWriteBookmarkNotification = @"write_bms";

static BookmarkManager* gBookmarkManager = nil;

+ (BookmarkManager*)sharedBookmarkManager
{
  if (!gBookmarkManager)
    gBookmarkManager = [[BookmarkManager alloc] init];

  return gBookmarkManager;
}

+ (BookmarkManager*)sharedBookmarkManagerDontCreate
{
  return gBookmarkManager;
}

// serialize to an array of UUIDs
+ (NSArray*)serializableArrayWithBookmarkItems:(NSArray*)bmArray
{
  NSMutableArray* dataArray = [NSMutableArray arrayWithCapacity:[bmArray count]];
  NSEnumerator* bmEnum = [bmArray objectEnumerator];
  id bmItem;
  while ((bmItem = [bmEnum nextObject])) {
    [dataArray addObject:[bmItem UUID]];
  }

  return dataArray;
}

+ (NSArray*)bookmarkItemsFromSerializableArray:(NSArray*)dataArray
{
  NSMutableArray* itemsArray = [NSMutableArray arrayWithCapacity:[dataArray count]];
  NSEnumerator* dataEnum = [dataArray objectEnumerator];
  BookmarkManager* bmManager = [BookmarkManager sharedBookmarkManager];
  id itemUUID;
  while ((itemUUID = [dataEnum nextObject])) {
    BookmarkItem* foundItem = [bmManager itemWithUUID:itemUUID];
    if (foundItem)
      [itemsArray addObject:foundItem];
    else
      NSLog(@"Failed to find bm item with uuid %@", itemUUID);
  }

  return itemsArray;
}

+ (NSArray*)bookmarkURLsFromSerializableArray:(NSArray*)dataArray
{
  NSArray* bookmarkItems = [self bookmarkItemsFromSerializableArray:dataArray];
  NSMutableArray* bookmarkURLs = [NSMutableArray arrayWithCapacity:[bookmarkItems count]];
  NSEnumerator* enumerator = [bookmarkItems objectEnumerator];
  BookmarkItem* aBookmark;
  while ((aBookmark = [enumerator nextObject])) {
    if ([aBookmark isKindOfClass:[Bookmark class]] && ![aBookmark isSeparator])
      [bookmarkURLs addObject:[(Bookmark*)aBookmark url]];
    else if ([aBookmark isKindOfClass:[BookmarkFolder class]])
      [bookmarkURLs addObjectsFromArray:[(BookmarkFolder*)aBookmark childURLs]];
  }
  return bookmarkURLs;
}

// return a string with the "canonical" bookmark url (strip trailing slashes, lowercase)
+ (NSString*)canonicalBookmarkURL:(NSString*)inBookmarkURL
{
  NSString* tempURL = inBookmarkURL;

  if ([tempURL hasSuffix:@"/"])
    tempURL = [tempURL substringToIndex:([tempURL length] - 1)];

  return [tempURL lowercaseString];
}

+ (NSString*)faviconURLForBookmark:(Bookmark*)inBookmark
{
  // if the bookmark has one, use it, otherwise assume the default location
  if ([[inBookmark faviconURL] length] > 0)
    return [inBookmark faviconURL];

  return [SiteIconProvider defaultFaviconLocationStringFromURI:[inBookmark url]];
}

#pragma mark -

//
// Init, dealloc
//
- (id)init
{
  if ((self = [super init])) {
    mBookmarkURLMap         = [[NSMutableDictionary alloc] initWithCapacity:50];
    mBookmarkFaviconURLMap  = [[NSMutableDictionary alloc] initWithCapacity:50];

    mBookmarksLoaded        = NO;
    mShowSiteIcons          = [[PreferenceManager sharedInstance] getBooleanPref:kGeckoPrefEnableFavicons
                                                                     withSuccess:NULL];

    mNotificationsSuppressedLock = [[NSRecursiveLock alloc] init];
  }

  return self;
}

- (void)dealloc
{
  if (gBookmarkManager == self)
    gBookmarkManager = nil;

  [[NSNotificationCenter defaultCenter] removeObserver:self];

  [mTop10Container release];
  [mRendezvousContainer release];
  [mAddressBookContainer release];
  [mLastUsedFolder release];

  [mUndoManager release];
  [mBookmarkRoot release];
  [mPathToBookmarkFile release];
  [mMetadataPath release];
  [mSmartFolderManager release];

  [mImportDlgController release];

  [mBookmarkURLMap release];
  [mBookmarkFaviconURLMap release];

  [mNotificationsSuppressedLock release];

  [super dealloc];
}

- (void)loadBookmarksLoadingSynchronously:(BOOL)loadSync
{
  if (loadSync)
    [self loadBookmarks];
  else
    [NSThread detachNewThreadSelector:@selector(loadBookmarksThreadEntry:) toTarget:self withObject:nil];
}

- (void)loadBookmarksThreadEntry:(id)inObject
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [self loadBookmarks];
  [pool release];
}

// NB: this is called on a thread!
- (void)loadBookmarks
{
  // Turn off the posting of update notifications while reading in bookmarks.
  // All interested parties haven't been init'd yet, and/or will receive the
  // managerStartedNotification when setup is actually complete.
  // Note that it's important that notifications are off while loading bookmarks
  // on this thread, because notifications are handled on the thread they are posted
  // on, and our code assumes that bookmark notifications happen on the main thread.
  [self startSuppressingChangeNotifications];

  // handle exceptions to ensure that turn notification suppression back off
  @try {
    BookmarkFolder* root = [[BookmarkFolder alloc] init];

    // We used to do this:
    // [root setParent:self];
    // but it was unclear why, and it broke logic in a bunch of places (like -setIsRoot).

    [root setIsRoot:YES];
    [root setTitle:NSLocalizedString(@"BookmarksRootName", nil)];
    [self setBookmarkRoot:root];
    [root release];

    BOOL bookmarksReadOK = [self readBookmarks];
    if (!bookmarksReadOK) {
      // We'll come here either when reading the bookmarks totally failed, or
      // when we did a partial read of the bookmark file.
      if ([root count] == 0) {
        // failed to read any folders. make some by hand.
        BookmarkFolder* menuFolder = [[[BookmarkFolder alloc] initWithIdentifier:kBookmarksMenuFolderIdentifier] autorelease];
        [menuFolder setTitle:NSLocalizedString(@"Bookmark Menu", nil)];
        [root appendChild:menuFolder];

        BookmarkFolder* toolbarFolder = [[[BookmarkFolder alloc] initWithIdentifier:kBookmarksToolbarFolderIdentifier] autorelease];
        [toolbarFolder setTitle:NSLocalizedString(@"Bookmark Toolbar", nil)];
        [toolbarFolder setIsToolbar:YES];
        [root appendChild:toolbarFolder];
      }
    }

    // make sure that the root folder has the special flag
    [[self bookmarkRoot] setIsRoot:YES];

    // setup special folders
    [self setupSmartCollections];

    mSmartFolderManager = [[KindaSmartFolderManager alloc] initWithBookmarkManager:self];

    // set the localized titles of these folders
    [[self toolbarFolder] setTitle:NSLocalizedString(@"Bookmark Bar", nil)];
    [[self bookmarkMenuFolder] setTitle:NSLocalizedString(@"Bookmark Menu", nil)];
  }
  @catch (id exception) {
      NSLog(@"Exception caught in loadBookmarks: %@", exception);
  }

  [self stopSuppressingChangeNotifications];

  // don't do this until after we've read in the bookmarks
  mUndoManager = [[NSUndoManager alloc] init];

  // Make sure the spotlight metadata folder exists on disk.
  [self initializeMetadataDirectory];

  // do the other startup stuff over on the main thread
  [self performSelectorOnMainThread:@selector(delayedStartupItems) withObject:nil waitUntilDone:NO];

  // Make sure the bookmark metadata is up to date and complete.
  [self writeBookmarksMetadataForSpotlight];
}


// Perform additional setup items on the main thread.
- (void)delayedStartupItems
{
  mBookmarksLoaded = YES;

  [mSmartFolderManager postStartupInitialization:self];
  [[self toolbarFolder] refreshIcon];

  NSArray* allBookmarks = [[self bookmarkRoot] allChildBookmarks];

  NSEnumerator* bmEnum = [allBookmarks objectEnumerator];
  Bookmark* thisBM;
  while ((thisBM = [bmEnum nextObject])) {
    [self registerBookmarkForLoads:thisBM];
  }

  // load favicons (w/out hitting the network, cache only). Spread it out so that we only get
  // ten every three seconds to avoid locking up the UI with large bookmark lists.
  // XXX probably want a better way to do this. This sets up a timer (internally) for every
  // bookmark
  if ([[PreferenceManager sharedInstance] getBooleanPref:kGeckoPrefEnableFavicons withSuccess:NULL]) {
    float delay = 3.0; //default value
    int count = [allBookmarks count];
    for (int i = 0; i < count; ++i) {
      if (i % 10 == 0)
        delay += 3.0;
      [[allBookmarks objectAtIndex:i] performSelector:@selector(refreshIcon) withObject:nil afterDelay:delay];
    }
  }

  // Generic notifications for Bookmark Client. Don't set these up until after all the smart
  // folders have loaded. Even though we coalesce bookmark update notifications down into a single
  // message, there's no need to write out even once for any of these changes.
  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
  [nc addObserver:self selector:@selector(bookmarkAdded:) name:BookmarkFolderAdditionNotification object:nil];
  [nc addObserver:self selector:@selector(bookmarkRemoved:) name:BookmarkFolderDeletionNotification object:nil];
  [nc addObserver:self selector:@selector(bookmarkChanged:) name:BookmarkItemChangedNotification object:nil];
  [nc addObserver:self selector:@selector(writeBookmarks:) name:kWriteBookmarkNotification object:nil];

  // listen for site icon and page loads, to forward to bookmarks
  [nc addObserver:self selector:@selector(onSiteIconLoad:) name:SiteIconLoadNotificationName object:nil];
  [nc addObserver:self selector:@selector(onPageLoad:) name:URLLoadNotification object:nil];

  // broadcast to everyone interested that we're loaded and ready for public consumption
  [[NSNotificationCenter defaultCenter] postNotificationName:kBookmarkManagerStartedNotification object:nil];
}

- (void)shutdown
{
  [self writeBookmarks:nil];
}

- (BOOL)bookmarksLoaded
{
  return mBookmarksLoaded;
}

- (BOOL)showSiteIcons
{
  return mShowSiteIcons;
}

//
// smart collections, as of now, are Rendezvous, Address Book, Top 10 List.
// We also have history, but that just points to the real history stuff.
- (void)setupSmartCollections
{
  int collectionIndex = 2;  // skip 0 and 1, the menu and toolbar folders

  // XXX this reliance of indices of the root for the special folders is bad; it makes it hard
  // for us to reorder the collections without breaking stuff. Also, there's no checking on
  // reading the file that the Nth folder of the root really is the Toolbar (for example).

  // add history
  BookmarkFolder* historyBMFolder = [[BookmarkFolder alloc] initWithIdentifier:kHistoryFolderIdentifier];
  [historyBMFolder setTitle:NSLocalizedString(@"History", nil)];
  [historyBMFolder setIsSmartFolder:YES];
  [mBookmarkRoot insertChild:historyBMFolder atIndex:(collectionIndex++) isMove:NO];
  [historyBMFolder release];

  // note: we retain smart folders, so they persist even if turned off and on
  mTop10Container = [[BookmarkFolder alloc] initWithIdentifier:kTop10BookmarksFolderIdentifier];
  [mTop10Container setTitle:NSLocalizedString(@"Top Ten List", nil)];
  [mTop10Container setIsSmartFolder:YES];
  [mBookmarkRoot insertChild:mTop10Container atIndex:(collectionIndex++) isMove:NO];

  mRendezvousContainer = [[BookmarkFolder alloc] initWithIdentifier:kRendezvousFolderIdentifier];
  [mRendezvousContainer setTitle:NSLocalizedString(@"Rendezvous", nil)];
  [mRendezvousContainer setIsSmartFolder:YES];
  [mBookmarkRoot insertChild:mRendezvousContainer atIndex:(collectionIndex++) isMove:NO];

  mAddressBookContainer = [[BookmarkFolder alloc] initWithIdentifier:kAddressBookFolderIdentifier];
  [mAddressBookContainer setTitle:NSLocalizedString(@"Address Book", nil)];
  [mAddressBookContainer setIsSmartFolder:YES];
  [mBookmarkRoot insertChild:mAddressBookContainer atIndex:(collectionIndex++) isMove:NO];

  // set pretty icons
  [[self historyFolder]       setIcon:[NSImage imageNamed:@"history_icon"]];
  [[self top10Folder]         setIcon:[NSImage imageNamed:@"top10_icon"]];
  [[self bookmarkMenuFolder]  setIcon:[NSImage imageNamed:@"bookmarkmenu_icon"]];
  [[self toolbarFolder]       setIcon:[NSImage imageNamed:@"bookmarktoolbar_icon"]];
  [[self rendezvousFolder]    setIcon:[NSImage imageNamed:@"rendezvous_icon"]];
  [[self addressBookFolder]   setIcon:[NSImage imageNamed:@"addressbook_icon"]];
}

//
// Getter/Setter methods
//

- (BookmarkFolder *)bookmarkRoot
{
  return mBookmarkRoot;
}

- (BookmarkFolder *)dockMenuFolder
{
  BookmarkFolder *folder = [self findDockMenuFolderInFolder:[self bookmarkRoot]];
  if (folder) {
    return folder;
  }
  else {
    // Set the default in addition to returning it
    BookmarkFolder* defaultDockMenu = [self top10Folder];
    [defaultDockMenu setIsDockMenu:YES];
    return defaultDockMenu;
  }
}

- (BookmarkFolder *)findDockMenuFolderInFolder:(BookmarkFolder *)aFolder
{
  NSEnumerator *enumerator = [[aFolder children] objectEnumerator];
  id aKid;
  BookmarkFolder *foundFolder = nil;
  while ((!foundFolder) && (aKid = [enumerator nextObject])) {
    if ([aKid isKindOfClass:[BookmarkFolder class]]) {
      if ([(BookmarkFolder *)aKid isDockMenu])
        return aKid;
      else
        foundFolder = [self findDockMenuFolderInFolder:aKid];
    }
  }
  return foundFolder;
}

- (BookmarkFolder*)rootBookmarkFolderWithIdentifier:(NSString*)inIdentifier
{
  NSArray* rootFolders = [[self bookmarkRoot] children];
  unsigned int numFolders = [rootFolders count];
  for (unsigned int i = 0; i < numFolders; i++) {
    id curItem = [rootFolders objectAtIndex:i];
    if ([curItem isKindOfClass:[BookmarkFolder class]] && [[curItem identifier] isEqualToString:inIdentifier])
      return (BookmarkFolder*)curItem;
  }
  return nil;
}

- (BOOL)itemsShareCommonParent:(NSArray*)inItems
{
  NSEnumerator* itemsEnum = [inItems objectEnumerator];

  id commonParent = nil;
  BookmarkItem* curItem;
  while ((curItem = [itemsEnum nextObject])) {
    if (curItem == [inItems firstObject]) {
      commonParent = [curItem parent];
      if (!commonParent)
        return NO;
    }

    if ([curItem parent] != commonParent)
      return NO;
  }

  return YES;
}

- (void)startSuppressingChangeNotifications
{
  [mNotificationsSuppressedLock lockBeforeDate:[NSDate distantFuture]];
  ++mNotificationsSuppressedCount;
  [mNotificationsSuppressedLock unlock];
}

- (void)stopSuppressingChangeNotifications
{
  [mNotificationsSuppressedLock lockBeforeDate:[NSDate distantFuture]];
  --mNotificationsSuppressedCount;
  [mNotificationsSuppressedLock unlock];
}

- (BOOL)areChangeNotificationsSuppressed
{
  return (mNotificationsSuppressedCount > 0);
}


- (BookmarkFolder *)top10Folder
{
  return mTop10Container;
}

- (BookmarkFolder *)toolbarFolder
{
  return [self rootBookmarkFolderWithIdentifier:kBookmarksToolbarFolderIdentifier];
}

- (BookmarkFolder *)bookmarkMenuFolder
{
  return [self rootBookmarkFolderWithIdentifier:kBookmarksMenuFolderIdentifier];
}

- (BookmarkFolder *)historyFolder
{
  return [self rootBookmarkFolderWithIdentifier:kHistoryFolderIdentifier];
}

- (BOOL)isUserCollection:(BookmarkFolder *)inFolder
{
  return ([inFolder parent] == mBookmarkRoot) &&
         ([[inFolder identifier] length] == 0);   // all our special folders have identifiers
}

- (BOOL)searchActive
{
  return mSearchActive;
}

- (void)setSearchActive:(BOOL)inSearching
{
  mSearchActive = inSearching;
}

- (unsigned)indexOfContainer:(BookmarkFolder*)inFolder
{
  return [mBookmarkRoot indexOfObject:inFolder];
}

- (BookmarkFolder*)containerAtIndex:(unsigned)inIndex
{
  return [mBookmarkRoot objectAtIndex:inIndex];
}

- (BookmarkFolder *)rendezvousFolder
{
  return mRendezvousContainer;
}

- (BookmarkFolder *)addressBookFolder
{
  return mAddressBookContainer;
}

- (BookmarkFolder*)lastUsedBookmarkFolder
{
  if (!mLastUsedFolder)
    return [self toolbarFolder];

  return mLastUsedFolder;
}

- (void)setLastUsedBookmarkFolder:(BookmarkFolder*)inFolder
{
  [mLastUsedFolder autorelease];
  mLastUsedFolder = [inFolder retain];
}

- (BookmarkItem*)itemWithUUID:(NSString*)uuid
{
  return [mBookmarkRoot itemWithUUID:uuid];
}

// only the main thread can get the undo manager.
// imports (on a background thread) get nothing, which is ok.
// this keeps things nice and thread safe
- (NSUndoManager *)undoManager
{
  if ([NSThread inMainThread])
    return mUndoManager;
  return nil;
}

- (void)setPathToBookmarkFile:(NSString *)aString
{
  [aString retain];
  [mPathToBookmarkFile release];
  mPathToBookmarkFile = aString;
}

- (void)setBookmarkRoot:(BookmarkFolder *)rootFolder
{
  if (rootFolder != mBookmarkRoot) {
    [rootFolder retain];
    [mBookmarkRoot release];
    mBookmarkRoot = rootFolder;
  }
}

//
// -clearAllVisits:
//
// resets all bookmarks visit counts to zero as part of Reset Camino
//

- (void)clearAllVisits
{
  // XXX this will fire a lot of changed notifications.
  NSEnumerator* bookmarksEnum = [[self bookmarkRoot] objectEnumerator];
  BookmarkItem* curItem;
  while ((curItem = [bookmarksEnum nextObject])) {
    if ([curItem isKindOfClass:[Bookmark class]])
      [(Bookmark*)curItem setNumberOfVisits:0];
  }
}


- (NSArray *)resolveBookmarksShortcut:(NSString *)shortcut
{
  NSArray *resolvedArray = nil;
  // Remove any leading or trailing whitespace since we can't trust input
  // and we don't support spaces as part of shortcuts anyway
  shortcut = [shortcut stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
  if ([shortcut length] > 0) {
    NSRange spaceRange = [shortcut rangeOfString:@" "];
    NSString *firstWord = nil;
    NSString *secondWord = nil;
    if (spaceRange.location != NSNotFound) {
      firstWord = [shortcut substringToIndex:spaceRange.location];
      secondWord = [shortcut substringFromIndex:(spaceRange.location + spaceRange.length)];
    }
    else {
      firstWord = shortcut;
      secondWord = @"";
    }
    resolvedArray = [[self bookmarkRoot] resolveShortcut:firstWord withArgs:secondWord];
  }
  return resolvedArray;
}

// a null container indicates to search all bookmarks
- (NSArray *)searchBookmarksContainer:(BookmarkFolder*)container forString:(NSString *)searchString inFieldWithTag:(int)tag
{
  if ((searchString) && [searchString length] > 0) {
    BookmarkFolder* searchContainer = container ? container : [self bookmarkRoot];
    return [searchContainer bookmarksWithString:searchString inFieldWithTag:tag];
  }
  return nil;
}

//
// Drag & drop
//

- (BOOL)isDropValid:(NSArray *)items toFolder:(BookmarkFolder *)parent
{
  // Enumerate through items, make sure we're not being dropped into
  // a child OR ourself OR that the a bookmark or group is going into root bookmarks.
  NSEnumerator *enumerator = [items objectEnumerator];
  id aBookmark;
  while ((aBookmark = [enumerator nextObject])) {
    if ([aBookmark isKindOfClass:[BookmarkFolder class]]) {
      if (aBookmark == parent)
        return NO;
      if ((parent == [self bookmarkRoot]) && [(BookmarkFolder *)aBookmark isGroup])
        return NO;
    }
    else if ([aBookmark isKindOfClass:[Bookmark class]]) {
      if (parent == [self bookmarkRoot])
        return NO;
    }
    if ([parent isChildOfItem:aBookmark])
      return NO;
  }
  return YES;
}

// unified context menu generator for all kinds of bookmarks
// this can be called from a bookmark outline view
// or from a bookmark button, which should pass a nil outlineView
- (NSMenu *)contextMenuForItems:(NSArray*)items fromView:(BookmarkOutlineView *)outlineView target:(id)target
{
  if ([items count] == 0)
    return nil;

  BOOL itemsContainsFolder = NO;
  BOOL itemsContainsBookmark = NO;
  BOOL itemsAllSeparators = YES;
  BOOL multipleItems = ([items count] > 1);

  NSEnumerator* itemsEnum = [items objectEnumerator];
  id curItem;
  while ((curItem = [itemsEnum nextObject])) {
    itemsContainsFolder   |= [curItem isKindOfClass:[BookmarkFolder class]];
    itemsContainsBookmark |= [curItem isKindOfClass:[Bookmark class]];
    itemsAllSeparators    &= [curItem isSeparator];
  }

  // All the methods in this context menu need to be able to handle > 1 item
  // being selected, and the selected items containing a mixture of folders,
  // bookmarks, and separators.
  NSMenu* contextMenu = [[[NSMenu alloc] initWithTitle:@"notitle"] autorelease];
  NSString* menuTitle = nil;
  NSMenuItem* menuItem = nil;
  NSMenuItem* shiftMenuItem = nil;

  // Selections with only separators shouldn't have these CM items at all.
  // We rely on the called selectors to do the Right Thing(tm) with embedded separators.
  if (!itemsAllSeparators) {
    // open in new window(s)
    if (itemsContainsFolder && !multipleItems)
      menuTitle = NSLocalizedString(@"Open Tabs in New Window", nil);
    else if (multipleItems)
      menuTitle = NSLocalizedString(@"Open in New Windows", nil);
    else
      menuTitle = NSLocalizedString(@"Open in New Window", nil);

    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(openBookmarkInNewWindow:) keyEquivalent:@""] autorelease];
    [menuItem setTarget:target];
    [menuItem setKeyEquivalentModifierMask:0]; //Needed since by default NSMenuItems have NSCommandKeyMask
    [contextMenu addItem:menuItem];

    shiftMenuItem = [NSMenuItem alternateMenuItemWithTitle:menuTitle action:@selector(openBookmarkInNewWindow:) target:target modifiers:NSShiftKeyMask];
    [contextMenu addItem:shiftMenuItem];

    // open in new tabs in new window
    if (multipleItems) {
      menuTitle = NSLocalizedString(@"Open in Tabs in New Window", nil);

      menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(openBookmarksInTabsInNewWindow:) keyEquivalent:@""] autorelease];
      [menuItem setKeyEquivalentModifierMask:0];
      [menuItem setTarget:target];
      [contextMenu addItem:menuItem];

      shiftMenuItem = [NSMenuItem alternateMenuItemWithTitle:menuTitle action:@selector(openBookmarksInTabsInNewWindow:) target:target modifiers:NSShiftKeyMask];
      [contextMenu addItem:shiftMenuItem];
    }

    // open in new tab in current window
    if (itemsContainsFolder || multipleItems)
      menuTitle = NSLocalizedString(@"Open in New Tabs", nil);
    else
      menuTitle = NSLocalizedString(@"Open in New Tab", nil);

    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(openBookmarkInNewTab:) keyEquivalent:@""] autorelease];
    [menuItem setKeyEquivalentModifierMask:0];
    [menuItem setTarget:target];
    [contextMenu addItem:menuItem];

    shiftMenuItem = [NSMenuItem alternateMenuItemWithTitle:menuTitle action:@selector(openBookmarkInNewTab:) target:target modifiers:NSShiftKeyMask];
    [contextMenu addItem:shiftMenuItem];
  }

  BookmarkFolder* collection = [target isKindOfClass:[BookmarkViewController class]] ? [target activeCollection] : nil;
  // We only want a "Reveal" menu item if the CM is on a BookmarkButton,
  // if the user is searching somewhere other than the History folder,
  // or if the Top 10 is the active collection.
  if ((!outlineView) ||
      (!multipleItems && (([self searchActive] && !(collection == [self historyFolder])) ||
                          (collection == [self top10Folder]))))
  {
    menuTitle = NSLocalizedString(@"Reveal in Bookmark Manager", nil);
    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(revealBookmark:) keyEquivalent:@""] autorelease];
    [menuItem setTarget:target];
    [contextMenu addItem:menuItem];
  }

  if (!itemsAllSeparators) {
    [contextMenu addItem:[NSMenuItem separatorItem]];

    if (!outlineView || !multipleItems) {
      menuTitle = NSLocalizedString(@"Bookmark Info", nil);
      menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(showBookmarkInfo:) keyEquivalent:@""] autorelease];
      [menuItem setTarget:target];
      [contextMenu addItem:menuItem];
    }
  }

  // copy URL(s) to clipboard
  // This makes no sense for separators, which have no URL.
  // We rely on |copyURLs:| to handle the selector-embedded-in-multiple-items case.
  if (!itemsAllSeparators) {
    if (itemsContainsFolder || multipleItems)
      menuTitle = NSLocalizedString(@"Copy URLs to Clipboard", nil);
    else
      menuTitle = NSLocalizedString(@"Copy URL to Clipboard", nil);

    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(copyURLs:) keyEquivalent:@""] autorelease];
    [menuItem setTarget:target];
    [contextMenu addItem:menuItem];
  }

  if (!multipleItems && itemsContainsFolder) {
    menuTitle = NSLocalizedString(@"Use as Dock Menu", nil);
    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(toggleIsDockMenu:) keyEquivalent:@""] autorelease];
    [menuItem setTarget:[items objectAtIndex:0]];
    if ([(BookmarkFolder*)[items objectAtIndex:0] isDockMenu])
      [menuItem setState:NSOnState];
    [contextMenu addItem:menuItem];
  }

  BOOL allowNewFolder = NO;
  if ([target isKindOfClass:[BookmarkViewController class]])
    allowNewFolder = ![[target activeCollection] isSmartFolder];

  // if we're not in a smart collection (other than history)
  if (!outlineView ||
      ![[target activeCollection] isSmartFolder] ||
      ([target activeCollection] == [self historyFolder]))
  {
    if ([contextMenu numberOfItems] != 0)
      // only add a separator if it won't be the first item in the menu
      [contextMenu addItem:[NSMenuItem separatorItem]];

    // delete
    menuTitle = NSLocalizedString(@"Delete", nil);
    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(deleteBookmarks:) keyEquivalent:@""] autorelease];
    [menuItem setTarget:target];
    [contextMenu addItem:menuItem];
  }

  if (allowNewFolder) {
    // space
    [contextMenu addItem:[NSMenuItem separatorItem]];
    // create new folder
    menuTitle = NSLocalizedString(@"Create New Folder...", nil);
    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:@selector(addBookmarkFolder:) keyEquivalent:@""] autorelease];
    [menuItem setTarget:target];
    [contextMenu addItem:menuItem];
  }

  // Arrange selections of multiple bookmark items or folders.
  // These may get removed again by the caller, so we tag them.
  if ([target isKindOfClass:[BookmarkViewController class]] &&
      ![[target activeCollection] isSmartFolder] &&
      (multipleItems || itemsContainsFolder) &&
      !itemsAllSeparators)
  {
    NSMenuItem* separatorItem = [NSMenuItem separatorItem];
    [separatorItem setTag:kBookmarksContextMenuArrangeSeparatorTag];
    [contextMenu addItem:separatorItem];

    menuTitle = NSLocalizedString(@"Arrange Bookmarks", nil);
    menuItem = [[[NSMenuItem alloc] initWithTitle:menuTitle action:NULL keyEquivalent:@""] autorelease];
    [menuItem setTarget:target];
    [contextMenu addItem:menuItem];

    // create submenu
    NSMenu* arrangeSubmenu = [[[NSMenu alloc] initWithTitle:@"notitle"] autorelease];

    NSMenuItem* subMenuItem = [[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Arrange Increasing by title", nil)
                                           action:@selector(arrange:)
                                    keyEquivalent:@""] autorelease];
    [subMenuItem setTarget:target];
    [subMenuItem setTag:(kArrangeBookmarksByTitleMask | kArrangeBookmarksAscendingMask)];
    [arrangeSubmenu addItem:subMenuItem];

    subMenuItem = [[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Arrange Decreasing by title", nil)
                                           action:@selector(arrange:)
                                    keyEquivalent:@""] autorelease];
    [subMenuItem setTarget:target];
    [subMenuItem setTag:(kArrangeBookmarksByTitleMask | kArrangeBookmarksDescendingMask)];
    [arrangeSubmenu addItem:subMenuItem];

    [arrangeSubmenu addItem:[NSMenuItem separatorItem]];

    subMenuItem = [[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Arrange Increasing by location", nil)
                                           action:@selector(arrange:)
                                    keyEquivalent:@""] autorelease];
    [subMenuItem setTarget:target];
    [subMenuItem setTag:(kArrangeBookmarksByLocationMask | kArrangeBookmarksAscendingMask)];
    [arrangeSubmenu addItem:subMenuItem];

    subMenuItem = [[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Arrange Decreasing by location", nil)
                                           action:@selector(arrange:)
                                    keyEquivalent:@""] autorelease];
    [subMenuItem setTarget:target];
    [subMenuItem setTag:(kArrangeBookmarksByLocationMask | kArrangeBookmarksDescendingMask)];
    [arrangeSubmenu addItem:subMenuItem];

    [contextMenu setSubmenu:arrangeSubmenu forItem:menuItem];
  }

  // Disable context menu items if the parent window is currently showing a sheet.
  if ((outlineView && [[outlineView window] attachedSheet]) ||
      (target && [target respondsToSelector:@selector(window)] && [[target window] attachedSheet]))
  {
    NSArray* menuArray = [contextMenu itemArray];
    for (unsigned i = 0; i < [menuArray count]; i++) {
      [[menuArray objectAtIndex:i] setEnabled:NO];
    }
  }

  return contextMenu;
}

//
// Copy a set of bookmarks URLs to the specified pasteboard.
// We don't care about item titles here, nor do we care about format.
// Separators have no URL and are ignored.
//
- (void)copyBookmarksURLs:(NSArray*)bookmarkItems toPasteboard:(NSPasteboard*)aPasteboard
{
  // handle URLs, and nothing else, for simplicity.
  [aPasteboard declareTypes:[NSArray arrayWithObject:kCorePasteboardFlavorType_url] owner:nil];

  NSMutableArray* urlList = [NSMutableArray array];
  NSMutableSet* seenBookmarks = [NSMutableSet setWithCapacity:[bookmarkItems count]];
  NSEnumerator* bookmarkItemsEnum = [bookmarkItems objectEnumerator];
  BookmarkItem* curItem;
  while ((curItem = [bookmarkItemsEnum nextObject])) {
    if ([curItem isKindOfClass:[Bookmark class]] && ![curItem isSeparator] && ![seenBookmarks containsObject:curItem]) {
      [seenBookmarks addObject:curItem]; // now we've seen it
      [urlList addObject:[(Bookmark*)curItem url]];
    }
    else if ([curItem isKindOfClass:[BookmarkFolder class]]) {
      // get all child bookmarks in a nice flattened array
      NSArray* children = [(BookmarkFolder*)curItem allChildBookmarks];
      NSEnumerator* childrenEnum = [children objectEnumerator];
      Bookmark* curChild;
      while ((curChild = [childrenEnum nextObject])) {
        if (![seenBookmarks containsObject:curChild] && ![curItem isSeparator]) {
          [seenBookmarks addObject:curChild]; // now we've seen it
          [urlList addObject:[curChild url]];
        }
      }
    }
  }
  [aPasteboard setURLs:urlList withTitles:nil];
}


#pragma mark -

//
// Methods relating to the multiplexing of page load and site icon load notifications
//

+ (void)addItem:(id)inBookmark toURLMap:(NSMutableDictionary*)urlMap usingURL:(NSString*)inURL
{
  NSMutableSet* urlSet = [urlMap objectForKey:inURL];
  if (!urlSet) {
    urlSet = [[NSMutableSet alloc] initWithCapacity:1];
    [urlMap setObject:urlSet forKey:inURL];
    [urlSet release];
  }
  [urlSet addObject:inBookmark];
}

// url may be nil, in which case exhaustive search is used
+ (void)removeItem:(id)inBookmark fromURLMap:(NSMutableDictionary*)urlMap usingURL:(NSString*)inURL
{
  if (inURL) {
    NSMutableSet* urlSet = [urlMap objectForKey:inURL];
    if (urlSet)
      [urlSet removeObject:inBookmark];
  }
  else {
    NSEnumerator* urlMapEnum = [urlMap objectEnumerator];
    NSMutableSet* curSet;
    while ((curSet = [urlMapEnum nextObject])) {
      if ([curSet containsObject:inBookmark]) {
        [curSet removeObject:inBookmark];
        break;   // it should only be in one set
      }
    }
  }
}

// unregister the bookmark using its old favicon url, set the new one (which might be nil),
// and reregister (setting a nil favicon url makes it use the default)
- (void)setAndReregisterFaviconURL:(NSString*)inFaviconURL forBookmark:(Bookmark*)inBookmark
{
  [BookmarkManager removeItem:inBookmark fromURLMap:mBookmarkFaviconURLMap usingURL:[BookmarkManager faviconURLForBookmark:inBookmark]];
  [inBookmark setFaviconURL:inFaviconURL];
  [BookmarkManager addItem:inBookmark toURLMap:mBookmarkFaviconURLMap usingURL:[BookmarkManager faviconURLForBookmark:inBookmark]];
}


+ (NSEnumerator*)enumeratorForBookmarksInMap:(NSMutableDictionary*)urlMap matchingURL:(NSString*)inURL
{
  return [[urlMap objectForKey:inURL] objectEnumerator];
}

- (void)registerBookmarkForLoads:(Bookmark*)inBookmark
{
  NSString* bookmarkURL = [BookmarkManager canonicalBookmarkURL:[inBookmark url]];

  // add to the bookmark url map
  [BookmarkManager addItem:inBookmark toURLMap:mBookmarkURLMap usingURL:bookmarkURL];

  // and add it to the site icon map
  NSString* faviconURL = [BookmarkManager faviconURLForBookmark:inBookmark];
  if ([faviconURL length] > 0)
    [BookmarkManager addItem:inBookmark toURLMap:mBookmarkFaviconURLMap usingURL:faviconURL];
}

- (void)unregisterBookmarkForLoads:(Bookmark*)inBookmark ignoringURL:(BOOL)inIgnoreURL
{
  NSString* bookmarkURL = inIgnoreURL ? nil : [BookmarkManager canonicalBookmarkURL:[inBookmark url]];
  [BookmarkManager removeItem:inBookmark fromURLMap:mBookmarkURLMap usingURL:bookmarkURL];

  NSString* faviconURL = [BookmarkManager faviconURLForBookmark:inBookmark];
  if ([faviconURL length] > 0)
    [BookmarkManager removeItem:inBookmark fromURLMap:mBookmarkFaviconURLMap usingURL:faviconURL];
}


- (void)onSiteIconLoad:(NSNotification *)inNotification
{
  NSDictionary* userInfo = [inNotification userInfo];
  //NSLog(@"onSiteIconLoad %@", inNotification);
  if (!userInfo)
    return;

  NSImage*  iconImage    = [userInfo objectForKey:SiteIconLoadImageKey];
  NSString* siteIconURI  = [userInfo objectForKey:SiteIconLoadURIKey];
  NSString* pageURI      = [userInfo objectForKey:SiteIconLoadUserDataKey];
  pageURI = [BookmarkManager canonicalBookmarkURL:pageURI];

  BOOL isDefaultSiteIconLocation = [siteIconURI isEqualToString:[SiteIconProvider defaultFaviconLocationStringFromURI:pageURI]];

  if (iconImage) {
    Bookmark* curBookmark;

    // look for bookmarks to this page. we might not have registered
    // this bookmark for a custom <link> favicon url yet
    NSArray* bookmarksForPage = [[mBookmarkURLMap objectForKey:pageURI] allObjects];
    NSEnumerator* bookmarksForPageEnum = [bookmarksForPage objectEnumerator];
    // note that we don't enumerate over the NSMutableSet directly, because we'll be
    // changing it inside the loop
    while ((curBookmark = [bookmarksForPageEnum nextObject])) {
      if (isDefaultSiteIconLocation) {
        // if we've got one from the default location, but the bookmark has a custom linked icon,
        // so remove the custom link
        if ([[curBookmark faviconURL] length] > 0)
          [self setAndReregisterFaviconURL:nil forBookmark:curBookmark];
      }
      else {  // custom location
        if (![[curBookmark faviconURL] isEqualToString:siteIconURI])
          [self setAndReregisterFaviconURL:siteIconURI forBookmark:curBookmark];
      }
    }

    // update bookmarks known to be using this favicon url
    NSEnumerator* bookmarksEnum = [BookmarkManager enumeratorForBookmarksInMap:mBookmarkFaviconURLMap matchingURL:siteIconURI];
    while ((curBookmark = [bookmarksEnum nextObject])) {
      [curBookmark setIcon:iconImage];
    }
  }
  else {
    // we got no image. If this was a network load for a custom favicon url, clear the favicon url from the bookmarks which use it
    BOOL networkLoad = [[userInfo objectForKey:SiteIconLoadUsedNetworkKey] boolValue];
    if (networkLoad && !isDefaultSiteIconLocation) {
      NSArray* bookmarksForPage = [[mBookmarkURLMap objectForKey:pageURI] allObjects];
      NSEnumerator* bookmarksForPageEnum = [bookmarksForPage objectEnumerator];
      // note that we don't enumerate over the NSMutableSet directly, because we'll be
      // changing it inside the loop
      Bookmark* curBookmark;
      while ((curBookmark = [bookmarksForPageEnum nextObject])) {
        // clear any custom favicon urls
        if ([[curBookmark faviconURL] isEqualToString:siteIconURI])
          [self setAndReregisterFaviconURL:nil forBookmark:curBookmark];
      }
    }
  }
}

- (void)onPageLoad:(NSNotification*)inNotification
{
  NSString* loadURL = [BookmarkManager canonicalBookmarkURL:[inNotification object]];
  BOOL successfullLoad = [[[inNotification userInfo] objectForKey:URLLoadSuccessKey] boolValue];

  NSEnumerator* bookmarksEnum = [BookmarkManager enumeratorForBookmarksInMap:mBookmarkURLMap matchingURL:loadURL];
  Bookmark* curBookmark;
  while ((curBookmark = [bookmarksEnum nextObject])) {
    [curBookmark notePageLoadedWithSuccess:successfullLoad];
  }
}

#pragma mark -

//
// BookmarkClient protocol - so we know when to write out
//
- (void)bookmarkAdded:(NSNotification *)inNotification
{
  // we only care about additions to non-smart folders.
  BookmarkItem* bmItem = [[inNotification userInfo] objectForKey:BookmarkFolderChildKey];
  BookmarkFolder* parentFolder = [inNotification object];

  if ([parentFolder isSmartFolder])
    return;

  if ([bmItem isKindOfClass:[Bookmark class]]) {
    [bmItem writeBookmarksMetadataToPath:mMetadataPath];

    [self registerBookmarkForLoads:(Bookmark*)bmItem];
  }

  [self noteBookmarksChanged];
}

- (void)bookmarkRemoved:(NSNotification *)inNotification
{
  BookmarkItem* bmItem = [[inNotification userInfo] objectForKey:BookmarkFolderChildKey];

  if ([bmItem isKindOfClass:[BookmarkFolder class]]) {
    if ([(BookmarkFolder*)bmItem containsChildItem:mLastUsedFolder]) {
      [mLastUsedFolder release];
      mLastUsedFolder = nil;
    }
  }

  BookmarkFolder* parentFolder = [inNotification object];
  if ([parentFolder isSmartFolder])
    return;

  if ([bmItem isKindOfClass:[Bookmark class]]) {
    [bmItem removeBookmarksMetadataFromPath:mMetadataPath];

    [self unregisterBookmarkForLoads:(Bookmark*)bmItem ignoringURL:YES];
  }

  [self noteBookmarksChanged];
}

- (void)bookmarkChanged:(NSNotification *)inNotification
{
  id item = [inNotification object];

  // don't write out the bookmark file or metadata for changes in a smart container.
  // we should really check to see that the bookmarks is in the tree, rather than
  // just checking its parent
  if (![item parent] || [(BookmarkFolder *)[item parent] isSmartFolder])
    return;

  unsigned int changeFlags = kBookmarkItemEverythingChangedMask;
  NSNumber* noteChangeFlags = [[inNotification userInfo] objectForKey:BookmarkItemChangedFlagsKey];
  if (noteChangeFlags)
    changeFlags = [noteChangeFlags unsignedIntValue];

  if ([item isKindOfClass:[Bookmark class]]) {
    // update Spotlight metadata
    if (changeFlags & kBookmarkItemSignificantChangeFlagsMask)
      [item writeBookmarksMetadataToPath:mMetadataPath];

    // and re-register in the maps if the url changed
    if (changeFlags & kBookmarkItemURLChangedMask) {
      // since we've lost the old url, we have to unregister the slow way
      [self unregisterBookmarkForLoads:item ignoringURL:YES];
      [self registerBookmarkForLoads:item];
    }
  }

  if (changeFlags & kBookmarkItemSignificantChangeFlagsMask)
    [self noteBookmarksChanged];
}

- (void)noteBookmarksChanged
{
  // post a coalescing notification to write the bookmarks file
  NSNotification *note = [NSNotification notificationWithName:kWriteBookmarkNotification object:self userInfo:nil];
  [[NSNotificationQueue defaultQueue] enqueueNotification:note
                                             postingStyle:NSPostASAP
                                             coalesceMask:NSNotificationCoalescingOnName
                                                 forModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

- (void)writeBookmarks:(NSNotification *)inNotification
{
  if (!mBookmarksLoaded)
    return;
  [self writePropertyListFile:mPathToBookmarkFile];
}

#pragma mark -

//
// -writeBookmarksMetadataForSpotlight
//
// Write out a flat list of all bookmarks in the caches folder so that Spotlight
// can parse them, unless bookmark file hasn't been modified since the last
// complete sync. Since metadata is maintained incrementally, this should only
// do significant work on the first launch after the cache is manually deleted
// or the bookmarks file is changed manually.
//
// Note that this is called on a thread, so it takes pains to ensure that the
// data it's working with won't be changing on the UI thread
//
- (void)writeBookmarksMetadataForSpotlight
{
  // Check whether a full refresh is necessary.
  NSFileManager* fileManager = [NSFileManager defaultManager];
  NSString* metadataMarkerPath = [self metadataSyncTimestampFile];
  NSDate* metadataDate = [[fileManager fileAttributesAtPath:metadataMarkerPath
                            traverseLink:YES]
                          objectForKey:NSFileModificationDate];
  NSDate* bookmarkDate = [[fileManager fileAttributesAtPath:mPathToBookmarkFile
                            traverseLink:YES]
                          objectForKey:NSFileModificationDate];
  if (metadataDate && bookmarkDate && [bookmarkDate isEqualToDate:metadataDate])
    return;

  // If we quit while this thread is still running, we'll end up with incomplete
  // metadata on disk, but it will get rebuilt on the next launch.
  mWritingSpotlightMetadata = YES;

  NSArray* allBookmarkItems = [mBookmarkRoot allChildBookmarks];

  // delete any existing contents
  NSEnumerator* dirContentsEnum =
      [[fileManager directoryContentsAtPath:mMetadataPath] objectEnumerator];
  NSString* curFile;
  while ((curFile = [dirContentsEnum nextObject])) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    NSString* curFilePath = [mMetadataPath stringByAppendingPathComponent:curFile];
    [fileManager removeFileAtPath:curFilePath handler:nil];

    [pool release];
  }

  unsigned int itemCount = 0;
  NSEnumerator* bmEnumerator = [allBookmarkItems objectEnumerator];
  BookmarkItem* curItem;
  while ((curItem = [bmEnumerator nextObject])) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    [curItem writeBookmarksMetadataToPath:mMetadataPath];

    if (!(++itemCount % 100) && ![NSThread inMainThread])
      usleep(10000);    // 10ms to give the UI some time

    [pool release];
  }
  mWritingSpotlightMetadata = NO;
}

- (void)initializeMetadataDirectory
{
  if (mMetadataPath)
    return;

  NSFileManager* fileManager = [NSFileManager defaultManager];
  NSString* metadataRoot =
      [@"~/Library/Caches/Metadata" stringByExpandingTildeInPath];
  [fileManager createDirectoryAtPath:metadataRoot attributes:nil];
  mMetadataPath =
      [[metadataRoot stringByAppendingPathComponent:@"Camino"] retain];
  [fileManager createDirectoryAtPath:mMetadataPath attributes:nil];
}

- (void)updateMetadataSyncTime
{
  // If the initial write hasn't finished, don't update the timestamp file.
  if (mWritingSpotlightMetadata)
    return;

  NSFileManager* fileManager = [NSFileManager defaultManager];
  NSString* filePath = [self metadataSyncTimestampFile];
  [fileManager createFileAtPath:filePath
                       contents:[NSData data]
                     attributes:nil];

  // Sync the timestamp to the bookmark file timestamp. Having them match
  // exactly makes it possible to correctly handle edge cases like switching
  // profiles or restoring a bookmark file from backup.
  NSDate* metadataDate = [[fileManager fileAttributesAtPath:mPathToBookmarkFile
                                               traverseLink:YES]
                          objectForKey:NSFileModificationDate];
  if (metadataDate) {
    NSDictionary* modificationAttribute =
        [NSDictionary dictionaryWithObject:metadataDate
                                    forKey:NSFileModificationDate];
    [fileManager changeFileAttributes:modificationAttribute atPath:filePath];
  }
}

- (NSString*)metadataSyncTimestampFile {
  return [mMetadataPath stringByAppendingPathComponent:@".LastSyncTimestamp"];
}

#pragma mark -

//
// Reading/Importing bookmark files
//
- (BOOL)readBookmarks
{
  // figure out where Bookmarks.plist is and store it as mPathToBookmarkFile
  // if there is a Bookmarks.plist, read it
  // if there isn't (or it's corrupt), but there's a backup, restore from the backup
  // otherwise, move default Bookmarks.plist to profile dir & read it.
  NSString *profileDir = [[PreferenceManager sharedInstance] profilePath];
  NSString *bookmarkPath = [profileDir stringByAppendingPathComponent:@"bookmarks.plist"];
  NSString *backupPath = [bookmarkPath stringByAppendingString:@".bak"];
  [self setPathToBookmarkFile:bookmarkPath];

  NSFileManager *fM = [NSFileManager defaultManager];

  // If the bookmark file is somehow missing, grab the backup if there is one.
  if (![fM fileExistsAtPath:bookmarkPath] && [fM fileExistsAtPath:backupPath])
    [fM copyPath:backupPath toPath:bookmarkPath handler:self];

  BOOL bookmarksAreCorrupt = NO;
  if ([fM isReadableFileAtPath:bookmarkPath]) {
    if ([self readCaminoBookmarks:bookmarkPath]) {
      // since the bookmarks look good, save them aside as a backup in case something goes
      // wrong later (e.g., bug 337750) since users really don't like losing their bookmarks.
      if ([fM fileExistsAtPath:backupPath])
        [fM removeFileAtPath:backupPath handler:self];
      [fM copyPath:bookmarkPath toPath:backupPath handler:self];

      return YES;
    }
    else {
      bookmarksAreCorrupt = YES;
      // save the corrupted bookmarks to a backup file
      NSString* uniqueName = [fM backupFileNameFromPath:bookmarkPath withSuffix:@"-corrupted"];
      if ([fM movePath:bookmarkPath toPath:uniqueName handler:nil])
        NSLog(@"Moved corrupted bookmarks file to '%@'", uniqueName);
      else
        NSLog(@"Failed to move corrupted bookmarks file to '%@'", uniqueName);

      // Try to recover from the backup, if there is one
      if ([fM fileExistsAtPath:backupPath]) {
        if ([self readCaminoBookmarks:backupPath]) {
          NSLog(@"Recovering from backup bookmarks file '%@'", backupPath);

          [fM copyPath:backupPath toPath:bookmarkPath handler:self];
          return YES;
        }
      }
    }
  }

  // if we're here, we have either no bookmarks or corrupted bookmarks with no backup; either way,
  // install the default plist so the bookmarks aren't totally empty.
  NSString *defaultBookmarks = [[NSBundle mainBundle] pathForResource:@"bookmarks" ofType:@"plist"];
  if ([fM copyPath:defaultBookmarks toPath:bookmarkPath handler:nil]) {
    if ([self readCaminoBookmarks:bookmarkPath] && !bookmarksAreCorrupt)
      return YES;
  }

  // if we're here, we've had a problem.
  // This is a background thread, so we can't put up an alert directly.
  [self performSelectorOnMainThread:@selector(showCorruptBookmarksAlert) withObject:nil waitUntilDone:NO];

  return NO;
}

- (void)showCorruptBookmarksAlert
{
  NSRunAlertPanel(NSLocalizedString(@"CorruptedBookmarksAlert", nil),
                  NSLocalizedString(@"CorruptedBookmarksMsg", nil),
                  NSLocalizedString(@"OKButtonText", nil),
                  nil,
                  nil);
}

- (BOOL)readCaminoBookmarks:(NSString *)pathToFile
{
  NSDictionary* plist = [NSDictionary dictionaryWithContentsOfFile:pathToFile];
  if (!plist)
    return NO;
  if (![[self bookmarkRoot] readNativeDictionary:plist])
    return NO;

  // find the menu and toolbar folders
  BookmarkFolder* menuFolder = nil;
  BookmarkFolder* toolbarFolder = nil;

  NSEnumerator* rootFoldersEnum = [[[self bookmarkRoot] children] objectEnumerator];
  id curChild;
  while ((curChild = [rootFoldersEnum nextObject])) {
    if ([curChild isKindOfClass:[BookmarkFolder class]]) {
      BookmarkFolder* bmFolder = (BookmarkFolder*)curChild;
      if ([bmFolder isToolbar]) {
        toolbarFolder = bmFolder; // remember that we've seen it
        [bmFolder setIdentifier:kBookmarksToolbarFolderIdentifier];
      }
      else if (!menuFolder) {
        menuFolder = bmFolder;
        [bmFolder setIdentifier:kBookmarksMenuFolderIdentifier];
      }

      if (toolbarFolder && menuFolder)
        break;
    }
  }

  if (!menuFolder) {
    menuFolder = [[[BookmarkFolder alloc] initWithIdentifier:kBookmarksMenuFolderIdentifier] autorelease];
    [menuFolder setTitle:NSLocalizedString(@"Bookmark Menu", nil)];
    [[self bookmarkRoot] insertChild:menuFolder atIndex:kBookmarkMenuContainerIndex isMove:NO];
  }

  if (!toolbarFolder) {
    toolbarFolder = [[[BookmarkFolder alloc] initWithIdentifier:kBookmarksToolbarFolderIdentifier] autorelease];
    [toolbarFolder setTitle:NSLocalizedString(@"Bookmark Toolbar", nil)];
    [toolbarFolder setIsToolbar:YES];
    [[self bookmarkRoot] insertChild:toolbarFolder atIndex:kToolbarContainerIndex isMove:NO];
  }

  return YES;
}

- (void)startImportBookmarks
{
  if (!mImportDlgController)
    mImportDlgController = [[BookmarkImportDlgController alloc] initWithWindowNibName:@"BookmarkImportDlg"];

  [mImportDlgController buildAvailableFileList];
  [mImportDlgController showWindow:nil];
}

- (void)importBookmarksThreadEntry:(NSDictionary *)aDict
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  BOOL success = YES;
  int currentFile = 0;
  NSArray *pathArray = [aDict objectForKey:kBookmarkImportPathIndentifier];
  NSArray *titleArray = [aDict objectForKey:kBookmarkImportNewFolderNameIdentifier];
  BookmarkFolder *topImportFolder = nil;

  NSEnumerator *pathEnumerator = [pathArray objectEnumerator];
  NSEnumerator *titleEnumerator = [titleArray objectEnumerator];

  [self startSuppressingChangeNotifications];

  NSString *pathToFile;
  while ((pathToFile = [pathEnumerator nextObject])) {
    BookmarkFolder *importFolder = nil;

    NSString *extension = [[pathToFile pathExtension] lowercaseString];
    // Older Opera bookmarks have no extension, so guess that for extensionless
    // files.
    if ([extension isEqualToString:@"adr"] || [extension isEqualToString:@""]) {
      importFolder = [[OperaBookmarkConverter operaBookmarkConverter]
                          bookmarksFromFile:pathToFile];
    }
    else if ([extension isEqualToString:@"html"] || [extension isEqualToString:@"htm"]) {
      importFolder = [[HTMLBookmarkConverter htmlBookmarkConverter]
                          bookmarksFromFile:pathToFile];
    }
    else if ([extension isEqualToString:@"plist"] || !importFolder) {
      importFolder = [self importPropertyListFile:pathToFile];
    }
    // We don't know the extension, or we failed to load. We'll take another
    // crack at it trying everything we know.
    if (!importFolder) {
      importFolder = [[OperaBookmarkConverter operaBookmarkConverter]
                          bookmarksFromFile:pathToFile];
    }
    if (!importFolder) {
      importFolder = [[HTMLBookmarkConverter htmlBookmarkConverter]
                          bookmarksFromFile:pathToFile];
    }
    if (!importFolder) {
      success = NO;
      break;
    }

    NSString* importFolderTitle = [titleEnumerator nextObject];
    if (!importFolderTitle)
      importFolderTitle = NSLocalizedString(@"Imported Bookmarks", nil);
    [importFolder setTitle:importFolderTitle];

    // The first folder will be top level, and the rest nested underneath it.
    // TODO: This was the existing behavior, but when we improve the feedback
    // for import failures we should do something better here.
    if (topImportFolder)
      [topImportFolder appendChild:importFolder];
    else
      topImportFolder = [importFolder retain];

    currentFile++;
  }

  // If somehow we imported nothing, return an empty folder.
  if (!topImportFolder) {
    NSString* title = NSLocalizedString(@"Imported Bookmarks", nil);
    topImportFolder = [BookmarkFolder bookmarkFolderWithTitle:title];
  }

  [self stopSuppressingChangeNotifications];

  NSDictionary *returnDict = [NSDictionary dictionaryWithObjectsAndKeys:
       [NSNumber numberWithBool:success], kBookmarkImportStatusIndentifier,
    [NSNumber numberWithInt:currentFile], kBookmarkImportNewFolderIndexIdentifier,
                               pathArray, kBookmarkImportPathIndentifier,
                         topImportFolder, kBookmarkImportNewFolderIdentifier,
                                          nil];

  [self performSelectorOnMainThread:@selector(importBookmarksThreadReturn:)
                         withObject:returnDict
                      waitUntilDone:YES];
  // release the top-level import folder we allocated - somebody else retains it by now if still needed.
  [topImportFolder release];

  [pool release];
}

- (void)importBookmarksThreadReturn:(NSDictionary *)aDict
{
  BOOL success = [[aDict objectForKey:kBookmarkImportStatusIndentifier] boolValue];
  NSArray *fileArray = [aDict objectForKey:kBookmarkImportPathIndentifier];
  int currentIndex = [[aDict objectForKey:kBookmarkImportNewFolderIndexIdentifier] intValue];
  BookmarkFolder *rootFolder = [self bookmarkRoot];
  BookmarkFolder *importFolder = [aDict objectForKey:kBookmarkImportNewFolderIdentifier];
  if (success || ((currentIndex - [fileArray count]) > 0)) {
    NSUndoManager *undoManager = [self undoManager];
    [rootFolder appendChild:importFolder];
    [undoManager setActionName:NSLocalizedString(@"Import Bookmarks", nil)];
  }
    [mImportDlgController finishThreadedImport:success
                                     fromFile:[[fileArray objectAtIndex:(--currentIndex)] lastPathComponent] ];
}

- (BookmarkFolder*)importPropertyListFile:(NSString *)pathToFile
{
  // Try Safari first.
  BookmarkFolder* importFolder = [[SafariBookmarkConverter safariBookmarkConverter]
                                      bookmarksFromFile:pathToFile];
  if (!importFolder) {
    BookmarkFolder* rootFolder = [BookmarkFolder bookmarkFolderWithTitle:nil];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfFile:pathToFile];
    if ([rootFolder readNativeDictionary:dict])
      importFolder = rootFolder;
  }
  return importFolder;
}

//
// Writing bookmark files
//

- (void)writeHTMLFile:(NSString *)pathToFile
{
  [[HTMLBookmarkConverter htmlBookmarkConverter] writeBookmarks:[self bookmarkRoot]
                                                         toFile:pathToFile];
}

- (void)writeSafariFile:(NSString *)pathToFile
{
  [[SafariBookmarkConverter safariBookmarkConverter] writeBookmarks:[self bookmarkRoot]
                                                             toFile:pathToFile];
}

//
// -writePropertyListFile:
//
// Writes all the bookmarks as a plist to the given file path. Write the file in
// two steps in case the initial write fails.
//
- (void)writePropertyListFile:(NSString *)pathToFile
{
  if (![NSThread inMainThread]) {
    NSLog(@"writePropertyListFile: called from background thread");
    return;
  }

  if (!pathToFile) {
    NSLog(@"writePropertyListFile: nil path argument");
    return;
  }

  BookmarkFolder* bookmarkRoot = [self bookmarkRoot];
  if (!bookmarkRoot)
    return;   // we never read anything

  NSDictionary* dict = [bookmarkRoot writeNativeDictionary];
  if (!dict) {
    NSLog(@"writePropertyListFile: writeNativeDictionary returned nil dictionary");
    return;
  }

  NSString* stdPath = [pathToFile stringByStandardizingPath];
  // Use the more roundabout NSPropertyListSerialization/NSData method to try to
  // get useful error data for bug 337750
  NSString* errorString = nil;
  NSData* bookmarkData = [NSPropertyListSerialization dataFromPropertyList:dict
                                                                    format:NSPropertyListXMLFormat_v1_0
                                                          errorDescription:&errorString];
  if (!bookmarkData) {
    NSLog(@"writePropertyListFile: dataFromPropertyList returned nil data: %@", errorString);
    [errorString release];
    return;
  }
  NSError* error = nil;
  BOOL success = [bookmarkData writeToFile:stdPath options:NSAtomicWrite error:&error];
  if (!success)
    NSLog(@"writePropertyListFile: %@ (%@)",
          [error localizedDescription], [error localizedFailureReason]);

  if (!success)
    NSLog(@"writePropertyList: Failed to write file %@", pathToFile);

  [self updateMetadataSyncTime];
}

- (BOOL)fileManager:(NSFileManager *)manager shouldProceedAfterError:(NSDictionary *)errorInfo
{
  NSLog(@"fileManager:shouldProceedAfterError:%@", errorInfo);
  return NO;
}

@end
