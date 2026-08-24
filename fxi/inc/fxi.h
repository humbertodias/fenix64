/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.82
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
 * FILE        : fxi.h
 * DESCRIPTION : Base includes in FXI and some vars and const defines
 *
 * HISTORY:
 *
 */

/*
 *  VERSION
 */

#define VERSION "FXI 0.83 " __DATE__ " " __TIME__

/*
 *  HEADER FILES
 */

#include <SDL.h>
#include "files.h"
#include "xctype.h"

/*
 *  CONSTANTS
 */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#include "typedef.h"
#if 0
#define TYPE_DWORD  0
#define TYPE_WORD   1
#define TYPE_BYTE   2
#define TYPE_STRING 3
#define TYPE_FLOAT  4
#define TYPE_POINTER 19
#endif
	
/*
 *  ENDIANESS TRICKS
 */

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define ARRANGE_DWORD(x)
#define ARRANGE_WORD(x)
#else

static __inline__ void DO_Swap16(Uint16 * D) {
	*D = ((*D<<8)|(*D>>8));
}

static __inline__ void DO_Swap32(Uint32 * D) {
	*D = ((*D<<24)|((*D<<8)&0x00FF0000)|((*D>>8)&0x0000FF00)|(*D>>24));
}

#define ARRANGE_DWORD(x)	DO_Swap32(x)
#define ARRANGE_WORD(x)		DO_Swap16(x)
#endif

#define ARRANGE_DWORDS(x,c) {			\
	int __n;							\
	Uint16 * __p = (Uint32 *)(x);		\
	for (__n = 0 ; __n < (c) ; __n++)	\
		ARRANGE_DWORD(&__p[__n]);		\
	}
#define ARRANGE_WORDS(x,c) {			\
	int __n;							\
	Uint16 * __p = (Uint16 *)(x);		\
	for (__n = 0 ; __n < (c) ; __n++)	\
		ARRANGE_WORD(&__p[__n]);		\
	}

#ifndef WIN32
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#endif

/*
 *  GLOBAL VARIABLES
 */

extern int debug ;              /* 1 = Actovate debug options       */
extern int fxi ;                /* 1 = EXE module is FXI.EXE        */

/*
 *  DEBUG FLAGS
 */

extern int report_audio ;
extern int report_string ;
extern int report_graphics ;

#include "xstrings.h"
#include "offsets.h"
#include "pslang.h"
#include "instance.h"
#include "i_procdef.h"
#include "grlib.h"
#include "sound.h"
#include "flic.h"

extern void   do_exit       () ;
extern int    dcb_load      (const char * filename) ;
extern char * getid         (unsigned int code) ;
extern int    path_find     (GRAPH * bitmap, int sx, int sy, int dx, int dy, int options) ;
extern int    path_get      (int * x, int * y) ;
extern int    path_set_wall (int n) ;

/* Symbol table management (import/export functions) */

extern void   fnc_init();
extern void   fnc_export (const char * name, void * addr);
extern void * fnc_import (const char * name);
extern void   fnc_show_information();

/* Save/load with type information */

#include "dcb.h"
#include "files_st.h"

extern int savetypes (file * file, void * data, DCB_TYPEDEF * var, int nvars);
extern int loadtypes (file * file, void * data, DCB_TYPEDEF * var, int nvars);
