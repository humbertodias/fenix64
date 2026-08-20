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
 * HISTORY: 0.82 - New enhanced FONT struct
 *			0.81 - Added MAP_HEADER struct and magic header for gfx files
 *          0.74 - Added apptitle and icono for WM functions
 *
 */

#ifndef __GRLIB_ST_H
#define __GRLIB_ST_H

#include "fbm.h"

/* -------------------------------------------------------------------- */
/* Librería gráfica                                                     */
/* -------------------------------------------------------------------- */

enum {
	AlphaBits  = 0,
	RedBits    = 5,
    GreenBits  = 6,
	BlueBits   = 5,

	AlphaShift = RedBits+GreenBits+BlueBits,
	RedShift   = GreenBits+BlueBits,
	GreenShift = BlueBits,
	BlueShift  = 0,

	AlphaMask = ((1 << AlphaBits) - 1) << AlphaShift,
	RedMask   = ((1 << RedBits) - 1) << RedShift,
	GreenMask = ((1 << GreenBits) - 1) << GreenShift,
	BlueMask  = ((1 << BlueBits) - 1) << BlueShift,

	RedBlueMask = RedMask | BlueMask
};


#define RESOLXY(r,x,y) \
	if (LOCDWORD(r, RESOLUTION)>0)              \
	{                                           \
		(x) /= LOCDWORD(r,RESOLUTION) ;         \
		(y) /= LOCDWORD(r,RESOLUTION) ;         \
	}                                           \
	else if (LOCDWORD(r, RESOLUTION)<0)         \
	{                                           \
		(x) *= -LOCDWORD(r,RESOLUTION) ;        \
		(y) *= -LOCDWORD(r,RESOLUTION) ;        \
	}

#define RESOLXYZ(r,x,y,z) \
	if (LOCDWORD(r, RESOLUTION)>0)              \
	{                                           \
		(x) /= LOCDWORD(r,RESOLUTION) ;         \
		(y) /= LOCDWORD(r,RESOLUTION) ;         \
		(z) /= LOCDWORD(r,RESOLUTION) ;         \
	}                                           \
	else if (LOCDWORD(r, RESOLUTION)<0)         \
	{                                           \
		(x) *= -LOCDWORD(r,RESOLUTION) ;        \
		(y) *= -LOCDWORD(r,RESOLUTION) ;        \
		(z) *= -LOCDWORD(r,RESOLUTION) ;        \
	}

/* Tipos y datos globales */

typedef struct _cpoint
{
    short int x ;
    short int y ;
}
CPOINT ;

typedef struct _palette
{
    SDL_Color       rgb[256] ;
    Uint16          colorequiv[256] ;
    int             use;
    struct _palette * next ;
    struct _palette * prev ;
}
PALETTE ;

typedef struct _bitmap
{
    void      * data ;				/* Pointer to the bitmap data at current frame */
    void      * data_start ;		/* Pointer to the bitmap data at first frame */
    Uint32		pitch ;				/* Bytes of distance between bitmap lines */
    Uint32      width ;				/* Width of a bitmap frame in pixels */
	Uint32		widthb ;			/* Width of a bitmap frame in bytes */
    Uint32		height ;			/* Height of a bitmap frame in pixels */
    int         code ;				/* Identifier of the graphic (in the FPG) */
    Uint32		depth ;				/* Bits per pixel (1, 8, 16) */
	Uint32		frames ;			/* Number of frames */
	Uint32		current_frame;		/* Number of frame currently selected */
    char        name[64] ;			/* Name of the graphic */
    CPOINT    * cpoints ;			/* Pointer to the control points ([0] = center) */
    Uint32      ncpoints;			/* Number of control points */
    Uint32      offset ;			/* Offset of the graphic (in the FPG file) */

    PALETTE   * palette ;           /* Palette for 8 bits graphics */

    Sint16    * blend_table ;		/* Pointer to 16 bits blend table if any */

    int         modified ;			/* 1 if bitmap needs analysis */
    int         info_flags ;		/* Analysis result (see bitmap_analize) */

	FBM_SEQUENCE *	sequences;		/* Pointer to sequence array (from FBM file) */
	FBM_KEYFRAME *	keyframes;		/* Pointer to keyframe array (from FBM file) */
	Sint32			max_sequence;	/* Number of maximum sequence index */
	Sint32			max_keyframe;	/* Number of maximum keyframe index */

	/* Information about current animation sequence */

	Uint32		last_frame;
	Uint32		next_time;
	Sint32		current_sequence;
	Sint32		current_keyframe;
	int			end_of_sequence;	/* 1 if the map is at the end of the sequence */

	/* Linked list of all bitmaps in memory */

	struct _bitmap * next;
	struct _bitmap * prev;
}
GRAPH ;

// info_flags bits
#define GI_NOCOLORKEY 1

#define CPOINT_UNDEFINED	32767	/* It's enough if X is set to this value */

enum
{
	CHARSET_ISO8859	= 0,
	CHARSET_CP850 = 1
};

typedef struct _font
{
	int    charset;
	Uint32 bpp;

	struct _glyph
	{
		GRAPH *	bitmap;
		int     xoffset;
		int     yoffset;
		int     xadvance;
		int     yadvance;
	}
	glyph[256] ;

	int maxheight;
	int maxwidth;
}
FONT ;

typedef struct _grlib
{
    GRAPH **	maps ;
    int			map_reserved ;
	char		name[64];
}
GRLIB ;

typedef struct _clipregion
{
    int x ;
    int y ;
    int x2 ;    /* Inclusive */
    int y2 ;
}
REGION ;

typedef struct _scrolldata
{
	int	fileid ;
	int	graphid ;
	int	backid ;
	REGION * region ;
	int	flags ;

	int	x0, y0 ;
	int	posx0, posy0 ;
	int	x1, y1 ;
	int	posx1, posy1 ;
	int	z ;

	INSTANCE * camera ;

	int	ratio ;
	int	speed ;
	REGION * region1 ;
	REGION * region2 ;

	int	active ;

	GRAPH  * graph ;
	GRAPH  * back ;

	struct _scrolldata * follows ;
}
scrolldata ;

typedef struct _keyequiv {
    int                 sdlk_equiv ;
    struct _keyequiv    * next ;
} key_equiv ;

/* Bajo nivel */
/* ---------- */

/* Dibujo de primitivas */
#define DRAWOBJ_LINE	 1
#define DRAWOBJ_RECT	 2
#define DRAWOBJ_BOX		 3
#define DRAWOBJ_CIRCLE	 4
#define DRAWOBJ_FCIRCLE  5
#define DRAWOBJ_CURVE    6

typedef struct _drawing_object
{
	int type;
	int x1;
	int y1;
	int x2;
	int y2;
	int x3;
	int y3;
	int x4;
	int y4;
	int level;

	/* Private */
	int color8;
	int color16;
	int id;
	struct _drawing_object * next;
}
DRAWING_OBJECT;

/* Flags para gr_blit */
/*
#define B_HMIRROR       0x0001
#define B_VMIRROR       0x0002
#define B_TRANSLUCENT   0x0004
#define B_ALPHA         0x0008
#define B_ABLEND		0x0010
#define B_SBLEND		0x0020
#define B_NOCOLORKEY    0x0080
*/
#define B_ALPHA_MASK    0xFF00
#define B_ALPHA_SHIFT   8

/* Graph Mode */
#define MODE_WINDOW         0x0000
#define MODE_16BITS         0x0010
#define MODE_2XSCALE        0x0100
#define MODE_FULLSCREEN     0x0200
#define MODE_DOUBLEBUFFER   0x0400
#define MODE_HARDWARE       0x0800
#define MODE_MODAL          0x1000
#define MODE_FRAMELESS      0x2000

/* CABECERAS DE FICHEROS */
#define MAP_MAGIC "map\x1A\x0D\x0A"
#define M16_MAGIC "m16\x1A\x0D\x0A"
#define M01_MAGIC "m01\x1A\x0D\x0A"
#define PAL_MAGIC "pal\x1A\x0D\x0A"
#define FNT_MAGIC "fnt\x1A\x0D\x0A"
#define FNX_MAGIC "fnx\x1A\x0D\x0A"
#define FPG_MAGIC "fpg\x1A\x0D\x0A"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct
{
	Uint8	magic[8] ;
	Uint16	width ;
	Uint16	height ;
	Uint32	code ;
	Sint8	name[32] ;
}
#ifdef __GNU_C__
__attribute__ ((packed))
#endif
MAP_HEADER ;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
