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

#import "SecurityPane.h"

#import "CHCertificateOverrideManager.h"
#import "ExtendedTableView.h"
#import "GeckoPrefConstants.h"

// prefs for showing security dialogs
#define LEAVE_SITE_PREF      "security.warn_leaving_secure"
#define MIXEDCONTENT_PREF    "security.warn_viewing_mixed"

const unsigned int kSelectAutomaticallyMatrixRowValue = 0;
const unsigned int kAskEveryTimeMatrixRowValue        = 1;

static NSString* const kOverrideHostKey = @"host";
static NSString* const kOverridePortKey = @"port";


@interface OrgMozillaCaminoPreferenceSecurity(Private)
// Loads the list of stored certificate overrides into mOverrides.
- (void)loadOverrides;
@end

@implementation OrgMozillaCaminoPreferenceSecurity

- (void)dealloc
{
  [mOverrides release];

  [super dealloc];
}

- (void)updateButtons
{
  // Set initial value on Security checkboxes
  BOOL leaveEncrypted = [self getBooleanPref:LEAVE_SITE_PREF withSuccess:NULL];
  [mLeaveEncrypted setState:(leaveEncrypted ? NSOnState : NSOffState)];

  BOOL viewMixed = [self getBooleanPref:MIXEDCONTENT_PREF withSuccess:NULL];
  [mViewMixed setState:(viewMixed ? NSOnState : NSOffState)];

  BOOL gotPref;
  NSString* certificateBehavior = [self getStringPref:kGeckoPrefDefaultCertificateBehavior
                                          withSuccess:&gotPref];
  if (gotPref) {
    if ([certificateBehavior isEqual:kPersonalCertificateSelectAutomatically])
      [mCertificateBehavior selectCellAtRow:kSelectAutomaticallyMatrixRowValue column:0];
    else if ([certificateBehavior isEqual:kPersonalCertificateAlwaysAsk])
      [mCertificateBehavior selectCellAtRow:kAskEveryTimeMatrixRowValue column:0];
  }
}

- (void)willSelect
{
  [self updateButtons];
}

- (void)didActivate
{
  [self updateButtons];
}

//
// clickEnableViewMixed:
// clickEnableLeaveEncrypted:
//
// Set prefs for warnings/alerts wrt secure sites.
//

- (IBAction)clickEnableViewMixed:(id)sender
{
  [self setPref:MIXEDCONTENT_PREF toBoolean:[sender state] == NSOnState];
}

- (IBAction)clickEnableLeaveEncrypted:(id)sender
{
  [self setPref:LEAVE_SITE_PREF toBoolean:[sender state] == NSOnState];
}

- (IBAction)clickCertificateSelectionBehavior:(id)sender
{
  unsigned int row = [mCertificateBehavior selectedRow];

  if (row == kSelectAutomaticallyMatrixRowValue)
    [self setPref:kGeckoPrefDefaultCertificateBehavior toString:kPersonalCertificateSelectAutomatically];
  else // row == kAskEveryTimeMatrixRowValue
    [self setPref:kGeckoPrefDefaultCertificateBehavior toString:kPersonalCertificateAlwaysAsk];
}

- (IBAction)showCertificates:(id)sender
{
  // We'll just fire off a notification and let the application show the UI.
  NSDictionary* userInfoDict = [NSDictionary dictionaryWithObject:[mLeaveEncrypted window]  // any view's window
                                                           forKey:@"parent_window"];
  [[NSNotificationCenter defaultCenter] postNotificationName:@"ShowCertificatesNotification"
                                                      object:nil
                                                    userInfo:userInfoDict];
}

#pragma mark -
#pragma mark Certificate Overrides Sheet

- (IBAction)editOverrides:(id)aSender
{
  [self loadOverrides];

  [mOverridesTable setDeleteAction:@selector(removeOverrides:)];
  [mOverridesTable setTarget:self];

  // bring up sheet
  [NSApp beginSheet:mOverridePanel
     modalForWindow:[aSender window]
      modalDelegate:self
     didEndSelector:NULL
        contextInfo:NULL];
}

- (IBAction)editOverridesDone:(id)aSender
{
  [mOverridePanel orderOut:self];
  [NSApp endSheet:mOverridePanel];

  [mOverrides release];
  mOverrides = nil;
}

- (void)loadOverrides
{
  CHCertificateOverrideManager* overrideManager =
    [CHCertificateOverrideManager certificateOverrideManager];
  NSArray* hostPortOverrides = [overrideManager overrideHosts];

  NSMutableArray* overrides =
    [NSMutableArray arrayWithCapacity:[hostPortOverrides count]];
  NSEnumerator* hostPortEnumerator = [hostPortOverrides objectEnumerator];
  NSString* hostPort;
  while ((hostPort = [hostPortEnumerator nextObject])) {
    // Entries should be "host:port"; split them apart and sanity check.
    NSArray* components = [hostPort componentsSeparatedByString:@":"];
    if ([components count] != 2)
      continue;
    [overrides addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                           [components objectAtIndex:0], kOverrideHostKey,
                           [components objectAtIndex:1], kOverridePortKey,
                           nil]];
  }

  NSSortDescriptor* initialSort =
    [[[NSSortDescriptor alloc] initWithKey:@"host"
                                 ascending:YES] autorelease];
  [self willChangeValueForKey:@"mOverrides"];
  [mOverrides autorelease];
  mOverrides = [overrides retain];
  [mOverrides sortUsingDescriptors:[NSArray arrayWithObject:initialSort]];
  [self didChangeValueForKey:@"mOverrides"];
}

- (IBAction)removeOverrides:(id)aSender
{
  CHCertificateOverrideManager* overrideManager =
    [CHCertificateOverrideManager certificateOverrideManager];

  // Walk the selected rows, removing overrides.
  NSArray* selectedOverrides = [mOverridesController selectedObjects];
  NSEnumerator* overrideEnumerator = [selectedOverrides objectEnumerator];
  NSDictionary* override;
  while ((override = [overrideEnumerator nextObject])) {
    [overrideManager removeOverrideForHost:[override objectForKey:kOverrideHostKey]
                                      port:[[override objectForKey:kOverridePortKey] intValue]];
  }
  [mOverridesController removeObjects:selectedOverrides];
}

- (IBAction)removeAllOverrides:(id)aSender
{
  NSAlert* removeAllAlert = [[[NSAlert alloc] init] autorelease];
  [removeAllAlert setMessageText:[self localizedStringForKey:@"RemoveAllSecurityExceptionsWarningTitle"]];
  [removeAllAlert setInformativeText:[self localizedStringForKey:@"RemoveAllSecurityExceptionsWarning"]];
  [removeAllAlert addButtonWithTitle:[self localizedStringForKey:@"RemoveAllExceptionsButtonText"]];
  NSButton* dontRemoveButton = [removeAllAlert addButtonWithTitle:[self localizedStringForKey:@"DontRemoveButtonText"]];
  [dontRemoveButton setKeyEquivalent:@"\e"]; // escape
  
  [removeAllAlert setAlertStyle:NSCriticalAlertStyle];
  
  if ([removeAllAlert runModal] == NSAlertFirstButtonReturn) {
    NSRange fullRange = NSMakeRange(0, [mOverrides count]);
    NSIndexSet* allIndexes = [NSIndexSet indexSetWithIndexesInRange:fullRange];
    [mOverridesController setSelectionIndexes:allIndexes];
    [self removeOverrides:aSender];
  }
}

@end
