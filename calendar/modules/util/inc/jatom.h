/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/*
 * jatom.h
 * John Sun
 * 2/27/98 4:35:03 PM
 */

#include "jdefines.h"
#include <unistring.h>

#ifndef __JATOM_H_
#define __JATOM_H_

#include "nscalutilexp.h"

/**
 *  An atom class for UnicodeStrings.  Used to speed up
 *  comparison of strings by calculating hashcode and comparing
 *  that.
 */
class NS_CAL_UTIL JAtom
{
private:
    /* UnicodeString m_String; */

    /** strings' hashcode */
    t_int32 m_HashCode;

public:

    /** default constructor */
    JAtom();

    /** sets the hashcode variable to string's hashcode value */
    JAtom(const UnicodeString & string);

    /**
     * sets the hashcode variable to string's hashcode value
     * @param           string      string to get hashcode from
     */
    void setString(const UnicodeString & string);


    /* UnicodeString toString() const { return m_String; } */

    /**
     * Return the hashcode
     *
     * @return          hashcode to return
     */
    t_int32 hashCode() const { return m_HashCode; }

    /**
     * Equality comparison with t_int32.
     * @param           aHashCode   hashcode to compare with
     *
     * @return          TRUE if equal, FALSE otherwise
     */
    t_bool operator==(t_int32 aHashCode) const { return (m_HashCode == aHashCode); }

    /**
     * Equality comparison with atom.
     * @param           anAtom      atom to compare with
     *
     * @return          TRUE if equal, FALSE otherwise
     */
    t_bool operator==(JAtom & anAtom) const { return (m_HashCode == anAtom.m_HashCode); }

    /**
     * Inequality comparison with t_int32.
     * @param           aHashCode   hashcode to compare with
     *
     * @return          TRUE if NOT equal, FALSE otherwise
     */
    t_bool operator!=(t_int32 aHashCode) const { return (m_HashCode != aHashCode); }

    /**
     * Inequality comparison with atom.
     * @param           anAtom      atom to compare with
     *
     * @return          TRUE if NOT equal, FALSE otherwise
     */
    t_bool operator!=(JAtom & anAtom) const { return (m_HashCode != anAtom.m_HashCode); }

};

#endif /* __JATOM_H_ */

