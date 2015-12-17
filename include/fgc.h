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
 * FILE        : fgc.h
 * DESCRIPTION : Fenix Graphic Collection headers and documentation
 *
 * HISTORY:      0.85 - first version
 */

#ifndef __FGC_H
#define __FGC_H

#define FGC_VERSION	0x0100
#define FGC_MAGIC   "FenixLibrary\x1A\x0D\x0A"

// FGC files don't use any compression whatsoever, but they
// natively support .gz expansion

	typedef struct
	{
		char		magic[16];		// "FenixLibrary\x1A\x0D\x0A" + '\x00'
		Uint32		version;		// Version code ((major << 16) | minor)
		Uint8		name[64];		// ASCIIZ name of the library
		Uint32		depth;			// 1, 8, 16 or 24
		Uint32		count;			// Number of graphics in the library
		Uint32		palette;		// Offset of the palette
		Uint32		offset[0];		// Offset to each graphic in the file
									// (0 = start of the file, including header, for all offsets)
	}
	FGC_HEADER;

// Palette information follows, but only if the library has a 8 bit depth
// There are 256 RGB triplets with a range of 0-255 for each color
// [768 bytes of optional information]
//
// The loading function should honor the offset present in the header.
// This allows to extend the file format mantaining backward compatibility
//
// The graphics are exactly in the same format as a FBM file with
// the following changes:
//
// - The "header", "version" and "depth" are not present (the header
//   begins with the graphic name, see fbm.h for details)
//
// - The palette is never present if the graphics are 8 bits
//   (there is already one before the graphic information)

#endif


