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
 * The Original Code is atom lists for CSS pseudos.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   L. David Baron <dbaron@dbaron.org>
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
#include "nsCSSAnonBoxes.h"
#include "nsAtomListUtils.h"

// define storage for all atoms
#define CSS_ANON_BOX(_name, _value) \
  nsICSSAnonBoxPseudo* nsCSSAnonBoxes::_name;
#include "nsCSSAnonBoxList.h"
#undef CSS_ANON_BOX

static nsrefcnt gRefCnt;

static const nsAtomListInfo CSSAnonBoxes_info[] = {
#define CSS_ANON_BOX(name_, value_) \
    { (nsIAtom**)&nsCSSAnonBoxes::name_, value_ },
#include "nsCSSAnonBoxList.h"
#undef CSS_ANON_BOX
};

void nsCSSAnonBoxes::AddRefAtoms()
{
  if (0 == gRefCnt++) {
    nsAtomListUtils::AddRefAtoms(CSSAnonBoxes_info,
                                 MOZ_ARRAY_LENGTH(CSSAnonBoxes_info));
  }
}

void nsCSSAnonBoxes::ReleaseAtoms()
{
  NS_PRECONDITION(gRefCnt != 0, "bad release atoms");
  if (--gRefCnt == 0) {
    nsAtomListUtils::ReleaseAtoms(CSSAnonBoxes_info,
                                  MOZ_ARRAY_LENGTH(CSSAnonBoxes_info));
  }
}

PRBool nsCSSAnonBoxes::IsAnonBox(nsIAtom *aAtom)
{
  return nsAtomListUtils::IsMember(aAtom, CSSAnonBoxes_info,
                                   MOZ_ARRAY_LENGTH(CSSAnonBoxes_info));
}

