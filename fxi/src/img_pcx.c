/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.8
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
 * FILE        : img_pcx.c
 * DESCRIPTION : PCX Loading functions
 *
 * HISTORY:
 *
 */

/*
 *	INCLUDES
 */

#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

#include "fxi.h"

/*
 *	GLOBAL VARIABLES
 */

extern SDL_Surface * screen ;

/*
 *	TYPE DEFINITIONS
 */

typedef struct {
	Uint8	Manufacturer;
	Uint8	Version;
	Uint8	Encoding;
	Uint8	BitsPerPixel;
	Sint16	Xmin, Ymin, Xmax, Ymax;
	Sint16	HDpi, VDpi;
	Uint8	Colormap[48];
	Uint8	Reserved;
	Uint8	NPlanes;
	Sint16	BytesPerLine;
	Sint16	PaletteInfo;
	Sint16	HscreenSize;
	Sint16	VscreenSize;
	Uint8	Filler[54];
} PCXheader ;

/*
 *  FUNCTION : gr_read_pcx
 *
 *  Returns GRAPH newly created from FILENAME.  GRAPH is created in LIB 0
 *
 *  PARAMS:
 *      CONST CHAR * filename: File to be read
 *
 *  RETURN VALUE: 
 *      pointer to the newly created GRAPH
 *
 */

GRAPH * 
gr_read_pcx (const char * filename)
{
	PCXheader header ;
	file *    file ;
	int       width, height, x, y, p, count ;
	GRAPH *   bitmap ;
	Uint8 *   ptr, ch ;
	
	file = file_open (filename, "rb") ;
	if (!file) gr_error ("%s: Could not open the file\n", filename) ;
	
	file_read (file, &header, sizeof(header)) ;

	/* Arrange the data for big-endian machines */
	ARRANGE_WORD (&header.Xmax);
	ARRANGE_WORD (&header.Xmin);
	ARRANGE_WORD (&header.Ymax);
	ARRANGE_WORD (&header.Ymin);
	ARRANGE_WORD (&header.BytesPerLine);
	ARRANGE_WORD (&header.PaletteInfo);
	ARRANGE_WORD (&header.HDpi);
	ARRANGE_WORD (&header.VDpi);
	ARRANGE_WORD (&header.HscreenSize);
	ARRANGE_WORD (&header.VscreenSize);

	width  = header.Xmax - header.Xmin + 1 ;
	height = header.Ymax - header.Ymin + 1 ;
	bitmap = bitmap_new (0, width, height, header.BitsPerPixel == 8 ? 8:16) ;
	if (!bitmap) gr_error ("%s: Could not allocate required memory\n", filename) ;

	assert (width <= header.BytesPerLine) ;
	
	if (header.BitsPerPixel == 8) {
		for (y = 0 ; y < height ; y++)
		for (p = 0 ; p < header.NPlanes ; p++) {
			ptr = (Uint8 *)bitmap->data + bitmap->pitch * y ;
			for (x = 0 ; x < header.BytesPerLine ; ) {
				if (file_read (file, &ch, 1) < 1)
					gr_error ("%s: Truncated file", filename) ;
				if ((ch & 0xC0) == 0xC0) {
					count = (ch & 0x3F) ;
					file_read (file, &ch, 1) ;
				} else {
					count = 1 ;
					}
				while (count--) {
					*ptr = ch ;
					x++ ;
					ptr += header.NPlanes ;
				}
			}
		}
	} else {
		gr_error ("%s: Non supported color depth\n", filename) ;
	}
	
	bitmap->modified = 1 ;
	return bitmap ;
}

