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
 * vjournal.cpp
 * John Sun
 * 4/23/98 10:33:36 AM
 */

#include "stdafx.h"
#include "jdefines.h"

#include "jutility.h"
#include "vjournal.h"
#include "icalredr.h"
#include "prprtyfy.h"
#include "unistrto.h"
#include "jlog.h"
#include "vtimezne.h"
#include "keyword.h"
#include "period.h"
#include "datetime.h"

//---------------------------------------------------------------------
void VJournal::setDefaultFmt(UnicodeString s)
{
    nsCalFormatString::Instance()->ms_VJournalStrDefaultFmt = s;
}
//---------------------------------------------------------------------
#if 0
VJournal::VJournal()
{
    PR_ASSERT(FALSE);
}
#endif
//---------------------------------------------------------------------

VJournal::VJournal(JLog * initLog)
: TimeBasedEvent(initLog)
{
}

//---------------------------------------------------------------------

VJournal::VJournal(VJournal & that)
: TimeBasedEvent(that)
{
}

//---------------------------------------------------------------------

ICalComponent *
VJournal::clone(JLog * initLog)
{
    m_Log = initLog;
    return new VJournal(*this);
}

//---------------------------------------------------------------------

VJournal::~VJournal()
{
    // should call TimeBasedEvent destructor
}

//---------------------------------------------------------------------

UnicodeString &
VJournal::parse(ICalReader * brFile, UnicodeString & sMethod, 
                UnicodeString & parseStatus, JulianPtrArray * vTimeZones,
                t_bool bIgnoreBeginError, nsCalUtility::MimeEncoding encoding) 
{
    UnicodeString u = nsCalKeyword::Instance()->ms_sVJOURNAL;
    return parseType(u, brFile, sMethod, parseStatus, vTimeZones, bIgnoreBeginError, encoding);
}

//---------------------------------------------------------------------

Date VJournal::difference()
{
    return 0;
}

//---------------------------------------------------------------------

void VJournal::selfCheck()
{
    TimeBasedEvent::selfCheck();   
    if (getStatus().size() > 0)
    {
        // check if I got a valid status
        UnicodeString u = getStatus();
        u.toUpper();
        ICalProperty::Trim(u);
        JAtom ua(u);
        if ((nsCalKeyword::Instance()->ms_ATOM_DRAFT!= ua) && 
            (nsCalKeyword::Instance()->ms_ATOM_FINAL != ua) && 
            (nsCalKeyword::Instance()->ms_ATOM_CANCELLED != ua))
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iInvalidPropertyValue, 
                nsCalKeyword::Instance()->ms_sVJOURNAL, 
                nsCalKeyword::Instance()->ms_sSTATUS, u, 200);

            setStatus("");
        }
    }
}

//---------------------------------------------------------------------

t_bool 
VJournal::storeData(UnicodeString & strLine, UnicodeString & propName, 
                    UnicodeString & propVal, JulianPtrArray * parameters, 
                    JulianPtrArray * vTimeZones)
{
    if (TimeBasedEvent::storeData(strLine, propName, propVal, 
                                  parameters, vTimeZones))
        return TRUE;
    else
    {
        if (m_Log) m_Log->logError(
            nsCalLogErrorMessage::Instance()->ms_iInvalidPropertyName, 
            nsCalKeyword::Instance()->ms_sVJOURNAL, propName, 200);
        UnicodeString u;
        u = nsCalLogErrorMessage::Instance()->ms_sRS202;
        u += '.'; u += ' ';
        u += strLine;
        //setRequestStatus(nsCalLogErrorMessage::Instance()->ms_iRS202); 
        addRequestStatus(u);
        return FALSE;
    }
}

//---------------------------------------------------------------------

void VJournal::populateDatesHelper(DateTime start, Date ldiff, 
                                   JulianPtrArray * vPeriods)
{
}

//---------------------------------------------------------------------

UnicodeString VJournal::formatHelper(UnicodeString & strFmt, 
                                     UnicodeString sFilterAttendee, 
                                     t_bool delegateRequest) 
{
    UnicodeString u = nsCalKeyword::Instance()->ms_sVJOURNAL;
    return ICalComponent::format(u, strFmt, sFilterAttendee, delegateRequest);
}

//---------------------------------------------------------------------

UnicodeString VJournal::toString()
{
    return ICalComponent::toStringFmt(
        nsCalFormatString::Instance()->ms_VJournalStrDefaultFmt);
}

//---------------------------------------------------------------------

UnicodeString VJournal::toStringChar(t_int32 c, UnicodeString & dateFmt)
{
    return TimeBasedEvent::toStringChar(c, dateFmt);
}

//---------------------------------------------------------------------

UnicodeString VJournal::formatChar(t_int32 c, UnicodeString sFilterAttendee, 
                                 t_bool delegateRequest) 
{
    return TimeBasedEvent::formatChar(c, sFilterAttendee, delegateRequest);
}

//---------------------------------------------------------------------

t_bool VJournal::isValid()
{
    // TODO: finish
    if ((getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sPUBLISH) == 0) ||
        (getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sREQUEST) == 0) ||
        (getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sADD) == 0))
    {
        // must have dtstart
        if ((!getDTStart().isValid()))
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingStartingTime, 300);
            return FALSE;
        }

        // If due exists, make sure it is not before dtstart
        //if (getDTEnd().isValid() && getDTEnd() < getDTStart())
        //    return FALSE;

        // must have dtstamp, summary, uid
        if (!getDTStamp().isValid())
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingDTStamp, 300);
            return FALSE;
        }
        if (getSummary().size() == 0) 
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingSummary, 300);
            return FALSE;
        }
        if (getUID().size() == 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingUID, 300);
            return FALSE;
        }
        // must have organizer
        if (getOrganizer().size() == 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingOrganizer, 300);
            return FALSE;
        }
        // must have sequence >= 0
        if (getSequence() < 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingSeqNo, 300);
            return FALSE;
        }
        if (getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sREQUEST) == 0)
        {
            if (getAttendees() == 0 || getAttendees()->GetSize() == 0)
            {
                if (m_Log) m_Log->logError(
                    nsCalLogErrorMessage::Instance()->ms_iMissingAttendees, 300);
                return FALSE;
            }
        }
    }
    else if ((getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sREPLY) == 0) ||
             (getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sCANCEL) == 0) ||
             (getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sDECLINECOUNTER) == 0))
    {
         // must have dtstamp, uid
        if (!getDTStamp().isValid())
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingDTStamp, 300);
            return FALSE;
        }
        if (getUID().size() == 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingUID, 300);
            return FALSE;
        }
        // must have organizer
        if (getOrganizer().size() == 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingOrganizer, 300);
            return FALSE;
        }
        // must have sequence >= 0
        if (getSequence() < 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingSeqNo, 300);
            return FALSE;
        }
        if (getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sREPLY) == 0)
        {
            if (getAttendees() == 0 || getAttendees()->GetSize() == 0)
            {
                if (m_Log) m_Log->logError(
                    nsCalLogErrorMessage::Instance()->ms_iMissingAttendees, 300);
                return FALSE;
            }
        }
    }
    else if ((getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sREFRESH) == 0))
    {
        // must have dtstamp, uid
        if (!getDTStamp().isValid())  
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingDTStamp, 300);
            return FALSE;
        }
        if (getUID().size() == 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingDTStamp, 300);
            return FALSE;
        }
        // TODO: attendees required?
    }
    else if ((getMethod().compareIgnoreCase(nsCalKeyword::Instance()->ms_sCOUNTER) == 0))
    {
        // must have dtstamp, uid
        if (!getDTStamp().isValid()) 
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingDTStamp, 300);
            return FALSE;
        }
        if (getUID().size() == 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingDTStamp, 300);
            return FALSE;
        }

        // must have sequence >= 0
        if (getSequence() < 0)
        {
            if (m_Log) m_Log->logError(
                nsCalLogErrorMessage::Instance()->ms_iMissingSeqNo, 300);
            return FALSE;
        }
    }
            
    // check super class isValid method
    return TimeBasedEvent::isValid();
}

//---------------------------------------------------------------------

UnicodeString VJournal::cancelMessage() 
{
    UnicodeString s = nsCalKeyword::Instance()->ms_sCANCELLED;
    setStatus(s);
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalCancelMessage, "");
}

//---------------------------------------------------------------------

UnicodeString VJournal::requestMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalRequestMessage, "");
}

//---------------------------------------------------------------------

UnicodeString VJournal::requestRecurMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalRecurRequestMessage, "");
}

//---------------------------------------------------------------------
 
UnicodeString VJournal::counterMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalCounterMessage, "");
}

//---------------------------------------------------------------------
  
UnicodeString VJournal::declineCounterMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalDeclineCounterMessage, "");
}

//---------------------------------------------------------------------

UnicodeString VJournal::addMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalAddMessage, "");
}

//---------------------------------------------------------------------
  
UnicodeString VJournal::refreshMessage(UnicodeString sAttendeeFilter) 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalRefreshMessage, sAttendeeFilter);
}

//---------------------------------------------------------------------
 
UnicodeString VJournal::allMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalAllPropertiesMessage, "");
}

//---------------------------------------------------------------------
 
UnicodeString VJournal::replyMessage(UnicodeString sAttendeeFilter) 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalReplyMessage, sAttendeeFilter);
}

//---------------------------------------------------------------------
 
UnicodeString VJournal::publishMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalPublishMessage, "");
}

//---------------------------------------------------------------------
 
UnicodeString VJournal::publishRecurMessage() 
{
    return formatHelper(nsCalFormatString::Instance()->ms_sVJournalRecurPublishMessage, "");
}

//---------------------------------------------------------------------

void 
VJournal::updateComponentHelper(TimeBasedEvent * updatedComponent)
{
    TimeBasedEvent::updateComponentHelper(updatedComponent);
}
