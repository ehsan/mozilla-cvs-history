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
 * The Original Code is Mozilla Foundation code.
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <stuart@mozilla.com>
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

#ifndef GFX_WINDOWSFONTS_H
#define GFX_WINDOWSFONTS_H

#include "prtypes.h"
#include "gfxTypes.h"
#include "gfxFont.h"

#include <cairo-win32.h>

class gfxWindowsFont : public gfxFont {

public:
    gfxWindowsFont(const nsAString &aName, const gfxFontGroup *aFontGroup, HDC aHWnd);
    virtual ~gfxWindowsFont();

    virtual const gfxFont::Metrics& GetMetrics() { return mMetrics; }

    cairo_font_face_t *CairoFontFace() { return mFontFace; }
    cairo_scaled_font_t *CairoScaledFont() { return mScaledFont; }
    void UpdateFonts(cairo_t *cr);

protected:
    cairo_font_face_t *MakeCairoFontFace();
    cairo_scaled_font_t *MakeCairoScaledFont(cairo_t *cr);
    void FillLogFont();

private:
    void ComputeMetrics();

    LOGFONTW mLogFont;
    cairo_font_face_t *mFontFace;
    cairo_scaled_font_t *mScaledFont;
    HDC mDC;

    gfxFont::Metrics mMetrics;
};



class NS_EXPORT gfxWindowsFontGroup : public gfxFontGroup {

public:
    gfxWindowsFontGroup(const nsAString& aFamilies, const gfxFontStyle* aStyle, HDC hwnd);
    virtual ~gfxWindowsFontGroup();


    virtual gfxTextRun *MakeTextRun(const nsAString& aString);

protected:
    static PRBool MakeFont(const nsAString& fontName, const nsAString& genericName, void *closure);

private:
    HDC mDC;

    friend class gfxWindowsTextRun;
};


class NS_EXPORT gfxWindowsTextRun : public gfxTextRun {
    THEBES_DECL_ISUPPORTS_INHERITED

public:
    gfxWindowsTextRun(const nsAString& aString, gfxWindowsFontGroup *aFontGroup);
    ~gfxWindowsTextRun();

    virtual void DrawString(gfxContext *aContext, gfxPoint pt);
    virtual gfxFloat MeasureString(gfxContext *aContext);

private:
    PRInt32 MeasureOrDrawUniscribe(gfxContext *aContext,
                                   const PRUnichar *aString, PRUint32 aLength,
                                   PRBool aDraw, PRInt32 aX, PRInt32 aY, const PRInt32 *aSpacing);


    nsString mString;
    gfxWindowsFontGroup *mGroup;
};

#endif /* GFX_WINDOWSFONTS_H */
