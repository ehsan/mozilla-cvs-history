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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Bryan Atwood
 * Portions created by the Initial Developer are Copyright (C) 2007
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

#import <Cocoa/Cocoa.h>

//
// interface FlashblockWhitelistManager
//
// A singleton class to manage list of sites where Flash is allowed
// when otherwise Flash is blocked
//
@interface FlashblockWhitelistManager : NSObject
{
  NSString*                 mFlashblockWhitelistPref;      // STRONG
  NSMutableArray*           mFlashblockWhitelistSites;     // STRONG
  NSCharacterSet*           mFlashblockSiteCharSet;        // STRONG
}

// Returns the shared FlashblockWhitelistManager instance.
+ (FlashblockWhitelistManager*)sharedInstance;

// Loads whitelisted sites from the preference.
- (void)loadWhitelistSites;

// Sets the Flashblock whitelist Gecko preference to the current whitelist array.
- (void)saveFlashblockWhitelist;

// Adds the site to the Flashblock whitelist, if valid and not already present.
// aSite should be just the host (site.tld, sub.site.tld, or *.site.tld).
// Returns YES if the site was added.
- (BOOL)addFlashblockWhitelistSite:(NSString*)aSite;

// Removes the site from the whitelist, if present, and saves the modified list.
// aSite should be just the host (site.tld, sub.site.tld, or *.site.tld).
- (void)removeFlashblockWhitelistSite:(NSString*)aSite;

// Returns YES if the site or any of its subdomains is in the whitelist.
// aSite should be just the host (site.tld or sub.site.tld).
- (BOOL)isFlashAllowedForSite:(NSString*)aSite;

// Returns YES if the string contains a valid site and is not already in the
// whitelist.
// aSite should be just the host (site.tld, sub.site.tld, or *.site.tld).
- (BOOL)canAddToWhitelist:(NSString*)aSite;

// Returns YES if string contains a valid site.
// aSite should be just the host (site.tld, sub.site.tld, or *.site.tld).
- (BOOL)isValidFlashblockSite:(NSString*)aSite;

// The current array of whitelist sites.
- (NSArray*)whitelistArray;

@end
