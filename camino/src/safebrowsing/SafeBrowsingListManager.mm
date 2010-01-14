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
 * Sean Murphy.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sean Murphy <murph@seanmurph.com>
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

#import "SafeBrowsingListManager.h"
#import "PreferenceManager.h"
#import "GeckoPrefConstants.h"
#import "CmXULAppInfo.h"
#import "NSString+Gecko.h"
#import "NSString+Utils.h"
#import "CHBrowserService.h"

#include "nsIUrlClassifierDBService.h"
#include "nsIUrlListManager.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsString.h"
#include "SafeBrowsingTestDataUpdater.h"

static NSString *const kSendToURLAfterReportingPlaceholder = @"%CONTINUE_TO_URL%";
static NSString *const kReportedURLPlaceholder = @"%REPORTED_URL%";
static NSString *const kLanguageCodePlaceholder = @"%LANGUAGE%";
static NSString *const kBrowserNamePlaceholder = @"{moz:client}";
static NSString *const kBrowserVersionPlaceholder = @"{moz:version}";

@implementation SafeBrowsingList

+ (id)listWithName:(NSString *)aName type:(ESafeBrowsingListType)aType
{
  return [[[self alloc] initWithName:aName type:aType] autorelease];
}

- (id)initWithName:(NSString *)aName type:(ESafeBrowsingListType)aType
{
  if ((self = [super init])) {
    [self setName:aName];
    [self setType:aType];
  }
  return self;
}

- (void)dealloc
{
  [self setName:nil];
  [super dealloc];
}

- (NSString *)name
{
  return [[mName retain] autorelease]; 
}

- (void)setName:(NSString *)aName
{
  if (mName != aName) {
    [mName release];
    mName = [aName copy];
  }
}

- (ESafeBrowsingListType)type
{
  return mType;
}

- (void)setType:(ESafeBrowsingListType)aListType
{
  mType = aListType;
}

@end

#pragma mark -

@interface SafeBrowsingListManager (Private)

- (void)setInitialDataProviderValuesFromPreferences;
- (void)getDataProviderURLPref:(nsACString&)outPrefValue forKey:(NSString *)aKeyTemplate usingProviderID:(int)aProviderID;
- (void)registerObservationOfSafeBrowsingPreferences;
- (void)establishSecureKeyWithDataProvider;
- (void)reRegisterAllLists;
- (void)setUpdatesAreEnabled:(BOOL)aShouldEnableUpdates forListTypes:(SafeBrowsingListTypesMask)aListTypesToChange;
- (int)preferredDataProviderIdentifier;
- (void)insertTestURLsIntoDatabase;

@end

#pragma mark -

@implementation SafeBrowsingListManager

+ (SafeBrowsingListManager *)sharedInstance
{
  static SafeBrowsingListManager *sharedSafeBrowsingService = nil;
  if (!sharedSafeBrowsingService)
    sharedSafeBrowsingService = [[SafeBrowsingListManager alloc] init];

  return sharedSafeBrowsingService;
}

- (id)init
{
  self = [super init];
  if (self) {
    nsresult rv;
    nsCOMPtr<nsIUrlListManager> listManager = do_GetService("@mozilla.org/url-classifier/listmanager;1", &rv);
    if (NS_FAILED(rv)) {
      NSLog(@"Error initializing SafeBrowsingListManager: Failed to get the '@mozilla.org/url-classifier/listmanager' service");
      [self release];
      return nil;
    }
    mListManager = listManager;
    NS_IF_ADDREF(mListManager);

    mRegisteredLists = [[NSMutableArray alloc] init];

    // Register for xpcom shutdown so we know if the list manager goes away.
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(xpcomShutdown:)
                                                 name:XPCOMShutDownNotificationName
                                               object:nil];

    [self registerObservationOfSafeBrowsingPreferences];
    [self setInitialDataProviderValuesFromPreferences];
    [self insertTestURLsIntoDatabase];
  }
  return self;
}

- (void)dealloc
{
  NS_IF_RELEASE(mListManager);

  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [[PreferenceManager sharedInstance] removeObserver:self];

  [mRegisteredLists release];
  [super dealloc];
}

- (void)xpcomShutdown:(NSNotification *)notification
{
  // This nulls out the pointer
  NS_IF_RELEASE(mListManager);
}

#pragma mark -

- (void)registerObservationOfSafeBrowsingPreferences
{
  [[NSNotificationCenter defaultCenter] addObserver:self 
                                           selector:@selector(safeBrowsingPrefChanged:) 
                                               name:kPrefChangedNotificationName 
                                             object:self];

  [[PreferenceManager sharedInstance] addObserver:self 
                                          forPref:kGeckoPrefSafeBrowsingPhishingCheckingEnabled];
  [[PreferenceManager sharedInstance] addObserver:self 
                                          forPref:kGeckoPrefSafeBrowsingMalwareCheckingEnabled];                                          
  [[PreferenceManager sharedInstance] addObserver:self 
                                          forPref:kGeckoPrefSafeBrowsingDataProvider];  
}

// Important: A side-effect of setting these initial provider values is that the
// underlying nsIUrlListManager will drop knowledge of any lists it is managing.
// Call |reRegisterAllLists| to re-register them with the nsIUrlListManager.
- (void)setInitialDataProviderValuesFromPreferences
{
  if (!mListManager) 
    return;

  int providerID = [self preferredDataProviderIdentifier];

  nsCAutoString updateURL;
  [self getDataProviderURLPref:updateURL
                        forKey:kGeckoPrefSafeBrowsingDataProviderUpdateURL 
               usingProviderID:providerID];
  mListManager->SetUpdateUrl(updateURL);

  nsCAutoString getHashURL;
  [self getDataProviderURLPref:getHashURL 
                        forKey:kGeckoPrefSafeBrowsingDataProviderGetHashURL 
               usingProviderID:providerID];
  mListManager->SetGethashUrl(getHashURL);
}

// Manually insert our own test URLs into the safe browsing database. This provides
// a way for users to verify that the feature is enabled and working and also
// to experience what the UI looks like when visiting a dangerous site.
- (void)insertTestURLsIntoDatabase
{
  // Create a CHSafeBrowsingTestDataUpdater instance to simulate the update stream.
  // The object will be freed automatically by XPCOM's memory management.
  CHSafeBrowsingTestDataUpdater *testDataController = new CHSafeBrowsingTestDataUpdater();
  testDataController->InsertTestURLsIntoSafeBrowsingDb();
}

// Initializes a secure MAC with the data provider, which is used to decrypt list data. 
// This value is not applied in |setInitialDataProviderValuesFromPreferences| because communication 
// with the provider's server is required to set it, making it preferable to do so only when
// safe browsing is enabled by the user. This method will not clear any registered lists.
- (void)establishSecureKeyWithDataProvider
{
  nsCAutoString keyURL;
  [self getDataProviderURLPref:keyURL
                        forKey:kGeckoPrefSafeBrowsingDataProviderKeyURL
               usingProviderID:[self preferredDataProviderIdentifier]];
  mListManager->SetKeyUrl(keyURL);
}

- (void)getDataProviderURLPref:(nsACString&)outPrefValue
                        forKey:(NSString *)keyTemplate
               usingProviderID:(int)providerID
{
  // Insert the providerID into the preference key template.
  NSString *prefKeyForProvider = [NSString stringWithFormat:keyTemplate,
                                                            providerID];
  NSString *urlPrefTemplate = [[PreferenceManager sharedInstance] getStringPref:[prefKeyForProvider UTF8String]
                                                                    withSuccess:NULL];
  // Fill in certain URL parameter tokens with values.
  NSMutableString *urlPref = [[urlPrefTemplate mutableCopy] autorelease];
  [urlPref replaceOccurrencesOfString:kBrowserNamePlaceholder 
                           withString:[XULAppInfo name]
                              options:NULL
                                range:NSMakeRange(0, [urlPref length])];

  [urlPref replaceOccurrencesOfString:kBrowserVersionPlaceholder
                           withString:[XULAppInfo version] 
                              options:NULL 
                                range:NSMakeRange(0, [urlPref length])];

  outPrefValue.Assign([urlPref UTF8String]);
}

- (int)preferredDataProviderIdentifier
{
  PreferenceManager *prefManager = [PreferenceManager sharedInstance];
  BOOL prefFetchSuccess = NO;
  int providerID = [prefManager getIntPref:kGeckoPrefSafeBrowsingDataProvider 
                               withSuccess:&prefFetchSuccess];
  if (!prefFetchSuccess)
    providerID = 0;

  return providerID;
}

#pragma mark -

- (BOOL)updatesAreEnabledInPrefsForListType:(ESafeBrowsingListType)aListType
{
  PreferenceManager *prefManager = [PreferenceManager sharedInstance];

  switch (aListType) {
    case eSafeBrowsingPhishingListType:
      return [prefManager getBooleanPref:kGeckoPrefSafeBrowsingPhishingCheckingEnabled 
                             withSuccess:NULL];
    case eSafeBrowsingMalwareListType:
      return [prefManager getBooleanPref:kGeckoPrefSafeBrowsingMalwareCheckingEnabled 
                             withSuccess:NULL];
    case eSafeBrowsingAnyListType:
      return ([self updatesAreEnabledInPrefsForListType:eSafeBrowsingMalwareListType] ||
              [self updatesAreEnabledInPrefsForListType:eSafeBrowsingPhishingListType]);
    default:
      return NO;
  }
}

- (void)enableUpdateCheckingAccordingToPrefs
{
  SafeBrowsingListTypesMask enabledListTypes = 0;

  if ([self updatesAreEnabledInPrefsForListType:eSafeBrowsingPhishingListType])
    enabledListTypes |= eSafeBrowsingPhishingListType;
  if ([self updatesAreEnabledInPrefsForListType:eSafeBrowsingMalwareListType])
    enabledListTypes |= eSafeBrowsingMalwareListType;

  [self enableUpdateCheckingForListTypes:enabledListTypes];
}

- (void)enableUpdateCheckingForListTypes:(SafeBrowsingListTypesMask)aListTypesMask
{
  // Only set the key URL when updates will be enabled.
  [self establishSecureKeyWithDataProvider];
  [self setUpdatesAreEnabled:YES forListTypes:aListTypesMask];
}

- (void)disableUpdateCheckingForListTypes:(SafeBrowsingListTypesMask)aListTypesMask
{
  [self setUpdatesAreEnabled:NO forListTypes:aListTypesMask];
}

- (void)disableAllUpdateChecking
{
  [self setUpdatesAreEnabled:NO forListTypes:eSafeBrowsingAnyListType];
}

- (void)checkForUpdates
{
  if (!mListManager) 
    return;

  mListManager->CheckForUpdates();
}

#pragma mark -

- (void)registerListWithName:(NSString *)aListName type:(ESafeBrowsingListType)aListType
{
  if (!mListManager)
    return;

  if ([aListName length] < 0 || !aListType)
    return;

  // Ensure we aren't already managing this list already. (Names are unique within
  // a given data provider, so we don't need to check both name and type).
  NSEnumerator *registeredListEnum = [mRegisteredLists objectEnumerator];
  SafeBrowsingList *currentList = nil;
  while ((currentList = [registeredListEnum nextObject])) {
    if ([aListName isEqualToString:[currentList name]])
      return;
  }

  nsCAutoString listName;
  listName.Assign([aListName UTF8String]);
  PRBool registerSuccess = PR_FALSE;
  mListManager->RegisterTable(listName, PR_FALSE, &registerSuccess);
  if (!registerSuccess) {
    NSLog(@"Warning: SafeBrowsingListManager failed to register list: %@", aListName);
    return;
  }

  SafeBrowsingList *newList = [SafeBrowsingList listWithName:aListName type:aListType];
  [mRegisteredLists addObject:newList];
}

- (void)unregisterListWithName:(NSString *)aListName
{
  if (!mListManager) 
    return;

  // Enumerate a copy, since we'll be removing an object from the array.
  NSArray *registeredListsCopy = [[mRegisteredLists copy] autorelease];
  NSEnumerator *registeredListEnum = [registeredListsCopy objectEnumerator];
  SafeBrowsingList *currentList = nil;
  while ((currentList = [registeredListEnum nextObject])) {
    if ([[currentList name] isEqualToString:aListName]) {
      [mRegisteredLists removeObject:currentList];

      // There's no API to actually unregister the table with the list manager, 
      // so just disable its updates. Updates will never be enabled until it is
      // re-registered with us again.
      nsCAutoString listName;
      listName.Assign([aListName UTF8String]);
      mListManager->DisableUpdate(listName);
      return;
    }
  }
  NSLog(@"Warning: SafeBrowsingListManager failure to unregister; not managing a list named: %@", aListName); 
}

#pragma mark -

- (void)setUpdatesAreEnabled:(BOOL)aShouldEnableUpdates
                forListTypes:(SafeBrowsingListTypesMask)aListTypesToChange
{
  if (!mListManager) 
    return;

  NSEnumerator *registeredListEnum = [mRegisteredLists objectEnumerator];
  SafeBrowsingList *currentList = nil;
  while ((currentList = [registeredListEnum nextObject])) {
    if (aListTypesToChange & [currentList type]) {
      nsCAutoString listName;
      listName.Assign([[currentList name] UTF8String]);
      if (aShouldEnableUpdates)
        mListManager->EnableUpdate(listName);
      else
        mListManager->DisableUpdate(listName);
    }
  }
}

// Registers lists we are in charge of with the underlying nsIUrlListManager again.
- (void)reRegisterAllLists
{
  if (!mListManager) 
    return;

  NSEnumerator *registeredListEnum = [mRegisteredLists objectEnumerator];
  SafeBrowsingList *currentList = nil;
  while ((currentList = [registeredListEnum nextObject])) {
    nsCAutoString listName;
    listName.Assign([[currentList name] UTF8String]);
    PRBool registerSuccess = PR_FALSE;
    mListManager->RegisterTable(listName, PR_FALSE, &registerSuccess);
    if (!registerSuccess)
      NSLog(@"Warning: SafeBrowsingListManager failed to re-register list: %@", [currentList name]);
  }
}

#pragma mark -

static const int kStringComparisonEqual = 0;

- (void)safeBrowsingPrefChanged:(NSNotification *)notification
{
  const char *changedPrefKey = [[[notification userInfo] objectForKey:kPrefChangedPrefNameUserInfoKey] UTF8String];

  if (strcmp(changedPrefKey, kGeckoPrefSafeBrowsingPhishingCheckingEnabled) == kStringComparisonEqual) {
    [self setUpdatesAreEnabled:[self updatesAreEnabledInPrefsForListType:eSafeBrowsingPhishingListType]
                  forListTypes:eSafeBrowsingPhishingListType];
  }
  else if (strcmp(changedPrefKey, kGeckoPrefSafeBrowsingMalwareCheckingEnabled) == kStringComparisonEqual) {
    [self setUpdatesAreEnabled:[self updatesAreEnabledInPrefsForListType:eSafeBrowsingMalwareListType]
                  forListTypes:eSafeBrowsingMalwareListType];
  }
  else if (strcmp(changedPrefKey, kGeckoPrefSafeBrowsingDataProvider) == kStringComparisonEqual) {
    [self setInitialDataProviderValuesFromPreferences];
    [self reRegisterAllLists];
    [self enableUpdateCheckingAccordingToPrefs];
  }
}

- (NSString *)listReportingURLForPrefKey:(NSString *)prefKeyTemplate
                             urlToReport:(NSString *)urlToReport
{
  BOOL prefFetchSuccess = NO;
  PreferenceManager *prefManager = [PreferenceManager sharedInstance];

  // |prefKeyTemplate| contains a token for the current data provider, filling in
  // the provider to get an actual pref key.
  NSString *reportURLPrefKey = [NSString stringWithFormat:prefKeyTemplate,
                                                          [self preferredDataProviderIdentifier]];
  NSString *reportURLPrefValue = [prefManager getStringPref:[reportURLPrefKey UTF8String]
                                                withSuccess:&prefFetchSuccess];
  if (!prefFetchSuccess)
    return nil;

  // Fill in certain URL parameter tokens with values:

  NSMutableString *reportURL = [[reportURLPrefValue mutableCopy] autorelease];

  NSString *sendToURLAfterReport = 
    [prefManager getStringPref:kGeckoPrefSafeBrowsingSendToURLAfterReporting
                   withSuccess:&prefFetchSuccess];
  if (!prefFetchSuccess)
    return nil;

  [reportURL replaceOccurrencesOfString:kSendToURLAfterReportingPlaceholder
                             withString:sendToURLAfterReport
                                options:NULL
                                  range:NSMakeRange(0, [reportURL length])];

  [reportURL replaceOccurrencesOfString:kReportedURLPlaceholder
                             withString:urlToReport
                                options:NULL
                                  range:NSMakeRange(0, [reportURL length])];

  NSString *currentLanguageCode =
    [[PreferenceManager sharedInstance] getStringPref:kGeckoPrefUserAgentLocale
                                          withSuccess:&prefFetchSuccess];
  // Google uses "no" as the language code for Norwegian Bokmål, while Mac OS X
  // uses "nb" or "nb-no" (depending on the .lproj name), so convert to what 
  // Google expects in the URLs.
  if ([currentLanguageCode isEqualToString:@"nb"] || [currentLanguageCode isEqualToString:@"nb-no"])
    currentLanguageCode = @"no";
  if (!prefFetchSuccess)
    currentLanguageCode = @"en";

  [reportURL replaceOccurrencesOfString:kLanguageCodePlaceholder
                             withString:currentLanguageCode
                                options:NULL
                                  range:NSMakeRange(0, [reportURL length])];

  [reportURL replaceOccurrencesOfString:kBrowserNamePlaceholder
                             withString:[XULAppInfo name]
                                options:NULL
                                  range:NSMakeRange(0, [reportURL length])];
  
  return reportURL;
}

@end
