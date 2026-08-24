/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.83
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
 * FILE        : img_png.c
 * DESCRIPTION : PNG Loading functions
 *
 * HISTORY:		0.83 - Added gr_save_png
 *
 */

/*
 *	INCLUDES
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

#include <png.h>
#include "fxi.h"

/*
 *	GLOBAL VARIABLES
 */

extern SDL_Surface * screen ;	/* DEFINED in g_main.c */
static file * png ;


static void user_read_data (png_structp png_ptr, 
		png_bytep data, png_size_t length)
{
	file_read (png, data, length) ;
}

GRAPH * gr_read_png (const char * filename)
{
	GRAPH * bitmap ;
	unsigned int n, x ;
	int shift;
	Uint16 * ptr ;
	Uint32 * orig ;
	Uint32 row[2048] ;
	Uint32 Rshift, Gshift, Bshift ;
	Uint32 Rmask, Gmask, Bmask ;

	png_bytep	rowpointers[2048] ;

	png_structp	png_ptr ;
	png_infop	info_ptr, end_info ;
	png_uint_32    	width, height, rowbytes;
	int		depth, color ;

	/* Abre el fichero y se asegura de que screen está inicializada */

	png = file_open (filename, "rb") ;
	if (!png) gr_error ("No existe %s\n", filename) ;
	if (!scr_initialized) gr_init(320, 200) ;

	/* Prepara las estructuras internas */

	png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0) ;
	if (!png_ptr) gr_error ("Error al cargar PNG") ;
	info_ptr = png_create_info_struct (png_ptr) ;
	end_info = png_create_info_struct (png_ptr) ;
	if (!info_ptr || !end_info) gr_error ("Error al cargar PNG") ;

	/* Rutina de error */

	if (setjmp (png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct (&png_ptr, &info_ptr, &end_info) ;
		file_close (png) ;
		return 0 ;
	}

	/* Recupera información sobre el PNG */

	png_set_read_fn (png_ptr, 0, user_read_data) ;
	png_read_info (png_ptr, info_ptr) ;
	png_get_IHDR (png_ptr, info_ptr, &width, &height, &depth, &color, 0, 0 , 0) ;

	if (height > 2048 || width > 2048) 
		gr_error ("PNG demasiado grande") ;

	/* Configura los distintos modos disponibles */

	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb (png_ptr);
		if (color == PNG_COLOR_TYPE_GRAY)
			png_set_filler (png_ptr, 0xFF, PNG_FILLER_AFTER) ;
	}
	if (depth == 16) 
		png_set_strip_16(png_ptr) ;
	if (color == PNG_COLOR_TYPE_RGB) 
		png_set_filler (png_ptr, 0xFF, PNG_FILLER_AFTER) ;
	png_set_bgr(png_ptr) ;

	/* Recupera el fichero, convirtiendo a 16 bits si es preciso */

	rowbytes = png_get_rowbytes (png_ptr, info_ptr) ;
	bitmap = bitmap_new (0, width, height, color == PNG_COLOR_TYPE_PALETTE ? 8 : 16) ;
	if (!bitmap) gr_error ("Error al cargar PNG") ;
	if (color == PNG_COLOR_TYPE_PALETTE)
	{
		/* Read the color palette */

		if (!palette_loaded)
		{
			png_colorp png_palette = (png_colorp) png_malloc (png_ptr, 256*sizeof(png_color)) ;
			if (!png_palette) 
				gr_error ("Sin memoria") ;
			else
			{
				png_get_PLTE (png_ptr, info_ptr, &png_palette, &n) ;

				for (n-- ; n <= 255 ; n--)
				{
					palette[n].r = png_palette[n].red;
					palette[n].g = png_palette[n].green;
					palette[n].b = png_palette[n].blue;
				}	
			}

			palette_loaded = 1 ;
			palette_changed = 1 ;
		}

		for (n = 0 ; n < height ; n++)
			rowpointers[n] = ((Uint8*)bitmap->data) + n*bitmap->pitch ;
		png_read_image (png_ptr, rowpointers) ;

		/* If the depth is less than 8, expand the pixel values */

		if (depth == 4)
		{
			shift = ((width & 0x01) << 2);
			for (n =  0 ; n < height ; n++)
			{
				char * orig = rowpointers[n];
				char * dest = orig + width - 1;
				orig += (width-1)/2;

				for (x = 0 ; x < width ; x++)
				{
					*dest-- = ((*orig >> shift) & 0x0F);
					if (shift == 4)
					{
						shift = 0;
						orig--;
					}
					else shift = 4;
				}
			}
		}
		else if (depth == 2)
		{
			shift = ((3 - ((width + 3) & 0x03)) << 1);
			for (n = 0 ; n < height ; n++)
			{
				char * orig = rowpointers[n];
				char * dest = orig + width - 1;
				orig += (width-1)/4;

				for (x = 0 ; x < width ; x++)
				{
					*dest-- = ((*orig >> shift) & 0x03);
					if (shift == 6)
					{
						shift = 0;
						orig--;
					}
					else shift += 2 ;
				}
			}
		}
		else if (depth == 1)
		{
			shift = ((7 - ((width + 7) & 0x07)) << 1);
			for (n = shift = 0 ; n < height ; n++)
			{
				char * orig = rowpointers[n];
				char * dest = orig + width - 1;
				orig += (width-1)/8;

				for (x = 0 ; x < width ; x++)
				{
					*dest-- = ((*orig >> shift) & 0x01);
					if (shift == 7)
					{
						shift = 0;
						orig--;
					}
					else shift++ ;
				}
			}
		}	
	}
	else
	{
		Bshift = 16-screen->format->Rshift ;
		Gshift =  8-screen->format->Gshift ;
		Rshift =  0-screen->format->Bshift ;

		Bmask  = 0xFF0000 ;
		Gmask  = 0x00FF00 ;
		Rmask  = 0x0000FF ;

		for (n = 0 ; n < screen->format->Rloss ; n++) Rshift++, Rmask <<= 1 ;
		for (n = 0 ; n < screen->format->Gloss ; n++) Gshift++, Gmask <<= 1 ;
		for (n = 0 ; n < screen->format->Bloss ; n++) Bshift++, Bmask <<= 1 ;

		Bmask &= 0xFF0000 ;
		Gmask &= 0x00FF00 ;
		Rmask &= 0x0000FF ;

		for (n = 0 ; n < height ; n++)
		{
			rowpointers[0] = (void *)row ;
			png_read_rows (png_ptr, rowpointers, 0, 1) ;

			ptr = (Uint16*) bitmap->data + n*bitmap->pitch/2;
			orig = row ;
			for (x = 0 ; x < width ; x++)
			{
				ARRANGE_DWORD(orig);

				if ((*orig) & 0x80000000) 
				{
				  *ptr = ((*orig & Rmask) >> Rshift)|
					 ((*orig & Gmask) >> Gshift)|
					 ((*orig & Bmask) >> Bshift)  ;

				  if (!*ptr) (*ptr)++ ;
				}
				else *ptr = 0 ;
				ptr++, orig++ ;
			}
		}
	}

	/* Fin */

	if (!setjmp (png_jmpbuf(png_ptr)))
		png_read_end (png_ptr, 0) ;
	file_close (png) ;
	bitmap->modified = 1 ;
	return bitmap ;
}

/*
 *  FUNCTION : gr_save_png
 *
 *  Save a GRAPH into a PNG file 
 *
 *  PARAMS : 
 *		gr				GRAPH with the image to save
 *		filename		name for the file
 *
 *  RETURN VALUE : 
 *      0 - FAILURE
 *		1 - SUCCESS
 *
 */

int gr_save_png (GRAPH * gr, const char * filename) 
{
	FILE * file = fopen (filename, "wb") ;

	png_structp	png_ptr ;
	png_infop	info_ptr ;
	png_uint_32	k, i ;
	png_bytep	row_pointers[8192] ;
	png_colorp	pal ;
	Uint32      * data, * ptr ;
	Uint16		* orig ;

	if (!file) return(0) ;	

	if (gr->height > 8192 || gr->width > 8192) {
		fclose(file) ;
		return(0) ;
	}

	png_ptr  = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0) ;
	info_ptr = png_create_info_struct  (png_ptr) ;
	
	if (!png_ptr || !info_ptr) 
	{
		fclose(file) ;
		return(0) ;
	}

	/* Error handling... */

	if (setjmp(png_jmpbuf(png_ptr))) 
	{
		fclose (file) ;
		png_destroy_write_struct (&png_ptr, NULL) ;
		return(0) ;
	}

	png_init_io (png_ptr, file) ;

	/* NOTE: NO SUPPORT FOR ANIMATIONS! */

	if (gr->depth == 8)
	{
		/* 8 bits PNG file */
		png_set_IHDR (png_ptr, info_ptr, gr->width,
			gr->height, 8, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
			PNG_FILTER_TYPE_BASE) ;

		pal = (png_colorp) png_malloc (png_ptr, 256*sizeof(png_color)) ;
		if (!pal) 
		{
			fclose (file) ;
			png_destroy_write_struct (&png_ptr, NULL) ;
			return(0) ;
		}

		/* Generate palette info */
		for (k = 0 ; k < 256 ; k++) 
		{
			pal[k].red   = palette[k].r ;
			pal[k].green = palette[k].g ;
			pal[k].blue  = palette[k].b ;
		}
		png_set_PLTE (png_ptr, info_ptr, pal, 256) ;
		png_write_info (png_ptr, info_ptr) ;

		/* no need to rearrange data */
		for (k = 0 ; k < (unsigned)gr->height ; k++)
			row_pointers[k] = (Uint8 *)gr->data + gr->pitch*k ;
		png_write_image (png_ptr, row_pointers) ;				

		/* Free allocated palette... */
		png_free (png_ptr, (png_voidp) pal) ;
	} 
	else 
	{	
		png_set_IHDR (png_ptr, info_ptr, gr->width,
			gr->height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
			PNG_FILTER_TYPE_BASE) ;
		png_write_info (png_ptr, info_ptr) ;

		data = malloc (gr->width * gr->height * 4) ;
		if (!data) 
		{
			fclose (file) ;
			png_destroy_write_struct (&png_ptr, NULL) ;
			return(0) ;
		}
		for (k = 0 ; k < (unsigned)gr->height ; k++) 
		{
			ptr  = data + gr->width * k ; /* uses dword for each pixel! */
			orig = (Uint16 *)gr->data + gr->width * k ;
			row_pointers[k] = (Uint8 *)ptr ;
			for (i = 0 ; i < (unsigned)gr->width ; i++)
			{
				if (*orig == 0 && !(gr->flags & GI_NOCOLORKEY)) 
					*ptr = 0x00000000 ;
				else
				{
					*ptr = ((*orig & 0xf800) >> 8) |
					       ((*orig & 0x07e0) << 5) |
					       ((*orig & 0x001f) << 19)|
					       0xFF000000 ;
				    /* Rearrange data */
					ARRANGE_DWORD(ptr) ;
				}
				orig++ ;
				ptr++  ;
			}
		}
		png_write_image (png_ptr, row_pointers) ;
		free (data) ;
		data = NULL ;
	}

	png_write_end (png_ptr, info_ptr) ;
	fclose (file) ;
	return (1) ;
}
