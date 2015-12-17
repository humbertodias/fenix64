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
 * FILE        : dirs.h
 * DESCRIPTION : Base include for directory functions
 *
 * HISTORY:
 *
 */

#ifndef __DIRS_H
#define __DIRS_H

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <errno.h>

extern char *	base_dir ;

#ifdef WIN32
extern int		base_drive ;
#endif

extern char *   dir_path_convert(const char *path) ;

extern char *	dir_current(void) ;
extern int		dir_change(const char *dir) ;
extern int		dir_create(const char *dir) ;
extern int		dir_delete(const char *dir) ;
extern int		dir_deletefile(const char *filename) ;

#endif
