/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Peter Annema <disttsc@bart.nl>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

/*

  Private interface to XUL content.

*/

#ifndef nsIXULContent_h__
#define nsIXULContent_h__

#include "nsIXMLContent.h"
class nsIAtom;
class nsIRDFCompositeDataSource;
class nsIXULTemplateBuilder;
class nsString;

// {da87924c-2638-4249-8297-4cf814d52d47}
#define NS_IXULCONTENT_IID \
{ 0xda87924c, 0x2638, 0x4249, { 0x82, 0x97, 0x4c, 0xf8, 0x14, 0xd5, 0x2d, 0x47 } }


class nsIXULContent : public nsISupports
{
public:
    NS_DEFINE_STATIC_IID_ACCESSOR(NS_IXULCONTENT_IID)

    /**
     * Peek at a XUL element's child count without forcing children to be
     * instantiated.
     */
    NS_IMETHOD_(PRUint32) PeekChildCount() const = 0;

    /**
     * These flags are used to maintain bookkeeping information for partially-
     * constructed content.
     *
     *   eChildrenMustBeRebuilt
     *     The element's children are invalid or unconstructed, and should
     *     be reconstructed.
     *
     *   eTemplateContentsBuilt
     *     Child content that is built from a XUL template has been
     *     constructed. 
     *
     *   eContainerContentsBuilt
     *     Child content that is built by following the ``containment''
     *     property in a XUL template has been built.
     */
    enum LazyState {
        eChildrenMustBeRebuilt  = 0x1,
        eTemplateContentsBuilt  = 0x2,
        eContainerContentsBuilt = 0x4
    };

    /**
     * Set one or more ``lazy state'' flags.
     * @aFlags a mask of flags to set
     */
    NS_IMETHOD SetLazyState(LazyState aFlags) = 0;

    /**
     * Clear one or more ``lazy state'' flags.
     * @aFlags a mask of flags to clear
     */
    NS_IMETHOD ClearLazyState(LazyState aFlags) = 0;

    /**
     * Get the value of a single ``lazy state'' flag.
     * @aFlag a flag to test
     * @aResult the result
     */
    NS_IMETHOD GetLazyState(LazyState aFlag, PRBool& aResult) = 0;

    /**
     * Add a script event listener to the element.
     */
    NS_IMETHOD AddScriptEventListener(nsIAtom* aName, const nsAString& aValue) = 0;
};

#endif // nsIXULContent_h__
