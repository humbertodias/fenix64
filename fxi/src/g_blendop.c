/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.84
 *  Last stable release   :
 *  Project documentation : http://fenix.divsite.net
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Copyright © 1999 José Luis Cebrián Pagüe
 *  Copyright © 2002 Fenix Team
 *
 */

/*
 * FILE        : g_blendop.c
 * DESCRIPTION : Blendop (16-bits translucency substitutes) tables
 *
 * HISTORY:      0.82 - Blendops functions moved from g_pal.c
 */

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "fxi.h"

/* Fast macros for color component extraction from a 16 bits value */

#define GETR(color) (((color & screen->format->Rmask) >> screen->format->Rshift) << screen->format->Rloss)
#define GETG(color) (((color & screen->format->Gmask) >> screen->format->Gshift) << screen->format->Gloss)
#define GETB(color) (((color & screen->format->Bmask) >> screen->format->Bshift) << screen->format->Bloss)

/* Fast macros for color composition */

#define MAKERGB_SATURATE(r,g,b)												\
(																			\
	((int)(r) > 255 ? screen->format->Rmask : (((int)(r) >> screen->format->Rloss) << screen->format->Rshift)) |	\
	((int)(g) > 255 ? screen->format->Gmask : (((int)(g) >> screen->format->Gloss) << screen->format->Gshift)) |	\
	((int)(b) > 255 ? screen->format->Bmask : (((int)(b) >> screen->format->Bloss) << screen->format->Bshift))      \
)

#define MAKERGB(r,g,b)													\
(																		\
    (((int)(r) >> screen->format->Rloss) << screen->format->Rshift) |	\
	(((int)(g) >> screen->format->Gloss) << screen->format->Gshift) |	\
    (((int)(b) >> screen->format->Bloss) << screen->format->Bshift)		\
)

/*
 *  FUNCTION : blend_create
 *
 *  Create a new blendop table and initialize it with blend_init
 *  A blendop table is a group of two tables that return intermediate
 *  colors to a composite (+) operation that does no saturation:
 *
 *		Src_param	=	Src_color
 *		Dst_param	=	Dst_color
 *		Dst_color	=	Src_param + Dst_param		// No clamp!
 *
 *	The other blend_x funcions change the Src_param and Dst_param formulas.
 *	The Dst_color formula is immutable and embedded in the blitter
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      Pointer to the new blendop table or NULL if not enough memory
 */

Sint16 * blend_create ()
{
	Sint16 * blend ;

    if (!scr_initialized) return NULL;

	blend = malloc(65536*2*sizeof(Sint16)) ;
	if (!blend) {
	    gr_con_printf ("blend_create: sin memoria") ;
	    return NULL;
	}
	blend_init (blend) ;

	return blend ;
}

/*
 *  FUNCTION : blend_free
 *
 *  Free the memory used by a blendop table
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *
 *  RETURN VALUE :
 *      None
 */

void blend_free (Sint16 * blend)
{
	if (blend) free (blend) ;
    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor
}

/*
 *  FUNCTION : blend_init
 *
 *  Initialize a blend table as a copy operation
 *
 *		Src_param	=	Src_color
 *		Dst_param	=	0
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *		ammount			Color component factor (1.0f leaves source unchanged)
 *
 *  RETURN VALUE :
 *      None
 */

void blend_init (Sint16 * blend)
{
	int i ;

    if (!blend) return;

	for (i = 0 ; i < 65536 ; i++) blend[i] = i ;

	blend += 65536 ;
	for (i = 0 ; i < 65536 ; i++) blend[i] = 0 ;

    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor

}

/*
 *  FUNCTION : blend_intensity
 *
 *  Modify a blend table as an intensity operation (changes the
 *  color value of the source object as the factor ammount but
 *  does no transparency or other operation with the background)
 *
 *		Src_param	=	(previous Src_param) * ammount
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *		ammount			Color component factor (1.0f leaves source unchanged)
 *
 *  RETURN VALUE :
 *      None
 */

void blend_intensity (Sint16 * blend, float ammount)
{
	int i, r, g, b ;

    if (!blend) return;

	if (ammount < 0.0f) ammount = 0.0f ;

	for (i = 0 ; i < 65536 ; i++)
	{
		r = (int)(GETR(blend[i]) * ammount) ;
		g = (int)(GETG(blend[i]) * ammount) ;
		b = (int)(GETB(blend[i]) * ammount) ;
		blend[i] = MAKERGB_SATURATE (r, g, b) ;
	}

    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor

}

/*
 *  FUNCTION : blend_translucency
 *
 *  Modify a blend table as a translucency combination operation
 *
 *		Src_param	=	(previous Src_param) * ammount
 *		Dst_param	=	Dst_color * (1.0 - ammount)
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *		ammount			Opacity factor (1.0f = opaque, 0.0f = transparent)
 *
 *  RETURN VALUE :
 *      None
 */

void blend_translucency (Sint16 * blend, float ammount)
{
	int i, r, g, b ;

    if (!blend) return;

	if (ammount > 1.0f) ammount = 1.0f ;
	if (ammount < 0.0f) ammount = 0.0f ;

	ammount = 1.0f - ammount ;
	for (i = 0 ; i < 65536 ; i++)
	{
		r = (int)(GETR(blend[i]) * ammount) ;
		g = (int)(GETG(blend[i]) * ammount) ;
		b = (int)(GETB(blend[i]) * ammount) ;
		blend[i] = MAKERGB (r, g, b) ;
	}

	blend += 65536 ;
	ammount = 1.0f - ammount ;
	for (i = 0 ; i < 65536 ; i++)
	{
		r = (int)(GETR(i) * ammount) ;
		g = (int)(GETG(i) * ammount) ;
		b = (int)(GETB(i) * ammount) ;
		blend[i] = MAKERGB (r, g, b) ;
	}

    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor

}

/*
 *  FUNCTION : blend_tint
 *
 *  Modify a blend table as a tint operation (changes the
 *  color value of the source object as a combination of a given
 *  color and the source color with the factor given)
 *
 *		Src_param = Const_color * ammount
 *					+ (previous Src_Param) * (1.0f-ammount)
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *		ammount			Color component factor (1.0f = Full Const color)
 *		cr				Constant color, red component (0-255)
 *		cg				Constant color, green component (0-255)
 *		cb				Constant color, blue component (0-255)
 *
 *  RETURN VALUE :
 *      None
 */

void blend_tint (Sint16 * blend, float ammount, Uint8 cr, Uint8 cg, Uint8 cb)
{
	int i, r, g, b ;

    if (!blend) return;

	if (ammount > 1.0f) ammount = 1.0f ;
	if (ammount < 0.0f) ammount = 0.0f ;

	for (i = 0 ; i < 65536 ; i++)
	{
		r = (int)(ammount * cr + (1.0f - ammount) * GETR(blend[i])) ;
		g = (int)(ammount * cg + (1.0f - ammount) * GETG(blend[i])) ;
		b = (int)(ammount * cb + (1.0f - ammount) * GETB(blend[i])) ;
		blend[i] = MAKERGB_SATURATE (r, g, b) ;
	}

    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor

}

/*
 *  FUNCTION : blend_grayscale
 *
 *  Initialize a blend table as a grayscale operation (changes the
 *  color value of the source object to grayscale). Three methods
 *  of operation are supported:
 *
 *		1.	Luminance:	result = 0.3R + 0.59G + 0.11B
 *		2.	Desaturate: result = (MAX(R,G,B)+MIN(R,G,B))/2
 *		3.	Maximum:    result = MAX(R,G,B)
 *
 *
 *		Src_param	=	Grayscale_operation(previous Src_param)
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *		method			Method of operation
 *
 *  RETURN VALUE :
 *      None
 */

void blend_grayscale (Sint16 * blend, int method)
{
	int i, r, g, b, max, min ;

    if (!blend) return;

	if (method == 1)
	{
		for (i = 0 ; i < 65536 ; i++)
		{
			r = (int)(GETR(i) * 0.3) ;
			g = (int)(GETG(i) * 0.59) ;
			b = (int)(GETB(i) * 0.11) ;
			r = r+g+b;
			blend[i] = MAKERGB(r, r, r) ;
		}
	}
	else if (method == 2)
	{
		for (i = 0 ; i < 65536 ; i++)
		{
			r = GETR(i);
			g = GETG(i);
			b = GETB(i);
			max = r > g ? r > b ? r : g : g > b ? g : b ;
			min = r < g ? r < b ? r : g : g < b ? g : b ;
			r = (max+min)/2;
			blend[i] = MAKERGB(r, r, r) ;
		}
	}
	else if (method == 3)
	{
		for (i = 0 ; i < 65536 ; i++)
		{
			r = GETR(i);
			g = GETG(i);
			b = GETB(i);
			max = r > g ? r > b ? r : g : g > b ? g : b ;
			blend[i] = MAKERGB(max, max, max) ;
		}
	}

	blend += 65536 ;
	for (i = 0 ; i < 65536 ; i++) blend[i] = 0 ;

    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor

}

/*
 *  FUNCTION : blend_swap
 *
 *  Swaps a blendop table. That is, change the blend operation so
 *  the Dst_color and Src_color are exchanged in the formulas.
 *
 *  PARAMS :
 *		blend			Pointer to the blend table
 *
 *  RETURN VALUE :
 *      None
 */

void blend_swap (Sint16 * blend)
{
	int i, j ;

    if (!blend) return;

	for (i = 0 ; i < 65536 ; i++)
	{
		j = blend[i] ;
		blend[i] = blend[65536+i] ;
		blend[65536+i] = j ;
	}

    background_dirty = 1 ; // Temporal, hasta buscar una solucion mejor

}

/*
 *  FUNCTION : blend_assign
 *
 *  Assign a blend operation to a graphic. The graphic will be
 *  drawn using this blend operation thereafter.
 *
 *  PARAMS :
 *		map				Pointer to the graphic
 *		blend			Pointer to the blend table
 *
 *  RETURN VALUE :
 *      None
 */

void blend_assign (GRAPH * map, Sint16 * blend)
{
	map->blend_table = blend ;
	map->modified = 1 ;
}

/*
 *  FUNCTION : blend_apply
 *
 *  Apply a blend operation to the pixels of a graphic, as if it was
 *  rendered into a black (color 0) background
 *
 *  PARAMS :
 *		map				Pointer to the graphic
 *		blend			Pointer to the blend table
 *
 *  RETURN VALUE :
 *      None
 */

void blend_apply (GRAPH * map, Sint16 * blend)
{
	Uint16 * ptr ;
	Uint32 x, y ;

    if (!blend) return;

	if (map->depth != 16) return ;

	for (y = 0 ; y < map->height ; y++)
	{
		ptr = (Uint16 *)map->data + map->pitch * y / 2 ;
		for (x = 0 ; x < map->width ; x++, ptr++)
		{
			if (*ptr) *ptr = blend[*ptr] + blend[65536+*ptr] ;
		}
	}

	map->modified = 1 ;

}
