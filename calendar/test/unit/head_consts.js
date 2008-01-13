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
 * The Original Code is Mozilla Calendar tests code.
 *
 * The Initial Developer of the Original Code is
 * Michiel van Leeuwen <mvl@exedo.nl>.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Daniel Boelzle <daniel.boelzle@sun.com>
 *   Sebastian Schwieger <sebo.moz@googlemail.com>
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

const Cc = Components.classes;
const Ci = Components.interfaces;

/// Shortcut to the timezone service
function getTimezoneService() {
    if (getTimezoneService.mObject === undefined) {
        getTimezoneService.mObject = Cc["@mozilla.org/calendar/timezone-service;1"]
                                     .getService(Ci.calITimezoneService);
    }
    return getTimezoneService.mObject;
}

/// @return the UTC timezone.
function UTC() {
    if (UTC.mObject === undefined) {
        UTC.mObject = getTimezoneService().UTC;
    }
    return UTC.mObject;
}

/// @return the floating timezone.
function floating() {
    if (floating.mObject === undefined) {
        floating.mObject = getTimezoneService().floating;
    }
    return floating.mObject;
}

function createDate(aYear, aMonth, aDay) {
    var cd = Cc["@mozilla.org/calendar/datetime;1"]
             .createInstance(Ci.calIDateTime);
    cd.resetTo(aYear, aMonth, aDay, 0, 0, 0, UTC());
    cd.isDate = true;
    return cd;
}

function createEventFromIcalString(icalString) {
    var event = Cc["@mozilla.org/calendar/event;1"]
                .createInstance(Ci.calIEvent);
    event.icalString = icalString;
    return event;
}

function createTodoFromIcalString(icalString) {
    var todo = Cc["@mozilla.org/calendar/todo;1"]
               .createInstance(Ci.calITodo);
    todo.icalString = icalString;
    return todo;
}

function getMemoryCal() {
    // create memory calendar
    var cal = Cc["@mozilla.org/calendar/calendar;1?type=memory"]
              .createInstance(Ci.calICalendar);

    // remove existing items
    var calendar = cal.QueryInterface(Ci.calICalendarProvider);
    try {
        calendar.deleteCalendar(calendar, null);
    } catch (e) {
        print("*** error purging calendar: " + e);
    }
    return cal;
}

function getStorageCal() {
    var dirSvc = Cc["@mozilla.org/file/directory_service;1"]
                 .getService(Ci.nsIProperties);
    var db = dirSvc.get("TmpD", Ci.nsIFile);
    db.append("test_storage.sqlite");

    // create URI
    var ioSvc = Cc["@mozilla.org/network/io-service;1"]
                .getService(Ci.nsIIOService);
    var uri = ioSvc.newFileURI(db);

    // create storage calendar
    var cal = Cc["@mozilla.org/calendar/calendar;1?type=storage"]
              .createInstance(Ci.calICalendar);
    cal.uri = uri;

    // remove existing items
    var calendar = cal.QueryInterface(Ci.calICalendarProvider);
    try {
        calendar.deleteCalendar(calendar, null);
    } catch (e) {
        print("*** error purging calendar: " + e);
    }
    return cal;
}

/**
 * Return an item property as string.
 * @param aItem
 * @param string aProp possible item properties: start, end, duration,
 *                     generation, title,
 *                     id, calendar, creationDate, lastModifiedTime,
 *                     stampTime, priority, privacy, status,
 *                     alarmOffset, alarmRelated,
 *                     alarmLastAck, recurrenceStartDate
 *                     and any property that can be obtained using getProperty()
 */
function getProps(aItem, aProp) {
    var value = null;
    switch (aProp) {
        case "start":
            value = aItem.startDate || aItem.entryDate || null;
            break;
        case "end":
            value = aItem.endDate || aItem.dueDate || null;
            break;
        case "duration":
            value = aItem.duration || null;
            break;
        case "generation":
            value = aItem.generation;
            break;
        case "title":
            value = aItem.title;
            break;
        case "id":
            value = aItem.id;
            break;
        case "calendar":
            value = aItem.calendar.id;
            break;
        case "creationDate":
            value = aItem.creationDate;
            break;
        case "lastModifiedTime":
            value = aItem.lastModifiedTime;
            break;
        case "stampTime":
            value = aItem.stampTime;
            break;
        case "priority":
            value = aItem.priority;
            break;
        case "privacy":
            value = aItem.privacy;
            break;
        case "status":
            value = aItem.status;
            break;
        case "alarmOffset":
            value = aItem.alarmOffset;
            break;
        case "alarmRelated":
            value = aItem.alarmRelated;
            break;
        case "alarmLastAck":
            value = aItem.alarmLastAck;
            break;
        case "recurrenceStartDate":
            value = aItem.recurrenceStartDate;
            break;
        default:
            value = aItem.getProperty(aProp);
    }
    if (value) {
        return value.toString();
    } else {
        return null;
    }
}

function compareItems(aLeftItem, aRightItem, aPropArray) {
    if (!aPropArray) {
        // left out:  "id", "calendar", "lastModifiedTime", "generation",
        // "stampTime" as these are expected to change
        aPropArray = ["start", "end", "duration",
                      "title", "priority", "privacy", "creationDate",
                      "status", "alarmOffset", "alarmRelated", "alarmLastAck",
                      "recurrenceStartDate"];
    }
    for (var i = 0; i < aPropArray.length; i++) {
        do_check_eq(getProps(aLeftItem, aPropArray[i]),
                    getProps(aRightItem,
                    aPropArray[i]));
    }
}

// Support do_check_true and  do_check_false on MOZILLA_1_8_BRANCH
if (typeof do_check_true != "function") {
    do_check_true = function do_check_true(condition) {
        do_check_eq(condition, true);
    }
}
if (typeof do_check_false != "function") {
    do_check_false = function do_check_false(condition) {
        do_check_eq(condition, false);
    }
}
