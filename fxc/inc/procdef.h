/* Fenix - Compilador/intérprete de videojuegos
 * Copyright (C) 1999 José Luis Cebrián Pagüe
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PROCDEF_H
#define __PROCDEF_H

/* Procesos (un "PROCDEF" es, en realidad, simplemente su definición) */

#ifndef __SEGMENT_H
#include "segment.h"
#endif

#ifndef __VARSPACE_H
#include "varspace.h"
#endif

#ifndef __CODEBLOCK_H
#include "codeblock.h"
#endif

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

#define MAX_PARAMS 16

typedef struct _sentence
{
	int file ;
	int line ;
	int col ;
	int offset ;
}
SENTENCE ;

typedef struct _procdef
{
	VARSPACE * privars ;
	segment  * pridata ;

	int typeid ;
	int identifier ;
	int params ;
	int defined ;

	BASETYPE   paramtype[MAX_PARAMS] ;
	BASETYPE   type ;

	CODEBLOCK  code ;

	SENTENCE   * sentences ;
	int          sentence_count ;
}
PROCDEF ;

extern int procdef_count ;

extern int       procdef_getid() ;
extern PROCDEF * procdef_new (int typeid, int identifier) ;
extern PROCDEF * procdef_get (int typeid) ;
extern PROCDEF * procdef_search (int identifier) ;
extern void      procdef_destroy(PROCDEF *) ;

/* Proceso "principal", el primero en definirse y ejecutarse */
extern PROCDEF * mainproc ;

#endif
