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
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "fxc.h"

/* ---------------------------------------------------------------------- */
/* Gestor de procesos y bloques de código. Este módulo contiene funciones */
/* de utilidad para crear procesos y bloques de código así como otras que */
/* se emplean durante y después del compilado.                            */
/* ---------------------------------------------------------------------- */

PROCDEF * mainproc = 0 ;
int procdef_count = 0 ;

int procdef_maxid = -1 ;
PROCDEF * * procs = 0 ;
int procs_allocated = 0 ;

int procdef_getid()
{
	return ++procdef_maxid ;
}

PROCDEF * procdef_new (int typeid, int id)
{
	PROCDEF * proc = (PROCDEF *) malloc (sizeof(PROCDEF)) ;
	int n ;

	if (!proc)
	{
		fprintf (stdout, "procdef_new: sin memoria\n") ;
		exit (1) ;
	}

	proc->pridata = segment_new() ;
	proc->privars = varspace_new() ;
	proc->params  = -1 ;
	proc->defined = 0 ;
	proc->type    = TYPE_DWORD ;

	proc->sentence_count = 0 ;
	proc->sentences      = 0 ;

	if (typeid >= procs_allocated)
	{
		procs_allocated = typeid + 15 ;
		procs = (PROCDEF **) realloc (procs, sizeof(PROCDEF **) * procs_allocated) ;
		if (!procs)
		{
			fprintf (stdout, "procdef_new: sin memoria\n") ;
			exit (1) ;
		}
	}
	proc->typeid     = typeid ;
	proc->identifier = id ;
	procs[typeid]    = proc ;

	for (n = 0 ; n < MAX_PARAMS ; n++) 
		proc->paramtype[n] = TYPE_UNDEFINED ;

	codeblock_init (&proc->code) ;
	procdef_count++ ;
	return proc ;
}

PROCDEF * procdef_search (int id)
{
	int n ;

	for (n = 0 ; n <= procdef_maxid; n++)
	{
		if (procs[n]->identifier == id)
			return procs[n] ;
	}
	return 0 ;
}

PROCDEF * procdef_get (int typeid)
{
	return procs_allocated > typeid ? procs[typeid] : 0 ;
}

void procdef_destroy (PROCDEF * proc)
{
	varspace_destroy (proc->privars) ;
	segment_destroy  (proc->pridata) ;

	procs[proc->typeid] = 0 ;
	free (proc->code.data) ;
	free (proc->code.loops) ;
	free (proc->code.labels) ;
	free (proc) ;

	procdef_count-- ;
}


/* Realiza acciones posteriores al compilado sobre el código:
 * - Convierte saltos de código de etiqueta a offset
 * - Convierte identificador de procesos en CALL o TYPE a typeid */

void program_postprocess ()
{
	int n ;

	for (n = 0 ; n <= procdef_maxid ; n++)
		codeblock_postprocess (&procs[n]->code) ;
}
