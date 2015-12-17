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

#ifndef __INSTANCE_ST_H
#define __INSTANCE_ST_H

#define STACK_NO_RETURN_VALUE	0x8000
#define STACK_SIZE_MASK			0x7FFF

/* Instancias. Una instancia se crea a partir de un proceso, pero en
 * realidad es independiente del proceso original */

typedef struct _instance
{
	void             * locdata ;
	void             * pridata ;
	void             * pubdata ;

	int              * code ;
	int              * codeptr ;
	int              * exitcode ;

	struct _procdef  * proc ;

	int              private_size ;
	int              public_size ;

	int				 first_run ;

	/* General list - unsorted */

	struct _instance * next ;
	struct _instance * prev ;

	/* Dirty instances list */

	struct _instance * next_dirty ;
	int                is_dirty ;

	/* Hashed list */

	struct _instance * next_by_priority ;
	struct _instance * prev_by_priority ;

	/* Function support */

/*	struct _instance * waiting_for ; */
	struct _instance * called_by ;

	/* The first integer at the stack is the stack size,
	   with optional NO_RETURN_VALUE mask. The stack contents follows */

	int              * stack ;

	/* For debugging */

	struct _procdef  * inproc ;
	void             * inpridata ;

	int               breakpoint;

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

#define PUBDWORD(a,b) ( *(Sint32 *) ((Uint8 *)(a->pubdata)+b) )
#define PUBWORD(a,b)  ( *(Uint16 *) ((Uint8 *)(a->pubdata)+b) )
#define PUBBYTE(a,b)  ( *(Uint8  *) ((Uint8 *)(a->pubdata)+b) )

#define GLODWORD(b)   ( *(Sint32 *) ((Uint8 *)(globaldata)+b) )
#define GLOWORD(b)    ( *(Uint16 *) ((Uint8 *)(globaldata)+b) )
#define GLOBYTE(b)    ( *(Uint8  *) ((Uint8 *)(globaldata)+b) )

#define FIRST_INSTANCE_ID 65537

#endif
