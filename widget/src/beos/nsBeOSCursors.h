/* -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Yannick Koehler <koehler@mythrium.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
 
#ifndef nsBeOSCursors_h__
#define nsBeOSCursors_h__

#include <app/Cursor.h>

static const uint8 cursorHyperlink[] = {
  0x10, 0x01, 0x2, 0x2, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x00,
  0x38, 0x00,
  0x24, 0x00,
  0x24, 0x00,
  0x13, 0xE0,
  0x12, 0x5C,
  0x09, 0x2A,
  0x08, 0x01,
  0x3C, 0x21,
  0x4C, 0x71,
  0x42, 0x71,
  0x30, 0xF9,
  0x0C, 0xF9,
  0x02, 0x00,
  0x01, 0x00,
  // Bitmask Data
  0x00, 0x00,
  0x00, 0x00,
  0x38, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x1F, 0xE0,
  0x1F, 0xFD,
  0x0F, 0xFE,
  0x0F, 0xFF,
  0x3F, 0xFF,
  0x7F, 0xFF,
  0x7F, 0xFF,
  0x3F, 0xFF,
  0x0F, 0xFF,
  0x03, 0xFE,
  0x01, 0xF8
};
static const BCursor bCursorHyperlink(cursorHyperlink);
#define B_CURSOR_HYPERLINK &bCursorHyperlink

static const uint8 cursorHelp[] = {
  0x10, 0x01, 0x2, 0x2, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x00,
  0x38, 0x00,
  0x24, 0x00,
  0x24, 0x00,
  0x13, 0xE0,
  0x12, 0x5C,
  0x09, 0x2A,
  0x08, 0x01,
  0x3C, 0x79,
  0x4C, 0xCD,
  0x42, 0x0D,
  0x30, 0x39,
  0x0C, 0x31,
  0x02, 0x00,
  0x01, 0x30,
  // Bitmask Data
  0x00, 0x00,
  0x00, 0x00,
  0x38, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x1F, 0xE0,
  0x1F, 0xFD,
  0x0F, 0xFE,
  0x0F, 0xFF,
  0x3F, 0xFF,
  0x7F, 0xFF,
  0x7F, 0xFF,
  0x3F, 0xFF,
  0x0F, 0xFF,
  0x03, 0xFE,
  0x01, 0xF8
};
static const BCursor bCursorHelp(cursorHelp);
#define B_CURSOR_HELP &bCursorHelp

static const uint8 cursorWait[] = {
  0x10, 0x01, 0x7, 0x8, 
  // Icon Data
  0x00, 0x04,
  0x07, 0xC6,
  0x18, 0x3F,
  0x21, 0x0C,
  0x41, 0x04,
  0x41, 0x04,
  0x81, 0x02,
  0x81, 0x02,
  0x81, 0x02,
  0x82, 0x02,
  0x84, 0x02,
  0x48, 0x04,
  0x40, 0x04,
  0x20, 0x08,
  0x18, 0x30,
  0x07, 0xC0,
  // Bitmask Data
  0x00, 0x04,
  0x07, 0xC6,
  0x1F, 0xFF,
  0x3F, 0xFC,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x3F, 0xF8,
  0x1F, 0xF0,
  0x07, 0xC0
};
static const BCursor bCursorWait(cursorWait);
#define B_CURSOR_WAIT &bCursorWait

static const uint8 cursorSpinning[] = {
  0x10, 0x01, 0x7, 0x7, 
  // Icon Data
  0x07, 0xC0,
  0x1F, 0x30,
  0x3F, 0x08,
  0x7F, 0x04,
  0x7F, 0x04,
  0xFF, 0x02,
  0xFF, 0x02,
  0xFF, 0xFE,
  0x81, 0xFE,
  0x81, 0xFE,
  0x41, 0xFC,
  0x41, 0xFC,
  0x21, 0xF8,
  0x19, 0xF0,
  0x07, 0xC0,
  0x00, 0x00,
  // Bitmask Data
  0x07, 0xC0,
  0x1F, 0xF0,
  0x3F, 0xF8,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x3F, 0xF8,
  0x1F, 0xF0,
  0x07, 0xC0,
  0x00, 0x00
};
static const BCursor bCursorSpinning(cursorSpinning);
#define B_CURSOR_SPINNING &bCursorSpinning

static const uint8 cursorNorthSouth[] = {
  0x10, 0x01, 0x7, 0x7, 
  // Icon Data
  0x01, 0x00,
  0x03, 0x80,
  0x07, 0xC0,
  0x0F, 0xE0,
  0x01, 0x00,
  0x01, 0x00,
  0x7F, 0xFE,
  0x00, 0x00,
  0x00, 0x00,
  0x7F, 0xFE,
  0x01, 0x00,
  0x01, 0x00,
  0x0F, 0xE0,
  0x07, 0xC0,
  0x03, 0x80,
  0x01, 0x00,
  // Bitmask Data
  0x01, 0x00,
  0x03, 0x80,
  0x07, 0xC0,
  0x0F, 0xE0,
  0x01, 0x00,
  0x01, 0x00,
  0x7F, 0xFE,
  0x00, 0x00,
  0x00, 0x00,
  0x7F, 0xFE,
  0x01, 0x00,
  0x01, 0x00,
  0x0F, 0xE0,
  0x07, 0xC0,
  0x03, 0x80,
  0x01, 0x00
};
static const BCursor bCursorNorthSouth(cursorNorthSouth);
#define B_CURSOR_NORTHSOUTH &bCursorNorthSouth

static const uint8 cursorWestEast[] = {
  0x10, 0x01, 0x7, 0x7, 
  // Icon Data
  0x00, 0x00,
  0x02, 0x40,
  0x02, 0x40,
  0x02, 0x40,
  0x12, 0x48,
  0x32, 0x4C,
  0x72, 0x4E,
  0xFE, 0x7F,
  0x72, 0x4E,
  0x32, 0x4C,
  0x12, 0x48,
  0x02, 0x40,
  0x02, 0x40,
  0x02, 0x40,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x00, 0x00,
  0x02, 0x40,
  0x02, 0x40,
  0x02, 0x40,
  0x12, 0x48,
  0x32, 0x4C,
  0x72, 0x4E,
  0xFE, 0x7F,
  0x72, 0x4E,
  0x32, 0x4C,
  0x12, 0x48,
  0x02, 0x40,
  0x02, 0x40,
  0x02, 0x40,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorWestEast(cursorWestEast);
#define B_CURSOR_WESTEAST &bCursorWestEast

static const uint8 cursorSouthEast[] = {
  0x10, 0x01, 0xD, 0xD, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x04, 0x06,
  0x02, 0x26,
  0x01, 0x26,
  0x00, 0xA6,
  0x00, 0x66,
  0x03, 0xE6,
  0x00, 0x06,
  0x00, 0x06,
  0x07, 0xFE,
  0x07, 0xFE,
  0x00, 0x00,
  // Bitmask Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x04, 0x0F,
  0x0E, 0x7F,
  0x07, 0x7F,
  0x03, 0xFF,
  0x01, 0xFF,
  0x07, 0xFF,
  0x07, 0xFF,
  0x07, 0xFF,
  0x0F, 0xFF,
  0x0F, 0xFF,
  0x0F, 0xFF,
  0x0F, 0xFF
};
static const BCursor bCursorSouthEast(cursorSouthEast);
#define B_CURSOR_SOUTHEAST &bCursorSouthEast

static const uint8 cursorSouthWest[] = {
  0x10, 0x01, 0xD, 0x2, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x60, 0x20,
  0x64, 0x40,
  0x64, 0x80,
  0x65, 0x00,
  0x66, 0x00,
  0x67, 0xC0,
  0x60, 0x00,
  0x60, 0x00,
  0x7F, 0xE0,
  0x7F, 0xE0,
  0x00, 0x00,
  // Bitmask Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0xF0, 0x20,
  0xFE, 0x70,
  0xFE, 0xE0,
  0xFF, 0xC0,
  0xFF, 0x80,
  0xFF, 0xE0,
  0xFF, 0xE0,
  0xFF, 0xE0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0
};
static const BCursor bCursorSouthWest(cursorSouthWest);
#define B_CURSOR_SOUTHWEST &bCursorSouthWest

static const uint8 cursorNorthWest[] = {
  0x10, 0x01, 0x2, 0x2, 
  // Icon Data
  0x00, 0x00,
  0x7F, 0xE0,
  0x7F, 0xE0,
  0x60, 0x00,
  0x60, 0x00,
  0x67, 0xC0,
  0x66, 0x00,
  0x65, 0x00,
  0x64, 0x80,
  0x64, 0x40,
  0x60, 0x20,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xE0,
  0xFF, 0xE0,
  0xFF, 0xE0,
  0xFF, 0x80,
  0xFF, 0xC0,
  0xFE, 0xE0,
  0xFE, 0x70,
  0xF0, 0x20,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorNorthWest(cursorNorthWest);
#define B_CURSOR_NORTHWEST &bCursorNorthWest

static const uint8 cursorNorthEast[] = {
  0x10, 0x01, 0x2, 0xD, 
  // Icon Data
  0x00, 0x00,
  0x07, 0xFE,
  0x07, 0xFE,
  0x00, 0x06,
  0x00, 0x06,
  0x03, 0xE6,
  0x00, 0x66,
  0x00, 0x96,
  0x01, 0x26,
  0x02, 0x26,
  0x04, 0x06,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x0F, 0xFF,
  0x0F, 0xFF,
  0x0F, 0xFF,
  0x0F, 0xFF,
  0x0E, 0xFF,
  0x0E, 0xFF,
  0x0E, 0xFF,
  0x01, 0xFF,
  0x03, 0xFF,
  0x07, 0x7F,
  0x0E, 0x7F,
  0x04, 0x0F,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorNorthEast(cursorNorthEast);
#define B_CURSOR_NORTHEAST &bCursorNorthEast

static const uint8 cursorNorth[] = {
  0x10, 0x01, 0x2, 0x7, 
  // Icon Data
  0x00, 0x00,
  0x3F, 0xF8,
  0x3F, 0xF8,
  0x00, 0x00,
  0x00, 0x00,
  0x01, 0x00,
  0x03, 0x80,
  0x05, 0x40,
  0x09, 0x20,
  0x01, 0x00,
  0x01, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x01, 0x00,
  0x03, 0x80,
  0x07, 0xC0,
  0x0F, 0xE0,
  0x1F, 0xF0,
  0x0B, 0xA0,
  0x03, 0x80,
  0x03, 0x80,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorNorth(cursorNorth);
#define B_CURSOR_NORTH &bCursorNorth

static const uint8 cursorSouth[] = {
  0x10, 0x01, 0xD, 0x7, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x09, 0x20,
  0x05, 0x40,
  0x03, 0x80,
  0x01, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x3F, 0xF8,
  0x3F, 0xF8,
  0x00, 0x00,
  // Bitmask Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x03, 0x80,
  0x03, 0x80,
  0x0B, 0xA0,
  0x1F, 0xF0,
  0x0F, 0xE0,
  0x07, 0xC0,
  0x03, 0x80,
  0x01, 0x00,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x7F, 0xFC,
  0x7F, 0xFC
};
static const BCursor bCursorSouth(cursorSouth);
#define B_CURSOR_SOUTH &bCursorSouth

static const uint8 cursorEast[] = {
  0x10, 0x01, 0x6, 0xD, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x06,
  0x00, 0x06,
  0x01, 0x06,
  0x00, 0x86,
  0x00, 0x46,
  0x0F, 0xE6,
  0x00, 0x46,
  0x00, 0x86,
  0x01, 0x06,
  0x00, 0x06,
  0x00, 0x06,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x00, 0x0F,
  0x00, 0x0F,
  0x01, 0x0F,
  0x03, 0x8F,
  0x01, 0xCF,
  0x1F, 0xEF,
  0x1F, 0xFF,
  0x1F, 0xEF,
  0x01, 0xCF,
  0x03, 0x8F,
  0x01, 0x0F,
  0x00, 0x0F,
  0x00, 0x0F,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorEast(cursorEast);
#define B_CURSOR_EAST &bCursorEast

static const uint8 cursorWest[] = {
  0x10, 0x01, 0x6, 0x2, 
  // Icon Data
  0x00, 0x00,
  0x60, 0x00,
  0x60, 0x00,
  0x60, 0x80,
  0x61, 0x00,
  0x62, 0x00,
  0x67, 0xE0,
  0x62, 0x00,
  0x61, 0x00,
  0x60, 0x80,
  0x60, 0x00,
  0x60, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0xF0, 0x00,
  0xF0, 0x00,
  0xF0, 0x80,
  0xF1, 0xC0,
  0xF3, 0x80,
  0xF7, 0xF0,
  0xFF, 0xF0,
  0xF7, 0xF0,
  0xF3, 0x80,
  0xF1, 0xC0,
  0xF0, 0x80,
  0xF0, 0x00,
  0xF0, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorWest(cursorWest);
#define B_CURSOR_WEST &bCursorWest

static const uint8 cursorCross[] = {
  0x10, 0x01, 0x7, 0x7, 
  // Icon Data
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0xFF, 0xFF,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x01, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x03, 0x80,
  0x00, 0x00
};
static const BCursor bCursorCross(cursorCross);
#define B_CURSOR_CROSS &bCursorCross

static const uint8 cursorGrab[] = {
  0x10, 0x01, 0x7, 0x5, 
  // Icon Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x0F, 0xE0,
  0x12, 0x5C,
  0x11, 0x2A,
  0x18, 0x01,
  0x24, 0x01,
  0x24, 0x01,
  0x22, 0x01,
  0x10, 0x01,
  0x0C, 0x01,
  0x02, 0x00,
  0x01, 0x00,
  // Bitmask Data
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x0F, 0xE0,
  0x1F, 0xFC,
  0x1F, 0xFE,
  0x1F, 0xFF,
  0x3F, 0xFF,
  0x3F, 0xFF,
  0x3F, 0xFF,
  0x1F, 0xFF,
  0x0F, 0xFF,
  0x03, 0xFE,
  0x01, 0xF8
};
static const BCursor bCursorGrab(cursorGrab);
#define B_CURSOR_GRAB &bCursorGrab

static const uint8 cursorMove[] = {
  0x10, 0x01, 0x0, 0x1, 
  // Icon Data
  0x30, 0x00,
  0x48, 0x00,
  0x48, 0x00,
  0x27, 0xC0,
  0x24, 0xB8,
  0x12, 0x54,
  0x10, 0x02,
  0x78, 0x02,
  0x98, 0x02,
  0x84, 0x02,
  0x60, 0x57,
  0x18, 0xAA,
  0x04, 0x41,
  0x02, 0x82,
  0x00, 0x55,
  0x00, 0xAA,
  // Bitmask Data
  0x30, 0x00,
  0x78, 0x00,
  0x78, 0x00,
  0x3F, 0xC0,
  0x3F, 0xF8,
  0x1F, 0xFC,
  0x1F, 0xFE,
  0x7F, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0x7F, 0xFF,
  0x1F, 0xFF,
  0x07, 0xFF,
  0x03, 0xFF,
  0x00, 0xFF,
  0x00, 0xFF
};
static const BCursor bCursorMove(cursorMove);
#define B_CURSOR_MOVE &bCursorMove

static const uint8 cursorCopy[] = {
  0x10, 0x01, 0x0, 0x1, 
  // Icon Data
  0x30, 0x00,
  0x48, 0x00,
  0x48, 0x00,
  0x27, 0xC0,
  0x24, 0xB8,
  0x12, 0x54,
  0x10, 0x02,
  0x78, 0x02,
  0x98, 0x02,
  0x84, 0x22,
  0x60, 0x22,
  0x18, 0xFA,
  0x04, 0x22,
  0x02, 0x22,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x30, 0x00,
  0x78, 0x00,
  0x78, 0x00,
  0x3F, 0xC0,
  0x3F, 0xF8,
  0x1F, 0xFC,
  0x1F, 0xFE,
  0x7F, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0x7F, 0xFE,
  0x1F, 0xFE,
  0x07, 0xFE,
  0x03, 0xFE,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorCopy(cursorCopy);
#define B_CURSOR_COPY &bCursorCopy

static const uint8 cursorAlias[] = {
  0x10, 0x01, 0x0, 0x1, 
  // Icon Data
  0x30, 0x00,
  0x48, 0x00,
  0x48, 0x00,
  0x27, 0xC0,
  0x24, 0xB8,
  0x12, 0x54,
  0x10, 0x02,
  0x78, 0x02,
  0x98, 0xF2,
  0x84, 0x72,
  0x60, 0xF2,
  0x18, 0x92,
  0x04, 0x40,
  0x02, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  // Bitmask Data
  0x30, 0x00,
  0x78, 0x00,
  0x78, 0x00,
  0x3F, 0xC0,
  0x3F, 0xF8,
  0x1F, 0xFC,
  0x1F, 0xFE,
  0x7F, 0xFE,
  0xFF, 0xFE,
  0xFF, 0xFE,
  0x7F, 0xFE,
  0x1F, 0xFE,
  0x07, 0xFE,
  0x03, 0xFE,
  0x00, 0x00,
  0x00, 0x00
};
static const BCursor bCursorAlias(cursorAlias);
#define B_CURSOR_ALIAS &bCursorAlias

#endif // nsBeOSCursors_h__

