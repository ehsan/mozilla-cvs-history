/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
*
* The contents of this file are subject to the Mozilla Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS
* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
* implied. See the License for the specific language governing
* rights and limitations under the License.
*
* The Original Code is the Mozilla browser.
*
* The Initial Developer of the Original Code is Netscape
* Communications Corporation. Portions created by Netscape are
* Copyright (C) 2002 Netscape Communications Corporation. All
* Rights Reserved.
*
* Contributor(s):
*   Mike Pinkerton <pinkerton@netscape.com> (Original Author)
*/

#import <Cocoa/Cocoa.h>

//
// protocol Find
//
// Any window who wants to be able to work with the Find dialog should implement
// this protocol.
//

@protocol Find

// Start a find at the current caret position
- (BOOL)findInPageWithPattern:(NSString*)text caseSensitive:(BOOL)inCaseSensitive
        wrap:(BOOL)inWrap backwards:(BOOL)inBackwards;


// Same as above, but use most recent values for search string,
// case sensitivity, wrap-around, and backwards search.
- (BOOL)findInPage;

// Get the most recent search string.
- (NSString*)lastFindText;

@end
