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
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include "fxc.h"

/* ---------------------------------------------------------------------- */
/* Gestor de cadenas                                                      */
/* ---------------------------------------------------------------------- */

char * string_mem = 0 ;
int    string_allocated = 0 ;
int    string_used = 0 ;

int  * string_offset = 0 ;
int    string_max = 0 ;
int    string_count = 0 ;

int    autoinclude = 0 ;

void string_init ()
{
	string_mem = (char *) malloc (4096) ;
	string_allocated = 4096 ;
	string_used = 1 ;
	string_count = 1 ;
	string_offset = (int *) malloc(1024 * sizeof(int)) ;
	string_max = 1024 ;
	string_mem[0] = 0 ;
	string_offset[0] = 0 ;
}

void string_alloc (int bytes)
{
	string_mem = (char *) realloc (string_mem, string_allocated += bytes) ;
	if (!string_mem)
	{
		fprintf (stdout, "string_alloc: sin memoria\n") ;
		exit(1) ;
	}
}

void string_dump ()
{
	int i ;
	printf ("---- %d strings ----\n", string_count) ;
	for (i = 0 ; i < string_count ; i++)
		printf ("%4d: %s\n", i, string_mem + string_offset[i]) ;
}

int string_new (const char * text)
{
	int len = strlen(text)+1 ;

	if (string_count == string_max)
	{
		string_max += 1024 ;
		string_offset = (int *) realloc (string_offset, 
			string_max * sizeof(int)) ;
		if (string_offset == 0)
		{
			fprintf (stdout, "Demasiadas cadenas\n") ;
			exit(1) ;
		}
	}

	while (string_used+len >= string_allocated)
		string_alloc (1024) ;

	string_offset[string_count] = string_used ;
	strcpy (string_mem + string_used, text) ;
	string_used += len ;
	return string_count++ ;
}

int string_compile (const char ** source)
{
	char c = *(*source)++, conv ;
	const char * ptr ;

	if (string_count == string_max)
	{
		string_max += 1024 ;
		string_offset = (int *) realloc (string_offset, string_max * sizeof(int)) ;
		if (string_offset == 0)
		{
			fprintf (stdout, "Demasiadas cadenas\n") ;
			exit(1) ;
		}
	}

	string_offset[string_count] = string_used ;

	while (*(*source))
	{
		if (*(*source) == c)  // Termina la string?
		{
			(*source)++ ;
			if (*(*source) == c) // Comienza una nueva? (esto es para strings divididas)
			{
				string_mem[string_used++] = c ;
				(*source)++ ;
			}
			else
			{
			    // Elimino todos los espacios para buscar si hay otra string, esto es para strings divididas
				ptr = (*source) ;
				while (ISSPACE(*ptr))
				{
					if (*ptr == '\n') line_count++ ;
					ptr++ ;
				}
				// Si despues de saltar todos los espacios, no tengo un delimitador de string, salgo
				if (*ptr != c) {
				    (*source) = ptr; // Fix: Splinter, por problema con numeracion de lineas
				    break ;
				}

				// Obtengo delimitador de string, me posiciono en el caracter siguiente, dentro de la string
				(*source) = ptr+1 ;
				continue ;
			}
		}
		else if (*(*source) == '\n')
		{
			line_count++ ;
			string_mem[string_used++] = '\n' ;

			(*source)++ ;
		}
		else
		{
			conv = convert(*(*source)) ;
			string_mem[string_used++] = conv ;

			(*source)++ ;
		}

		if (string_used >= string_allocated)
			string_alloc (1024) ;
	}

	string_mem[string_used++] = 0 ;
	if (string_used >= string_allocated)
		string_alloc (1024) ;

	/* Hack: añade el posible fichero al DCB */

	if (autoinclude) {
    	if (strchr(string_mem + string_offset[string_count], '\\') ||
	        strchr(string_mem + string_offset[string_count], '.'))
	    {
    		dcb_add_file (string_mem + string_offset[string_count]) ;
    	}
    }

	return string_count++ ;
}

const char * string_get (int code)
{
	assert (code < string_count && code >= 0) ;
	return string_mem + string_offset[code] ;
}

void string_save (file * fp)
{
	file_writeUint32 (fp, &string_count) ;
	file_writeUint32 (fp, &string_allocated) ;
	if (string_count)
	{
		ARRANGE_DWORDS (string_offset, string_count);
		file_write (fp,  string_offset, 4 * string_count) ;
		ARRANGE_DWORDS (string_offset, string_count);
	}
	if (string_used)
	{
		file_write (fp, string_mem, string_used) ;
	}
}

void string_load (file * fp)
{
	file_readUint32 (fp, &string_count) ;
	file_readUint32 (fp, &string_used) ;

	if (string_count > string_max)
	{
		string_max = ((string_count + 1024) & 1023) ;
		string_offset = (int *) realloc (string_offset,
			string_max * sizeof(int)) ;
	}
	
	file_read (fp, string_offset, 4 * string_count) ;
	ARRANGE_DWORDS (string_offset, string_count);

	if (string_used > string_allocated)
	{
		string_allocated = string_used ;
		string_mem = (char *) realloc (string_mem, string_used) ;
	}
	file_read (fp, string_mem, string_used) ;
}
