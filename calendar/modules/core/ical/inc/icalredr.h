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
 * icalredr.h
 * John Sun
 * 2/10/98 11:33:55 AM
 */

#include "jdefines.h"
#include <unistring.h>
#include "nscalcoreicalexp.h"

#ifndef __ICALREADER_H_
#define __ICALREADER_H_

/**
 * ICalReader is an abstract class that defines an interface for
 * reading iCalendar objects from a data source.  All methods in
 * this class are defined to be pure virtual methods.  Currently, this
 * data source can be a string (ICalStringReader) or a file (ICalFileReader).
 * The interface defined allows for the reading of a character, a line, and a full-line.
 * A full-line is defined by the iCal Content-Line definition.
 * For clients, object acts like an input stream of UnicodeString data.
 */
class NS_CAL_CORE_ICAL ICalReader
{
protected:

    /**
     * Hide constructor from clients.
     */
    ICalReader();

public:

    virtual ~ICalReader() {}
    /**
     * Pure virtual.  Read the next character from the data.
     * @param           status      return status > 0 if failure in reading character
     *
     * @return          next character in data
     */
    virtual t_int8 read(ErrorCode & status)
    {
        PR_ASSERT(FALSE);
        if (status == 0) {}
        return -1;
    }

    /**
     * Pure virtual.  Read the next line from the data.  A line
     * is defined to be characters terminated by a '\n', '\r', or '\n\r'.
     * @param           aLine       placeholder to return next line
     * @param           status      return status > 0 if failure in reading line
     *
     * @return          aLine (next line)
     */
    virtual UnicodeString & readLine(UnicodeString & aLine, ErrorCode & status)
    {
        PR_ASSERT(FALSE);
        if (status == 0) {}
        if (aLine.size() == 0) {}
        return aLine;
    }

    /**
     * Pure virtual.  Read the next full-line from the data.  A full-line
     * is defined by the iCalendar Content-Line definition.
     * @param           aLine       placeholder to return next full-line
     * @param           status      return status > 0 if failure in reading full-line
     *
     * @return          aLine (next full-line)
     */
    virtual UnicodeString & readFullLine(UnicodeString & aLine, ErrorCode & status, t_int32 i = 0)
    {
        PR_ASSERT(FALSE);
        if (status == 0) {}
        if (aLine.size() == 0) {}
        return aLine;
    }

    /**
     * Sets a the buffer to read from.  This method should only be overwritten by
     * the ICalCAPIReader class. (4-16-98)
     */
    virtual void setBuffer(const char * capiChunk)
    {
        if (capiChunk != 0) {}
    }

    virtual void * getMonitor() { return 0; }

    /* static methods */

    static t_bool isHex(t_int8 aToken);
    static t_int8 convertHex(char fToken, char sToken);
    static UnicodeString & convertQuotedPrintableString(UnicodeString & stringToConvert);
};

#endif /* __ICALREADER_H_ */


