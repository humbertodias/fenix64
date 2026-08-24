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

#ifndef __SEGMENT_H
#define __SEGMENT_H

#ifdef TARGET_MAC
#include <SDL/SDL_types.h>
#else
#include <SDL_types.h>
#endif

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

/* Un segmento es una zona lineal de datos que puede crecer dinámicamente */

typedef struct _segment
{
	void    * bytes ;
	int	current ;
	int	reserved ;
	int	id ;
}
segment ;

extern segment * segment_new() ;
extern segment * segment_duplicate(segment * s) ;
extern segment * segment_get(int id) ;

/* Devuelven el offset del nuevo dato */
extern int       segment_add_as    (segment * s, Sint32 value, BASETYPE t) ;
extern int       segment_add_dword (segment * s, Sint32 value) ;
extern int       segment_add_word  (segment * s, Sint16 value) ;
extern int       segment_add_byte  (segment * s, Sint8  value) ;
extern int       segment_add_from  (segment * s, segment * from) ;

extern void      segment_dump(segment *) ;
extern void      segment_destroy(segment *) ;
extern void      segment_copy(segment *, int base_offset, int total_length) ;
extern void      segment_alloc (segment * n, int count) ;

extern segment  * globaldata ;
extern segment  * localdata ;

/* Segmentos nombrados (para tipos definidos por el usuario */

extern segment * segment_by_name (int code) ;
extern void      segment_name    (segment * s, int code) ;

#endif
