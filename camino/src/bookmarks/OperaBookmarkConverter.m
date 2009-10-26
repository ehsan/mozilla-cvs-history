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
 * Portions created by the Initial Developer are Copyright (C) 2009
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

#import "OperaBookmarkConverter.h"

#import "NSString+Utils.h"

#import "BookmarkItem.h"
#import "Bookmark.h"
#import "BookmarkFolder.h"

// Opera bookmark file markers and property keys.
static NSString* const kFolderStartMarker = @"#FOLDER";
static NSString* const kFolderEndMarker = @"-";
static NSString* const kBookmarkStartMarker = @"#URL";
static NSString* const kSeparatorMarker = @"#SEPERATOR";
static NSString* const kTitleKey = @"NAME";
static NSString* const kURLKey = @"URL";
static NSString* const kDescriptionKey = @"DESCRIPTION";
static NSString* const kShortcutKey = @"SHORT NAME";

// The format for Opera bookmarks is a flat file with a series of
// space-separated blocks:
// 
// MARKER
//   KEY=value
//   KEY=value
//   KEY=value
// 
// MARKER
//   KEY=value
// 
// ...
// 
// Folders work the same way, and everything following is enclosed in that
// folder until a line consisting of:
// -
// ends that folder.

@interface OperaBookmarkConverter (Private)

// Consumes lines from enumerator, constructing the appropriate bookmark items,
// until the end of the current folder is reached.
- (void)readLines:(NSEnumerator*)enumerator intoFolder:(BookmarkFolder*)parent;

// Reads key-value pairs from the enumerator until a non-key-value line is
// reached, returning the pairs as a dictionary.
- (NSDictionary*)readProperties:(NSEnumerator*)lineEnumerator;

@end

@implementation OperaBookmarkConverter

+ (id)operaBookmarkConverter
{
  return [[[self alloc] init] autorelease];
}

- (BookmarkFolder*)bookmarksFromFile:(NSString*)filePath
{
  NSString* fileAsString = [NSString stringWithContentsOfFile:filePath
                                                     encoding:NSUTF8StringEncoding
                                                        error:NULL];
  if (!fileAsString) {
    NSLog(@"Couldn't read Opera bookmark file.");
    return nil;
  }
  NSRange headerRange = [fileAsString rangeOfString:@"Opera Hotlist"
                                            options:NSCaseInsensitiveSearch];
  if (headerRange.location == NSNotFound) {
    NSLog(@"Bookmark file not recognized as Opera Hotlist.");
    return nil;
  }

  BookmarkFolder *rootFolder = [[[BookmarkFolder alloc] init] autorelease];

  NSArray *lines = [fileAsString componentsSeparatedByString:@"\n"];
  [self readLines:[lines objectEnumerator] intoFolder:rootFolder];

  return rootFolder;
}

- (void)readLines:(NSEnumerator*)enumerator intoFolder:(BookmarkFolder*)parent
{
  NSString *line;
  while ((line = [enumerator nextObject])) {
    if ([line hasPrefix:kFolderStartMarker]) {
      NSDictionary* properties = [self readProperties:enumerator];
      BookmarkFolder* folder =
          [BookmarkFolder bookmarkFolderWithTitle:[properties objectForKey:kTitleKey]];
      [parent appendChild:folder];
      [self readLines:enumerator intoFolder:folder];
    }
    else if ([line hasPrefix:kBookmarkStartMarker]) {
      NSDictionary* properties = [self readProperties:enumerator];
      BookmarkItem* bookmark =
          [Bookmark bookmarkWithTitle:[properties objectForKey:kTitleKey]
                                  url:[properties objectForKey:kURLKey]
                            lastVisit:nil];
      if ([properties objectForKey:kDescriptionKey])
        [bookmark setItemDescription:[properties objectForKey:kDescriptionKey]];
      if ([properties objectForKey:kShortcutKey])
        [bookmark setItemDescription:[properties objectForKey:kShortcutKey]];
      [parent appendChild:bookmark];
    }
    else if ([line hasPrefix:kSeparatorMarker]) {
      [parent appendChild:[Bookmark separator]];
    }
    else if ([line hasPrefix:kFolderEndMarker])
      return;
  }
}

- (NSDictionary*)readProperties:(NSEnumerator*)lineEnumerator
{
  NSMutableDictionary* properties = [NSMutableDictionary dictionary];
  NSString *line;
  while ((line = [lineEnumerator nextObject])) {
    NSRange equalsRange = [line rangeOfString:@"="];
    // Each section ends with a blank line, so it's okay that this eats one
    // line past the key-value lines.
    if (equalsRange.location == NSNotFound)
      break;
    [properties setObject:[line substringFromIndex:(equalsRange.location + 1)]
                   forKey:[[line substringToIndex:equalsRange.location]
                              stringByTrimmingWhitespace]];
  }
  return properties;
}

@end
