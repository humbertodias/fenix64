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
 * FILE        : fpl.h
 * DESCRIPTION : Fenix Palette headers and documentation
 *
 * HISTORY:      0.85 - first version
 */

#ifndef __FPL_H
#define __FPL_H

#define FPL_VERSION	0x0100

#define FPL_VALID_DEPTH(a)  ((a) == 8)


// FPL files don't use any compression whatsoever, but they
// natively support .gz expansion

	typedef struct
	{
		char		magic[16];		// "FenixPalette\x1A\x0D\x0A" + '\x00'
		Uint32		version;		// Version code ((major << 16) | minor)
		Uint32		depth;			// currently supports only 8 bits palettes
	}
	FPL_HEADER;

    #define FPL_MAGIC	"FenixPalette\x1A\x0D\x0A"

// Palette information follows. Only 8 bpp color palettes supported.
// There are 256 RGB triplets with a range of 0-255 for each color
// [768 bytes of information]

// Prototipos

int fpl_load_from (file * fp);
int fpl_save_to (file * fp);
int fpl_save (const char * filename);
int fpl_load (const char * filename);


#endif

