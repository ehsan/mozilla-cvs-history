/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
* Version: NPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Netscape Public License
* Version 1.1 (the "License"); you may not use this file except in
* compliance with the License. You may obtain a copy of the License at
* http://www.mozilla.org/NPL/
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
*    David Hyatt <hyatt@apple.com> (Original Author)
*    David Haas <haasd@cae.wisc.edu>
*
*
* Alternatively, the contents of this file may be used under the terms of
* either the GNU General Public License Version 2 or later (the "GPL"), or
* the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
* in which case the provisions of the GPL or the LGPL are applicable instead
* of those above. If you wish to allow use of your version of this file only
* under the terms of either the GPL or the LGPL, and not to allow others to
* use your version of this file under the terms of the NPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the NPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */

#import "BookmarkButton.h"
#import "NSString+Utils.h"
#import "NSArray+Utils.h"
#import "NSPasteboard+Utils.h"
#import "DraggableImageAndTextCell.h"
#import "BookmarkManager.h"
#import "Bookmark.h"
#import "BookmarkFolder.h"
#import "BookmarkMenu.h"
#import "BookmarkInfoController.h"
#import "BrowserWindowController.h"
#import "MainController.h"
#import "PreferenceManager.h"

@interface BookmarkButton(Private)

- (void)showFolderPopupAction:(id)aSender;
- (void)showFolderPopup:(NSEvent*)event;

@end


@implementation BookmarkButton
- (id)initWithFrame:(NSRect)frame
{
  if ( (self = [super initWithFrame:frame]) )
  {
    DraggableImageAndTextCell* newCell = [[[DraggableImageAndTextCell alloc] init] autorelease];
    [newCell setDraggable:YES];
    [self setCell:newCell];

    [self setBezelStyle: NSRegularSquareBezelStyle];
    [self setButtonType: NSMomentaryChangeButton];
    [self setBordered: NO];
    [self setImagePosition: NSImageLeft];
    [self setRefusesFirstResponder: YES];
    [self setFont: [NSFont labelFontOfSize: 11.0]];
  }
  return self;
}

-(id)initWithFrame:(NSRect)frame item:(BookmarkItem*)item
{
  if ( (self = [self initWithFrame:frame]) )
  {
    [self setBookmarkItem:item];
  }
  return self;
}

- (void)dealloc
{
  [mItem release];
  [super dealloc];
}


- (void)setBookmarkItem:(BookmarkItem*)aItem
{
  [aItem retain];
  [mItem release];
  mItem = aItem;
  [self setTitle:[aItem title]];
  [self setImage:[aItem icon]];
  [self setTarget:self];
  if ([aItem isKindOfClass:[Bookmark class]]) {
    [self setAction:@selector(openBookmark:)];
    [self setToolTip:[(Bookmark *)aItem url]];
  } else {
    [[self cell] setClickHoldTimeout:0.5];
    if ([(BookmarkFolder *)aItem isGroup])
      [self setAction: @selector(openBookmark:)];
    else 
      [self setAction:@selector(showFolderPopupAction:)];
  }
}

- (BookmarkItem*)BookmarkItem
{
  return mItem;
}

-(IBAction)openBookmark:(id)aSender
{
  BrowserWindowController* brController = [[self window] windowController];
  // See if we're a group.
  BookmarkItem *item = [self BookmarkItem];
  if ([item isKindOfClass:[BookmarkFolder class]]) {
    if ([(BookmarkFolder *)item isGroup]) {
      [brController openTabGroup:[(BookmarkFolder *)item childURLs] replaceExistingTabs:YES];
      return;
    }
  }
  // if the command key is down, follow the command-click pref
  if (([[NSApp currentEvent] modifierFlags] & NSCommandKeyMask) &&
      [[PreferenceManager sharedInstance] getBooleanPref:"browser.tabs.opentabfor.middleclick" withSuccess:NULL])
  {
    [self openBookmarkInNewTab:aSender];
    return;
  }
  [brController loadURL:[(Bookmark *)item url] referrer:nil activate:YES];
}

-(IBAction)openBookmarkInNewTab:(id)aSender
{
  BookmarkItem *item = [self BookmarkItem];
  BrowserWindowController* brController = [[self window] windowController];
  if ([item isKindOfClass:[Bookmark class]]) {
    NSString* hrefStr = [(Bookmark *)item url];
    BOOL loadInBackground = [[PreferenceManager sharedInstance] getBooleanPref:"browser.tabs.loadInBackground" withSuccess:NULL];
    [brController openNewTabWithURL: hrefStr referrer:nil loadInBackground: loadInBackground];
  } else if ([item isKindOfClass:[BookmarkFolder class]]) {
    [brController openTabGroup:[(BookmarkFolder *)item childURLs] replaceExistingTabs:YES];
  }
}

-(IBAction)openBookmarkInNewWindow:(id)aSender
{
  BOOL loadInBackground = [[PreferenceManager sharedInstance] getBooleanPref:"browser.tabs.loadInBackground" withSuccess:NULL];
  BrowserWindowController* brController = [[self window] windowController];
  // See if we're a bookmark.
  BookmarkItem *item = [self BookmarkItem];
  if ([item isKindOfClass:[Bookmark class]])
    [brController openNewWindowWithURL:[(Bookmark *)item url] referrer: nil loadInBackground: loadInBackground];
  else if ([item isKindOfClass:[BookmarkFolder class]])
    [brController openNewWindowWithGroupURLs:[(BookmarkFolder *)item childURLs] loadInBackground: loadInBackground];
  return; 
}

-(IBAction)showBookmarkInfo:(id)aSender
{
  BookmarkInfoController *bic = [BookmarkInfoController sharedBookmarkInfoController];
  [bic setBookmark:[self BookmarkItem]];
  [bic showWindow:self];
}

-(IBAction)deleteBookmarks: (id)aSender
{
  BookmarkItem *item = [self BookmarkItem];
  [[item parent] deleteChild:item];
  [self removeFromSuperview];
}

-(IBAction)addFolder:(id)aSender
{
  BookmarkManager* bmManager = [BookmarkManager sharedBookmarkManager];
  BookmarkFolder* toolbarFolder = [bmManager toolbarFolder];
  BookmarkFolder* aFolder = [toolbarFolder addBookmarkFolder];
  [aFolder setTitle:NSLocalizedString(@"NewBookmarkFolder",@"New Folder")];
}

-(void)drawRect:(NSRect)aRect
{
  [super drawRect: aRect];
}

-(NSMenu*)menuForEvent:(NSEvent*)aEvent
{
  BookmarkItem *item = [self BookmarkItem];
  if (item) {
    NSMenu* contextMenu = [[[self superview] menu] copy];
    [[contextMenu itemArray] makeObjectsPerformSelector:@selector(setTarget:) withObject: self];
    NSString *nulString = [NSString string];
    // clean the menu out
    int numItems = [contextMenu numberOfItems];
    int itemIndex = [contextMenu indexOfItemWithTarget:self andAction:@selector(showBookmarkInfo:)];
    while (numItems > (itemIndex+1))
      [contextMenu removeItemAtIndex:(--numItems)];
    // set up menu
    if ([item isKindOfClass:[Bookmark class]]) {
      itemIndex = [contextMenu indexOfItemWithTarget:self andAction:@selector(openBookmarkInNewWindow:)];
      [[contextMenu itemAtIndex:itemIndex] setTitle:NSLocalizedString(@"Open in New Window",@"Open in New Window")];
      itemIndex = [contextMenu indexOfItemWithTarget:self andAction:@selector(openBookmarkInNewTab:)];
      [[contextMenu itemAtIndex:itemIndex] setTitle:NSLocalizedString(@"Open in New Tab",@"Open in New Tab")];
    } else if ([item isKindOfClass:[BookmarkFolder class]]) {
      itemIndex = [contextMenu indexOfItemWithTarget:self andAction:@selector(openBookmarkInNewWindow:)];
      [[contextMenu itemAtIndex:itemIndex] setTitle:NSLocalizedString(@"Open Tabs in New Window",@"Open Tabs in New Window")];
      itemIndex = [contextMenu indexOfItemWithTarget:self andAction:@selector(openBookmarkInNewTab:)];
      [[contextMenu itemAtIndex:itemIndex] setTitle:NSLocalizedString(@"Open in Tabs",@"Open in Tabs")];
    }
    // if it's a button, it's got to be on toolbar folder, so we can delete & make new folders
    NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Delete",@"Delete") action:@selector(deleteBookmarks:) keyEquivalent:nulString];
    [menuItem setTarget:self];
    [contextMenu addItem:menuItem];
    [menuItem release];
    [contextMenu addItem:[NSMenuItem separatorItem]];
    // create new folder
    menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Create New Folder...",@"Create New Folder...") action:@selector(addFolder:) keyEquivalent:nulString];
    [menuItem setTarget:self];
    [contextMenu addItem:menuItem];
    [menuItem release];
    return [contextMenu autorelease];
  }
  return nil;
}

//
// context menu has only what we need
//
-(BOOL)validateMenuItem:(NSMenuItem*)aMenuItem
{
  return YES;
}

- (void)showFolderPopupAction:(id)aSender
{
  [self showFolderPopup:[NSApp currentEvent]];
}

//
// -showFolderPopup:
//
// For bookmarks that are folders, display their children in a menu. Uses a transient
// NSPopUpButtonCell to handle the menu tracking. Even though the toolbar is drawn
// at 11pt, use the normal font size for these submenus. Not only do context menus use
// this size, it's easier on the eyes.
//
- (void)showFolderPopup:(NSEvent*)event
{
  NSMenu* popupMenu = [[NSMenu alloc] init];
  // dummy first item
  [popupMenu addItemWithTitle:@"" action:NULL keyEquivalent:@""];
  // make a temporary BookmarkMenu to build the menu
  BookmarkMenu* bmMenu = [[BookmarkMenu alloc] initWithMenu:popupMenu firstItem:1 rootBookmarkFolder:(BookmarkFolder *)[self BookmarkItem]];
  // use a temporary NSPopUpButtonCell to display the menu.
  NSPopUpButtonCell	*popupCell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:YES];
  [popupCell setMenu: popupMenu];
  [popupCell trackMouse:event inRect:[self bounds] ofView:self untilMouseUp:YES];
  [popupCell release];
  [bmMenu release];
  [popupMenu release];
}

-(void)mouseDown:(NSEvent*)aEvent
{
  [super mouseDown:aEvent];
  if ([[self cell] lastClickHoldTimedOut])
    [self showFolderPopup:aEvent];
}

- (unsigned int)draggingSourceOperationMaskForLocal:(BOOL)localFlag
{
  if (localFlag)
    return (NSDragOperationCopy | NSDragOperationGeneric | NSDragOperationMove);

  return (NSDragOperationDelete | NSDragOperationGeneric);
}

- (void) mouseDragged: (NSEvent*) aEvent
{
  BookmarkItem *item = [self BookmarkItem];
  BOOL isSingleBookmark = [item isKindOfClass:[Bookmark class]];
  NSPasteboard *pboard = [NSPasteboard pasteboardWithName:NSDragPboard];
  NSString     *title = [item title];
  if (isSingleBookmark)
  {
    [pboard declareURLPasteboardWithAdditionalTypes:[NSArray arrayWithObject:@"MozBookmarkType"] owner:self];
    NSString     *url 	= [(Bookmark *)item url];
    NSString     *cleanedTitle = [title stringByReplacingCharactersInSet:[NSCharacterSet controlCharacterSet] withString:@" "];
    [pboard setDataForURL:url title:cleanedTitle];
  }
  else
  {
    [pboard declareTypes:[NSArray arrayWithObject:@"MozBookmarkType"] owner:self];
  }
  // MozBookmarkType
  NSArray *pointerArray = [NSArray dataArrayFromPointerArrayForMozBookmarkDrop:[NSArray arrayWithObject:item]];
  [pboard setPropertyList:pointerArray forType: @"MozBookmarkType"];
  [self dragImage: [MainController createImageForDragging:[self image] title:title]
               at: NSMakePoint(0,NSHeight([self bounds])) offset: NSMakeSize(0,0)
            event: aEvent pasteboard: pboard source: self slideBack: YES];
}

- (void)draggedImage:(NSImage *)anImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)operation
{
  if (operation == NSDragOperationDelete)
  {
    NSPasteboard* pboard = [NSPasteboard pasteboardWithName:NSDragPboard];
    NSArray* bookmarks = [NSArray pointerArrayFromDataArrayForMozBookmarkDrop:[pboard propertyListForType: @"MozBookmarkType"]];
    if (bookmarks)
    {
      for (unsigned int i = 0; i < [bookmarks count]; ++i)
      {
        BookmarkItem* item = [bookmarks objectAtIndex:i];
        [[item parent] deleteChild:item];
      }
    }
  }
}



@end
