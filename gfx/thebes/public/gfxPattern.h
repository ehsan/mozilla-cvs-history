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

#ifndef PATTERN_H
#define PATTERN_H

#include <cairo.h>

#include "gfxColor.h"
#include "gfxASurface.h"
#include "gfxMatrix.h"

class gfxPattern {
public:
    // from another surface
    gfxPattern(gfxASurface& surface) {
        mPattern = cairo_pattern_create_for_surface(surface.CairoSurface());
    }
    // linear
    gfxPattern(double x0, double y0, double x1, double y1) {
        mPattern = cairo_pattern_create_linear(x0, y0, x1, y1);
    }
    // radial
    gfxPattern(double cx0, double cy0, double radius0,
               double cx1, double cy1, double radius1) {
        mPattern = cairo_pattern_create_radial(cx0, cy0, radius0,
                                               cx1, cy1, radius1);
    }
    ~gfxPattern() {
        cairo_pattern_destroy(mPattern);
    }

    cairo_pattern_t *CairoPattern() {
        return mPattern;
    }

    void AddColorStop(double offset, const gfxRGBA& c) {
        cairo_pattern_add_color_stop(mPattern, offset, c.r, c.g, c.b, c.a);
    }

    void SetMatrix(const gfxMatrix& matrix) {
        cairo_matrix_t *t = cairo_matrix_create();
        matrix.FillInCairoMatrix(t);
        cairo_pattern_set_matrix(mPattern, t); // this does a copy
        cairo_matrix_destroy(t);
    }
    gfxMatrix CurrentMatrix() const {
        gfxMatrix matrix;
        cairo_matrix_t *t = cairo_matrix_create();
        cairo_pattern_get_matrix(mPattern, t);
        matrix = t;
        cairo_matrix_destroy(t);
        return matrix;
    }

    // none, repeat, reflect
    void SetExtend(int extend) {
        cairo_pattern_set_extend(mPattern, (cairo_extend_t)extend);
    }
    int Extend() const {
        return (int)cairo_pattern_get_extend(mPattern);
    }

    void SetFilter(int filter) {
        cairo_pattern_set_filter(mPattern, (cairo_filter_t)filter);
    }
    int Filter() const {
        return (int)cairo_pattern_get_filter(mPattern);
    }

private:
    gfxPattern(cairo_pattern_t *pattern) : mPattern(pattern) {}
    cairo_pattern_t *mPattern;
};

#endif /* PATTERN_H */
