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
 *   Simon Woodside <sbwoodside@yahoo.com>
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
#import "NSPasteboard+Utils.h"
#import "NSDate+Utils.h"

#import "BrowserWindowController.h"
#import "HistoryDataSource.h"
#import "CHBrowserView.h"
#import "ExtendedOutlineView.h"
#import "PreferenceManager.h"
#import "HistoryItem.h"
#import "SiteIconProvider.h"

#import "BookmarkViewController.h"    // only for +greyStringWithItemCount

#import "nsCOMPtr.h"

#include "nsIBrowserHistory.h"
#include "nsIServiceManager.h"
#include "nsINavHistoryService.h"

#include "nsNetUtil.h"
#include "nsString.h"

#include "nsComponentManagerUtils.h"

NSString* const kHistoryViewByDate    = @"date";
NSString* const kHistoryViewBySite    = @"site";
NSString* const kHistoryViewFlat      = @"flat";


NSString* const kNotificationNameHistoryDataSourceChanged                     = @"history_changed";
NSString* const kNotificationHistoryDataSourceChangedUserInfoChangedItem      = @"changed_item";
NSString* const kNotificationHistoryDataSourceChangedUserInfoChangedItemOnly  = @"item_only";
NSString* const kNotificationNameHistoryDataSourceCleared                     = @"history_cleared";

struct SortData
{
  SEL       mSortSelector;
  NSNumber* mReverseSort;
};

static int HistoryItemSort(id firstItem, id secondItem, void* context)
{
  SortData* sortData = (SortData*)context;
  int comp = (int)[firstItem performSelector:sortData->mSortSelector withObject:secondItem withObject:sortData->mReverseSort];
  return comp;
}


#pragma mark -

// base class for a 'builder' object. This one just builds a flat list
@interface HistoryTreeBuilder : NSObject
{
  HistoryDataSource*    mDataSource;    // not retained (it owns us)
  HistoryCategoryItem*  mRootItem;      // retained
  SEL                   mSortSelector;
  BOOL                  mSortDescending;
}

// sets up the tree and sorts it
- (id)initWithDataSource:(HistoryDataSource*)inDataSource sortSelector:(SEL)sortSelector descending:(BOOL)descending;
- (void)buildTree;

- (HistoryItem*)rootItem;
- (HistoryItem*)addItem:(HistorySiteItem*)item;
- (HistoryItem*)removeItem:(HistorySiteItem*)item;

- (void)resortWithSelector:(SEL)sortSelector descending:(BOOL)descending;

// for internal use
- (void)buildTreeWithItems:(NSArray*)items;
- (void)resortFromItem:(HistoryItem*)item;

@end

#pragma mark -

@interface HistoryBySiteTreeBuilder : HistoryTreeBuilder
{
  NSMutableDictionary*    mSiteDictionary;
}

- (HistoryCategoryItem*)ensureHostCategoryForItem:(HistorySiteItem*)inItem;
- (void)removeSiteCategory:(HistoryCategoryItem*)item forHostname:(NSString*)hostname;

@end

#pragma mark -

@interface HistoryByDateTreeBuilder : HistoryTreeBuilder
{
  NSMutableArray*         mDateCategories;        // array of HistoryCategoryItems ordered recent -> old
}

- (void)setupDateCategories;
- (HistoryCategoryItem*)categoryItemForDate:(NSDate*)date;

@end

#pragma mark -

@interface HistoryDataSource(Private)

- (void)cleanupHistory;
- (void)rebuildHistory;

- (void)startBatching;
- (void)endBatching;

- (void)itemAdded:(HistorySiteItem*)item;
- (void)itemRemoved:(HistorySiteItem*)item;
- (void)itemChanged:(HistorySiteItem*)item;

- (void)notifyChanged:(HistoryItem*)changeRoot itemOnly:(BOOL)itemOnly;
- (SEL)selectorForSortColumn;

- (void)rebuildSearchResults;
- (void)sortSearchResults;

- (NSArray*)historyItems;
- (void)removeAllObjects;
- (HistorySiteItem*)itemWithIdentifier:(NSString*)identifier;

- (void)siteIconLoaded:(NSNotification*)inNotification;
- (void)checkForNewDay;

@end

#pragma mark -

// this little object exists simply to avoid a ref cycle between the refresh
// timer and the data source.
@interface HistoryTimerProxy : NSObject
{
  HistoryDataSource*    mHistoryDataSource;   // NOT owned
}

- (void)refreshTimerFired:(NSTimer*)timer;

@end

@implementation HistoryTimerProxy

- (id)initWithHistoryDataSource:(HistoryDataSource*)inDataSource
{
  if ((self = [super init]))
  {
    mHistoryDataSource = inDataSource;
  }
  return self;
}

- (void)refreshTimerFired:(NSTimer*)timer
{
  [mHistoryDataSource checkForNewDay];
}

@end // HistoryTimerProxy


#pragma mark -
#pragma mark --HistoryTreeBuilder--

@implementation HistoryTreeBuilder

- (id)initWithDataSource:(HistoryDataSource*)inDataSource sortSelector:(SEL)sortSelector descending:(BOOL)descending
{
  if ((self = [super init]))
  {
    mDataSource     = inDataSource;    // not retained
    mSortSelector   = sortSelector;
    mSortDescending = descending;
  }
  return self;
}

- (void)dealloc
{
  [mRootItem release];
  [super dealloc];
}

- (void)buildTree
{
  [self buildTreeWithItems:[mDataSource historyItems]];
}

- (HistoryItem*)rootItem
{
  return mRootItem;
}

- (HistoryItem*)addItem:(HistorySiteItem*)item
{
  [mRootItem addChild:item];
  [self resortFromItem:mRootItem];
  return mRootItem;
}

- (HistoryItem*)removeItem:(HistorySiteItem*)item
{
  [mRootItem removeChild:item];
  // no need to resort
  return mRootItem;
}

- (void)buildTreeWithItems:(NSArray*)items
{
  mRootItem = [[HistoryCategoryItem alloc] initWithDataSource:mDataSource title:@"" childCapacity:[items count]];

  [mRootItem addChildren:items];
  [self resortFromItem:mRootItem];
}

- (void)resortWithSelector:(SEL)sortSelector descending:(BOOL)descending
{
  mSortSelector = sortSelector;
  mSortDescending = descending;
  [self resortFromItem:nil];
}

// recursive sort
- (void)resortFromItem:(HistoryItem*)item
{
  SortData sortData;
  sortData.mSortSelector = mSortSelector;
  sortData.mReverseSort = [NSNumber numberWithBool:mSortDescending];
  
  if (!item)
    item = mRootItem;
    
  NSMutableArray* itemChildren = [item children];
  [itemChildren sortUsingFunction:HistoryItemSort context:&sortData];

  unsigned int numChildren = [itemChildren count];
  for (unsigned int i = 0; i < numChildren; i ++)
  {
    HistoryItem* curItem = [itemChildren objectAtIndex:i];
    if ([curItem isKindOfClass:[HistoryCategoryItem class]])
      [self resortFromItem:curItem];
  }
}

@end

#pragma mark -
#pragma mark --HistoryBySiteTreeBuilder--

@implementation HistoryBySiteTreeBuilder

- (id)initWithDataSource:(HistoryDataSource*)inDataSource sortSelector:(SEL)sortSelector descending:(BOOL)descending
{
  if ((self = [super initWithDataSource:inDataSource sortSelector:sortSelector descending:descending]))
  {
  }
  return self;
}

- (void)dealloc
{
  [mSiteDictionary release];
  [super dealloc];
}

- (HistoryCategoryItem*)ensureHostCategoryForItem:(HistorySiteItem*)inItem
{
  NSString* itemHostname = [inItem hostname];
  HistoryCategoryItem* hostCategory = [mSiteDictionary objectForKey:itemHostname];
  if (!hostCategory)
  {
    NSString* itemTitle = itemHostname;
    if ([itemHostname isEqualToString:@"local_file"])
      itemTitle = NSLocalizedString(@"LocalFilesCategoryTitle", @"");
    
    hostCategory = [[HistorySiteCategoryItem alloc] initWithDataSource:mDataSource site:itemHostname title:itemTitle childCapacity:10];
    [mSiteDictionary setObject:hostCategory forKey:itemHostname];
    [mRootItem addChild:hostCategory];
    [hostCategory release];
  }
  return hostCategory;
}

- (void)removeSiteCategory:(HistoryCategoryItem*)item forHostname:(NSString*)hostname
{
  [mSiteDictionary removeObjectForKey:hostname];
  [mRootItem removeChild:item];
}

- (HistoryItem*)addItem:(HistorySiteItem*)item
{
  HistoryCategoryItem* hostCategory = [mSiteDictionary objectForKey:[item hostname]];
  BOOL newHost = (hostCategory == nil);

  if (!hostCategory)
    hostCategory = [self ensureHostCategoryForItem:item];

  [hostCategory addChild:item];

  [self resortFromItem:newHost ? mRootItem : hostCategory];
  return hostCategory;
}

- (HistoryItem*)removeItem:(HistorySiteItem*)item
{
  NSString* itemHostname = [item hostname];
  HistoryCategoryItem* hostCategory = [mSiteDictionary objectForKey:itemHostname];
  [hostCategory removeChild:item];

  // is the category is now empty, remove it
  if ([hostCategory numberOfChildren] == 0)
  {
    [self removeSiteCategory:hostCategory forHostname:itemHostname];
    return mRootItem;
  }
  
  return hostCategory;
}

- (void)buildTreeWithItems:(NSArray*)items
{
  if (!mSiteDictionary)
    mSiteDictionary = [[NSMutableDictionary alloc] initWithCapacity:100];
  else
    [mSiteDictionary removeAllObjects];
  
  [mRootItem release];
  mRootItem = [[HistoryCategoryItem alloc] initWithDataSource:mDataSource title:@"" childCapacity:100];

  NSEnumerator* itemsEnum = [items objectEnumerator];
  HistorySiteItem* item;
  while ((item = [itemsEnum nextObject]))
  {
    HistoryCategoryItem* hostCategory = [self ensureHostCategoryForItem:item];
    [hostCategory addChild:item];
  }

  [self resortFromItem:mRootItem];
}

@end

#pragma mark -
#pragma mark --HistoryByDateTreeBuilder--

@implementation HistoryByDateTreeBuilder

- (id)initWithDataSource:(HistoryDataSource*)inDataSource sortSelector:(SEL)sortSelector descending:(BOOL)descending
{
  if ((self = [super initWithDataSource:inDataSource sortSelector:sortSelector descending:descending]))
  {
    [self setupDateCategories];
  }
  return self;
}

- (void)dealloc
{
  [mDateCategories release];
  [super dealloc];
}

- (void)setupDateCategories
{
  if (!mDateCategories)
    mDateCategories =  [[NSMutableArray alloc] initWithCapacity:9];
  else
    [mDateCategories removeAllObjects];  

  static const int kOlderThanAWeek = 7;
  static const int kDefaultExpireDays = 9;

  // Read the history cutoff so that we don't create too many folders
  BOOL gotPref = NO;
  int expireDays = [[PreferenceManager sharedInstance] getIntPref:kGeckoPrefHistoryLifetimeDays
                                                      withSuccess:&gotPref];
  if (!gotPref)
    expireDays = kDefaultExpireDays;
  else if (expireDays == 0) {
    // Return with an empty array, there is no history
    return;
  }

  NSDictionary* curCalendarLocale = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
  NSString* dateFormat = NSLocalizedString(@"HistoryMenuDateFormat", @"");
  
  NSCalendarDate* nowDate      = [NSCalendarDate calendarDate];
  NSCalendarDate* lastMidnight = [NSCalendarDate dateWithYear:[nowDate yearOfCommonEra]
                                                    month:[nowDate monthOfYear]
                                                      day:[nowDate dayOfMonth]
                                                     hour:0
                                                   minute:0
                                                   second:0
                                                 timeZone:[nowDate timeZone]];

  int dayLimit = (expireDays < kOlderThanAWeek ? expireDays : kOlderThanAWeek);
  for (int ageDays = 0; ageDays <= dayLimit; ++ageDays)
  {
    NSDate* dayStartDate;
    if (ageDays < kOlderThanAWeek) {
      dayStartDate = [lastMidnight dateByAddingYears:0
                                              months:0
                                                days:(-1)*ageDays
                                               hours:0
                                             minutes:0
                                             seconds:0];
    } else
      dayStartDate = [NSDate distantPast];
    
    NSString* itemTitle;
    int childCapacity = 10;
    int ageInDays = ageDays;
    if (ageDays == 0)
      itemTitle = NSLocalizedString(@"Today", @"");
    else if (ageDays == 1 )
      itemTitle = NSLocalizedString(@"Yesterday", @"");
    else if (ageDays == kOlderThanAWeek) {
      itemTitle = NSLocalizedString(@"HistoryMoreThanAWeek", @"");
      ageInDays = -1;
      childCapacity = 100;
    } else {
      itemTitle = [dayStartDate descriptionWithCalendarFormat:dateFormat
                                                     timeZone:nil
                                                       locale:curCalendarLocale];
    }

    HistoryCategoryItem* newItem = [[HistoryDateCategoryItem alloc] initWithDataSource:mDataSource
                                                                             startDate:dayStartDate
                                                                             ageInDays:ageInDays
                                                                                 title:itemTitle
                                                                         childCapacity:childCapacity];
    [mDateCategories addObject:newItem];
    [newItem release];
  }
}

- (HistoryCategoryItem*)categoryItemForDate:(NSDate*)date
{
  // find the first category whose start date is older  
  unsigned int numDateCategories = [mDateCategories count];
  for (unsigned int i = 0; i < numDateCategories; i ++)
  {
    HistoryDateCategoryItem* curItem = [mDateCategories objectAtIndex:i];
    NSComparisonResult comp = [date compare:[curItem startDate]];
    if (comp == NSOrderedDescending)
      return curItem;
  }
  
  // in theory we should never get here, because the last item has a date of 'distant past'
  return nil;
}

- (HistoryItem*)addItem:(HistorySiteItem*)item
{
  HistoryCategoryItem* dateCategory = [self categoryItemForDate:[item lastVisit]];
  [dateCategory addChild:item];

  [self resortFromItem:dateCategory];
  return dateCategory;
}

- (HistoryItem*)removeItem:(HistorySiteItem*)item
{
  HistoryCategoryItem* dateCategory = [self categoryItemForDate:[item lastVisit]];
  [dateCategory removeChild:item];
  // no need to resort
  return dateCategory;
}

- (void)buildTreeWithItems:(NSArray*)items
{
  NSEnumerator* itemsEnum = [items objectEnumerator];
  HistorySiteItem* item;
  while ((item = [itemsEnum nextObject]))
  {
    HistoryCategoryItem* dateCategory = [self categoryItemForDate:[item lastVisit]];
    [dateCategory addChild:item];
  }

  mRootItem = [[HistoryCategoryItem alloc] initWithDataSource:mDataSource title:@"" childCapacity:[mDateCategories count]];
  [mRootItem addChildren:mDateCategories];

  [self resortFromItem:mRootItem];
}

@end

#pragma mark -

class nsNavHistoryObserver : public nsINavHistoryObserver
{

public:
  nsNavHistoryObserver(HistoryDataSource* inDataSource)
  : mDataSource(inDataSource)
  {
  }
  
  virtual ~nsNavHistoryObserver()
  {
  }

protected:

  HistorySiteItem* HistoryItemFromURI(nsIURI* inURI)
  {
    nsCAutoString url;
    if (inURI && NS_SUCCEEDED(inURI->GetSpec(url))) {
      NSString* identifierString = [NSString stringWith_nsACString:url];
      return [mDataSource itemWithIdentifier:identifierString];
    }

    return nil;
  }

public:

  NS_DECL_ISUPPORTS

  //
  // OnVisit
  //
  // This is called each time a URI is visited. For the first visit, we need
  // to create a new HistorySiteItem so we can add it to the data source.
  // For all subsequent visits, we just need to find that item and update it
  // with a new last visit date.
  //
  NS_IMETHOD OnVisit(nsIURI *aURI, PRInt64 aVisitID, PRTime aTime, PRInt64 aSessionID,
                     PRInt64 aReferringID, PRUint32 aTransitionType, PRUint32 *aAdded)
  {
    NS_ENSURE_ARG_POINTER(aURI);
    NS_ENSURE_ARG_POINTER(aAdded);

    // Ignore embedded objects, such as images.
    if (aTransitionType == nsINavHistoryService::TRANSITION_EMBED)
      return NS_OK;

    @try { // make sure we don't throw out into gecko
      HistorySiteItem* item = HistoryItemFromURI(aURI);

      if (item) {
        NSDate* newDate = [NSDate dateWithPRTime:aTime];
        if (![[item lastVisit] isEqual:newDate]) {
          [item setLastVisitDate:newDate];
          [mDataSource itemChanged:item];
        }
      }
      else {
        nsresult rv;
        nsCOMPtr<nsINavHistoryService> histSvc =
          do_GetService("@mozilla.org/browser/nav-history-service;1", &rv);
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsINavHistoryQuery> query;
        rv = histSvc->GetNewQuery(getter_AddRefs(query));
        NS_ENSURE_SUCCESS(rv, rv);
        NS_ENSURE_TRUE(query, NS_ERROR_UNEXPECTED);

        rv = query->SetUri(aURI);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = query->SetMinVisits(1);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = query->SetMaxVisits(1);
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsINavHistoryQueryOptions> options;
        rv = histSvc->GetNewQueryOptions(getter_AddRefs(options));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsINavHistoryResult> result;
        rv = histSvc->ExecuteQuery(query, options, getter_AddRefs(result));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsINavHistoryContainerResultNode> rootNode;
        rv = result->GetRoot(getter_AddRefs(rootNode));
        NS_ENSURE_SUCCESS(rv, rv);
        NS_ENSURE_TRUE(rootNode, NS_ERROR_UNEXPECTED);

        rootNode->SetContainerOpen(PR_TRUE);

        // There should be 1 and only 1 result node returned from our query,
        // since this is the first visit.
        PRUint32 childCount = 0;
        rv = rootNode->GetChildCount(&childCount);
        NS_ENSURE_SUCCESS(rv, rv);
        NS_ENSURE_TRUE(childCount == 1, NS_ERROR_UNEXPECTED);

        nsCOMPtr<nsINavHistoryResultNode> resultNode;
        rv = rootNode->GetChild(0, getter_AddRefs(resultNode));
        NS_ENSURE_SUCCESS(rv, rv);

        item = [[HistorySiteItem alloc] initWithDataSource:mDataSource resultNode:resultNode];

        [mDataSource itemAdded:item];
        [item release];
      }
    }
    @catch (id exception) {
      NSLog(@"Exception caught in OnVisit: %@", exception);
    }

    return NS_OK;
  }

  NS_IMETHOD OnBeginUpdateBatch()
  {
    [mDataSource startBatching];
    return NS_OK;
  }

  NS_IMETHOD OnEndUpdateBatch()
  {
    [mDataSource endBatching];
    return NS_OK;
  }

  NS_IMETHOD OnTitleChanged(nsIURI *aURI, const nsAString & aPageTitle)
  {
    NS_ENSURE_ARG_POINTER(aURI);

    HistorySiteItem* item = HistoryItemFromURI(aURI);
    NS_ENSURE_TRUE(item, NS_ERROR_UNEXPECTED);

    @try { // make sure we don't throw out into gecko
      NSString* newTitle = [NSString stringWith_nsAString:aPageTitle];
      BOOL titleChanged = ![newTitle isEqualToString:[item title]];
      if (titleChanged) {
        [item setTitle:newTitle];
        [mDataSource itemChanged:item];
      }
    }
    @catch (id exception) {
      NSLog(@"Exception caught in OnTitleChanged: %@", exception);
    }
    return NS_OK;
  }

  NS_IMETHOD OnBeforeDeleteURI(nsIURI *aURI)
  {
    return NS_OK;
  }

  NS_IMETHOD OnDeleteURI(nsIURI *aURI)
  {
    NS_ENSURE_ARG_POINTER(aURI);

    @try { // make sure we don't throw out into gecko
      HistorySiteItem* item = HistoryItemFromURI(aURI);
      if (item)
        [mDataSource itemRemoved:item];
    }
    @catch (id exception) {
      NSLog(@"Exception caught in OnDeleteURI: %@", exception);
    }
    return NS_OK;
  }

  NS_IMETHOD OnClearHistory()
  {
    // Rather than calling |itemRemoved| for every item, just remove
    // all the items at once and rebuild.
    [mDataSource removeAllObjects];
    [mDataSource rebuildHistory];
    return NS_OK;
  }

  NS_IMETHOD OnPageChanged(nsIURI *aURI, PRUint32 aWhat, const nsAString & aValue)
  {
    return NS_OK;
  }

  NS_IMETHOD OnPageExpired(nsIURI *aURI, PRTime aVisitTime, PRBool aWholeEntry)
  {
    return NS_OK;
  }

protected:

  HistoryDataSource* mDataSource;  // Weak
};

NS_IMPL_ISUPPORTS1(nsNavHistoryObserver, nsINavHistoryObserver);

#pragma mark -

@implementation HistoryDataSource

- (id)init
{
  if ((self = [super init]))
  {
    nsCOMPtr<nsINavHistoryService> histSvc = do_GetService("@mozilla.org/browser/nav-history-service;1");
    mNavHistoryService = histSvc;
    if (!mNavHistoryService)
    {
      NSLog(@"Failed to initialize HistoryDataSource (couldn't get global history)");
      [self autorelease];
      return nil;
    }
    NS_IF_ADDREF(mNavHistoryService);

    mNavHistoryObserver = new nsNavHistoryObserver(self);
    NS_ADDREF(mNavHistoryObserver);
    mNavHistoryService->AddObserver(mNavHistoryObserver, PR_FALSE);
    
    mCurrentViewIdentifier = [kHistoryViewByDate retain];
    
    mSortColumn = [[NSString stringWithString:@"last_visit"] retain];    // save last settings in prefs?
    mSortDescending = YES;

    mSearchString = nil;
    
    mLastDayOfCommonEra = [[NSCalendarDate calendarDate] dayOfCommonEra];
    
    // Set up a timer to fire every 30 secs, to refresh the dates when midnight rolls around.
    // You might think it would be better to set up a timer to fire just after midnight,
    // but that might not be robust to clock adjustments.

    // the proxy avoids a ref cycle between the timer and self.
    // the timer retains its target, thus owning the proxy.
    HistoryTimerProxy* timerProxy = [[[HistoryTimerProxy alloc] initWithHistoryDataSource:self] autorelease];
    mRefreshTimer = [[NSTimer scheduledTimerWithTimeInterval:30.0
                                                      target:timerProxy   // takes ownership
                                                    selector:@selector(refreshTimerFired:)
                                                    userInfo:nil
                                                     repeats:YES] retain];

    // register for site icon loads
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(siteIconLoaded:)
                                                 name:SiteIconLoadNotificationName
                                               object:nil];

    mShowSiteIcons = [[PreferenceManager sharedInstance] getBooleanPref:kGeckoPrefEnableFavicons
                                                            withSuccess:NULL];
  }

  return self;
}

- (void)dealloc
{
  [self cleanupHistory];
  
  [mCurrentViewIdentifier release];
  [mSearchString release];

  [super dealloc];
}

- (void)cleanupHistory
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];

  if (mNavHistoryObserver)
  {
    if (mNavHistoryService)
      mNavHistoryService->RemoveObserver(mNavHistoryObserver);
    NS_RELEASE(mNavHistoryObserver);
  }

  NS_IF_RELEASE(mNavHistoryService);
  
  [mHistoryItems release];
  mHistoryItems = nil;

  [mHistoryItemsDictionary release];
  mHistoryItemsDictionary = nil;
  
  [mSearchResultsArray release];
  mSearchResultsArray = nil;
  
  [mTreeBuilder release];
  mTreeBuilder = nil;
  
  [mRefreshTimer invalidate];
  [mRefreshTimer release];
  mRefreshTimer = nil;
}

- (void)rebuildHistory
{
  [mTreeBuilder release];
  mTreeBuilder = nil;

  if ([mCurrentViewIdentifier isEqualToString:kHistoryViewFlat])
    mTreeBuilder = [[HistoryTreeBuilder alloc] initWithDataSource:self sortSelector:[self selectorForSortColumn] descending:mSortDescending];
  else if ([mCurrentViewIdentifier isEqualToString:kHistoryViewBySite])
    mTreeBuilder = [[HistoryBySiteTreeBuilder alloc] initWithDataSource:self sortSelector:[self selectorForSortColumn] descending:mSortDescending];
  else    // default to by date
    mTreeBuilder = [[HistoryByDateTreeBuilder alloc] initWithDataSource:self sortSelector:[self selectorForSortColumn] descending:mSortDescending];
  
  [mTreeBuilder buildTree];
  
  [self notifyChanged:nil itemOnly:NO];
}

- (void)startBatching
{
}

- (void)endBatching
{
  [self loadLazily];
}

- (void)itemAdded:(HistorySiteItem*)item
{
  [mHistoryItems addObject:item];
  [mHistoryItemsDictionary setObject:item forKey:[item identifier]];

  HistoryItem* parentCategory = [mTreeBuilder addItem:item];

  [self rebuildSearchResults];
  [self notifyChanged:parentCategory itemOnly:NO];
}

- (void)itemRemoved:(HistorySiteItem*)item
{
  HistoryItem* parentCategory = [mTreeBuilder removeItem:item];
  [mHistoryItems removeObject:item];
  [mHistoryItemsDictionary removeObjectForKey:[item identifier]];

  [self rebuildSearchResults];
  [self notifyChanged:parentCategory itemOnly:NO];
}

- (void)itemChanged:(HistorySiteItem*)item
{
  // we remove then re-add it
  HistoryItem* oldParent = [mTreeBuilder removeItem:item];
  HistoryItem* newParent = [mTreeBuilder addItem:item];
  
  [self rebuildSearchResults];

  [self notifyChanged:oldParent itemOnly:NO];
  if (oldParent != newParent)
    [self notifyChanged:newParent itemOnly:NO];
}

- (void)loadLazily
{
  nsCOMPtr<nsINavHistoryQuery> query;
  nsresult rv;
  rv = mNavHistoryService->GetNewQuery(getter_AddRefs(query));
  NS_ENSURE_SUCCESS(rv, /* void */);

  nsCOMPtr<nsINavHistoryQueryOptions> options;
  rv = mNavHistoryService->GetNewQueryOptions(getter_AddRefs(options));
  NS_ENSURE_SUCCESS(rv, /* void */);

  nsCOMPtr<nsINavHistoryResult> result;
  rv = mNavHistoryService->ExecuteQuery(query, options, getter_AddRefs(result));
  NS_ENSURE_SUCCESS(rv, /* void */);

  nsCOMPtr<nsINavHistoryContainerResultNode> rootNode;
  rv = result->GetRoot(getter_AddRefs(rootNode));
  NS_ENSURE_SUCCESS(rv, /* void */);

  rv = rootNode->SetContainerOpen(PR_TRUE);
  NS_ENSURE_SUCCESS(rv, /* void */);

  PRUint32 childCount = 0;
  rv = rootNode->GetChildCount(&childCount);
  NS_ENSURE_SUCCESS(rv, /* void */);

  if (!mHistoryItems)
    mHistoryItems = [[NSMutableArray alloc] initWithCapacity:childCount];
  else
    [mHistoryItems removeAllObjects];

  if (!mHistoryItemsDictionary)
    mHistoryItemsDictionary = [[NSMutableDictionary alloc] initWithCapacity:childCount];
  else
    [mHistoryItemsDictionary removeAllObjects];

  for (unsigned int i = 0; i < childCount; i++) {
    nsCOMPtr<nsINavHistoryResultNode> child;
    rootNode->GetChild(i, getter_AddRefs(child));
    if (!child)
      continue;
    HistorySiteItem* item = [[HistorySiteItem alloc] initWithDataSource:self resultNode:child];
    [mHistoryItems addObject:item];
    [mHistoryItemsDictionary setObject:item forKey:[item identifier]];
    [item release];
  }

  [self rebuildHistory];
}

- (HistoryItem*)rootItem
{
  return [mTreeBuilder rootItem];
}

- (BOOL)showSiteIcons
{
  return mShowSiteIcons;
}

- (void)notifyChanged:(HistoryItem*)changeRoot itemOnly:(BOOL)itemOnly
{
  // if we are displaying search results, make sure that updates
  // display any new results
  if (mSearchResultsArray)
  {
    changeRoot = nil;
    itemOnly = NO;
  }

  if (changeRoot == [mTreeBuilder rootItem])
  {
    changeRoot = nil;
    itemOnly = NO;
  }
  
  NSDictionary* userInfoDict = nil;
  if (changeRoot || itemOnly)
  {
    userInfoDict = [NSDictionary dictionaryWithObjectsAndKeys:
                             changeRoot, kNotificationHistoryDataSourceChangedUserInfoChangedItem,
     [NSNumber numberWithBool:itemOnly], kNotificationHistoryDataSourceChangedUserInfoChangedItemOnly,
                                         nil];
  }
  
  [[NSNotificationCenter defaultCenter] postNotificationName:kNotificationNameHistoryDataSourceChanged
        object:self
        userInfo:userInfoDict];
}

- (void)setHistoryView:(NSString*)inView
{
  if (![mCurrentViewIdentifier isEqualToString:inView])
  {
    [mCurrentViewIdentifier release];
    mCurrentViewIdentifier = [inView retain];
    [self rebuildHistory];
  } 
}

- (NSString*)historyView
{
  return mCurrentViewIdentifier;
}

- (void)setSortColumnIdentifier:(NSString*)sortColumnIdentifier
{
  NSString* oldSortColumn = mSortColumn;
  mSortColumn = [sortColumnIdentifier retain];
  [oldSortColumn release];

  [mTreeBuilder resortWithSelector:[self selectorForSortColumn] descending:mSortDescending];
  [self sortSearchResults];
  
  [self notifyChanged:nil itemOnly:NO];
}

- (NSString*)sortColumnIdentifier
{
  return mSortColumn;
}

- (BOOL)sortDescending
{
  return mSortDescending;
}

- (void)setSortDescending:(BOOL)inDescending
{
  if (inDescending != mSortDescending)
  {
    mSortDescending = inDescending;
    [mTreeBuilder resortWithSelector:[self selectorForSortColumn] descending:mSortDescending];
    [self sortSearchResults];

    [self notifyChanged:nil itemOnly:NO];
  }
}

- (SEL)selectorForSortColumn
{
  if ([mSortColumn isEqualToString:@"url"])
    return @selector(compareURL:sortDescending:);

  if ([mSortColumn isEqualToString:@"title"])
    return @selector(compareTitle:sortDescending:);

  if ([mSortColumn isEqualToString:@"first_visit"])
    return @selector(compareFirstVisitDate:sortDescending:);

  if ([mSortColumn isEqualToString:@"last_visit"])
    return @selector(compareLastVisitDate:sortDescending:);

  if ([mSortColumn isEqualToString:@"visit_count"])
    return @selector(compareVisitCount:sortDescending:);

  if ([mSortColumn isEqualToString:@"hostname"])
    return @selector(compareHostname:sortDescending:);

  return @selector(compareLastVisitDate:sortDescending:);
}

- (NSArray*)historyItems
{
  return mHistoryItems;
}

- (HistorySiteItem*)itemWithIdentifier:(NSString*)identifier
{
  return [mHistoryItemsDictionary objectForKey:identifier];
}

- (void)siteIconLoaded:(NSNotification*)inNotification
{
  HistoryItem* theItem = [inNotification object];
  // if it's not a site item, or it doesn't belong to this data source, ignore it
  // (we instantiate multiple data sources)
  if (![theItem isKindOfClass:[HistorySiteItem class]] || [theItem dataSource] != self)
    return;

  NSImage* iconImage = [[inNotification userInfo] objectForKey:SiteIconLoadImageKey];
  if (iconImage)
  {
    [(HistorySiteItem*)theItem setSiteIcon:iconImage];
    [self notifyChanged:theItem itemOnly:YES];
  }
}

- (void)checkForNewDay
{
  int curDayOfCommonEra = [[NSCalendarDate calendarDate] dayOfCommonEra];
  // it's a brand new day...
  if (curDayOfCommonEra != mLastDayOfCommonEra)
  {
    [self rebuildHistory];
    mLastDayOfCommonEra = curDayOfCommonEra;
  }
}

- (void)searchFor:(NSString*)searchString inFieldWithTag:(int)tag
{
  [mSearchString autorelease];
  mSearchString = [searchString retain];

  mSearchFieldTag = tag;

  if (!mSearchResultsArray)
    mSearchResultsArray = [[NSMutableArray alloc] initWithCapacity:100];

  [self rebuildSearchResults];
}

- (void)rebuildSearchResults
{
  // if mSearchResultsArray is null, we're not showing search results
  if (!mSearchResultsArray) return;
  
  [mSearchResultsArray removeAllObjects];

  NSEnumerator* itemsEnumerator = [mHistoryItems objectEnumerator];
  id obj;
  while ((obj = [itemsEnumerator nextObject]))
  {
    if ([obj matchesString:mSearchString inFieldWithTag:mSearchFieldTag])
      [mSearchResultsArray addObject:obj];
  }
  
  [self sortSearchResults];
}

- (void)sortSearchResults
{
  if (!mSearchResultsArray) return;

  SortData sortData;
  sortData.mSortSelector = [self selectorForSortColumn];
  sortData.mReverseSort = [NSNumber numberWithBool:mSortDescending];

  [mSearchResultsArray sortUsingFunction:HistoryItemSort context:&sortData];
}

- (void)clearSearchResults
{
  [mSearchResultsArray release];
  mSearchResultsArray = nil;
  [mSearchString release];
  mSearchString = nil;
}

- (void)removeItem:(HistorySiteItem*)item
{
  nsCOMPtr<nsIURI> doomedURI;
  NS_NewURI(getter_AddRefs(doomedURI), [[item url] UTF8String]);
  if (doomedURI)
  {
    nsCOMPtr<nsIBrowserHistory> hist(do_QueryInterface(mNavHistoryService));
    if (hist)
      hist->RemovePage(doomedURI);
    [self itemRemoved:item];
  }
}

- (void)removeAllObjects
{
  [mHistoryItems removeAllObjects];
  [mHistoryItemsDictionary removeAllObjects];

  [[NSNotificationCenter defaultCenter]
      postNotificationName:kNotificationNameHistoryDataSourceCleared
                    object:self
                  userInfo:nil];
}

#pragma mark -

// Implementation of NSOutlineViewDataSource protocol

- (id)outlineView:(NSOutlineView*)aOutlineView child:(int)aIndex ofItem:(id)item
{
  if (mSearchResultsArray)
  {
    if (!item)
      return [mSearchResultsArray objectAtIndex:aIndex];
    return nil;
  }

  if (!item)
    item = [mTreeBuilder rootItem];
  return [item childAtIndex:aIndex];
}

- (int)outlineView:(NSOutlineView*)outlineView numberOfChildrenOfItem:(id)item
{
  if (mSearchResultsArray)
  {
    if (!item)
      return [mSearchResultsArray count];
    return 0;
  }

  if (!item)
    item = [mTreeBuilder rootItem];
  return [item numberOfChildren];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
  return [item isKindOfClass:[HistoryCategoryItem class]];
}

// identifiers: title url last_visit first_visit
- (id)outlineView:(NSOutlineView*)outlineView objectValueForTableColumn:(NSTableColumn*)aTableColumn byItem:(id)item
{
  if ([[aTableColumn identifier] isEqualToString:@"title"])
    return [item title];

  if ([item isKindOfClass:[HistorySiteItem class]])
  {
    if ([[aTableColumn identifier] isEqualToString:@"url"])
      return [item url];

    if ([[aTableColumn identifier] isEqualToString:@"last_visit"])
      return [item lastVisit];

    if ([[aTableColumn identifier] isEqualToString:@"first_visit"])
      return [item firstVisit];
  }

  if ([item isKindOfClass:[HistoryCategoryItem class]])
  {
    if ([[aTableColumn identifier] isEqualToString:@"url"])
      return [BookmarkViewController greyStringWithItemCount:[item numberOfChildren]];
  }
  
  return nil;

// TODO truncate string
//  - (void)truncateToWidth:(float)maxWidth at:kTruncateAtMiddle withAttributes:(NSDictionary *)attributes
}

- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray*)items toPasteboard:(NSPasteboard*)pboard
{
  //Need to filter out folders from the list, only allow the urls to be dragged
  NSMutableArray* urlsArray   = [[[NSMutableArray alloc] init] autorelease];
  NSMutableArray* titlesArray = [[[NSMutableArray alloc] init] autorelease];

  NSEnumerator *enumerator = [items objectEnumerator];
  id curItem;
  while ((curItem = [enumerator nextObject]))
  {
    if ([curItem isSiteItem])
    {
      NSString* itemURL = [curItem url];
      NSString* cleanedTitle = [[curItem title] stringByReplacingCharactersInSet:[NSCharacterSet controlCharacterSet] withString:@" "];

      [urlsArray addObject:itemURL];
      [titlesArray addObject:cleanedTitle];
    }
  }

  if ([urlsArray count] > 0)
  {
    [pboard declareURLPasteboardWithAdditionalTypes:[NSArray array] owner:self];
    [pboard setURLs:urlsArray withTitles:titlesArray];
    return YES;
  }
  
  return NO;
}

@end
