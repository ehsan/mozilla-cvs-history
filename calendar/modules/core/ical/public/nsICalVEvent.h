/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef nsICalVEvent_h___
#define nsICalVEvent_h___

#include "nsISupports.h"
#include "vevent.h"

//20355f10-6912-11d2-943d-006008268c31
#define NS_ICALVEVENT_IID \
{ 0x20355f10, 0x6912, 0x11d2, \
{ 0x94, 0x3d, 0x00, 0x60, 0x08, 0x26, 0x8c, 0x31 } }

class nsICalVEvent : public nsISupports
{
public: 

  // for now.
  NS_IMETHOD_(DateTime) GetDTEnd() const = 0;
  NS_IMETHOD SetDTEnd(DateTime d, JulianPtrArray * parameters) = 0;
  NS_IMETHOD SetDTEndProperty(nsICalProperty * property) = 0;

  NS_IMETHOD_(VEvent *) GetICalEvent() = 0;
  NS_IMETHOD_(PRBool) IsValid() = 0;
};

#endif





