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
#include <stdio.h>
#include <stdlib.h>

#include "fxc.h"

/* ---------------------------------------------------------------------- */
/* Gestor de constantes                                                   */
/* ---------------------------------------------------------------------- */

static CONSTANT * constants ;
static int constants_used ;
static int constants_reserved ;

void constants_init ()
{
	constants = (CONSTANT *) malloc (sizeof(CONSTANT)*16);
	constants_reserved = 16 ;
	constants_used = 0 ;
}

void constants_alloc (int count)
{
	constants = (CONSTANT *) realloc (constants,
			(constants_reserved += count) * sizeof(CONSTANT)) ;
	if (!constants)
	{
		fprintf (stdout, "constants_alloc: sin memoria\n") ;
		exit (1) ;
	}
}

CONSTANT * constants_search (int code)
{
	int i ;

	for (i = 0 ; i < constants_used ; i++)
		if (constants[i].code == code)
			return &constants[i] ;
	return 0 ;
}

void constants_add (int code, TYPEDEF type, int value)
{
	if (constants_used == constants_reserved)
		constants_alloc (16) ;

	constants[constants_used].code = code ;
	constants[constants_used].type = type ;
	constants[constants_used].value = value ;
	constants_used++ ;
}

void constants_dump ()
{
	int i ;

	printf ("---- %d constants of %d ----\n", constants_used, constants_reserved) ;
	for (i = 0 ; i < constants_used ; i++)
		printf ("%4d: %-16s= %d\n", i, 
			identifier_name(constants[i].code), constants[i].value) ;
}
