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
 *   Vladimir Vukicevic <vladimir@pobox.com>
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

#include "gfxPlatformGtk.h"

#include "nsIAtom.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "gfxImageSurface.h"
#include "gfxXlibSurface.h"

#ifdef MOZ_ENABLE_GLITZ
#include "gfxGlitzSurface.h"
#include "glitz-glx.h"
#endif

#ifndef THEBES_USE_PANGO_CAIRO
#include <fontconfig/fontconfig.h>
#endif

static cairo_user_data_key_t cairo_gdk_window_key;
static cairo_user_data_key_t cairo_gdk_pixmap_key;
static void do_gdk_pixmap_unref (void *data)
{
    GdkPixmap *pmap = (GdkPixmap*)data;
    gdk_pixmap_unref (pmap);
}

gfxPlatformGtk::gfxPlatformGtk()
{
#ifdef MOZ_ENABLE_GLITZ
    if (UseGlitz())
        glitz_glx_init(NULL);
#endif
}

gfxASurface*
gfxPlatformGtk::CreateOffscreenSurface(PRUint32 width,
                                       PRUint32 height,
                                       gfxASurface::gfxImageFormat imageFormat)
{
    gfxASurface *newSurface = nsnull;

    int bpp, glitzf;
    int bestbpp = gdk_visual_get_best_depth();
    switch (imageFormat) {
        case gfxASurface::ImageFormatARGB32:
            bpp = 32;
            glitzf = 0; // GLITZ_STANDARD_ARGB32;
            break;
        case gfxASurface::ImageFormatRGB24:
            bpp = 24;
            glitzf = 1; // GLITZ_STANDARD_RGB24;
            break;
        case gfxASurface::ImageFormatA8:
            bpp = 8;
            glitzf = 2; // GLITZ_STANDARD_A8;
        case gfxASurface::ImageFormatA1:
            bpp = 1;
            glitzf = 3; // GLITZ_STANDARD_A1;
            break;
        default:
            return nsnull;
    }

    if (bestbpp < bpp)
        bpp = bestbpp;

    if (!UseGlitz()) {
        GdkPixmap *pixmap = ::gdk_pixmap_new(nsnull, width, height, bpp);
        gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap), gdk_rgb_get_colormap());

        newSurface = new gfxXlibSurface(GDK_WINDOW_XDISPLAY(GDK_DRAWABLE(pixmap)),
                                        GDK_WINDOW_XWINDOW(GDK_DRAWABLE(pixmap)),
                                        GDK_VISUAL_XVISUAL(gdk_drawable_get_visual(GDK_DRAWABLE(pixmap))),
                                        width, height);

        // set up the surface to auto-unref the gdk pixmap when the surface
        // is released
        cairo_surface_set_user_data(newSurface->CairoSurface(),
                                    &cairo_gdk_pixmap_key,
                                    pixmap,
                                    do_gdk_pixmap_unref);
    } else {
#ifdef MOZ_ENABLE_GLITZ
        glitz_drawable_format_t *gdformat = glitz_glx_find_pbuffer_format
            (GDK_DISPLAY(),
             gdk_x11_get_default_screen(),
             0, NULL, 0);

        glitz_drawable_t *gdraw =
            glitz_glx_create_pbuffer_drawable(GDK_DISPLAY(),
                                              DefaultScreen(GDK_DISPLAY()),
                                              gdformat,
                                              width,
                                              height);
        glitz_format_t *gformat =
            glitz_find_standard_format(gdraw, (glitz_format_name_t)glitzf);

        glitz_surface_t *gsurf =
            glitz_surface_create(gdraw,
                                 gformat,
                                 width,
                                 height,
                                 0,
                                 NULL);

        glitz_surface_attach(gsurf, gdraw, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
        newSurface = new gfxGlitzSurface(gdraw, gsurf, PR_TRUE);
#endif
    }

    return newSurface;
}

GdkDrawable*
gfxPlatformGtk::GetSurfaceGdkDrawable(gfxASurface *aSurf)
{
    GdkDrawable *gd;
    gd = (GdkDrawable*) cairo_surface_get_user_data(aSurf->CairoSurface(), &cairo_gdk_pixmap_key);
    if (gd)
        return gd;

    gd = (GdkDrawable*) cairo_surface_get_user_data(aSurf->CairoSurface(), &cairo_gdk_window_key);
    if (gd)
        return gd;

    return nsnull;
}

void
gfxPlatformGtk::SetSurfaceGdkWindow(gfxASurface *aSurf,
                                    GdkWindow *win)
{
    cairo_surface_set_user_data(aSurf->CairoSurface(),
                                &cairo_gdk_window_key,
                                win,
                                nsnull);
}

// this is in nsFontConfigUtils.h
extern void NS_AddLangGroup (FcPattern *aPattern, nsIAtom *aLangGroup);

nsresult
gfxPlatformGtk::GetFontList(const nsACString& aLangGroup,
                            const nsACString& aGenericFamily,
                            nsStringArray& aListOfFonts)
{
#ifndef THEBES_USE_PANGO_CAIRO
    FcPattern *pat = NULL;
    FcObjectSet *os = NULL;
    FcFontSet *fs = NULL;
    nsresult rv = NS_ERROR_FAILURE;

    aListOfFonts.Clear();

    PRInt32 serif = 0, sansSerif = 0, monospace = 0, nGenerics;

    pat = FcPatternCreate();
    if (!pat)
        goto end;

    os = FcObjectSetBuild(FC_FAMILY, FC_FOUNDRY, 0);
    if (!os)
        goto end;

    // take the pattern and add the lang group to it
    if (!aLangGroup.IsEmpty()) {
        nsCOMPtr<nsIAtom> langAtom = do_GetAtom(aLangGroup);
        //XXX fix me //NS_AddLangGroup(pat, langAtom);
    }

    fs = FcFontList(0, pat, os);
    if (!fs)
        goto end;

    if (fs->nfont == 0) {
        rv = NS_OK;
        goto end;
    }

    // Fontconfig supports 3 generic fonts, "serif", "sans-serif", and
    // "monospace", slightly different from CSS's 5.
    if (aGenericFamily.IsEmpty())
        serif = sansSerif = monospace = 1;
    else if (aGenericFamily.EqualsLiteral("serif"))
        serif = 1;
    else if (aGenericFamily.EqualsLiteral("sans-serif"))
        sansSerif = 1;
    else if (aGenericFamily.EqualsLiteral("monospace"))
        monospace = 1;
    else if (aGenericFamily.EqualsLiteral("cursive") || aGenericFamily.EqualsLiteral("fantasy"))
        serif = sansSerif = 1;
    else
        NS_NOTREACHED("unexpected CSS generic font family");
    nGenerics = serif + sansSerif + monospace;

    if (serif)
        aListOfFonts.AppendString(NS_LITERAL_STRING("serif"));
    if (sansSerif)
        aListOfFonts.AppendString(NS_LITERAL_STRING("sans-serif"));
    if (monospace)
        aListOfFonts.AppendString(NS_LITERAL_STRING("monospace"));

    for (int i = 0; i < fs->nfont; i++) {
        char *family;

        // if there's no family name, skip this match
        if (FcPatternGetString (fs->fonts[i], FC_FAMILY, 0,
                                (FcChar8 **) &family) != FcResultMatch)
        {
            continue;
        }

        aListOfFonts.AppendString(NS_ConvertASCIItoUTF16(nsDependentCString(family)));
    }

    aListOfFonts.Sort();
    rv = NS_OK;

  end:
    if (NS_FAILED(rv))
        aListOfFonts.Clear();

    if (pat)
        FcPatternDestroy(pat);
    if (os)
        FcObjectSetDestroy(os);
    if (fs)
        FcFontSetDestroy(fs);

    return rv;
#else
    // pango_cairo case; needs to be written
    return NS_ERROR_NOT_IMPLEMENTED;
#endif
}
