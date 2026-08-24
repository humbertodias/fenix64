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

#ifndef __TOKEN_H
#define __TOKEN_H

/* Tokenizador */

/* Tipos de token */
#define IDENTIFIER 1
#define STRING     2
#define NUMBER     3
#define FLOAT      4
#define NOTOKEN    5

extern struct _token
{
	int type ;
	int code ;
	float value ;
} token ;

extern void token_init (const char * source, int file) ;
extern void token_next () ;
extern void token_back () ;

extern int line_count ;
extern int current_file ;
extern int n_files ;
extern char files[][256] ;

#endif
