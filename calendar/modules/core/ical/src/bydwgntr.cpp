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

// bydwgntr.cpp
// John Sun
// 4:10 PM February 3 1998

#include "stdafx.h"
#include "jdefines.h"

#include <assert.h>
#include "bydwgntr.h"
#include "jutility.h"

ByDayWeeklyGenerator::ByDayWeeklyGenerator() 
:
    DateGenerator(nsCalUtility::RT_WEEKLY, 0),
    m_aiParams(0),
    m_iWkSt((t_int32) Calendar::SUNDAY)
{}

//---------------------------------------------------------------------

t_int32 
ByDayWeeklyGenerator::getInterval() const { return nsCalUtility::RT_DAILY; }

//---------------------------------------------------------------------

t_bool 
ByDayWeeklyGenerator::generate(DateTime * start, JulianPtrArray & dateVector,
                                      DateTime * until)
{
    DateTime * t, * nextWkSt;
    t_int32 i;

    PR_ASSERT(start != 0);
    PR_ASSERT(m_aiParams != 0);
    if (start == 0 || m_aiParams == 0)
    {
        return FALSE;
    }

    t = new DateTime(start->getTime());  PR_ASSERT(t != 0);
    nextWkSt = new DateTime(t->getTime()); PR_ASSERT(nextWkSt != 0);

    nextWkSt->moveTo(m_iWkSt, 1, TRUE);

    for (i = 0; i < m_iParamsLen; i++)
    {
        t->setTime(start->getTime());
        t->moveTo(m_aiParams[i][0], 1, FALSE);

        if (until != 0 && until->isValid() && t->after(until))
        {
            delete t; t = 0;
            delete nextWkSt; nextWkSt = 0;
            return TRUE;
        }
        if ((t->after(start) || t->equals(start)) && t->before(nextWkSt))
        {
            dateVector.Add(new DateTime(t->getTime()));
        }
    }
    delete t; t = 0;
    delete nextWkSt; nextWkSt = 0;
    return FALSE;
}
//---------------------------------------------------------------------


