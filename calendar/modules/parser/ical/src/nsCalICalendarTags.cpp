/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */

/* Do not edit - generated by gentags.pl */
#include "nsCRT.h"
#include "nsCalICalendarTags.h"

static char* tagTable[] = {
  "ACTION", "ATTACH", "ATTENDEE", "BEGIN", "CALSCALE", "CATEGORIES", "CLASS", 
  "COMMENT", "COMPLETED", "CONTACT", "CREATED", "DAYLIGHT", "DESCRIPTION", 
  "DTEND", "DTSTART", "DUE", "DURATION", "END", "EXDATE", "EXRULE", 
  "FREEBUSY", "GEO", "LAST-MODIFIED", "LOCATION", "METHOD", "ORGANIZER", 
  "PERCENT-COMPLETE", "PRIORITY", "PRODID", "RDATE", "RECURRENCE-ID", 
  "RELATED-TO", "REPEAT", "REQUEST-STATUS", "RESOURCES", "RRULE", "SEQUENCE", 
  "STANDARD", "STATUS", "SUMMARY", "TRANSP", "TRIGGER", "TZID", "TZNAME", 
  "TZOFFSETFROM", "TZOFFSETTO", "TZURL", "UID", "URL", "VALARM", "VCALENDAR", 
  "VERSION", "VEVENT", "VFREEBUSY", "VJOURNAL", "VTIMEZONE", "VTODO"
};

nsCalICalendarTag NS_CalICalendarTagToEnum(const char* aTag) {
  int low = 0;
  int high = NS_CALICALENDAR_TAG_MAX - 1;
  while (low <= high) {
    int middle = (low + high) >> 1;
    int result = nsCRT::strcasecmp(aTag, tagTable[middle]);
    if (result == 0)
      return (nsCalICalendarTag) (middle + 1);
    if (result < 0)
      high = middle - 1; 
    else
      low = middle + 1; 
  }
  return eCalICalendarTag_userdefined;
}

const char* NS_CalICalendarEnumToTag(nsCalICalendarTag aTagID) {
  if ((int(aTagID) <= 0) || (int(aTagID) > NS_CALICALENDAR_TAG_MAX)) {
    return 0;
  }
  return tagTable[int(aTagID) - 1];
}

#ifdef NS_DEBUG
#include <stdio.h>

class nsCalICalendarTestTagTable {
public:
   nsCalICalendarTestTagTable() {
     const char *tag;
     nsCalICalendarTag id;

     // Make sure we can find everything we are supposed to
     for (int i = 0; i < NS_CALICALENDAR_TAG_MAX; i++) {
       tag = tagTable[i];
       id = NS_CalICalendarTagToEnum(tag);
       NS_ASSERTION(id != eCalICalendarTag_userdefined, "can't find tag id");
       const char* check = NS_CalICalendarEnumToTag(id);
       NS_ASSERTION(check == tag, "can't map id back to tag");
     }

     // Make sure we don't find things that aren't there
     id = NS_CalICalendarTagToEnum("@");
     NS_ASSERTION(id == eCalICalendarTag_userdefined, "found @");
     id = NS_CalICalendarTagToEnum("zzzzz");
     NS_ASSERTION(id == eCalICalendarTag_userdefined, "found zzzzz");

     tag = NS_CalICalendarEnumToTag((nsCalICalendarTag) 0);
     NS_ASSERTION(0 == tag, "found enum 0");
     tag = NS_CalICalendarEnumToTag((nsCalICalendarTag) -1);
     NS_ASSERTION(0 == tag, "found enum -1");
     tag = NS_CalICalendarEnumToTag((nsCalICalendarTag) (NS_CALICALENDAR_TAG_MAX + 1));
     NS_ASSERTION(0 == tag, "found past max enum");
   }
};
nsCalICalendarTestTagTable validateTagTable;
#endif

