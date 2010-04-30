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

#import <AppKit/AppKit.h>


@class HistoryItem;

//
// This class is in charge of retrieving/sorting the history and bookmark results
// for the location bar autocomplete, and is also the datasource for the table view
// in the autocomplete popup window. All of the work is performed on the main thread
// using a chunked asynchronous method, which should allow everything to run smoothly 
// even with large number of bookmark/history items.
//
@interface AutoCompleteDataSource : NSObject
{
  id                      mDelegate;

  NSRange                 mChunkRange;
  NSPredicate*            mURLRegexTest;                // owned
  NSPredicate*            mTitleRegexTest;              // owned

  NSMutableArray*         mBookmarkData;                // owned
  NSMutableArray*         mHistoryData;                 // owned
  NSMutableArray*         mBookmarkResultsInProgress;   // owned
  NSMutableArray*         mHistoryResultsInProgress;    // owned
  NSMutableArray*         mResults;                     // owned

  NSImage*                mGenericSiteIcon;             // owned
  NSImage*                mGenericFileIcon;             // owned
}

// Pulls bookmarks and history items and puts them into mBookmarkData and mHistoryData.
- (void)loadSearchableData;

// Starts the search for matching bookmarks/history items using the string passed
// from AutoCompleteTextField. This is an asynchronous method--when the search is
// complete, searchResultsAvailable will be called on the delegate.
- (void)performSearchWithString:(NSString *)searchString delegate:(id)delegate;

// Stops the search, meaning that no further results for the current search
// string will be sent to the delegate.
- (void)cancelSearch;

// Returns the number of rows matching the search string, including headers.
- (int)rowCount;

// Datasource methods.
- (int)numberOfRowsInTableView:(NSTableView*)aTableView;
- (id)resultForRow:(int)aRow columnIdentifier:(NSString *)aColumnIdentifier;

@end
