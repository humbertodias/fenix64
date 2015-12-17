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
 * FILE        : fxc.h
 * DESCRIPTION : Base includes in FXC and some vars and const defines
 *
 * HISTORY:
 *
 */

#ifndef ENGLISH
#define ENGLISH
#endif

#ifdef TARGET_MAC
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include "files.h"
#include "xctype.h"

/* ---------------------------------------------------------------------- */
/* Módulos generales de mantenimiento de datos                            */
/* ---------------------------------------------------------------------- */

#include "typedef.h"
#include "constants.h"
#include "identifiers.h"
#include "xstrings.h"

/* ---------------------------------------------------------------------- */
/* Trucos de portabilidad                                                 */
/* ---------------------------------------------------------------------- */

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    #define ARRANGE_DWORD(x)
    #define ARRANGE_WORD(x)

    #define ARRANGE_DWORDS(x,c)
    #define ARRANGE_WORDS(x,c)
#else
    static __inline__ void DO_Swap16(Uint16 * D) {
    	*D = ((*D<<8)|(*D>>8));
    }

    static __inline__ void DO_Swap32(Uint32 * D) {
    	*D = ((*D<<24)|((*D<<8)&0x00FF0000)|((*D>>8)&0x0000FF00)|(*D>>24));
    }

    #define ARRANGE_DWORD(x)	DO_Swap32(x)
    #define ARRANGE_WORD(x)		DO_Swap16(x)

    #define ARRANGE_DWORDS(x,c) {				\
    	int __n;								\
    	Uint32 * __p = (Uint32 *)(x);			\
    	for (__n = 0 ; __n < (int)(c) ; __n++)	\
    		ARRANGE_DWORD(&__p[__n]);			\
    	}
    #define ARRANGE_WORDS(x,c) {				\
    	int __n;								\
    	Uint16 * __p = (Uint16 *)(x);			\
    	for (__n = 0 ; __n < (int)(c) ; __n++)	\
    		ARRANGE_WORD(&__p[__n]);			\
    	}

#endif

/* ---------------------------------------------------------------------- */
/* Compilador                                                             */
/* ---------------------------------------------------------------------- */

#include "segment.h"
#include "varspace.h"
#include "token.h"
#include "codeblock.h"
#include "procdef.h"
#include "compiler.h"

extern int autoinclude ;	/* Incluye ficheros en el DCB automáticamente */
extern int imports[] ;		/* Códigos de cadena con nombres de imports */
extern int nimports ;		/* Número de imports */

extern char langinfo[64] ;	/* language setting */

/* Funciones para guardar y cargar un fichero DCB */

#include "dcb.h"

extern void dcb_add_file (const char * filename) ;
extern int  dcb_save (const char * filename, int options, const char * stubname) ;
extern void dcb_settype (DCB_TYPEDEF * d, TYPEDEF * t) ;

/* Funciones del sistema (no definidas) */

typedef struct _sysproc
{
	int    code ;
	char * name ;
	char * paramtypes ;
	BASETYPE type ;
	int    params ;
	int    id ;

	/* For sysproc_list */
	struct _sysproc * next;
}
SYSPROC ;

extern int         sysproc_add    (char * name, char * paramtypes, int type, void * func);
extern SYSPROC *   sysproc_get    (int id) ;
extern SYSPROC * * sysproc_getall (int id) ;
extern char    *   sysproc_name   (int code) ;

/* Constantes DIV */

extern void div_init() ;

#include "offsets.h"
#include "pslang.h"

