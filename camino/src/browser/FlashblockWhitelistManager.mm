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
 * Bryan Atwood
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Bryan Atwood <bryan.h.atwood@gmail.com>
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

#import "FlashblockWhitelistManager.h"

#import "PreferenceManager.h"
#import "NSString+Utils.h"

static FlashblockWhitelistManager* sFlashblockWhitelistManager = nil;
static NSString* const kFlashblockWhitelistChangedNotificationName = @"FlashblockWhitelistChanged";

@implementation FlashblockWhitelistManager

+ (FlashblockWhitelistManager*)sharedInstance
{
  if (!sFlashblockWhitelistManager)
    sFlashblockWhitelistManager = [[self alloc] init];

  return sFlashblockWhitelistManager;
}

- (id)init
{
  if ((self = [super init])) {
    [self loadWhitelistSites];

    [[PreferenceManager sharedInstance] addObserver:self forPref:kGeckoPrefFlashblockWhitelist];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(loadWhitelistSites)
                                                 name:kPrefChangedNotificationName
                                               object:self]; // since we added ourself as the Gecko pref observer
  }
  return self;
}

// Currently dealloc won't be called since there is no callback to tell the instance
// that the application is shutting down. But nothing happens here that isn't already
// taken care of when the application closes, so we don't need to enforce calling this.
- (void)dealloc
{
  [mFlashblockWhitelistPref release];
  [mFlashblockWhitelistSites release];
  [mFlashblockSiteCharSet release];

  sFlashblockWhitelistManager = nil;

  [[PreferenceManager sharedInstanceDontCreate] removeObserver:self forPref:kGeckoPrefFlashblockWhitelist];
  [[NSNotificationCenter defaultCenter] removeObserver:self];

  [super dealloc];
}

//
// -loadWhitelistSites:
//
// Load the list of Flashblock whitelist sites from the Gecko preference.
// Parse each site to allow subdomain matching within |isFlashAllowedForSite|.
// Post a notification that the whitelist has changed, so that observers can
// obtain an updated list using |whitelistArray|.
//
- (void)loadWhitelistSites
{
  NSString* whitelistPref = [[PreferenceManager sharedInstance] getStringPref:kGeckoPrefFlashblockWhitelist
                                                                  withSuccess:NULL];
  // Update array of whitelisted sites only if the preference has changed
  if (!mFlashblockWhitelistPref || ![mFlashblockWhitelistPref isEqualToString:whitelistPref]) {
    [mFlashblockWhitelistPref release];
    mFlashblockWhitelistPref = [whitelistPref retain];

    if (!mFlashblockWhitelistSites)
      mFlashblockWhitelistSites = [[NSMutableArray alloc] init];
    else
      [mFlashblockWhitelistSites removeAllObjects];

    // Whitelist is a string with format:
    // siteA.tld, www.siteB.tld, *.siteC.tld
    NSArray* whitelistSites = [mFlashblockWhitelistPref componentsSeparatedByString:@","];
    NSEnumerator* prefEnumerator = [whitelistSites objectEnumerator];
    NSString* prefSite;
    while ((prefSite = [prefEnumerator nextObject])) {
      // Require at least one '.' so that an entry of 'tld' or an empty string won't
      // match every site.
      if ([prefSite rangeOfString:@"."].location == NSNotFound)
        continue;

      prefSite = [[prefSite lowercaseString] stringByTrimmingCharactersInSet:
                  [NSCharacterSet whitespaceAndNewlineCharacterSet]];

      // Goal: For 'site.tld' whitelist, match www.site.tld and site.tld, but not thesite.tld.
      // Strategy: Append '.' to whitelist strings, then match the end of the site string
      // with the whitelist strings.  This will prevent 'site.tld' from matching the end of
      // thesite.tld but will match site.tld and www.site.tld. If site string is not in proper
      // format, it will (gracefully) not match.
      if ([prefSite rangeOfString:@"*."].location == 0)
        prefSite = [prefSite substringFromIndex:1];
      else
        prefSite = [@"." stringByAppendingString:prefSite];

      [mFlashblockWhitelistSites addObject:prefSite];
    }
    [[NSNotificationCenter defaultCenter] postNotificationName:kFlashblockWhitelistChangedNotificationName object:nil];
  }
}

- (void)saveFlashblockWhitelist
{
  NSArray* arrayToSave = [self whitelistArray];
  NSString* siteString = [arrayToSave componentsJoinedByString:@","];
  [[PreferenceManager sharedInstance] setPref:kGeckoPrefFlashblockWhitelist toString:siteString];
}

- (BOOL)addFlashblockWhitelistSite:(NSString*)aSite
{
  NSString* site = [aSite stringByRemovingCharactersInSet:
                    [NSCharacterSet whitespaceAndNewlineCharacterSet]];

  // Remove any protocol specifier (eg. "http://")
  NSRange protocolRange = [site rangeOfString:@"://"];
  if (protocolRange.location != NSNotFound)
    site = [site substringFromIndex:(protocolRange.location + protocolRange.length)];

  // Only add a Flashblock whitelist site if it's properly formatted and not already added
  if ([self canAddToWhitelist:site]) {
    [mFlashblockWhitelistSites addObject:site];
    [self saveFlashblockWhitelist];
    return YES;
  } else {
    return NO;
  }
}

- (void)removeFlashblockWhitelistSite:(NSString*)aSite
{
  [mFlashblockWhitelistSites removeObject:[@"." stringByAppendingString:aSite]];
  [self saveFlashblockWhitelist];
}

- (BOOL)isFlashAllowedForSite:(NSString*)aSite
{
  // As above, add "." to the beginning of site so that site.tld will match a whitelist
  // of site.tld that has been saved in the array as '.site.tld'
  aSite = [@"." stringByAppendingString:aSite];
  NSEnumerator* enumerator = [mFlashblockWhitelistSites objectEnumerator];
  NSString* whitelistSite;
  while ((whitelistSite = [enumerator nextObject])) {
    if ([aSite hasSuffix:whitelistSite])
      return YES;
  }

  return NO;
}

- (BOOL)canAddToWhitelist:(NSString*)aSite
{
  return ([self isValidFlashblockSite:aSite]
          && ![mFlashblockWhitelistSites containsObject:[@"." stringByAppendingString:aSite]]);
}

- (BOOL)isValidFlashblockSite:(NSString*)aSite
{
  if ([aSite length] == 0)
    return NO;

  // Reuse character string for hostname validation since it's expensive to make.
  if (!mFlashblockSiteCharSet) {
    NSMutableCharacterSet* charSet = [[[NSCharacterSet alphanumericCharacterSet] mutableCopy] autorelease];
    [charSet addCharactersInString:@"-_"];
    mFlashblockSiteCharSet = [charSet copy];
  }

  // Site may begin with '*.', in which case drop the first 2 characters.
  if ([aSite rangeOfString:@"*."].location == 0)
    aSite = [aSite substringFromIndex:2];

  // Split string on '.' and check components for valid string.
  NSArray* subdomains = [aSite componentsSeparatedByString:@"."];

  // There must be at least two components (e.g., something.tld)
  if ([subdomains count] < 2)
    return NO;

  NSEnumerator* enumerator = [subdomains objectEnumerator];
  NSString* subdomain;
  while ((subdomain = [enumerator nextObject])) {
    if ([subdomain length] == 0)
      return NO;
    NSScanner* scanner = [NSScanner scannerWithString:subdomain];
    [scanner scanCharactersFromSet:mFlashblockSiteCharSet intoString:NULL];
    if (![scanner isAtEnd])
      return NO;
  }

  return YES;
}

//
// -whitelistArray:
//
// Returns an array of the current entries in the whitelist. Removes the dots
// at the beginning of each string, making them suitable for displaying in
// the Web Features preference pane and for saving to the preference.
//
- (NSArray*)whitelistArray
{
  NSEnumerator* enumerator = [mFlashblockWhitelistSites objectEnumerator];
  NSString* singleSite;
  NSMutableArray* noDotArray = [NSMutableArray arrayWithCapacity:[mFlashblockWhitelistSites count]];
  while ((singleSite = (NSString*)[enumerator nextObject])) {
    if ([singleSite rangeOfString:@"*."].location == 0)
      [noDotArray addObject:[singleSite substringFromIndex:2]];
    else if ([singleSite rangeOfString:@"."].location == 0)
      [noDotArray addObject:[singleSite substringFromIndex:1]];
    else
      [noDotArray addObject:singleSite];
  }
  return noDotArray;
}

@end

