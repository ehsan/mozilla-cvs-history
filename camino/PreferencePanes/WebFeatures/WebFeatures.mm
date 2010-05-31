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
 *   William Dell Wisner <william@dell.wisner.name>
 *   Josh Aas <josh@mozilla.com>
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

#import "WebFeatures.h"

#import "NSString+Utils.h"
#import "CHPermissionManager.h"
#import "ExtendedTableView.h"             
#import "GeckoPrefConstants.h"
#import "FlashblockWhitelistManager.h"

// need to match the strings in PreferenceManager.mm
static NSString* const AdBlockingChangedNotificationName = @"AdBlockingChanged";
static NSString* const kFlashblockChangedNotificationName = @"FlashblockChanged";
static NSString* const kFlashblockWhitelistChangedNotificationName = @"FlashblockWhitelistChanged";

// for annoyance blocker prefs
const int kAnnoyancePrefNone = 1;
const int kAnnoyancePrefAll  = 2;
const int kAnnoyancePrefSome = 3;

@interface OrgMozillaCaminoPreferenceWebFeatures(Private)

- (int)annoyingWindowPrefs;
- (int)popupIndexForCurrentTabFocusPref;
- (int)preventAnimationCheckboxState;
- (BOOL)isFlashblockAllowed;
- (void)updateFlashblock;
- (void)populatePermissionCache;

@end

@implementation OrgMozillaCaminoPreferenceWebFeatures

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];

  [mFlashblockSites release];
  [super dealloc];
}

- (void)mainViewDidLoad
{
  // Set up policy popups
  NSPopUpButtonCell *popupButtonCell = [mPolicyColumn dataCell];
  [popupButtonCell setEditable:YES];
  [popupButtonCell addItemsWithTitles:[NSArray arrayWithObjects:NSLocalizedString(@"Allow", nil),
                                                                NSLocalizedString(@"Deny", nil),
                                                                nil]];
}

- (void)willSelect
{
  BOOL gotPref = NO;

  // Set initial value on JavaScript checkbox.
  BOOL jsEnabled = [self getBooleanPref:kGeckoPrefEnableJavascript withSuccess:&gotPref] && gotPref;
  [mEnableJS setState:jsEnabled];

  // Set initial value on Java checkbox, and disable it if plugins are off
  BOOL pluginsEnabled = [self getBooleanPref:kGeckoPrefEnablePlugins withSuccess:&gotPref] || !gotPref;
  [mEnableJava setEnabled:pluginsEnabled];
  BOOL javaEnabled = pluginsEnabled && [self getBooleanPref:kGeckoPrefEnableJava withSuccess:NULL];
  [mEnableJava setState:javaEnabled];

  // set initial value on popup blocking checkbox and disable the whitelist
  // button if it's off
  BOOL enablePopupBlocking = [self getBooleanPref:kGeckoPrefBlockPopups withSuccess:&gotPref] && gotPref;  
  [mEnablePopupBlocking setState:enablePopupBlocking];
  [mEditWhitelist setEnabled:enablePopupBlocking];

  // set initial value on annoyance blocker checkbox.
  if([self annoyingWindowPrefs] == kAnnoyancePrefAll)
    [mEnableAnnoyanceBlocker setState:NSOnState];
  else if([self annoyingWindowPrefs] == kAnnoyancePrefNone)
    [mEnableAnnoyanceBlocker setState:NSOffState];
  else // annoyingWindowPrefs == kAnnoyancePrefSome
    [mEnableAnnoyanceBlocker setState:NSMixedState];

  [mPreventAnimation setState:[self preventAnimationCheckboxState]];

  BOOL enableAdBlock = [self getBooleanPref:kGeckoPrefBlockAds withSuccess:&gotPref];
  [mEnableAdBlocking setState:enableAdBlock];

  // Only allow Flashblock if dependencies are set correctly
  BOOL flashblockAllowed = [self isFlashblockAllowed];
  [mEnableFlashblock setEnabled:flashblockAllowed];
 
  if (flashblockAllowed) {
    BOOL enableFlashblock = [self getBooleanPref:kGeckoPrefBlockFlash withSuccess:NULL];
    [mEnableFlashblock setState:(enableFlashblock ? NSOnState : NSOffState)];
    [mEditFlashblockWhitelist setEnabled:enableFlashblock];
  }
  else {
    [mEditFlashblockWhitelist setEnabled:NO];
  }

  // Set tab focus popup.
  [mTabBehaviorPopup selectItemAtIndex:[self popupIndexForCurrentTabFocusPref]];
}

//
// -clickEnableJS:
//
// Enable and disable JavaScript
//
-(IBAction) clickEnableJS:(id)sender
{
  [self setPref:kGeckoPrefEnableJavascript toBoolean:([sender state] == NSOnState)];

  // Flashblock depends on JavaScript, so make sure to update the Flashblock settings.
  [self updateFlashblock];
}

//
// -clickEnableJava:
//
// Enable and disable Java
//
-(IBAction) clickEnableJava:(id)sender
{
  [self setPref:kGeckoPrefEnableJava toBoolean:([sender state] == NSOnState)];
}

//
// -clickEnableAdBlocking:
//
// Enable and disable ad blocking via an ad_blocking.css file that we provide
// in our package.
//
- (IBAction)clickEnableAdBlocking:(id)sender
{
  [self setPref:kGeckoPrefBlockAds toBoolean:([sender state] == NSOnState)];
  [[NSNotificationCenter defaultCenter] postNotificationName:AdBlockingChangedNotificationName object:nil]; 
}

//
// clickEnablePopupBlocking
//
// Enable and disable mozilla's popup blocking feature. We use a combination of 
// two prefs to suppress bad popups.
//
- (IBAction)clickEnablePopupBlocking:(id)sender
{
  [self setPref:kGeckoPrefBlockPopups toBoolean:([sender state] == NSOnState)];
  [mEditWhitelist setEnabled:[sender state]];
}

//
// -clickPreventAnimation:
//
// Enable and disable mozilla's limiting of how animated images repeat
//
-(IBAction) clickPreventAnimation:(id)sender
{
  [sender setAllowsMixedState:NO];
  [self setPref:kGeckoPrefImageAnimationBehavior
       toString:([sender state] ? kImageAnimationOnce : kImageAnimationLoop)];
}

//
// clickEnableFlashblock:
//
// Enable and disable Flashblock.  When enabled, an icon is displayed and the 
// Flash animation plays when the user clicks it.  When disabled, Flash plays 
// automatically.
//
-(IBAction) clickEnableFlashblock:(id)sender
{
  [self setPref:kGeckoPrefBlockFlash toBoolean:([sender state] == NSOnState)];
  [[NSNotificationCenter defaultCenter] postNotificationName:kFlashblockChangedNotificationName object:nil];
  [mEditFlashblockWhitelist setEnabled:[sender state]];
}

//
// populatePermissionCache
//
// Builds a popup-blocking cache that we can quickly refer to later.
//
-(void) populatePermissionCache
{
  if (mCachedPermissions)
    [mCachedPermissions release];
  mCachedPermissions = [[[CHPermissionManager permissionManager]
                            permissionsOfType:CHPermissionTypePopup] mutableCopy];
  if (!mCachedPermissions)
    mCachedPermissions = [[NSMutableArray alloc] init];
}

//
// editWhitelist:
//
// put up a sheet to allow people to edit the popup blocker whitelist
//
-(IBAction) editWhitelist:(id)sender
{
  // build parallel permission list for speed with a lot of blocked sites
  [self populatePermissionCache];

  [NSApp beginSheet:mWhitelistPanel
     modalForWindow:[mEditWhitelist window]   // any old window accessor
      modalDelegate:self
     didEndSelector:@selector(editWhitelistSheetDidEnd:returnCode:contextInfo:)
        contextInfo:NULL];

  // ensure a row is selected (cocoa doesn't do this for us, but will keep
  // us from unselecting a row once one is set; go figure).
  if ([mWhitelistTable numberOfRows] > 0) {
    [mWhitelistTable selectRowIndexes:[NSIndexSet indexSetWithIndex:0]
                 byExtendingSelection:NO];
  }

  [mWhitelistTable setDeleteAction:@selector(removeWhitelistSite:)];
  [mWhitelistTable setTarget:self];

  [mAddButton setEnabled:NO];

  // we shouldn't need to do this, but the scrollbar won't enable unless we
  // force the table to reload its data. Oddly it gets the number of rows correct,
  // it just forgets to tell the scrollbar. *shrug*
  [mWhitelistTable reloadData];
}

// whitelist sheet methods
-(IBAction) editWhitelistDone:(id)aSender
{
  // save stuff??

  [mWhitelistPanel orderOut:self];
  [NSApp endSheet:mWhitelistPanel];

  [mCachedPermissions release];
  mCachedPermissions = nil;
}

-(IBAction) removeWhitelistSite:(id)aSender
{
  CHPermissionManager* permManager = [CHPermissionManager permissionManager];

  // Walk the selected rows backwards, removing permissions.
  NSIndexSet* selectedIndexes = [mWhitelistTable selectedRowIndexes];
  for (unsigned int i = [selectedIndexes lastIndex];
       i != NSNotFound;
       i = [selectedIndexes indexLessThanIndex:i])
  {
    [permManager removePermissionForHost:[[mCachedPermissions objectAtIndex:i] host]
                                    type:CHPermissionTypePopup];
    [mCachedPermissions removeObjectAtIndex:i];
  }

  [mWhitelistTable reloadData];

  // Select the row after the last deleted row.
  if ([mWhitelistTable numberOfRows] > 0) {
    int rowToSelect = [selectedIndexes lastIndex] - ([selectedIndexes count] - 1);
    if ((rowToSelect < 0) || (rowToSelect >= [mWhitelistTable numberOfRows]))
      rowToSelect = [mWhitelistTable numberOfRows] - 1;
    [mWhitelistTable selectRowIndexes:[NSIndexSet indexSetWithIndex:rowToSelect]
                 byExtendingSelection:NO];
  }
}

//
// addWhitelistSite:
//
// adds a new site to the permission manager whitelist for popups
//
-(IBAction) addWhitelistSite:(id)sender
{
  NSString* host = [[mAddField stringValue] stringByRemovingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

  [[CHPermissionManager permissionManager] setPolicy:CHPermissionAllow
                                             forHost:host
                                                type:CHPermissionTypePopup];
  //TODO: Create a new permission rather than starting from scratch.
  [self populatePermissionCache];

  [mAddField setStringValue:@""];
  [mAddButton setEnabled:NO];
  [mWhitelistTable reloadData];
}

- (void) editWhitelistSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{
  [mAddField setStringValue:@""];
}

- (IBAction)editFlashblockWhitelist:(id)aSender
{
  [self updateFlashblockWhitelist];

  [NSApp beginSheet:mFlashblockWhitelistPanel
     modalForWindow:[mEditFlashblockWhitelist window]   // any old window accessor
      modalDelegate:self
     didEndSelector:nil
        contextInfo:NULL];

  [mFlashblockWhitelistTable setDeleteAction:@selector(removeFlashblockWhitelistSite:)];
  [mFlashblockWhitelistTable setTarget:self];

  [mAddFlashblockButton setEnabled:NO];
  [mAddFlashblockField setStringValue:@""];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(updateFlashblockWhitelist)
                                               name:kFlashblockWhitelistChangedNotificationName
                                             object:nil];
}

- (IBAction)editFlashblockWhitelistDone:(id)aSender
{
  [mFlashblockWhitelistPanel orderOut:self];
  [NSApp endSheet:mFlashblockWhitelistPanel];

  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:kFlashblockWhitelistChangedNotificationName
                                                object:nil];
  [mFlashblockSites release];
  mFlashblockSites = nil;
}

- (IBAction)removeFlashblockWhitelistSite:(id)aSender
{
  int row = [mFlashblockWhitelistTable selectedRow];
  if (row < 0)
    return;

  [[FlashblockWhitelistManager sharedInstance] removeFlashblockWhitelistSite:[mFlashblockSites objectAtIndex:row]];
}

- (IBAction)addFlashblockWhitelistSite:(id)sender
{
  NSString* site = [mAddFlashblockField stringValue];
  [[FlashblockWhitelistManager sharedInstance] addFlashblockWhitelistSite:site];
  // The user can no longer add the site that's in the text field,
  // so clear the text field and disable the Add button.
  [mAddFlashblockField setStringValue:@""];
  [mAddFlashblockButton setEnabled:NO];
}

-(void) updateFlashblockWhitelist
{
  // Get the updated whitelist array.
  [mFlashblockSites release];
  mFlashblockSites = [[[FlashblockWhitelistManager sharedInstance] whitelistArray] retain];
  [mFlashblockWhitelistTable reloadData];
}

// data source informal protocol (NSTableDataSource)
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
  if (aTableView == mWhitelistTable)
    return [mCachedPermissions count];

  // otherwise aTableView == mFlashblockWhitelistTable
  return [mFlashblockSites count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
  id retVal = nil;
  if (aTableView == mWhitelistTable) {
    CHPermission* permission = [mCachedPermissions objectAtIndex:rowIndex];
    if (aTableColumn == mPolicyColumn)
      retVal = [NSNumber numberWithInt:(([permission policy] == CHPermissionAllow) ? 0 : 1)];
    else // host column
      retVal = [permission host];
  }
  else { // aTableView == mFlashblockWhitelistTable
    retVal = [mFlashblockSites objectAtIndex:rowIndex];
  }

  return retVal;
}

// currently, this only applies to the site allow/deny, since that's the only editable column
-(void) tableView:(NSTableView *)aTableView
   setObjectValue:anObject
   forTableColumn:(NSTableColumn *)aTableColumn
              row:(int)rowIndex
{
  if (aTableColumn == mPolicyColumn) {
    CHPermission* permission = [mCachedPermissions objectAtIndex:rowIndex];
    [permission setPolicy:(([anObject intValue] == 0) ? CHPermissionAllow
                                                      : CHPermissionDeny)];
  }
}

- (void)controlTextDidChange:(NSNotification*)notification
{
  id textField = [notification object];

  if (textField == mAddField)
    [mAddButton setEnabled:[[mAddField stringValue] length] > 0];
  else if (textField == mAddFlashblockField)
    [mAddFlashblockButton setEnabled:[[FlashblockWhitelistManager sharedInstance] canAddToWhitelist:[mAddFlashblockField stringValue]]];
}

//
// tabFocusBehaviorChanged:
//
// Enable and disable tabbing to various elements. We expose only three options,
// but internally it's a bitwise additive pref of text fields, other form
// elements, and links
//
- (IBAction)tabFocusBehaviorChanged:(id)sender
{
  int tabFocusValue = 0;
  switch ([sender indexOfSelectedItem]) {
    case 0:
      tabFocusValue = kTabFocusesTextFields;
      break;
    case 1:
      tabFocusValue = kTabFocusesTextFields | kTabFocusesForms;
      break;
    case 2:
      tabFocusValue = kTabFocusesTextFields | kTabFocusesForms | kTabFocusesLinks;
      break;
  }

  [self setPref:kGeckoPrefTabFocusBehavior toInt:tabFocusValue];
}

//
// popupIndexForCurrentTabFocusPref
//
// Returns the tab focus popup index for the current setting of the tab focus
// pref. Since we may not be able to show the actual pref, we err on the side
// of showing an over-inclusive item.
//
- (int)popupIndexForCurrentTabFocusPref
{
  int tabFocusValue = [self getIntPref:kGeckoPrefTabFocusBehavior withSuccess:NULL];
  if (tabFocusValue & kTabFocusesLinks)
    return 2;
  else if (tabFocusValue & kTabFocusesForms)
    return 1;
  else
    return 0;
}

//
// clickEnableAnnoyanceBlocker:
//
// Enable and disable prefs for allowing webpages to be annoying and move/resize the
// window or tweak the status bar and make it unusable.
//
-(IBAction) clickEnableAnnoyanceBlocker:(id)sender
{
  [sender setAllowsMixedState:NO];
  if ( [sender state] ) 
    [self setAnnoyingWindowPrefsTo:YES];
  else
    [self setAnnoyingWindowPrefsTo:NO];
}

//
// setAnnoyingWindowPrefsTo:
//
// Set all the prefs that allow webpages to muck with the status bar and window position
// (ie, be really annoying) to the given value
//
-(void) setAnnoyingWindowPrefsTo:(BOOL)inValue
{
    [self setPref:kGeckoPrefPreventDOMWindowResize toBoolean:inValue];
    [self setPref:kGeckoPrefPreventDOMStatusChange toBoolean:inValue];
    [self setPref:kGeckoPrefPreventDOMWindowFocus toBoolean:inValue];
}

- (int)annoyingWindowPrefs
{
  BOOL disableStatusChangePref = [self getBooleanPref:kGeckoPrefPreventDOMStatusChange withSuccess:NULL];
  BOOL disableMoveResizePref = [self getBooleanPref:kGeckoPrefPreventDOMWindowResize withSuccess:NULL];
  BOOL disableWindowFlipPref = [self getBooleanPref:kGeckoPrefPreventDOMWindowFocus withSuccess:NULL];

  if(disableStatusChangePref && disableMoveResizePref && disableWindowFlipPref)
    return kAnnoyancePrefAll;
  if(!disableStatusChangePref && !disableMoveResizePref && !disableWindowFlipPref)
    return kAnnoyancePrefNone;

  return kAnnoyancePrefSome;
}

- (int)preventAnimationCheckboxState
{
  NSString* preventAnimation = [self getStringPref:kGeckoPrefImageAnimationBehavior withSuccess:NULL];
  if ([preventAnimation isEqualToString:kImageAnimationOnce])
    return NSOnState;
  else if ([preventAnimation isEqualToString:kImageAnimationLoop])
    return NSOffState;
  else
    return NSMixedState;
}

//
// isFlashblockAllowed
//
// Checks whether Flashblock can be enabled.
// Flashblock is only allowed if javascript and plug-ins enabled.
// NOTE: This code is duplicated in PreferenceManager.mm since the Flashblock checkbox
// settings are done by WebFeatures and stylesheet loading is done by PreferenceManager.
//
-(BOOL) isFlashblockAllowed
{
  BOOL gotPref = NO;
  BOOL jsEnabled = [self getBooleanPref:kGeckoPrefEnableJavascript withSuccess:&gotPref] && gotPref;
  BOOL pluginsEnabled = [self getBooleanPref:kGeckoPrefEnablePlugins withSuccess:&gotPref] || !gotPref;

  return jsEnabled && pluginsEnabled;
}

//
// updateFlashblock
//
// Update the state of the Flashblock checkbox
//
-(void) updateFlashblock
{
  BOOL allowed = [self isFlashblockAllowed];
  [mEnableFlashblock setEnabled:allowed];

  // Flashblock state can only change if it's already enabled, 
  // since changing dependencies won't have any effect on disabled Flashblock.
  if (![self getBooleanPref:kGeckoPrefBlockFlash withSuccess:NULL])
    return;

  // Flashblock preference is enabled.  Checkbox is on if Flashblock is also allowed.
  [mEnableFlashblock setState:(allowed ? NSOnState : NSOffState)];
  [mEditFlashblockWhitelist setEnabled:allowed];
 
  // Always send a notification, dependency verification is done by receiver.
  [[NSNotificationCenter defaultCenter] postNotificationName:kFlashblockChangedNotificationName object:nil];
}

@end
