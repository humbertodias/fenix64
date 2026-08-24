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

#ifndef __CONSTANTS_H
#define __CONSTANTS_H
/* Gestor de constantes */

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

typedef struct _constant
{
	int		code ;
	int		value ;
	TYPEDEF type ;
}
CONSTANT ;

extern void constants_init () ;
extern void constants_dump () ;
extern void constants_add (int code, TYPEDEF type, int value) ;
extern CONSTANT * constants_search (int code) ;

#endif
