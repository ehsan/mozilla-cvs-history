/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@pavlov.net>
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

#ifndef POINT_H
#define POINT_H

struct gfxSize {
    double width, height;

    gfxSize() {}
    gfxSize(const gfxSize& s) : width(s.width), height(s.height) {}
    gfxSize(double _width, double _height) : width(_width), height(_height) {}

    void SizeTo(double _width, double _height) {width = _width; height = _height;}

    int operator==(const gfxSize& s) const {
        return ((width == s.width) && (height == s.height));
    }
    int operator!=(const gfxSize& s) const {
        return ((width != s.width) || (height != s.height));
    }
};

struct gfxPoint {
    double x, y;

    gfxPoint() { }
    gfxPoint(const gfxPoint& p) : x(p.x), y(p.y) {}
    gfxPoint(double _x, double _y) : x(_x), y(_y) {}

    void MoveTo(double aX, double aY) { x = aX; y = aY; }

    int operator==(const gfxPoint& p) const {
        return ((x == p.x) && (y == p.y));
    }
    int operator!=(const gfxPoint& p) const {
        return ((x != p.x) || (y != p.y));
    }
};

#endif /* POINT_H */
