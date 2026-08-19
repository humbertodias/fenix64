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
#include <string.h>

#include "fxc.h"
#include "messages.c"

/* Fast access sysproc list, by identifier code */

SYSPROC * * sysproc_list = NULL;
int         sysproc_maxid = 0;

/* Este fichero contiene sólo las definiciones de las funciones del sistema */

#define SYSMACRO(a) 0
#include "sysprocs.h"

/*
 *  FUNCTION : sysproc_add
 *
 *  Add a new system function to the internal system function list
 *
 *  PARAMS:
 *		name			Name of the process
 *		paramtypes		String representation of the parameter
 *		type			Type of the returning value
 *		func			Pointer to the function itself or a stub
 *
 *  RETURN VALUE:
 *      Identifier code allocated for the function
 */

int sysproc_add (char * name, char * paramtypes, int type, void * func)
{
	static SYSPROC * last = 0 ;
	static int sysproc_count = 0 ;

	if (!last)
	{
		last = sysprocs ;
		sysproc_count++ ;
		while (last[1].name) last++, sysproc_count++ ;
	}
	last[1].code = last[0].code + 1 ;
	last[1].name = name ;
	last[1].paramtypes = paramtypes ;
	last[1].params = strlen(paramtypes) ;
	last[1].type = type ;
	last[1].id   = 0 ;
	last[1].next = NULL ;
	last++ ;
	sysproc_count++ ;
	if (sysproc_count == MAX_SYSPROCS) compile_error (MSG_TOO_MANY_SYSPROCS) ;

	/* If the fast-access list is already filled, free it to fill it again
	 * in sysproc_get. We should add the new process to the list, but this
	 * is a very rare possibility and we're lazy */

	if (sysproc_list != NULL)
	{
		free (sysproc_list);
		sysproc_list = NULL;
		sysproc_maxid = 0;
	}

	return last->code ;
}

/*
 *  FUNCTION : sysproc_get
 *
 *  Search for a system function by name and return a pointer
 *  to the first ocurrence or NULL if none exists
 *
 *  PARAMS:
 *		id			Unique code of the identifier of the name
 *
 *  RETURN VALUE:
 *      Pointer to the SYSPROC object or NULL if none exists
 */

SYSPROC * sysproc_get (int id)
{
	SYSPROC * s ;

	/* If the table is filled, get the process with direct access */

	if (id < sysproc_maxid)
		return sysproc_list[id];

	/* Fill the table */

	if (sysproc_list == NULL)
	{
		/* Alloc IDs if necessary and get the maximum one */

		for (s = sysprocs ; s->name ; s++)
		{
			if (s->id == 0) s->id = identifier_search_or_add(s->name) ;
			if (s->id > sysproc_maxid) sysproc_maxid = s->id;

			s->next = NULL;
		}

		/* Alloc the table */

		sysproc_maxid = ((sysproc_maxid+1) & ~31) + 32;
		sysproc_list  = (SYSPROC * *) calloc (sysproc_maxid, sizeof(SYSPROC*));
		if (sysproc_list == NULL) abort();

		/* Fill it */

		for (s = sysprocs ; s->name ; s++)
		{
			if (s->id > 0)
			{
				s->next = sysproc_list[s->id];
				sysproc_list[s->id] = s;
			}
		}

		if (id < sysproc_maxid)
			return sysproc_list[id];
	}
	return NULL;
}

/*
 *  FUNCTION : sysproc_getall
 *
 *  Search for a system function by name and return a pointer
 *  to a zero-terminated table with pointers to all ocurrences.
 *  Up to 31 occurences can be found with this function.
 *
 *  PARAMS:
 *		id			Unique code of the identifier of the name
 *
 *  RETURN VALUE:
 *      Pointer to a new SYSPROC table allocated with malloc()
 *      NULL if no process with this id exists
 */

SYSPROC ** sysproc_getall (int id)
{
	SYSPROC * s = sysproc_get(id) ;
	SYSPROC ** table;
	int found = 0 ;

	if (s == NULL) return NULL;

	table = malloc(sizeof(SYSPROC *) * 32) ;
	do
	{
		table[found++] = s;
		s = s->next;
	}
	while (s && found <= 31);
	table[found] = NULL;
	return table ;
}

/*
 *  FUNCTION : sysproc_name
 *
 *  Return the name of a given system function
 *
 *  PARAMS:
 *		code		Internal code of the function
 *
 *  RETURN VALUE:
 *      Pointer to the name or NULL if it was not found
 */

char * sysproc_name (int code)
{
	SYSPROC * s = sysprocs ;

	while (s->name)
	{
		if (s->code == code) return s->name ;
		s++ ;
	}
	return 0 ;
}

