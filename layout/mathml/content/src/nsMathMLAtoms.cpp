/*
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
 * The Original Code is Mozilla MathML Project.
 * 
 * The Initial Developer of the Original Code is The University Of 
 * Queensland.  Portions created by The University Of Queensland are
 * Copyright (C) 1999 The University Of Queensland.  All Rights Reserved.
 * 
 * Contributor(s): 
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 */

#include "nsMathMLAtoms.h"
#include "nsAtomListUtils.h"

// define storage for all atoms
#define MATHML_ATOM(_name, _value) nsIAtom* nsMathMLAtoms::_name;
#include "nsMathMLAtomList.h"
#undef MATHML_ATOM


static nsrefcnt gRefCnt = 0;

static const nsAtomListInfo MathMLAtoms_info[] = {
#define MATHML_ATOM(name_, value_) { &nsMathMLAtoms::name_, value_ },
#include "nsMathMLAtomList.h"
#undef MATHML_ATOM
};

void nsMathMLAtoms::AddRefAtoms() {
  if (gRefCnt == 0) {
    nsAtomListUtils::AddRefAtoms(MathMLAtoms_info,
                                 MOZ_ARRAY_LENGTH(MathMLAtoms_info));
  }
  ++gRefCnt;
}

void nsMathMLAtoms::ReleaseAtoms() {
  NS_PRECONDITION(gRefCnt != 0, "bad release of MathML atoms");
  if (--gRefCnt == 0) {
    nsAtomListUtils::ReleaseAtoms(MathMLAtoms_info,
                                  MOZ_ARRAY_LENGTH(MathMLAtoms_info));
  }
}
