/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Mozilla Corporation code.
 *
 * The Initial Developer of the Original Code is Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Vladimir Vukicevic <vladimir@pobox.com>
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

#ifndef GFXQUARTZFONTCACHE_H_
#define GFXQUARTZFONTCACHE_H_

#include "nsDataHashtable.h"

#include "gfxAtsuiFonts.h"

#include "nsUnicharUtils.h"

class gfxQuartzFontCache {
public:
    static gfxQuartzFontCache* SharedFontCache() {
        if (!sSharedFontCache)
            sSharedFontCache = new gfxQuartzFontCache();
        return sSharedFontCache;
    }

    ATSUFontID FindATSUFontIDForFamilyAndStyle (const nsAString& aFamily,
                                                const gfxFontStyle* aStyle);

    ATSUFontID GetDefaultATSUFontID (const gfxFontStyle* aStyle);

private:
    static gfxQuartzFontCache *sSharedFontCache;

    gfxQuartzFontCache();

    ATSUFontID FindFromSystem (const nsAString& aFamily,
                               const gfxFontStyle* aStyle);

    struct FontAndFamilyContainer {
        FontAndFamilyContainer (const nsAString& family, const gfxFontStyle& style)
            : mFamily(family), mStyle(style)
        {
            ToLowerCase(mFamily);
        }

        FontAndFamilyContainer (const FontAndFamilyContainer& other)
            : mFamily(other.mFamily), mStyle(other.mStyle)
        { }

        nsString mFamily;
        gfxFontStyle mStyle;
    };

    struct FontAndFamilyKey : public PLDHashEntryHdr {
        typedef const FontAndFamilyContainer& KeyType;
        typedef const FontAndFamilyContainer* KeyTypePointer;

        FontAndFamilyKey(KeyTypePointer aObj) : mObj(*aObj) { }
        FontAndFamilyKey(const FontAndFamilyKey& other) : mObj(other.mObj) { }
        ~FontAndFamilyKey() { }

        KeyType GetKey() const { return mObj; }
        KeyTypePointer GetKeyPointer() const { return &mObj; }

        PRBool KeyEquals(KeyTypePointer aKey) const {
            return
                aKey->mFamily.Equals(mObj.mFamily) &&
                aKey->mStyle.Equals(mObj.mStyle);
        }

        static KeyTypePointer KeyToPointer(KeyType aKey) { return &aKey; }
        static PLDHashNumber HashKey(KeyTypePointer aKey) {
            return HashString(aKey->mFamily);
        }
        enum { ALLOW_MEMMOVE = PR_FALSE };
    private:
        const FontAndFamilyContainer mObj;
    };

    nsDataHashtable<FontAndFamilyKey, ATSUFontID> mCache;
};

#endif /* GFXQUARTZFONTCACHE_H_ */
