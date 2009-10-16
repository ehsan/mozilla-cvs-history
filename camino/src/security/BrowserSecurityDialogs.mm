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
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Simon Fraser <smfr@smfr.org>
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

#import "NSImage+Utils.h"
#import "NSString+Utils.h"
#import "NSString+Gecko.h"

#import "CHCertificateOverrideManager.h"

#import "nsCOMPtr.h"
#import "nsString.h"

#import "nsIX509Cert.h"
#import "nsIRecentBadCertsService.h"
#import "nsISSLStatus.h"
#import "nsServiceManagerUtils.h"

#import "AutoSizingTextField.h"
#import "CertificateItem.h"
#import "CertificateView.h"
#import "ViewCertificateDialogController.h"
#import "BrowserSecurityDialogs.h"


static const int kDefaultHTTPSPort = 443;

#pragma mark BrowserSecurityUIProvider
#pragma mark -

@implementation BrowserSecurityUIProvider

static BrowserSecurityUIProvider* gBrowserSecurityUIProvider;

+ (BrowserSecurityUIProvider*)sharedBrowserSecurityUIProvider
{
  if (!gBrowserSecurityUIProvider)
    gBrowserSecurityUIProvider = [[BrowserSecurityUIProvider alloc] init];

  return gBrowserSecurityUIProvider;
}

+ (NSImage*)lockBadgedApplicationIcon
{
  return [[NSImage imageNamed:@"NSApplicationIcon"] imageByApplyingBadge:[NSImage imageNamed:@"lock_badge"]
                                                               withAlpha:1.0f
                                                                   scale:0.6f];
}

- (id)init
{
  if ((self = [super init]))
  {
  }
  return self;
}

- (void)dealloc
{
  if (gBrowserSecurityUIProvider == self)
    gBrowserSecurityUIProvider = nil;
  [super dealloc];
}

- (DownloadCACertDialogController*)downloadCACertDialogController
{
  DownloadCACertDialogController* dialogController = [[DownloadCACertDialogController alloc] initWithWindowNibName:@"DownloadCACertDialog"];
  return [dialogController autorelease];
}

- (InvalidCertOverrideDialogController*)invalidCertOverrideDialogController
{
  InvalidCertOverrideDialogController* dialogController = [[InvalidCertOverrideDialogController alloc] initWithWindowNibName:@"InvalidCertOverrideDialog"];
  return [dialogController autorelease];
}

- (CreatePasswordDialogController*)createPasswordDialogController
{
  CreatePasswordDialogController* dialogController = [[CreatePasswordDialogController alloc] initWithWindowNibName:@"CreatePasswordDialog"];
  return [dialogController autorelease];
}

- (GenKeyPairDialogController*)genKeyPairDialogController
{
  GenKeyPairDialogController* dialogController = [[GenKeyPairDialogController alloc] initWithWindowNibName:@"GenerateKeyPairDialog"];
  return [dialogController autorelease];
}

- (ChooseCertDialogController*)chooseCertDialogController
{
  ChooseCertDialogController* dialogController = [[ChooseCertDialogController alloc] initWithWindowNibName:@"ChooseCertificateDialog"];
  return [dialogController autorelease];
}

@end

#pragma mark -
#pragma mark CertificateDialogController
#pragma mark -

@implementation CertificateDialogController

- (void)dealloc
{
  [super dealloc];
}

- (void)windowDidLoad
{
  [mIcon setImage:[BrowserSecurityUIProvider lockBadgedApplicationIcon]];
}

- (void)setCertificateItem:(CertificateItem*)inCert
{
  [self window];  // ensure window loaded

  [mCertificateView setCertificateItem:inCert];
  [mCertificateView setDelegate:self];
}

// CertificateViewDelegate method
- (void)certificateView:(CertificateView*)certView showIssuerCertificate:(CertificateItem*)issuerCert
{
  // if we are modal, then this must also be modal
  [ViewCertificateDialogController runModalCertificateWindowWithCertificateItem:issuerCert
                                                       certTypeForTrustSettings:nsIX509Cert::CA_CERT];
}

@end

#pragma mark -
#pragma mark DownloadCACertDialogController
#pragma mark -

@interface DownloadCACertDialogController(Private)

- (void)setupWindow;

@end

#pragma mark -

@implementation DownloadCACertDialogController

- (void)dealloc
{
  [super dealloc];
}

- (IBAction)onOK:(id)sender
{
  [NSApp stopModalWithCode:NSAlertDefaultReturn];
  [[self window] orderOut:nil];
}

- (IBAction)onCancel:(id)sender
{
  [NSApp stopModalWithCode:NSAlertAlternateReturn];
  [[self window] orderOut:nil];
}

- (void)setCertificateItem:(CertificateItem*)inCert
{
  [self window];    // ensure window loaded

  NSString* messageFormatString = NSLocalizedStringFromTable(@"ConfirmCAMessageFormat", @"CertificateDialogs", @"");
  [mMessageField setStringValue:[NSString stringWithFormat:messageFormatString, [inCert displayName]]];

  [mCertificateView setShowTrust:YES];
  [mCertificateView setDetailsInitiallyExpanded:NO];
  [mCertificateView setTrustInitiallyExpanded:YES];
  [mCertificateView setCertTypeForTrustSettings:nsIX509Cert::CA_CERT];

  [super setCertificateItem:inCert];
}

- (unsigned int)trustMaskSetting
{
  return [mCertificateView trustMaskSetting];
}

@end


#pragma mark -
#pragma mark InvalidCertOverrideDialogController
#pragma mark -

static int kInvalidCertAddOverride = 1;
static int kInvalidCertCancelOverride = 0;

@implementation InvalidCertOverrideDialogController

- (void)dealloc {
  [mSourceHost release];

  [super dealloc];
}

- (IBAction)onTrust:(id)sender
{
  [NSApp endSheet:[self window] returnCode:kInvalidCertAddOverride];
}

- (IBAction)onCancel:(id)sender
{
  [NSApp endSheet:[self window] returnCode:kInvalidCertCancelOverride];
}

// Loads the recent bad cert for the current host+post into the certificate
// view, and stores its failure flags in mCertFailureFlags.
// Also return the cert for convenience.
//
// mSourceHost and mSourcePort must be set before calling this method. 
- (CertificateItem*)loadRecentBadCert {
  nsCOMPtr<nsIRecentBadCertsService> badCertService = do_GetService(NS_RECENTBADCERTS_CONTRACTID);
  if (badCertService) {
    nsAutoString hostAndPort;
    [[NSString stringWithFormat:@"%@:%d", mSourceHost, mSourcePort] assignTo_nsAString:hostAndPort];
    nsCOMPtr<nsISSLStatus> certStatus;
    badCertService->GetRecentBadCert(hostAndPort, getter_AddRefs(certStatus));
    if (certStatus) {
      mCertFailureFlags = 0;
      PRBool isDomainMismatch, isInvalidTime, isUntrusted;
      certStatus->GetIsDomainMismatch(&isDomainMismatch);
      certStatus->GetIsNotValidAtThisTime(&isInvalidTime);
      certStatus->GetIsUntrusted(&isUntrusted);
      if (isUntrusted)
        mCertFailureFlags |= CHCertificateOverrideFlagUntrusted;
      if (isDomainMismatch)
        mCertFailureFlags |= CHCertificateOverrideFlagDomainMismatch;
      if (isInvalidTime)
        mCertFailureFlags |= CHCertificateOverrideFlagInvalidTime;

      nsCOMPtr<nsIX509Cert> cert;
      certStatus->GetServerCert(getter_AddRefs(cert));
      if (cert) {
        CertificateItem* certItem = [CertificateItem certificateItemWithCert:cert];
        [certItem setDomainIsMismatched:isDomainMismatch];
        // Partially work around bug 453075, so the certificate failure message
        // matches what we are telling the user in the dialog text.
        if (isUntrusted)
          [certItem setFallbackProblemMessageKey:@"InvalidStateCertNotTrusted"];
        else if (isInvalidTime)
          [certItem setFallbackProblemMessageKey:@"InvalidStateExpired"];
        [self setCertificateItem:certItem];
        return certItem;
      }
    }
  }
  return nil;
}

- (void)showWithSourceURL:(NSURL*)url parentWindow:(NSWindow*)parent delegate:(id)delegate
{
  [mCertificateView setDetailsInitiallyExpanded:NO];
  [mCertificateView setTrustInitiallyExpanded:YES];

  mDelegate = delegate;

  mSourceHost = [[url host] retain];
  mSourcePort = [[url port] intValue];
  if (mSourcePort == 0)
    mSourcePort = kDefaultHTTPSPort;

  CertificateItem* certItem = [self loadRecentBadCert];

  // If we didn't find the cert, we are probably in the unlikely case where a
  // user has opened so many different sites with cert problems before trying to
  // add an exception for the first that they've blown out the cache. Rather
  // than adding special recovery for a rare case, just tell them to try again.
  if (!certItem) {
    NSString* titleString = NSLocalizedStringFromTable(@"CertOverrideInfoMissingTitle", @"CertificateDialogs", nil);
    NSString* messageFormat = NSLocalizedStringFromTable(@"CertOverrideInfoMissingMessageFormat", @"CertificateDialogs", nil);
    NSAlert* alert = [[[NSAlert alloc] init] autorelease];
    [alert setAlertStyle:NSInformationalAlertStyle];
    [alert setMessageText:titleString];
    [alert setInformativeText:[NSString stringWithFormat:messageFormat, mSourceHost]];
    [alert beginSheetModalForWindow:parent
                      modalDelegate:self
                     didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
                        contextInfo:nil];
    return;
  }

  NSString* titleFormat = NSLocalizedStringFromTable(@"InvalidCertTitleFormat", @"CertificateDialogs", nil);
  [mTitleField setStringValue:[NSString stringWithFormat:titleFormat, mSourceHost]];

  // Rather than overwhelm the user with information, just pick the most
  // important problem to tell them about.
  NSString* problemDescription = nil;
  if (mCertFailureFlags & CHCertificateOverrideFlagUntrusted) {
    NSString* messageFormat = NSLocalizedStringFromTable(@"InvalidCertMessageFormat", @"CertificateDialogs", nil);
    problemDescription = [NSString stringWithFormat:messageFormat, mSourceHost];
  }
  else if (mCertFailureFlags & CHCertificateOverrideFlagDomainMismatch) {
    NSString* messageFormat = NSLocalizedStringFromTable(@"MismatchedCertMessageFormat", @"CertificateDialogs", nil);
    problemDescription = [NSString stringWithFormat:messageFormat, [certItem commonName], mSourceHost];
  }
  else if (mCertFailureFlags & CHCertificateOverrideFlagInvalidTime) {
    NSString* messageFormat = NSLocalizedStringFromTable(@"ExpiredCertMessageFormat", @"CertificateDialogs", nil);
    problemDescription = [NSString stringWithFormat:messageFormat, mSourceHost];
  } else {
    NSString* messageFormat = NSLocalizedStringFromTable(@"InvalidCertMessageFormat", @"CertificateDialogs", nil);
    problemDescription = [NSString stringWithFormat:messageFormat, mSourceHost];
  }
  float nibMessageHeight = NSHeight([mMessageField frame]);
  [mMessageField setStringValue:problemDescription];

  // Adjust the the sheet and other controls to account for the message change.
  float heightChange = NSHeight([mMessageField frame]) - nibMessageHeight;
  NSView* scrollView = mCertificateView;
  do {
    scrollView = [scrollView superview];
  } while (![scrollView isKindOfClass:[NSScrollView class]]);
  NSRect certScrollViewFrame = [scrollView frame];
  certScrollViewFrame.size.height -= heightChange;
  [scrollView setFrame:certScrollViewFrame];
  NSRect sheetFrame = [[self window] frame];
  sheetFrame.size.height += heightChange;
  [[self window] setFrame:sheetFrame display:NO];

  [NSApp beginSheet:[self window]
     modalForWindow:parent
      modalDelegate:self
     didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:)
        contextInfo:nil];
}

- (void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
  if ([mDelegate respondsToSelector:@selector(certOverride:finishedWithResult:)])
    [mDelegate certOverride:self finishedWithResult:NO];
}

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
  [[self window] orderOut:self];
  BOOL addedOverride = NO;
  if (returnCode == kInvalidCertAddOverride) {
    nsCOMPtr<nsIX509Cert> cert = [[mCertificateView certificateItem] cert];
    if (cert) {
      CHCertificateOverrideManager* overrideManager =
        [CHCertificateOverrideManager certificateOverrideManager];
      addedOverride = [overrideManager addOverrideForHost:mSourceHost
                                                     port:mSourcePort
                                                 withCert:cert
                                          validationFlags:mCertFailureFlags];
    }
  }

  if ([mDelegate respondsToSelector:@selector(certOverride:finishedWithResult:)])
    [mDelegate certOverride:self finishedWithResult:addedOverride];
}

@end

#pragma mark -
#pragma mark CreatePasswordDialogController
#pragma mark -

@interface CreatePasswordDialogController(Private)

+ (float)strengthOfPassword:(NSString*)inPassword;
+ (BOOL)isDictionaryWord:(NSString*)inPassword;

- (void)onPasswordFieldChanged:(id)sender;
- (BOOL)checkOldPassword;

@end

#pragma mark -

@implementation CreatePasswordDialogController

+ (float)strengthOfPassword:(NSString*)inPassword
{
  // length
  int numChars = [inPassword length];
  if (numChars > 5)
    numChars = 5;

  // number of numbers
  NSString* strippedString = [inPassword stringByRemovingCharactersInSet:[NSCharacterSet decimalDigitCharacterSet]];
  int numNumeric = [inPassword length] - [strippedString length];
  if (numNumeric > 3)
    numNumeric = 3;

  // number of symbols
  strippedString = [inPassword stringByRemovingCharactersInSet:[NSCharacterSet punctuationCharacterSet]];
  int numSymbols = [inPassword length] - [strippedString length];
  if (numSymbols > 3)
    numSymbols = 3;

  strippedString = [inPassword stringByRemovingCharactersInSet:[NSCharacterSet uppercaseLetterCharacterSet]];
  int numUppercase = [inPassword length] - [strippedString length];
  if (numUppercase > 3)
    numUppercase = 3;

  float passwordStrength = ((numChars * 10) - 20) + (numNumeric * 10) + (numSymbols * 15) + (numUppercase * 10);
  if (passwordStrength < 0.0f)
    passwordStrength = 0.0f;

  if (passwordStrength > 100.0f)
    passwordStrength = 100.0f;
  
  return passwordStrength;
}

+ (BOOL)isDictionaryWord:(NSString*)inPassword
{
  NSSpellChecker* spellChecker = [NSSpellChecker sharedSpellChecker];
  if (!spellChecker)
    return NO;

  // finds misspelled words
  NSRange foundRange = [spellChecker checkSpellingOfString:[inPassword lowercaseString] startingAt:0];
//  if (foundRange.location == NSNotFound)    // no mispellings
//    return YES;

  // if it's a substring, it's OK  
  return !NSEqualRanges(foundRange, NSMakeRange(0, [inPassword length]));
}

- (void)dealloc
{
  [mCurPasswordContainer release];
  [super dealloc];
}

- (void)windowDidLoad
{
  [mCurPasswordContainer retain];   // so we can remove without it going away
  [mIcon setImage:[BrowserSecurityUIProvider lockBadgedApplicationIcon]];
  
  [mNewPasswordNotesField setStringValue:@""];
  [mVerifyPasswordNotesField setStringValue:@""];
  
  mShowingOldPassword = YES;
}

- (void)setDelegate:(id)inDelegate
{
  mDelegate = inDelegate;
}

- (id)delegate
{
  return mDelegate;
}

- (IBAction)onOK:(id)sender
{
  if (![self checkOldPassword])
    return;

  [NSApp stopModalWithCode:NSAlertDefaultReturn];
  [[self window] orderOut:nil];
}

- (IBAction)onCancel:(id)sender
{
  [NSApp stopModalWithCode:NSAlertAlternateReturn];
  [[self window] orderOut:nil];
}

- (void)hideChangePasswordField
{
  [self window];

  NSRect curPasswordFrame = [mCurPasswordContainer frame];
  [mCurPasswordContainer removeFromSuperview];
  
  // move the rest up
  NSPoint containerOrigin = curPasswordFrame.origin;
  containerOrigin.y -= NSHeight([mNewPasswordContainer frame]) - NSHeight(curPasswordFrame);
  [mNewPasswordContainer setFrameOrigin:containerOrigin];

  // and shrink the window
  float heightDelta = NSMinY([mNewPasswordContainer frame]);     // we're flipped
  
  NSRect windowBounds = [[self window] frame];
  windowBounds.origin.x -= heightDelta;
  windowBounds.size.height -= heightDelta;
  [[self window] setFrame:windowBounds display:YES];
  
  [[self window] makeFirstResponder:mNewPasswordField];
  mShowingOldPassword = NO;
}

- (void)setTitle:(NSString*)inTitle message:(NSString*)inMessage
{
  [self window];

  [mTitleField setStringValue:inTitle];
  [mMessageField setStringValue:inMessage];
}

- (NSString*)currentPassword
{
  return [mCurPasswordField stringValue];
}

- (NSString*)newPassword
{
  return [mNewPasswordField stringValue];
}

// called when either of the "new" password fields changes
- (void)onPasswordFieldChanged:(id)sender
{
  // update password quality
  float pwQuality = [CreatePasswordDialogController strengthOfPassword:[mNewPasswordField stringValue]];
  BOOL isDictionaryWord = [CreatePasswordDialogController isDictionaryWord:[mNewPasswordField stringValue]];
  if (isDictionaryWord)
    pwQuality = 1.0f;

  [mQualityIndicator setDoubleValue:pwQuality];
  
  // check for dictionary words
  if (isDictionaryWord)
  {
    NSString* warningString = NSLocalizedStringFromTable(@"IsDictionaryWord", @"CertificateDialogs", @"");

    NSDictionary* attribs = [NSDictionary dictionaryWithObject:[NSColor orangeColor] forKey:NSForegroundColorAttributeName];
    NSAttributedString* attribString = [[[NSAttributedString alloc] initWithString:warningString attributes:attribs] autorelease];
    [mNewPasswordNotesField setAttributedStringValue:attribString];
  }
  else
    [mNewPasswordNotesField setStringValue:@""];
  
  BOOL passwordMatch = [[mNewPasswordField stringValue] isEqualToString:[mVerifyPasswordField stringValue]];
  if (([[mVerifyPasswordField stringValue] length] > 0) && !passwordMatch)
  {
    NSString* warningString = NSLocalizedStringFromTable(@"UnmatchedPasswords", @"CertificateDialogs", @"");
    [mVerifyPasswordNotesField setStringValue:warningString];
  }
  else
  {
    [mVerifyPasswordNotesField setStringValue:@""];
  }

  [mOKButton setEnabled:passwordMatch];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
  if ([aNotification object] == mNewPasswordField ||
      [aNotification object] == mVerifyPasswordField)
    [self onPasswordFieldChanged:[aNotification object]];
}

- (BOOL)checkOldPassword
{
  if (!mShowingOldPassword)
    return YES;

  BOOL canContinue = YES;
  if (mDelegate && [mDelegate respondsToSelector:@selector(changePasswordDialogController:oldPasswordValid:)])
    canContinue = [mDelegate changePasswordDialogController:self oldPasswordValid:[mCurPasswordField stringValue]];

  if (!canContinue)
  {
    [mCurPasswordField selectText:nil];
  }
  
  return canContinue;
}

@end

#pragma mark -
#pragma mark GenKeyPairDialogController
#pragma mark -

@implementation GenKeyPairDialogController

- (void)windowDidLoad
{
  [mIcon setImage:[BrowserSecurityUIProvider lockBadgedApplicationIcon]];
  [mProgressIndicator startAnimation:nil];    // it just animates the whole time
}

- (IBAction)onCancel:(id)sender
{
  [NSApp stopModalWithCode:NSAlertAlternateReturn];
  [[self window] orderOut:nil];
}

- (void)keyPairGenerationComplete
{
  [NSApp stopModalWithCode:NSAlertDefaultReturn];
  [[self window] orderOut:nil];
}

@end

#pragma mark -
#pragma mark GenKeyPairDialogController
#pragma mark -

@implementation ChooseCertDialogController

- (void)windowDidLoad
{
  [mIcon setImage:[BrowserSecurityUIProvider lockBadgedApplicationIcon]];
}

- (IBAction)onOK:(id)sender
{
  [NSApp stopModalWithCode:NSAlertDefaultReturn];
  [[self window] orderOut:nil];
}

- (IBAction)onCancel:(id)sender
{
  [NSApp stopModalWithCode:NSAlertAlternateReturn];
  [[self window] orderOut:nil];
}

- (IBAction)onCertPopupChanged:(id)sender
{
  [mCertificateView setCertificateItem:[self selectedCert]];
}

- (void)setCommonName:(NSString*)inCommonName organization:(NSString*)inOrg issuer:(NSString*)inIssuer
{
  [self window];  // force window loading

  NSString* messageFormat = NSLocalizedStringFromTable(@"ChooseCertMessageFormat", @"CertificateDialogs", @"");
  NSString* messageString = [NSString stringWithFormat:messageFormat, inCommonName, inOrg];
  [mMessageText setStringValue:messageString];
}

- (void)setCertificates:(NSArray*)inCertificates
{
  [self window];  // force window loading
  
  [mCertificateView setDelegate:self];

  [mCertPopup removeAllItems];
  NSEnumerator* certsEnum = [inCertificates objectEnumerator];
  CertificateItem* thisCert;

  // could improve this format to show the email address
  NSString* popupItemFormat = NSLocalizedStringFromTable(@"ChooserCertPopupFormat", @"CertificateDialogs", @"");
  
  while ((thisCert = [certsEnum nextObject]))
  {
    NSString* itemTitle = [NSString stringWithFormat:popupItemFormat, [thisCert nickname], [thisCert serialNumber]];
    NSMenuItem* certMenuItem = [[[NSMenuItem alloc] initWithTitle:itemTitle
                                                           action:NULL
                                                    keyEquivalent:@""] autorelease];
    [certMenuItem setRepresentedObject:thisCert];
    [[mCertPopup menu] addItem:certMenuItem];
  }
  
  [mCertPopup synchronizeTitleAndSelectedItem];
  [self onCertPopupChanged:nil];
}

- (CertificateItem*)selectedCert
{
  return [[mCertPopup selectedItem] representedObject];
}

// CertificateViewDelegate method
- (void)certificateView:(CertificateView*)certView showIssuerCertificate:(CertificateItem*)issuerCert
{
  [ViewCertificateDialogController runModalCertificateWindowWithCertificateItem:issuerCert
                                                       certTypeForTrustSettings:nsIX509Cert::CA_CERT];
}

@end
