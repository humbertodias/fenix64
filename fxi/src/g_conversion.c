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
 * FILE        : g_conversion.c
 * DESCRIPTION : Functions to convert bitmap data between formats
 *
 * HISTORY:      0.83 - Alpha operations and tables
 *               0.82 - First version
 */

#include <stdlib.h>

#include "fxi.h"

/* Conversion tables - used by 16 bits conversions - 256K */
static Uint16 * convert565ToScreen ;
static Uint16 * convertScreenTo565 ;
static int      conversion_tables_ok = 0 ;

/* Alpha multiplication tables - multiply a color by its alpha.
 * There will be less than 256 levels
 */
static Uint16 * alpha16[256];
static Uint8  * alpha8[256];
static int      alpha16_tables_ok = 0 ;
static int      alpha8_tables_ok = 0 ;

/*
 *  FUNCTION : init_alpha16_tables
 *
 *  Init the 16 bit alpha tables (create as many tables as the parameter)
 *  Each alpha table has a 128K memory footprint. Having many alpha tables
 *  provides more transparency values.
 *
 *  PARAMS :
 *		count			Number of tables
 *
 *  RETURN VALUE :
 *      None
 *
 */

static void init_alpha16_tables (int count)
{
	int      i, color, inc, next = 0, factor;
	Uint16 * table16 = NULL;

	if (alpha16_tables_ok == count) return;
	if (count <= 0 || count > 128) return;

	inc = 256/count;

	/* Destroy existing tables */

	if (alpha16_tables_ok != 0)
	{
		for (table16 = NULL, i = 0 ; i < 256 ; i++)
		{
			if (alpha16[i] != table16)
			{
				table16 = alpha16[i];
				free(table16);
			}
			alpha16[i] = NULL;
		}
	}

	/* Make new ones */

	for (i = 0 ; i < 256 ; i++)
	{
		if (i == next)
		{
			table16 = malloc(131072);
			factor = next+inc/2;
			next += inc;
			if (factor > 255) factor = 256;

			for (color = 0 ; color < 65536 ; color++)
			{
				int r = ((color & screen->format->Rmask) >> screen->format->Rshift << screen->format->Rloss) ;
				int g = ((color & screen->format->Gmask) >> screen->format->Gshift << screen->format->Gloss) ;
				int b = ((color & screen->format->Bmask) >> screen->format->Bshift << screen->format->Bloss) ;

				table16[color] =
					((((r * factor) >> 8) >> screen->format->Rloss) << screen->format->Rshift) |
					((((g * factor) >> 8) >> screen->format->Gloss) << screen->format->Gshift) |
					((((b * factor) >> 8) >> screen->format->Bloss) << screen->format->Bshift) ;
			}
		}
		alpha16[i] = table16;
	}

	alpha16_tables_ok = count;
}


/*
 *  FUNCTION : init_alpha8_tables
 *
 *  Init the 8 bit alpha tables (create as many tables as the parameter)
 *  Each alpha table has a 64K memory footprint. Having many alpha tables
 *  provides more transparency values. Those tables should be updated
 *  when the palette changes.
 *
 *  PARAMS :
 *		count			Number of tables
 *
 *  RETURN VALUE :
 *      None
 *
 */

extern Uint8 nearest_table[64][64][64] ;

static void init_alpha8_tables (int count)
{
	int     i, color, color2, inc, next = 0, factor;
	Uint8 * table8 = NULL;

	if (alpha8_tables_ok == count)
		return;
	if (count <= 0 || count > 128)
		return;

	inc = 256/count;

	gr_fill_nearest_table();

	/* Destroy existing tables */

	if (alpha8_tables_ok != 0)
	{
		for (table8 = NULL, i = 0 ; i < 256 ; i++)
		{
			if (alpha8[i] != table8)
			{
				table8 = alpha8[i];
				free(table8);
			}
			alpha8[i] = NULL;
		}
	}

	/* Make new ones */

	for (i = 0 ; i < 256 ; i++)
	{
		if (i == next)
		{
			table8  = malloc(65536);
			factor = next+inc/2;
			next += inc;
			if (factor > 255) factor = 256;

			for (color = 0 ; color < 256 ; color++)
			{
				for (color2 = 0 ; color2 < 256 ; color2++)
				{
					int r = (palette[color].r * factor + palette[color2].r * (255-factor));
					int g = (palette[color].g * factor + palette[color2].g * (255-factor));
					int b = (palette[color].b * factor + palette[color2].b * (255-factor));
					table8[(color << 8) + color2] = nearest_table[r >> 10][g >> 10][b >> 10];
				}
				table8[color] = color;
			}
		}
		alpha8[i] = table8;
	}

	alpha8_tables_ok = count;
}


/*
 *  FUNCTION : init_conversion_tables
 *
 *  Static routine used to initialize the 16 bits conversion
 *  tables (this only needs to be done once)
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 *
 */

static void init_conversion_tables()
{
	Uint8 r, g, b ;
	int n ;

	/* Alloc space for the lookup tables */

	convert565ToScreen = (Uint16 *) malloc(sizeof(Uint16) * 65536);
	convertScreenTo565 = (Uint16 *) malloc(sizeof(Uint16) * 65536);
	if (convert565ToScreen == NULL && convertScreenTo565 == NULL)
	{
		gr_error ("init_conversion_tables: sin memoria");
		return;
	}

	if (!scr_initialized) gr_init(320,200);
	conversion_tables_ok = 1;

	/* Special case if screen already in 565 format */

	if (screen->format->Rmask == 0xF800 &&
		screen->format->Gmask == 0x07E0 &&
		screen->format->Bmask == 0x001F)
	{
		for (n = 0 ; n < 65536 ; n++)
		{
			convert565ToScreen[n] = n;
			convertScreenTo565[n] = n;
		}
		return;
	}

	/* Create a fast lookup array */

	for (n = 0 ; n < 65536 ; n++)
	{
		/* Calculate conversion from 565 to screen format */

		r = ((n >> 8) & 0xF8) >> screen->format->Rloss ;
		g = ((n >> 3) & 0xFC) >> screen->format->Gloss ;
		b = ((n << 3) & 0xF8) >> screen->format->Bloss ;

		convert565ToScreen[n] = (r << screen->format->Rshift) |
			                    (g << screen->format->Gshift) |
			                    (b << screen->format->Bshift) ;

		/* Calculate conversion from 565 to screen format */

		r = (((n & screen->format->Rmask) >> screen->format->Rshift)
			    << screen->format->Rloss);
		g = (((n & screen->format->Gmask) >> screen->format->Gshift)
			    << screen->format->Gloss);
		b = (((n & screen->format->Bmask) >> screen->format->Bshift)
			    << screen->format->Bloss);

		convertScreenTo565[n] = ((r & 0xF8) << 8) |
			                    ((g & 0xFC) << 3) |
			                    ((b & 0xF8) >> 3) ;
	}
}

/*
 *  FUNCTION : gr_convert16_565ToScreen
 *
 *  Convert a sequence of 16 bits pixels from 5:6:5 format to
 *  the format used by the screen (usually 5:5:5 or 5:6:5)
 *
 *  PARAMS :
 *		ptr				Pointer to the first pixel
 *		len				Number of pixels (not bytes!)
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_convert16_565ToScreen (Uint16 * ptr, int len)
{
	if (!conversion_tables_ok) init_conversion_tables();

	while (len--) {
	    *ptr = convert565ToScreen[*ptr] ;
	    ptr++;
	}
}

/*
 *  FUNCTION : gr_convert16_ScreenTo565
 *
 *  Convert a sequence of 16 bits pixels in screen format
 *  (usually 5:5:5 or 5:6:5) to the 5:6:5 format used to
 *  store 16 bits pixel values to files in disk
 *
 *  PARAMS :
 *		ptr				Pointer to the first pixel
 *		len				Number of pixels (not bytes!)
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_convert16_ScreenTo565 (Uint16 * ptr, int len)
{
	if (!conversion_tables_ok) init_conversion_tables();

	while (len--) {
	    *ptr = convertScreenTo565[*ptr] ;
	    ptr++;
	}
}

/*
 *  FUNCTION : gr_fade16
 *
 *  Fade a 16-bit graphic a given ammount. Fading values are given in percent
 *  (0% = black, 100% = original color, 200% = full color value)
 *
 *  PARAMS :
 *		graph			Pointer to the graphic object
 *		r				Percent of Red component
 *		g				Percent of Green component
 *		b				Percent of Blue component
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_fade16 (GRAPH * graph, int r, int g, int b)
{
	Uint16 Rtable[32];
	Uint16 Gtable[32];
	Uint16 Btable[32];
	Uint32 x, y;
	Uint32 Rmask;
	Uint32 Rshift;
	Uint32 Gmask;
	Uint32 Gshift;
	Uint32 Bmask;
	Uint32 Bshift;

	for (x = 0 ; x < 32 ; x++)
	{
		int c = 8*x+7;

		if (r <= 100)
			Rtable[x] = (c * r / 100) >> screen->format->Rloss << screen->format->Rshift;
		else
			Rtable[x] = (c + (255-c) * (r-100) / 100) >> screen->format->Rloss << screen->format->Rshift;

		if (g <= 100)
			Gtable[x] = (c * g / 100) >> screen->format->Gloss << screen->format->Gshift;
		else
			Gtable[x] = (c + (255-c) * (g-100) / 100) >> screen->format->Gloss << screen->format->Gshift;

		if (b <= 100)
			Btable[x] = (c * b / 100) >> screen->format->Bloss << screen->format->Bshift;
		else
			Btable[x] = (c + (255-c) * (b-100) / 100) >> screen->format->Bloss << screen->format->Bshift;
	}

	Rmask  = screen->format->Rmask;
	Gmask  = screen->format->Gmask;
	Bmask  = screen->format->Bmask;
	Rshift = screen->format->Rshift - screen->format->Rloss + 3;
	Gshift = screen->format->Gshift - screen->format->Gloss + 3;
	Bshift = screen->format->Bshift - screen->format->Bloss + 3;

	for (y = 0 ; y < graph->height ; y++)
	{
		Uint16 * ptr = (Uint16 *)graph->data + graph->pitch*y/2;

		for (x = 0 ; x < graph->width ; x++, ptr++)
		{
			*ptr = (Rtable[((*ptr & Rmask) >> Rshift)] |
			        Gtable[((*ptr & Gmask) >> Gshift)] |
			        Btable[((*ptr & Bmask) >> Bshift)] );
		}
	}
}

/*
 *  FUNCTION : gr_alpha16
 *
 *  Get an alpha multiplication table (a table that, given a 16 bit color,
 *  returns the color multiplied by the alpha value)
 *
 *  PARAMS :
 *		alpha			Alpha value for the requested table
 *
 *  RETURN VALUE :
 *      None
 *
 */

Uint16 * gr_alpha16 (int alpha)
{
	if (alpha16_tables_ok == 0) init_alpha16_tables(GLODWORD(ALPHA_STEPS));
	return alpha16[alpha];
}

/*
 *  FUNCTION : gr_alpha8
 *
 *  Get an alpha translation table (a table that, given two 8 bit color,
 *  returns the composite color given the alpha value)
 *
 *  PARAMS :
 *		alpha			Alpha value for the requested table
 *
 *  RETURN VALUE :
 *      None
 *
 */

Uint8 * gr_alpha8 (int alpha)
{
	if (alpha8_tables_ok == 0)
		init_alpha8_tables(GLODWORD(ALPHA_STEPS));
	return (Uint8 *) alpha8[alpha];
}
