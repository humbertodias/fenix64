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

#ifndef __FILES_ST_H
#define __FILES_ST_H

#include <stdio.h>

/* Funciones de acceso a ficheros */
/* ------------------------------ */

/* Ahora mismo son casi wrappers de stdio.h, pero en el futuro
 * el tipo "file" puede ser una estructura y las funciones,
 * ofrecer soporte transparente para ficheros PAK, etc. */

#define F_XFILE  1
#define F_FILE   2
#define F_GZFILE 3

#include <zlib.h>

#ifndef MAX_PATH
#define MAX_PATH	260
#endif

#ifdef TARGET_Win32
#define PATH_SEP "\\"
#define PATH_BACKSLASH
#else
#define PATH_SEP "/"
#define PATH_SLASH
#endif


typedef struct
{
    int     type ;

    FILE *  fp ;
    gzFile  gz ;
    int     n ;
    int     error ;
	char	name[MAX_PATH];
	long    pos ;
	int     eof ;
}
file ;

#endif
