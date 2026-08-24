/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.82
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

/*
 * FILE        : dirs.c
 * DESCRIPTION : directory functions
 *
 * HISTORY:
 *
 */

#include "fxi.h"
#include "dirs.h"
#include "xstrings.h"
#include <string.h>
#include <malloc.h>

#ifdef WIN32
int base_drive ;
#endif

char * base_dir ;

/*
 *  FUNCTION : dir_path_convert
 *
 *  Convert a path to the valid OS format
 *
 *  PARAMS : 
 *		char * path:	path to convert
 *
 *  RETURN VALUE : 
 *      char *:			converted path
 *
 */

char * dir_path_convert(const char * dir) {
	
	char *c,*p ;

	p = strdup(dir) ;
	c = p ;
	// Convert characters
	while (*p) {
#ifdef WIN32
		if (*p=='/') *p='\\' ;
#else
		if (*p=='\\') *p='/' ;
#endif
		p++ ;
		}
	return c;

}


/*
 *  FUNCTION : dir_current
 *
 *  Retrieve current directory
 *
 *  PARAMS : 
 *
 *  RETURN VALUE : 
 *      STRING ID pointing to a system string with the current dir
 *
 */

#ifdef WIN32

char * dir_current(void) {

  char dir[1024] ; /* buffer to the directory */
  char * c ;
  c = getcwd(dir,1024) ;
  return strdup(c) ;

}

#else

char * dir_current(void) {
  return strdup(".");
}

#endif

/*
 *  FUNCTION : dir_change
 *
 *  Retrieve current directory
 *
 *  PARAMS : 
 *		char * dir:		the new current directory
 *
 *  RETURN VALUE : 
 *		0			- FAILURE
 *		NON_ZERO	- SUCCESS
 *
 */

int dir_change(const char * dir) {
	
	int r ;
	char *c ;
	
	c = dir_path_convert(dir) ;		
	r = chdir(c) ;
	free(c) ;
	return r ;

}


/*
 *  FUNCTION : dir_create
 *
 *  Retrieve current directory
 *
 *  PARAMS : 
 *		char * dir:		the directory to create
 *
 *  RETURN VALUE : 
 *		0			- FAILURE
 *		NON_ZERO	- SUCCESS
 *
 */

int dir_create(const char * dir) {
	
	char *c ;
	int r ;

	c = dir_path_convert(dir) ;	
	r = mkdir(c) ;
	free(c) ;
	return r ;

}

/*
 *  FUNCTION : dir_delete
 *
 *  Retrieve current directory
 *
 *  PARAMS : 
 *		char * dir:		the directory to delete
 *
 *  RETURN VALUE : 
 *		0			- FAILURE
 *		NON_ZERO	- SUCCESS
 *
 */

int dir_delete(const char * dir) {
	
	char *c ;
	int r ;

	c = dir_path_convert(dir) ;	
	r = rmdir(c) ;
	free(c) ;
	return r ;

}
