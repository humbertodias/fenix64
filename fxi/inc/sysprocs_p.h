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

/*
 * FILE        : sysprocs_p.h
 * DESCRIPTION : Defines prototipes for internal Fenix system functions
 */

#ifndef __SYSPROCS_P_H
#define __SYSPROCS_P_H

/* CD Playing (f_cd.c) */

extern int fxi_cd_drives		(INSTANCE * my, int * params);
extern int fxi_cd_status		(INSTANCE * my, int * params);
extern int fxi_cd_name			(INSTANCE * my, int * params);
extern int fxi_cd_getinfo		(INSTANCE * my, int * params);
extern int fxi_cd_play			(INSTANCE * my, int * params);
extern int fxi_cd_playtracks	(INSTANCE * my, int * params);
extern int fxi_cd_eject			(INSTANCE * my, int * params);
extern int fxi_cd_pause			(INSTANCE * my, int * params);
extern int fxi_cd_resume		(INSTANCE * my, int * params);
extern int fxi_cd_stop			(INSTANCE * my, int * params);
extern int fxi_cd_numtracks     (INSTANCE * my, int * params);
extern int fxi_cd_getcurtrack   (INSTANCE * my, int * params);

/* Array sort functions (f_sort.c) */

extern int fxi_sort				(INSTANCE * my, int * params);
extern int fxi_sort_n			(INSTANCE * my, int * params);
extern int fxi_ksort			(INSTANCE * my, int * params);
extern int fxi_ksort_n			(INSTANCE * my, int * params);

/* Background load functions (f_bgload.c) */

extern int fxi_bgload_fpg		(INSTANCE * my, int * params);
extern int fxi_bgload_fgc		(INSTANCE * my, int * params);


/* Joystick functions (f_joystick.c) */

extern int fxi_joy_num			(INSTANCE * my, int * params) ;
extern int fxi_joy_name			(INSTANCE * my, int * params) ;
extern int fxi_joy_select		(INSTANCE * my, int * params) ;
extern int fxi_joy_buttons		(INSTANCE * my, int * params) ;
extern int fxi_joy_axes			(INSTANCE * my, int * params) ;
extern int fxi_joy_get_button	(INSTANCE * my, int * params) ;
extern int fxi_joy_get_position	(INSTANCE * my, int * params) ;

extern int fxi_joy_buttons_specific		    (INSTANCE * my, int * params) ;
extern int fxi_joy_axes_specific			(INSTANCE * my, int * params) ;
extern int fxi_joy_get_button_specific	    (INSTANCE * my, int * params) ;
extern int fxi_joy_get_position_specific	(INSTANCE * my, int * params) ;

#endif
