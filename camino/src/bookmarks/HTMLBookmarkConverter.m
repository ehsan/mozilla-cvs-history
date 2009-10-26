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
 * The Original Code is Camino code.
 *
 * The Initial Developer of the Original Code is
 * Stuart Morgan
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Morgan <stuart.morgan@alumni.case.edu>
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

#import "HTMLBookmarkConverter.h"

#import "BookmarkItem.h"
#import "Bookmark.h"
#import "BookmarkFolder.h"

#import "NSArray+Utils.h"
#import "NSString+Utils.h"

@interface NSString (HTMLBookmarkImportHelper)
- (NSString*)cleanedStringFromHTMLBookmarkImport;
@end

@implementation NSString (HTMLBookmarkImportHelper)
// Removes any whitespace or control characters that we don't want in our
// bookmark fields.
- (NSString*)cleanedStringFromHTMLBookmarkImport;
{
  return [[self stringByReplacingCharactersInSet:[NSCharacterSet controlCharacterSet]
                                      withString:@" "] stringByTrimmingWhitespace];
}
@end

#pragma mark -

@interface HTMLBookmarkConverter (Private)
- (BookmarkItem*)bookmarkItemForElement:(NSXMLElement*)element;
- (Bookmark*)bookmarkForElement:(NSXMLElement*)element;
- (BookmarkFolder*)bookmarkFolderForElement:(NSXMLElement*)element;
- (NSString*)descriptionAssociatedWithElement:(NSXMLElement*)element;

- (void)writeBookmarkFolder:(BookmarkFolder*)bookmarkFolder
               toFileHandle:(NSFileHandle*)fileHandle
            withIndentation:(unsigned int)indentationLevel;
- (void)writeBookmark:(Bookmark*)bookmark
         toFileHandle:(NSFileHandle*)fileHandle
      withIndentation:(unsigned int)indentationLevel;
@end

@implementation HTMLBookmarkConverter

+ (id)htmlBookmarkConverter
{
  return [[[self alloc] init] autorelease];
}

- (BookmarkFolder*)bookmarksFromFile:(NSString*)filePath
{
  NSError* error = nil;
  NSXMLDocument* bookmarkDoc = nil;
  // NSXMLDocument will sometimes throw an exception
  @try {
    bookmarkDoc = [[[NSXMLDocument alloc] initWithData:[NSData dataWithContentsOfFile:filePath]
                                              options:NSXMLDocumentTidyHTML
                                                error:&error] autorelease];
  }
  @catch (id exception) {
  }
  if (!bookmarkDoc) {
    NSLog(@"Unable to read bookmark file '%@' for import", filePath);
    return nil;
  }
  NSXMLElement* root = [[[bookmarkDoc rootElement] nodesForXPath:@"/html/body"
                                                           error:&error] firstObject];
  if (!root) {
    NSLog(@"Unable to parse bookmark file '%@' for import", filePath);
    return nil;
  }
  BookmarkItem* rootItem = [self bookmarkItemForElement:root];
  if (rootItem && ![rootItem isKindOfClass:[BookmarkFolder class]]) {
    BookmarkFolder* newFolder = [[[BookmarkFolder alloc] init] autorelease];
    [newFolder appendChild:rootItem];
    rootItem = newFolder;
  }
  return (BookmarkFolder*)rootItem;
}

- (BookmarkItem*)bookmarkItemForElement:(NSXMLElement*)element
{
  // Just find the top-level node corresponding to each entry.
  // Although it doesn't look that way from the source, all folders end up
  // under <dd> elements due to implicit node creation.
  if ([[element name] isEqual:@"body"] || [[element name] isEqual:@"dd"]) {
    return [self bookmarkFolderForElement:element];
  }
  else if ([[element name] isEqual:@"dt"]) {
    return [self bookmarkForElement:element];
  }
  // Anything else at the top level is junk (e.g., spacing <p>s).
  return nil;
}

- (Bookmark*)bookmarkForElement:(NSXMLElement*)element
{
  NSXMLElement* linkElement = [[element elementsForName:@"a"] firstObject];
  if (!linkElement)
    return nil;

  NSString* title = [[linkElement stringValue] cleanedStringFromHTMLBookmarkImport];
  NSString* href = [[[linkElement attributeForName:@"href"] stringValue] cleanedStringFromHTMLBookmarkImport];
  // Firefox 3 exports placeholders for their smart folders with place:
  // links. They don't serve any use in Camino, so throw them away.
  if ([href hasPrefix:@"place:"])
    return nil;
  // OmniWeb uses bare <a> elements for separators rather than <hr>,
  // and does put them in their own <dt>s
  if (!href && [title length] == 0)
    return [Bookmark separator];
  if (!title || !href)
    return nil;

  Bookmark* bookmark = [Bookmark bookmarkWithTitle:title
                                               url:href
                                         lastVisit:nil];
  NSString* description = [self descriptionAssociatedWithElement:element];
  if (description)
    [bookmark setItemDescription:description];
  NSString* shortcut = [[[linkElement attributeForName:@"shortcuturl"] stringValue] cleanedStringFromHTMLBookmarkImport];
  if (shortcut)
    [bookmark setShortcut:shortcut];
  // Tidy does a last_visit -> last-visit transform, so try both.
  int lastVisitSeconds = [[[linkElement attributeForName:@"last-visit"] stringValue] intValue];
  if (lastVisitSeconds <= 0)
    lastVisitSeconds = [[[linkElement attributeForName:@"last_visit"] stringValue] intValue];
  if (lastVisitSeconds > 0)
    [bookmark setLastVisit:[NSDate dateWithTimeIntervalSince1970:(NSTimeInterval)lastVisitSeconds]];
  int visitCount = [[[linkElement attributeForName:@"visitation_count"] stringValue] intValue];
  if (visitCount > 0)
    [bookmark setNumberOfVisits:(unsigned int)visitCount];

  return bookmark;
}

- (BookmarkFolder*)bookmarkFolderForElement:(NSXMLElement*)element
{
  NSXMLElement* titleElement = [[element elementsForName:@"h3"] firstObject];
  NSString* title = [[titleElement stringValue] cleanedStringFromHTMLBookmarkImport];
  if (!title && [[element name] isEqual:@"body"]) {
    // Use an empty title; we'll rename the top-level folder anyway
    title = @"";
  }
  // The HTML format is terrible, end ends up synthesizing lots of empty <dd>
  // elements; the ones that are actually folders start contain an <h3>.
  if (!title)
    return nil;

  BookmarkFolder* folder = [BookmarkFolder bookmarkFolderWithTitle:title];
  NSString* description = [self descriptionAssociatedWithElement:element];
  if (description)
    [folder setItemDescription:description];
  NSString* shortcut = [[[titleElement attributeForName:@"shortcuturl"] stringValue] cleanedStringFromHTMLBookmarkImport];
  if (shortcut)
    [folder setShortcut:shortcut];
  NSString* foldergroup = [[titleElement attributeForName:@"folder_group"] stringValue];
  if (foldergroup && [foldergroup compare:@"true" options:NSCaseInsensitiveSearch] == NSOrderedSame)
    [folder setIsGroup:YES];

  NSArray* children = [[[element elementsForName:@"dl"] firstObject] children];
  // Folders can end up split into two sibling nodes in the parse tree (for
  // example, a folder with a description), so if there is no child list
  // in this node, check to see if the next node has a child list but no <h3>.
  if (!children) {
    NSXMLNode* nextSibling = [element nextSibling];
    if ([nextSibling kind] == NSXMLElementKind &&
        [[nextSibling name] isEqual:@"dd"] &&
        [[(NSXMLElement*)nextSibling elementsForName:@"h3"] count] == 0)
    {
      children = [[[(NSXMLElement*)nextSibling elementsForName:@"dl"] firstObject] children];
      // We don't have to do anything special to skip this node later, because
      // it will be thrown away for not having a title.
    }
  }

  NSEnumerator* childEnumerator = [children objectEnumerator];
  NSXMLElement* childElement;
  while ((childElement = [childEnumerator nextObject])) {
    // Create an inner pool before recursing so that we don't accumulate
    // temporary objects for the entire import process.
    NSAutoreleasePool* innerPool = [[NSAutoreleasePool alloc] init];

    BookmarkItem* childItem = [self bookmarkItemForElement:childElement];
    if (childItem)
      [folder appendChild:childItem];
    
    // No-one writes out a <dt> for <hr> separators, so they end up underneath
    // the previous block, so we have to look for them under every element.
    unsigned int dividerCount = [[childElement elementsForName:@"hr"] count];
    for (unsigned int i = 0; i < dividerCount; ++i)
      [folder appendChild:[Bookmark separator]];

    [innerPool release];
  }

  return folder;  
}

// For a <dt> or <dd> node corresponding to a bookmark or folder, this returns
// the description associated with that bookmark item if any, or nil if there
// isn't one.
- (NSString*)descriptionAssociatedWithElement:(NSXMLElement*)element {
  // A description is in a <dd> following actual item's node, but because the
  // parser synthesizes a bunch of <dd>s, we can't assume that a <dd> is
  // actually a description. A real description <dd> will start with a node
  // that has actual text in it, so look for that.
  NSXMLNode* nextSibling = [element nextSibling];
  if ([nextSibling kind] == NSXMLElementKind &&
      [(NSXMLElement*)[nextSibling name] isEqual:@"dd"])
  {
    NSXMLNode* possibleDescriptionNode = [[nextSibling children] firstObject];
    if ([possibleDescriptionNode kind] != NSXMLTextKind)
      return nil;
    NSString* description = [[possibleDescriptionNode stringValue] cleanedStringFromHTMLBookmarkImport];
    return ([description length] > 0) ? description : nil;
  }
  return nil;
}

#pragma mark -

- (void)writeBookmarks:(BookmarkFolder*)bookmarkRoot toFile:(NSString*)filePath
{
  // Create a new, empty file to write bookmarks into
  NSFileManager* fileManager = [NSFileManager defaultManager];
  BOOL existingFileIsDirectory;
  if ([fileManager fileExistsAtPath:filePath isDirectory:&existingFileIsDirectory]) {
    if (existingFileIsDirectory) {
      NSLog(@"Cannot replace directory '%@' with exported bookmarks", filePath);
      return;
    }
    if (![fileManager removeFileAtPath:filePath handler:nil]) {
      NSLog(@"Cannot replace file '%@' with exported bookmarks", filePath);
      return;
    }
  }
  if (![fileManager createFileAtPath:filePath contents:[NSData data] attributes:nil]) {
    NSLog(@"Cannot create '%@' for exported bookmarks", filePath);
    return;
  }

  // open the file for writing
  NSFileHandle* outHandle = [NSFileHandle fileHandleForWritingAtPath:filePath];
  if (!outHandle) {
    NSLog(@"Unable to open '%@' for writing", filePath);
    return;
  }

  [outHandle writeData:[[NSString stringWithFormat:@"%@\n%@\n%@\n%@\n\n<DL><p>\n",
                          @"<!DOCTYPE NETSCAPE-Bookmark-file-1>",
                          @"<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">",
                          @"<TITLE>Bookmarks</TITLE>",
                          @"<H1>Bookmarks</H1>"] dataUsingEncoding:NSUTF8StringEncoding]];

  NSEnumerator* folderEnumerator = [[(BookmarkFolder*)bookmarkRoot children] objectEnumerator];
  BookmarkItem* child;
  while ((child = [folderEnumerator nextObject])) {
    if ([child isKindOfClass:[BookmarkFolder class]]) {
      [self writeBookmarkFolder:(BookmarkFolder*)child
                   toFileHandle:outHandle
                withIndentation:1];
    }
    else {
      [self writeBookmark:(Bookmark*)child
             toFileHandle:outHandle
          withIndentation:1];
    }
  }

  [outHandle writeData:[@"</DL><p>\n" dataUsingEncoding:NSUTF8StringEncoding]];
}

- (void)writeBookmarkFolder:(BookmarkFolder*)bookmarkFolder
               toFileHandle:(NSFileHandle*)fileHandle
            withIndentation:(unsigned int)indentationLevel;
{
  // Skip smart folders entirely
  if ([bookmarkFolder isSmartFolder])
    return;

  NSMutableString* bookmarkLine = [NSMutableString string];
  NSMutableString* padString = [NSMutableString string];
  for (unsigned i = 0; i < indentationLevel; i++)
    [padString appendString:@"\t"];
  [bookmarkLine appendString:padString];

  NSString* attributes = nil;
  if ([bookmarkFolder isToolbar])
    attributes = @" PERSONAL_TOOLBAR_FOLDER=\"true\"";
  else if ([bookmarkFolder isGroup])
    attributes = @" FOLDER_GROUP=\"true\"";
  [bookmarkLine appendFormat:@"<DT><H3%@>%@</H3>\n%@<DL><p>\n",
    attributes ? attributes : @"", [[bookmarkFolder title] stringByAddingAmpEscapes], padString];

  [fileHandle writeData:[bookmarkLine dataUsingEncoding:NSUTF8StringEncoding]];

  NSEnumerator* childEnumerator = [[bookmarkFolder children] objectEnumerator];
  BookmarkItem* child;
  while ((child = [childEnumerator nextObject])) {
    // Create an inner pool before recursing so that we don't accumulate
    // temporary objects for the entire export process.
    NSAutoreleasePool* innerPool = [[NSAutoreleasePool alloc] init];

    if ([child isKindOfClass:[BookmarkFolder class]]) {
      [self writeBookmarkFolder:(BookmarkFolder*)child
             toFileHandle:fileHandle
          withIndentation:(indentationLevel + 1)];
    }
    else {
      [self writeBookmark:(Bookmark*)child
             toFileHandle:fileHandle
          withIndentation:(indentationLevel + 1)];
    }

    [innerPool release];
  }

  [fileHandle writeData:[[padString stringByAppendingString:@"</DL><p>\n"]
                           dataUsingEncoding:NSUTF8StringEncoding]];
}

- (void)writeBookmark:(Bookmark*)bookmark
         toFileHandle:(NSFileHandle*)fileHandle
      withIndentation:(unsigned int)indentationLevel
{
  NSMutableString* bookmarkLine = [NSMutableString string];
  NSMutableString* padString = [NSMutableString string];
  for (unsigned i = 0; i < indentationLevel; i++)
    [padString appendString:@"\t"];
  [bookmarkLine appendString:padString];

  if ([bookmark isSeparator]) {
    [bookmarkLine appendString:@"<HR>"];
  }
  else {
    [bookmarkLine appendFormat:@"<DT><A HREF=\"%@\"", [bookmark url]];
    if ([bookmark lastVisit])
      [bookmarkLine appendFormat:@" LAST_VISIT=\"%d\"", [[bookmark lastVisit] timeIntervalSince1970]];
    if ([[bookmark shortcut] length] > 0)
      [bookmarkLine appendFormat:@" SHORTCUTURL=\"%@\"", [bookmark shortcut]];
    // close up the attributes, export the title, close the A tag
    [bookmarkLine appendFormat:@">%@</A>", [[bookmark title] stringByAddingAmpEscapes]];

    NSString* description = [bookmark itemDescription];
    if ([description length] > 0)
      [bookmarkLine appendFormat:@"\n%@<DD>%@", padString, [description stringByAddingAmpEscapes]];
  }
  [bookmarkLine appendString:@"\n"];
  [fileHandle writeData:[bookmarkLine dataUsingEncoding:NSUTF8StringEncoding]];
}

@end
