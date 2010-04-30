/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   David Hyatt <hyatt@mozilla.org> (Original Author)
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

#import "NSString+Gecko.h"
#import "NSString+Utils.h"

#import <AppKit/AppKit.h>
#import "AutoCompleteDataSource.h"
#import "AutoCompleteTextField.h"
#import "AutoCompleteResult.h"
#import "CHBrowserService.h"
#import "SiteIconProvider.h"

#import "Bookmark.h"
#import "BookmarkFolder.h"
#import "BookmarkManager.h"

#import "HistoryItem.h"
#import "HistoryDataSource.h"


const unsigned int kMaxResultsPerHeading = 5;
const unsigned int kNumberOfItemsPerChunk = 100;

@interface AutoCompleteDataSource (Private)
// Clears data previously loaded into mBookmarkData and mHistoryData.
// Also resets the chunk for a new search.
- (void)resetSearch;

// Checks if a given item (bookmark or history) matches the search string.
- (BOOL)searchStringMatchesItem:(id)item;

// Creates and returns an AutoCompleteResult object for the given item.
// The AutoCompleteResult class is used by the AutoCompleteCell to store
// all of the data relevant to drawing a cell.
- (AutoCompleteResult *)autoCompleteResultForItem:(id)item;

// Processes one chunk of the bookmark/history data (as specified by
// kNumberOfItemsPerChunk), then checks if we are done (i.e. when we
// have finished looking at all bookmark/history data or when we have
// enough results to satisfy kMaxResultsPerHeading).
- (void)processNextSearchChunk;

// Iterates though a chunk of the data array and looks for matches to the
// search string. When a match is found, it is added to the results array.
- (void)processChunkOfData:(NSArray *)dataArray forResults:(NSMutableArray *)resultsArray;

// Adds headers to results arrays and consolidates into mResults array,
// then calls searchResultsAvailable on the delegate.
- (void)reportResults;

// Adds the header to the specified results array.
- (void)addHeader:(NSString *)header toResults:(NSMutableArray *)results;
@end

@implementation AutoCompleteDataSource

-(id)init
{
  if ((self = [super init])) {
    mBookmarkData = [[NSMutableArray alloc] init];
    mHistoryData = [[NSMutableArray alloc] init];
    mBookmarkResultsInProgress = [[NSMutableArray alloc] init];
    mHistoryResultsInProgress = [[NSMutableArray alloc] init];
    mResults = [[NSMutableArray alloc] init];
    mGenericSiteIcon = [[NSImage imageNamed:@"globe_ico"] retain];
    mGenericFileIcon = [[NSImage imageNamed:@"smallDocument"] retain];
  }
  return self;
}

-(void)dealloc
{
  [mBookmarkData release];
  [mHistoryData release];
  [mBookmarkResultsInProgress release];
  [mHistoryResultsInProgress release];
  [mResults release];
  [mURLRegexTest release];
  [mTitleRegexTest release];
  [mGenericSiteIcon release];
  [mGenericFileIcon release];
  [super dealloc];
}

- (void)resetSearch
{
  [mBookmarkResultsInProgress removeAllObjects];
  [mHistoryResultsInProgress removeAllObjects];
  [mURLRegexTest release];
  mURLRegexTest = nil;
  [mTitleRegexTest release];
  mTitleRegexTest = nil;
  mChunkRange = NSMakeRange(0, kNumberOfItemsPerChunk);
}

- (void)loadSearchableData
{
  [mBookmarkData removeAllObjects];
  [mHistoryData removeAllObjects];

  BookmarkFolder *bookmarkRoot = [[BookmarkManager sharedBookmarkManager] bookmarkRoot];
  [mBookmarkData addObjectsFromArray:[bookmarkRoot allChildBookmarks]];
  NSSortDescriptor *visitCountDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"numberOfVisits"
                                                                        ascending:NO] autorelease];
  [mBookmarkData sortUsingDescriptors:[NSArray arrayWithObject:visitCountDescriptor]];

  HistoryDataSource *historyDataSource = [[[HistoryDataSource alloc] init] autorelease];
  [historyDataSource setHistoryView:kHistoryViewFlat];
  [historyDataSource setSortColumnIdentifier:@"visit_count"];
  [historyDataSource setSortDescending:YES];
  [historyDataSource loadLazily];
  HistoryItem *rootHistoryItem = [historyDataSource rootItem];
  NSEnumerator *historyEnum = [[rootHistoryItem children] objectEnumerator];
  HistoryItem *curChild;
  while ((curChild = [historyEnum nextObject])) {
    if ([curChild isKindOfClass:[HistorySiteItem class]])
      [mHistoryData addObject:curChild];
  }
}

- (void)performSearchWithString:(NSString *)searchString delegate:(id)delegate
{
  mDelegate = delegate;
  [self resetSearch];
  [NSObject cancelPreviousPerformRequestsWithTarget:self];

  // Construct the regular expression for url matching. NSPredicate will
  // only evaluate to true if the entire string matches--thus the leading
  // and trails stars. Matching works as follows:
  // 1. If the user has not typed '://', match any protocol followed by
  //    '://' followed by an optional 'www.' followed by the search string.
  // 2. If the user has typed '://', then match the entire search string.
  NSString *regex;
  if ([searchString rangeOfString:@"://"].location == NSNotFound)
    regex = [NSString stringWithFormat:@".*://(www\\.)?%@.*", searchString];
  else
    regex = [NSString stringWithFormat:@".*%@.*", searchString];
  mURLRegexTest = [[NSPredicate predicateWithFormat:@"SELF MATCHES[cd] %@", regex] retain];

  // Construct the regular expression for title matching. The title will
  // only match if the search string appears at the beginning of a word.
  regex = [NSString stringWithFormat:@".*\\b%@.*", searchString];
  mTitleRegexTest = [[NSPredicate predicateWithFormat:@"SELF MATCHES[cd] %@", regex] retain];

  [self performSelector:@selector(processNextSearchChunk) withObject:nil afterDelay:0.0];
}

- (void)processNextSearchChunk
{
  [self processChunkOfData:mBookmarkData forResults:mBookmarkResultsInProgress];
  [self processChunkOfData:mHistoryData forResults:mHistoryResultsInProgress];

  // Check if finished.
  BOOL bookmarksDone = [mBookmarkResultsInProgress count] == kMaxResultsPerHeading ||
                       NSMaxRange(mChunkRange) >= [mBookmarkData count];
  BOOL historyDone = [mHistoryResultsInProgress count] == kMaxResultsPerHeading ||
                     NSMaxRange(mChunkRange) >= [mHistoryData count];
  if (bookmarksDone && historyDone) {
    [self reportResults];
  } else {
    // Set new range and process the next chunk.
    mChunkRange = NSMakeRange(NSMaxRange(mChunkRange), kNumberOfItemsPerChunk);
    [self performSelector:@selector(processNextSearchChunk) withObject:nil afterDelay:0.0];
  }
}

- (void)processChunkOfData:(NSArray *)dataArray forResults:(NSMutableArray *)resultsArray
{
  for (unsigned int i = mChunkRange.location; i < [dataArray count] && i < NSMaxRange(mChunkRange)
       && [resultsArray count] < kMaxResultsPerHeading; i++) {
    id dataItem = [dataArray objectAtIndex:i];
    if ([self searchStringMatchesItem:dataItem]) {
      AutoCompleteResult *info = [self autoCompleteResultForItem:dataItem];
      if (![mBookmarkResultsInProgress containsObject:info] && ![mHistoryResultsInProgress containsObject:info])
        [resultsArray addObject:info];
    }
  }
}

- (BOOL)searchStringMatchesItem:(id)item
{
  // Never autocomplete bookmark shortcuts for searches
  if ([[item url] rangeOfString:@"%s"].location != NSNotFound)
    return NO;
  return [mURLRegexTest evaluateWithObject:[item url]] || [mTitleRegexTest evaluateWithObject:[item title]];
}

- (AutoCompleteResult *)autoCompleteResultForItem:(id)item
{
  AutoCompleteResult *info = [[[AutoCompleteResult alloc] init] autorelease];
  [info setTitle:[item title]];
  [info setUrl:[item url]];
  if ([[info title] isEqualToString:@""]) {
    NSString *host = [[NSURL URLWithString:[info url]] host];
    [info setTitle:(host ? host : [info url])];
  }
  NSImage* cachedFavicon = [[SiteIconProvider sharedFavoriteIconProvider] favoriteIconForPage:[info url]];
  if (cachedFavicon)
    [info setIcon:cachedFavicon];
  else if ([[info url] hasPrefix:@"file://"])
    [info setIcon:mGenericFileIcon];
  else
    [info setIcon:mGenericSiteIcon];
  return info;
}

- (void)reportResults
{
  [self addHeader:@"Bookmarks" toResults:mBookmarkResultsInProgress];
  [self addHeader:@"History" toResults:mHistoryResultsInProgress];
  [mResults removeAllObjects];
  [mResults addObjectsFromArray:mBookmarkResultsInProgress];
  [mResults addObjectsFromArray:mHistoryResultsInProgress];
  [self resetSearch];
  [mDelegate searchResultsAvailable];
}

- (void)addHeader:(NSString *)header toResults:(NSMutableArray *)results
{
  if ([results count] == 0) {
    // Don't add a header to empty results.
    return;
  }
  AutoCompleteResult *info = [[[AutoCompleteResult alloc] init] autorelease];
  [info setIsHeader:YES];
  [info setTitle:header];
  [results insertObject:info atIndex:0];
}

- (int)rowCount {
  return [mResults count];
}

- (id)resultForRow:(int)aRow columnIdentifier:(NSString *)aColumnIdentifier
{
  if (aRow >= 0 && aRow < [self rowCount])
    return [mResults objectAtIndex:aRow];
  return nil;
}

- (int)numberOfRowsInTableView:(NSTableView*)aTableView
{
  return [self rowCount];
}

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(int)aRowIndex
{
  return [self resultForRow:aRowIndex columnIdentifier:[aTableColumn identifier]];
}

@end
