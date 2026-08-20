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

#ifndef __VARSPACE_H
#define __VARSPACE_H

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

/* Un VARSPACE es una zona de definición de variables */

typedef struct _varspace
{
	struct _variable * vars ;
	int	size ;
	int	count ;
	int	reserved ;
	int	last_offset ;

	int	* stringvars ;
	int	stringvar_reserved ;
	int	stringvar_count ;
}
VARSPACE ;

typedef struct _variable
{
	TYPEDEF type ;
	int	code ;
	int	offset ;
}
VARIABLE ;

extern VARSPACE * varspace_new () ;
extern void       varspace_alloc (VARSPACE * n, int count) ;
extern void       varspace_init (VARSPACE * n) ;
extern void       varspace_add (VARSPACE * n, VARIABLE v) ;
extern VARIABLE * varspace_search (VARSPACE * n, int code) ;
extern void       varspace_dump (VARSPACE * n, int indent) ;
extern void       varspace_destroy (VARSPACE * n) ;
extern void	  varspace_varstring (VARSPACE * n, int offset) ;

/* Datos globales y locales */

extern VARSPACE global ;
extern VARSPACE local ;

#endif
