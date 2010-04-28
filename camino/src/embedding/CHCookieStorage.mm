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
* The Original Code is Camino code.
*
* The Initial Developer of the Original Code is
* Stuart Morgan
* Portions created by the Initial Developer are Copyright (C) 2008
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
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

#import "CHCookieStorage.h"

// For shutdown notification names.
#import "CHBrowserService.h"

#include "nsCOMPtr.h"
#include "nsNetCID.h"
#include "nsString.h"
#include "nsServiceManagerUtils.h"
#include "nsICookie.h"
#include "nsICookie2.h"
#include "nsICookieManager.h"
#include "nsISimpleEnumerator.h"

static NSString* const kCookieNullPlaceholderString = @"<null>";
// The weird string break here is to avoid creating an ANSI C trigraph.
static NSString* const kCookieUnreadablePlaceholderString = @"<???" ">";

// Returns a best-effort conversion of cString into a displayable string.
// Guaranteed never to return nil.
static NSString* BestGuessStringFromCString(const char* cString)
{
  // Cookie values can be arbitrary data, so we can't reliably interpret them as
  // any given encoding. Start with UTF-8, since it's a good default choice and
  // we can usually tell if it's wrong.
  NSString* displayString = [NSString stringWithUTF8String:cString];
  // Next, try ISO-Latin-1, since it's a fairly common web encoding.
  if (!displayString) {
    displayString = [NSString stringWithCString:cString
                                       encoding:NSISOLatin1StringEncoding];
  }
  // If all else fails, use a placeholder.
  if (!displayString)
    displayString = kCookieUnreadablePlaceholderString;

  return displayString;
}

// Xcode 2.x's ld dead-strips this symbol.  Xcode 3.0's ld is fine.
asm(".no_dead_strip .objc_class_name_CHCookieStorage");

@interface NSHTTPCookie (GeckoConstructor)
+ (id)cookieFromGeckoCookie:(nsICookie2*)geckoCookie;
@end

@implementation NSHTTPCookie (GeckoConstructor)

// Creates an autoreleased NSHTTPCookie from the data in the given Gecko cookie.
+ (id)cookieFromGeckoCookie:(nsICookie2*)geckoCookie
{
  NSMutableDictionary* properties = [NSMutableDictionary dictionaryWithCapacity:6];

  id convertedCookie = nil;
  @try {
    nsCAutoString val;
    geckoCookie->GetHost(val);
    [properties setObject:[NSString stringWithUTF8String:val.get()]
                   forKey:NSHTTPCookieDomain];

    geckoCookie->GetPath(val);
    [properties setObject:[NSString stringWithUTF8String:val.get()]
                   forKey:NSHTTPCookiePath];

    geckoCookie->GetName(val);
    // NSHTTPCookie (and RFC 2109 and RFC 2965) requires that cookies have a
    // name. Core doesn't and will happily set nameless cookies. Substitute a
    // placeholder string for the missing name so NSHTTPCookie doesn't choke
    // on it.
    if (val.Length() == 0) {
      [properties setObject:kCookieNullPlaceholderString
                     forKey:NSHTTPCookieName];
    }
    else {
      [properties setObject:BestGuessStringFromCString(val.get())
                     forKey:NSHTTPCookieName];
    }

    geckoCookie->GetValue(val);
    [properties setObject:BestGuessStringFromCString(val.get())
                   forKey:NSHTTPCookieValue];

    PRBool secure = PR_FALSE;
    geckoCookie->GetIsSecure(&secure);
    if (secure)
      [properties setObject:@"TRUE" forKey:NSHTTPCookieSecure];

    PRUint64 expiry = 0;
    geckoCookie->GetExpires(&expiry);
    NSDate *date = [NSDate dateWithTimeIntervalSince1970:(NSTimeInterval)expiry];
    [properties setObject:date forKey:NSHTTPCookieExpires];

    PRBool isSession = PR_FALSE;
    geckoCookie->GetIsSession(&isSession);
    if (isSession)
      [properties setObject:@"TRUE" forKey:NSHTTPCookieDiscard];

    convertedCookie = [self cookieWithProperties:properties];
  }
  @catch (NSException* e) {
    NSLog(@"Failed to convert cookie: %@ %@", [e reason], properties);
  }

  return convertedCookie;
}

@end

static CHCookieStorage* sCookieStorage = nil;

@implementation CHCookieStorage

+ (CHCookieStorage*)cookieStorage
{
  if (!sCookieStorage)
    sCookieStorage = [[self alloc] init];
  return sCookieStorage;
}

- (id)init
{
  if ((self = [super init])) {
    nsCOMPtr<nsICookieManager> cm(do_GetService(NS_COOKIEMANAGER_CONTRACTID));
    mManager = cm.get();
    if (!mManager) {
      [self release];
      return nil;
    }
    NS_ADDREF(mManager);

    // Register for xpcom shutdown so that we can release the manager.
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(xpcomShutdown:)
                                                 name:XPCOMShutDownNotificationName
                                               object:nil];
  }
  return self;
}

- (void)dealloc
{
  sCookieStorage = nil;
  NS_IF_RELEASE(mManager);

  [super dealloc];
}

- (void)xpcomShutdown:(NSNotification*)notification
{
  // This nulls out the pointer
  NS_IF_RELEASE(mManager);
}

- (NSArray*)cookies
{
  if (!mManager)
    return nil;

  nsCOMPtr<nsISimpleEnumerator> cookieEnumerator;
  mManager->GetEnumerator(getter_AddRefs(cookieEnumerator));
  if (!cookieEnumerator)
    return nil;

  NSMutableArray* cookies = [NSMutableArray array];

  PRBool hasMoreElements;
  cookieEnumerator->HasMoreElements(&hasMoreElements);
  while (hasMoreElements) {
    nsCOMPtr<nsICookie2> cookie;
    cookieEnumerator->GetNext(getter_AddRefs(cookie));
    if (cookie) {
      NSHTTPCookie* theCookie = [NSHTTPCookie cookieFromGeckoCookie:cookie.get()];
      // For some reason we don't understand, the above can sometimes fail.
      // XXX if we ever discover a good failure testcase in the future, we should fix
      // cookieFromGeckoCookie: to deal with it.
      if (theCookie)
        [cookies addObject:theCookie];
    }
    cookieEnumerator->HasMoreElements(&hasMoreElements);
  }
  return cookies;
}

- (void)deleteCookie:(NSHTTPCookie*)cookie
{
  if (!mManager)
    return;
  NSString* cookieName = [cookie name];
  // Look for the placeholder string and replace it with an empty string.
  if ([cookieName isEqualToString:kCookieNullPlaceholderString])
    cookieName = @"";

  mManager->Remove(nsDependentCString([[cookie domain] UTF8String]),
                   nsDependentCString([cookieName UTF8String]),
                   nsDependentCString([[cookie path] UTF8String]),
                   PR_FALSE);  // don't block permanently
}

- (void)deleteAllCookies
{
  if (!mManager)
    return;
  mManager->RemoveAll();
}

@end
