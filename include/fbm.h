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
 * FILE        : fbm.h
 * DESCRIPTION : Fenix Bitmap headers and documentation
 *
 * HISTORY:      0.85 - first version
 */

#ifndef __FBM_H
#define __FBM_H

#define FBM_VERSION	0x0100

#define FBM_VALID_DEPTH(a)  ((a) == 8 || (a) == 16 || (a) == 24 || (a) == 1)
#define FBM_MAX_WIDTH		8192
#define FBM_MAX_HEIGHT		8192


// FBM files don't use any compression whatsoever, but they
// natively support .gz expansion

	typedef struct
	{
		char		magic[16];		// "FenixBitmap \x1A\x0D\x0A" + '\x00'
		Uint32		version;		// Version code ((major << 16) | minor)
		Uint32		depth;			// 1, 8, 16 or 24
	}
	FBM_FILE_HEADER;

	typedef struct
	{
		Uint8		name[64];		// ASCIIZ name of the graphic
		Uint32		width;			// Width in pixels
		Uint32		height;			// Height in pixels
		Uint32		flags;			// See enum FBM_FLAGS
		Uint32		code;			// Graphics code (for library index)
		Uint32		max_frame;		// Number of different frames, less 1 (0 if only 1 frame)
		Uint32		max_sequence;	// Number of sequences, less 1
		Uint32		max_keyframe;	// Number of keyframes, less 1
		Uint32		max_point;		// Maximum index of all the control points
		Uint32		points;			// Number of control points present in file
	}
	FBM_HEADER;

#	define FBM_MAGIC	"FenixBitmap \x1A\x0D\x0A"

// Sequence information follows, as many as the "max_frame" values + 1
// [44 bytes for each sequence] : see FBM_SEQUENCE

	typedef struct
	{
		Uint8	name[32];		// Name of the animation sequence, i.e. "Walking"
		Uint32	first_keyframe;	// First keyframe in sequence
		Uint32	last_keyframe;	// Last keyframe in sequence
		Sint32	next_sequence;	// Next sequence to play after this one ends
	}
	FBM_SEQUENCE;

// Keyframe information follows, as many as the "max_keyframe" value + 1
// [12 bytes for each keyframe] : see FBM_KEYFRAME

	typedef struct
	{
		Uint32	frame;			// Number of frame to show
		Uint32	angle;			// Frame to add to the one used by the process
		Uint32	flags;			// Flags to XOR to the ones used by the process
		Uint32	pause;			// Pause for the next frame, in ms
	}
	FBM_KEYFRAME;

// Control point information follows, as many as the "points" value
// (undefined control points are not present in the file at all)
// [12 bytes for each control point] : see FBM_CONTROL_POINT

	typedef struct
	{
		Uint32	index;			// 0 for center, >= 1 otherwise
		Sint32	x;				// Does not need to be inside
		Sint32	y;				// Does not need to be inside
	}
	FBM_CONTROL_POINT;

// Palette information follows, but only if the graphic has a 8 bit depth
// There are 256 RGB triplets with a range of 0-255 for each color
// [768 bytes of optional information]

// Graphic data follows, uncompressed and with left->right, top->bottom order
// Each frame counts as an additional graphic of the same size

// Graphics with depth = 24 are stored as RGB888 with Intel order
// Graphics with depth = 16 are stored as RGB565 with Intel order
// Graphics with depth = 1  are padded to the byte boundary each row


enum FBM_FLAGS
{
	FBM_NOCOLORKEY	= 1,		// No color equals exactly 0
};

#endif



