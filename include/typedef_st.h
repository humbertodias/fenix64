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

#ifndef __TYPEDEF_ST_H
#define __TYPEDEF_ST_H

/* Tipos de dato */

typedef enum {
	TYPE_UNDEFINED = 0,

	TYPE_DWORD   = 1,
	TYPE_WORD,
	TYPE_BYTE,
	TYPE_FLOAT   = 8,
	TYPE_STRING  = 16,
	TYPE_ARRAY,
	TYPE_STRUCT,
	TYPE_POINTER
}
BASETYPE ;

typedef struct _typechunk
{
	BASETYPE   type ;
	int	   count ;	/* Para type == TYPE_ARRAY */
}
TYPECHUNK ;

#define MAX_TYPECHUNKS 6

typedef struct _typedef
{
	TYPECHUNK	   chunk[MAX_TYPECHUNKS] ;
	int		   depth ;
	struct _varspace * varspace ;
}
TYPEDEF ;

#define typedef_is_numeric(t)   (t.chunk[0].type < 16)
#define typedef_is_integer(t)   (t.chunk[0].type < 8)
#define typedef_is_float(t)     (t.chunk[0].type == TYPE_FLOAT)
#define typedef_is_string(t)    (t.chunk[0].type == TYPE_STRING)
#define typedef_is_struct(t)    (t.chunk[0].type == TYPE_STRUCT)
#define typedef_is_array(t)     (t.chunk[0].type == TYPE_ARRAY)
#define typedef_is_pointer(t)   (t.chunk[0].type == TYPE_POINTER)
#define typedef_count(t)        (t.chunk[0].count)
#define typedef_base(t)         (t.chunk[0].type)
#define typedef_members(t)      (t.varspace)

#endif
