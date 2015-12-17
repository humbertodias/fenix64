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

#ifndef __TYPEDEF_H
#define __TYPEDEF_H

#include <typedef_st.h>

/* Tipos de dato */

extern TYPEDEF      typedef_new      (BASETYPE type) ;
extern TYPEDEF      typedef_enlarge  (TYPEDEF base) ;
extern TYPEDEF      typedef_reduce   (TYPEDEF base) ;
extern int          typedef_size     (TYPEDEF t) ;
extern int          typedef_subsize  (TYPEDEF t, int c) ;
extern void         typedef_describe (char * buffer, TYPEDEF t) ;
extern TYPEDEF      typedef_pointer  (TYPEDEF to) ;
extern int			typedef_tcount   (TYPEDEF t) ;
extern int			typedef_is_equal (TYPEDEF a, TYPEDEF b);

extern TYPEDEF    * typedef_by_name  (int code) ;
extern void         typedef_name     (TYPEDEF t, int code) ;

extern int			mntype			 (TYPEDEF type, int accept_structs);

#endif
