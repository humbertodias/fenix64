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

#ifndef __I_PROCDEF_ST_H
#define __I_PROCDEF_ST_H

#include "instance_st.h"

/* Definiciones de procesos, mucho más simple que en el compilador */

typedef struct _procdef
{
	int * pridata ;
	int * code ;
	int * strings ;
	int private_size ;
	int code_size ;
	int string_count ;
	int params ;
	int id ;
	int type ;
}
PROCDEF ;

/* Funciones del sistema */

typedef int SYSFUNC (INSTANCE *, int *) ;
typedef struct _sysproc
{
	int       code ;
	char    * name ;
	char    * paramtypes ;
	int       type ;
	int       params ;
	SYSFUNC * func ;
	int       id ;
}
SYSPROC ;

#endif
