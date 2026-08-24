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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fxc.h"

/* ---------------------------------------------------------------------- */
/* Este módulo contiene funciones de utilidad para crear e ir rellenando  */
/* varspaces (tablas con identificador, offset y tipo de cada VARIABLE)   */
/* y segmentos de datos                                                   */
/* ---------------------------------------------------------------------- */

VARSPACE global, local ;

void varspace_dump (VARSPACE * n, int indent)
{
	int i, t, to ;
	char buffer[128] ;

	// if (!indent) printf ("* %d vars of %d \n", n->count, n->reserved) ;
	for (i = 0 ; i < n->count ; i++)
	{
		if (i < n->count-1)
			to = n->vars[i+1].offset - 1 ;
		else
			to = n->last_offset - 1 ;
		printf ("[%04d:%04d]\t", n->vars[i].offset, to) ;
		for (t = 0 ; t < indent ; t++) printf (" + ") ;
		typedef_describe (buffer, n->vars[i].type) ;
		printf ("%s %s", buffer, identifier_name(n->vars[i].code)) ;
		if (typedef_is_struct(n->vars[i].type))
		{
			printf (":\n") ;
			varspace_dump (typedef_members(n->vars[i].type), indent+1) ;
		}
		else	printf ("\n") ;
	}
}

VARSPACE * varspace_new ()
{
	VARSPACE * v = (VARSPACE *) malloc (sizeof(VARSPACE)) ;

	if (!v)
	{
		compile_error ("varspace_new: out of memory\n") ;
	}
	varspace_init (v) ;
	return v ;
}

void varspace_destroy (VARSPACE * v)
{
	free (v->vars) ;
	free (v) ;
}

void varspace_init (VARSPACE * n)
{
	n->vars = (VARIABLE *) malloc (sizeof(VARIABLE) * 16) ;
	n->reserved = 16 ;
	n->count = 0 ;
	n->size = 0 ;
	n->stringvars = 0 ;
	n->stringvar_reserved = 0 ;
	n->stringvar_count = 0 ;
	if (!n->vars) compile_error ("varspace_init: out of memory\n") ;
}

void varspace_varstring (VARSPACE * n, int offset)
{
	if (n->stringvar_reserved == n->stringvar_count)
	{
		n->stringvars = (int *) realloc (n->stringvars,
				(n->stringvar_reserved+=16)*sizeof(int)) ;
		if (!n->stringvars) compile_error ("varspace_varstring: out of memory\n") ;
	}
	n->stringvars[n->stringvar_count++] = offset ;
}

void varspace_alloc (VARSPACE * n, int count)
{
	n->vars = (VARIABLE *) realloc (n->vars, sizeof(VARIABLE) * (n->reserved += count)) ;
	if (!n->vars) compile_error ("varspace_alloc: out of memory\n") ;
}

void varspace_add (VARSPACE * n, VARIABLE v)
{
	if (n->count == n->reserved)
		varspace_alloc (n, 16) ;
	n->vars[n->count++] = v ;
	n->size += typedef_size(v.type) ;
}

VARIABLE * varspace_search (VARSPACE * n, int code)
{
	int i ;

	for (i = 0 ; i < n->count ; i++)
		if (n->vars[i].code == code)
			return &n->vars[i] ;
	return 0 ;
}

