/* -*- Mode: C++; tab-width: 3; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Waterson <waterson@netscape.com>
 *   Jan Varga <varga@nixcorp.com>
 *   Nate Nielsen <nielsen@memberwebs.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsTreeUtils_h__
#define nsTreeUtils_h__

#include "nsString.h"
#include "nsISupportsArray.h"
#include "nsIContent.h"

class nsTreeUtils
{
  public:
    /**
     * Parse a whitespace separated list of properties into an array
     * of atoms.
     */
    static nsresult
    TokenizeProperties(const nsAString& aProperties, nsISupportsArray* aPropertiesArray);

    static nsresult
    GetImmediateChild(nsIContent* aContainer, nsIAtom* aTag, nsIContent** aResult);

    static nsresult
    GetDescendantChild(nsIContent* aContainer, nsIAtom* aTag, nsIContent** aResult);

    static nsresult
    UpdateSortIndicators(nsIContent* aColumn, const nsAString& aDirection);

    static nsresult
    GetColumnIndex(nsIContent* aColumn, PRInt32* aResult);
};

#endif // nsTreeUtils_h__
