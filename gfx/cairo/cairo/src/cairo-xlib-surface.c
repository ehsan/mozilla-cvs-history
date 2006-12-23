/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2002 University of Southern California
 * Copyright © 2005 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *	Carl D. Worth <cworth@cworth.org>
 *	Behdad Esfahbod <behdad@behdad.org>
 */

#include "cairoint.h"
#include "cairo-xlib.h"
#include "cairo-xlib-xrender.h"
#include "cairo-xlib-test.h"
#include "cairo-xlib-private.h"
#include "cairo-clip-private.h"
#include <X11/extensions/Xrender.h>
#include <X11/extensions/renderproto.h>

/* Xlib doesn't define a typedef, so define one ourselves */
typedef int (*cairo_xlib_error_func_t) (Display     *display,
					XErrorEvent *event);

typedef struct _cairo_xlib_surface cairo_xlib_surface_t;

static void
_cairo_xlib_surface_ensure_gc (cairo_xlib_surface_t *surface);

static void
_cairo_xlib_surface_ensure_src_picture (cairo_xlib_surface_t *surface);

static void
_cairo_xlib_surface_ensure_dst_picture (cairo_xlib_surface_t *surface);

static cairo_bool_t
_cairo_surface_is_xlib (cairo_surface_t *surface);

static cairo_bool_t
_native_byte_order_lsb (void);

static cairo_int_status_t
_cairo_xlib_surface_show_glyphs (void                *abstract_dst,
				 cairo_operator_t     op,
				 cairo_pattern_t     *src_pattern,
				 cairo_glyph_t       *glyphs,
				 int		      num_glyphs,
				 cairo_scaled_font_t *scaled_font);

/*
 * Instead of taking two round trips for each blending request,
 * assume that if a particular drawable fails GetImage that it will
 * fail for a "while"; use temporary pixmaps to avoid the errors
 */

#define CAIRO_ASSUME_PIXMAP	20

struct _cairo_xlib_surface {
    cairo_surface_t base;

    Display *dpy;
    cairo_xlib_screen_info_t *screen_info;

    GC gc;
    Drawable drawable;
    Screen *screen;
    cairo_bool_t owns_pixmap;
    Visual *visual;

    int use_pixmap;

    int render_major;
    int render_minor;

    /* TRUE if the server has a bug with repeating pictures
     *
     *  https://bugs.freedesktop.org/show_bug.cgi?id=3566
     *
     * We can't test for this because it depends on whether the
     * picture is in video memory or not.
     *
     * We also use this variable as a guard against a second
     * independent bug with transformed repeating pictures:
     *
     * http://lists.freedesktop.org/archives/cairo/2004-September/001839.html
     *
     * Both are fixed in xorg >= 6.9 and hopefully in > 6.8.2, so
     * we can reuse the test for now.
     */
    cairo_bool_t buggy_repeat;

    int width;
    int height;
    int depth;

    Picture dst_picture, src_picture;

    cairo_bool_t have_clip_rects;
    XRectangle *clip_rects;
    int num_clip_rects;

    XRenderPictFormat *xrender_format;
};

#define CAIRO_SURFACE_RENDER_AT_LEAST(surface, major, minor)	\
	(((surface)->render_major > major) ||			\
	 (((surface)->render_major == major) && ((surface)->render_minor >= minor)))

#define CAIRO_SURFACE_RENDER_HAS_CREATE_PICTURE(surface)		CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 0)
#define CAIRO_SURFACE_RENDER_HAS_COMPOSITE(surface)		CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 0)
#define CAIRO_SURFACE_RENDER_HAS_COMPOSITE_TEXT(surface)	CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 0)

#define CAIRO_SURFACE_RENDER_HAS_FILL_RECTANGLE(surface)		CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 1)
#define CAIRO_SURFACE_RENDER_HAS_FILL_RECTANGLES(surface)		CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 1)

#define CAIRO_SURFACE_RENDER_HAS_DISJOINT(surface)			CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 2)
#define CAIRO_SURFACE_RENDER_HAS_CONJOINT(surface)			CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 2)

#define CAIRO_SURFACE_RENDER_HAS_TRAPEZOIDS(surface)		CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 4)
#define CAIRO_SURFACE_RENDER_HAS_TRIANGLES(surface)		CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 4)
#define CAIRO_SURFACE_RENDER_HAS_TRISTRIP(surface)			CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 4)
#define CAIRO_SURFACE_RENDER_HAS_TRIFAN(surface)			CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 4)

#define CAIRO_SURFACE_RENDER_HAS_PICTURE_TRANSFORM(surface)	CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 6)
#define CAIRO_SURFACE_RENDER_HAS_FILTERS(surface)	CAIRO_SURFACE_RENDER_AT_LEAST((surface), 0, 6)

static cairo_bool_t cairo_xlib_render_disabled = FALSE;

/**
 * _cairo_xlib_test_disable_render:
 *
 * Disables the use of the RENDER extension.
 *
 * <note>
 * This function is <emphasis>only</emphasis> intended for internal
 * testing use within the cairo distribution. It is not installed in
 * any public header file.
 * </note>
 **/
void
_cairo_xlib_test_disable_render (void)
{
    cairo_xlib_render_disabled = TRUE;
}

static int
_CAIRO_FORMAT_DEPTH (cairo_format_t format)
{
    switch (format) {
    case CAIRO_FORMAT_A1:
	return 1;
    case CAIRO_FORMAT_A8:
	return 8;
    case CAIRO_FORMAT_RGB24:
	return 24;
    case CAIRO_FORMAT_ARGB32:
    default:
	return 32;
    }
}

static XRenderPictFormat *
_CAIRO_FORMAT_TO_XRENDER_FORMAT(Display *dpy, cairo_format_t format)
{
    int	pict_format;
    switch (format) {
    case CAIRO_FORMAT_A1:
	pict_format = PictStandardA1; break;
    case CAIRO_FORMAT_A8:
	pict_format = PictStandardA8; break;
    case CAIRO_FORMAT_RGB24:
	pict_format = PictStandardRGB24; break;
    case CAIRO_FORMAT_ARGB32:
    default:
	pict_format = PictStandardARGB32; break;
    }
    return XRenderFindStandardFormat (dpy, pict_format);
}

static cairo_surface_t *
_cairo_xlib_surface_create_similar_with_format (void	       *abstract_src,
						cairo_format_t	format,
						int		width,
						int		height)
{
    cairo_xlib_surface_t *src = abstract_src;
    Display *dpy = src->dpy;
    Pixmap pix;
    cairo_xlib_surface_t *surface;
    int depth = _CAIRO_FORMAT_DEPTH (format);
    XRenderPictFormat *xrender_format = _CAIRO_FORMAT_TO_XRENDER_FORMAT (dpy,
									 format);

    /* As a good first approximation, if the display doesn't have even
     * the most elementary RENDER operation, then we're better off
     * using image surfaces for all temporary operations, so return NULL
     * and let the fallback code happen.
     */
    if (!CAIRO_SURFACE_RENDER_HAS_COMPOSITE(src)) {
	return NULL;
    }

    pix = XCreatePixmap (dpy, RootWindowOfScreen (src->screen),
			 width <= 0 ? 1 : width, height <= 0 ? 1 : height,
			 depth);

    surface = (cairo_xlib_surface_t *)
	cairo_xlib_surface_create_with_xrender_format (dpy, pix, src->screen,
						       xrender_format,
						       width, height);
    if (surface->base.status != CAIRO_STATUS_SUCCESS) {
	_cairo_error (CAIRO_STATUS_NO_MEMORY);
	return (cairo_surface_t*) &_cairo_surface_nil;
    }

    surface->owns_pixmap = TRUE;

    return &surface->base;
}

static cairo_content_t
_xrender_format_to_content (XRenderPictFormat *xrender_format)
{
    cairo_bool_t xrender_format_has_alpha;
    cairo_bool_t xrender_format_has_color;

    /* This only happens when using a non-Render server. Let's punt
     * and say there's no alpha here. */
    if (xrender_format == NULL)
	return CAIRO_CONTENT_COLOR;

    xrender_format_has_alpha = (xrender_format->direct.alpha != 0);
    xrender_format_has_color = (xrender_format->direct.red   != 0 ||
				xrender_format->direct.green != 0 ||
				xrender_format->direct.blue  != 0);

    if (xrender_format_has_alpha)
	if (xrender_format_has_color)
	    return CAIRO_CONTENT_COLOR_ALPHA;
	else
	    return CAIRO_CONTENT_ALPHA;
    else
	return CAIRO_CONTENT_COLOR;
}

static cairo_surface_t *
_cairo_xlib_surface_create_similar (void	       *abstract_src,
				    cairo_content_t	content,
				    int			width,
				    int			height)
{
    cairo_xlib_surface_t *src = abstract_src;
    XRenderPictFormat *xrender_format = src->xrender_format;
    cairo_xlib_surface_t *surface;
    Pixmap pix;

    /* Start by examining the surface's XRenderFormat, or if it
     * doesn't have one, then look one up through its visual (in the
     * case of a bitmap, it won't even have that). */
    if (xrender_format == NULL && src->visual != NULL)
        xrender_format = XRenderFindVisualFormat (src->dpy, src->visual);

    /* If we never found an XRenderFormat or if it isn't compatible
     * with the content being requested, then we fallback to just
     * constructing a cairo_format_t instead, (which will fairly
     * arbitrarily pick a visual/depth for the similar surface.
     */
    if (xrender_format == NULL ||
	_xrender_format_to_content (xrender_format) != content)
    {
	return _cairo_xlib_surface_create_similar_with_format (abstract_src,
							       _cairo_format_from_content (content),
							       width, height);
    }

    /* We've got a compatible XRenderFormat now, which means the
     * similar surface will match the existing surface as closely in
     * visual/depth etc. as possible. */
    pix = XCreatePixmap (src->dpy, RootWindowOfScreen (src->screen),
			 width <= 0 ? 1 : width, height <= 0 ? 1 : height,
			 xrender_format->depth);

    surface = (cairo_xlib_surface_t *)
	cairo_xlib_surface_create_with_xrender_format (src->dpy, pix,
						       src->screen,
						       xrender_format,
						       width, height);
    if (surface->base.status != CAIRO_STATUS_SUCCESS) {
	_cairo_error (CAIRO_STATUS_NO_MEMORY);
	return (cairo_surface_t*) &_cairo_surface_nil;
    }

    surface->owns_pixmap = TRUE;

    return &surface->base;
}

static cairo_status_t
_cairo_xlib_surface_finish (void *abstract_surface)
{
    cairo_xlib_surface_t *surface = abstract_surface;
    if (surface->dst_picture != None)
	XRenderFreePicture (surface->dpy, surface->dst_picture);

    if (surface->src_picture != None)
	XRenderFreePicture (surface->dpy, surface->src_picture);

    if (surface->owns_pixmap)
	XFreePixmap (surface->dpy, surface->drawable);

    if (surface->gc != NULL)
	XFreeGC (surface->dpy, surface->gc);

    if (surface->clip_rects != NULL)
	free (surface->clip_rects);

    surface->dpy = NULL;

    return CAIRO_STATUS_SUCCESS;
}

static int
_noop_error_handler (Display     *display,
		     XErrorEvent *event)
{
    return False;		/* return value is ignored */
}

static cairo_bool_t
_CAIRO_MASK_FORMAT (cairo_format_masks_t *masks, cairo_format_t *format)
{
    switch (masks->bpp) {
    case 32:
	if (masks->alpha_mask == 0xff000000 &&
	    masks->red_mask == 0x00ff0000 &&
	    masks->green_mask == 0x0000ff00 &&
	    masks->blue_mask == 0x000000ff)
	{
	    *format = CAIRO_FORMAT_ARGB32;
	    return True;
	}
	if (masks->alpha_mask == 0x00000000 &&
	    masks->red_mask == 0x00ff0000 &&
	    masks->green_mask == 0x0000ff00 &&
	    masks->blue_mask == 0x000000ff)
	{
	    *format = CAIRO_FORMAT_RGB24;
	    return True;
	}
	break;
    case 8:
	if (masks->alpha_mask == 0xff)
	{
	    *format = CAIRO_FORMAT_A8;
	    return True;
	}
	break;
    case 1:
	if (masks->alpha_mask == 0x1)
	{
	    *format = CAIRO_FORMAT_A1;
	    return True;
	}
	break;
    }
    return False;
}

static void
_swap_ximage_2bytes (XImage *ximage)
{
    int i, j;
    char *line = ximage->data;

    for (j = ximage->height; j; j--) {
	uint16_t *p = (uint16_t *)line;
	for (i = ximage->width; i; i--) {
	    *p = (((*p & 0x00ff) << 8) |
		  ((*p)          >> 8));
	    p++;
	}

	line += ximage->bytes_per_line;
    }
}

static void
_swap_ximage_4bytes (XImage *ximage)
{
    int i, j;
    char *line = ximage->data;

    for (j = ximage->height; j; j--) {
	uint32_t *p = (uint32_t *)line;
	for (i = ximage->width; i; i--) {
	    *p = (((*p & 0x000000ff) << 24) |
		  ((*p & 0x0000ff00) << 8) |
		  ((*p & 0x00ff0000) >> 8) |
		  ((*p)              >> 24));
	    p++;
	}

	line += ximage->bytes_per_line;
    }
}

static void
_swap_ximage_bits (XImage *ximage)
{
    int i, j;
    char *line = ximage->data;
    int unit = ximage->bitmap_unit;
    int line_bytes = ((ximage->width + unit - 1) & ~(unit - 1)) / 8;

    for (j = ximage->height; j; j--) {
	char *p = line;

	for (i = line_bytes; i; i--) {
	    char b = *p;
	    b = ((b << 1) & 0xaa) | ((b >> 1) & 0x55);
	    b = ((b << 2) & 0xcc) | ((b >> 2) & 0x33);
	    b = ((b << 4) & 0xf0) | ((b >> 4) & 0x0f);
	    *p = b;

	    p++;
	}

	line += ximage->bytes_per_line;
    }
}

static void
_swap_ximage_to_native (XImage *ximage)
{
    int unit_bytes = 0;
    int native_byte_order = _native_byte_order_lsb () ? LSBFirst : MSBFirst;

    if (ximage->bits_per_pixel == 1 &&
	ximage->bitmap_bit_order != native_byte_order) {
	_swap_ximage_bits (ximage);
	if (ximage->bitmap_bit_order == ximage->byte_order)
	    return;
    }

    if (ximage->byte_order == native_byte_order)
	return;

    switch (ximage->bits_per_pixel) {
    case 1:
	unit_bytes = ximage->bitmap_unit / 8;
	break;
    case 8:
    case 16:
    case 32:
	unit_bytes = ximage->bits_per_pixel / 8;
	break;
    default:
        /* This could be hit on some uncommon but possible cases,
	 * such as bpp=4. These are cases that libpixman can't deal
	 * with in any case.
	 */
	ASSERT_NOT_REACHED;
    }

    switch (unit_bytes) {
    case 1:
	return;
    case 2:
	_swap_ximage_2bytes (ximage);
	break;
    case 4:
	_swap_ximage_4bytes (ximage);
	break;
    default:
	ASSERT_NOT_REACHED;
    }
}

static cairo_status_t
_get_image_surface (cairo_xlib_surface_t    *surface,
		    cairo_rectangle_int16_t *interest_rect,
		    cairo_image_surface_t  **image_out,
		    cairo_rectangle_int16_t *image_rect)
{
    cairo_image_surface_t *image;
    XImage *ximage;
    int x1, y1, x2, y2;
    cairo_format_masks_t masks;
    cairo_format_t format;

    x1 = 0;
    y1 = 0;
    x2 = surface->width;
    y2 = surface->height;

    if (interest_rect) {
	cairo_rectangle_int16_t rect;

	rect.x = interest_rect->x;
	rect.y = interest_rect->y;
	rect.width = interest_rect->width;
	rect.height = interest_rect->height;

	if (rect.x > x1)
	    x1 = rect.x;
	if (rect.y > y1)
	    y1 = rect.y;
	if (rect.x + rect.width < x2)
	    x2 = rect.x + rect.width;
	if (rect.y + rect.height < y2)
	    y2 = rect.y + rect.height;

	if (x1 >= x2 || y1 >= y2) {
	    *image_out = NULL;
	    return CAIRO_STATUS_SUCCESS;
	}
    }

    if (image_rect) {
	image_rect->x = x1;
	image_rect->y = y1;
	image_rect->width = x2 - x1;
	image_rect->height = y2 - y1;
    }

    /* XXX: This should try to use the XShm extension if available */

    if (surface->use_pixmap == 0)
    {
	cairo_xlib_error_func_t old_handler;

	old_handler = XSetErrorHandler (_noop_error_handler);

	ximage = XGetImage (surface->dpy,
			    surface->drawable,
			    x1, y1,
			    x2 - x1, y2 - y1,
			    AllPlanes, ZPixmap);

	XSetErrorHandler (old_handler);

	/* If we get an error, the surface must have been a window,
	 * so retry with the safe code path.
	 */
	if (!ximage)
	    surface->use_pixmap = CAIRO_ASSUME_PIXMAP;
    }
    else
    {
	surface->use_pixmap--;
	ximage = NULL;
    }

    if (!ximage)
    {

	/* XGetImage from a window is dangerous because it can
	 * produce errors if the window is unmapped or partially
	 * outside the screen. We could check for errors and
	 * retry, but to keep things simple, we just create a
	 * temporary pixmap
	 */
	Pixmap pixmap = XCreatePixmap (surface->dpy,
				       surface->drawable,
				       x2 - x1, y2 - y1,
				       surface->depth);
	_cairo_xlib_surface_ensure_gc (surface);

	XCopyArea (surface->dpy, surface->drawable, pixmap, surface->gc,
		   x1, y1, x2 - x1, y2 - y1, 0, 0);

	ximage = XGetImage (surface->dpy,
			    pixmap,
			    0, 0,
			    x2 - x1, y2 - y1,
			    AllPlanes, ZPixmap);

	XFreePixmap (surface->dpy, pixmap);
    }
    if (!ximage)
	return CAIRO_STATUS_NO_MEMORY;

    _swap_ximage_to_native (ximage);

    /*
     * Compute the pixel format masks from either a visual or a
     * XRenderFormat, failing we assume the drawable is an
     * alpha-only pixmap as it could only have been created
     * that way through the cairo_xlib_surface_create_for_bitmap
     * function.
     */
    if (surface->visual) {
	masks.bpp = ximage->bits_per_pixel;
	masks.alpha_mask = 0;
	masks.red_mask = surface->visual->red_mask;
	masks.green_mask = surface->visual->green_mask;
	masks.blue_mask = surface->visual->blue_mask;
    } else if (surface->xrender_format) {
	masks.bpp = ximage->bits_per_pixel;
	masks.red_mask = (unsigned long)surface->xrender_format->direct.redMask << surface->xrender_format->direct.red;
	masks.green_mask = (unsigned long)surface->xrender_format->direct.greenMask << surface->xrender_format->direct.green;
	masks.blue_mask = (unsigned long)surface->xrender_format->direct.blueMask << surface->xrender_format->direct.blue;
	masks.alpha_mask = (unsigned long)surface->xrender_format->direct.alphaMask << surface->xrender_format->direct.alpha;
    } else {
	masks.bpp = ximage->bits_per_pixel;
	masks.red_mask = 0;
	masks.green_mask = 0;
	masks.blue_mask = 0;
	if (surface->depth < 32)
	    masks.alpha_mask = (1 << surface->depth) - 1;
	else
	    masks.alpha_mask = 0xffffffff;
    }

    /*
     * Prefer to use a standard pixman format instead of the
     * general masks case.
     */
    if (_CAIRO_MASK_FORMAT (&masks, &format))
    {
	image = (cairo_image_surface_t*)
	    cairo_image_surface_create_for_data ((unsigned char *) ximage->data,
						 format,
						 ximage->width,
						 ximage->height,
						 ximage->bytes_per_line);
	if (image->base.status)
	    goto FAIL;
    }
    else
    {
	/*
	 * XXX This can't work.  We must convert the data to one of the
	 * supported pixman formats.  Pixman needs another function
	 * which takes data in an arbitrary format and converts it
	 * to something supported by that library.
	 */
	image = (cairo_image_surface_t*)
	    _cairo_image_surface_create_with_masks ((unsigned char *) ximage->data,
						    &masks,
						    ximage->width,
						    ximage->height,
						    ximage->bytes_per_line);
	if (image->base.status)
	    goto FAIL;
    }

    /* Let the surface take ownership of the data */
    _cairo_image_surface_assume_ownership_of_data (image);
    ximage->data = NULL;
    XDestroyImage (ximage);

    *image_out = image;
    return CAIRO_STATUS_SUCCESS;

 FAIL:
    XDestroyImage (ximage);
    return CAIRO_STATUS_NO_MEMORY;
}

static void
_cairo_xlib_surface_ensure_src_picture (cairo_xlib_surface_t    *surface)
{
    if (!surface->src_picture)
	surface->src_picture = XRenderCreatePicture (surface->dpy,
						     surface->drawable,
						     surface->xrender_format,
						     0, NULL);
}

static void
_cairo_xlib_surface_set_picture_clip_rects (cairo_xlib_surface_t *surface)
{
    if (surface->have_clip_rects)
	XRenderSetPictureClipRectangles (surface->dpy, surface->dst_picture,
					 0, 0,
					 surface->clip_rects,
					 surface->num_clip_rects);
}

static void
_cairo_xlib_surface_set_gc_clip_rects (cairo_xlib_surface_t *surface)
{
    if (surface->have_clip_rects)
	XSetClipRectangles(surface->dpy, surface->gc,
			   0, 0,
			   surface->clip_rects,
			   surface->num_clip_rects, YXSorted);
}

static void
_cairo_xlib_surface_ensure_dst_picture (cairo_xlib_surface_t    *surface)
{
    if (!surface->dst_picture) {
	surface->dst_picture = XRenderCreatePicture (surface->dpy,
						     surface->drawable,
						     surface->xrender_format,
						     0, NULL);
	_cairo_xlib_surface_set_picture_clip_rects (surface);
    }

}

static void
_cairo_xlib_surface_ensure_gc (cairo_xlib_surface_t *surface)
{
    XGCValues gcv;

    if (surface->gc)
	return;

    gcv.graphics_exposures = False;
    surface->gc = XCreateGC (surface->dpy, surface->drawable,
			     GCGraphicsExposures, &gcv);
    _cairo_xlib_surface_set_gc_clip_rects (surface);
}

static cairo_status_t
_draw_image_surface (cairo_xlib_surface_t   *surface,
		     cairo_image_surface_t  *image,
		     int                    src_x,
		     int                    src_y,
		     int                    width,
		     int                    height,
		     int                    dst_x,
		     int                    dst_y)
{
    XImage ximage;
    unsigned int bpp, alpha, red, green, blue;
    int native_byte_order = _native_byte_order_lsb () ? LSBFirst : MSBFirst;

    pixman_format_get_masks (pixman_image_get_format (image->pixman_image),
			     &bpp, &alpha, &red, &green, &blue);

    ximage.width = image->width;
    ximage.height = image->height;
    ximage.format = ZPixmap;
    ximage.data = (char *)image->data;
    ximage.byte_order = native_byte_order;
    ximage.bitmap_unit = 32;	/* always for libpixman */
    ximage.bitmap_bit_order = native_byte_order;
    ximage.bitmap_pad = 32;	/* always for libpixman */
    ximage.depth = image->depth;
    ximage.bytes_per_line = image->stride;
    ximage.bits_per_pixel = bpp;
    ximage.red_mask = red;
    ximage.green_mask = green;
    ximage.blue_mask = blue;
    ximage.xoffset = 0;

    XInitImage (&ximage);

    _cairo_xlib_surface_ensure_gc (surface);
    XPutImage(surface->dpy, surface->drawable, surface->gc,
	      &ximage, src_x, src_y, dst_x, dst_y,
	      width, height);

    return CAIRO_STATUS_SUCCESS;

}

static cairo_status_t
_cairo_xlib_surface_acquire_source_image (void                    *abstract_surface,
					  cairo_image_surface_t  **image_out,
					  void                   **image_extra)
{
    cairo_xlib_surface_t *surface = abstract_surface;
    cairo_image_surface_t *image;
    cairo_status_t status;

    status = _get_image_surface (surface, NULL, &image, NULL);
    if (status)
	return status;

    *image_out = image;
    *image_extra = NULL;

    return CAIRO_STATUS_SUCCESS;
}

static void
_cairo_xlib_surface_release_source_image (void                   *abstract_surface,
					  cairo_image_surface_t  *image,
					  void                   *image_extra)
{
    cairo_surface_destroy (&image->base);
}

static cairo_status_t
_cairo_xlib_surface_acquire_dest_image (void                    *abstract_surface,
					cairo_rectangle_int16_t *interest_rect,
					cairo_image_surface_t  **image_out,
					cairo_rectangle_int16_t *image_rect_out,
					void                   **image_extra)
{
    cairo_xlib_surface_t *surface = abstract_surface;
    cairo_image_surface_t *image;
    cairo_status_t status;

    status = _get_image_surface (surface, interest_rect, &image, image_rect_out);
    if (status)
	return status;

    *image_out = image;
    *image_extra = NULL;

    return CAIRO_STATUS_SUCCESS;
}

static void
_cairo_xlib_surface_release_dest_image (void                    *abstract_surface,
					cairo_rectangle_int16_t *interest_rect,
					cairo_image_surface_t   *image,
					cairo_rectangle_int16_t *image_rect,
					void                    *image_extra)
{
    cairo_xlib_surface_t *surface = abstract_surface;

    /* ignore errors */
    _draw_image_surface (surface, image, 0, 0, image->width, image->height,
			 image_rect->x, image_rect->y);

    cairo_surface_destroy (&image->base);
}

/*
 * Return whether two xlib surfaces share the same
 * screen.  Both core and Render drawing require this
 * when using multiple drawables in an operation.
 */
static cairo_bool_t
_cairo_xlib_surface_same_screen (cairo_xlib_surface_t *dst,
				 cairo_xlib_surface_t *src)
{
    return dst->dpy == src->dpy && dst->screen == src->screen;
}

static cairo_status_t
_cairo_xlib_surface_clone_similar (void			*abstract_surface,
				   cairo_surface_t	*src,
				   int                   src_x,
				   int                   src_y,
				   int                   width,
				   int                   height,
				   cairo_surface_t     **clone_out)
{
    cairo_xlib_surface_t *surface = abstract_surface;
    cairo_xlib_surface_t *clone;

    if (src->backend == surface->base.backend ) {
	cairo_xlib_surface_t *xlib_src = (cairo_xlib_surface_t *)src;

	if (_cairo_xlib_surface_same_screen (surface, xlib_src)) {
	    *clone_out = cairo_surface_reference (src);

	    return CAIRO_STATUS_SUCCESS;
	}
    } else if (_cairo_surface_is_image (src)) {
	cairo_image_surface_t *image_src = (cairo_image_surface_t *)src;

	if (! CAIRO_FORMAT_VALID (image_src->format))
	    return CAIRO_INT_STATUS_UNSUPPORTED;

	clone = (cairo_xlib_surface_t *)
	    _cairo_xlib_surface_create_similar_with_format (surface, image_src->format,
						image_src->width, image_src->height);
	if (clone->base.status)
	    return CAIRO_STATUS_NO_MEMORY;

	_draw_image_surface (clone, image_src, src_x, src_y,
			     width, height, src_x, src_y);

	*clone_out = &clone->base;

	return CAIRO_STATUS_SUCCESS;
    }

    return CAIRO_INT_STATUS_UNSUPPORTED;
}

static cairo_status_t
_cairo_xlib_surface_set_matrix (cairo_xlib_surface_t *surface,
				cairo_matrix_t	     *matrix)
{
    XTransform xtransform;

    if (!surface->src_picture)
	return CAIRO_STATUS_SUCCESS;

    xtransform.matrix[0][0] = _cairo_fixed_from_double (matrix->xx);
    xtransform.matrix[0][1] = _cairo_fixed_from_double (matrix->xy);
    xtransform.matrix[0][2] = _cairo_fixed_from_double (matrix->x0);

    xtransform.matrix[1][0] = _cairo_fixed_from_double (matrix->yx);
    xtransform.matrix[1][1] = _cairo_fixed_from_double (matrix->yy);
    xtransform.matrix[1][2] = _cairo_fixed_from_double (matrix->y0);

    xtransform.matrix[2][0] = 0;
    xtransform.matrix[2][1] = 0;
    xtransform.matrix[2][2] = _cairo_fixed_from_double (1);

    if (!CAIRO_SURFACE_RENDER_HAS_PICTURE_TRANSFORM (surface))
    {
	static const XTransform identity = { {
	    { 1 << 16, 0x00000, 0x00000 },
	    { 0x00000, 1 << 16, 0x00000 },
	    { 0x00000, 0x00000, 1 << 16 },
	} };

	if (memcmp (&xtransform, &identity, sizeof (XTransform)) == 0)
	    return CAIRO_STATUS_SUCCESS;

	return CAIRO_INT_STATUS_UNSUPPORTED;
    }

    XRenderSetPictureTransform (surface->dpy, surface->src_picture, &xtransform);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_xlib_surface_set_filter (cairo_xlib_surface_t *surface,
				cairo_filter_t	     filter)
{
    const char *render_filter;

    if (!surface->src_picture)
	return CAIRO_STATUS_SUCCESS;

    if (!CAIRO_SURFACE_RENDER_HAS_FILTERS (surface))
    {
	if (filter == CAIRO_FILTER_FAST || filter == CAIRO_FILTER_NEAREST)
	    return CAIRO_STATUS_SUCCESS;

	return CAIRO_INT_STATUS_UNSUPPORTED;
    }

    switch (filter) {
    case CAIRO_FILTER_FAST:
	render_filter = FilterFast;
	break;
    case CAIRO_FILTER_GOOD:
	render_filter = FilterGood;
	break;
    case CAIRO_FILTER_BEST:
	render_filter = FilterBest;
	break;
    case CAIRO_FILTER_NEAREST:
	render_filter = FilterNearest;
	break;
    case CAIRO_FILTER_BILINEAR:
	render_filter = FilterBilinear;
	break;
    case CAIRO_FILTER_GAUSSIAN:
	/* XXX: The GAUSSIAN value has no implementation in cairo
	 * whatsoever, so it was really a mistake to have it in the
	 * API. We could fix this by officially deprecating it, or
	 * else inventing semantics and providing an actual
	 * implementation for it. */
    default:
	render_filter = FilterBest;
	break;
    }

    XRenderSetPictureFilter (surface->dpy, surface->src_picture,
			     (char *) render_filter, NULL, 0);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_xlib_surface_set_repeat (cairo_xlib_surface_t *surface, int repeat)
{
    XRenderPictureAttributes pa;
    unsigned long	     mask;

    if (!surface->src_picture)
	return CAIRO_STATUS_SUCCESS;

    mask = CPRepeat;
    pa.repeat = repeat;

    XRenderChangePicture (surface->dpy, surface->src_picture, mask, &pa);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_int_status_t
_cairo_xlib_surface_set_attributes (cairo_xlib_surface_t	  *surface,
				       cairo_surface_attributes_t *attributes)
{
    cairo_int_status_t status;

    _cairo_xlib_surface_ensure_src_picture (surface);

    status = _cairo_xlib_surface_set_matrix (surface, &attributes->matrix);
    if (status)
	return status;

    switch (attributes->extend) {
    case CAIRO_EXTEND_NONE:
	_cairo_xlib_surface_set_repeat (surface, 0);
	break;
    case CAIRO_EXTEND_REPEAT:
	_cairo_xlib_surface_set_repeat (surface, 1);
	break;
    case CAIRO_EXTEND_REFLECT:
    case CAIRO_EXTEND_PAD:
	return CAIRO_INT_STATUS_UNSUPPORTED;
    }

    status = _cairo_xlib_surface_set_filter (surface, attributes->filter);
    if (status)
	return status;

    return CAIRO_STATUS_SUCCESS;
}

/* Checks whether we can can directly draw from src to dst with
 * the core protocol: either with CopyArea or using src as a
 * a tile in a GC.
 */
static cairo_bool_t
_surfaces_compatible (cairo_xlib_surface_t *dst,
		      cairo_xlib_surface_t *src)
{
    /* same screen */
    if (!_cairo_xlib_surface_same_screen (dst, src))
	return FALSE;

    /* same depth (for core) */
    if (src->depth != dst->depth)
	return FALSE;

    /* if Render is supported, match picture formats */
    if (src->xrender_format != NULL && src->xrender_format == dst->xrender_format)
	return TRUE;

    /* Without Render, match visuals instead */
    if (src->visual == dst->visual)
	return TRUE;

    return FALSE;
}

static cairo_bool_t
_surface_has_alpha (cairo_xlib_surface_t *surface)
{
    if (surface->xrender_format) {
	if (surface->xrender_format->type == PictTypeDirect &&
	    surface->xrender_format->direct.alphaMask != 0)
	    return TRUE;
	else
	    return FALSE;
    } else {

	/* In the no-render case, we never have alpha */
	return FALSE;
    }
}

/* Returns true if the given operator and source-alpha combination
 * requires alpha compositing to complete.
 */
static cairo_bool_t
_operator_needs_alpha_composite (cairo_operator_t op,
				 cairo_bool_t     surface_has_alpha)
{
    if (op == CAIRO_OPERATOR_SOURCE ||
	(!surface_has_alpha &&
	 (op == CAIRO_OPERATOR_OVER ||
	  op == CAIRO_OPERATOR_ATOP ||
	  op == CAIRO_OPERATOR_IN)))
	return FALSE;

    return TRUE;
}

/* There is a bug in most older X servers with compositing using a
 * untransformed repeating source pattern when the source is in off-screen
 * video memory, and another with repeated transformed images using a
 * general tranform matrix. When these bugs could be triggered, we need a
 * fallback: in the common case where we have no transformation and the
 * source and destination have the same format/visual, we can do the
 * operation using the core protocol for the first bug, otherwise, we need
 * a software fallback.
 *
 * We can also often optimize a compositing operation by calling XCopyArea
 * for some common cases where there is no alpha compositing to be done.
 * We figure that out here as well.
 */
typedef enum {
    DO_RENDER,		/* use render */
    DO_XCOPYAREA,	/* core protocol XCopyArea optimization/fallback */
    DO_XTILE,		/* core protocol XSetTile optimization/fallback */
    DO_UNSUPPORTED	/* software fallback */
} composite_operation_t;

/* Initial check for the render bugs; we need to recheck for the
 * offscreen-memory bug after we turn patterns into surfaces, since that
 * may introduce a repeating pattern for gradient patterns.  We don't need
 * to check for the repeat+transform bug because gradient surfaces aren't
 * transformed.
 *
 * All we do here is reject cases where we *know* are going to
 * hit the bug and won't be able to use a core protocol fallback.
 */
static composite_operation_t
_categorize_composite_operation (cairo_xlib_surface_t *dst,
				 cairo_operator_t      op,
				 cairo_pattern_t      *src_pattern,
				 cairo_bool_t	       have_mask)

{
    if (!dst->buggy_repeat)
	return DO_RENDER;

    if (src_pattern->type == CAIRO_PATTERN_TYPE_SURFACE)
    {
	cairo_surface_pattern_t *surface_pattern = (cairo_surface_pattern_t *)src_pattern;

	if (_cairo_matrix_is_integer_translation (&src_pattern->matrix, NULL, NULL) &&
	    src_pattern->extend == CAIRO_EXTEND_REPEAT)
	{
	    /* This is the case where we have the bug involving
	     * untransformed repeating source patterns with off-screen
	     * video memory; reject some cases where a core protocol
	     * fallback is impossible.
	     */
	    if (have_mask ||
		!(op == CAIRO_OPERATOR_SOURCE || op == CAIRO_OPERATOR_OVER))
		return DO_UNSUPPORTED;

	    if (_cairo_surface_is_xlib (surface_pattern->surface)) {
		cairo_xlib_surface_t *src = (cairo_xlib_surface_t *)surface_pattern->surface;

		if (op == CAIRO_OPERATOR_OVER && _surface_has_alpha (src))
		    return DO_UNSUPPORTED;

		/* If these are on the same screen but otherwise incompatible,
		 * make a copy as core drawing can't cross depths and doesn't
		 * work rightacross visuals of the same depth
		 */
		if (_cairo_xlib_surface_same_screen (dst, src) &&
		    !_surfaces_compatible (dst, src))
		    return DO_UNSUPPORTED;
	    }
	}

	/* Check for the other bug involving repeat patterns with general
	 * transforms. */
	if (!_cairo_matrix_is_integer_translation (&src_pattern->matrix, NULL, NULL) &&
	    src_pattern->extend == CAIRO_EXTEND_REPEAT)
	    return DO_UNSUPPORTED;
    }

    return DO_RENDER;
}

/* Recheck for composite-repeat once we've turned patterns into Xlib surfaces
 * If we end up returning DO_UNSUPPORTED here, we're throwing away work we
 * did to turn gradients into a pattern, but most of the time we can handle
 * that case with core protocol fallback.
 *
 * Also check here if we can just use XCopyArea, instead of going through
 * Render.
 */
static composite_operation_t
_recategorize_composite_operation (cairo_xlib_surface_t	      *dst,
				   cairo_operator_t	       op,
				   cairo_xlib_surface_t	      *src,
				   cairo_surface_attributes_t *src_attr,
				   cairo_bool_t		       have_mask)
{
    cairo_bool_t is_integer_translation =
	_cairo_matrix_is_integer_translation (&src_attr->matrix, NULL, NULL);
    cairo_bool_t needs_alpha_composite =
	_operator_needs_alpha_composite (op, _surface_has_alpha (src));

    if (!have_mask &&
	is_integer_translation &&
	src_attr->extend == CAIRO_EXTEND_NONE &&
	!needs_alpha_composite &&
	_surfaces_compatible(src, dst))
    {
	return DO_XCOPYAREA;
    }

    if (!dst->buggy_repeat)
	return DO_RENDER;

    if (is_integer_translation &&
	src_attr->extend == CAIRO_EXTEND_REPEAT &&
	(src->width != 1 || src->height != 1))
    {
	if (!have_mask &&
	    !needs_alpha_composite &&
	    _surfaces_compatible (dst, src))
	{
	    return DO_XTILE;
	}

	return DO_UNSUPPORTED;
    }

    return DO_RENDER;
}

static int
_render_operator (cairo_operator_t op)
{
    switch (op) {
    case CAIRO_OPERATOR_CLEAR:
	return PictOpClear;

    case CAIRO_OPERATOR_SOURCE:
	return PictOpSrc;
    case CAIRO_OPERATOR_OVER:
	return PictOpOver;
    case CAIRO_OPERATOR_IN:
	return PictOpIn;
    case CAIRO_OPERATOR_OUT:
	return PictOpOut;
    case CAIRO_OPERATOR_ATOP:
	return PictOpAtop;

    case CAIRO_OPERATOR_DEST:
	return PictOpDst;
    case CAIRO_OPERATOR_DEST_OVER:
	return PictOpOverReverse;
    case CAIRO_OPERATOR_DEST_IN:
	return PictOpInReverse;
    case CAIRO_OPERATOR_DEST_OUT:
	return PictOpOutReverse;
    case CAIRO_OPERATOR_DEST_ATOP:
	return PictOpAtopReverse;

    case CAIRO_OPERATOR_XOR:
	return PictOpXor;
    case CAIRO_OPERATOR_ADD:
	return PictOpAdd;
    case CAIRO_OPERATOR_SATURATE:
	return PictOpSaturate;
    default:
	return PictOpOver;
    }
}

static cairo_int_status_t
_cairo_xlib_surface_composite (cairo_operator_t		op,
			       cairo_pattern_t		*src_pattern,
			       cairo_pattern_t		*mask_pattern,
			       void			*abstract_dst,
			       int			src_x,
			       int			src_y,
			       int			mask_x,
			       int			mask_y,
			       int			dst_x,
			       int			dst_y,
			       unsigned int		width,
			       unsigned int		height)
{
    cairo_surface_attributes_t	src_attr, mask_attr;
    cairo_xlib_surface_t	*dst = abstract_dst;
    cairo_xlib_surface_t	*src;
    cairo_xlib_surface_t	*mask;
    cairo_int_status_t		status;
    composite_operation_t       operation;
    int				itx, ity;

    if (!CAIRO_SURFACE_RENDER_HAS_COMPOSITE (dst))
	return CAIRO_INT_STATUS_UNSUPPORTED;

    operation = _categorize_composite_operation (dst, op, src_pattern,
						 mask_pattern != NULL);
    if (operation == DO_UNSUPPORTED)
	return CAIRO_INT_STATUS_UNSUPPORTED;

    status = _cairo_pattern_acquire_surfaces (src_pattern, mask_pattern,
					      &dst->base,
					      src_x, src_y,
					      mask_x, mask_y,
					      width, height,
					      (cairo_surface_t **) &src,
					      (cairo_surface_t **) &mask,
					      &src_attr, &mask_attr);
    if (status)
	return status;

    operation = _recategorize_composite_operation (dst, op, src, &src_attr,
						   mask_pattern != NULL);
    if (operation == DO_UNSUPPORTED) {
	status = CAIRO_INT_STATUS_UNSUPPORTED;
	goto BAIL;
    }

    status = _cairo_xlib_surface_set_attributes (src, &src_attr);
    if (status)
	goto BAIL;

    switch (operation)
    {
    case DO_RENDER:
	_cairo_xlib_surface_ensure_dst_picture (dst);
	if (mask) {
	    status = _cairo_xlib_surface_set_attributes (mask, &mask_attr);
	    if (status)
		goto BAIL;

	    XRenderComposite (dst->dpy,
			      _render_operator (op),
			      src->src_picture,
			      mask->src_picture,
			      dst->dst_picture,
			      src_x + src_attr.x_offset,
			      src_y + src_attr.y_offset,
			      mask_x + mask_attr.x_offset,
			      mask_y + mask_attr.y_offset,
			      dst_x, dst_y,
			      width, height);
	} else {
	    XRenderComposite (dst->dpy,
			      _render_operator (op),
			      src->src_picture,
			      0,
			      dst->dst_picture,
			      src_x + src_attr.x_offset,
			      src_y + src_attr.y_offset,
			      0, 0,
			      dst_x, dst_y,
			      width, height);
	}

	break;

    case DO_XCOPYAREA:
	_cairo_xlib_surface_ensure_gc (dst);
	XCopyArea (dst->dpy,
		   src->drawable,
		   dst->drawable,
		   dst->gc,
		   src_x + src_attr.x_offset,
		   src_y + src_attr.y_offset,
		   width, height,
		   dst_x, dst_y);
	break;

    case DO_XTILE:
	/* This case is only used for bug fallbacks, though it is theoretically
	 * applicable to the case where we don't have the RENDER extension as
	 * well.
	 *
	 * We've checked that we have a repeating unscaled source in
	 * _recategorize_composite_operation.
	 */

	_cairo_xlib_surface_ensure_gc (dst);
	_cairo_matrix_is_integer_translation (&src_attr.matrix, &itx, &ity);

	XSetTSOrigin (dst->dpy, dst->gc,
		      - (itx + src_attr.x_offset), - (ity + src_attr.y_offset));
	XSetTile (dst->dpy, dst->gc, src->drawable);
	XSetFillStyle (dst->dpy, dst->gc, FillTiled);

	XFillRectangle (dst->dpy, dst->drawable, dst->gc,
			dst_x, dst_y, width, height);
	break;

    case DO_UNSUPPORTED:
    default:
	ASSERT_NOT_REACHED;
    }

    if (!_cairo_operator_bounded_by_source (op))
      status = _cairo_surface_composite_fixup_unbounded (&dst->base,
							 &src_attr, src->width, src->height,
							 mask ? &mask_attr : NULL,
							 mask ? mask->width : 0,
							 mask ? mask->height : 0,
							 src_x, src_y,
							 mask_x, mask_y,
							 dst_x, dst_y, width, height);

 BAIL:
    if (mask)
	_cairo_pattern_release_surface (mask_pattern, &mask->base, &mask_attr);

    _cairo_pattern_release_surface (src_pattern, &src->base, &src_attr);

    return status;
}

static cairo_int_status_t
_cairo_xlib_surface_fill_rectangles (void		     *abstract_surface,
				     cairo_operator_t	      op,
				     const cairo_color_t     *color,
				     cairo_rectangle_int16_t *rects,
				     int			      num_rects)
{
    cairo_xlib_surface_t *surface = abstract_surface;
    XRenderColor render_color;

    if (!CAIRO_SURFACE_RENDER_HAS_FILL_RECTANGLE (surface))
	return CAIRO_INT_STATUS_UNSUPPORTED;

    render_color.red   = color->red_short;
    render_color.green = color->green_short;
    render_color.blue  = color->blue_short;
    render_color.alpha = color->alpha_short;

    /* XXX: This XRectangle cast is evil... it needs to go away somehow. */
    _cairo_xlib_surface_ensure_dst_picture (surface);
    XRenderFillRectangles (surface->dpy,
			   _render_operator (op),
			   surface->dst_picture,
			   &render_color, (XRectangle *) rects, num_rects);

    return CAIRO_STATUS_SUCCESS;
}

/* Creates an A8 picture of size @width x @height, initialized with @color
 */
static Picture
_create_a8_picture (cairo_xlib_surface_t *surface,
		    XRenderColor         *color,
		    int                   width,
		    int                   height,
		    cairo_bool_t          repeat)
{
    XRenderPictureAttributes pa;
    unsigned long mask = 0;

    Pixmap pixmap = XCreatePixmap (surface->dpy, surface->drawable,
				   width <= 0 ? 1 : width,
				   height <= 0 ? 1 : height,
				   8);
    Picture picture;

    if (repeat) {
	pa.repeat = TRUE;
	mask = CPRepeat;
    }

    picture = XRenderCreatePicture (surface->dpy, pixmap,
				    XRenderFindStandardFormat (surface->dpy, PictStandardA8),
				    mask, &pa);
    XRenderFillRectangle (surface->dpy, PictOpSrc, picture, color,
			  0, 0, width, height);
    XFreePixmap (surface->dpy, pixmap);

    return picture;
}

/* Creates a temporary mask for the trapezoids covering the area
 * [@dst_x, @dst_y, @width, @height] of the destination surface.
 */
static Picture
_create_trapezoid_mask (cairo_xlib_surface_t *dst,
			cairo_trapezoid_t    *traps,
			int                   num_traps,
			int                   dst_x,
			int                   dst_y,
			int                   width,
			int                   height,
			XRenderPictFormat     *pict_format)
{
    XRenderColor transparent = { 0, 0, 0, 0 };
    XRenderColor solid = { 0xffff, 0xffff, 0xffff, 0xffff };
    Picture mask_picture, solid_picture;
    XTrapezoid *offset_traps;
    int i;

    /* This would be considerably simpler using XRenderAddTraps(), but since
     * we are only using this in the unbounded-operator case, we stick with
     * XRenderCompositeTrapezoids, which is available on older versions
     * of RENDER rather than conditionalizing. We should still hit an
     * optimization that avoids creating another intermediate surface on
     * the servers that have XRenderAddTraps().
     */
    mask_picture = _create_a8_picture (dst, &transparent, width, height, FALSE);
    solid_picture = _create_a8_picture (dst, &solid, width, height, TRUE);

    offset_traps = malloc (sizeof (XTrapezoid) * num_traps);
    if (!offset_traps)
	return None;

    for (i = 0; i < num_traps; i++) {
	offset_traps[i].top = traps[i].top - 0x10000 * dst_y;
	offset_traps[i].bottom = traps[i].bottom - 0x10000 * dst_y;
	offset_traps[i].left.p1.x = traps[i].left.p1.x - 0x10000 * dst_x;
	offset_traps[i].left.p1.y = traps[i].left.p1.y - 0x10000 * dst_y;
	offset_traps[i].left.p2.x = traps[i].left.p2.x - 0x10000 * dst_x;
	offset_traps[i].left.p2.y = traps[i].left.p2.y - 0x10000 * dst_y;
	offset_traps[i].right.p1.x = traps[i].right.p1.x - 0x10000 * dst_x;
	offset_traps[i].right.p1.y = traps[i].right.p1.y - 0x10000 * dst_y;
	offset_traps[i].right.p2.x = traps[i].right.p2.x - 0x10000 * dst_x;
	offset_traps[i].right.p2.y = traps[i].right.p2.y - 0x10000 * dst_y;
    }

    XRenderCompositeTrapezoids (dst->dpy, PictOpAdd,
				solid_picture, mask_picture,
				pict_format,
				0, 0,
				offset_traps, num_traps);

    XRenderFreePicture (dst->dpy, solid_picture);
    free (offset_traps);

    return mask_picture;
}

static cairo_int_status_t
_cairo_xlib_surface_composite_trapezoids (cairo_operator_t	op,
					  cairo_pattern_t	*pattern,
					  void			*abstract_dst,
					  cairo_antialias_t	antialias,
					  int			src_x,
					  int			src_y,
					  int			dst_x,
					  int			dst_y,
					  unsigned int		width,
					  unsigned int		height,
					  cairo_trapezoid_t	*traps,
					  int			num_traps)
{
    cairo_surface_attributes_t	attributes;
    cairo_xlib_surface_t	*dst = abstract_dst;
    cairo_xlib_surface_t	*src;
    cairo_int_status_t		status;
    composite_operation_t       operation;
    int				render_reference_x, render_reference_y;
    int				render_src_x, render_src_y;
    XRenderPictFormat		*pict_format;

    if (!CAIRO_SURFACE_RENDER_HAS_TRAPEZOIDS (dst))
	return CAIRO_INT_STATUS_UNSUPPORTED;

    operation = _categorize_composite_operation (dst, op, pattern, TRUE);
    if (operation == DO_UNSUPPORTED)
	return CAIRO_INT_STATUS_UNSUPPORTED;

    status = _cairo_pattern_acquire_surface (pattern, &dst->base,
					     src_x, src_y, width, height,
					     (cairo_surface_t **) &src,
					     &attributes);
    if (status)
	return status;

    operation = _recategorize_composite_operation (dst, op, src, &attributes, TRUE);
    if (operation == DO_UNSUPPORTED) {
	status = CAIRO_INT_STATUS_UNSUPPORTED;
	goto BAIL;
    }

    switch (antialias) {
    case CAIRO_ANTIALIAS_NONE:
	pict_format = XRenderFindStandardFormat (dst->dpy, PictStandardA1);
	break;
    case CAIRO_ANTIALIAS_GRAY:
    case CAIRO_ANTIALIAS_SUBPIXEL:
    case CAIRO_ANTIALIAS_DEFAULT:
    default:
	pict_format = XRenderFindStandardFormat (dst->dpy, PictStandardA8);
	break;
    }

    if (traps[0].left.p1.y < traps[0].left.p2.y) {
	render_reference_x = _cairo_fixed_integer_floor (traps[0].left.p1.x);
	render_reference_y = _cairo_fixed_integer_floor (traps[0].left.p1.y);
    } else {
	render_reference_x = _cairo_fixed_integer_floor (traps[0].left.p2.x);
	render_reference_y = _cairo_fixed_integer_floor (traps[0].left.p2.y);
    }

    render_src_x = src_x + render_reference_x - dst_x;
    render_src_y = src_y + render_reference_y - dst_y;

    _cairo_xlib_surface_ensure_dst_picture (dst);
    status = _cairo_xlib_surface_set_attributes (src, &attributes);
    if (status)
	goto BAIL;

    if (!_cairo_operator_bounded_by_mask (op)) {
	/* XRenderCompositeTrapezoids() creates a mask only large enough for the
	 * trapezoids themselves, but if the operator is unbounded, then we need
	 * to actually composite all the way out to the bounds, so we create
	 * the mask and composite ourselves. There actually would
	 * be benefit to doing this in all cases, since RENDER implementations
	 * will frequently create a too temporary big mask, ignoring destination
	 * bounds and clip. (XRenderAddTraps() could be used to make creating
	 * the mask somewhat cheaper.)
	 */
	Picture mask_picture = _create_trapezoid_mask (dst, traps, num_traps,
						       dst_x, dst_y, width, height,
						       pict_format);
	if (!mask_picture) {
	    status = CAIRO_STATUS_NO_MEMORY;
	    goto BAIL;
	}

	XRenderComposite (dst->dpy,
			  _render_operator (op),
			  src->src_picture,
			  mask_picture,
			  dst->dst_picture,
			  src_x + attributes.x_offset,
			  src_y + attributes.y_offset,
			  0, 0,
			  dst_x, dst_y,
			  width, height);

	XRenderFreePicture (dst->dpy, mask_picture);

	status = _cairo_surface_composite_shape_fixup_unbounded (&dst->base,
								 &attributes, src->width, src->height,
								 width, height,
								 src_x, src_y,
								 0, 0,
								 dst_x, dst_y, width, height);

    } else {
	/* XXX: The XTrapezoid cast is evil and needs to go away somehow. */
	XRenderCompositeTrapezoids (dst->dpy,
				    _render_operator (op),
				    src->src_picture, dst->dst_picture,
				    pict_format,
				    render_src_x + attributes.x_offset,
				    render_src_y + attributes.y_offset,
				    (XTrapezoid *) traps, num_traps);
    }

 BAIL:
    _cairo_pattern_release_surface (pattern, &src->base, &attributes);

    return status;
}

static cairo_int_status_t
_cairo_xlib_surface_set_clip_region (void              *abstract_surface,
				     pixman_region16_t *region)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (surface->clip_rects) {
	free (surface->clip_rects);
	surface->clip_rects = NULL;
    }

    surface->have_clip_rects = FALSE;
    surface->num_clip_rects = 0;

    if (region == NULL) {
	if (surface->gc)
	    XSetClipMask (surface->dpy, surface->gc, None);

	if (surface->xrender_format && surface->dst_picture) {
	    XRenderPictureAttributes pa;
	    pa.clip_mask = None;
	    XRenderChangePicture (surface->dpy, surface->dst_picture,
				  CPClipMask, &pa);
	}
    } else {
	pixman_box16_t *boxes;
	XRectangle *rects = NULL;
	int n_boxes, i;

	n_boxes = pixman_region_num_rects (region);
	if (n_boxes > 0) {
	    rects = malloc (sizeof(XRectangle) * n_boxes);
	    if (rects == NULL)
		return CAIRO_STATUS_NO_MEMORY;
	} else {
	    rects = NULL;
	}

	boxes = pixman_region_rects (region);

	for (i = 0; i < n_boxes; i++) {
	    rects[i].x = boxes[i].x1;
	    rects[i].y = boxes[i].y1;
	    rects[i].width = boxes[i].x2 - boxes[i].x1;
	    rects[i].height = boxes[i].y2 - boxes[i].y1;
	}

	surface->have_clip_rects = TRUE;
	surface->clip_rects = rects;
	surface->num_clip_rects = n_boxes;

	if (surface->gc)
	    _cairo_xlib_surface_set_gc_clip_rects (surface);

	if (surface->dst_picture)
	    _cairo_xlib_surface_set_picture_clip_rects (surface);
    }

    return CAIRO_STATUS_SUCCESS;
}

static cairo_int_status_t
_cairo_xlib_surface_get_extents (void		         *abstract_surface,
				 cairo_rectangle_int16_t *rectangle)
{
    cairo_xlib_surface_t *surface = abstract_surface;

    rectangle->x = 0;
    rectangle->y = 0;

    rectangle->width  = surface->width;
    rectangle->height = surface->height;

    return CAIRO_STATUS_SUCCESS;
}

static void
_cairo_xlib_surface_get_font_options (void                  *abstract_surface,
				      cairo_font_options_t  *options)
{
    cairo_xlib_surface_t *surface = abstract_surface;

    *options = surface->screen_info->font_options;
}

static void
_cairo_xlib_surface_scaled_font_fini (cairo_scaled_font_t *scaled_font);

static void
_cairo_xlib_surface_scaled_glyph_fini (cairo_scaled_glyph_t *scaled_glyph,
				       cairo_scaled_font_t  *scaled_font);

static const cairo_surface_backend_t cairo_xlib_surface_backend = {
    CAIRO_SURFACE_TYPE_XLIB,
    _cairo_xlib_surface_create_similar,
    _cairo_xlib_surface_finish,
    _cairo_xlib_surface_acquire_source_image,
    _cairo_xlib_surface_release_source_image,
    _cairo_xlib_surface_acquire_dest_image,
    _cairo_xlib_surface_release_dest_image,
    _cairo_xlib_surface_clone_similar,
    _cairo_xlib_surface_composite,
    _cairo_xlib_surface_fill_rectangles,
    _cairo_xlib_surface_composite_trapezoids,
    NULL, /* copy_page */
    NULL, /* show_page */
    _cairo_xlib_surface_set_clip_region,
    NULL, /* intersect_clip_path */
    _cairo_xlib_surface_get_extents,
    NULL, /* old_show_glyphs */
    _cairo_xlib_surface_get_font_options,
    NULL, /* flush */
    NULL, /* mark_dirty_rectangle */
    _cairo_xlib_surface_scaled_font_fini,
    _cairo_xlib_surface_scaled_glyph_fini,

    NULL, /* paint */
    NULL, /* mask */
    NULL, /* stroke */
    NULL, /* fill */
    _cairo_xlib_surface_show_glyphs,
    NULL  /* snapshot */
};

/**
 * _cairo_surface_is_xlib:
 * @surface: a #cairo_surface_t
 *
 * Checks if a surface is a #cairo_xlib_surface_t
 *
 * Return value: True if the surface is an xlib surface
 **/
static cairo_bool_t
_cairo_surface_is_xlib (cairo_surface_t *surface)
{
    return surface->backend == &cairo_xlib_surface_backend;
}

static cairo_surface_t *
_cairo_xlib_surface_create_internal (Display		       *dpy,
				     Drawable		        drawable,
				     Screen		       *screen,
				     Visual		       *visual,
				     XRenderPictFormat	       *xrender_format,
				     int			width,
				     int			height,
				     int			depth)
{
    cairo_xlib_surface_t *surface;
    cairo_xlib_screen_info_t *screen_info;

    screen_info = _cairo_xlib_screen_info_get (dpy, screen);
    if (screen_info == NULL) {
	_cairo_error (CAIRO_STATUS_NO_MEMORY);
	return (cairo_surface_t*) &_cairo_surface_nil;
    }

    surface = malloc (sizeof (cairo_xlib_surface_t));
    if (surface == NULL) {
	_cairo_error (CAIRO_STATUS_NO_MEMORY);
	return (cairo_surface_t*) &_cairo_surface_nil;
    }

    if (xrender_format) {
	depth = xrender_format->depth;
    } else if (visual) {
	int j, k;

	/* This is ugly, but we have to walk over all visuals
	 * for the display to find the depth.
	 */
	for (j = 0; j < screen->ndepths; j++) {
	    Depth *d = &screen->depths[j];
	    for (k = 0; k < d->nvisuals; k++) {
		if (&d->visuals[k] == visual) {
		    depth = d->depth;
		    goto found;
		}
	    }
	}
    found:
	;
    }

    if (cairo_xlib_render_disabled ||
	! XRenderQueryVersion (dpy, &surface->render_major, &surface->render_minor)) {
	surface->render_major = -1;
	surface->render_minor = -1;
    }

    if (CAIRO_SURFACE_RENDER_HAS_CREATE_PICTURE (surface)) {
	if (!xrender_format) {
	    if (visual)
		xrender_format = XRenderFindVisualFormat (dpy, visual);
	    else if (depth == 1)
		xrender_format = XRenderFindStandardFormat (dpy, PictStandardA1);
	}
    } else {
	xrender_format = NULL;
    }

    _cairo_surface_init (&surface->base, &cairo_xlib_surface_backend,
			 _xrender_format_to_content (xrender_format));

    surface->dpy = dpy;
    surface->screen_info = screen_info;

    surface->gc = NULL;
    surface->drawable = drawable;
    surface->screen = screen;
    surface->owns_pixmap = FALSE;
    surface->use_pixmap = 0;
    surface->width = width;
    surface->height = height;

    surface->buggy_repeat = FALSE;
    if (strstr (ServerVendor (dpy), "X.Org") != NULL) {
	if (VendorRelease (dpy) <= 60802000)
	    surface->buggy_repeat = TRUE;
    } else if (strstr (ServerVendor (dpy), "XFree86") != NULL) {
	if (VendorRelease (dpy) <= 40500000)
	    surface->buggy_repeat = TRUE;
    } else if (strstr (ServerVendor (dpy), "Sun Microsystems, Inc.") != NULL) {
	if (VendorRelease (dpy) <= 60900000)
	    surface->buggy_repeat = TRUE;
    }

    surface->dst_picture = None;
    surface->src_picture = None;

    surface->visual = visual;
    surface->xrender_format = xrender_format;
    surface->depth = depth;

    surface->have_clip_rects = FALSE;
    surface->clip_rects = NULL;
    surface->num_clip_rects = 0;

    return (cairo_surface_t *) surface;
}

static Screen *
_cairo_xlib_screen_from_visual (Display *dpy, Visual *visual)
{
    int	    s;
    int	    d;
    int	    v;
    Screen *screen;
    Depth  *depth;

    for (s = 0; s < ScreenCount (dpy); s++) {
	screen = ScreenOfDisplay (dpy, s);
	if (visual == DefaultVisualOfScreen (screen))
	    return screen;
	for (d = 0; d < screen->ndepths; d++) {
	    depth = &screen->depths[d];
	    for (v = 0; v < depth->nvisuals; v++)
		if (visual == &depth->visuals[v])
		    return screen;
	}
    }
    return NULL;
}

/**
 * cairo_xlib_surface_create:
 * @dpy: an X Display
 * @drawable: an X Drawable, (a Pixmap or a Window)
 * @visual: the visual to use for drawing to @drawable. The depth
 *          of the visual must match the depth of the drawable.
 *          Currently, only TrueColor visuals are fully supported.
 * @width: the current width of @drawable.
 * @height: the current height of @drawable.
 *
 * Creates an Xlib surface that draws to the given drawable.
 * The way that colors are represented in the drawable is specified
 * by the provided visual.
 *
 * NOTE: If @drawable is a Window, then the function
 * cairo_xlib_surface_set_size must be called whenever the size of the
 * window changes.
 *
 * Return value: the newly created surface
 **/
cairo_surface_t *
cairo_xlib_surface_create (Display     *dpy,
			   Drawable	drawable,
			   Visual      *visual,
			   int		width,
			   int		height)
{
    Screen *screen = _cairo_xlib_screen_from_visual (dpy, visual);

    if (screen == NULL) {
	_cairo_error (CAIRO_STATUS_INVALID_VISUAL);
	return (cairo_surface_t*) &_cairo_surface_nil;
    }

    return _cairo_xlib_surface_create_internal (dpy, drawable, screen,
						visual, NULL, width, height, 0);
}

/**
 * cairo_xlib_surface_create_for_bitmap:
 * @dpy: an X Display
 * @bitmap: an X Drawable, (a depth-1 Pixmap)
 * @screen: the X Screen associated with @bitmap
 * @width: the current width of @bitmap.
 * @height: the current height of @bitmap.
 *
 * Creates an Xlib surface that draws to the given bitmap.
 * This will be drawn to as a CAIRO_FORMAT_A1 object.
 *
 * Return value: the newly created surface
 **/
cairo_surface_t *
cairo_xlib_surface_create_for_bitmap (Display  *dpy,
				      Pixmap	bitmap,
				      Screen   *screen,
				      int	width,
				      int	height)
{
    return _cairo_xlib_surface_create_internal (dpy, bitmap, screen,
						NULL, NULL, width, height, 1);
}

/**
 * cairo_xlib_surface_create_with_xrender_format:
 * @dpy: an X Display
 * @drawable: an X Drawable, (a Pixmap or a Window)
 * @screen: the X Screen associated with @drawable
 * @format: the picture format to use for drawing to @drawable. The depth
 *          of @format must match the depth of the drawable.
 * @width: the current width of @drawable.
 * @height: the current height of @drawable.
 *
 * Creates an Xlib surface that draws to the given drawable.
 * The way that colors are represented in the drawable is specified
 * by the provided picture format.
 *
 * NOTE: If @drawable is a Window, then the function
 * cairo_xlib_surface_set_size must be called whenever the size of the
 * window changes.
 *
 * Return value: the newly created surface
 **/
cairo_surface_t *
cairo_xlib_surface_create_with_xrender_format (Display		    *dpy,
					       Drawable		    drawable,
					       Screen		    *screen,
					       XRenderPictFormat    *format,
					       int		    width,
					       int		    height)
{
    return _cairo_xlib_surface_create_internal (dpy, drawable, screen,
						NULL, format, width, height, 0);
}
slim_hidden_def (cairo_xlib_surface_create_with_xrender_format);

/**
 * cairo_xlib_surface_set_size:
 * @surface: a #cairo_surface_t for the XLib backend
 * @width: the new width of the surface
 * @height: the new height of the surface
 *
 * Informs cairo of the new size of the X Drawable underlying the
 * surface. For a surface created for a Window (rather than a Pixmap),
 * this function must be called each time the size of the window
 * changes. (For a subwindow, you are normally resizing the window
 * yourself, but for a toplevel window, it is necessary to listen for
 * ConfigureNotify events.)
 *
 * A Pixmap can never change size, so it is never necessary to call
 * this function on a surface created for a Pixmap.
 **/
void
cairo_xlib_surface_set_size (cairo_surface_t *abstract_surface,
			     int              width,
			     int              height)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_surface_set_error (abstract_surface,
				  CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return;
    }

    surface->width = width;
    surface->height = height;
}
/**
 * cairo_xlib_surface_set_drawable:
 * @surface: a #cairo_surface_t for the XLib backend
 * @drawable: the new drawable for the surface
 * @width: the width of the new drawable
 * @height: the height of the new drawable
 *
 * Informs cairo of a new X Drawable underlying the
 * surface. The drawable must match the display, screen
 * and format of the existing drawable or the application
 * will get X protocol errors and will probably terminate.
 * No checks are done by this function to ensure this
 * compatibility.
 **/
void
cairo_xlib_surface_set_drawable (cairo_surface_t   *abstract_surface,
				 Drawable	    drawable,
				 int		    width,
				 int		    height)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *)abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_surface_set_error (abstract_surface, CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return;
    }

    /* XXX: and what about this case? */
    if (surface->owns_pixmap)
	return;

    if (surface->drawable != drawable) {
	if (surface->dst_picture)
	    XRenderFreePicture (surface->dpy, surface->dst_picture);

	if (surface->src_picture)
	    XRenderFreePicture (surface->dpy, surface->src_picture);

	surface->dst_picture = None;
	surface->src_picture = None;

	surface->drawable = drawable;
    }
    surface->width = width;
    surface->height = height;
}

/**
 * cairo_xlib_surface_get_display:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the X Display for the underlying X Drawable.
 *
 * Return value: the display.
 *
 * Since: 1.2
 **/
Display *
cairo_xlib_surface_get_display (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return NULL;
    }

    return surface->dpy;
}

/**
 * cairo_xlib_surface_get_drawable:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the underlying X Drawable used for the surface.
 *
 * Return value: the drawable.
 *
 * Since: 1.2
 **/
Drawable
cairo_xlib_surface_get_drawable (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return 0;
    }

    return surface->drawable;
}

/**
 * cairo_xlib_surface_get_screen:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the X Screen for the underlying X Drawable.
 *
 * Return value: the screen.
 *
 * Since: 1.2
 **/
Screen *
cairo_xlib_surface_get_screen (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return NULL;
    }

    return surface->screen;
}

/**
 * cairo_xlib_surface_get_visual:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the X Visual used for underlying X Drawable.
 *
 * Return value: the visual.
 *
 * Since: 1.2
 **/
Visual *
cairo_xlib_surface_get_visual (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return NULL;
    }

    return surface->visual;
}

/**
 * cairo_xlib_surface_get_depth:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the number of bits used to represent each pixel value.
 *
 * Return value: the depth of the surface in bits.
 *
 * Since: 1.2
 **/
int
cairo_xlib_surface_get_depth (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return 0;
    }

    return surface->depth;
}

/**
 * cairo_xlib_surface_get_width:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the width of the X Drawable underlying the surface in pixels.
 *
 * Return value: the width of the surface in pixels.
 *
 * Since: 1.2
 **/
int
cairo_xlib_surface_get_width (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return -1;
    }

    return surface->width;
}

/**
 * cairo_xlib_surface_get_height:
 * @surface: a #cairo_xlib_surface_t
 *
 * Get the height of the X Drawable underlying the surface in pixels.
 *
 * Return value: the height of the surface in pixels.
 *
 * Since: 1.2
 **/
int
cairo_xlib_surface_get_height (cairo_surface_t *abstract_surface)
{
    cairo_xlib_surface_t *surface = (cairo_xlib_surface_t *) abstract_surface;

    if (! _cairo_surface_is_xlib (abstract_surface)) {
	_cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);
	return -1;
    }

    return surface->height;
}

typedef struct _cairo_xlib_surface_font_private {
    Display		*dpy;
    GlyphSet		glyphset;
    cairo_format_t	format;
    XRenderPictFormat	*xrender_format;
} cairo_xlib_surface_font_private_t;

static cairo_status_t
_cairo_xlib_surface_font_init (Display		    *dpy,
			       cairo_scaled_font_t  *scaled_font,
			       cairo_format_t	     format)
{
    cairo_xlib_surface_font_private_t	*font_private;

    font_private = malloc (sizeof (cairo_xlib_surface_font_private_t));
    if (!font_private)
	return CAIRO_STATUS_NO_MEMORY;

    font_private->dpy = dpy;
    font_private->format = format;
    font_private->xrender_format = _CAIRO_FORMAT_TO_XRENDER_FORMAT(dpy, format);
    font_private->glyphset = XRenderCreateGlyphSet (dpy, font_private->xrender_format);
    scaled_font->surface_private = font_private;
    scaled_font->surface_backend = &cairo_xlib_surface_backend;
    return CAIRO_STATUS_SUCCESS;
}

static void
_cairo_xlib_surface_scaled_font_fini (cairo_scaled_font_t *scaled_font)
{
    cairo_xlib_surface_font_private_t	*font_private = scaled_font->surface_private;

    if (font_private) {
	XRenderFreeGlyphSet (font_private->dpy, font_private->glyphset);
	free (font_private);
    }
}

static void
_cairo_xlib_surface_scaled_glyph_fini (cairo_scaled_glyph_t *scaled_glyph,
				       cairo_scaled_font_t  *scaled_font)
{
    cairo_xlib_surface_font_private_t	*font_private = scaled_font->surface_private;

    if (font_private != NULL && scaled_glyph->surface_private != NULL) {
	unsigned long	glyph_index = _cairo_scaled_glyph_index(scaled_glyph);
	XRenderFreeGlyphs (font_private->dpy,
			   font_private->glyphset,
			   &glyph_index, 1);
    }
}

static cairo_bool_t
_native_byte_order_lsb (void)
{
    int	x = 1;

    return *((char *) &x) == 1;
}

static cairo_status_t
_cairo_xlib_surface_add_glyph (Display *dpy,
			       cairo_scaled_font_t  *scaled_font,
			       cairo_scaled_glyph_t *scaled_glyph)
{
    XGlyphInfo glyph_info;
    unsigned long glyph_index;
    unsigned char *data;
    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    cairo_xlib_surface_font_private_t *font_private;
    cairo_image_surface_t *glyph_surface = scaled_glyph->surface;

    if (scaled_font->surface_private == NULL) {
	status = _cairo_xlib_surface_font_init (dpy, scaled_font,
						glyph_surface->format);
	if (status)
	    return status;
    }
    font_private = scaled_font->surface_private;

    /* If the glyph format does not match the font format, then we
     * create a temporary surface for the glyph image with the font's
     * format.
     */
    if (glyph_surface->format != font_private->format) {
	cairo_t *cr;
	cairo_surface_t *tmp_surface;
	double x_offset, y_offset;

	tmp_surface = cairo_image_surface_create (font_private->format,
						  glyph_surface->width,
						  glyph_surface->height);
	cr = cairo_create (tmp_surface);
	cairo_surface_get_device_offset (&glyph_surface->base, &x_offset, &y_offset);
	cairo_set_source_surface (cr, &glyph_surface->base, x_offset, y_offset);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);

	status = cairo_status (cr);

	cairo_destroy (cr);

	tmp_surface->device_transform = glyph_surface->base.device_transform;
	tmp_surface->device_transform_inverse = glyph_surface->base.device_transform_inverse;

	glyph_surface = (cairo_image_surface_t *) tmp_surface;

	if (status)
	    goto BAIL;
    }

    /*
     *  Most of the font rendering system thinks of glyph tiles as having
     *  an origin at (0,0) and an x and y bounding box "offset" which
     *  extends possibly off into negative coordinates, like so:
     *
     *
     *       (x,y) <-- probably negative numbers
     *         +----------------+
     *         |      .         |
     *         |      .         |
     *         |......(0,0)     |
     *         |                |
     *         |                |
     *         +----------------+
     *                  (width+x,height+y)
     *
     *  This is a postscript-y model, where each glyph has its own
     *  coordinate space, so it's what we expose in terms of metrics. It's
     *  apparantly what everyone's expecting. Everyone except the Render
     *  extension. Render wants to see a glyph tile starting at (0,0), with
     *  an origin offset inside, like this:
     *
     *       (0,0)
     *         +---------------+
     *         |      .        |
     *         |      .        |
     *         |......(x,y)    |
     *         |               |
     *         |               |
     *         +---------------+
     *                   (width,height)
     *
     *  Luckily, this is just the negation of the numbers we already have
     *  sitting around for x and y.
     */

    /* XXX: FRAGILE: We're ignore device_transform scaling here. A bug? */
    glyph_info.x = - _cairo_lround (glyph_surface->base.device_transform.x0);
    glyph_info.y = - _cairo_lround (glyph_surface->base.device_transform.y0);
    glyph_info.width = glyph_surface->width;
    glyph_info.height = glyph_surface->height;
    glyph_info.xOff = scaled_glyph->x_advance;
    glyph_info.yOff = scaled_glyph->y_advance;

    data = glyph_surface->data;

    /* flip formats around */
    switch (scaled_glyph->surface->format) {
    case CAIRO_FORMAT_A1:
	/* local bitmaps are always stored with bit == byte */
	if (_native_byte_order_lsb() != (BitmapBitOrder (dpy) == LSBFirst)) {
	    int		    c = glyph_surface->stride * glyph_surface->height;
	    unsigned char   *d;
	    unsigned char   *new, *n;

	    new = malloc (c);
	    if (!new) {
		status = CAIRO_STATUS_NO_MEMORY;
		goto BAIL;
	    }
	    n = new;
	    d = data;
	    while (c--)
	    {
		char	b = *d++;
		b = ((b << 1) & 0xaa) | ((b >> 1) & 0x55);
		b = ((b << 2) & 0xcc) | ((b >> 2) & 0x33);
		b = ((b << 4) & 0xf0) | ((b >> 4) & 0x0f);
		*n++ = b;
	    }
	    data = new;
	}
	break;
    case CAIRO_FORMAT_A8:
	break;
    case CAIRO_FORMAT_ARGB32:
	if (_native_byte_order_lsb() != (ImageByteOrder (dpy) == LSBFirst)) {
	    unsigned int    c = glyph_surface->stride * glyph_surface->height;
	    unsigned char   *d;
	    unsigned char   *new, *n;

	    new = malloc (c);
	    if (new == NULL) {
		status = CAIRO_STATUS_NO_MEMORY;
		goto BAIL;
	    }
	    n = new;
	    d = data;
	    while (c >= 4)
	    {
		n[3] = d[0];
		n[2] = d[1];
		n[1] = d[2];
		n[0] = d[3];
		d += 4;
		n += 4;
		c -= 4;
	    }
	    data = new;
	}
	break;
    case CAIRO_FORMAT_RGB24:
    default:
	ASSERT_NOT_REACHED;
	break;
    }
    /* XXX assume X server wants pixman padding. Xft assumes this as well */

    glyph_index = _cairo_scaled_glyph_index (scaled_glyph);

    XRenderAddGlyphs (dpy, font_private->glyphset,
		      &glyph_index, &(glyph_info), 1,
		      (char *) data,
		      glyph_surface->stride * glyph_surface->height);

    if (data != glyph_surface->data)
	free (data);

 BAIL:
    if (glyph_surface != scaled_glyph->surface)
	cairo_surface_destroy (&glyph_surface->base);

    return status;
}

typedef void (*cairo_xrender_composite_text_func_t)
	      (Display                      *dpy,
	       int                          op,
	       Picture                      src,
	       Picture                      dst,
	       _Xconst XRenderPictFormat    *maskFormat,
	       int                          xSrc,
	       int                          ySrc,
	       int                          xDst,
	       int                          yDst,
	       _Xconst XGlyphElt8           *elts,
	       int                          nelt);

/* Build a struct of the same size of cairo_glyph_t that can be used both as
 * an input glyph with double coordinates, and as "working" glyph with
 * integer from-current-point offsets. */
typedef struct {
  unsigned long index;
  union {
    struct {
      double x;
      double y;
    } d;
    struct {
      int x;
      int y;
    } i;
  } p;
} cairo_xlib_glyph_t;

#define GLYPH_INDEX_SKIP ((unsigned long) -1)
#define STACK_ELTS_LEN ((int) (CAIRO_STACK_BUFFER_SIZE / sizeof (XGlyphElt8)))

static cairo_status_t
_cairo_xlib_surface_emit_glyphs_chunk (cairo_xlib_surface_t *dst,
				       cairo_xlib_glyph_t *glyphs,
				       int num_glyphs,
				       int width,
				       int num_elts,
				       cairo_scaled_font_t *scaled_font,
				       cairo_operator_t op,
				       cairo_xlib_surface_t *src,
				       cairo_surface_attributes_t *attributes)
{
    /* Which XRenderCompositeText function to use */
    cairo_xrender_composite_text_func_t composite_text_func;
    int size;

    /* Element buffer stuff */
    XGlyphElt8 *elts;
    XGlyphElt8 stack_elts[STACK_ELTS_LEN];

    /* Reuse the input glyph array for output char generation */
    char *char8 = (char *) glyphs;
    unsigned short *char16 = (unsigned short *) glyphs;
    unsigned int *char32 = (unsigned int *) glyphs;

    int i;
    int nelt; /* Element index */
    int n; /* Num output glyphs in current element */
    int j; /* Num output glyphs so far */

    cairo_xlib_surface_font_private_t *font_private = scaled_font->surface_private;

    switch (width) {
    case 1:
	/* don't cast the 8-variant, to catch possible mismatches */
	composite_text_func = XRenderCompositeText8;
	size = sizeof (char);
	break;
    case 2:
	composite_text_func = (cairo_xrender_composite_text_func_t) XRenderCompositeText16;
	size = sizeof (unsigned short);
	break;
    default:
    case 4:
	composite_text_func = (cairo_xrender_composite_text_func_t) XRenderCompositeText32;
	size = sizeof (unsigned int);
    }

    /* Allocate element array */
    if (num_elts <= STACK_ELTS_LEN) {
      elts = stack_elts;
    } else {
      elts = malloc (num_elts * sizeof (XGlyphElt8));
      if (elts == NULL)
	  return CAIRO_STATUS_NO_MEMORY;
    }

    /* Fill them in */
    nelt = 0;
    n = 0;
    j = 0;
    for (i = 0; i < num_glyphs; i++) {

      /* Skip glyphs marked so */
      if (glyphs[i].index == GLYPH_INDEX_SKIP)
	continue;

      /* Start a new element for first output glyph, and for glyphs with
       * unexpected position */
      if (!j || glyphs[i].p.i.x || glyphs[i].p.i.y) {
	  if (j) {
	    elts[nelt].nchars = n;
	    nelt++;
	    n = 0;
	  }
	  elts[nelt].chars = char8 + size * j;
	  elts[nelt].glyphset = font_private->glyphset;
	  elts[nelt].xOff = glyphs[i].p.i.x;
	  elts[nelt].yOff = glyphs[i].p.i.y;
      }

      switch (width) {
      case 1: char8 [j] = (char)           glyphs[i].index; break;
      case 2: char16[j] = (unsigned short) glyphs[i].index; break;
      default:
      case 4: char32[j] = (unsigned int)   glyphs[i].index; break;
      }

      n++;
      j++;
    }

    if (n) {
	elts[nelt].nchars = n;
	nelt++;
	n = 0;
    }

    composite_text_func (dst->dpy,
			 _render_operator (op),
			 src->src_picture,
			 dst->dst_picture,
			 font_private->xrender_format,
			 attributes->x_offset + elts[0].xOff,
			 attributes->y_offset + elts[0].yOff,
			 elts[0].xOff, elts[0].yOff,
			 (XGlyphElt8 *) elts, nelt);

    if (elts != stack_elts)
      free (elts);

    return CAIRO_STATUS_SUCCESS;
}

#undef STACK_ELTS_LEN

static cairo_status_t
_cairo_xlib_surface_emit_glyphs (cairo_xlib_surface_t *dst,
				 cairo_xlib_glyph_t *glyphs,
				 int num_glyphs,
				 cairo_scaled_font_t *scaled_font,
				 cairo_operator_t op,
				 cairo_xlib_surface_t *src,
				 cairo_surface_attributes_t *attributes)
{
    int i;
    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    cairo_scaled_glyph_t *scaled_glyph;
    cairo_fixed_t x = 0, y = 0;

    unsigned long max_index = 0;
    int width = 1;
    int num_elts = 0;
    int num_out_glyphs = 0;

    int max_request_size = XMaxRequestSize (dst->dpy)
			 - MAX (sz_xRenderCompositeGlyphs8Req,
				MAX(sz_xRenderCompositeGlyphs16Req,
				    sz_xRenderCompositeGlyphs32Req));
    int request_size = 0;

    _cairo_xlib_surface_ensure_dst_picture (dst);

    for (i = 0; i < num_glyphs; i++) {
	int this_x, this_y;
	int old_width;

	status = _cairo_scaled_glyph_lookup (scaled_font,
					     glyphs[i].index,
					     CAIRO_SCALED_GLYPH_INFO_SURFACE |
					     CAIRO_SCALED_GLYPH_INFO_METRICS,
					     &scaled_glyph);
	if (status != CAIRO_STATUS_SUCCESS)
	    return status;

	this_x = _cairo_lround (glyphs[i].p.d.x);
	this_y = _cairo_lround (glyphs[i].p.d.y);

	/* Glyph skipping:
	 *
	 * We skip any initial size-zero glyphs to avoid an X server bug (present
	 * in at least Xorg 7.1 without EXA) which stops rendering glyphs after
	 * the first zero-size glyph.  However, we don't skip all size-zero
	 * glyphs, since that will force a new element at every space.  We
	 * skip initial size-zero glyphs and hope that it's enough.  Since
	 * Xft never exposed that bug, this assumptation should be correct.
	 *
	 * We also skip any glyph that hav troublesome coordinates.  We want
	 * to make sure that (glyph2.x - (glyph1.x + glyph1.width)) fits in
	 * a signed 16bit integer, otherwise it will overflow in the render
	 * protocol.
	 * To ensure this, we'll make sure that (glyph2.x - glyph1.x) fits in
	 * a signed 15bit integer.  The trivial option would be to allow
	 * coordinates -8192..8192, but that's kinda dull.  It probably will
	 * take a decade or so to get monitors 8192x4096 or something.  A
	 * negative value of -8192 on the other hand, is absolutely useless.
	 * Note that we do want to allow some negative positions.  The glyph
	 * may start off the screen but part of it make it to the screen.
	 * Anyway, we will allow positions in the range -1024..15359.  That
	 * will buy us a few more years before this stops working.
	 */
	if ((!num_out_glyphs && !(scaled_glyph->surface->width && scaled_glyph->surface->height)) ||
	    (((this_x+1024)|(this_y+1024))&~0x3fffu)) {
	    glyphs[i].index = GLYPH_INDEX_SKIP;
	    continue;
	}

	old_width = width;

	/* Update max glyph index */
	if (glyphs[i].index > max_index) {
	    max_index = glyphs[i].index;
	    if (max_index >= 65536)
	      width = 4;
	    else if (max_index >= 256)
	      width = 2;
	    if (width != old_width)
	      request_size += (width - old_width) * num_out_glyphs;
	}

	/* If we will pass the max request size by adding this glyph,
	 * flush current glyphs.  Note that we account for a
	 * possible element being added below. */
	if (request_size + width > max_request_size - sz_xGlyphElt) {
	    status = _cairo_xlib_surface_emit_glyphs_chunk (dst, glyphs, i,
							    old_width, num_elts,
							    scaled_font, op, src, attributes);
	    if (status != CAIRO_STATUS_SUCCESS)
		return status;

	    glyphs += i;
	    num_glyphs -= i;
	    i = 0;
	    max_index = glyphs[i].index;
	    width = max_index < 256 ? 1 : max_index < 65536 ? 2 : 4;
	    request_size = 0;
	    num_elts = 0;
	    num_out_glyphs = 0;
	    x = y = 0;

	}

	/* Convert absolute glyph position to relative-to-current-point
	 * position */
	glyphs[i].p.i.x = this_x - x;
	glyphs[i].p.i.y = this_y - y;

	/* Start a new element for the first glyph, or for any glyph that
	 * has unexpected position */
	if (!num_out_glyphs || glyphs[i].p.i.x || glyphs[i].p.i.y) {
	    num_elts++;
	    request_size += sz_xGlyphElt;
	}

	/* Send unsent glyphs to the server */
	if (scaled_glyph->surface_private == NULL) {
	    _cairo_xlib_surface_add_glyph (dst->dpy, scaled_font, scaled_glyph);
	    scaled_glyph->surface_private = (void *) 1;
	}

	/* adjust current-position */
	x = this_x + scaled_glyph->x_advance;
	y = this_y + scaled_glyph->y_advance;

	num_out_glyphs++;
	request_size += width;
    }

    if (num_elts)
	status = _cairo_xlib_surface_emit_glyphs_chunk (dst, glyphs, num_glyphs,
							width, num_elts,
							scaled_font, op, src, attributes);

    return status;
}

#undef GLYPH_INDEX_SKIP

static cairo_int_status_t
_cairo_xlib_surface_show_glyphs (void                *abstract_dst,
				 cairo_operator_t     op,
				 cairo_pattern_t     *src_pattern,
				 cairo_glyph_t       *glyphs,
				 int		      num_glyphs,
				 cairo_scaled_font_t *scaled_font)
{
    cairo_int_status_t status = CAIRO_STATUS_SUCCESS;
    cairo_xlib_surface_t *dst = (cairo_xlib_surface_t*) abstract_dst;

    composite_operation_t operation;
    cairo_surface_attributes_t attributes;
    cairo_xlib_surface_t *src = NULL;

    cairo_xlib_surface_font_private_t *font_private;

    cairo_pattern_union_t solid_pattern;

    if (!CAIRO_SURFACE_RENDER_HAS_COMPOSITE_TEXT (dst) || !dst->xrender_format)
	return CAIRO_INT_STATUS_UNSUPPORTED;

    /* Just let unbounded operators go through the fallback code
     * instead of trying to do the fixups here */
    if (!_cairo_operator_bounded_by_mask (op))
        return CAIRO_INT_STATUS_UNSUPPORTED;

    /* Render <= 0.10 seems to have a bug with PictOpSrc and glyphs --
     * the solid source seems to be multiplied by the glyph mask, and
     * then the entire thing is copied to the destination surface,
     * including the fully transparent "background" of the rectangular
     * glyph surface. */
    if (op == CAIRO_OPERATOR_SOURCE &&
        !CAIRO_SURFACE_RENDER_AT_LEAST(dst, 0, 11))
        return CAIRO_INT_STATUS_UNSUPPORTED;

    /* We can only use our code if we either have no clip or
     * have a real native clip region set.  If we're using
     * fallback clip masking, we have to go through the full
     * fallback path.
     */
    if (dst->base.clip &&
        (dst->base.clip->mode != CAIRO_CLIP_MODE_REGION ||
         dst->base.clip->surface != NULL))
        return CAIRO_INT_STATUS_UNSUPPORTED;

    operation = _categorize_composite_operation (dst, op, src_pattern, TRUE);
    if (operation == DO_UNSUPPORTED)
	return CAIRO_INT_STATUS_UNSUPPORTED;

    font_private = scaled_font->surface_private;
    if ((scaled_font->surface_backend != NULL &&
	 scaled_font->surface_backend != &cairo_xlib_surface_backend) ||
	(font_private != NULL && font_private->dpy != dst->dpy))
	return CAIRO_INT_STATUS_UNSUPPORTED;

    /* After passing all those tests, we're now committed to rendering
     * these glyphs or to fail trying. We first upload any glyphs to
     * the X server that it doesn't have already, then we draw
     * them. We tie into the scaled_font's glyph cache and remove
     * glyphs from the X server when they are ejected from the
     * scaled_font cache. Because of this we first freeze the
     * scaled_font's cache so that we don't cause any of our glyphs to
     * be ejected and removed from the X server before we have a
     * chance to render them. */
    _cairo_scaled_font_freeze_cache (scaled_font);

    /* PictOpClear doesn't seem to work with CompositeText; it seems to ignore
     * the mask (the glyphs).  This code below was executed as a side effect
     * of going through the _clip_and_composite fallback code for old_show_glyphs,
     * so PictOpClear was never used with CompositeText before.
     */
    if (op == CAIRO_OPERATOR_CLEAR) {
	_cairo_pattern_init_solid (&solid_pattern.solid, CAIRO_COLOR_WHITE);
	src_pattern = &solid_pattern.base;
	op = CAIRO_OPERATOR_DEST_OUT;
    }

    if (src_pattern->type == CAIRO_PATTERN_TYPE_SOLID) {
        status = _cairo_pattern_acquire_surface (src_pattern, &dst->base,
                                                 0, 0, 1, 1,
                                                 (cairo_surface_t **) &src,
                                                 &attributes);
    } else {
        cairo_rectangle_int16_t glyph_extents;

        status = _cairo_scaled_font_glyph_device_extents (scaled_font,
                                                          glyphs,
                                                          num_glyphs,
                                                          &glyph_extents);
        if (status)
	    goto BAIL;

        status = _cairo_pattern_acquire_surface (src_pattern, &dst->base,
                                                 glyph_extents.x, glyph_extents.y,
                                                 glyph_extents.width, glyph_extents.height,
                                                 (cairo_surface_t **) &src,
                                                 &attributes);
    }

    if (status)
        goto BAIL;

    operation = _recategorize_composite_operation (dst, op, src, &attributes, TRUE);
    if (operation == DO_UNSUPPORTED) {
	status = CAIRO_INT_STATUS_UNSUPPORTED;
	goto BAIL;
    }

    status = _cairo_xlib_surface_set_attributes (src, &attributes);
    if (status)
        goto BAIL;

    _cairo_xlib_surface_emit_glyphs (dst, (cairo_xlib_glyph_t *) glyphs, num_glyphs,
				     scaled_font, op, src, &attributes);

  BAIL:
    _cairo_scaled_font_thaw_cache (scaled_font);

    if (src)
        _cairo_pattern_release_surface (src_pattern, &src->base, &attributes);
    if (src_pattern == &solid_pattern.base)
	_cairo_pattern_fini (&solid_pattern.base);

    return status;
}
