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

#ifndef __INSTANCE_ST_H
#define __INSTANCE_ST_H

/* Instancias. Una instancia se crea a partir de un proceso, pero en
 * realidad es independiente del proceso original */

typedef struct _instance
{
	void             * locdata ;
	void             * pridata ;
	int              * code ;
	int              * codeptr ;

	struct _procdef  * proc ;

	int              private_size ;
	int				 first_run ;

	struct _instance * next ;
	struct _instance * prev ;
}
INSTANCE ;

//#ifndef __I_PROCDEF_H
//#include "i_procdef.h"
//#endif

/* Macros para acceder a datos locales o privados de una instancia */
#define LOCDWORD(a,b) ( *(Sint32 *) ((Uint8 *)(a->locdata)+b) )
#define LOCWORD(a,b)  ( *(Uint16 *) ((Uint8 *)(a->locdata)+b) )
#define LOCBYTE(a,b)  ( *(Uint8  *) ((Uint8 *)(a->locdata)+b) )
#define PRIDWORD(a,b) ( *(Sint32 *) ((Uint8 *)(a->pridata)+b) )
#define PRIWORD(a,b)  ( *(Uint16 *) ((Uint8 *)(a->pridata)+b) )
#define PRIBYTE(a,b)  ( *(Uint8  *) ((Uint8 *)(a->pridata)+b) )
#define GLODWORD(b)   ( *(Sint32 *) ((Uint8 *)(globaldata)+b) )
#define GLOWORD(b)    ( *(Uint16 *) ((Uint8 *)(globaldata)+b) )
#define GLOBYTE(b)    ( *(Uint8  *) ((Uint8 *)(globaldata)+b) )

#define FIRST_INSTANCE_ID 65537

#endif
