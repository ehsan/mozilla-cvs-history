/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "smpdtfmt.h"

FieldPosition::FieldPosition()
{
}

FieldPosition::~FieldPosition()
{
}

FieldPosition::FieldPosition(PRInt32 aField)
{
}


SimpleDateFormat::SimpleDateFormat()
{
}

SimpleDateFormat::~SimpleDateFormat()
{
}

SimpleDateFormat::SimpleDateFormat(ErrorCode& aStatus)
{
}

SimpleDateFormat::SimpleDateFormat(const UnicodeString& aPattern, const Locale& aLocale, ErrorCode& aStatus)
{
}


UnicodeString& SimpleDateFormat::format(Date aDate, UnicodeString& aAppendTo, FieldPosition& aPosition) const
{
  UnicodeString u;
  return (u);
}

UnicodeString& SimpleDateFormat::format(const Formattable& aObject, UnicodeString& aAppendTo, FieldPosition& aPosition, ErrorCode& aStatus) const
{
  UnicodeString u;
  return (u);
}

void SimpleDateFormat::applyLocalizedPattern(const UnicodeString& aPattern, ErrorCode& aStatus)
{
  return ;
}

Date SimpleDateFormat::parse(const UnicodeString& aUnicodeString, ParsePosition& aPosition) const
{
  return ((Date)nsnull);
}

PRBool SimpleDateFormat::operator==(const Format& other) const
{
  return PR_TRUE;
}

