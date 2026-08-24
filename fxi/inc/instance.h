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

#ifndef __INSTANCE_H
#define __INSTANCE_H

#include <instance_st.h>
#include "i_procdef_st.h"

/* Instancias. Una instancia se crea a partir de un proceso, pero en
 * realidad es independiente del proceso original */

extern void      * globaldata ;
extern void      * localdata ;

extern int       local_size ;

extern INSTANCE  * first_instance ;
extern INSTANCE  * last_instance ;

extern int must_exit ;

extern int        instance_getid        () ;
extern INSTANCE * instance_get          (int id) ;
extern INSTANCE * instance_getfather    (INSTANCE * i) ;
extern INSTANCE * instance_getson       (INSTANCE * i) ;
extern INSTANCE * instance_getbigbro    (INSTANCE * i) ;
extern INSTANCE * instance_getsmallbro  (INSTANCE * i) ;
extern INSTANCE * instance_new          (PROCDEF * proc, INSTANCE * father) ;
extern INSTANCE * instance_duplicate    (INSTANCE * i) ;
extern void       instance_destroy      (INSTANCE * r) ;
extern void       instance_dump         (INSTANCE * father, int indent) ;
extern void       instance_dump_all     () ;
extern void		  instance_posupdate    (INSTANCE * i) ;
extern int		  instance_poschanged   (INSTANCE * i) ;

/* Las siguientes funciones son el punto de entrada del intérprete */

extern int        instance_go     (INSTANCE * r) ;
extern void       instance_go_all () ;

#endif
