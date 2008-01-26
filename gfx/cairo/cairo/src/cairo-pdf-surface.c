/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2004 Red Hat, Inc
 * Copyright © 2006 Red Hat, Inc
 * Copyright © 2007, 2008 Adrian Johnson
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
 *	Kristian Høgsberg <krh@redhat.com>
 *	Carl Worth <cworth@cworth.org>
 *	Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairoint.h"
#include "cairo-pdf.h"
#include "cairo-pdf-surface-private.h"
#include "cairo-pdf-operators-private.h"
#include "cairo-scaled-font-subsets-private.h"
#include "cairo-paginated-private.h"
#include "cairo-output-stream-private.h"
#include "cairo-meta-surface-private.h"

#include <time.h>
#include <zlib.h>

/* Issues:
 *
 * - We embed an image in the stream each time it's composited.  We
 *   could add generation counters to surfaces and remember the stream
 *   ID for a particular generation for a particular surface.
 *
 * - Images of other formats than 8 bit RGBA.
 *
 * - Backend specific meta data.
 *
 * - Surface patterns.
 *
 * - Should/does cairo support drawing into a scratch surface and then
 *   using that as a fill pattern?  For this backend, that would involve
 *   using a tiling pattern (4.6.2).  How do you create such a scratch
 *   surface?  cairo_surface_create_similar() ?
 *
 * - What if you create a similar surface and does show_page and then
 *   does show_surface on another surface?
 *
 * - Add test case for RGBA images.
 *
 * - Add test case for RGBA gradients.
 *
 * - Coordinate space for create_similar() args?
 */

/*
 * Page Structure of the Generated PDF:
 *
 * Each page requiring fallbacks images contains a knockout group at
 * the top level. The first operation of the knockout group paints a
 * group containing all the supported drawing operations. Fallback
 * images (if any) are painted in the knockout group. This ensures
 * that fallback images do not composite with any content under the
 * fallback images.
 *
 * Streams:
 *
 * This PDF surface has three types of streams:
 *  - PDF Stream
 *  - Content Stream
 *  - Group Stream
 *
 * Calling _cairo_output_stream_printf (surface->output, ...) will
 * write to the currently open stream.
 *
 * PDF Stream:
 *   A PDF Stream may be opened and closed with the following functions:
 *     _cairo_pdf_surface_open stream ()
 *     _cairo_pdf_surface_close_stream ()
 *
 *   PDF Streams are written directly to the PDF file. They are used for
 *   fonts, images and patterns.
 *
 * Content Stream:
 *   The Content Stream is opened and closed with the following functions:
 *     _cairo_pdf_surface_open_content_stream ()
 *     _cairo_pdf_surface_close_content_stream ()
 *
 *   The Content Stream contains the text and graphics operators.
 *
 * Group Stream:
 *   A Group Stream may be opened and closed with the following functions:
 *     _cairo_pdf_surface_open_group ()
 *     _cairo_pdf_surface_close_group ()
 *
 *   A Group Stream is a Form XObject. It is used for short sequences
 *   of operators. As the content is very short the group is stored in
 *   memory until it is closed. This allows some optimization such as
 *   including the Resource dictionary and stream length inside the
 *   XObject instead of using an indirect object.
 */

typedef struct _cairo_pdf_object {
    long offset;
} cairo_pdf_object_t;

typedef struct _cairo_pdf_font {
    unsigned int font_id;
    unsigned int subset_id;
    cairo_pdf_resource_t subset_resource;
} cairo_pdf_font_t;

typedef struct _cairo_pdf_rgb_linear_function {
    cairo_pdf_resource_t resource;
    double               color1[3];
    double               color2[3];
} cairo_pdf_rgb_linear_function_t;

typedef struct _cairo_pdf_alpha_linear_function {
    cairo_pdf_resource_t resource;
    double               alpha1;
    double               alpha2;
} cairo_pdf_alpha_linear_function_t;

static cairo_pdf_resource_t
_cairo_pdf_surface_new_object (cairo_pdf_surface_t *surface);

static void
_cairo_pdf_surface_clear (cairo_pdf_surface_t *surface);

static void
_cairo_pdf_smask_group_destroy (cairo_pdf_smask_group_t *group);

static cairo_status_t
_cairo_pdf_surface_add_font (unsigned int        font_id,
			     unsigned int        subset_id,
			     void		*closure);

static void
_cairo_pdf_group_resources_init (cairo_pdf_group_resources_t *res);

static cairo_status_t
_cairo_pdf_surface_open_stream (cairo_pdf_surface_t	*surface,
				cairo_pdf_resource_t    *resource,
                                cairo_bool_t             compressed,
				const char		*fmt,
				...) CAIRO_PRINTF_FORMAT(4, 5);
static cairo_status_t
_cairo_pdf_surface_close_stream (cairo_pdf_surface_t	*surface);

static cairo_status_t
_cairo_pdf_surface_write_page (cairo_pdf_surface_t *surface);

static void
_cairo_pdf_surface_write_pages (cairo_pdf_surface_t *surface);

static cairo_pdf_resource_t
_cairo_pdf_surface_write_info (cairo_pdf_surface_t *surface);

static cairo_pdf_resource_t
_cairo_pdf_surface_write_catalog (cairo_pdf_surface_t *surface);

static long
_cairo_pdf_surface_write_xref (cairo_pdf_surface_t *surface);

static cairo_status_t
_cairo_pdf_surface_write_page (cairo_pdf_surface_t *surface);

static cairo_status_t
_cairo_pdf_surface_emit_font_subsets (cairo_pdf_surface_t *surface);

static const cairo_surface_backend_t cairo_pdf_surface_backend;
static const cairo_paginated_surface_backend_t cairo_pdf_surface_paginated_backend;

static cairo_pdf_resource_t
_cairo_pdf_surface_new_object (cairo_pdf_surface_t *surface)
{
    cairo_pdf_resource_t resource;
    cairo_status_t status;
    cairo_pdf_object_t object;

    object.offset = _cairo_output_stream_get_position (surface->output);

    status = _cairo_array_append (&surface->objects, &object);
    if (status) {
	resource.id = 0;
	return resource;
    }

    resource = surface->next_available_resource;
    surface->next_available_resource.id++;

    return resource;
}

static void
_cairo_pdf_surface_update_object (cairo_pdf_surface_t	*surface,
				  cairo_pdf_resource_t	 resource)
{
    cairo_pdf_object_t *object;

    object = _cairo_array_index (&surface->objects, resource.id - 1);
    object->offset = _cairo_output_stream_get_position (surface->output);
}

static cairo_surface_t *
_cairo_pdf_surface_create_for_stream_internal (cairo_output_stream_t	*output,
					       double			 width,
					       double			 height)
{
    cairo_pdf_surface_t *surface;
    cairo_status_t status, status_ignored;

    surface = malloc (sizeof (cairo_pdf_surface_t));
    if (surface == NULL) {
	/* destroy stream on behalf of caller */
	status = _cairo_output_stream_destroy (output);
	return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
    }

    _cairo_surface_init (&surface->base, &cairo_pdf_surface_backend,
			 CAIRO_CONTENT_COLOR_ALPHA);

    surface->output = output;
    surface->width = width;
    surface->height = height;
    cairo_matrix_init (&surface->cairo_to_pdf, 1, 0, 0, -1, 0, height);

    _cairo_array_init (&surface->objects, sizeof (cairo_pdf_object_t));
    _cairo_array_init (&surface->pages, sizeof (cairo_pdf_resource_t));
    _cairo_array_init (&surface->rgb_linear_functions, sizeof (cairo_pdf_rgb_linear_function_t));
    _cairo_array_init (&surface->alpha_linear_functions, sizeof (cairo_pdf_alpha_linear_function_t));
    _cairo_array_init (&surface->fonts, sizeof (cairo_pdf_font_t));
    _cairo_array_init (&surface->patterns, sizeof (cairo_pdf_pattern_t));
    _cairo_array_init (&surface->smask_groups, sizeof (cairo_pdf_smask_group_t *));
    _cairo_array_init (&surface->knockout_group, sizeof (cairo_pdf_resource_t));

    _cairo_pdf_group_resources_init (&surface->resources);

    surface->font_subsets = _cairo_scaled_font_subsets_create_composite ();
    if (! surface->font_subsets) {
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);
	goto BAIL0;
    }

    surface->next_available_resource.id = 1;
    surface->pages_resource = _cairo_pdf_surface_new_object (surface);
    if (surface->pages_resource.id == 0) {
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);
        goto BAIL1;
    }

    surface->compress_content = TRUE;
    surface->pdf_stream.active = FALSE;
    surface->pdf_stream.old_output = NULL;
    surface->group_stream.active = FALSE;
    surface->group_stream.stream = NULL;
    surface->group_stream.mem_stream = NULL;

    surface->paginated_mode = CAIRO_PAGINATED_MODE_ANALYZE;

    surface->force_fallbacks = FALSE;

    _cairo_pdf_operators_init (&surface->pdf_operators,
			       surface->output,
			       surface->cairo_to_pdf,
			       surface->font_subsets);
    _cairo_pdf_operators_set_font_subsets_callback (&surface->pdf_operators,
						    _cairo_pdf_surface_add_font,
						    surface);

    /* Document header */
    _cairo_output_stream_printf (surface->output,
				 "%%PDF-1.4\r\n");
    _cairo_output_stream_printf (surface->output,
				 "%%%c%c%c%c\r\n", 181, 237, 174, 251);

    surface->paginated_surface =  _cairo_paginated_surface_create (
	                                  &surface->base,
					  CAIRO_CONTENT_COLOR_ALPHA,
					  width, height,
					  &cairo_pdf_surface_paginated_backend);
    status = surface->paginated_surface->status;
    if (status == CAIRO_STATUS_SUCCESS)
	return surface->paginated_surface;

BAIL1:
    _cairo_scaled_font_subsets_destroy (surface->font_subsets);
BAIL0:
    _cairo_array_fini (&surface->objects);
    free (surface);

    /* destroy stream on behalf of caller */
    status_ignored = _cairo_output_stream_destroy (output);

    return _cairo_surface_create_in_error (status);
}

/**
 * cairo_pdf_surface_create_for_stream:
 * @write_func: a #cairo_write_func_t to accept the output data
 * @closure: the closure argument for @write_func
 * @width_in_points: width of the surface, in points (1 point == 1/72.0 inch)
 * @height_in_points: height of the surface, in points (1 point == 1/72.0 inch)
 *
 * Creates a PDF surface of the specified size in points to be written
 * incrementally to the stream represented by @write_func and @closure.
 *
 * Return value: a pointer to the newly created surface. The caller
 * owns the surface and should call cairo_surface_destroy when done
 * with it.
 *
 * This function always returns a valid pointer, but it will return a
 * pointer to a "nil" surface if an error such as out of memory
 * occurs. You can use cairo_surface_status() to check for this.
 *
 * Since: 1.2
 */
cairo_surface_t *
cairo_pdf_surface_create_for_stream (cairo_write_func_t		 write_func,
				     void			*closure,
				     double			 width_in_points,
				     double			 height_in_points)
{
    cairo_output_stream_t *output;

    output = _cairo_output_stream_create (write_func, NULL, closure);
    if (_cairo_output_stream_get_status (output))
	return _cairo_surface_create_in_error (_cairo_output_stream_destroy (output));

    return _cairo_pdf_surface_create_for_stream_internal (output,
							  width_in_points,
							  height_in_points);
}

/**
 * cairo_pdf_surface_create:
 * @filename: a filename for the PDF output (must be writable)
 * @width_in_points: width of the surface, in points (1 point == 1/72.0 inch)
 * @height_in_points: height of the surface, in points (1 point == 1/72.0 inch)
 *
 * Creates a PDF surface of the specified size in points to be written
 * to @filename.
 *
 * Return value: a pointer to the newly created surface. The caller
 * owns the surface and should call cairo_surface_destroy when done
 * with it.
 *
 * This function always returns a valid pointer, but it will return a
 * pointer to a "nil" surface if an error such as out of memory
 * occurs. You can use cairo_surface_status() to check for this.
 *
 * Since: 1.2
 **/
cairo_surface_t *
cairo_pdf_surface_create (const char		*filename,
			  double		 width_in_points,
			  double		 height_in_points)
{
    cairo_output_stream_t *output;

    output = _cairo_output_stream_create_for_filename (filename);
    if (_cairo_output_stream_get_status (output))
	return _cairo_surface_create_in_error (_cairo_output_stream_destroy (output));

    return _cairo_pdf_surface_create_for_stream_internal (output,
							  width_in_points,
							  height_in_points);
}

static cairo_bool_t
_cairo_surface_is_pdf (cairo_surface_t *surface)
{
    return surface->backend == &cairo_pdf_surface_backend;
}

/* If the abstract_surface is a paginated surface, and that paginated
 * surface's target is a pdf_surface, then set pdf_surface to that
 * target. Otherwise return CAIRO_STATUS_SURFACE_TYPE_MISMATCH.
 */
static cairo_status_t
_extract_pdf_surface (cairo_surface_t		 *surface,
		      cairo_pdf_surface_t	**pdf_surface)
{
    cairo_surface_t *target;

    if (! _cairo_surface_is_paginated (surface))
	return _cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    target = _cairo_paginated_surface_get_target (surface);

    if (! _cairo_surface_is_pdf (target))
	return _cairo_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    *pdf_surface = (cairo_pdf_surface_t *) target;

    return CAIRO_STATUS_SUCCESS;
}

/**
 * cairo_pdf_surface_set_size:
 * @surface: a PDF cairo_surface_t
 * @width_in_points: new surface width, in points (1 point == 1/72.0 inch)
 * @height_in_points: new surface height, in points (1 point == 1/72.0 inch)
 *
 * Changes the size of a PDF surface for the current (and
 * subsequent) pages.
 *
 * This function should only be called before any drawing operations
 * have been performed on the current page. The simplest way to do
 * this is to call this function immediately after creating the
 * surface or immediately after completing a page with either
 * cairo_show_page() or cairo_copy_page().
 *
 * Since: 1.2
 **/
void
cairo_pdf_surface_set_size (cairo_surface_t	*surface,
			    double		 width_in_points,
			    double		 height_in_points)
{
    cairo_pdf_surface_t *pdf_surface = NULL; /* hide compiler warning */
    cairo_status_t status;

    status = _extract_pdf_surface (surface, &pdf_surface);
    if (status) {
	status = _cairo_surface_set_error (surface, status);
	return;
    }

    pdf_surface->width = width_in_points;
    pdf_surface->height = height_in_points;
    cairo_matrix_init (&pdf_surface->cairo_to_pdf, 1, 0, 0, -1, 0, height_in_points);
    _cairo_pdf_operators_set_cairo_to_pdf_matrix (&pdf_surface->pdf_operators,
						  pdf_surface->cairo_to_pdf);
    status = _cairo_paginated_surface_set_size (pdf_surface->paginated_surface,
						width_in_points,
						height_in_points);
    if (status)
	status = _cairo_surface_set_error (surface, status);
}

static void
_cairo_pdf_surface_clear (cairo_pdf_surface_t *surface)
{
    int i, size;
    cairo_pdf_pattern_t *pattern;
    cairo_pdf_smask_group_t *group;

    size = _cairo_array_num_elements (&surface->patterns);
    for (i = 0; i < size; i++) {
	pattern = (cairo_pdf_pattern_t *) _cairo_array_index (&surface->patterns, i);
	cairo_pattern_destroy (pattern->pattern);
    }
    _cairo_array_truncate (&surface->patterns, 0);

    size = _cairo_array_num_elements (&surface->smask_groups);
    for (i = 0; i < size; i++) {
	_cairo_array_copy_element (&surface->smask_groups, i, &group);
	_cairo_pdf_smask_group_destroy (group);
    }
    _cairo_array_truncate (&surface->smask_groups, 0);
    _cairo_array_truncate (&surface->knockout_group, 0);
}

static void
_cairo_pdf_group_resources_init (cairo_pdf_group_resources_t *res)
{
    _cairo_array_init (&res->alphas, sizeof (double));
    _cairo_array_init (&res->smasks, sizeof (cairo_pdf_resource_t));
    _cairo_array_init (&res->patterns, sizeof (cairo_pdf_resource_t));
    _cairo_array_init (&res->xobjects, sizeof (cairo_pdf_resource_t));
    _cairo_array_init (&res->fonts, sizeof (cairo_pdf_font_t));
}

static void
_cairo_pdf_group_resources_fini (cairo_pdf_group_resources_t *res)
{
    _cairo_array_fini (&res->alphas);
    _cairo_array_fini (&res->smasks);
    _cairo_array_fini (&res->patterns);
    _cairo_array_fini (&res->xobjects);
    _cairo_array_fini (&res->fonts);
}

static void
_cairo_pdf_group_resources_clear (cairo_pdf_group_resources_t *res)
{
    _cairo_array_truncate (&res->alphas, 0);
    _cairo_array_truncate (&res->smasks, 0);
    _cairo_array_truncate (&res->patterns, 0);
    _cairo_array_truncate (&res->xobjects, 0);
    _cairo_array_truncate (&res->fonts, 0);
}

static cairo_status_t
_cairo_pdf_surface_add_alpha (cairo_pdf_surface_t *surface,
			      double               alpha,
			      int                 *index)
{
    int num_alphas, i;
    double other;
    cairo_status_t status;
    cairo_pdf_group_resources_t *res = &surface->resources;

    num_alphas = _cairo_array_num_elements (&res->alphas);
    for (i = 0; i < num_alphas; i++) {
	_cairo_array_copy_element (&res->alphas, i, &other);
	if (alpha == other) {
	    *index = i;
	    return CAIRO_STATUS_SUCCESS;
	}
    }

    status = _cairo_array_append (&res->alphas, &alpha);
    if (status)
	return status;

    *index = _cairo_array_num_elements (&res->alphas) - 1;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_pdf_surface_add_smask (cairo_pdf_surface_t  *surface,
			      cairo_pdf_resource_t  smask)
{
    return _cairo_array_append (&(surface->resources.smasks), &smask);
}

static cairo_status_t
_cairo_pdf_surface_add_pattern (cairo_pdf_surface_t  *surface,
				cairo_pdf_resource_t  pattern)
{
    return _cairo_array_append (&(surface->resources.patterns), &pattern);
}

static cairo_status_t
_cairo_pdf_surface_add_xobject (cairo_pdf_surface_t  *surface,
				cairo_pdf_resource_t  xobject)
{
    return _cairo_array_append (&(surface->resources.xobjects), &xobject);
}

static cairo_status_t
_cairo_pdf_surface_add_font (unsigned int        font_id,
			     unsigned int        subset_id,
			     void		*closure)
{
    cairo_pdf_surface_t *surface = closure;
    cairo_pdf_font_t font;
    int num_fonts, i;
    cairo_status_t status;
    cairo_pdf_group_resources_t *res = &surface->resources;

    num_fonts = _cairo_array_num_elements (&res->fonts);
    for (i = 0; i < num_fonts; i++) {
	_cairo_array_copy_element (&res->fonts, i, &font);
	if (font.font_id == font_id &&
	    font.subset_id == subset_id)
	    return CAIRO_STATUS_SUCCESS;
    }

    num_fonts = _cairo_array_num_elements (&surface->fonts);
    for (i = 0; i < num_fonts; i++) {
	_cairo_array_copy_element (&surface->fonts, i, &font);
	if (font.font_id == font_id &&
	    font.subset_id == subset_id)
	    return _cairo_array_append (&res->fonts, &font);
    }

    font.font_id = font_id;
    font.subset_id = subset_id;
    font.subset_resource = _cairo_pdf_surface_new_object (surface);
    if (font.subset_resource.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    status = _cairo_array_append (&surface->fonts, &font);
    if (status)
	return status;

    return _cairo_array_append (&res->fonts, &font);
}

static cairo_pdf_resource_t
_cairo_pdf_surface_get_font_resource (cairo_pdf_surface_t *surface,
				      unsigned int         font_id,
				      unsigned int         subset_id)
{
    cairo_pdf_font_t font;
    int num_fonts, i;

    num_fonts = _cairo_array_num_elements (&surface->fonts);
    for (i = 0; i < num_fonts; i++) {
	_cairo_array_copy_element (&surface->fonts, i, &font);
	if (font.font_id == font_id && font.subset_id == subset_id)
	    return font.subset_resource;
    }

    font.subset_resource.id = 0;
    return font.subset_resource;
}

static void
_cairo_pdf_surface_emit_group_resources (cairo_pdf_surface_t         *surface,
					 cairo_pdf_group_resources_t *res)
{
    int num_alphas, num_smasks, num_resources, i;
    double alpha;
    cairo_pdf_resource_t *smask, *pattern, *xobject;
    cairo_pdf_font_t *font;

    _cairo_output_stream_printf (surface->output, "<<\r\n");

    num_alphas = _cairo_array_num_elements (&res->alphas);
    num_smasks = _cairo_array_num_elements (&res->smasks);
    if (num_alphas > 0 || num_smasks > 0) {
	_cairo_output_stream_printf (surface->output,
				     "   /ExtGState <<\r\n");

	for (i = 0; i < num_alphas; i++) {
	    _cairo_array_copy_element (&res->alphas, i, &alpha);
	    _cairo_output_stream_printf (surface->output,
					 "      /a%d << /CA %f /ca %f >>\r\n",
					 i, alpha, alpha);
	}

	for (i = 0; i < num_smasks; i++) {
	    smask = _cairo_array_index (&res->smasks, i);
	    _cairo_output_stream_printf (surface->output,
					 "      /s%d %d 0 R\r\n",
					 smask->id, smask->id);
	}

	_cairo_output_stream_printf (surface->output,
				     "   >>\r\n");
    }

    num_resources = _cairo_array_num_elements (&res->patterns);
    if (num_resources > 0) {
	_cairo_output_stream_printf (surface->output,
				     "   /Pattern <<");
	for (i = 0; i < num_resources; i++) {
	    pattern = _cairo_array_index (&res->patterns, i);
	    _cairo_output_stream_printf (surface->output,
					 " /p%d %d 0 R",
					 pattern->id, pattern->id);
	}

	_cairo_output_stream_printf (surface->output,
				     " >>\r\n");
    }

    num_resources = _cairo_array_num_elements (&res->xobjects);
    if (num_resources > 0) {
	_cairo_output_stream_printf (surface->output,
				     "   /XObject <<");

	for (i = 0; i < num_resources; i++) {
	    xobject = _cairo_array_index (&res->xobjects, i);
	    _cairo_output_stream_printf (surface->output,
					 " /x%d %d 0 R",
					 xobject->id, xobject->id);
	}

	_cairo_output_stream_printf (surface->output,
				     " >>\r\n");
    }

    num_resources = _cairo_array_num_elements (&res->fonts);
    if (num_resources > 0) {
	_cairo_output_stream_printf (surface->output,"   /Font <<\r\n");
	for (i = 0; i < num_resources; i++) {
	    font = _cairo_array_index (&res->fonts, i);
	    _cairo_output_stream_printf (surface->output,
					 "      /f-%d-%d %d 0 R\r\n",
					 font->font_id,
					 font->subset_id,
					 font->subset_resource.id);
	}
	_cairo_output_stream_printf (surface->output, "   >>\r\n");
    }

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n");
}

static cairo_pdf_smask_group_t *
_cairo_pdf_surface_create_smask_group (cairo_pdf_surface_t	*surface)
{
    cairo_pdf_smask_group_t	*group;

    group = calloc (1, sizeof (cairo_pdf_smask_group_t));
    if (group == NULL) {
	_cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
	return NULL;
    }

    group->group_res = _cairo_pdf_surface_new_object (surface);
    if (group->group_res.id == 0) {
	_cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
	free (group);
	return NULL;
    }

    return group;
}

static void
_cairo_pdf_smask_group_destroy (cairo_pdf_smask_group_t *group)
{
    if (group->operation == PDF_FILL ||	group->operation == PDF_STROKE)
	_cairo_path_fixed_fini (&group->path);
    if (group->source)
	cairo_pattern_destroy (group->source);
    if (group->mask)
	cairo_pattern_destroy (group->mask);
    if (group->scaled_font)
	cairo_scaled_font_destroy (group->scaled_font);
    free (group);
}

static cairo_status_t
_cairo_pdf_surface_add_smask_group (cairo_pdf_surface_t     *surface,
				    cairo_pdf_smask_group_t *group)
{
    return _cairo_array_append (&surface->smask_groups, &group);
}

static cairo_status_t
_cairo_pdf_surface_add_pdf_pattern (cairo_pdf_surface_t		*surface,
				    cairo_pattern_t		*pattern,
				    cairo_pdf_resource_t	*pattern_res,
				    cairo_pdf_resource_t	*gstate_res)
{
    cairo_pdf_pattern_t pdf_pattern;
    cairo_status_t status;

    /* Solid colors are emitted into the content stream */
    if (pattern->type == CAIRO_PATTERN_TYPE_SOLID) {
	pattern_res->id = 0;
	gstate_res->id = 0;
	return CAIRO_STATUS_SUCCESS;
    }

    /* Gradients with zero stops do not produce any output */
    if (pattern->type == CAIRO_PATTERN_TYPE_LINEAR ||
        pattern->type == CAIRO_PATTERN_TYPE_RADIAL)
    {
	cairo_gradient_pattern_t *gradient;

	gradient = (cairo_gradient_pattern_t *) pattern;
	if (gradient->n_stops == 0)
	    return CAIRO_INT_STATUS_NOTHING_TO_DO;
    }

    pdf_pattern.pattern = cairo_pattern_reference (pattern);
    pdf_pattern.pattern_res = _cairo_pdf_surface_new_object (surface);
    if (pdf_pattern.pattern_res.id == 0) {
	cairo_pattern_destroy (pattern);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    pdf_pattern.gstate_res.id = 0;

    /* gradient patterns require an smask object to implement transparency */
    if (pattern->type == CAIRO_PATTERN_TYPE_LINEAR ||
        pattern->type == CAIRO_PATTERN_TYPE_RADIAL) {
        if (_cairo_pattern_is_opaque (pattern) == FALSE) {
            pdf_pattern.gstate_res = _cairo_pdf_surface_new_object (surface);
	    if (pdf_pattern.gstate_res.id == 0) {
		cairo_pattern_destroy (pattern);
		return _cairo_error (CAIRO_STATUS_NO_MEMORY);
	    }
        }
    }

    *pattern_res = pdf_pattern.pattern_res;
    *gstate_res = pdf_pattern.gstate_res;

    status = _cairo_array_append (&surface->patterns, &pdf_pattern);
    if (status) {
	cairo_pattern_destroy (pattern);
	return status;
    }

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_pdf_surface_open_stream (cairo_pdf_surface_t	*surface,
				cairo_pdf_resource_t    *resource,
				cairo_bool_t             compressed,
				const char		*fmt,
				...)
{
    va_list ap;
    cairo_pdf_resource_t self, length;
    cairo_output_stream_t *output = NULL;

    if (resource) {
	self = *resource;
	_cairo_pdf_surface_update_object (surface, self);
    } else {
	self = _cairo_pdf_surface_new_object (surface);
	if (self.id == 0)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    length = _cairo_pdf_surface_new_object (surface);
    if (length.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    if (compressed) {
	output = _cairo_deflate_stream_create (surface->output);
	if (_cairo_output_stream_get_status (output))
	    return _cairo_output_stream_destroy (output);
    }

    surface->pdf_stream.active = TRUE;
    surface->pdf_stream.self = self;
    surface->pdf_stream.length = length;
    surface->pdf_stream.compressed = compressed;

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Length %d 0 R\r\n",
				 surface->pdf_stream.self.id,
				 surface->pdf_stream.length.id);
    if (compressed)
	_cairo_output_stream_printf (surface->output,
				     "   /Filter /FlateDecode\r\n");

    if (fmt != NULL) {
	va_start (ap, fmt);
	_cairo_output_stream_vprintf (surface->output, fmt, ap);
	va_end (ap);
    }

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "stream\r\n");

    surface->pdf_stream.start_offset = _cairo_output_stream_get_position (surface->output);

    if (compressed) {
	assert (surface->pdf_stream.old_output == NULL);
        surface->pdf_stream.old_output = surface->output;
        surface->output = output;
	_cairo_pdf_operators_set_stream (&surface->pdf_operators, surface->output);
    }

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
_cairo_pdf_surface_close_stream (cairo_pdf_surface_t *surface)
{
    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    long length;

    if (! surface->pdf_stream.active)
	return CAIRO_STATUS_SUCCESS;

    if (surface->pdf_stream.compressed) {
	status = _cairo_output_stream_destroy (surface->output);
	surface->output = surface->pdf_stream.old_output;
	_cairo_pdf_operators_set_stream (&surface->pdf_operators, surface->output);
	surface->pdf_stream.old_output = NULL;
	_cairo_output_stream_printf (surface->output,
				     "\r\n");
    }

    length = _cairo_output_stream_get_position (surface->output) -
	surface->pdf_stream.start_offset;
    _cairo_output_stream_printf (surface->output,
				 "endstream\r\n"
				 "endobj\r\n");

    _cairo_pdf_surface_update_object (surface,
				      surface->pdf_stream.length);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "   %ld\r\n"
				 "endobj\r\n",
				 surface->pdf_stream.length.id,
				 length);

    surface->pdf_stream.active = FALSE;

    if (status == CAIRO_STATUS_SUCCESS)
	status = _cairo_output_stream_get_status (surface->output);

    return status;
}

static void
_cairo_pdf_surface_write_memory_stream (cairo_pdf_surface_t         *surface,
					cairo_output_stream_t       *mem_stream,
					cairo_pdf_resource_t         resource,
					cairo_pdf_group_resources_t *resources,
					cairo_bool_t                 is_knockout_group)
{
    _cairo_pdf_surface_update_object (surface, resource);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /XObject\r\n"
				 "   /Length %d\r\n",
				 resource.id,
				 _cairo_memory_stream_length (mem_stream));

    if (surface->compress_content) {
	_cairo_output_stream_printf (surface->output,
				     "   /Filter /FlateDecode\r\n");
    }

    _cairo_output_stream_printf (surface->output,
				 "   /Subtype /Form\r\n"
				 "   /BBox [ 0 0 %f %f ]\r\n"
				 "   /Group <<\r\n"
				 "      /Type /Group\r\n"
				 "      /S /Transparency\r\n"
				 "      /CS /DeviceRGB\r\n",
				 surface->width,
				 surface->height);

    if (is_knockout_group)
        _cairo_output_stream_printf (surface->output,
                                     "      /K true\r\n");

    _cairo_output_stream_printf (surface->output,
				 "   >>\r\n"
				 "   /Resources\r\n");
    _cairo_pdf_surface_emit_group_resources (surface, resources);
    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "stream\r\n");
    _cairo_memory_stream_copy (mem_stream, surface->output);
    _cairo_output_stream_printf (surface->output,
				 "endstream\r\n"
				 "endobj\r\n");
}

static cairo_status_t
_cairo_pdf_surface_open_group (cairo_pdf_surface_t  *surface,
			       cairo_pdf_resource_t *resource)
{
    cairo_status_t status;

    assert (surface->pdf_stream.active == FALSE);
    assert (surface->group_stream.active == FALSE);

    surface->group_stream.active = TRUE;

    surface->group_stream.mem_stream = _cairo_memory_stream_create ();

    if (surface->compress_content) {
	surface->group_stream.stream =
	    _cairo_deflate_stream_create (surface->group_stream.mem_stream);
    } else {
	surface->group_stream.stream = surface->group_stream.mem_stream;
    }
    status = _cairo_output_stream_get_status (surface->group_stream.stream);

    surface->group_stream.old_output = surface->output;
    surface->output = surface->group_stream.stream;
    _cairo_pdf_operators_set_stream (&surface->pdf_operators, surface->output);
    _cairo_pdf_group_resources_clear (&surface->resources);

    if (resource) {
	surface->group_stream.resource = *resource;
    } else {
	surface->group_stream.resource = _cairo_pdf_surface_new_object (surface);
	if (surface->group_stream.resource.id == 0)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }
    surface->group_stream.is_knockout = FALSE;

    return status;
}

static cairo_status_t
_cairo_pdf_surface_open_knockout_group (cairo_pdf_surface_t  *surface)
{
    cairo_status_t status;

    status = _cairo_pdf_surface_open_group (surface, NULL);
    if (status)
	return status;

    surface->group_stream.is_knockout = TRUE;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_pdf_surface_close_group (cairo_pdf_surface_t *surface,
				cairo_pdf_resource_t *group)
{
    cairo_status_t status = CAIRO_STATUS_SUCCESS, status2;

    assert (surface->pdf_stream.active == FALSE);
    assert (surface->group_stream.active == TRUE);

    if (surface->compress_content) {
	status = _cairo_output_stream_destroy (surface->group_stream.stream);
	surface->group_stream.stream = NULL;

	_cairo_output_stream_printf (surface->group_stream.mem_stream,
				     "\r\n");
    }
    surface->output = surface->group_stream.old_output;
    _cairo_pdf_operators_set_stream (&surface->pdf_operators, surface->output);
    surface->group_stream.active = FALSE;
    _cairo_pdf_surface_write_memory_stream (surface,
					    surface->group_stream.mem_stream,
					    surface->group_stream.resource,
					    &surface->resources,
					    surface->group_stream.is_knockout);
    if (group)
	*group = surface->group_stream.resource;

    status2 = _cairo_output_stream_destroy (surface->group_stream.mem_stream);
    if (status == CAIRO_STATUS_SUCCESS)
	status = status2;

    surface->group_stream.mem_stream = NULL;
    surface->group_stream.stream = NULL;

    return status;
}

static cairo_status_t
_cairo_pdf_surface_open_content_stream (cairo_pdf_surface_t  *surface,
					cairo_bool_t          is_form)
{
    cairo_status_t status;

    assert (surface->pdf_stream.active == FALSE);
    assert (surface->group_stream.active == FALSE);

    surface->content_resources = _cairo_pdf_surface_new_object (surface);
    if (surface->content_resources.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    if (is_form) {
	status =
	    _cairo_pdf_surface_open_stream (surface,
					    NULL,
					    surface->compress_content,
					    "   /Type /XObject\r\n"
					    "   /Subtype /Form\r\n"
					    "   /BBox [ 0 0 %f %f ]\r\n"
					    "   /Group <<\r\n"
					    "      /Type /Group\r\n"
					    "      /S /Transparency\r\n"
					    "      /CS /DeviceRGB\r\n"
					    "   >>\r\n"
					    "   /Resources %d 0 R\r\n",
					    surface->width,
					    surface->height,
					    surface->content_resources.id);
	if (status)
	    return status;
    } else {
	status =
	    _cairo_pdf_surface_open_stream (surface,
					    NULL,
					    surface->compress_content,
					    NULL);
	if (status)
	    return status;
    }

    surface->content = surface->pdf_stream.self;

    _cairo_output_stream_printf (surface->output, "q\r\n");

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
_cairo_pdf_surface_close_content_stream (cairo_pdf_surface_t *surface)
{
    cairo_status_t status;

    assert (surface->pdf_stream.active == TRUE);
    assert (surface->group_stream.active == FALSE);

    _cairo_output_stream_printf (surface->output, "Q\r\n");
    status = _cairo_pdf_surface_close_stream (surface);
    if (status)
	return status;

    _cairo_pdf_surface_update_object (surface, surface->content_resources);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n",
				 surface->content_resources.id);
    _cairo_pdf_surface_emit_group_resources (surface, &surface->resources);
    _cairo_output_stream_printf (surface->output,
				 "endobj\r\n");

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_surface_t *
_cairo_pdf_surface_create_similar (void			*abstract_surface,
				   cairo_content_t	 content,
				   int			 width,
				   int			 height)
{
    return _cairo_meta_surface_create (content, width, height);
}

static cairo_status_t
_cairo_pdf_surface_finish (void *abstract_surface)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    long offset;
    cairo_pdf_resource_t info, catalog;
    cairo_status_t status, status2;

    status = surface->base.status;
    if (status == CAIRO_STATUS_SUCCESS)
	status = _cairo_pdf_surface_emit_font_subsets (surface);

    _cairo_pdf_surface_write_pages (surface);

    info = _cairo_pdf_surface_write_info (surface);
    if (info.id == 0 && status == CAIRO_STATUS_SUCCESS)
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);

    catalog = _cairo_pdf_surface_write_catalog (surface);
    if (catalog.id == 0 && status == CAIRO_STATUS_SUCCESS)
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);

    offset = _cairo_pdf_surface_write_xref (surface);

    _cairo_output_stream_printf (surface->output,
				 "trailer\r\n"
				 "<< /Size %d\r\n"
				 "   /Root %d 0 R\r\n"
				 "   /Info %d 0 R\r\n"
				 ">>\r\n",
				 surface->next_available_resource.id,
				 catalog.id,
				 info.id);

    _cairo_output_stream_printf (surface->output,
				 "startxref\r\n"
				 "%ld\r\n"
				 "%%%%EOF\r\n",
				 offset);

     _cairo_pdf_operators_fini (&surface->pdf_operators);

    /* close any active streams still open due to fatal errors */
    status2 = _cairo_pdf_surface_close_stream (surface);
    if (status == CAIRO_STATUS_SUCCESS)
	status = status2;

    if (surface->group_stream.stream != NULL) {
	status2 = _cairo_output_stream_destroy (surface->group_stream.stream);
	if (status == CAIRO_STATUS_SUCCESS)
	    status = status2;
    }
    if (surface->group_stream.mem_stream != NULL) {
	status2 = _cairo_output_stream_destroy (surface->group_stream.mem_stream);
	if (status == CAIRO_STATUS_SUCCESS)
	    status = status2;
    }
    if (surface->pdf_stream.active)
	surface->output = surface->pdf_stream.old_output;
    if (surface->group_stream.active)
	surface->output = surface->group_stream.old_output;

    /* and finish the pdf surface */
    status2 = _cairo_output_stream_destroy (surface->output);
    if (status == CAIRO_STATUS_SUCCESS)
	status = status2;

    _cairo_pdf_surface_clear (surface);
    _cairo_pdf_group_resources_fini (&surface->resources);

    _cairo_array_fini (&surface->objects);
    _cairo_array_fini (&surface->pages);
    _cairo_array_fini (&surface->rgb_linear_functions);
    _cairo_array_fini (&surface->alpha_linear_functions);
    _cairo_array_fini (&surface->patterns);
    _cairo_array_fini (&surface->smask_groups);
    _cairo_array_fini (&surface->fonts);
    _cairo_array_fini (&surface->knockout_group);

    if (surface->font_subsets) {
	_cairo_scaled_font_subsets_destroy (surface->font_subsets);
	surface->font_subsets = NULL;
    }

    return status;
}

static cairo_int_status_t
_cairo_pdf_surface_start_page (void *abstract_surface)
{
    cairo_status_t status;
    cairo_pdf_surface_t *surface = abstract_surface;

    surface->has_fallback_images = FALSE;
    _cairo_pdf_group_resources_clear (&surface->resources);
    status = _cairo_pdf_surface_open_content_stream (surface, TRUE);
    if (status)
	return status;

    return CAIRO_STATUS_SUCCESS;
}

static void *
compress_dup (const void *data, unsigned long data_size,
	      unsigned long *compressed_size)
{
    void *compressed;

    /* Bound calculation taken from zlib. */
    *compressed_size = data_size + (data_size >> 12) + (data_size >> 14) + 11;
    compressed = malloc (*compressed_size);
    if (compressed == NULL) {
	_cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
	return NULL;
    }

    if (compress (compressed, compressed_size, data, data_size) != Z_OK) {
	_cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
	free (compressed);
	compressed = NULL;
    }

    return compressed;
}

/* Emit alpha channel from the image into the given data, providing
 * an id that can be used to reference the resulting SMask object.
 *
 * In the case that the alpha channel happens to be all opaque, then
 * no SMask object will be emitted and *id_ret will be set to 0.
 */
static cairo_status_t
_cairo_pdf_surface_emit_smask (cairo_pdf_surface_t	*surface,
			       cairo_image_surface_t	*image,
			       cairo_pdf_resource_t	*stream_ret)
{
    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    char *alpha, *alpha_compressed;
    unsigned long alpha_size, alpha_compressed_size;
    uint32_t *pixel32;
    uint8_t *pixel8;
    int i, x, y;
    cairo_bool_t opaque;
    uint8_t a;
    int src_bit, dst_bit;

    /* This is the only image format we support, which simplifies things. */
    assert (image->format == CAIRO_FORMAT_ARGB32 ||
	    image->format == CAIRO_FORMAT_A8 ||
	    image->format == CAIRO_FORMAT_A1 );

    stream_ret->id = 0;

    if (image->format == CAIRO_FORMAT_A1)
	alpha_size = (image->height * image->width + 7)/8;
    else
	alpha_size = image->height * image->width;
    alpha = malloc (alpha_size);
    if (alpha == NULL) {
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);
	goto CLEANUP;
    }

    opaque = TRUE;
    i = 0;
    src_bit = 0;
    dst_bit = 7;
    for (y = 0; y < image->height; y++) {
	if (image->format == CAIRO_FORMAT_ARGB32) {
	    pixel32 = (uint32_t *) (image->data + y * image->stride);

	    for (x = 0; x < image->width; x++, pixel32++) {
		a = (*pixel32 & 0xff000000) >> 24;
		alpha[i++] = a;
		if (a != 0xff)
		    opaque = FALSE;
	    }
	} else if (image->format == CAIRO_FORMAT_A8){
	    pixel8 = (uint8_t *) (image->data + y * image->stride);

	    for (x = 0; x < image->width; x++, pixel8++) {
		a = *pixel8;
		alpha[i++] = a;
		if (a != 0xff)
		    opaque = FALSE;
	    }
	} else { /* image->format == CAIRO_FORMAT_A1 */
	    pixel8 = (uint8_t *) (image->data + y * image->stride);

	    for (x = 0; x < image->width; x++, pixel8++) {
		if (dst_bit == 7)
		    alpha[i] = 0;
		if ((*pixel8 >> src_bit) & 1) {
			opaque = FALSE;
			alpha[i] |= (1 << dst_bit);
		}
		if (++src_bit > 7) {
		    src_bit = 0;
		    pixel8++;
		}
		if (--dst_bit < 0) {
		    dst_bit = 7;
		    i++;
		}
	    }
	}
    }

    /* Bail out without emitting smask if it's all opaque. */
    if (opaque)
	goto CLEANUP_ALPHA;

    alpha_compressed = compress_dup (alpha, alpha_size, &alpha_compressed_size);
    if (alpha_compressed == NULL) {
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);
	goto CLEANUP_ALPHA;
    }

    status = _cairo_pdf_surface_open_stream (surface,
					     NULL,
					     FALSE,
					     "   /Type /XObject\r\n"
					     "   /Subtype /Image\r\n"
					     "   /Width %d\r\n"
					     "   /Height %d\r\n"
					     "   /ColorSpace /DeviceGray\r\n"
					     "   /BitsPerComponent %d\r\n"
					     "   /Filter /FlateDecode\r\n",
					     image->width, image->height,
					     image->format == CAIRO_FORMAT_A1 ? 1 : 8);
    if (status)
	goto CLEANUP_ALPHA_COMPRESSED;

    *stream_ret = surface->pdf_stream.self;

    _cairo_output_stream_write (surface->output, alpha_compressed, alpha_compressed_size);
    _cairo_output_stream_printf (surface->output, "\r\n");
    status = _cairo_pdf_surface_close_stream (surface);

 CLEANUP_ALPHA_COMPRESSED:
    free (alpha_compressed);
 CLEANUP_ALPHA:
    free (alpha);
 CLEANUP:
    return status;
}

/* Emit image data into the given surface, providing a resource that
 * can be used to reference the data in image_ret. */
static cairo_status_t
_cairo_pdf_surface_emit_image (cairo_pdf_surface_t   *surface,
                               cairo_image_surface_t *image,
                               cairo_pdf_resource_t  *image_ret)
{
    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    char *rgb, *compressed;
    unsigned long rgb_size, compressed_size;
    uint32_t *pixel;
    int i, x, y;
    cairo_pdf_resource_t smask = {0}; /* squelch bogus compiler warning */
    cairo_bool_t need_smask;

    /* XXX: Need to rewrite this as a pdf_surface function with
     * pause/resume of content_stream, (currently the only caller does
     * the pause/resume already, but that is expected to change in the
     * future). */

    /* These are the only image formats we currently support, (which
     * makes things a lot simpler here). This is enforced through
     * _cairo_pdf_surface_analyze_operation which only accept source surfaces of
     * CONTENT_COLOR or CONTENT_COLOR_ALPHA.
     */
    assert (image->format == CAIRO_FORMAT_RGB24 ||
	    image->format == CAIRO_FORMAT_ARGB32 ||
	    image->format == CAIRO_FORMAT_A8 ||
	    image->format == CAIRO_FORMAT_A1);

    rgb_size = image->height * image->width * 3;
    rgb = malloc (rgb_size);
    if (rgb == NULL) {
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);
	goto CLEANUP;
    }

    i = 0;
    for (y = 0; y < image->height; y++) {
	pixel = (uint32_t *) (image->data + y * image->stride);

	for (x = 0; x < image->width; x++, pixel++) {
	    /* XXX: We're un-premultiplying alpha here. My reading of the PDF
	     * specification suggests that we should be able to avoid having
	     * to do this by filling in the SMask's Matte dictionary
	     * appropriately, but my attempts to do that so far have
	     * failed. */
	    if (image->format == CAIRO_FORMAT_ARGB32) {
		uint8_t a;
		a = (*pixel & 0xff000000) >> 24;
		if (a == 0) {
		    rgb[i++] = 0;
		    rgb[i++] = 0;
		    rgb[i++] = 0;
		} else {
		    rgb[i++] = (((*pixel & 0xff0000) >> 16) * 255 + a / 2) / a;
		    rgb[i++] = (((*pixel & 0x00ff00) >>  8) * 255 + a / 2) / a;
		    rgb[i++] = (((*pixel & 0x0000ff) >>  0) * 255 + a / 2) / a;
		}
	    } else if (image->format == CAIRO_FORMAT_RGB24) {
		rgb[i++] = (*pixel & 0x00ff0000) >> 16;
		rgb[i++] = (*pixel & 0x0000ff00) >>  8;
		rgb[i++] = (*pixel & 0x000000ff) >>  0;
	    } else {
		rgb[i++] = 0;
		rgb[i++] = 0;
		rgb[i++] = 0;
	    }
	}
    }

    compressed = compress_dup (rgb, rgb_size, &compressed_size);
    if (compressed == NULL) {
	status = _cairo_error (CAIRO_STATUS_NO_MEMORY);
	goto CLEANUP_RGB;
    }

    need_smask = FALSE;
    if (image->format == CAIRO_FORMAT_ARGB32 ||
	image->format == CAIRO_FORMAT_A8 ||
	image->format == CAIRO_FORMAT_A1) {
	status = _cairo_pdf_surface_emit_smask (surface, image, &smask);
	if (status)
	    goto CLEANUP_COMPRESSED;

	if (smask.id)
	    need_smask = TRUE;
    }

#define IMAGE_DICTIONARY	"   /Type /XObject\r\n"		\
				"   /Subtype /Image\r\n"	\
				"   /Width %d\r\n"		\
				"   /Height %d\r\n"		\
				"   /ColorSpace /DeviceRGB\r\n"	\
				"   /BitsPerComponent 8\r\n"	\
				"   /Filter /FlateDecode\r\n"

    if (need_smask)
	status = _cairo_pdf_surface_open_stream (surface,
						 NULL,
						 FALSE,
						 IMAGE_DICTIONARY
						 "   /SMask %d 0 R\r\n",
						 image->width, image->height,
						 smask.id);
    else
	status = _cairo_pdf_surface_open_stream (surface,
						 NULL,
						 FALSE,
						 IMAGE_DICTIONARY,
						 image->width, image->height);
    if (status)
	goto CLEANUP_COMPRESSED;

#undef IMAGE_DICTIONARY

    *image_ret = surface->pdf_stream.self;

    _cairo_output_stream_write (surface->output, compressed, compressed_size);
    _cairo_output_stream_printf (surface->output, "\r\n");
    status = _cairo_pdf_surface_close_stream (surface);

CLEANUP_COMPRESSED:
    free (compressed);
CLEANUP_RGB:
    free (rgb);
CLEANUP:
    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_image_surface (cairo_pdf_surface_t     *surface,
				       cairo_surface_pattern_t *pattern,
				       cairo_pdf_resource_t    *resource,
				       int                     *width,
				       int                     *height)
{
    cairo_image_surface_t *image;
    void *image_extra;
    cairo_status_t status;

    status = _cairo_surface_acquire_source_image (pattern->surface, &image, &image_extra);
    if (status)
	goto BAIL;

    status = _cairo_pdf_surface_emit_image (surface, image, resource);
    if (status)
	goto BAIL;

    *width = image->width;
    *height = image->height;

BAIL:
    _cairo_surface_release_source_image (pattern->surface, image, image_extra);

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_meta_surface (cairo_pdf_surface_t  *surface,
				      cairo_surface_t      *meta_surface,
				      cairo_pdf_resource_t *resource)
{
    double old_width, old_height;
    cairo_matrix_t old_cairo_to_pdf;
    cairo_paginated_mode_t old_paginated_mode;
    cairo_rectangle_int_t meta_extents;
    cairo_status_t status;
    int alpha = 0;

    status = _cairo_surface_get_extents (meta_surface, &meta_extents);
    if (status)
	return status;

    old_width = surface->width;
    old_height = surface->height;
    old_cairo_to_pdf = surface->cairo_to_pdf;
    old_paginated_mode = surface->paginated_mode;
    surface->width = meta_extents.width;
    surface->height = meta_extents.height;
    /* Patterns are emitted after fallback images. The paginated mode
     * needs to be set to _RENDER while the meta surface is replayed
     * back to this surface.
     */
    surface->paginated_mode = CAIRO_PAGINATED_MODE_RENDER;
    cairo_matrix_init (&surface->cairo_to_pdf, 1, 0, 0, -1, 0, surface->height);
    _cairo_pdf_operators_set_cairo_to_pdf_matrix (&surface->pdf_operators,
						  surface->cairo_to_pdf);

    _cairo_pdf_group_resources_clear (&surface->resources);
    status = _cairo_pdf_surface_open_content_stream (surface, TRUE);
    if (status)
	return status;

    *resource = surface->content;
    if (cairo_surface_get_content (meta_surface) == CAIRO_CONTENT_COLOR) {
	status = _cairo_pdf_surface_add_alpha (surface, 1.0, &alpha);
	if (status)
	    goto CLEANUP_GROUP;

	_cairo_output_stream_printf (surface->output,
				     "q /a%d gs 0 0 0 rg 0 0 %f %f re f Q\r\n",
				     alpha,
				     surface->width,
				     surface->height);
    }

    status = _cairo_meta_surface_replay_region (meta_surface, &surface->base,
						CAIRO_META_REGION_NATIVE);
    assert (status != CAIRO_INT_STATUS_UNSUPPORTED);
    if (status)
	goto CLEANUP_GROUP;

    status = _cairo_pdf_surface_close_content_stream (surface);

 CLEANUP_GROUP:
    surface->width = old_width;
    surface->height = old_height;
    surface->paginated_mode = old_paginated_mode;
    surface->cairo_to_pdf = old_cairo_to_pdf;
    _cairo_pdf_operators_set_cairo_to_pdf_matrix (&surface->pdf_operators,
						  surface->cairo_to_pdf);

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_surface_pattern (cairo_pdf_surface_t	*surface,
					 cairo_pdf_pattern_t	*pdf_pattern)
{
    cairo_surface_pattern_t *pattern = (cairo_surface_pattern_t *) pdf_pattern->pattern;
    cairo_status_t status;
    cairo_pdf_resource_t pattern_resource = {0}; /* squelch bogus compiler warning */
    cairo_matrix_t cairo_p2d, pdf_p2d;
    cairo_extend_t extend = cairo_pattern_get_extend (&pattern->base);
    double xstep, ystep;
    cairo_rectangle_int_t surface_extents;
    int pattern_width = 0; /* squelch bogus compiler warning */
    int pattern_height = 0; /* squelch bogus compiler warning */
    int bbox_x, bbox_y;
    char draw_surface[200];

    if (_cairo_surface_is_meta (pattern->surface)) {
	cairo_surface_t *meta_surface = pattern->surface;
	cairo_rectangle_int_t pattern_extents;

	status = _cairo_pdf_surface_emit_meta_surface (surface,
						       meta_surface,
						       &pattern_resource);
	if (status)
	    return status;

	status = _cairo_surface_get_extents (meta_surface, &pattern_extents);
	if (status)
	    return status;

	pattern_width = pattern_extents.width;
	pattern_height = pattern_extents.height;
    } else {
	status = _cairo_pdf_surface_emit_image_surface (surface,
							pattern,
							&pattern_resource,
							&pattern_width,
							&pattern_height);
	if (status)
	    return status;
    }

    status = _cairo_surface_get_extents (&surface->base, &surface_extents);
    if (status)
	return status;

    bbox_x = pattern_width;
    bbox_y = pattern_height;
    switch (extend) {
	/* We implement EXTEND_PAD like EXTEND_NONE for now */
    case CAIRO_EXTEND_PAD:
    case CAIRO_EXTEND_NONE:
    {
	/* In PS/PDF, (as far as I can tell), all patterns are
	 * repeating. So we support cairo's EXTEND_NONE semantics
	 * by setting the repeat step size to a size large enough
	 * to guarantee that no more than a single occurrence will
	 * be visible.
	 *
	 * First, map the surface extents into pattern space (since
	 * xstep and ystep are in pattern space).  Then use an upper
	 * bound on the length of the diagonal of the pattern image
	 * and the surface as repeat size.  This guarantees to never
	 * repeat visibly.
	 */
	double x1 = 0.0, y1 = 0.0;
	double x2 = surface->width, y2 = surface->height;
	_cairo_matrix_transform_bounding_box (&pattern->base.matrix,
					      &x1, &y1, &x2, &y2,
					      NULL);

	/* Rather than computing precise bounds of the union, just
	 * add the surface extents unconditionally. We only
	 * required an answer that's large enough, we don't really
	 * care if it's not as tight as possible.*/
	xstep = ystep = ceil ((x2 - x1) + (y2 - y1) +
			      pattern_width + pattern_height);
    }
    break;
    case CAIRO_EXTEND_REPEAT:
	xstep = pattern_width;
	ystep = pattern_height;
	break;
    case CAIRO_EXTEND_REFLECT:
	bbox_x = pattern_width*2;
	bbox_y = pattern_height*2;
	xstep = bbox_x;
	ystep = bbox_y;
	break;
	/* All the rest (if any) should have been analyzed away, so this
	 * case should be unreachable. */
    default:
	ASSERT_NOT_REACHED;
	xstep = 0;
	ystep = 0;
    }

    /* At this point, (that is, within the surface backend interface),
     * the pattern's matrix maps from cairo's device space to cairo's
     * pattern space, (both with their origin at the upper-left, and
     * cairo's pattern space of size width,height).
     *
     * Then, we must emit a PDF pattern object that maps from its own
     * pattern space, (which has a size that we establish in the BBox
     * dictionary entry), to the PDF page's *initial* space, (which
     * does not benefit from the Y-axis flipping matrix that we emit
     * on each page). So the PDF patterns matrix maps from a
     * (width,height) pattern space to a device space with the origin
     * in the lower-left corner.
     *
     * So to handle all of that, we start with an identity matrix for
     * the PDF pattern to device matrix. We translate it up by the
     * image height then flip it in the Y direction, (moving us from
     * the PDF origin to cairo's origin). We then multiply in the
     * inverse of the cairo pattern matrix, (since it maps from device
     * to pattern, while we're setting up pattern to device). Finally,
     * we translate back down by the image height and flip again to
     * end up at the lower-left origin that PDF expects.
     *
     * Additionally, within the stream that paints the pattern itself,
     * we are using a PDF image object that has a size of (1,1) so we
     * have to scale it up by the image width and height to fill our
     * pattern cell.
     */
    cairo_p2d = pattern->base.matrix;
    status = cairo_matrix_invert (&cairo_p2d);
    /* cairo_pattern_set_matrix ensures the matrix is invertible */
    assert (status == CAIRO_STATUS_SUCCESS);

    cairo_matrix_init_identity (&pdf_p2d);
    cairo_matrix_translate (&pdf_p2d, 0.0, surface_extents.height);
    cairo_matrix_scale (&pdf_p2d, 1.0, -1.0);
    cairo_matrix_multiply (&pdf_p2d, &cairo_p2d, &pdf_p2d);
    cairo_matrix_translate (&pdf_p2d, 0.0, pattern_height);
    cairo_matrix_scale (&pdf_p2d, 1.0, -1.0);

    _cairo_pdf_surface_update_object (surface, pdf_pattern->pattern_res);
    status = _cairo_pdf_surface_open_stream (surface,
				             &pdf_pattern->pattern_res,
					     FALSE,
					     "   /PatternType 1\r\n"
					     "   /BBox [0 0 %d %d]\r\n"
					     "   /XStep %f\r\n"
					     "   /YStep %f\r\n"
					     "   /TilingType 1\r\n"
					     "   /PaintType 1\r\n"
					     "   /Matrix [ %f %f %f %f %f %f ]\r\n"
					     "   /Resources << /XObject << /x%d %d 0 R >> >>\r\n",
					     bbox_x, bbox_y,
					     xstep, ystep,
					     pdf_p2d.xx, pdf_p2d.yx,
					     pdf_p2d.xy, pdf_p2d.yy,
					     pdf_p2d.x0, pdf_p2d.y0,
					     pattern_resource.id,
					     pattern_resource.id);
    if (status)
	return status;

    if (_cairo_surface_is_meta (pattern->surface)) {
	snprintf(draw_surface,
		 sizeof (draw_surface),
		 "/x%d Do\r\n",
		 pattern_resource.id);
    } else {
	snprintf(draw_surface,
		 sizeof (draw_surface),
		 "q %d 0 0 %d 0 0 cm /x%d Do Q",
		 pattern_width,
		 pattern_height,
		 pattern_resource.id);
    }

    if (extend == CAIRO_EXTEND_REFLECT) {
	_cairo_output_stream_printf (surface->output,
				     "q 0 0 %d %d re W n %s Q\r\n"
				     "q -1 0 0 1 %d 0 cm 0 0 %d %d re W n %s Q\r\n"
				     "q 1 0 0 -1 0 %d cm 0 0 %d %d re W n %s Q\r\n"
				     "q -1 0 0 -1 %d %d cm 0 0 %d %d re W n %s Q\r\n",
				     pattern_width, pattern_height,
				     draw_surface,
				     pattern_width*2, pattern_width, pattern_height,
				     draw_surface,
				     pattern_height*2, pattern_width, pattern_height,
				     draw_surface,
				     pattern_width*2, pattern_height*2, pattern_width, pattern_height,
				     draw_surface);
    } else {
	_cairo_output_stream_printf (surface->output,
				     " %s \r\n",
				     draw_surface);
    }

    status = _cairo_pdf_surface_close_stream (surface);
    if (status)
	return status;

    return _cairo_output_stream_get_status (surface->output);
}

typedef struct _cairo_pdf_color_stop {
    double offset;
    double color[4];
    cairo_pdf_resource_t resource;
} cairo_pdf_color_stop_t;

static cairo_status_t
cairo_pdf_surface_emit_rgb_linear_function (cairo_pdf_surface_t    *surface,
                                            cairo_pdf_color_stop_t *stop1,
                                            cairo_pdf_color_stop_t *stop2,
                                            cairo_pdf_resource_t   *function)
{
    int num_elems, i;
    cairo_pdf_rgb_linear_function_t elem;
    cairo_pdf_resource_t res;
    cairo_status_t status;

    num_elems = _cairo_array_num_elements (&surface->rgb_linear_functions);
    for (i = 0; i < num_elems; i++) {
	_cairo_array_copy_element (&surface->rgb_linear_functions, i, &elem);
        if (memcmp (&elem.color1[0], &stop1->color[0], sizeof (double)*3) != 0)
            continue;
        if (memcmp (&elem.color2[0], &stop2->color[0], sizeof (double)*3) != 0)
            continue;
        *function =  elem.resource;
        return CAIRO_STATUS_SUCCESS;
    }

    res = _cairo_pdf_surface_new_object (surface);
    if (res.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /FunctionType 2\r\n"
				 "   /Domain [ 0 1 ]\r\n"
				 "   /C0 [ %f %f %f ]\r\n"
				 "   /C1 [ %f %f %f ]\r\n"
				 "   /N 1\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 res.id,
                                 stop1->color[0],
                                 stop1->color[1],
                                 stop1->color[2],
                                 stop2->color[0],
                                 stop2->color[1],
                                 stop2->color[2]);

    elem.resource = res;
    memcpy (&elem.color1[0], &stop1->color[0], sizeof (double)*3);
    memcpy (&elem.color2[0], &stop2->color[0], sizeof (double)*3);

    status = _cairo_array_append (&surface->rgb_linear_functions, &elem);
    *function = res;

    return status;
}

static cairo_status_t
cairo_pdf_surface_emit_alpha_linear_function (cairo_pdf_surface_t    *surface,
                                              cairo_pdf_color_stop_t *stop1,
                                              cairo_pdf_color_stop_t *stop2,
                                              cairo_pdf_resource_t   *function)
{
    int num_elems, i;
    cairo_pdf_alpha_linear_function_t elem;
    cairo_pdf_resource_t res;
    cairo_status_t status;

    num_elems = _cairo_array_num_elements (&surface->alpha_linear_functions);
    for (i = 0; i < num_elems; i++) {
	_cairo_array_copy_element (&surface->alpha_linear_functions, i, &elem);
        if (elem.alpha1 != stop1->color[3])
            continue;
        if (elem.alpha2 != stop2->color[3])
            continue;
        *function =  elem.resource;
        return CAIRO_STATUS_SUCCESS;
    }

    res = _cairo_pdf_surface_new_object (surface);
    if (res.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /FunctionType 2\r\n"
				 "   /Domain [ 0 1 ]\r\n"
				 "   /C0 [ %f ]\r\n"
				 "   /C1 [ %f ]\r\n"
				 "   /N 1\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 res.id,
                                 stop1->color[3],
                                 stop2->color[3]);

    elem.resource = res;
    elem.alpha1 = stop1->color[3];
    elem.alpha2 = stop2->color[3];

    status = _cairo_array_append (&surface->alpha_linear_functions, &elem);
    *function = res;

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_stitched_colorgradient (cairo_pdf_surface_t    *surface,
                                                unsigned int	        n_stops,
                                                cairo_pdf_color_stop_t *stops,
                                                cairo_bool_t	        is_alpha,
                                                cairo_pdf_resource_t   *function)
{
    cairo_pdf_resource_t res;
    unsigned int i;
    cairo_status_t status;

    /* emit linear gradients between pairs of subsequent stops... */
    for (i = 0; i < n_stops-1; i++) {
        if (is_alpha) {
            status = cairo_pdf_surface_emit_alpha_linear_function (surface,
                                                                   &stops[i],
                                                                   &stops[i+1],
                                                                   &stops[i].resource);
            if (status)
                return status;
        } else {
            status = cairo_pdf_surface_emit_rgb_linear_function (surface,
                                                                 &stops[i],
                                                                 &stops[i+1],
                                                                 &stops[i].resource);
            if (status)
                return status;
        }
    }

    /* ... and stitch them together */
    res = _cairo_pdf_surface_new_object (surface);
    if (res.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /FunctionType 3\r\n"
				 "   /Domain [ %f %f ]\r\n",
				 res.id,
                                 stops[0].offset,
                                 stops[n_stops - 1].offset);

    _cairo_output_stream_printf (surface->output,
				 "   /Functions [ ");
    for (i = 0; i < n_stops-1; i++)
        _cairo_output_stream_printf (surface->output,
                                     "%d 0 R ", stops[i].resource.id);
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");

    _cairo_output_stream_printf (surface->output,
				 "   /Bounds [ ");
    for (i = 1; i < n_stops-1; i++)
        _cairo_output_stream_printf (surface->output,
				     "%f ", stops[i].offset);
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");

    _cairo_output_stream_printf (surface->output,
				 "   /Encode [ ");
    for (i = 1; i < n_stops; i++)
        _cairo_output_stream_printf (surface->output,
				     "0 1 ");
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    *function = res;

    return _cairo_output_stream_get_status (surface->output);
}


static void
calc_gradient_color (cairo_pdf_color_stop_t *new_stop,
		     cairo_pdf_color_stop_t *stop1,
		     cairo_pdf_color_stop_t *stop2)
{
    int i;
    double offset = stop1->offset / (stop1->offset + 1.0 - stop2->offset);

    for (i = 0; i < 4; i++)
	new_stop->color[i] = stop1->color[i] + offset*(stop2->color[i] - stop1->color[i]);
}

#define COLOR_STOP_EPSILON 1e-6

static cairo_status_t
_cairo_pdf_surface_emit_pattern_stops (cairo_pdf_surface_t      *surface,
                                       cairo_gradient_pattern_t *pattern,
                                       cairo_pdf_resource_t     *color_function,
                                       cairo_pdf_resource_t     *alpha_function)
{
    cairo_pdf_color_stop_t *allstops, *stops;
    unsigned int n_stops;
    unsigned int i;
    cairo_bool_t emit_alpha = FALSE;
    cairo_status_t status;

    color_function->id = 0;
    alpha_function->id = 0;

    allstops = _cairo_malloc_ab ((pattern->n_stops + 2), sizeof (cairo_pdf_color_stop_t));
    if (allstops == NULL)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    stops = &allstops[1];
    n_stops = pattern->n_stops;

    for (i = 0; i < n_stops; i++) {
	stops[i].color[0] = pattern->stops[i].color.red;
	stops[i].color[1] = pattern->stops[i].color.green;
	stops[i].color[2] = pattern->stops[i].color.blue;
	stops[i].color[3] = pattern->stops[i].color.alpha;
        if (!CAIRO_ALPHA_IS_OPAQUE (stops[i].color[3]))
            emit_alpha = TRUE;
	stops[i].offset = _cairo_fixed_to_double (pattern->stops[i].x);
    }

    if (pattern->base.extend == CAIRO_EXTEND_REPEAT ||
	pattern->base.extend == CAIRO_EXTEND_REFLECT) {
	if (stops[0].offset > COLOR_STOP_EPSILON) {
	    if (pattern->base.extend == CAIRO_EXTEND_REFLECT)
		memcpy (allstops, stops, sizeof (cairo_pdf_color_stop_t));
	    else
		calc_gradient_color (&allstops[0], &stops[0], &stops[n_stops-1]);
	    stops = allstops;
	    n_stops++;
	}
	stops[0].offset = 0.0;

	if (stops[n_stops-1].offset < 1.0 - COLOR_STOP_EPSILON) {
	    if (pattern->base.extend == CAIRO_EXTEND_REFLECT) {
		memcpy (&stops[n_stops],
			&stops[n_stops - 1],
			sizeof (cairo_pdf_color_stop_t));
	    } else {
		calc_gradient_color (&stops[n_stops], &stops[0], &stops[n_stops-1]);
	    }
	    n_stops++;
	}
	stops[n_stops-1].offset = 1.0;
    }

    if (n_stops == 2) {
        /* no need for stitched function */
        status = cairo_pdf_surface_emit_rgb_linear_function (surface,
                                                             &stops[0],
                                                             &stops[1],
                                                             color_function);
        if (status)
            goto BAIL;

        if (emit_alpha) {
            status = cairo_pdf_surface_emit_alpha_linear_function (surface,
                                                                   &stops[0],
                                                                   &stops[1],
                                                                   alpha_function);
            if (status)
                goto BAIL;
        }
    } else {
        /* multiple stops: stitch. XXX possible optimization: regularly spaced
         * stops do not require stitching. XXX */
        status = _cairo_pdf_surface_emit_stitched_colorgradient (surface,
                                                                 n_stops,
                                                                 stops,
                                                                 FALSE,
                                                                 color_function);
        if (status)
            goto BAIL;

        if (emit_alpha) {
            status = _cairo_pdf_surface_emit_stitched_colorgradient (surface,
                                                                     n_stops,
                                                                     stops,
                                                                     TRUE,
                                                                     alpha_function);
            if (status)
                goto BAIL;
        }
    }

BAIL:
    free (allstops);
    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_repeating_function (cairo_pdf_surface_t      *surface,
					    cairo_gradient_pattern_t *pattern,
					    cairo_pdf_resource_t     *function,
					    int                       begin,
					    int                       end)
{
    cairo_pdf_resource_t res;
    int i;

    res = _cairo_pdf_surface_new_object (surface);
    if (res.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /FunctionType 3\r\n"
				 "   /Domain [ %d %d ]\r\n",
				 res.id,
                                 begin,
                                 end);

    _cairo_output_stream_printf (surface->output,
				 "   /Functions [ ");
    for (i = begin; i < end; i++)
        _cairo_output_stream_printf (surface->output,
                                     "%d 0 R ", function->id);
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");

    _cairo_output_stream_printf (surface->output,
				 "   /Bounds [ ");
    for (i = begin + 1; i < end; i++)
        _cairo_output_stream_printf (surface->output,
				     "%d ", i);
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");

    _cairo_output_stream_printf (surface->output,
				 "   /Encode [ ");
    for (i = begin; i < end; i++) {
	if ((i % 2) && pattern->base.extend == CAIRO_EXTEND_REFLECT) {
	    _cairo_output_stream_printf (surface->output,
					 "1 0 ");
	} else {
	    _cairo_output_stream_printf (surface->output,
					 "0 1 ");
	}
    }
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    *function = res;

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
cairo_pdf_surface_emit_transparency_group (cairo_pdf_surface_t  *surface,
					   cairo_pdf_resource_t  gstate_resource,
					   cairo_pdf_resource_t  gradient_mask)
{
    cairo_pdf_resource_t smask_resource;
    cairo_status_t status;

    status = _cairo_pdf_surface_open_stream (surface,
					     NULL,
					     surface->compress_content,
					     "   /Type /XObject\r\n"
					     "   /Subtype /Form\r\n"
					     "   /FormType 1\r\n"
					     "   /BBox [ 0 0 %f %f ]\r\n"
					     "   /Resources\r\n"
					     "      << /ExtGState\r\n"
					     "            << /a0 << /ca 1 /CA 1 >>"
					     "      >>\r\n"
					     "         /Pattern\r\n"
					     "            << /p%d %d 0 R >>\r\n"
					     "      >>\r\n"
					     "   /Group\r\n"
					     "      << /Type /Group\r\n"
					     "         /S /Transparency\r\n"
					     "         /CS /DeviceGray\r\n"
					     "      >>\r\n",
					     surface->width,
					     surface->height,
					     gradient_mask.id,
					     gradient_mask.id);
    if (status)
	return status;

    _cairo_output_stream_printf (surface->output,
                                 "q\r\n"
                                 "/a0 gs\r\n"
                                 "/Pattern cs /p%d scn\r\n"
                                 "0 0 %f %f re\r\n"
                                 "f\r\n"
                                 "Q\r\n",
                                 gradient_mask.id,
                                 surface->width,
                                 surface->height);

     status = _cairo_pdf_surface_close_stream (surface);
     if (status)
	return status;

    smask_resource = _cairo_pdf_surface_new_object (surface);
    if (smask_resource.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
                                 "%d 0 obj\r\n"
                                 "<< /Type /Mask\r\n"
                                 "   /S /Luminosity\r\n"
                                 "   /G %d 0 R\r\n"
                                 ">>\r\n"
                                 "endobj\r\n",
                                 smask_resource.id,
                                 surface->pdf_stream.self.id);

    /* Create GState which uses the transparency group as an SMask. */
    _cairo_pdf_surface_update_object (surface, gstate_resource);

    _cairo_output_stream_printf (surface->output,
                                 "%d 0 obj\r\n"
                                 "<< /Type /ExtGState\r\n"
                                 "   /SMask %d 0 R\r\n"
                                 "   /ca 1\r\n"
                                 "   /CA 1\r\n"
                                 "   /AIS false\r\n"
                                 ">>\r\n"
                                 "endobj\r\n",
                                 gstate_resource.id,
                                 smask_resource.id);

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
_cairo_pdf_surface_emit_linear_pattern (cairo_pdf_surface_t    *surface,
					cairo_pdf_pattern_t    *pdf_pattern)
{
    cairo_linear_pattern_t *pattern = (cairo_linear_pattern_t *) pdf_pattern->pattern;
    cairo_pdf_resource_t color_function, alpha_function;
    double x1, y1, x2, y2;
    double _x1, _y1, _x2, _y2;
    cairo_matrix_t pat_to_pdf;
    cairo_extend_t extend;
    cairo_status_t status;
    cairo_gradient_pattern_t *gradient = &pattern->base;
    double first_stop, last_stop;
    int repeat_begin = 0, repeat_end = 1;

    assert (pattern->base.n_stops != 0);

    extend = cairo_pattern_get_extend (pdf_pattern->pattern);

    pat_to_pdf = pattern->base.base.matrix;
    status = cairo_matrix_invert (&pat_to_pdf);
    /* cairo_pattern_set_matrix ensures the matrix is invertible */
    assert (status == CAIRO_STATUS_SUCCESS);

    cairo_matrix_multiply (&pat_to_pdf, &pat_to_pdf, &surface->cairo_to_pdf);
    first_stop = _cairo_fixed_to_double (gradient->stops[0].x);
    last_stop = _cairo_fixed_to_double (gradient->stops[gradient->n_stops - 1].x);

    if (pattern->base.base.extend == CAIRO_EXTEND_REPEAT ||
	pattern->base.base.extend == CAIRO_EXTEND_REFLECT) {
	double dx, dy;
	int x_rep = 0, y_rep = 0;

	x1 = _cairo_fixed_to_double (pattern->p1.x);
	y1 = _cairo_fixed_to_double (pattern->p1.y);
	cairo_matrix_transform_point (&pat_to_pdf, &x1, &y1);

	x2 = _cairo_fixed_to_double (pattern->p2.x);
	y2 = _cairo_fixed_to_double (pattern->p2.y);
	cairo_matrix_transform_point (&pat_to_pdf, &x2, &y2);

	dx = fabs (x2 - x1);
	dy = fabs (y2 - y1);
	if (dx > 1e-6)
	    x_rep = (int) ceil (surface->width/dx);
	if (dy > 1e-6)
	    y_rep = (int) ceil (surface->height/dy);

	repeat_end = MAX (x_rep, y_rep);
	repeat_begin = -repeat_end;
	first_stop = repeat_begin;
	last_stop = repeat_end;
    }

    /* PDF requires the first and last stop to be the same as the line
     * coordinates. For repeating patterns this moves the line
     * coordinates out to the begin/end of the repeating function. For
     * non repeating patterns this may move the line coordinates in if
     * there are not stops at offset 0 and 1. */
    x1 = _cairo_fixed_to_double (pattern->p1.x);
    y1 = _cairo_fixed_to_double (pattern->p1.y);
    x2 = _cairo_fixed_to_double (pattern->p2.x);
    y2 = _cairo_fixed_to_double (pattern->p2.y);

    _x1 = x1 + (x2 - x1)*first_stop;
    _y1 = y1 + (y2 - y1)*first_stop;
    _x2 = x1 + (x2 - x1)*last_stop;
    _y2 = y1 + (y2 - y1)*last_stop;

    x1 = _x1;
    x2 = _x2;
    y1 = _y1;
    y2 = _y2;

    /* For EXTEND_NONE and EXTEND_PAD if there are only two stops a
     * Type 2 function is used by itself without a stitching
     * function. Type 2 functions always have the domain [0 1] */
    if ((pattern->base.base.extend == CAIRO_EXTEND_NONE ||
	 pattern->base.base.extend == CAIRO_EXTEND_PAD) &&
	gradient->n_stops == 2) {
	first_stop = 0.0;
	last_stop = 1.0;
    }

    status = _cairo_pdf_surface_emit_pattern_stops (surface,
                                                    &pattern->base,
                                                    &color_function,
                                                    &alpha_function);
    if (status)
	return status;

    if (pattern->base.base.extend == CAIRO_EXTEND_REPEAT ||
	pattern->base.base.extend == CAIRO_EXTEND_REFLECT) {
	status = _cairo_pdf_surface_emit_repeating_function (surface,
							     &pattern->base,
							     &color_function,
							     repeat_begin,
							     repeat_end);
	if (status)
	    return status;

	if (alpha_function.id != 0) {
	    status = _cairo_pdf_surface_emit_repeating_function (surface,
								 &pattern->base,
								 &alpha_function,
								 repeat_begin,
								 repeat_end);
	    if (status)
		return status;
	}
    }

    _cairo_pdf_surface_update_object (surface, pdf_pattern->pattern_res);
    _cairo_output_stream_printf (surface->output,
                                 "%d 0 obj\r\n"
                                 "<< /Type /Pattern\r\n"
                                 "   /PatternType 2\r\n"
                                 "   /Matrix [ %f %f %f %f %f %f ]\r\n"
                                 "   /Shading\r\n"
                                 "      << /ShadingType 2\r\n"
                                 "         /ColorSpace /DeviceRGB\r\n"
                                 "         /Coords [ %f %f %f %f ]\r\n"
                                 "         /Domain [ %f %f ]\r\n"
                                 "         /Function %d 0 R\r\n",
				 pdf_pattern->pattern_res.id,
                                 pat_to_pdf.xx, pat_to_pdf.yx,
                                 pat_to_pdf.xy, pat_to_pdf.yy,
                                 pat_to_pdf.x0, pat_to_pdf.y0,
                                 x1, y1, x2, y2,
                                 first_stop, last_stop,
                                 color_function.id);

    if (extend == CAIRO_EXTEND_PAD) {
        _cairo_output_stream_printf (surface->output,
                                     "         /Extend [ true true ]\r\n");
    } else {
        _cairo_output_stream_printf (surface->output,
                                     "         /Extend [ false false ]\r\n");
    }

    _cairo_output_stream_printf (surface->output,
                                 "      >>\r\n"
                                 ">>\r\n"
                                 "endobj\r\n");

    if (alpha_function.id != 0) {
	cairo_pdf_resource_t mask_resource;

	assert (pdf_pattern->gstate_res.id != 0);

        /* Create pattern for SMask. */
        mask_resource = _cairo_pdf_surface_new_object (surface);
	if (mask_resource.id == 0)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

        _cairo_output_stream_printf (surface->output,
                                     "%d 0 obj\r\n"
                                     "<< /Type /Pattern\r\n"
                                     "   /PatternType 2\r\n"
                                     "   /Matrix [ %f %f %f %f %f %f ]\r\n"
                                     "   /Shading\r\n"
                                     "      << /ShadingType 2\r\n"
                                     "         /ColorSpace /DeviceGray\r\n"
                                     "         /Coords [ %f %f %f %f ]\r\n"
				     "         /Domain [ %f %f ]\r\n"
                                     "         /Function %d 0 R\r\n",
                                     mask_resource.id,
                                     pat_to_pdf.xx, pat_to_pdf.yx,
                                     pat_to_pdf.xy, pat_to_pdf.yy,
                                     pat_to_pdf.x0, pat_to_pdf.y0,
                                     x1, y1, x2, y2,
				     first_stop, last_stop,
                                     alpha_function.id);

        if (extend == CAIRO_EXTEND_PAD) {
            _cairo_output_stream_printf (surface->output,
                                         "         /Extend [ true true ]\r\n");
        } else {
            _cairo_output_stream_printf (surface->output,
                                         "         /Extend [ false false ]\r\n");
        }

        _cairo_output_stream_printf (surface->output,
                                     "      >>\r\n"
                                     ">>\r\n"
                                     "endobj\r\n");
        status = _cairo_pdf_surface_add_pattern (surface, mask_resource);
        if (status)
            return status;

	status = cairo_pdf_surface_emit_transparency_group (surface,
						            pdf_pattern->gstate_res,
							    mask_resource);
	if (status)
	    return status;
    }

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
_cairo_pdf_surface_emit_radial_pattern (cairo_pdf_surface_t    *surface,
					cairo_pdf_pattern_t    *pdf_pattern)
{
    cairo_pdf_resource_t color_function, alpha_function;
    double x1, y1, x2, y2, r1, r2;
    cairo_matrix_t pat_to_pdf;
    cairo_extend_t extend;
    cairo_status_t status;
    cairo_radial_pattern_t *pattern = (cairo_radial_pattern_t *) pdf_pattern->pattern;

    assert (pattern->base.n_stops != 0);

    extend = cairo_pattern_get_extend (pdf_pattern->pattern);

    status = _cairo_pdf_surface_emit_pattern_stops (surface,
                                                    &pattern->base,
                                                    &color_function,
                                                    &alpha_function);
    if (status)
	return status;

    pat_to_pdf = pattern->base.base.matrix;
    status = cairo_matrix_invert (&pat_to_pdf);
    /* cairo_pattern_set_matrix ensures the matrix is invertible */
    assert (status == CAIRO_STATUS_SUCCESS);

    cairo_matrix_multiply (&pat_to_pdf, &pat_to_pdf, &surface->cairo_to_pdf);
    x1 = _cairo_fixed_to_double (pattern->c1.x);
    y1 = _cairo_fixed_to_double (pattern->c1.y);
    r1 = _cairo_fixed_to_double (pattern->r1);
    x2 = _cairo_fixed_to_double (pattern->c2.x);
    y2 = _cairo_fixed_to_double (pattern->c2.y);
    r2 = _cairo_fixed_to_double (pattern->r2);

    _cairo_pdf_surface_update_object (surface, pdf_pattern->pattern_res);

    _cairo_output_stream_printf (surface->output,
                                 "%d 0 obj\r\n"
                                 "<< /Type /Pattern\r\n"
                                 "   /PatternType 2\r\n"
                                 "   /Matrix [ %f %f %f %f %f %f ]\r\n"
                                 "   /Shading\r\n"
                                 "      << /ShadingType 3\r\n"
                                 "         /ColorSpace /DeviceRGB\r\n"
                                 "         /Coords [ %f %f %f %f %f %f ]\r\n"
                                 "         /Function %d 0 R\r\n",
				 pdf_pattern->pattern_res.id,
                                 pat_to_pdf.xx, pat_to_pdf.yx,
                                 pat_to_pdf.xy, pat_to_pdf.yy,
                                 pat_to_pdf.x0, pat_to_pdf.y0,
                                 x1, y1, r1, x2, y2, r2,
                                 color_function.id);

    if (extend == CAIRO_EXTEND_PAD) {
        _cairo_output_stream_printf (surface->output,
                                     "         /Extend [ true true ]\r\n");
    } else {
        _cairo_output_stream_printf (surface->output,
                                     "         /Extend [ false false ]\r\n");
    }

    _cairo_output_stream_printf (surface->output,
                                 "      >>\r\n"
                                 ">>\r\n"
                                 "endobj\r\n");

    if (alpha_function.id != 0) {
	cairo_pdf_resource_t mask_resource;

	assert (pdf_pattern->gstate_res.id != 0);

	/* Create pattern for SMask. */
        mask_resource = _cairo_pdf_surface_new_object (surface);
	if (mask_resource.id == 0)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

        _cairo_output_stream_printf (surface->output,
                                     "%d 0 obj\r\n"
                                     "<< /Type /Pattern\r\n"
                                     "   /PatternType 2\r\n"
                                     "   /Matrix [ %f %f %f %f %f %f ]\r\n"
                                     "   /Shading\r\n"
                                     "      << /ShadingType 3\r\n"
                                     "         /ColorSpace /DeviceGray\r\n"
                                     "         /Coords [ %f %f %f %f %f %f ]\r\n"
                                     "         /Function %d 0 R\r\n",
                                     mask_resource.id,
                                     pat_to_pdf.xx, pat_to_pdf.yx,
                                     pat_to_pdf.xy, pat_to_pdf.yy,
                                     pat_to_pdf.x0, pat_to_pdf.y0,
                                     x1, y1, r1, x2, y2, r2,
                                     alpha_function.id);

        if (extend == CAIRO_EXTEND_PAD) {
            _cairo_output_stream_printf (surface->output,
                                         "         /Extend [ true true ]\r\n");
        } else {
            _cairo_output_stream_printf (surface->output,
                                         "         /Extend [ false false ]\r\n");
        }

        _cairo_output_stream_printf (surface->output,
                                     "      >>\r\n"
                                     ">>\r\n"
                                     "endobj\r\n");

	status = cairo_pdf_surface_emit_transparency_group (surface,
						            pdf_pattern->gstate_res,
							    mask_resource);
	if (status)
	    return status;
    }

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
_cairo_pdf_surface_emit_pattern (cairo_pdf_surface_t *surface, cairo_pdf_pattern_t *pdf_pattern)
{
    switch (pdf_pattern->pattern->type) {
    case CAIRO_PATTERN_TYPE_SOLID:
	ASSERT_NOT_REACHED;
	break;

    case CAIRO_PATTERN_TYPE_SURFACE:
	return _cairo_pdf_surface_emit_surface_pattern (surface, pdf_pattern);

    case CAIRO_PATTERN_TYPE_LINEAR:
	return _cairo_pdf_surface_emit_linear_pattern (surface, pdf_pattern);

    case CAIRO_PATTERN_TYPE_RADIAL:
	return _cairo_pdf_surface_emit_radial_pattern (surface, pdf_pattern);
    }

    ASSERT_NOT_REACHED;
    return _cairo_error (CAIRO_STATUS_PATTERN_TYPE_MISMATCH);
}

static cairo_status_t
_cairo_pdf_surface_select_pattern (cairo_pdf_surface_t *surface,
				   cairo_pattern_t     *pattern,
				   cairo_pdf_resource_t pattern_res,
				   cairo_bool_t         is_stroke)
{
    cairo_status_t status;
    int alpha;

    if (pattern->type == CAIRO_PATTERN_TYPE_SOLID) {
	cairo_solid_pattern_t *solid_pattern = (cairo_solid_pattern_t *) pattern;

	status = _cairo_pdf_surface_add_alpha (surface, solid_pattern->color.alpha, &alpha);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q %f %f %f ",
				     solid_pattern->color.red,
				     solid_pattern->color.green,
				     solid_pattern->color.blue);

	if (is_stroke)
            _cairo_output_stream_printf (surface->output, "RG ");
        else
            _cairo_output_stream_printf (surface->output, "rg ");

        _cairo_output_stream_printf (surface->output,
                                     "/a%d gs\r\n",
                                     alpha);
    } else {
	status = _cairo_pdf_surface_add_alpha (surface, 1.0, &alpha);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_pattern (surface, pattern_res);
	if (status)
	    return status;

	if (is_stroke) {
	    _cairo_output_stream_printf (surface->output,
					 "q /Pattern CS /p%d SCN ",
					 pattern_res.id);
	} else {
	    _cairo_output_stream_printf (surface->output,
					 "q /Pattern cs /p%d scn ",
					 pattern_res.id);
	}
	_cairo_output_stream_printf (surface->output,
				     "/a%d gs\r\n",
				     alpha);
    }

    return _cairo_output_stream_get_status (surface->output);
}

static void
_cairo_pdf_surface_unselect_pattern (cairo_pdf_surface_t *surface)
{
    _cairo_output_stream_printf (surface->output, "Q\r\n");
}

static cairo_int_status_t
_cairo_pdf_surface_show_page (void *abstract_surface)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    cairo_int_status_t status;

    status = _cairo_pdf_surface_close_content_stream (surface);
    if (status)
	return status;

    status = _cairo_pdf_surface_write_page (surface);
    if (status)
	return status;

    _cairo_pdf_surface_clear (surface);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_int_status_t
_cairo_pdf_surface_get_extents (void		        *abstract_surface,
				cairo_rectangle_int_t   *rectangle)
{
    cairo_pdf_surface_t *surface = abstract_surface;

    rectangle->x = 0;
    rectangle->y = 0;

    /* XXX: The conversion to integers here is pretty bogus, (not to
     * mention the arbitrary limitation of width to a short(!). We
     * may need to come up with a better interface for get_size.
     */
    rectangle->width  = (int) ceil (surface->width);
    rectangle->height = (int) ceil (surface->height);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_int_status_t
_cairo_pdf_surface_intersect_clip_path (void			*abstract_surface,
					cairo_path_fixed_t	*path,
					cairo_fill_rule_t	fill_rule,
					double			tolerance,
					cairo_antialias_t	antialias)
{
    cairo_pdf_surface_t *surface = abstract_surface;

    if (path == NULL) {
	_cairo_output_stream_printf (surface->output, "Q q\r\n");
	return CAIRO_STATUS_SUCCESS;
    }

    return _cairo_pdf_operators_clip (&surface->pdf_operators, path, fill_rule);
}

static void
_cairo_pdf_surface_get_font_options (void                  *abstract_surface,
				     cairo_font_options_t  *options)
{
    _cairo_font_options_init_default (options);

    cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_OFF);
    cairo_font_options_set_antialias (options, CAIRO_ANTIALIAS_GRAY);
}

static cairo_pdf_resource_t
_cairo_pdf_surface_write_info (cairo_pdf_surface_t *surface)
{
    cairo_pdf_resource_t info;

    info = _cairo_pdf_surface_new_object (surface);
    if (info.id == 0)
	return info;

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Creator (cairo %s (http://cairographics.org))\r\n"
				 "   /Producer (cairo %s (http://cairographics.org))\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 info.id,
                                 cairo_version_string (),
                                 cairo_version_string ());

    return info;
}

static void
_cairo_pdf_surface_write_pages (cairo_pdf_surface_t *surface)
{
    cairo_pdf_resource_t page;
    int num_pages, i;

    _cairo_pdf_surface_update_object (surface, surface->pages_resource);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Pages\r\n"
				 "   /Kids [ ",
				 surface->pages_resource.id);

    num_pages = _cairo_array_num_elements (&surface->pages);
    for (i = 0; i < num_pages; i++) {
	_cairo_array_copy_element (&surface->pages, i, &page);
	_cairo_output_stream_printf (surface->output, "%d 0 R ", page.id);
    }

    _cairo_output_stream_printf (surface->output, "]\r\n");
    _cairo_output_stream_printf (surface->output, "   /Count %d\r\n", num_pages);


    /* TODO: Figure out which other defaults to be inherited by /Page
     * objects. */
    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");
}

static cairo_int_status_t
_cairo_pdf_surface_emit_to_unicode_stream (cairo_pdf_surface_t		*surface,
					   cairo_scaled_font_subset_t	*font_subset,
                                           cairo_bool_t                  is_composite,
					   cairo_pdf_resource_t         *stream)
{
    const cairo_scaled_font_backend_t *backend;
    unsigned int i, num_bfchar;
    cairo_int_status_t status;

    stream->id = 0;
    if (font_subset->to_unicode == NULL)
	return CAIRO_INT_STATUS_UNSUPPORTED;

    status = _cairo_truetype_create_glyph_to_unicode_map (font_subset);
    if (status) {
	if (status != CAIRO_INT_STATUS_UNSUPPORTED)
	    return status;

        backend = font_subset->scaled_font->backend;
        if (backend->map_glyphs_to_unicode == NULL)
	    return CAIRO_INT_STATUS_UNSUPPORTED;

        status = backend->map_glyphs_to_unicode (font_subset->scaled_font,
		                                 font_subset);
	if (status)
	    return status;
    }

    status = _cairo_pdf_surface_open_stream (surface,
					      NULL,
					      surface->compress_content,
					      NULL);
    if (status)
	return status;

    _cairo_output_stream_printf (surface->output,
                                 "/CIDInit /ProcSet findresource begin\r\n"
                                 "12 dict begin\r\n"
                                 "begincmap\r\n"
                                 "/CIDSystemInfo\r\n"
                                 "<< /Registry (Adobe)\r\n"
                                 "   /Ordering (UCS)\r\n"
                                 "   /Supplement 0\r\n"
                                 ">> def\r\n"
                                 "/CMapName /Adobe-Identity-UCS def\r\n"
                                 "/CMapType 2 def\r\n"
                                 "1 begincodespacerange\r\n");

    if (is_composite) {
        _cairo_output_stream_printf (surface->output,
                                     "<0000> <ffff>\r\n");
    } else {
        _cairo_output_stream_printf (surface->output,
                                     "<00> <ff>\r\n");
    }

    _cairo_output_stream_printf (surface->output,
                                  "endcodespacerange\r\n");

    num_bfchar = font_subset->num_glyphs - 1;
    /* The CMap specification has a limit of 100 characters per beginbfchar operator */
    _cairo_output_stream_printf (surface->output,
                                 "%d beginbfchar\r\n",
                                 num_bfchar > 100 ? 100 : num_bfchar);
    for (i = 0; i < num_bfchar; i++) {
        if (i != 0 && i % 100 == 0) {
            _cairo_output_stream_printf (surface->output,
                                         "endbfchar\r\n"
                                         "%d beginbfchar\r\n",
                                         num_bfchar - i > 100 ? 100 : num_bfchar - i);
        }
        if (is_composite) {
            _cairo_output_stream_printf (surface->output,
                                         "<%04x> <%04x>\r\n",
                                         i + 1, font_subset->to_unicode[i + 1]);
        } else {
            _cairo_output_stream_printf (surface->output,
                                         "<%02x> <%04x>\r\n",
                                         i + 1, font_subset->to_unicode[i + 1]);
        }
    }
    _cairo_output_stream_printf (surface->output,
                                 "endbfchar\r\n");

    _cairo_output_stream_printf (surface->output,
                                 "endcmap\r\n"
                                 "CMapName currentdict /CMap defineresource pop\r\n"
                                 "end\r\n"
                                 "end\r\n");

    *stream = surface->pdf_stream.self;
    return _cairo_pdf_surface_close_stream (surface);
}

static cairo_status_t
_cairo_pdf_surface_emit_cff_font (cairo_pdf_surface_t		*surface,
                                  cairo_scaled_font_subset_t	*font_subset,
                                  cairo_cff_subset_t            *subset)
{
    cairo_pdf_resource_t stream, descriptor, cidfont_dict;
    cairo_pdf_resource_t subset_resource, to_unicode_stream;
    cairo_pdf_font_t font;
    unsigned long compressed_length;
    char *compressed;
    unsigned int i;
    cairo_status_t status;

    subset_resource = _cairo_pdf_surface_get_font_resource (surface,
							    font_subset->font_id,
							    font_subset->subset_id);
    if (subset_resource.id == 0)
	return CAIRO_STATUS_SUCCESS;

    compressed = compress_dup (subset->data, subset->data_length, &compressed_length);
    if (compressed == NULL)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    stream = _cairo_pdf_surface_new_object (surface);
    if (stream.id == 0) {
	free (compressed);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Filter /FlateDecode\r\n"
				 "   /Length %lu\r\n"
				 "   /Subtype /CIDFontType0C\r\n"
				 ">>\r\n"
				 "stream\r\n",
				 stream.id,
				 compressed_length);
    _cairo_output_stream_write (surface->output, compressed, compressed_length);
    _cairo_output_stream_printf (surface->output,
				 "\r\n"
				 "endstream\r\n"
				 "endobj\r\n");
    free (compressed);

    status = _cairo_pdf_surface_emit_to_unicode_stream (surface,
	                                                font_subset, TRUE,
							&to_unicode_stream);
    if (status && status != CAIRO_INT_STATUS_UNSUPPORTED)
	return status;

    descriptor = _cairo_pdf_surface_new_object (surface);
    if (descriptor.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /FontDescriptor\r\n"
				 "   /FontName /%s\r\n"
				 "   /Flags 4\r\n"
				 "   /FontBBox [ %ld %ld %ld %ld ]\r\n"
				 "   /ItalicAngle 0\r\n"
				 "   /Ascent %ld\r\n"
				 "   /Descent %ld\r\n"
				 "   /CapHeight 500\r\n"
				 "   /StemV 80\r\n"
				 "   /StemH 80\r\n"
				 "   /FontFile3 %u 0 R\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 descriptor.id,
				 subset->base_font,
				 subset->x_min,
				 subset->y_min,
				 subset->x_max,
				 subset->y_max,
				 subset->ascent,
				 subset->descent,
				 stream.id);

    cidfont_dict = _cairo_pdf_surface_new_object (surface);
    if (cidfont_dict.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
                                 "%d 0 obj\r\n"
                                 "<< /Type /Font\r\n"
                                 "   /Subtype /CIDFontType0\r\n"
                                 "   /BaseFont /%s\r\n"
                                 "   /CIDSystemInfo\r\n"
                                 "   << /Registry (Adobe)\r\n"
                                 "      /Ordering (Identity)\r\n"
                                 "      /Supplement 0\r\n"
                                 "   >>\r\n"
                                 "   /FontDescriptor %d 0 R\r\n"
                                 "   /W [0 [",
                                 cidfont_dict.id,
                                 subset->base_font,
                                 descriptor.id);

    for (i = 0; i < font_subset->num_glyphs; i++)
	_cairo_output_stream_printf (surface->output,
				     " %d",
				     subset->widths[i]);

    _cairo_output_stream_printf (surface->output,
                                 " ]]\r\n"
				 ">>\r\n"
				 "endobj\r\n");

    _cairo_pdf_surface_update_object (surface, subset_resource);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Font\r\n"
				 "   /Subtype /Type0\r\n"
				 "   /BaseFont /%s\r\n"
                                 "   /Encoding /Identity-H\r\n"
				 "   /DescendantFonts [ %d 0 R]\r\n",
				 subset_resource.id,
				 subset->base_font,
				 cidfont_dict.id);

    if (to_unicode_stream.id != 0)
        _cairo_output_stream_printf (surface->output,
                                     "   /ToUnicode %d 0 R\r\n",
                                     to_unicode_stream.id);

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    font.font_id = font_subset->font_id;
    font.subset_id = font_subset->subset_id;
    font.subset_resource = subset_resource;
    status = _cairo_array_append (&surface->fonts, &font);

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_cff_font_subset (cairo_pdf_surface_t	     *surface,
                                         cairo_scaled_font_subset_t  *font_subset)
{
    cairo_status_t status;
    cairo_cff_subset_t subset;
    char name[64];

    snprintf (name, sizeof name, "CairoFont-%d-%d",
              font_subset->font_id, font_subset->subset_id);
    status = _cairo_cff_subset_init (&subset, name, font_subset);
    if (status)
        return status;

    status = _cairo_pdf_surface_emit_cff_font (surface, font_subset, &subset);

    _cairo_cff_subset_fini (&subset);

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_cff_fallback_font (cairo_pdf_surface_t	       *surface,
                                           cairo_scaled_font_subset_t  *font_subset)
{
    cairo_status_t status;
    cairo_cff_subset_t subset;
    char name[64];

    snprintf (name, sizeof name, "CairoFont-%d-%d",
              font_subset->font_id, font_subset->subset_id);
    status = _cairo_cff_fallback_init (&subset, name, font_subset);
    if (status)
        return status;

    status = _cairo_pdf_surface_emit_cff_font (surface, font_subset, &subset);

    _cairo_cff_fallback_fini (&subset);

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_type1_font (cairo_pdf_surface_t		*surface,
                                    cairo_scaled_font_subset_t	*font_subset,
                                    cairo_type1_subset_t        *subset)
{
    cairo_pdf_resource_t stream, descriptor, subset_resource, to_unicode_stream;
    cairo_pdf_font_t font;
    cairo_status_t status;
    unsigned long length, compressed_length;
    char *compressed;
    unsigned int i;

    subset_resource = _cairo_pdf_surface_get_font_resource (surface,
							    font_subset->font_id,
							    font_subset->subset_id);
    if (subset_resource.id == 0)
	return CAIRO_STATUS_SUCCESS;

    /* We ignore the zero-trailer and set Length3 to 0. */
    length = subset->header_length + subset->data_length;
    compressed = compress_dup (subset->data, length, &compressed_length);
    if (compressed == NULL)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    stream = _cairo_pdf_surface_new_object (surface);
    if (stream.id == 0) {
	free (compressed);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Filter /FlateDecode\r\n"
				 "   /Length %lu\r\n"
				 "   /Length1 %lu\r\n"
				 "   /Length2 %lu\r\n"
				 "   /Length3 0\r\n"
				 ">>\r\n"
				 "stream\r\n",
				 stream.id,
				 compressed_length,
				 subset->header_length,
				 subset->data_length);
    _cairo_output_stream_write (surface->output, compressed, compressed_length);
    _cairo_output_stream_printf (surface->output,
				 "\r\n"
				 "endstream\r\n"
				 "endobj\r\n");
    free (compressed);

    status = _cairo_pdf_surface_emit_to_unicode_stream (surface,
	                                                font_subset, FALSE,
							&to_unicode_stream);
    if (status && status != CAIRO_INT_STATUS_UNSUPPORTED)
	return status;

    descriptor = _cairo_pdf_surface_new_object (surface);
    if (descriptor.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /FontDescriptor\r\n"
				 "   /FontName /%s\r\n"
				 "   /Flags 4\r\n"
				 "   /FontBBox [ %ld %ld %ld %ld ]\r\n"
				 "   /ItalicAngle 0\r\n"
				 "   /Ascent %ld\r\n"
				 "   /Descent %ld\r\n"
				 "   /CapHeight 500\r\n"
				 "   /StemV 80\r\n"
				 "   /StemH 80\r\n"
				 "   /FontFile %u 0 R\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 descriptor.id,
				 subset->base_font,
				 subset->x_min,
				 subset->y_min,
				 subset->x_max,
				 subset->y_max,
				 subset->ascent,
				 subset->descent,
				 stream.id);

    _cairo_pdf_surface_update_object (surface, subset_resource);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Font\r\n"
				 "   /Subtype /Type1\r\n"
				 "   /BaseFont /%s\r\n"
				 "   /FirstChar 0\r\n"
				 "   /LastChar %d\r\n"
				 "   /FontDescriptor %d 0 R\r\n"
				 "   /Widths [",
				 subset_resource.id,
				 subset->base_font,
				 font_subset->num_glyphs - 1,
				 descriptor.id);

    for (i = 0; i < font_subset->num_glyphs; i++)
	_cairo_output_stream_printf (surface->output,
				     " %d",
				     subset->widths[i]);

    _cairo_output_stream_printf (surface->output,
				 " ]\r\n");

    if (to_unicode_stream.id != 0)
        _cairo_output_stream_printf (surface->output,
                                     "    /ToUnicode %d 0 R\r\n",
                                     to_unicode_stream.id);

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    font.font_id = font_subset->font_id;
    font.subset_id = font_subset->subset_id;
    font.subset_resource = subset_resource;
    return _cairo_array_append (&surface->fonts, &font);
}

#if CAIRO_HAS_FT_FONT
static cairo_status_t
_cairo_pdf_surface_emit_type1_font_subset (cairo_pdf_surface_t		*surface,
					   cairo_scaled_font_subset_t	*font_subset)
{
    cairo_status_t status;
    cairo_type1_subset_t subset;
    char name[64];

    snprintf (name, sizeof name, "CairoFont-%d-%d",
	      font_subset->font_id, font_subset->subset_id);
    status = _cairo_type1_subset_init (&subset, name, font_subset, FALSE);
    if (status)
	return status;

    status = _cairo_pdf_surface_emit_type1_font (surface, font_subset, &subset);

    _cairo_type1_subset_fini (&subset);
    return status;
}
#endif

static cairo_status_t
_cairo_pdf_surface_emit_type1_fallback_font (cairo_pdf_surface_t	*surface,
                                             cairo_scaled_font_subset_t	*font_subset)
{
    cairo_status_t status;
    cairo_type1_subset_t subset;
    char name[64];

    snprintf (name, sizeof name, "CairoFont-%d-%d",
	      font_subset->font_id, font_subset->subset_id);
    status = _cairo_type1_fallback_init_binary (&subset, name, font_subset);
    if (status)
	return status;

    status = _cairo_pdf_surface_emit_type1_font (surface, font_subset, &subset);

    _cairo_type1_fallback_fini (&subset);
    return status;
}

#define PDF_UNITS_PER_EM 1000

static cairo_status_t
_cairo_pdf_surface_emit_truetype_font_subset (cairo_pdf_surface_t		*surface,
					      cairo_scaled_font_subset_t	*font_subset)
{
    cairo_pdf_resource_t stream, descriptor, cidfont_dict;
    cairo_pdf_resource_t subset_resource, to_unicode_stream;
    cairo_status_t status;
    cairo_pdf_font_t font;
    cairo_truetype_subset_t subset;
    unsigned long compressed_length;
    char *compressed;
    unsigned int i;

    subset_resource = _cairo_pdf_surface_get_font_resource (surface,
							    font_subset->font_id,
							    font_subset->subset_id);
    if (subset_resource.id == 0)
	return CAIRO_STATUS_SUCCESS;

    status = _cairo_truetype_subset_init (&subset, font_subset);
    if (status)
	return status;

    compressed = compress_dup (subset.data, subset.data_length,
			       &compressed_length);
    if (compressed == NULL) {
	_cairo_truetype_subset_fini (&subset);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    stream = _cairo_pdf_surface_new_object (surface);
    if (stream.id == 0) {
	free (compressed);
	_cairo_truetype_subset_fini (&subset);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Filter /FlateDecode\r\n"
				 "   /Length %lu\r\n"
				 "   /Length1 %lu\r\n"
				 ">>\r\n"
				 "stream\r\n",
				 stream.id,
				 compressed_length,
				 subset.data_length);
    _cairo_output_stream_write (surface->output, compressed, compressed_length);
    _cairo_output_stream_printf (surface->output,
				 "\r\n"
				 "endstream\r\n"
				 "endobj\r\n");
    free (compressed);

    status = _cairo_pdf_surface_emit_to_unicode_stream (surface,
	                                                font_subset, TRUE,
							&to_unicode_stream);
    if (status && status != CAIRO_INT_STATUS_UNSUPPORTED) {
	_cairo_truetype_subset_fini (&subset);
	return status;
    }

    descriptor = _cairo_pdf_surface_new_object (surface);
    if (descriptor.id == 0) {
	_cairo_truetype_subset_fini (&subset);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /FontDescriptor\r\n"
				 "   /FontName /%s\r\n"
				 "   /Flags 4\r\n"
				 "   /FontBBox [ %ld %ld %ld %ld ]\r\n"
				 "   /ItalicAngle 0\r\n"
				 "   /Ascent %ld\r\n"
				 "   /Descent %ld\r\n"
				 "   /CapHeight %ld\r\n"
				 "   /StemV 80\r\n"
				 "   /StemH 80\r\n"
				 "   /FontFile2 %u 0 R\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 descriptor.id,
				 subset.base_font,
				 (long)(subset.x_min*PDF_UNITS_PER_EM),
				 (long)(subset.y_min*PDF_UNITS_PER_EM),
                                 (long)(subset.x_max*PDF_UNITS_PER_EM),
				 (long)(subset.y_max*PDF_UNITS_PER_EM),
				 (long)(subset.ascent*PDF_UNITS_PER_EM),
				 (long)(subset.descent*PDF_UNITS_PER_EM),
				 (long)(subset.y_max*PDF_UNITS_PER_EM),
				 stream.id);

    cidfont_dict = _cairo_pdf_surface_new_object (surface);
    if (cidfont_dict.id == 0) {
	_cairo_truetype_subset_fini (&subset);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    _cairo_output_stream_printf (surface->output,
                                 "%d 0 obj\r\n"
                                 "<< /Type /Font\r\n"
                                 "   /Subtype /CIDFontType2\r\n"
                                 "   /BaseFont /%s\r\n"
                                 "   /CIDSystemInfo\r\n"
                                 "   << /Registry (Adobe)\r\n"
                                 "      /Ordering (Identity)\r\n"
                                 "      /Supplement 0\r\n"
                                 "   >>\r\n"
                                 "   /FontDescriptor %d 0 R\r\n"
                                 "   /W [0 [",
                                 cidfont_dict.id,
                                 subset.base_font,
                                 descriptor.id);

    for (i = 0; i < font_subset->num_glyphs; i++)
        _cairo_output_stream_printf (surface->output,
                                     " %ld",
                                     (long)(subset.widths[i]*PDF_UNITS_PER_EM));

    _cairo_output_stream_printf (surface->output,
                                 " ]]\r\n"
				 ">>\r\n"
				 "endobj\r\n");

    _cairo_pdf_surface_update_object (surface, subset_resource);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Font\r\n"
				 "   /Subtype /Type0\r\n"
				 "   /BaseFont /%s\r\n"
                                 "   /Encoding /Identity-H\r\n"
				 "   /DescendantFonts [ %d 0 R]\r\n",
				 subset_resource.id,
				 subset.base_font,
				 cidfont_dict.id);

    if (to_unicode_stream.id != 0)
        _cairo_output_stream_printf (surface->output,
                                     "   /ToUnicode %d 0 R\r\n",
                                     to_unicode_stream.id);

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    font.font_id = font_subset->font_id;
    font.subset_id = font_subset->subset_id;
    font.subset_resource = subset_resource;
    status = _cairo_array_append (&surface->fonts, &font);

    _cairo_truetype_subset_fini (&subset);

    return status;
}

static cairo_int_status_t
_cairo_pdf_surface_emit_bitmap_glyph (cairo_pdf_surface_t	*surface,
				      cairo_scaled_font_t	*scaled_font,
				      unsigned long		 glyph_index,
				      cairo_pdf_resource_t	*glyph_ret,
                                      cairo_box_t               *bbox,
                                      double                    *width)
{
    cairo_scaled_glyph_t *scaled_glyph;
    cairo_status_t status;
    cairo_image_surface_t *image;
    unsigned char *row, *byte;
    int rows, cols;
    double x_advance, y_advance;

    status = _cairo_scaled_glyph_lookup (scaled_font,
					 glyph_index,
					 CAIRO_SCALED_GLYPH_INFO_METRICS|
					 CAIRO_SCALED_GLYPH_INFO_SURFACE,
					 &scaled_glyph);
    if (status)
	return status;

    x_advance = scaled_glyph->metrics.x_advance;
    y_advance = scaled_glyph->metrics.y_advance;
    cairo_matrix_transform_distance (&scaled_font->ctm, &x_advance, &y_advance);
    *bbox = scaled_glyph->bbox;
    *width = x_advance;

    image = scaled_glyph->surface;
    if (image->format != CAIRO_FORMAT_A1) {
	image = _cairo_image_surface_clone (image, CAIRO_FORMAT_A1);
	status = cairo_surface_status (&image->base);
	if (status)
	    return status;
    }

    status = _cairo_pdf_surface_open_stream (surface,
				             NULL,
					     surface->compress_content,
					     NULL);
    if (status) {
	if (image != scaled_glyph->surface)
	    cairo_surface_destroy (&image->base);
	return status;
    }

    *glyph_ret = surface->pdf_stream.self;

    _cairo_output_stream_printf (surface->output,
				 "%f 0 %f %f %f %f d1\r\n",
                                 x_advance,
				 _cairo_fixed_to_double (scaled_glyph->bbox.p1.x),
				 _cairo_fixed_to_double (scaled_glyph->bbox.p2.y),
				 _cairo_fixed_to_double (scaled_glyph->bbox.p2.x),
				 _cairo_fixed_to_double (scaled_glyph->bbox.p1.y));

    _cairo_output_stream_printf (surface->output,
				 "%f 0 0 %f %f %f cm\r\n",
				 _cairo_fixed_to_double (scaled_glyph->bbox.p2.x) - _cairo_fixed_to_double (scaled_glyph->bbox.p1.x),
				 _cairo_fixed_to_double (scaled_glyph->bbox.p1.y) - _cairo_fixed_to_double (scaled_glyph->bbox.p2.y),
				 _cairo_fixed_to_double (scaled_glyph->bbox.p1.x),
				 _cairo_fixed_to_double (scaled_glyph->bbox.p2.y));

    _cairo_output_stream_printf (surface->output,
				 "BI\r\n"
				 "/IM true\r\n"
				 "/W %d\r\n"
				 "/H %d\r\n"
				 "/BPC 1\r\n"
				 "/D [1 0]\r\n",
				 image->width,
				 image->height);

    _cairo_output_stream_printf (surface->output,
				 "ID ");
    for (row = image->data, rows = image->height; rows; row += image->stride, rows--) {
	for (byte = row, cols = (image->width + 7) / 8; cols; byte++, cols--) {
	    unsigned char output_byte = CAIRO_BITSWAP8_IF_LITTLE_ENDIAN (*byte);
	    _cairo_output_stream_write (surface->output, &output_byte, 1);
	}
    }
    _cairo_output_stream_printf (surface->output,
				 "\r\nEI\r\n");

    status = _cairo_pdf_surface_close_stream (surface);

    if (image != scaled_glyph->surface)
	cairo_surface_destroy (&image->base);

    return status;
}

static cairo_status_t
_cairo_pdf_surface_emit_type3_font_subset (cairo_pdf_surface_t		*surface,
					   cairo_scaled_font_subset_t	*font_subset)
{
    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    cairo_pdf_resource_t *glyphs, encoding, char_procs, subset_resource, to_unicode_stream;
    cairo_pdf_font_t font;
    cairo_matrix_t matrix;
    double *widths;
    unsigned int i;
    cairo_box_t font_bbox = {{0,0},{0,0}};
    cairo_box_t bbox = {{0,0},{0,0}};

    if (font_subset->num_glyphs == 0)
	return CAIRO_STATUS_SUCCESS;

    subset_resource = _cairo_pdf_surface_get_font_resource (surface,
							    font_subset->font_id,
							    font_subset->subset_id);
    if (subset_resource.id == 0)
	return CAIRO_STATUS_SUCCESS;

    glyphs = _cairo_malloc_ab (font_subset->num_glyphs, sizeof (cairo_pdf_resource_t));
    if (glyphs == NULL)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    widths = _cairo_malloc_ab (font_subset->num_glyphs, sizeof (double));
    if (widths == NULL) {
        free (glyphs);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    for (i = 0; i < font_subset->num_glyphs; i++) {
        status = _cairo_pdf_surface_emit_bitmap_glyph (surface,
                                                       font_subset->scaled_font,
                                                       font_subset->glyphs[i],
                                                       &glyphs[i],
                                                       &bbox,
                                                       &widths[i]);
	if (status)
	    break;

        if (i == 0) {
            font_bbox.p1.x = bbox.p1.x;
            font_bbox.p1.y = bbox.p1.y;
            font_bbox.p2.x = bbox.p2.x;
            font_bbox.p2.y = bbox.p2.y;
        } else {
            if (bbox.p1.x < font_bbox.p1.x)
                font_bbox.p1.x = bbox.p1.x;
            if (bbox.p1.y < font_bbox.p1.y)
                font_bbox.p1.y = bbox.p1.y;
            if (bbox.p2.x > font_bbox.p2.x)
                font_bbox.p2.x = bbox.p2.x;
            if (bbox.p2.y > font_bbox.p2.y)
                font_bbox.p2.y = bbox.p2.y;
        }
    }
    if (status) {
	free (glyphs);
	free (widths);
	return status;
    }

    encoding = _cairo_pdf_surface_new_object (surface);
    if (encoding.id == 0) {
	free (glyphs);
	free (widths);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Encoding\r\n"
				 "   /Differences [0", encoding.id);
    for (i = 0; i < font_subset->num_glyphs; i++)
	_cairo_output_stream_printf (surface->output,
				     " /%d", i);
    _cairo_output_stream_printf (surface->output,
				 "]\r\n"
				 ">>\r\n"
				 "endobj\r\n");

    char_procs = _cairo_pdf_surface_new_object (surface);
    if (char_procs.id == 0) {
	free (glyphs);
	free (widths);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<<\r\n", char_procs.id);
    for (i = 0; i < font_subset->num_glyphs; i++)
	_cairo_output_stream_printf (surface->output,
				     " /%d %d 0 R\r\n",
				     i, glyphs[i].id);
    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    free (glyphs);

    status = _cairo_pdf_surface_emit_to_unicode_stream (surface,
	                                                font_subset, FALSE,
							&to_unicode_stream);
    if (status && status != CAIRO_INT_STATUS_UNSUPPORTED) {
	free (widths);
	return status;
    }

    _cairo_pdf_surface_update_object (surface, subset_resource);
    matrix = font_subset->scaled_font->scale;
    status = cairo_matrix_invert (&matrix);
    /* _cairo_scaled_font_init ensures the matrix is invertible */
    assert (status == CAIRO_STATUS_SUCCESS);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Font\r\n"
				 "   /Subtype /Type3\r\n"
				 "   /FontBBox [%f %f %f %f]\r\n"
				 "   /FontMatrix [ %f %f %f %f 0 0 ]\r\n"
				 "   /Encoding %d 0 R\r\n"
				 "   /CharProcs %d 0 R\r\n"
				 "   /FirstChar 0\r\n"
				 "   /LastChar %d\r\n",
				 subset_resource.id,
				 _cairo_fixed_to_double (font_bbox.p1.x),
				 _cairo_fixed_to_double (font_bbox.p1.y),
				 _cairo_fixed_to_double (font_bbox.p2.x),
				 _cairo_fixed_to_double (font_bbox.p2.y),
				 matrix.xx,
				 matrix.yx,
				 -matrix.xy,
				 -matrix.yy,
				 encoding.id,
				 char_procs.id,
				 font_subset->num_glyphs - 1);

    _cairo_output_stream_printf (surface->output,
				 "   /Widths [");
    for (i = 0; i < font_subset->num_glyphs; i++)
	_cairo_output_stream_printf (surface->output, " %f", widths[i]);
    _cairo_output_stream_printf (surface->output,
				 "]\r\n");
    free (widths);

    if (to_unicode_stream.id != 0)
        _cairo_output_stream_printf (surface->output,
                                     "    /ToUnicode %d 0 R\r\n",
                                     to_unicode_stream.id);

    _cairo_output_stream_printf (surface->output,
				 ">>\r\n"
				 "endobj\r\n");

    font.font_id = font_subset->font_id;
    font.subset_id = font_subset->subset_id;
    font.subset_resource = subset_resource;
    return _cairo_array_append (&surface->fonts, &font);
}

static cairo_status_t
_cairo_pdf_surface_emit_unscaled_font_subset (cairo_scaled_font_subset_t *font_subset,
                                              void			 *closure)
{
    cairo_pdf_surface_t *surface = closure;
    cairo_status_t status;

    if (font_subset->is_composite) {
        status = _cairo_pdf_surface_emit_cff_font_subset (surface, font_subset);
        if (status != CAIRO_INT_STATUS_UNSUPPORTED)
            return status;

        status = _cairo_pdf_surface_emit_truetype_font_subset (surface, font_subset);
        if (status != CAIRO_INT_STATUS_UNSUPPORTED)
            return status;

        status = _cairo_pdf_surface_emit_cff_fallback_font (surface, font_subset);
        if (status != CAIRO_INT_STATUS_UNSUPPORTED)
            return status;
    } else {
#if CAIRO_HAS_FT_FONT
        status = _cairo_pdf_surface_emit_type1_font_subset (surface, font_subset);
        if (status != CAIRO_INT_STATUS_UNSUPPORTED)
            return status;
#endif

        status = _cairo_pdf_surface_emit_type1_fallback_font (surface, font_subset);
        if (status != CAIRO_INT_STATUS_UNSUPPORTED)
            return status;
    }

    ASSERT_NOT_REACHED;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_pdf_surface_emit_scaled_font_subset (cairo_scaled_font_subset_t *font_subset,
                                            void		       *closure)
{
    cairo_pdf_surface_t *surface = closure;
    cairo_status_t status;

    status = _cairo_pdf_surface_emit_type3_font_subset (surface, font_subset);
    if (status != CAIRO_INT_STATUS_UNSUPPORTED)
	return status;

    ASSERT_NOT_REACHED;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_pdf_surface_emit_font_subsets (cairo_pdf_surface_t *surface)
{
    cairo_status_t status;

    status = _cairo_scaled_font_subsets_foreach_unscaled (surface->font_subsets,
                                                          _cairo_pdf_surface_emit_unscaled_font_subset,
                                                          surface);
    if (status)
	goto BAIL;

    status = _cairo_scaled_font_subsets_foreach_scaled (surface->font_subsets,
                                                        _cairo_pdf_surface_emit_scaled_font_subset,
                                                        surface);

BAIL:
    _cairo_scaled_font_subsets_destroy (surface->font_subsets);
    surface->font_subsets = NULL;

    return status;
}

static cairo_pdf_resource_t
_cairo_pdf_surface_write_catalog (cairo_pdf_surface_t *surface)
{
    cairo_pdf_resource_t catalog;

    catalog = _cairo_pdf_surface_new_object (surface);
    if (catalog.id == 0)
	return catalog;

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Catalog\r\n"
				 "   /Pages %d 0 R\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 catalog.id,
				 surface->pages_resource.id);

    return catalog;
}

static long
_cairo_pdf_surface_write_xref (cairo_pdf_surface_t *surface)
{
    cairo_pdf_object_t *object;
    int num_objects, i;
    long offset;
    char buffer[11];

    num_objects = _cairo_array_num_elements (&surface->objects);

    offset = _cairo_output_stream_get_position (surface->output);
    _cairo_output_stream_printf (surface->output,
				 "xref\r\n"
				 "%d %d\r\n",
				 0, num_objects + 1);

    _cairo_output_stream_printf (surface->output,
				 "0000000000 65535 f\r\n");
    for (i = 0; i < num_objects; i++) {
	object = _cairo_array_index (&surface->objects, i);
	snprintf (buffer, sizeof buffer, "%010ld", object->offset);
	_cairo_output_stream_printf (surface->output,
				     "%s 00000 n\r\n", buffer);
    }

    return offset;
}

static cairo_status_t
_cairo_pdf_surface_write_mask_group (cairo_pdf_surface_t	*surface,
				     cairo_pdf_smask_group_t	*group)
{
    cairo_pdf_resource_t mask_group;
    cairo_pdf_resource_t smask;
    cairo_pdf_smask_group_t *smask_group;
    cairo_pdf_resource_t pattern_res, gstate_res;
    cairo_status_t status;

    /* Create mask group */
    status = _cairo_pdf_surface_open_group (surface, NULL);
    if (status)
	return status;

    pattern_res.id = 0;
    gstate_res.id = 0;
    status = _cairo_pdf_surface_add_pdf_pattern (surface, group->mask, &pattern_res, &gstate_res);
    if (status)
	return status;

    if (gstate_res.id != 0) {
	smask_group = _cairo_pdf_surface_create_smask_group (surface);
	if (smask_group == NULL)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

	smask_group->operation = PDF_PAINT;
	smask_group->source = cairo_pattern_reference (group->mask);
	smask_group->source_res = pattern_res;
	status = _cairo_pdf_surface_add_smask_group (surface, smask_group);
	if (status) {
	    _cairo_pdf_smask_group_destroy (smask_group);
	    return status;
	}

	status = _cairo_pdf_surface_add_smask (surface, gstate_res);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_xobject (surface, smask_group->group_res);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q /s%d gs /x%d Do Q\r\n",
				     gstate_res.id,
				     smask_group->group_res.id);
    } else {
	status = _cairo_pdf_surface_select_pattern (surface, group->mask, pattern_res, FALSE);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "0 0 %f %f re f\r\n",
				     surface->width, surface->height);

	_cairo_pdf_surface_unselect_pattern (surface);
    }

    status = _cairo_pdf_surface_close_group (surface, &mask_group);
    if (status)
	return status;

    /* Create source group */
    status = _cairo_pdf_surface_open_group (surface, &group->source_res);
    if (status)
	return status;

    pattern_res.id = 0;
    gstate_res.id = 0;
    status = _cairo_pdf_surface_add_pdf_pattern (surface, group->source, &pattern_res, &gstate_res);
    if (status)
	return status;

    if (gstate_res.id != 0) {
	smask_group = _cairo_pdf_surface_create_smask_group (surface);
	if (smask_group == NULL)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

	smask_group->operation = PDF_PAINT;
	smask_group->source = cairo_pattern_reference (group->source);
	smask_group->source_res = pattern_res;
	status = _cairo_pdf_surface_add_smask_group (surface, smask_group);
	if (status) {
	    _cairo_pdf_smask_group_destroy (smask_group);
	    return status;
	}

	status = _cairo_pdf_surface_add_smask (surface, gstate_res);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_xobject (surface, smask_group->group_res);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q /s%d gs /x%d Do Q\r\n",
				     gstate_res.id,
				     smask_group->group_res.id);
    } else {
	status = _cairo_pdf_surface_select_pattern (surface, group->source, pattern_res, FALSE);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "0 0 %f %f re f\r\n",
				     surface->width, surface->height);

	_cairo_pdf_surface_unselect_pattern (surface);
    }

    status = _cairo_pdf_surface_close_group (surface, NULL);
    if (status)
	return status;

    /* Create an smask based on the alpha component of mask_group */
    smask = _cairo_pdf_surface_new_object (surface);
    if (smask.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Mask\r\n"
				 "   /S /Alpha\r\n"
				 "   /G %d 0 R\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 smask.id,
				 mask_group.id);

    /* Create a GState that uses the smask */
    _cairo_pdf_surface_update_object (surface, group->group_res);
    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /ExtGState\r\n"
				 "   /SMask %d 0 R\r\n"
				 "   /ca 1\r\n"
				 "   /CA 1\r\n"
				 "   /AIS false\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 group->group_res.id,
				 smask.id);

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_status_t
_cairo_pdf_surface_write_smask_group (cairo_pdf_surface_t     *surface,
				      cairo_pdf_smask_group_t *group)
{
    cairo_status_t status;

    /* _mask is a special case that requires two groups - source
     * and mask as well as a smask and gstate dictionary */
    if (group->operation == PDF_MASK)
	return _cairo_pdf_surface_write_mask_group (surface, group);

    status = _cairo_pdf_surface_open_group (surface, &group->group_res);
    if (status)
	return status;

    status = _cairo_pdf_surface_select_pattern (surface,
						group->source,
						group->source_res,
						group->operation == PDF_STROKE);
    if (status)
	return status;

    switch (group->operation) {
    case PDF_PAINT:
	_cairo_output_stream_printf (surface->output,
				     "0 0 %f %f re f\r\n",
				     surface->width, surface->height);
	break;
    case PDF_MASK:
	ASSERT_NOT_REACHED;
	break;
    case PDF_FILL:
	status = _cairo_pdf_operators_fill (&surface->pdf_operators,
					    &group->path,
					    group->fill_rule);
	break;
    case PDF_STROKE:
	status = _cairo_pdf_operator_stroke (&surface->pdf_operators,
					     &group->path,
					     group->style,
					     &group->ctm,
					     &group->ctm_inverse);
	break;
    case PDF_SHOW_GLYPHS:
	status = _cairo_pdf_operators_show_glyphs (&surface->pdf_operators,
						   group->glyphs,
						   group->num_glyphs,
						   group->scaled_font);
	break;
    }
    if (status)
	return status;

    _cairo_pdf_surface_unselect_pattern (surface);

    return _cairo_pdf_surface_close_group (surface, NULL);
}

static cairo_status_t
_cairo_pdf_surface_write_patterns_and_smask_groups (cairo_pdf_surface_t *surface)
{
    cairo_pdf_pattern_t pattern;
    cairo_pdf_smask_group_t *group;
    int pattern_index, group_index;
    cairo_status_t status;

    /* Writing out PDF_MASK groups will cause additional smask groups
     * to be appended to surface->smask_groups. Additional patterns
     * may also be appended to surface->patterns.
     *
     * Writing meta surface patterns will cause additional patterns
     * and groups to be appended.
     */
    pattern_index = 0;
    group_index = 0;
    while ((pattern_index < _cairo_array_num_elements (&surface->patterns)) ||
	   (group_index < _cairo_array_num_elements (&surface->smask_groups)))
    {
	for (; group_index < _cairo_array_num_elements (&surface->smask_groups); group_index++) {
	    _cairo_array_copy_element (&surface->smask_groups, group_index, &group);
	    status = _cairo_pdf_surface_write_smask_group (surface, group);
	    if (status)
		return status;
	}

	for (; pattern_index < _cairo_array_num_elements (&surface->patterns); pattern_index++) {
	    _cairo_array_copy_element (&surface->patterns, pattern_index, &pattern);
	    status = _cairo_pdf_surface_emit_pattern (surface, &pattern);
	    if (status)
		return status;
	}
    }

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_pdf_surface_write_page (cairo_pdf_surface_t *surface)
{
    cairo_pdf_resource_t page, knockout, res;
    cairo_status_t status;
    int i, len;

    _cairo_pdf_group_resources_clear (&surface->resources);
    if (surface->has_fallback_images) {
	status = _cairo_pdf_surface_open_knockout_group (surface);
	if (status)
	    return status;

	len = _cairo_array_num_elements (&surface->knockout_group);
	for (i = 0; i < len; i++) {
	    _cairo_array_copy_element (&surface->knockout_group, i, &res);
	    _cairo_output_stream_printf (surface->output,
					 "/x%d Do\r\n",
					 res.id);
	    status = _cairo_pdf_surface_add_xobject (surface, res);
	    if (status)
		return status;
	}
	_cairo_output_stream_printf (surface->output,
				     "/x%d Do\r\n",
				     surface->content.id);
	status = _cairo_pdf_surface_add_xobject (surface, surface->content);
	if (status)
	    return status;

	status = _cairo_pdf_surface_close_group (surface, &knockout);
	if (status)
	    return status;

	_cairo_pdf_group_resources_clear (&surface->resources);
	status = _cairo_pdf_surface_open_content_stream (surface, FALSE);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "/x%d Do\r\n",
				     knockout.id);
	status = _cairo_pdf_surface_add_xobject (surface, knockout);
	if (status)
	    return status;

	status = _cairo_pdf_surface_close_content_stream (surface);
	if (status)
	    return status;
    }

    page = _cairo_pdf_surface_new_object (surface);
    if (page.id == 0)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    _cairo_output_stream_printf (surface->output,
				 "%d 0 obj\r\n"
				 "<< /Type /Page\r\n"
				 "   /Parent %d 0 R\r\n"
				 "   /MediaBox [ 0 0 %f %f ]\r\n"
				 "   /Contents %d 0 R\r\n"
				 "   /Group <<\r\n"
				 "      /Type /Group\r\n"
				 "      /S /Transparency\r\n"
				 "      /CS /DeviceRGB\r\n"
				 "   >>\r\n"
				 "   /Resources %d 0 R\r\n"
				 ">>\r\n"
				 "endobj\r\n",
				 page.id,
				 surface->pages_resource.id,
				 surface->width,
				 surface->height,
				 surface->content.id,
				 surface->content_resources.id);

    status = _cairo_array_append (&surface->pages, &page);
    if (status)
	return status;

    status = _cairo_pdf_surface_write_patterns_and_smask_groups (surface);
    if (status)
	return status;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_bool_t
_surface_pattern_supported (cairo_surface_pattern_t *pattern)
{
    cairo_extend_t extend;

    if (_cairo_surface_is_meta (pattern->surface))
	return TRUE;

    if (pattern->surface->backend->acquire_source_image == NULL)
	return FALSE;

    /* Does an ALPHA-only source surface even make sense? Maybe, but I
     * don't think it's worth the extra code to support it. */

/* XXX: Need to write this function here...
    content = cairo_surface_get_content (pattern->surface);
    if (content == CAIRO_CONTENT_ALPHA)
	return FALSE;
*/

    extend = cairo_pattern_get_extend (&pattern->base);
    switch (extend) {
    case CAIRO_EXTEND_NONE:
    case CAIRO_EXTEND_REPEAT:
    case CAIRO_EXTEND_REFLECT:
    /* There's no point returning FALSE for EXTEND_PAD, as the image
     * surface does not currently implement it either */
    case CAIRO_EXTEND_PAD:
	return TRUE;
    }

    ASSERT_NOT_REACHED;
    return FALSE;
}

static cairo_bool_t
_gradient_pattern_supported (cairo_pattern_t *pattern)
{
    cairo_extend_t extend;

    extend = cairo_pattern_get_extend (pattern);


    /* Radial gradients are currently only supported with EXTEND_NONE
     * and EXTEND_PAD and when one circle is inside the other. */
    if (pattern->type == CAIRO_PATTERN_TYPE_RADIAL) {
        double x1, y1, x2, y2, r1, r2, d;
        cairo_radial_pattern_t *radial = (cairo_radial_pattern_t *) pattern;

	if (extend == CAIRO_EXTEND_REPEAT ||
	    extend == CAIRO_EXTEND_REFLECT) {
	    return FALSE;
	}

	x1 = _cairo_fixed_to_double (radial->c1.x);
        y1 = _cairo_fixed_to_double (radial->c1.y);
        r1 = _cairo_fixed_to_double (radial->r1);
        x2 = _cairo_fixed_to_double (radial->c2.x);
        y2 = _cairo_fixed_to_double (radial->c2.y);
        r2 = _cairo_fixed_to_double (radial->r2);

        d = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
        if (d > fabs(r2 - r1)) {
            return FALSE;
        }
    }

    return TRUE;
}

static cairo_bool_t
_pattern_supported (cairo_pattern_t *pattern)
{
    if (pattern->type == CAIRO_PATTERN_TYPE_SOLID)
	return TRUE;

    if (pattern->type == CAIRO_PATTERN_TYPE_LINEAR ||
	pattern->type == CAIRO_PATTERN_TYPE_RADIAL)
	return _gradient_pattern_supported (pattern);

    if (pattern->type == CAIRO_PATTERN_TYPE_SURFACE)
	return _surface_pattern_supported ((cairo_surface_pattern_t *) pattern);

    return FALSE;
}

static cairo_int_status_t
_cairo_pdf_surface_analyze_operation (cairo_pdf_surface_t  *surface,
				      cairo_operator_t      op,
				      cairo_pattern_t      *pattern)
{
    if (surface->force_fallbacks && surface->paginated_mode == CAIRO_PAGINATED_MODE_ANALYZE)
	return CAIRO_INT_STATUS_UNSUPPORTED;

    if (! _pattern_supported (pattern))
	return CAIRO_INT_STATUS_UNSUPPORTED;

    if (op == CAIRO_OPERATOR_OVER || op == CAIRO_OPERATOR_SOURCE) {
	if (pattern->type == CAIRO_PATTERN_TYPE_SURFACE) {
	    cairo_surface_pattern_t *surface_pattern = (cairo_surface_pattern_t *) pattern;
	    if ( _cairo_surface_is_meta (surface_pattern->surface))
		return CAIRO_INT_STATUS_ANALYZE_META_SURFACE_PATTERN;
	}
    }

    if (op == CAIRO_OPERATOR_OVER)
	return CAIRO_STATUS_SUCCESS;

    /* The SOURCE operator is only if there is nothing painted
     * underneath. */
    if (op == CAIRO_OPERATOR_SOURCE)
	return CAIRO_INT_STATUS_FLATTEN_TRANSPARENCY;

    return CAIRO_INT_STATUS_UNSUPPORTED;
}

static cairo_bool_t
_cairo_pdf_surface_operation_supported (cairo_pdf_surface_t  *surface,
					cairo_operator_t      op,
					cairo_pattern_t      *pattern)
{
    if (_cairo_pdf_surface_analyze_operation (surface, op, pattern) != CAIRO_INT_STATUS_UNSUPPORTED)
	return TRUE;
    else
	return FALSE;
}

static cairo_int_status_t
_cairo_pdf_surface_start_fallback (cairo_pdf_surface_t *surface)
{
    cairo_status_t status;

    status = _cairo_pdf_surface_close_content_stream (surface);
    if (status)
	return status;

    status = _cairo_array_append (&surface->knockout_group, &surface->content);
    if (status)
	return status;

    surface->has_fallback_images = TRUE;
    _cairo_pdf_group_resources_clear (&surface->resources);
    return _cairo_pdf_surface_open_content_stream (surface, TRUE);
}

static cairo_int_status_t
_cairo_pdf_surface_paint (void			*abstract_surface,
			  cairo_operator_t	 op,
			  cairo_pattern_t	*source)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    cairo_status_t status;
    cairo_pdf_smask_group_t *group;
    cairo_pdf_resource_t pattern_res, gstate_res;

    if (surface->paginated_mode == CAIRO_PAGINATED_MODE_ANALYZE) {
	return _cairo_pdf_surface_analyze_operation (surface, op, source);
    } else if (surface->paginated_mode == CAIRO_PAGINATED_MODE_FALLBACK) {
	status = _cairo_pdf_surface_start_fallback (surface);
	if (status)
	    return status;
    }

    assert (_cairo_pdf_surface_operation_supported (surface, op, source));

    pattern_res.id = 0;
    gstate_res.id = 0;
    status = _cairo_pdf_surface_add_pdf_pattern (surface, source, &pattern_res, &gstate_res);
    if (status == CAIRO_INT_STATUS_NOTHING_TO_DO)
	return CAIRO_STATUS_SUCCESS;
    if (status)
	return status;

    if (gstate_res.id != 0) {
	group = _cairo_pdf_surface_create_smask_group (surface);
	if (group == NULL)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

	group->operation = PDF_PAINT;
	group->source = cairo_pattern_reference (source);
	group->source_res = pattern_res;
	status = _cairo_pdf_surface_add_smask_group (surface, group);
	if (status) {
	    _cairo_pdf_smask_group_destroy (group);
	    return status;
	}

	status = _cairo_pdf_surface_add_smask (surface, gstate_res);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_xobject (surface, group->group_res);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q /s%d gs /x%d Do Q\r\n",
				     gstate_res.id,
				     group->group_res.id);
    } else {
	status = _cairo_pdf_surface_select_pattern (surface, source, pattern_res, FALSE);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "0 0 %f %f re f\r\n",
				     surface->width, surface->height);

	_cairo_pdf_surface_unselect_pattern (surface);
    }

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_int_status_t
_cairo_pdf_surface_mask	(void			*abstract_surface,
			 cairo_operator_t	 op,
			 cairo_pattern_t	*source,
			 cairo_pattern_t	*mask)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    cairo_pdf_smask_group_t *group;
    cairo_status_t status, status2;

    if (surface->paginated_mode == CAIRO_PAGINATED_MODE_ANALYZE) {
	status = _cairo_pdf_surface_analyze_operation (surface, op, source);
	if (status != CAIRO_STATUS_SUCCESS &&
	    status != CAIRO_INT_STATUS_ANALYZE_META_SURFACE_PATTERN)
	    return status;

	status2 = _cairo_pdf_surface_analyze_operation (surface, op, mask);
	if (status2 != CAIRO_STATUS_SUCCESS)
	    return status2;

	return status;
    } else if (surface->paginated_mode == CAIRO_PAGINATED_MODE_FALLBACK) {
	status = _cairo_pdf_surface_start_fallback (surface);
	if (status)
	    return status;
    }

    assert (_cairo_pdf_surface_operation_supported (surface, op, source));
    assert (_cairo_pdf_surface_operation_supported (surface, op, mask));

    group = _cairo_pdf_surface_create_smask_group (surface);
    if (group == NULL)
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);

    group->operation = PDF_MASK;
    group->source = cairo_pattern_reference (source);
    group->mask = cairo_pattern_reference (mask);
    group->source_res = _cairo_pdf_surface_new_object (surface);
    if (group->source_res.id == 0) {
	_cairo_pdf_smask_group_destroy (group);
	return _cairo_error (CAIRO_STATUS_NO_MEMORY);
    }

    status = _cairo_pdf_surface_add_smask_group (surface, group);
    if (status) {
	_cairo_pdf_smask_group_destroy (group);
	return status;
    }

    status = _cairo_pdf_surface_add_smask (surface, group->group_res);
    if (status)
	return status;

    status = _cairo_pdf_surface_add_xobject (surface, group->source_res);
    if (status)
	return status;

    _cairo_output_stream_printf (surface->output,
				 "q /s%d gs /x%d Do Q\r\n",
				 group->group_res.id,
				 group->source_res.id);

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_int_status_t
_cairo_pdf_surface_stroke (void			*abstract_surface,
			   cairo_operator_t	 op,
			   cairo_pattern_t	*source,
			   cairo_path_fixed_t	*path,
			   cairo_stroke_style_t	*style,
			   cairo_matrix_t	*ctm,
			   cairo_matrix_t	*ctm_inverse,
			   double		 tolerance,
			   cairo_antialias_t	 antialias)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    cairo_status_t status;
    cairo_pdf_smask_group_t *group;
    cairo_pdf_resource_t pattern_res, gstate_res;

    if (surface->paginated_mode == CAIRO_PAGINATED_MODE_ANALYZE)
	return _cairo_pdf_surface_analyze_operation (surface, op, source);

    assert (_cairo_pdf_surface_operation_supported (surface, op, source));

    pattern_res.id = 0;
    gstate_res.id = 0;
    status = _cairo_pdf_surface_add_pdf_pattern (surface, source, &pattern_res, &gstate_res);
    if (status == CAIRO_INT_STATUS_NOTHING_TO_DO)
	return CAIRO_STATUS_SUCCESS;
    if (status)
	return status;

    if (gstate_res.id != 0) {
	group = _cairo_pdf_surface_create_smask_group (surface);
	if (group == NULL)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

	group->operation = PDF_STROKE;
	group->source = cairo_pattern_reference (source);
	group->source_res = pattern_res;
	status = _cairo_path_fixed_init_copy (&group->path, path);
	if (status) {
	    _cairo_pdf_smask_group_destroy (group);
	    return status;
	}

	group->style = style;
	group->ctm = *ctm;
	group->ctm_inverse = *ctm_inverse;
	status = _cairo_pdf_surface_add_smask_group (surface, group);
	if (status) {
	    _cairo_pdf_smask_group_destroy (group);
	    return status;
	}

	status = _cairo_pdf_surface_add_smask (surface, gstate_res);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_xobject (surface, group->group_res);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q /s%d gs /x%d Do Q\r\n",
				     gstate_res.id,
				     group->group_res.id);
    } else {
	status = _cairo_pdf_surface_select_pattern (surface, source, pattern_res, TRUE);
	if (status)
	    return status;

	status = _cairo_pdf_operator_stroke (&surface->pdf_operators,
					     path,
					     style,
					     ctm,
					     ctm_inverse);
	if (status)
	    return status;

	_cairo_pdf_surface_unselect_pattern (surface);
    }

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_int_status_t
_cairo_pdf_surface_fill (void			*abstract_surface,
			 cairo_operator_t	 op,
			 cairo_pattern_t	*source,
			 cairo_path_fixed_t	*path,
			 cairo_fill_rule_t	 fill_rule,
			 double			 tolerance,
			 cairo_antialias_t	 antialias)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    cairo_status_t status;
    cairo_pdf_smask_group_t *group;
    cairo_pdf_resource_t pattern_res, gstate_res;

    if (surface->paginated_mode == CAIRO_PAGINATED_MODE_ANALYZE) {
	return _cairo_pdf_surface_analyze_operation (surface, op, source);
    } else if (surface->paginated_mode == CAIRO_PAGINATED_MODE_FALLBACK) {
	status = _cairo_pdf_surface_start_fallback (surface);
	if (status)
	    return status;
    }

    assert (_cairo_pdf_surface_operation_supported (surface, op, source));

    pattern_res.id = 0;
    gstate_res.id = 0;
    status = _cairo_pdf_surface_add_pdf_pattern (surface, source, &pattern_res, &gstate_res);
    if (status == CAIRO_INT_STATUS_NOTHING_TO_DO)
	return CAIRO_STATUS_SUCCESS;
    if (status)
	return status;

    if (gstate_res.id != 0) {
	group = _cairo_pdf_surface_create_smask_group (surface);
	if (group == NULL)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

	group->operation = PDF_FILL;
	group->source = cairo_pattern_reference (source);
	group->source_res = pattern_res;
	status = _cairo_path_fixed_init_copy (&group->path, path);
	if (status) {
	    _cairo_pdf_smask_group_destroy (group);
	    return status;
	}

	group->fill_rule = fill_rule;
	status = _cairo_pdf_surface_add_smask_group (surface, group);
	if (status) {
	    _cairo_pdf_smask_group_destroy (group);
	    return status;
	}

	status = _cairo_pdf_surface_add_smask (surface, gstate_res);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_xobject (surface, group->group_res);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q /s%d gs /x%d Do Q\r\n",
				     gstate_res.id,
				     group->group_res.id);
    } else {
	status = _cairo_pdf_surface_select_pattern (surface, source, pattern_res, FALSE);
	if (status)
	    return status;

	status = _cairo_pdf_operators_fill (&surface->pdf_operators,
					    path,
					    fill_rule);
	if (status)
	    return status;

	_cairo_pdf_surface_unselect_pattern (surface);
    }

    return _cairo_output_stream_get_status (surface->output);
}

static cairo_int_status_t
_cairo_pdf_surface_show_glyphs (void			*abstract_surface,
				cairo_operator_t	 op,
				cairo_pattern_t		*source,
				cairo_glyph_t		*glyphs,
				int			 num_glyphs,
				cairo_scaled_font_t	*scaled_font)
{
    cairo_pdf_surface_t *surface = abstract_surface;
    cairo_status_t status;
    cairo_pdf_smask_group_t *group;
    cairo_pdf_resource_t pattern_res, gstate_res;

    if (surface->paginated_mode == CAIRO_PAGINATED_MODE_ANALYZE)
	return _cairo_pdf_surface_analyze_operation (surface, op, source);

    assert (_cairo_pdf_surface_operation_supported (surface, op, source));

    pattern_res.id = 0;
    gstate_res.id = 0;
    status = _cairo_pdf_surface_add_pdf_pattern (surface, source, &pattern_res, &gstate_res);
    if (status == CAIRO_INT_STATUS_NOTHING_TO_DO)
	return CAIRO_STATUS_SUCCESS;
    if (status)
	return status;

    if (gstate_res.id != 0) {
	group = _cairo_pdf_surface_create_smask_group (surface);
	if (group == NULL)
	    return _cairo_error (CAIRO_STATUS_NO_MEMORY);

	group->operation = PDF_SHOW_GLYPHS;
	group->source = cairo_pattern_reference (source);
	group->source_res = pattern_res;
	group->glyphs = glyphs;
	group->num_glyphs = num_glyphs;
	group->scaled_font = cairo_scaled_font_reference (scaled_font);
	status = _cairo_pdf_surface_add_smask_group (surface, group);
	if (status) {
	    _cairo_pdf_smask_group_destroy (group);
	    return status;
	}

	status = _cairo_pdf_surface_add_smask (surface, gstate_res);
	if (status)
	    return status;

	status = _cairo_pdf_surface_add_xobject (surface, group->group_res);
	if (status)
	    return status;

	_cairo_output_stream_printf (surface->output,
				     "q /s%d gs /x%d Do Q\r\n",
				     gstate_res.id,
				     group->group_res.id);
    } else {
	status = _cairo_pdf_surface_select_pattern (surface, source, pattern_res, FALSE);
	if (status)
	    return status;

	status = _cairo_pdf_operators_show_glyphs (&surface->pdf_operators,
						   glyphs,
						   num_glyphs,
						   scaled_font);
	if (status)
	    return status;

	_cairo_pdf_surface_unselect_pattern (surface);
    }

    return _cairo_output_stream_get_status (surface->output);
}

static void
_cairo_pdf_surface_set_paginated_mode (void			*abstract_surface,
				       cairo_paginated_mode_t	 paginated_mode)
{
    cairo_pdf_surface_t *surface = abstract_surface;

    surface->paginated_mode = paginated_mode;
}

static const cairo_surface_backend_t cairo_pdf_surface_backend = {
    CAIRO_SURFACE_TYPE_PDF,
    _cairo_pdf_surface_create_similar,
    _cairo_pdf_surface_finish,
    NULL, /* acquire_source_image */
    NULL, /* release_source_image */
    NULL, /* acquire_dest_image */
    NULL, /* release_dest_image */
    NULL, /* clone_similar */
    NULL, /* composite */
    NULL, /* fill_rectangles */
    NULL, /* composite_trapezoids */
    NULL,  /* _cairo_pdf_surface_copy_page */
    _cairo_pdf_surface_show_page,
    NULL, /* set_clip_region */
    _cairo_pdf_surface_intersect_clip_path,
    _cairo_pdf_surface_get_extents,
    NULL, /* old_show_glyphs */
    _cairo_pdf_surface_get_font_options,
    NULL, /* flush */
    NULL, /* mark_dirty_rectangle */
    NULL, /* scaled_font_fini */
    NULL, /* scaled_glyph_fini */

    /* Here are the drawing functions */

    _cairo_pdf_surface_paint,
    _cairo_pdf_surface_mask,
    _cairo_pdf_surface_stroke,
    _cairo_pdf_surface_fill,
    _cairo_pdf_surface_show_glyphs,
    NULL, /* snapshot */

    NULL, /* is_compatible */
    NULL, /* reset */
};

static const cairo_paginated_surface_backend_t cairo_pdf_surface_paginated_backend = {
    _cairo_pdf_surface_start_page,
    _cairo_pdf_surface_set_paginated_mode
};
