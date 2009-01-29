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

#import <Cocoa/Cocoa.h>

class nsIUrlListManager;

// Used to specify a type of list which can be obtained from the data provider.
typedef enum {
  eSafeBrowsingPhishingListType = (1 << 0),
  eSafeBrowsingMalwareListType  = (1 << 1),
  eSafeBrowsingAnyListType      = 0xFFFFFFFF
} ESafeBrowsingListType;

// A bit mask specifying a combination of |ESafeBrowsingListType| values.
typedef int SafeBrowsingListTypesMask;

#pragma mark -

//
// SafeBrowsingList
//
// Simple data object used by the SafeBrowsingListManager 
// to represent a data provider list.
//
@interface SafeBrowsingList : NSObject
{
 @private
  NSString              *mName;
  ESafeBrowsingListType  mType;
}

+ (id)listWithName:(NSString *)aName type:(ESafeBrowsingListType)aType;
- (id)initWithName:(NSString *)aName type:(ESafeBrowsingListType)aType;

- (NSString *)name;
- (void)setName:(NSString *)aName;
- (ESafeBrowsingListType)type;
- (void)setType:(ESafeBrowsingListType)aListType;

@end

#pragma mark -

//
// SafeBrowsingListManager
//
// A shared object offering control over the maintenance and update process of the
// local database of safe browsing information, which is populated from lists made 
// available by remote data providers. Data provider properties are stored in Gecko's
// preferences.
//
//
@interface SafeBrowsingListManager : NSObject
{
 @private
  nsIUrlListManager *mListManager;               // strong
  NSMutableArray    *mRegisteredLists;           // strong; An array of SafeBrowsingList objects.
}

+ (SafeBrowsingListManager *)sharedInstance;

// The SafeBrowsingListManager will determine which list types to enable updates on based
// on current preference values.
- (void)enableUpdateCheckingAccordingToPrefs;

// |listTypesMask| should contain |ESafeBrowsingListType| values combined with the C bitwise
// OR operator.
- (void)enableUpdateCheckingForListTypes:(SafeBrowsingListTypesMask)aListTypesMask;
- (void)disableUpdateCheckingForListTypes:(SafeBrowsingListTypesMask)aListTypesMask;
- (void)disableAllUpdateChecking;

// Calling this method with |eSafeBrowsingAnyListType| will return YES if update checking
// is enabled for at least one list type.
- (BOOL)updatesAreEnabledInPrefsForListType:(ESafeBrowsingListType)aListType;

// Registered lists are fetched from the data provider during each update cycle.
- (void)registerListWithName:(NSString *)aListName type:(ESafeBrowsingListType)aListType;
- (void)unregisterListWithName:(NSString *)aListName;

// While update checks are performed periodically on enabled lists, this method forces a manual
// check to take place immediately.
- (void)checkForUpdates;

@end
