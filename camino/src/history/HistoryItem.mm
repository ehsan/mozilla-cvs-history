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
 *   Simon Fraser <smfr@smfr.org>
 *   Christopher Henderson <trendyhendy2000@gmail.com>
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

#import "NSString+Utils.h"
#import "NSString+Gecko.h"
#import "NSDate+Utils.h"
#import "NSImage+Utils.h"

#import "HistoryItem.h"
#import "HistoryDataSource.h"

#import "SiteIconProvider.h"

#import "nsString.h"

#import "nsINavHistoryService.h"
#import "nsIURI.h"
#import "nsCOMPtr.h"
#import "nsNetUtil.h"


// search field tags, used in search field context menu item tags
enum
{
  eHistorySearchFieldAll = 1,
  eHistorySearchFieldTitle,
  eHistorySearchFieldURL
};


@implementation HistoryItem

- (id)initWithDataSource:(HistoryDataSource*)inDataSource
{
  if ((self = [super init]))
  {
    mDataSource = inDataSource;   // not retained
  }
  return self;
}

- (void)dealloc
{
  [super dealloc];
}

- (HistoryDataSource*)dataSource
{
  return mDataSource;
}

- (NSString*)title
{
  return @"";   // subclasses override
}

- (BOOL)isSiteItem
{
  return NO;
}

- (NSImage*)icon
{
  return nil;
}

- (NSImage*)iconAllowingLoad:(BOOL)inAllowLoad
{
  return [self icon];
}

- (NSString*)url
{
  return @"";
}

- (NSDate*)firstVisit
{
  return nil;
}

- (NSDate*)lastVisit
{
  return nil;
}

- (NSNumber*)visitCount
{
  return nil;
}

- (NSString*)hostname
{
  return @"";
}

- (NSString*)identifier
{
  return @"";
}

- (void)setParentItem:(HistoryItem*)inParent
{
  mParentItem = inParent;
}

- (HistoryItem*)parentItem
{
  return mParentItem;
}

- (BOOL)isDescendentOfItem:(HistoryItem*)inItem
{
  if (inItem == mParentItem)
    return YES;

  return [mParentItem isDescendentOfItem:inItem];
}

- (NSMutableArray*)children
{
  return nil;
}

- (int)numberOfChildren
{
  return 0;
}

- (HistoryItem*)childAtIndex:(int)inIndex
{
  return nil;
}

- (NSComparisonResult)compareURL:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return NSOrderedSame;
}

- (NSComparisonResult)compareTitle:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return NSOrderedSame;
}

- (NSComparisonResult)compareFirstVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return NSOrderedSame;
}

- (NSComparisonResult)compareLastVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return NSOrderedSame;
}

- (NSComparisonResult)compareVisitCount:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return NSOrderedSame;
}

- (NSComparisonResult)compareHostname:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return NSOrderedSame;
}

@end

#pragma mark -

@implementation HistoryCategoryItem

- (id)initWithDataSource:(HistoryDataSource*)inDataSource title:(NSString*)title childCapacity:(int)capacity
{
  if ((self = [super initWithDataSource:inDataSource]))
  {
    mTitle = [title retain];
    mChildren = [[NSMutableArray alloc] initWithCapacity:capacity];
  }
  return self;
}

- (void)dealloc
{
  [mTitle release];
  [mChildren release];
  [mUUIDString release];
  [super dealloc];
}

- (NSString*)title
{
  return mTitle;
}

- (NSString*)identifier
{
  if (!mUUIDString)
    mUUIDString = [[NSString stringWithUUID] retain];
  return mUUIDString;
}

- (NSString*)categoryIdentifier
{
  return [self identifier];
}

- (NSString*)url
{
  return @"";
}

- (NSDate*)firstVisit
{
  return nil;
}

- (NSDate*)lastVisit
{
  return nil;
}

- (NSNumber*)visitCount
{
  return nil;
}

- (NSString*)hostname
{
  return @"";
}

- (NSImage*)icon
{
  return [NSImage osFolderIcon];
}

- (NSMutableArray*)children
{
  return mChildren;
}

- (int)numberOfChildren
{
  return [mChildren count];
}

- (HistoryItem*)childAtIndex:(int)inIndex
{
  if (inIndex >= 0 && inIndex < (int)[mChildren count])
    return [mChildren objectAtIndex:inIndex];
    
  return nil;
}

- (void)addChild:(HistoryItem*)inChild
{
  [mChildren addObject:inChild];
  [inChild setParentItem:self];
}

- (void)insertChild:(HistoryItem*)inChild atIndex:(unsigned int)inIndex
{
  [mChildren insertObject:inChild atIndex:inIndex];
  [inChild setParentItem:self];
}

- (void)removeChild:(HistoryItem*)inChild
{
  [inChild setParentItem:nil];
  [mChildren removeObject:inChild];
}

- (void)addChildren:(NSArray*)inChildren
{
  [inChildren makeObjectsPerformSelector:@selector(setParentItem:) withObject:self];
  [mChildren addObjectsFromArray:inChildren];
}

- (void)removeAllChildren
{
  [mChildren makeObjectsPerformSelector:@selector(setParentItem:) withObject:nil];
  [mChildren removeAllObjects];
}

- (NSComparisonResult)compareURL:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if ([aItem isKindOfClass:[HistorySiteItem class]])
    return NSOrderedAscending;

  // sort category items on title, always ascending
  return [self compareTitle:aItem sortDescending:inDescending];
}

- (NSComparisonResult)compareTitle:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if ([aItem isKindOfClass:[HistorySiteItem class]])
    return NSOrderedAscending;

  NSComparisonResult result = [[self title] compare:[aItem title] options:NSCaseInsensitiveSearch];
  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareFirstVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if ([aItem isKindOfClass:[HistorySiteItem class]])
    return NSOrderedAscending;

  // sort on title, always ascending
  return [[self title] compare:[aItem title] options:NSCaseInsensitiveSearch];
}

- (NSComparisonResult)compareLastVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if ([aItem isKindOfClass:[HistorySiteItem class]])
    return NSOrderedAscending;

  // sort on title, always ascending
  return [[self title] compare:[aItem title] options:NSCaseInsensitiveSearch];
}

- (NSComparisonResult)compareHostname:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if ([aItem isKindOfClass:[HistorySiteItem class]])
    return NSOrderedAscending;

  // sort on title, always ascending
  return [[self title] compare:[aItem title] options:NSCaseInsensitiveSearch];
}

@end

#pragma mark -

@implementation HistorySiteCategoryItem

- (id)initWithDataSource:(HistoryDataSource*)inDataSource site:(NSString*)site title:(NSString*)title childCapacity:(int)capacity
{
  if ((self = [super initWithDataSource:inDataSource title:title childCapacity:capacity]))
  {
    mSite = [site retain];
  }
  return self;
}

- (void)dealloc
{
  [mSite release];
  [super dealloc];
}

- (NSString*)site
{
  return mSite;
}

- (NSString*)identifier
{
  return [NSString stringWithFormat:@"site:%d", mSite];
}

- (NSString*)description
{
  return [NSString stringWithFormat:@"HistorySiteCategoryItem %p, site %@", self, mSite];
}

@end // HistorySiteCategoryItem

#pragma mark -

@implementation HistoryDateCategoryItem

- (id)initWithDataSource:(HistoryDataSource*)inDataSource startDate:(NSDate*)startDate ageInDays:(int)days title:(NSString*)title childCapacity:(int)capacity
{
  if ((self = [super initWithDataSource:inDataSource title:title childCapacity:capacity]))
  {
    mStartDate = [startDate retain];
    mAgeInDays = days;
  }
  return self;
}

- (void)dealloc
{
  [mStartDate release];
  [super dealloc];
}

- (NSDate*)startDate
{
  return mStartDate;
}

- (BOOL)isTodayCategory
{
  return (mAgeInDays == 0);
}

- (NSString*)identifier
{
  return [NSString stringWithFormat:@"age_in_days:%d", mAgeInDays];
}

- (NSString*)description
{
  return [NSString stringWithFormat:@"HistoryDateCategoryItem %p, start date %@, age %d days", self, mStartDate, mAgeInDays];
}

- (NSComparisonResult)startDateCompare:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if (![aItem isKindOfClass:[HistoryDateCategoryItem class]])
    return NSOrderedAscending;

  // sort category items on date, always ascending
  return (NSComparisonResult)(-1 * (int)[[self startDate] compare:[(HistoryDateCategoryItem*)aItem startDate]]);
}

- (NSComparisonResult)compareURL:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return [self startDateCompare:aItem sortDescending:inDescending];
}

- (NSComparisonResult)compareTitle:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return [self startDateCompare:aItem sortDescending:inDescending];
}

- (NSComparisonResult)compareFirstVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return [self startDateCompare:aItem sortDescending:inDescending];
}

- (NSComparisonResult)compareLastVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  // always sort categories before sites
  if (![aItem isKindOfClass:[HistoryDateCategoryItem class]])
    return NSOrderedAscending;

  // sort category items on date, always ascending
  NSComparisonResult result = [[self startDate] compare:[(HistoryDateCategoryItem*)aItem startDate]];
  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareHostname:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  return [self startDateCompare:aItem sortDescending:inDescending];
}

@end // HistoryDateCategoryItem

#pragma mark -

@implementation HistorySiteItem

- (id)initWithDataSource:(HistoryDataSource*)inDataSource resultNode:(nsINavHistoryResultNode*)inNode
{
  if ((self = [super initWithDataSource:inDataSource]))
  {
    nsCAutoString url;
    if (NS_SUCCEEDED(inNode->GetUri(url))) {
      mURL = [[NSString alloc] initWith_nsACString:url];
      // nsINavHistoryResultNodes don't have any other unique identifier.
      mItemIdentifier = [[NSString alloc] initWith_nsACString:url];
    }

    nsCAutoString title;
    if (NS_SUCCEEDED(inNode->GetTitle(title)))
      mTitle = [[NSString alloc] initWith_nsACString:title];

    nsCOMPtr<nsIURI> uri;
    if (NS_SUCCEEDED(NS_NewURI(getter_AddRefs(uri), url))) {
      nsCAutoString hostname;
      uri->GetHost(hostname);
      mHostname = [[NSString alloc] initWith_nsACString:hostname];
    }

    if ([mHostname length] == 0 && [mURL hasPrefix:@"file://"])
      mHostname = [[NSString alloc] initWithString:@"local_file"];

    PRTime lastVisit;
    if (NS_SUCCEEDED(inNode->GetTime(&lastVisit)))
      mLastVisitDate = [[NSDate dateWithPRTime:lastVisit] retain];

    PRUint32 visitCount;
    if (NS_SUCCEEDED(inNode->GetAccessCount(&visitCount)))
      mVisitCount = [[NSNumber numberWithUnsignedInt:visitCount] retain];
  }
  return self;
}

- (void)dealloc
{
  [mItemIdentifier release];
  [mURL release];
  [mTitle release];
  [mHostname release];
  [mFirstVisitDate release];
  [mLastVisitDate release];
  [mVisitCount release];
  [mSiteIcon release];

  [super dealloc];
}

- (NSString*)url
{
  return mURL;
}

- (NSString*)title
{
  return mTitle;
}

//
// -firstVisit
//
// Returns the date of the first visit to this item. Finding this date involves
// querying Places for the earliest visit to the item's URL. Getting the first
// visit date of every item in history during initial load is slower by an order
// of magnitude versus finding it only when needed.
//
- (NSDate*)firstVisit
{
  if (!mFirstVisitDate) {
    // First set up a sensible default in case we can't get the true date.
    mFirstVisitDate = [[NSDate alloc] init];

    nsresult rv;
    nsCOMPtr<nsINavHistoryService> histSvc =
      do_GetService("@mozilla.org/browser/nav-history-service;1", &rv);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    nsCOMPtr<nsINavHistoryQuery> query;
    rv = histSvc->GetNewQuery(getter_AddRefs(query));
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    nsCOMPtr<nsIURI> uri;
    rv = NS_NewURI(getter_AddRefs(uri), [mURL UTF8String]);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    rv = query->SetUri(uri);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    nsCOMPtr<nsINavHistoryQueryOptions> options;
    rv = histSvc->GetNewQueryOptions(getter_AddRefs(options));
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    rv = options->SetResultType(nsINavHistoryQueryOptions::RESULTS_AS_VISIT);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    rv = options->SetSortingMode(nsINavHistoryQueryOptions::SORT_BY_DATE_ASCENDING);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    // We only want one result: the oldest.
    rv = options->SetMaxResults(1);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    nsCOMPtr<nsINavHistoryResult> result;
    rv = histSvc->ExecuteQuery(query, options, getter_AddRefs(result));
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    nsCOMPtr<nsINavHistoryContainerResultNode> rootNode;
    rv = result->GetRoot(getter_AddRefs(rootNode));
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    rv = rootNode->SetContainerOpen(PR_TRUE);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    PRUint32 childCount = 0;
    rv = rootNode->GetChildCount(&childCount);
    NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

    nsCOMPtr<nsINavHistoryResultNode> resultNode;
    if (childCount) {
      rv = rootNode->GetChild(0, getter_AddRefs(resultNode));
      NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

      PRTime firstVisit;
      rv = resultNode->GetTime(&firstVisit);
      NS_ENSURE_SUCCESS(rv, mFirstVisitDate);

      [mFirstVisitDate release];
      mFirstVisitDate = [[NSDate dateWithPRTime:firstVisit] retain];
    }
  }

  return mFirstVisitDate;
}

- (NSDate*)lastVisit
{
  return mLastVisitDate;
}

- (NSNumber*)visitCount
{
  return mVisitCount;
}

- (NSString*)hostname
{
  return mHostname;
}

- (NSString*)identifier
{
  return mItemIdentifier;
}

- (BOOL)isSiteItem
{
  return YES;
}

- (NSImage*)icon
{
  return [self iconAllowingLoad:NO];
}

- (NSImage*)iconAllowingLoad:(BOOL)inAllowLoad
{
  if (mSiteIcon)
    return mSiteIcon;

  NSImage* siteIcon = [[SiteIconProvider sharedFavoriteIconProvider] favoriteIconForPage:[self url]];
  if (siteIcon)
  {
    [self setSiteIcon:siteIcon];
    return mSiteIcon;
  }

  // firing off site icon loads here interferes with history submenu display
  // (maybe a slew of Carbon or other events causes events to get lost?)
  if (inAllowLoad && [mDataSource showSiteIcons])
  {
    if (!mAttemptedIconLoad)
    {
      // fire off site icon load
      [[SiteIconProvider sharedFavoriteIconProvider] fetchFavoriteIconForPage:[self url]
                                                             withIconLocation:nil
                                                                 allowNetwork:NO
                                                              notifyingClient:self];
      mAttemptedIconLoad = YES;
    }
  }

  return [NSImage imageNamed:@"globe_ico"];
}

- (void)setSiteIcon:(NSImage*)inImage
{
  [mSiteIcon autorelease];
  mSiteIcon = [inImage retain];
}

- (void)setTitle:(NSString*)inTitle
{
  [mTitle autorelease];
  mTitle = [inTitle retain];
}

- (void)setLastVisitDate:(NSDate*)inDate
{
  [mLastVisitDate release];
  mLastVisitDate = [inDate retain];
}


// ideally, we'd strip the protocol from the URL before comparing so that https:// doesn't
// sort after http://
- (NSComparisonResult)compareURL:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  NSComparisonResult result;
  // sort categories before sites
  if ([aItem isKindOfClass:[HistoryCategoryItem class]])
    result = NSOrderedDescending;
  else
    result = [mURL compare:[aItem url] options:NSCaseInsensitiveSearch];

  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareTitle:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  NSComparisonResult result;
  // sort categories before sites
  if ([aItem isKindOfClass:[HistoryCategoryItem class]])
    result = NSOrderedDescending;
  else
    result = [mTitle compare:[aItem title] options:NSCaseInsensitiveSearch];

  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareFirstVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  NSComparisonResult result;
  // sort categories before sites
  if ([aItem isKindOfClass:[HistoryCategoryItem class]])
    result = NSOrderedDescending;
  else
    result = [mFirstVisitDate compare:[aItem firstVisit]];

  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareLastVisitDate:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  NSComparisonResult result;
  // sort categories before sites
  if ([aItem isKindOfClass:[HistoryCategoryItem class]])
    result = NSOrderedDescending;
  else
    result = [mLastVisitDate compare:[aItem lastVisit]];

  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareVisitCount:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  NSComparisonResult result;
  // sort categories before sites
  if ([aItem isKindOfClass:[HistoryCategoryItem class]])
    result = NSOrderedDescending;
  else
    result = [mVisitCount compare:[aItem visitCount]];

  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (NSComparisonResult)compareHostname:(HistoryItem *)aItem sortDescending:(NSNumber*)inDescending
{
  NSComparisonResult result;

  // sort categories before sites
  if ([aItem isKindOfClass:[HistoryCategoryItem class]])
    result = NSOrderedDescending;
  else
    result = [mHostname compare:[aItem hostname] options:NSCaseInsensitiveSearch];

  return [inDescending boolValue] ? (NSComparisonResult)(-1 * (int)result) : result;
}

- (BOOL)matchesString:(NSString*)searchString inFieldWithTag:(int)tag
{
  switch (tag)
  {
    case eHistorySearchFieldAll:
      return (([[self title] rangeOfString:searchString options:NSCaseInsensitiveSearch].location != NSNotFound) ||
              ([[self url]   rangeOfString:searchString options:NSCaseInsensitiveSearch].location != NSNotFound));

    case eHistorySearchFieldTitle:
      return ([[self title] rangeOfString:searchString options:NSCaseInsensitiveSearch].location != NSNotFound);

    case eHistorySearchFieldURL:
      return ([[self url]   rangeOfString:searchString options:NSCaseInsensitiveSearch].location != NSNotFound);
  }

  return NO;    
}


@end
