/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.82
 *  Last stable release   :
 *  Project documentation : http://fenix.sourceforge.net
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
 * FILE        : i_func.h
 * DESCRIPTION : Implements FENIX language function handlers
 *
 * HISTORY:  0.83 - Added FILE_EXISTS 
 *			 0.83 - Added DIRECTORY functions
 *			 0.83 - Added SAVE_PNG, new string functions
 *           0.82 - Added some new functions
 *			 0.81 - Moved MOD to SONG in soundlib
 *			 0.81 - Introduced some casts to remove Visual C++ warnings
 *           0.80 - div_ prefix substituted with fxi_
 *           0.80 - Added DLL support
 *			 0.80 - Added flag param in map_block_copy
 *			 0.80 - Removed region 0 clipper from map_put/map_xput
 *           0.76 - Patched SAVE to real BYTE size on saving
 *			 0.76 - Corrected a clipper bug in MAP_BLOCK_COPY
 *			 0.76 - DRAW_AT patched to avoid XGRAPH rotation
 *			 0.75 - SET_ICON second WM fenix function to set window/taskbar icon
 *			 0.74 - SET_TITLE first WM fenix function to set window title
 *			 0.74 - WRITE_VAR, WRITE_INT, WRITE_FLOAT, WRITE_STRING
 *			 0.74 - Collision now accepts TYPE or PROCESS ID
 *			 0.73 - Corrected advance & xadvance to init cos_tables
 *			 0.73 - Added some Win32 patches due to non-portable
 *					strftime formats
 *			 0.73 - Corrected fxi_set_center
 *			 0.73 - Added fxi_memory_free & fxi_memory_total
 *			 0.72 - Corrected fxi_get_real_point
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <math.h>
#include <time.h>
#include <SDL.h>

#include "fxi.h"
#include "fmath.h"
#include "dirs.h"

/* WIN32 INCLUDES */
#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#endif

/* BeOS INCLUDES */
#ifdef TARGET_BeOS
#include <dlfcn.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glob.h>
#include <ctype.h>
#include <be/kernel/OS.h>
#endif

/* LINUX INCLUDES */
#ifdef TARGET_linux
#include <dlfcn.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <glob.h>
#include <ctype.h>
#define KERNELC_V_1 2
#define KERNELC_V_2 3
#define KERNELC_V_3 16
#endif

/* Mac OS X INCLUDES */
#ifdef TARGET_MAC
#include <dlfcn.h>
#include <unistd.h>
//#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <glob.h>
#include <ctype.h>
#endif

#include <fnx_loadlib.h>

#include "dcb.h"

extern int cos_table_initialized ;

#define VP(p) vm_ptr (my, (int)(p))

/* ---------------------------------------------------------------------- */
/* Funciones del sistema                                                  */
/* ---------------------------------------------------------------------- */

/* Funciones incompatibles con DIV */

static int fxi_say (INSTANCE * my, int * params)
{
	/* Show debugging info also in stdout */
	printf ("%s\n", string_get(params[0]));

	gr_con_printf("%s\n", string_get(params[0])) ;
	string_discard(params[0]) ;
	return 1 ;
}

/* Funciones matemáticas */

static int fxi_rand (INSTANCE * my, int * params)
{
	int num1 = params[0] ;
	int num2 = params[1] ;

	return num1 + (int)(((double)(num2-num1+1) * rand()) / (RAND_MAX+1.0)) ;
}

static int fxi_rand_seed (INSTANCE * my, int * params)
{
	srand (params[0]) ;
	return 1 ;
}

static int fxi_abs (INSTANCE * my, int * params)
{
	float num = *(float *)&params[0] ;
	float res = num < 0 ? -num:num ;
	return *(int *)&res ;
}

static int fxi_pow (INSTANCE * my, int * params)
{
	float res = (float)pow (*(float *)&params[0], *(float *)&params[1]) ;
	return *(int *)&res ;
}

static int fxi_fget_angle (INSTANCE * my, int * params)
{
	double dx = params[2] - params[0] ;
	double dy = params[3] - params[1] ;
	int angle ;

	if (dx == 0) return dy > 0 ? 270000 : 90000 ;

	angle = (int) (atan(dy / dx) * 180000.0 / M_PI) ;

	return dx > 0 ? -angle:-angle+180000 ;
}

static int fxi_fget_dist (INSTANCE * my, int * params)
{
	double dx = params[2] - params[0] ;
	double dy = params[3] - params[1] ;

	return (int)sqrt (dx*dx + dy*dy) ;
}

static int fxi_near_angle (INSTANCE * my, int * params)
{
	int angle = params[0] ;
	int dest  = params[1] ;
	int incr  = params[2] ;

	if (angle < dest && dest-angle > 180000)
		angle += 360000 ;

	if (angle > dest && angle-dest > 180000)
		angle -= 360000 ;

	if (angle < dest)
	{
		angle += incr ;
		if (angle > dest) angle = dest ;
	}
	else
	{
		angle -= incr ;
		if (angle < dest) angle = dest ;
	}

	if (angle < 0) return angle + 360000 ;
	if (angle >= 360000) return angle - 360000 ;
	return angle ;
}

static int fxi_advance (INSTANCE * my, int * params)
{
	int angle ;

	angle = LOCDWORD(my,ANGLE) ;
	if (!cos_table_initialized) init_cos_tables() ;
	LOCDWORD(my,COORDX) += fixtoi(fmul(fcos(angle), itofix(params[0]))) ;
	LOCDWORD(my,COORDY) -= fixtoi(fmul(fsin(angle), itofix(params[0]))) ;
	return 1 ;
}

static int fxi_xadvance (INSTANCE * my, int * params)
{
	int angle ;

	angle = params[0] ;
	if (!cos_table_initialized) init_cos_tables() ;
	LOCDWORD(my,COORDX) += fixtoi(fmul(fcos(angle), itofix(params[1]))) ;
	LOCDWORD(my,COORDY) -= fixtoi(fmul(fsin(angle), itofix(params[1]))) ;
	return 1 ;
}

static int fxi_sqrt (INSTANCE * my, int * params)
{
	float res = (float)sqrt(*(float *)&params[0]) ;
	return *(int *)&res ;
}

/* Funciones matemáticas con floats */

static int fxi_cos (INSTANCE * my, int * params)
{
	float param = *(float *)&params[0] ;
	float result = (float)cos((double)(param*M_PI/180000.0)) ;
	return *(int *)&result ;
}
static int fxi_sin (INSTANCE * my, int * params)
{
	float param = *(float *)&params[0] ;
	float result = (float)sin((double)(param*M_PI/180000.0)) ;
	return *(int *)&result ;
}
static int fxi_tan (INSTANCE * my, int * params)
{
	float param = *(float *)&params[0] ;
	float result = (float)tan((double)(param*M_PI/180000.0)) ;
	return *(int *)&result ;
}
static int fxi_acos (INSTANCE * my, int * params)
{
	float param = *(float *)&params[0] ;
	float result = (float)(acos((double)param)*180000.0/M_PI) ;
	return *(int *)&result ;
}
static int fxi_asin (INSTANCE * my, int * params)
{
	float param = *(float *)&params[0] ;
	float result = (float)(asin((double)param)*180000.0/M_PI) ;
	return *(int *)&result ;
}
static int fxi_atan (INSTANCE * my, int * params)
{
	float param = *(float *)&params[0] ;
	float result = (float)(atan((double)param)*180000.0/M_PI) ;
	return *(int *)&result ;
}

/* Interacción entre procesos */

/** EXIT ()
 *  Leaves the program at next frame (two parameters accepted for compatibility)
 */

static int fxi_exit (INSTANCE * my, int * params)
{
	INSTANCE * i = first_instance ;

	while (i)
	{
		LOCDWORD(i,STATUS) = STATUS_KILLED ;
		i = i->next ;
	}
	must_exit = 1 ;
	return must_exit ;
}

static int fxi_exit_0 (INSTANCE * my, int * params)
{
	return fxi_exit (my, params);
}

static int fxi_exit_1 (INSTANCE * my, int * params)
{
	return fxi_exit (my, params);
}

static int fxi_running (INSTANCE * my, int * params)
{
	INSTANCE * i;

	if (params[0] == 0)
		return 0;

	if (params[0] >= FIRST_INSTANCE_ID)
	{
		i = instance_get (params[0]) ;
		if (i && LOCDWORD(i,STATUS) >= STATUS_RUNNING)
			return 1;
		return 0;
	}

	for (i = first_instance ; i ; i = i->next)
	{
		if (LOCDWORD(i,PROCESS_TYPE) == params[0] &&
			LOCDWORD(i,STATUS) >= STATUS_RUNNING)
			return 1;
	}
	return 0;
}

static int fxi_signal (INSTANCE * my, int * params)
{
	INSTANCE * i = instance_get (params[0]) ;
	int status ;
	int subcall[2] ;

	if (!params[0]) return 0 ;

	if (!i && params[0] < FIRST_INSTANCE_ID)
	{
		i = first_instance ;
		subcall[1] = params[1] ;
		while (i)
		{
			if (LOCDWORD(i,PROCESS_TYPE) == params[0])
			{
				subcall[0] = LOCDWORD(i, PROCESS_ID) ;
				fxi_signal (my, subcall) ;
			}
			i = i->next ;
		}
		return 0 ;
	}

	if (i)
	{
		object_list_dirty = 1;
		if (instance_visible(i))
			gr_mark_instance(i);

		switch (params[1] >= 100 ? params[1]-100:params[1])
		{
			case 0:		/* S_KILL */
				status = STATUS_KILLED ;
				LOCDWORD(i,STATUS) = status ;
				break ;

			case 1:		/* S_WAKEUP */
				status = STATUS_RUNNING ;
				LOCDWORD(i,STATUS) = status ;
				break ;

			case 2: 	/* S_SLEEP */
				status = STATUS_SLEEPING ;
				LOCDWORD(i,STATUS) = status ;
				break ;

			case 3:		/* S_FREEZE */
				status = STATUS_FROZEN ;
				LOCDWORD(i,STATUS) = status ;
				break ;

			default:
				gr_error ("Tipo de señal desconocida") ;
		}

		if (instance_visible(i))
		{
			instance_update_bbox(i);
			gr_mark_instance(i);
		}

		if (params[1] >= 100)
		{
			i = instance_getson(i) ;
			while (i)
			{
				subcall[0] = LOCDWORD(i,PROCESS_ID) ;
				subcall[1] = params[1] ;
				fxi_signal (my, subcall) ;
				i = instance_getbigbro(i)  ;
			}
		}
	}
	return 1 ;
}

static int fxi_let_me_alone (INSTANCE * my, int * params)
{
	INSTANCE * i = first_instance ;

	while (i)
	{
		if (i != my) LOCDWORD(i, STATUS) = STATUS_KILLED ;
		i = i->next ;
	}
	return 1 ;
}

static int fxi_get_angle (INSTANCE * my, int * params)
{
	INSTANCE * b = instance_get (params[0]) ;
	int subcall[4] ;

	if (my && b)
	{
		subcall[0] = LOCDWORD(my, COORDX) ;
		subcall[1] = LOCDWORD(my, COORDY) ;
		subcall[2] = LOCDWORD(b,  COORDX) ;
		subcall[3] = LOCDWORD(b,  COORDY) ;
		return fxi_fget_angle (my, subcall) ;
	}
	return -1 ;
}

static int fxi_get_distx (INSTANCE * my, int * params)
{
	double angle = params[0] * M_PI / 180000.0 ;
	return (int)(params[1] * cos(angle)) ;
}

static int fxi_get_disty (INSTANCE * my, int * params)
{
	double angle = params[0] * M_PI / 180000.0 ;
	return (int)(params[1] * -sin(angle)) ;
}

static int fxi_get_dist (INSTANCE * a, int * params)
{
	INSTANCE * b = instance_get (params[0]) ;
	int subcall[4] ;

	if (a && b)
	{
		subcall[0] = LOCDWORD(a, COORDX) ;
		subcall[1] = LOCDWORD(a, COORDY) ;
		if (LOCDWORD(a, RESOLUTION))
		{
			subcall[0] /= LOCDWORD(a,RESOLUTION) ;
			subcall[1] /= LOCDWORD(a,RESOLUTION) ;
		}

		subcall[2] = LOCDWORD(b, COORDX) ;
		subcall[3] = LOCDWORD(b, COORDY) ;
		if (LOCDWORD(b, RESOLUTION))
		{
			subcall[2] /= LOCDWORD(b,RESOLUTION) ;
			subcall[3] /= LOCDWORD(b,RESOLUTION) ;
		}

		if (LOCDWORD(a, RESOLUTION))
			return fxi_fget_dist (a, subcall) * LOCDWORD(a, RESOLUTION) ;

		return fxi_fget_dist (a, subcall) ;
	}
	return -1 ;
}

static int fxi_get_id (INSTANCE * my, int * params)
{
	INSTANCE * ptr = first_instance ;

	if (!params[0])
	{
		if (LOCDWORD(my, ID_SCAN))
		{
			ptr = instance_get (LOCDWORD(my,ID_SCAN)) ;
			if (ptr) ptr = ptr->next ;
		}
		while (ptr)
		{
			if (LOCDWORD(ptr,STATUS) >= STATUS_RUNNING)
			{
				LOCDWORD(my,ID_SCAN) = LOCDWORD(ptr, PROCESS_ID) ;
				return LOCDWORD(ptr, PROCESS_ID) ;
			}
			ptr = ptr->next ;
		}
		LOCDWORD(my,ID_SCAN) = 0 ;
		return 0 ;
	}

	if (LOCDWORD(my,TYPE_SCAN))
	{
		ptr = instance_get (LOCDWORD(my,TYPE_SCAN)) ;
		if (LOCDWORD(ptr,PROCESS_TYPE) != params[0])
			ptr = first_instance ;
		else if (ptr)
			ptr = ptr->next ;
	}
	while (ptr)
	{
		if (LOCDWORD(ptr,PROCESS_TYPE) == params[0])
		{
			if (LOCDWORD(ptr,STATUS) >= STATUS_RUNNING)
			{
				LOCDWORD(my,TYPE_SCAN) = LOCDWORD(ptr,PROCESS_ID);
				return LOCDWORD(ptr,PROCESS_ID) ;
			}
		}
		ptr = ptr->next ;
	}
	LOCDWORD(my,TYPE_SCAN) = 0 ;
	return 0 ;
}

static int fxi_set_point (INSTANCE * my, int * params)
{
	GRAPH * bmp = bitmap_get (params[0], params[1]) ;
	if (!bmp || params[2] < 0 || params[2] > 999) return -1 ;
	while (params[2] >= (bmp->flags & F_NCPOINTS))
		bitmap_add_cpoint (bmp, -1, -1) ;
	bmp->cpoints[params[2]].x = params[3] ;
	bmp->cpoints[params[2]].y = params[4] ;
	return 1 ;
}

static int fxi_set_center (INSTANCE * my, int * params)
{
	GRAPH * bmp = bitmap_get (params[0], params[1]) ;
	if (!bmp) return -1 ;
	if ((bmp->flags & F_NCPOINTS) == 0)
		bitmap_add_cpoint (bmp, params[2], params[3]) ;
	else
	{
		/* CORRECTED: NOT PARAMS[2] AS CPOINT INDEX BUT 0 */
		bmp->cpoints[0].x = params[2] ;
		bmp->cpoints[0].y = params[3] ;
	}
	return 1 ;
}

static int fxi_get_point (INSTANCE * my, int * params)
{
	GRAPH * bmp ;

	bmp = bitmap_get (params[0], params[1]) ;
	if (!bmp) return 0 ;

	/* Use the center as control point if it is not there */
	if (params[2] == 0 && ((bmp->flags & F_NCPOINTS) == 0 || bmp->cpoints[0].x == -1))
	{
		*(int *)VP(params[3]) = bmp->width/2;
		*(int *)VP(params[4]) = bmp->height/2;
		return 1 ;
	}

	if (params[2] >= (bmp->flags & F_NCPOINTS) || params[2] < 0) 
		return 0 ;
	if (bmp->cpoints[params[2]].x == -1 && bmp->cpoints[params[2]].y == -1)
		return 0;

	*(int *)VP(params[3]) = bmp->cpoints[params[2]].x ;
	*(int *)VP(params[4]) = bmp->cpoints[params[2]].y ;
	return 1 ;
}

static int fxi_get_real_point (INSTANCE * my, int * params)
{
	GRAPH * b ;
	int x, y, r, centerx, centery, px, py, rx, ry ;

	double angle ;

	b = instance_graph (my) ;
	if (!b)  return 0 ;

	/* Point 0 is the graphic center, but it may be not defined */
	if (params[0] == 0 && ((b->flags & F_NCPOINTS) == 0 || b->cpoints[0].x == -1))
	{
		if ((b->flags & F_NCPOINTS) == 0)
			bitmap_add_cpoint (b, b->width/2, b->height/2);
		else
		{
			b->cpoints[0].x = b->width/2;
			b->cpoints[0].y = b->height/2;
		}
	}
	else if (params[0] >= (b->flags & F_NCPOINTS) || params[0] < 0) 
		return 0 ;
	if (b->cpoints[params[0]].x == -1 && b->cpoints[params[0]].y == -1)
		return 0;

	r = LOCDWORD(my,REGIONID) ;
	if (r < 0 || r > 31) r = 0 ;

	if (b->cpoints[0].x >= 0)
	{
		centerx = b->cpoints[0].x ;
		centery = b->cpoints[0].y ;
	}
	else
	{
		centerx = b->width/2 ;
		centery = b->height/2 ;
	}

	if (b->cpoints[params[0]].x >= 0) {
		if (LOCDWORD(my,FLAGS) & B_HMIRROR)
			px = centerx - b->cpoints[params[0]].x - 1 ;
		else
			px = b->cpoints[params[0]].x - centerx ;

		if (LOCDWORD(my,FLAGS) & B_VMIRROR)
			py = centery - b->cpoints[params[0]].y - 1 ;
		else
			py = b->cpoints[params[0]].y - centery ;
	}
	else	px = py = 0 ;

	if (LOCDWORD(my,GRAPHSIZEX)==100 && LOCDWORD(my,GRAPHSIZEY)==100)
	{
		if (LOCDWORD(my,GRAPHSIZE) > 0)
		{
			// Corrected a bug from the casting that rounded to 0
			px = (int)(px * (LOCDWORD(my,GRAPHSIZE) / 100.0F)) ;
			py = (int)(py * (LOCDWORD(my,GRAPHSIZE) / 100.0F)) ;
		}
	} 
	else 
	{
		// Adding size_x/size_y control
		if (LOCDWORD(my,GRAPHSIZEX) > 0)	
			px = (int)(px * (LOCDWORD(my,GRAPHSIZEX) / 100.0F)) ;
		if (LOCDWORD(my,GRAPHSIZEY) > 0)
			py = (int)(py * (LOCDWORD(my,GRAPHSIZEY) / 100.0F)) ;
	}

	if (LOCDWORD(my,ANGLE) != 0)
	{
		angle = LOCDWORD(my,ANGLE) * M_PI / 180000.0 ;
		rx = (int)((double)px * cos(-angle) - (double)py * sin(-angle)) ;
		ry = (int)((double)px * sin(-angle) + (double)py * cos(-angle)) ;
		px = rx ;
		py = ry ;
	}

	x = LOCDWORD(my,COORDX) ;
	y = LOCDWORD(my,COORDY) ;
	if (LOCDWORD(my,RESOLUTION))
	{
		x /= LOCDWORD(my,RESOLUTION) ;
		y /= LOCDWORD(my,RESOLUTION) ;
	}

	rx = x+px ;
	ry = y+py ;

	if (LOCDWORD(my,RESOLUTION) > 0)
	{
		rx *= LOCDWORD(my,RESOLUTION) ;
		ry *= LOCDWORD(my,RESOLUTION) ;
	}

	*(int *)VP(params[1]) = rx ;
	*(int *)VP(params[2]) = ry ;
	return 1 ;
}

/* Rutinas de utilidad local */

static void draw_at (GRAPH * dest, int x, int y, REGION * r, INSTANCE * i)
{
	GRAPH * map ;
	int scalex, scaley;

	scalex = LOCDWORD(i,GRAPHSIZEX);
	scaley = LOCDWORD(i,GRAPHSIZEY);
	if (scalex == 100 && scaley == 100) 
		scalex = scaley = LOCDWORD(i,GRAPHSIZE);

	map = instance_graph (i) ;
	if (!map) return ;
	// PATCH - XGRAPH DOES NOT ROTATE DESTINATION GRAPHIC
	if (LOCDWORD(i,ANGLE) || scaley != 100 || scalex != 100) {
		if (LOCDWORD(i,XGRAPH) && scalex == 100 && scaley == 100) {
			gr_blit (dest, r, x, y, LOCDWORD(i,FLAGS), map) ;
		} else {
			if (LOCDWORD(i,XGRAPH)) {
				gr_rotated_blit (dest, r, x, y, LOCDWORD(i,FLAGS), 0, scalex, scaley, map) ;
			} else {
				gr_rotated_blit (dest, r, x, y, LOCDWORD(i,FLAGS), LOCDWORD(i,ANGLE), scalex, scaley, map) ;
			}
		}
	} else {
		gr_blit (dest, r, x, y, LOCDWORD(i,FLAGS), map) ;
	}
}

static void get_pos (INSTANCE * proc, int *x, int *y)
{
	*x = LOCDWORD(proc,COORDX) ;
	*y = LOCDWORD(proc,COORDY) ;
	if (LOCDWORD(proc,RESOLUTION))
	{
		*x /= LOCDWORD(proc,RESOLUTION) ;
		*y /= LOCDWORD(proc,RESOLUTION) ;
	}

}

static int get_bbox (REGION * bbox, INSTANCE * proc)
{
	GRAPH * b ;
	int     x, y ;
	int     scalex, scaley ;

	b = instance_graph (proc) ;
	if (!b) return 0 ;

	scalex = LOCDWORD(proc,GRAPHSIZEX);
	scaley = LOCDWORD(proc,GRAPHSIZEY);
	if (scalex == 100 && scaley == 100) 
		scalex = scaley = LOCDWORD(proc,GRAPHSIZE);

	get_pos (proc, &x, &y) ;

	gr_get_bbox (bbox, 0, x, y, LOCDWORD(proc,FLAGS), 
		LOCDWORD(proc,ANGLE), scalex, scaley, b) ;
	return 1 ;
}

/* Colisiones */

static int check_collision_with_mouse (INSTANCE * proc1)
{
	REGION bbox1 ;
	int x, y, mx, my, i ;
	static GRAPH * bmp ;

	mx = GLODWORD(MOUSEX) ;
	my = GLODWORD(MOUSEY) ;

	/* Checks the process's bounding box to optimize checking
	   (only for screen-type objects) */

	if (LOCDWORD(proc1,CTYPE) == 0)
	{
		if (!get_bbox (&bbox1, proc1)) 
			return 0 ;

		if (bbox1.x > mx || bbox1.x2 < mx ||
			bbox1.y > my || bbox1.y2 < my)
			return 0 ;
	}

	/* Creates a temporary bitmap (only once) */

	if (!bmp) bmp = bitmap_new (0, 2, 2, enable_16bits ? 16:8) ;
	if (!bmp) return 0 ;
	memset (bmp->data, 0, 2) ;

	/* Retrieves process information */

	bbox1.x = 0 ; bbox1.x2 = 1 ;
	bbox1.y = 0 ; bbox1.y2 = 1 ;
	get_pos (proc1, &x, &y) ;

	/* Scroll-type process: check for each region */

	if (LOCDWORD(proc1,CTYPE) == 1)
	{
		for (i = 0 ; i < 10 ; i++)
		{
			int cflags = LOCDWORD(proc1,CFLAGS);

			if (scrolls[i].active && (cflags & (1 << i)))
			{
				REGION * r = scrolls[i].region;

				if (r->x > mx || r->x2 < mx || r->y > my || r->y2 < my)
					continue;
				draw_at (bmp, x+r->x-mx-scrolls[i].posx0, y+r->y-my-scrolls[i].posy0, &bbox1, proc1);
				if (enable_16bits ? *(Uint16 *)bmp->data : *(Uint8 *)bmp->data)
					return 1;
			}
		}
		return 0;
	}

	/* Collision check (blits into temporary space and checks the resulting pixel) */
	draw_at (bmp, x-mx, y-my, &bbox1, proc1) ;
	return enable_16bits ? *(Uint16 *)bmp->data : *(Uint8 *)bmp->data ;
}

static int check_collision (INSTANCE * proc1, INSTANCE * proc2)
{
	REGION bbox1, bbox2 ;
	int x, y, w, h ;
	GRAPH * bmp, * bmp2 ;
	int depth ;

	if (!get_bbox (&bbox1, proc1)) return 0 ;
	if (!get_bbox (&bbox2, proc2)) return 0 ;
	region_union (&bbox1, &bbox2) ;
	if (region_is_empty (&bbox1)) return 0 ;

	w = bbox1.x2 - bbox1.x + 1 ;
	h = bbox1.y2 - bbox1.y + 1 ;
	bbox2.x = bbox2.y = 0 ;
	bbox2.x2 = w-1 ;
	bbox2.y2 = h-1 ;

	depth = (enable_16bits ? 16:8) ;
	bmp = bitmap_new (0, w, h, depth) ;
	bmp2 = bitmap_new (0, w, h, depth) ;
	if (!bmp || !bmp2) return 0 ;
	memset (bmp->data, 0, w*h*(depth == 16?2:1)) ;
	memset (bmp2->data, 0, w*h*(depth == 16?2:1)) ;
	get_pos (proc1, &x, &y) ;
	x -= bbox1.x ;
	y -= bbox1.y ;
	draw_at (bmp,  x, y, &bbox2, proc1) ;
	get_pos (proc2, &x, &y) ;
	x -= bbox1.x ;
	y -= bbox1.y ;
	draw_at (bmp2, x, y, &bbox2, proc2) ;

	if (depth == 16)
	{
		Uint16 * ptr, * ptr2 ;

		for (y = 0 ; y < h ; y++)
		{
			ptr  = (Uint16 *)bmp->data  + w*y ;
			ptr2 = (Uint16 *)bmp2->data + w*y ;

			for (x = 0 ; x < w ; x++, ptr++, ptr2++)
			{
				if (*ptr && *ptr2)
				{
					bitmap_destroy (bmp) ;
					bitmap_destroy (bmp2) ;
					return 1;
				}
			}
		}
	}
	else
	{
		Uint8 * ptr, * ptr2 ;

		for (y = 0 ; y < h ; y++)
		{
			ptr  = (Uint8 *)bmp->data  + w*y ;
			ptr2 = (Uint8 *)bmp2->data + w*y ;

			for (x = 0 ; x < w ; x++, ptr++, ptr2++)
			{
				if (*ptr && *ptr2)
				{
					bitmap_destroy (bmp) ;
					bitmap_destroy (bmp2) ;
					return 1;
				}
			}
		}
	}

	bitmap_destroy (bmp) ;
	bitmap_destroy (bmp2) ;
	return 0;
}

static int fxi_collision (INSTANCE * my, int * params)
{
	INSTANCE * ptr = instance_get(params[0]) ;

	if (params[0] == -1)
		return check_collision_with_mouse(my) ? 1:0 ;

	/* ADDED IN 0.74 - Checks only for a single instance */
	/*DEBUG*/

	if (params[0] >= FIRST_INSTANCE_ID && ptr)
		return check_collision(my,ptr) ;

	/* we must use full list of instances or get types from it */
	ptr = first_instance ;

	if (!params[0])
	{
		if (LOCDWORD(my, ID_SCAN))
		{
			ptr = instance_get (LOCDWORD(my, ID_SCAN)) ;
			if (ptr) ptr = ptr->next ;
		}
		while (ptr)
		{
			if (ptr == my)
			{
				ptr = ptr->next ;
				continue ;
			}
			if ((LOCDWORD(ptr,STATUS) == STATUS_RUNNING ||
			     LOCDWORD(ptr,STATUS) == STATUS_FROZEN)
				&& check_collision (my, ptr))
			{
				LOCDWORD(my, ID_SCAN) = LOCDWORD(ptr, PROCESS_ID) ;
				return LOCDWORD(ptr, PROCESS_ID) ;
			}
			ptr = ptr->next ;
		}
		LOCDWORD (my, ID_SCAN) = 0 ;
		return 0 ;
	}

	if (LOCDWORD(my,TYPE_SCAN))
	{
		ptr = instance_get (LOCDWORD(my,TYPE_SCAN)) ;
		if (ptr && LOCDWORD(ptr,PROCESS_TYPE) != params[0])
			ptr = first_instance ;
		else if (ptr) ptr = ptr->next ;
	}
	while (ptr)
	{
		if (LOCDWORD(ptr,PROCESS_TYPE) == params[0] && ptr != my)
		{
			if ((LOCDWORD(ptr,STATUS) == STATUS_RUNNING ||
			     LOCDWORD(ptr,STATUS) == STATUS_FROZEN)
				&& check_collision (my, ptr))
			{
				LOCDWORD(my, TYPE_SCAN) = LOCDWORD(ptr, PROCESS_ID) ;
				return LOCDWORD(ptr, PROCESS_ID) ;
			}
		}
		ptr = ptr->next ;
	}
	LOCDWORD(my,TYPE_SCAN) = 0 ;
	return 0 ;
}

/* Entrada/salida */

SDL_Joystick * selected_joystick;

static int fxi_key (INSTANCE * my, int * params)
{
	key_equiv * curr ;
	int found = 0 ;

	if (params[0] > 126 || params[0] < 0)
		return 0;

	curr = &key_table[params[0]] ;
	if (keytab_initialized==0) keytab_init() ;
	while (curr!=NULL && found==0) {
		found = keystate[curr->sdlk_equiv] ;
		curr = curr->next ;
		}
	return found ;
}

static int fxi_get_joy_button (INSTANCE * my, int * params)
{
	if (params[0]<SDL_JoystickNumButtons(selected_joystick))
		return SDL_JoystickGetButton(selected_joystick,params[0]) ;
	else
		return -1 ;
}

static int fxi_get_joy_position (INSTANCE * my, int * params)
{
	if (params[0]<2)
		return SDL_JoystickGetAxis(selected_joystick,params[0]) ;//(params[0] & 1) ? joy_y[selected_joystick] : joy_x[selected_joystick] ;
	else
		return 0 ;
}

static int fxi_num_joy (INSTANCE * my, int * params) 
{
	return SDL_NumJoysticks() ;
}

static int fxi_select_joy (INSTANCE * my, int * params)
{
	if (params[0] >= 0 && params[0]<SDL_NumJoysticks()) {
		// Close previous one
		if (selected_joystick!=NULL) SDL_JoystickClose(selected_joystick) ;
		// Open new
		selected_joystick = SDL_JoystickOpen(params[0]) ;
		return params[0] ;
		}		
	return -1 ;
}

/* Funciones de inicialización y carga */

static int
fxi_set_mode (INSTANCE * my, int * params)
{
	if (params[0] < 3200200 || params[0] > 16001400)
		gr_error ("Modo gráfico no soportado") ;

	gr_init(params[0]/10000, params[0]%10000) ;
	return 1 ;
}

static int
fxi_set_mode_2 (INSTANCE * my, int * params)
{
	if (params[0]>1600 || params[1]>1400)
		gr_error ("Modo gráfico no soportado") ;

	gr_init(params[0], params[1]) ;
	return 1 ;
}

static int
fxi_set_mode_3 (INSTANCE * my, int * params)
{
	
	if (params[0]>1600 || params[1]>1400)
		gr_error ("Modo gráfico no soportado");
	
	GLODWORD(GRAPH_MODE) = ((GLODWORD(GRAPH_MODE) & 0xFF) | params[2]);
	gr_init(params[0], params[1]) ;
	return 1 ;
}

static int
fxi_set_mode_4 (INSTANCE * my, int * params)
{
	
	if (params[0]>1600 || params[1]>1400)
		gr_error ("Modo gráfico no soportado");
	
	GLODWORD(GRAPH_MODE) = (params[2] | params[3]);
	gr_init(params[0], params[1]) ;
	return 1 ;
}

static int
fxi_set_fps (INSTANCE * my, int * params)
{
	gr_set_fps (params[0], params[1]) ;
	return params[0];
}

static int
fxi_load_png (INSTANCE * my, int * params)
{
	int r;
	
	if (!scr_initialized) gr_init (320, 200) ;
	r = gr_load_png (string_get(params[0])) ;
	string_discard(params[0]) ;
	return r ;
}

static int
fxi_load_pcx (INSTANCE * my, int * params)
{
	int r;
	
	if (!scr_initialized) gr_init (320, 200) ;
	r = gr_load_pcx (string_get(params[0])) ;
	string_discard(params[0]) ;
	return r ;
}

static int
fxi_save_png (INSTANCE * my, int * params)
{
	int r;
	
	if (!scr_initialized) gr_init (320, 200) ;
	r = gr_save_png (bitmap_get(params[0],params[1]),string_get(params[2])) ;
	string_discard(params[2]) ;
	return r ;
}

static int
fxi_load_map (INSTANCE * my, int * params)
{
	int r;
	
	if (!scr_initialized) gr_init (320, 200) ;
	r = gr_load_map (string_get(params[0])) ;
	string_discard(params[0]) ;
	return r ;
}

static int
fxi_save_map (INSTANCE * my, int * params)
{
	int r;
	
	if (!scr_initialized) gr_init (320, 200) ;
	r = gr_save_map (bitmap_get(params[0],params[1]),string_get(params[2])) ;
	string_discard(params[2]) ;
	return r ;
}

static int fxi_load_fpg (INSTANCE * my, int * params)
{
	int r;
	
	if (!scr_initialized) gr_init (320, 200) ;
	r = gr_load_fpg (string_get(params[0])) ;
	string_discard(params[0]) ;
	return r ;
}

static int fxi_unload_map (INSTANCE * my, int * params)
{
	GRAPH * map ;

	if (params[0] == 0 && params[1] >= 1000)
	{
		map = bitmap_get (0, params[1]) ;
		if (map)
		{
			grlib_unload_map (0, params[1]) ;
			return 1 ;
		}
	}

	map = bitmap_get (params[0], params[1]) ;
	if (!map) return 0 ;
	grlib_unload_map (params[0], params[1]) ;
	return 1 ;
}

static int fxi_unload_fpg (INSTANCE * my, int * params)
{
	grlib_destroy (params[0]) ;
	if (my && LOCDWORD(my,FILEID))
		LOCDWORD(my,FILEID) = 0 ;
	return 1 ;
}

/* Regiones */

static int fxi_define_region (INSTANCE * my, int * params)
{
	region_define (params[0], params[1], params[2], params[3], params[4]) ;
	gr_mark_rect (params[1], params[2], params[3], params[4]);
	return params[0] ;
}

static int fxi_out_region (INSTANCE * my, int * params)
{
	INSTANCE * proc = instance_get (params[0]) ;
	int region = params[1], nscroll ;
	REGION bbox ;

	if (region < 0 || region > 31 || proc == 0)
		return 0 ;

	if (!get_bbox (&bbox, proc))
		return 0 ;

	if (LOCDWORD(proc,CTYPE) == 1)
	{
		nscroll = 0 ;
		if (LOCDWORD(proc,CFLAGS))
		{
			nscroll = 1 ;
			while (!(LOCDWORD(proc,CFLAGS) & (1 << nscroll)))
				nscroll++ ;
			nscroll-- ;
		}
		scroll_region (nscroll, &bbox) ;
	}

	return region_is_out (&regions[region], &bbox) ;
}

/* Paleta de colores */

static int fxi_load_pal (INSTANCE * my, int * params)
{
	const char * palname = string_get (params[0]) ;
	int r = palname ? gr_load_pal(palname) : 0 ;
	string_discard (params[0]) ;
	return r ;
}

static int fxi_save_pal (INSTANCE * my, int * params)
{
	const char * palname = string_get(params[0]);
	int r = palname ? gr_save_pal(palname) : 0;
	string_discard(params[0]);
	return r;
}

static int fxi_convert_palette (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	int * newpal = (int *) VP(params[2]), x, y ;
	Uint8 * orig, * ptr ;
	int total_width ;
	
	if (!map) return -1 ;
	if (map->depth != 8)
	{
		gr_error ("Intento de usar convert_palette con un gráfico de 16 bits") ;
		return 0;
	}
	
	total_width = map->width ;
	orig = (Uint8 *)map->data ;
	if ((map->flags & F_ANIMATION) && map->animation)
	{
		orig = (Uint8 *)map->data - map->offset ;
		total_width = map->pitch * map->animation->frames ;
	}
	for (y = 0 ; y < map->height ; y++)
	{
		ptr = orig + map->pitch * y ;
		for (x = 0 ; x < total_width ; x++, ptr++)
			*ptr = newpal[*ptr] ;
	}
	return 1 ;
}

static int fxi_set_colors (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	gr_set_colors (params[0], params[1], (Uint8 *)VP(params[2])) ;
	return 1 ;
}

static int fxi_get_colors (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	gr_get_colors (params[0], params[1], (Uint8 *)VP(params[2])) ;
	return 1 ;
}

static int fxi_roll_palette (INSTANCE * my, int * params)
{
	gr_roll_palette (params[0], params[1], params[2]) ;
	return 1 ;
}

static int fxi_fade (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;

	gr_fade_init (params[0], params[1], params[2], params[3]);
	return 1 ;
}

static int fxi_fade_on (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	gr_fade_init (100, 100, 100, 16) ;
	return 1 ;
}

static int fxi_fade_off (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	gr_fade_init (0, 0, 0, 16) ;

	while (GLODWORD(FADING))
	{
		gr_wait_frame() ;
		gr_draw_frame() ;
	}
    return 1;
}

static int fxi_find_color (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	return gr_find_nearest_color (params[0], params[1], params[2]) ;
}

static int fxi_get_rgb (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
    gr_get_rgb (params[0], (int *)VP(params[1]), (int *)VP(params[2]), (int *)VP(params[3])) ;
	return 1 ;
}

static int fxi_rgb (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	return enable_16bits ? gr_rgb (params[0], params[1], params[2]) :
		gr_find_nearest_color (params[0], params[1], params[2]) ;
}

extern int fade_step ;

/* Blendops */

static int fxi_create_blendop (INSTANCE * my, int * params)
{
        return (int)(intptr_t) blend_create() ;
}

static int fxi_blendop_apply (INSTANCE * my, int * params)
{
    GRAPH * graph = bitmap_get (params[0], params[1]) ;
    if (graph) blend_apply(graph, (Sint16 *)VP(params[2])) ;
	return 1 ;
}

static int fxi_blendop_assign (INSTANCE * my, int * params)
{
    GRAPH * graph = bitmap_get (params[0], params[1]) ;
    if (graph) blend_assign(graph, (Sint16 *)VP(params[2])) ;
	return 1 ;
}

static int fxi_destroy_blendop (INSTANCE * my, int * params)
{
    blend_free((Sint16 *)VP(params[0])) ;
	return 1 ;
}

static int fxi_blendop_identity (INSTANCE * my, int * params)
{
    blend_init ((Sint16 *)VP(params[0])) ;
	return 1 ;
}

static int fxi_blendop_grayscale (INSTANCE * my, int * params)
{
    blend_grayscale((Sint16 *)VP(params[0]), params[1]) ;
	return 1 ;
}

static int fxi_blendop_translucency (INSTANCE * my, int * params)
{
    blend_translucency ((Sint16 *)VP(params[0]), *(float *)(&params[1])) ;
	return 1 ;
}

static int fxi_blendop_intensity (INSTANCE * my, int * params)
{
    blend_intensity ((Sint16 *)VP(params[0]), *(float *)(&params[1])) ;
	return 1 ;
}

static int fxi_blendop_swap (INSTANCE * my, int * params)
{
    blend_swap ((Sint16 *)VP(params[0])) ;
	return 1 ;
}

static int fxi_blendop_tint (INSTANCE * my, int * params)
{
    blend_tint ((Sint16 *)VP(params[0]), 
		       *(float *)(&params[1]), 
				(Uint8) params[2], 
				(Uint8) params[3], 
				(Uint8) params[4]) ;
	return 1 ;
}

/* Primitivas */

static GRAPH * drawing_graph ;
static int     drawing_z = -512 ;
extern int background_is_black ;

static int fxi_drawing_map (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	if (params[0] == 0 && params[1] == 0)
	{
		drawing_graph = background_8bits_used ? background_8bits : background ;
		background_is_black = 0 ;
	}
	else
		drawing_graph = bitmap_get (params[0], params[1]) ;
	return 1 ;
}

static int fxi_drawing_at (INSTANCE * my, int * params)
{
	drawing_graph = NULL;
	drawing_z = params[0];
	return 1 ;
}

static int fxi_drawing_stipple (INSTANCE * my, int * params)
{
	drawing_stipple = params[0];
	return 1;
}

static int fxi_delete_drawing (INSTANCE * my, int * params)
{
	gr_drawing_destroy (params[0]);
	return 1;
}

static int fxi_move_drawing (INSTANCE * my, int * params)
{
	gr_drawing_move (params[0], params[1], params[2]);
	return 1;
}

static int fxi_drawing_color (INSTANCE * my, int * params)
{
	gr_setcolor (params[0]) ;
	return 1 ;
}

static int fxi_set_text_color (INSTANCE * my, int * params)
{
	gr_text_setcolor (params[0]) ;
	return 1 ;
}

static int fxi_get_text_color (INSTANCE * my, int * params)
{
	return (gr_text_getcolor()) ;
}

static int fxi_drawing_alpha (INSTANCE * my, int * params)
{
	gr_setalpha(params[0]);
	return 1;
}

static int fxi_box (INSTANCE * my, int * params)
{
	if (!drawing_graph) 
	{
		DRAWING_OBJECT dr;

		dr.type = DRAWOBJ_BOX;
		dr.x1 = params[0];
		dr.y1 = params[1];
		dr.x2 = params[2];
		dr.y2 = params[3];
		return gr_drawing_new (dr, drawing_z);
	}
	else
	{
		gr_box (drawing_graph, 0, params[0], params[1], params[2]-params[0], params[3]-params[1]) ;
		return 1 ;
	}
}

static int fxi_rect (INSTANCE * my, int * params)
{
	if (!drawing_graph) 
	{
		DRAWING_OBJECT dr;

		dr.type = DRAWOBJ_RECT;
		dr.x1 = params[0];
		dr.y1 = params[1];
		dr.x2 = params[2];
		dr.y2 = params[3];
		return gr_drawing_new (dr, drawing_z);
	}
	else
	{
		gr_rectangle (drawing_graph, 0, params[0], params[1], params[2]-params[0], params[3]-params[1]) ;
		return 1 ;
	}
}

static int fxi_line (INSTANCE * my, int * params)
{
	if (!drawing_graph) 
	{
		DRAWING_OBJECT dr;

		dr.type = DRAWOBJ_LINE;
		dr.x1 = params[0];
		dr.y1 = params[1];
		dr.x2 = params[2];
		dr.y2 = params[3];
		return gr_drawing_new (dr, drawing_z);
	}
	else
	{
		gr_line (drawing_graph, 0, params[0], params[1], params[2]-params[0], params[3]-params[1]) ;
		return 1 ;
	}
}

static int fxi_circle (INSTANCE * my, int * params)
{
	if (!drawing_graph) 
	{
		DRAWING_OBJECT dr;

		dr.type = DRAWOBJ_CIRCLE;
		dr.x1 = params[0];
		dr.y1 = params[1];
		dr.x2 = params[2];
		return gr_drawing_new (dr, drawing_z);
	}
	else
	{
		gr_circle (drawing_graph, 0, params[0], params[1], params[2]) ;
		return 1 ;
	}
}

static int fxi_fcircle (INSTANCE * my, int * params)
{
	if (!drawing_graph) 
	{
		DRAWING_OBJECT dr;

		dr.type = DRAWOBJ_FCIRCLE;
		dr.x1 = params[0];
		dr.y1 = params[1];
		dr.x2 = params[2];
		return gr_drawing_new (dr, drawing_z);
	}
	else
	{
		gr_fcircle (drawing_graph, 0, params[0], params[1], params[2]) ;
		return 1 ;
	}
}

static int fxi_bezier (INSTANCE * my, int * params)
{
	if (!drawing_graph) 
	{
		DRAWING_OBJECT dr;

		dr.type = DRAWOBJ_CURVE;
		dr.x1 = params[0];
		dr.y1 = params[1];
		dr.x2 = params[2];
		dr.y2 = params[3];
		dr.x3 = params[4];
		dr.y3 = params[5];
		dr.x4 = params[6];
		dr.y4 = params[7];
		dr.level = params[8];
		return gr_drawing_new (dr, drawing_z);
	}
	else
	{
		gr_bezier (drawing_graph, 0, params);
		return 1;
	}
}

/* Funciones gráficas */

static int fxi_get_pixel (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;

	if (enable_16bits && background_8bits_used)
		return gr_get_pixel (background_8bits, params[0], params[1]) ;
	else
		return gr_get_pixel (background, params[0], params[1]) ;
}

static int fxi_graphic_set (INSTANCE * my, int * params)
{
	GRAPH * map ;

	map = bitmap_get (params[0], params[1]) ;
	if (!map) return 0 ;

	switch (params[2])
	{
		case 2:		/* g_center_x */
			if ((map->flags & F_NCPOINTS) > 0)
				map->cpoints[0].x = params[3] ;
			else
				bitmap_add_cpoint
					(map, params[3], map->height/2) ;
			return 1 ;

		case 3:		/* g_center_y */
			if ((map->flags & F_NCPOINTS) > 0)
				map->cpoints[0].y = params[3] ;
			else
				bitmap_add_cpoint
					(map, map->width/2, params[3]) ;
			return 1 ;

		case 8:		/* animation step */
                        if (map->animation)
                                bitmap_animate_to (map, params[3], map->animation->speed) ;
			return 1 ;

		case 9:		/* animation speed */
                        if (map->animation)
                                bitmap_animate_to (map, map->animation->pos, params[3]) ;
			return 1 ;
	}
	return 1 ;
}

static int fxi_graphic_info (INSTANCE * my, int * params)
{
	GRAPH * map ;

	if (params[0] || params[1])
		map = bitmap_get (params[0], params[1]) ;
	else if (enable_16bits && background_8bits_used)
		map = background_8bits ;
	else
		map = background ;

	if (!map) return 0 ;

	switch (params[2])
	{
		case 0:		/* g_wide */
			return map->width ;

		case 1:		/* g_height */
			return map->height ;

		case 2:		/* g_center_x */
			if ((map->flags & F_NCPOINTS) > 0)
				if (map->cpoints[0].x >= 0)
					return map->cpoints[0].x ;
			return map->width/2 ;

		case 3:		/* g_center_y */
			if ((map->flags & F_NCPOINTS) > 0)
				if (map->cpoints[0].y >= 0)
					return map->cpoints[0].y ;
			return map->height/2 ;

		case 4:		/* g_pitch */
			return map->pitch ;

		case 5:		/* g_depth */
			return map->depth ;

		case 6:		/* frames */
			return map->animation ? map->animation->frames : 1 ;

		case 7:		/* animation steps */
			return map->animation ? map->animation->length : 0 ;

		case 8:		/* animation step */
			return map->animation ? map->animation->pos : 0 ;

		case 9:		/* animation speed */
			return map->animation ? map->animation->speed : 0 ;
	}
	return 1 ;
}

static int fxi_put (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	if (!map) return 0 ;
	if (!scr_initialized) gr_init (320, 200) ;

	if (enable_16bits && (background_is_black || background_8bits_used) && map->depth == 8) {
		background_8bits_used = 1 ;
		gr_blit (background_8bits, 0, params[2], params[3], 0, map) ;
	} else if (enable_16bits && background_8bits_used && map->depth == 16) {
		background_8bits_used = 0;
		gr_blit (background, 0, 0, 0, 0, background_8bits);
		gr_blit (background, 0, params[2], params[3], 0, map) ;
	} else
		gr_blit (background, 0, params[2], params[3], 0, map) ;

	background_is_black = 0 ;
	return 1 ;
}

static int fxi_xput (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	int r ;

	if (!map) return 0 ;
	if (!scr_initialized) gr_init (320, 200) ;
	r = params[7] ;
	if (r < 0 || r > 31) r = 0 ;

	background_is_black = 0 ;

	if (enable_16bits && background_8bits_used)
	{
		gr_blit (background, 0, 0, 0, 0, background_8bits);
		background_8bits_used = 0;
	}

	if (params[4] == 0 && params[5] == 100)
	{
		gr_blit (background, &regions[r],
				params[2], params[3], params[6], map) ;
		return 0 ;
	}

	gr_rotated_blit (background, &regions[r], params[2], params[3],
			 params[6], params[4], params[5], params[5], map) ;
	return 0 ;
}

static int fxi_put_screen (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	GRAPH * bg ;
	int     x, y ;

	if (!map) gr_error ("Mapa %d no disponible en el fichero %d", params[1], params[0]) ;
	if (!scr_initialized) gr_init (320, 200) ;

	if (enable_16bits && map->depth == 8)
	{
		background_8bits_used = 1 ;
		bg = background_8bits;
	}
	else
	{
		bg = background;
	}

	x = bg->width/2;
	y = bg->height/2;
	if ((map->flags & F_NCPOINTS) > 0 && map->cpoints[0].x >= 0)
	{
		x = x - map->width/2  + map->cpoints[0].x ;
		y = y - map->height/2 + map->cpoints[0].y ;
	}

	gr_clear (bg) ;
	gr_blit (bg, 0, x, y, 0, map) ;

	background_is_black = 0;
	return 1 ;
}

static int fxi_put_pixel (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;

	if (enable_16bits && background_8bits_used)
		gr_put_pixel (background_8bits, params[0], params[1], params[2]) ;
	else
		gr_put_pixel (background, params[0], params[1], params[2]) ;
	background_is_black = 0 ;
	return 1 ;
}

static int fxi_clear_screen (INSTANCE * my, int * params)
{
	if (!scr_initialized) gr_init (320, 200) ;
	gr_clear (background) ;

	if (enable_16bits && background_8bits_used)
	{
		gr_clear (background_8bits) ;
		background_8bits_used = 0 ;
	}
	background_is_black = 1 ;
	background_dirty = 1 ;
	return 1 ;
}

static int fxi_map_buffer (INSTANCE * my, int * params)
{
	GRAPH * map ;

	if (params[0] || params[1])
		map = bitmap_get (params[0], params[1]) ;
	else
	{
		background_is_black = 0 ;
		if (enable_16bits && background_8bits_used)
			map = background_8bits ;
		else
		{
			if (!background)
			{
				background = bitmap_new (0, scr_width, scr_height, enable_16bits ? 16:8) ;
				assert (background) ;
				gr_clear (background) ;
				bitmap_add_cpoint (background, 0, 0) ;			
			}
			map = background ;
		}
	}

	return map ? (int)map->data : 0 ;
}

static int fxi_map_clear (INSTANCE * my, int * params)
{
	GRAPH *map = bitmap_get (params[0], params[1]) ;
	if (map) gr_clear_as (map, params[2]) ;
	return 1 ;
}

static int fxi_new_map (INSTANCE * my, int * params)
{
	GRAPH * map ;
	if (!scr_initialized) gr_init (320, 200) ;
	map = bitmap_new_syslib (params[0], params[1], params[2]) ;
	if (map) gr_clear (map);
	return map ? map->code : 0 ;
}

static int fxi_get_screen (INSTANCE * my, int * params)
{
	GRAPH * map ;
	if (!scr_initialized) gr_init (320, 200) ;
	map = bitmap_new_syslib (scr_width, scr_height, enable_16bits ? 16:8) ;
	if (map) 
	{
		gr_draw_screen (map, 1, 1);
		if (map->modified > 0)
			map->modified = 0;
	}
	return map ? map->code : 0 ;
}

static int fxi_map_clone (INSTANCE * my, int * params)
{
	GRAPH * origin, * map = NULL ;
	if (!scr_initialized) gr_init (320, 200) ;
        origin = bitmap_get (params[0], params[1]) ;
	if (origin) map = bitmap_clone (origin) ;
	return map ? map->code : 0 ;
}

static int fxi_map_get_pixel (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	if (map) return gr_get_pixel (map, params[2], params[3]) ;
	return 1 ;
}

static int fxi_map_put (INSTANCE * my, int * params)
{
	GRAPH * dest = bitmap_get (params[0], params[1]) ;
	GRAPH * orig = bitmap_get (params[0], params[2]) ;
	
	if (!dest) gr_error ("map_put: mapa de destino no válido") ;
	if (!orig) gr_error ("map_put: mapa de origen no válido") ;

	gr_blit (dest, 0, params[3], params[4], 0, orig) ;
	return 1 ;
}

/** MAP_XPUT (FILE, GRAPH_DEST, GRAPH_SRC, X, Y, ANGLE, SIZE, FLAGS)
 *  Draws a map into another one, with most blitter options including flags and alpha
 */

static int fxi_map_xput (INSTANCE * my, int * params)
{
	GRAPH * dest = bitmap_get (params[0], params[1]) ;
	GRAPH * orig = bitmap_get (params[0], params[2]) ;

	if (!dest || !orig) return 0 ;

	if (params[5] == 0 && params[6] == 100)
		gr_blit (dest, 0, params[3], params[4], params[7], orig) ;
	else
		gr_rotated_blit (dest, 0, params[3], params[4], params[7],
			 params[5], params[6], params[6], orig) ;
	return 1 ;
}

/** MAP_XPUTNP (FILE_DST, GRAPH_DST, FILE_SRC, GRAPH_SRC, X, Y, ANGLE, SCALE_X, SCALE_Y, FLAGS)
 *  Enhanced MAP_XPUT with all parametes and different FPG file and non-proportional scale
 */

static int fxi_map_xputnp (INSTANCE * my, int * params)
{
	GRAPH * dest = bitmap_get (params[0], params[1]) ;
	GRAPH * orig = bitmap_get (params[2], params[3]) ;

	if (!dest || !orig) return 0 ;

	if (params[6] == 0 && params[7] == 100 && params[8] == 100)
		gr_blit (dest, 0, params[4], params[5], params[9], orig) ;
	else
		gr_rotated_blit (dest, 0, params[4], params[5], params[9],
			 params[6], params[7], params[8], orig) ;
	return 1 ;
}

static int fxi_map_name (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]);
	int result;

	if (!map) return 0;
	result = string_new (map->name);
	string_use(result);
	return result;
}

static int fxi_map_set_name (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]);
	const char * ptr = string_get (params[2]) ;
	if (map) 
	{
		strncpy (map->name, ptr, sizeof(map->name));
		map->name[sizeof(map->name)-1] = 0;
	}
	string_discard (params[2]);
	return 0;
}

static int fxi_map_exists (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]);
	return map == NULL ? 0:1 ;
}

static int fxi_fpg_exists (INSTANCE * my, int * params)
{
	GRLIB * lib = grlib_get (params[0]);
	return lib == NULL ? 0:1;
}

static int fxi_map_put_pixel (INSTANCE * my, int * params)
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	if (map) gr_put_pixel (map, params[2], params[3], params[4]) ;
	else     gr_error ("Mapa %d no disponible en el fichero %d", params[1], params[0]) ;
	return 1 ;
}

static int fxi_map_block_copy (INSTANCE * my, int * params)
{
    GRAPH * dest = bitmap_get (params[0], params[1]) ;
    GRAPH * orig = bitmap_get (params[0], params[4]) ;
    REGION clip ;
    int centerx, centery, flag ;
    int x, y, w, h, dx, dy ;
   
    if (!dest)
        gr_error ("Mapa %d no disponible en el fichero %d",
        params[1], params[0]) ;
   
    if (!orig)
    {
        if (!params[4])
        {
            orig = background ;
            if (enable_16bits && background_8bits_used)
                orig = background_8bits ;
        }
        else
            gr_error ("Mapa %d no disponible en el fichero %d",
            params[4], params[0]) ;
    }
   
    x  = params[5] ;
    y  = params[6] ;
    w  = params[7] ;
    h  = params[8] ;
    dx = params[2] ;
    dy = params[3] ;
    flag = params[9] ;

    centery = orig->height/2 ;
    centerx = orig->width/2 ;
    if ((orig->flags & F_NCPOINTS) > 0)
    {
        if (orig->cpoints[0].x >= 0)
            centerx = orig->cpoints[0].x ;
        if (orig->cpoints[0].y >= 0)
            centery = orig->cpoints[0].y ;
		if (flag & B_HMIRROR)
			centerx = orig->width - 1 - centerx;
		if (flag & B_VMIRROR)
			centery = orig->height - 1 - centery;
	}

    if (x < 0)
    {
        dx += x ;
        w  += x ;
        x   = 0 ;
    }
    if (y < 0)
    {
        dy += y ;
        h  += y ;
        y   = 0 ;
    }
    if (dx < 0)
    {
        x += dx ;
        w += dx ;
        dx = 0 ;
    }
    if (dy < 0)
    {
        y += dy ;
        h += dy ;
        dy = 0 ;
    }

    if (x + w > orig->width)
        w = orig->width - x ;

    if (y + h > orig->height)
        h = orig->height - y ;

    if (dx + w > dest->width)
        w = dest->width - dx ;

    if (dy + h > dest->height)
        h = dest->height - dy ;

    if (x  >= orig->width ||  y >= orig->height ||
        dx >= dest->width || dy >= dest->height || w <= 0 || h <= 0)
        return 0 ;

    clip.x = dx ;
    clip.y = dy ;
    clip.x2 = dx + w - 1 ;
    clip.y2 = dy + h - 1 ;
    gr_blit (dest, &clip, dx-x+centerx, dy-y+centery, flag, orig) ;
    return 1 ;
}


/* Funciones de FPG */

static int fxi_fpg_add (INSTANCE * my, int * params)
{
	GRAPH * orig = bitmap_get (params[2], params[3]);
	GRAPH * dest ;

	if (orig == NULL) return 0;
	dest = bitmap_clone(orig) ;
	dest->code = params[1] ;
	return grlib_add_map (params[0], dest);
}

static int fxi_fpg_new (INSTANCE * my, int * params)
{
	return grlib_new();
}

static int fxi_fpg_save (INSTANCE * my, int * params)
{
	const char * filename = string_get (params[1]) ;
	int result = gr_save_fpg (params[0], filename) ;
	string_discard(params[1]) ;
	return result;
}

/* Funciones de búsqueda de caminos */

static int fxi_path_find (INSTANCE * my, int * params)
{
	GRAPH * dest = bitmap_get (params[0], params[1]) ;
	if (!dest) gr_error ("Mapa %d-%d no existe%d", params[0], params[1]) ;
	return path_find (dest, params[2], params[3], params[4], params[5], params[6]) ;
}

static int fxi_path_getxy (INSTANCE * my, int * params)
{
	return path_get ((int *)VP(params[0]), (int *)VP(params[1])) ;
}

static int fxi_path_wall (INSTANCE * my, int * params)
{
	return path_set_wall (params[0]) ;
}

/* Funciones de visualziación de textos */

/** LOAD_FNT (STRING FILENAME)
 *	Load a .FNT font from disk (returns the font ID)
 */

static int fxi_load_fnt (INSTANCE * my, int * params)
{
	char * text ;
	int r ;

	if (!scr_initialized) gr_init (320, 200) ;

	text = (char *)string_get (params[0]) ;
	r = text ? gr_font_load (text) : 0 ;
	string_discard (params[0]) ;
	return r ;
}

/** LOAD_BDF (STRING FILENAME)
 *	Load a .BDF font from disk (returns the font ID)
 */

static int fxi_load_bdf (INSTANCE * my, int * params)
{
	char * text = (char *)string_get (params[0]) ;
	int r = text ? gr_load_bdf (text) : 0 ;
	string_discard (params[0]) ;
	return r ;
}

/** UNLOAD_FNT (FONT)
 *	Destroys a font in memory
 */

static int fxi_unload_fnt (INSTANCE * my, int * params)
{
	gr_font_destroy (params[0]);
	return 0;
}

/** FNT_NEW (DEPTH)
 *	Create a new font in memory (returns the font ID)
 */

static int fxi_fnt_new (INSTANCE * my, int * params)
{
	int result = gr_font_new();
	FONT * font = gr_font_get(result);
	if (font && (params[0] == 8 || params[0] == 16))
		font->bpp = params[0];
	return result;
}

/** GET_GLYPH (FONT, GLYPH)
 *	Create a system map as a copy of one of the font glyphs (returns the map ID)
 */

static int fxi_get_glyph (INSTANCE * my, int * params)
{
	FONT  * font = gr_font_get(params[0]);
	GRAPH * map ;
	unsigned char c = params[1];

	if (font->charset == CHARSET_CP850)
		c = win_to_dos[c];

	if (!scr_initialized) 
		gr_init (320, 200) ;
	if (!font)
		return 0;
	map = bitmap_clone (font->glyph[c].bitmap);
	if (!map)
		return 0;
	if (!(map->flags & F_NCPOINTS))
		bitmap_add_cpoint (map, map->width/2, map->height/2);
	bitmap_add_cpoint (map, font->glyph[c].xoffset, font->glyph[c].yoffset);
	bitmap_add_cpoint (map, font->glyph[c].xadvance, font->glyph[c].yadvance);
	return map->code;
}

/** SET_GLYPH (FONT, GLYPH, LIBRARY, GRAPHIC)
 *  Change one of the font's glyphs
 */

static int fxi_set_glyph (INSTANCE * my, int * params)
{
	FONT  * font = gr_font_get(params[0]);
	GRAPH * map  = bitmap_get(params[2], params[3]);
	unsigned char c = params[1];

	if (font->charset == CHARSET_CP850)
		c = win_to_dos[c];

	if (font && map)
	{
		if (font->glyph[c].bitmap)
			bitmap_destroy (font->glyph[c].bitmap);
		font->glyph[c].bitmap = bitmap_clone(map);
		if ((map->flags & F_NCPOINTS) >= 3 && map->cpoints)
		{
			font->glyph[c].xoffset = map->cpoints[1].x;
			font->glyph[c].yoffset = map->cpoints[1].y;
			font->glyph[c].xadvance = map->cpoints[2].x;
			font->glyph[c].yadvance = map->cpoints[2].y;
		}
	}
	return 0;
}

/** SAVE_FNT (FONT, STRING FILENAME)
 *	Saves a font to disk
 */

static int fxi_save_fnt (INSTANCE * my, int * params)
{
	char * text ;
	int r ;

	text = (char *)string_get (params[1]) ;
	r = text ? gr_font_save (params[0], text) : 0 ;
	string_discard (params[1]) ;
	return r ;
}

/* Funciones de visualización de textos */

static int fxi_write (INSTANCE * my, int * params)
{
	const char * text = string_get (params[4]) ;
	int r ;

	gr_text_set_creator (LOCDWORD(my, PROCESS_ID)) ;
	r = text ? gr_text_new (params[0], params[1], params[2], params[3], text) : 0 ;
	gr_text_set_creator (0) ;
	string_discard (params[4]) ;
	return r ;
}

static int fxi_write_in_map (INSTANCE * my, int * params)
{
	const char * text = string_get (params[1]) ;
	GRAPH * gr ;

	if (!text) return 0 ;
	gr = gr_text_bitmap (params[0], text, params[2]) ;
	string_discard (params[1]) ;
	if (!gr) return 0 ;
	return gr->code ;
}

/*
 *  FUNCTION : fxi_write_var
 *
 *  Creates a new text associated with a variable
 *
 *  FENIX LANG PARAMS:
 *      params[0] : fnt to use
 *      params[1] : X
 *      params[2] : Y
 *      params[3] : align
 *      params[4] : pointer
 *      params[5] : DCB_TYPE
 *
 *  FENIX RETURN VALUE:
 *     Text ID for the newly created text
 *
 */

static int fxi_write_var (INSTANCE * my, int * params)
{
	DCB_TYPEDEF * var = (DCB_TYPEDEF *)VP(params[5]) ;
	int t = 0 ;

	switch (var->BaseType[0])
	{
		case TYPE_FLOAT:
			t = 4 ;
			break ;
		case TYPE_DWORD:
			t = 3 ;
			break ;
		case TYPE_WORD:
			t = 6 ;
			break ;
		case TYPE_BYTE:
			t = 5 ;
			break ;
		case TYPE_STRING:
			t = 2 ;
			break ;
		case TYPE_ARRAY:
			if (var->BaseType[1] == TYPE_BYTE)
			{
				t = 7;
				break;
			}
		default:
			gr_error ("No es un tipo de dato válido");
			break ;
	}
	gr_text_set_creator (LOCDWORD(my, PROCESS_ID)) ;
	t = gr_text_new_var (params[0], params[1], params[2], params[3], VP(params[4]), t) ;
	gr_text_set_creator (0) ;
	return t ;
}

/*
 *  Same as fxi_write_var, but param[5] not given and always set to VAR_STRING
 */

static int fxi_write_string (INSTANCE * my, int * params)
{
	int r ;
	gr_text_set_creator (LOCDWORD(my, PROCESS_ID)) ;
	r = gr_text_new_var (params[0], params[1], params[2], params[3], VP(params[4]), 2) ;
	gr_text_set_creator (0) ;
	return r ;
}

/*
 *  Same as fxi_write_var, but param[5] not given and always set to VAR_INT
 */

static int fxi_write_int (INSTANCE * my, int * params)
{
	int r ;
	gr_text_set_creator (LOCDWORD(my, PROCESS_ID)) ;
	r = gr_text_new_var (params[0], params[1], params[2], params[3], VP(params[4]), 3) ;
	gr_text_set_creator (0) ;
	return r ;
}

/*
 *  Same as fxi_write_var, but param[5] not given and always set to VAR_FLOAT
 */

static int fxi_write_float (INSTANCE * my, int * params)
{
	int r ;
	gr_text_set_creator (LOCDWORD(my, PROCESS_ID)) ;
	r = gr_text_new_var (params[0], params[1], params[2], params[3], VP(params[4]), 4) ;
	gr_text_set_creator (0) ;
	return r ;
}

static int fxi_move_text (INSTANCE * my, int * params)
{
	gr_text_move (params[0], params[1], params[2]) ;
	return 1;
}

static int fxi_delete_text (INSTANCE * my, int * params)
{
	gr_text_destroy_from (params[0], LOCDWORD(my, PROCESS_ID)) ;
	return 1;
}

static int fxi_text_height (INSTANCE * my, int * params)
{
	const char * str = string_get (params[1]) ;
	int result = gr_text_height (params[0], str) ;
	string_discard (params[1]) ;
	return result ;
}

static int fxi_text_width (INSTANCE * my, int * params)
{
	const char * str = string_get (params[1]) ;
	int result = gr_text_width (params[0], str) ;
	string_discard (params[1]) ;
	return result ;
}

/* Funciones de acceso a ficheros */

static int fxi_save (INSTANCE * my, int * params)
{
	file * fp ;
	const char * filename ;
	int result = 0 ;

	filename = string_get (params[0]) ;
	if (!filename) return 0 ;

	fp = file_open (filename, "wb0") ;
	if (fp)
	{
		result = savetypes (fp, VP(params[1]), VP(params[2]), params[3]);
		file_close (fp) ;
	}
	string_discard (params[0]) ;
	return result ;
}

static int fxi_load (INSTANCE * my, int * params)
{
	file * fp ;
	const char * filename ;
	int result ;

	filename = string_get (params[0]) ;
	if (!filename) return 0 ;

	fp = file_open (filename, "rb0") ;
	if (fp)
	{
		result = loadtypes (fp, VP(params[1]), VP(params[2]), params[3]);
		file_close (fp) ;
	}
	string_discard(params[0]) ;
	return result ;
}

static file * vm_files[256] ;

static int vm_file_add (file * fp)
{
	int i ;

	if (!fp) return 0 ;
	for (i = 1 ; i < 256 ; i++)
	{
		if (!vm_files[i])
		{
			vm_files[i] = fp ;
			return i ;
		}
	}
	return 0 ;
}

static file * vm_file (int h)
{
	if (h > 0 && h < 256) return vm_files[h] ;
	return NULL ;
}

static int fxi_fopen (INSTANCE * my, int * params)
{
        static char * ops[] = { "rb0", "r+b0", "wb0", "rb", "wb6" } ;
        file * fp ;

        if (params[1] < 0 || params[1] > 4)
                params[0] = 0 ;

	fp = file_open (string_get(params[0]), ops[params[1]]) ;
	string_discard (params[0]) ;
	return vm_file_add (fp) ;
}

static int fxi_fclose (INSTANCE * my, int * params)
{
	file * fp = vm_file (params[0]) ;
	if (fp)
	{
		file_close (fp) ;
		if (params[0] > 0 && params[0] < 256)
			vm_files[params[0]] = NULL ;
	}
	return 1 ;
}

static int fxi_fread (INSTANCE * my, int * params)
{
	return loadtypes (vm_file (params[0]), VP(params[1]), VP(params[2]), params[3]);
}

static int fxi_fwrite (INSTANCE * my, int * params)
{
	return savetypes (vm_file (params[0]), VP(params[1]), VP(params[2]), params[3]);
}

static int fxi_fseek (INSTANCE * my, int * params)
{
	return file_seek (vm_file (params[0]), params[1], params[2]) ;
}

static int fxi_ftell (INSTANCE * my, int * params)
{
	return file_pos (vm_file (params[0])) ;
}

static int fxi_filelength (INSTANCE * my, int * params)
{
	return file_size (vm_file (params[0])) ;
}

static int fxi_fputs (INSTANCE * my, int * params)
{
	int r = file_puts (vm_file (params[0]), string_get(params[1])) ;
	string_discard(params[1]) ;
	return r ;
}

static int fxi_file (INSTANCE * my, int * params)
{
	char buffer[1030], *ptr = buffer ;
        int str = string_new("") ;
        file * f ;

        f = file_open (string_get(params[0]), "rb") ;
        if (!f) return 0 ;
        string_discard(params[0]) ;

	while (!file_eof(f))
	{
		if (!file_read (f, ptr, 1))
                        break ;
                ptr++ ;
                if (ptr == buffer+1024)
                {
                        *ptr = 0 ;
                        string_concat (str, buffer) ;
                        ptr = buffer ;
                }
	}
        *ptr = 0 ;
        string_concat (str, buffer) ;
	string_use (str) ;
        file_close (f) ;
	return str ;
}

static int fxi_fgets (INSTANCE * my, int * params)
{
	char buffer[1030] ;
	int str, str2 = 0, str3 ;
	int len, sigue ;

	for (;;)
	{
		file_gets (vm_file (params[0]), buffer, 1024) ;
		len = strlen(buffer) ;
		if (len > 1 && buffer[len-1] == '\n' && buffer[len-2] == '\\')
		{
			buffer[len-2] = 0 ;
			sigue = 1 ;
		}
		else	sigue = 0 ;

		str = string_new (buffer) ;
		if (str2)
		{
			str3 = string_add (str2, str) ;
			string_discard (str) ;
			string_discard (str2) ;
			str2 = str3 ;
		}
		else	str2 = str ;

		if (!sigue) break ;
	}
	string_use (str2) ;
	return str2 ;
}

static int fxi_feof (INSTANCE * my, int * params)
{
	return file_eof (vm_file (params[0])) ;
}

static int fxi_file_exists (INSTANCE * my, int * params)
{
	int r ;
    r = file_exists (string_get(params[0])) ;
	string_discard (params[0]) ;
	return r ;
}

/* Sonido */



/*
 *  FUNCTION : fxi_load_song
 *
 *  Load a MOD from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE:
 *
 *		mod id
 *
 */

static int fxi_load_song (INSTANCE * my, int * params) {

	int var;

	const char * filename ;

	filename = string_get (params[0]) ;
	if (!filename) return (-1) ;

	var=(load_song(filename));
	string_discard(params[0]);
	return (var);
}


/*
 *  FUNCTION : fxi_play_song
 *
 *  Play a MOD
 *
 *  PARAMS:
 *      mod id;
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


static int fxi_play_song (INSTANCE * my, int * params)
{
	if (params[0] == -1)
		return -1;
	return(play_song(params[0],params[1]));
}


/*
 *  FUNCTION : fxi_unload_song
 *
 *  Frees the resources from a MOD and unloads it
 *
 *  PARAMS:
 *      mod id;
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


static int fxi_unload_song (INSTANCE * my, int * params)
{
	   if (params[0]<0) return (-1);
       return(unload_song(params[0]));

}



/*
 *  FUNCTION : fxi_stop_song
 *
 *  Stop the play of a mod
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */

int fxi_stop_song (INSTANCE * my, int * params)
{
	return(stop_song());
}



/*
 *  FUNCTION : fxi_pause_song
 *
 *  Pause the mod in curse, you can resume it after
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


int fxi_pause_song (INSTANCE * my, int * params)
{
	return(pause_song());
}


/*
 *  FUNCTION : fxi_resume_song
 *
 *  Resume the mod, paused before
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


int fxi_resume_song (INSTANCE * my, int * params)
{
	return(resume_song());
}



/*
 *  FUNCTION : fxi_is_playing_song
 *
 *  Check if there is any mod playing
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  TRUE OR FALSE if there is no error
 */


static int fxi_is_playing_song(INSTANCE * my, int * params) {
    return (is_playing_song());
}


/*
 *  FUNCTION : fxi_set_song_volume
 *
 *  Set the volume for mod playing (0-128)
 *
 *  PARAMS:
 *
 *  int volume
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if there is no error
 */


static int fxi_set_song_volume (INSTANCE * my, int * params)
{
	return (set_song_volume(params[0]));
}




/*
 *  FUNCTION : fxi_fade_music_in
 *
 *  Play a MOD/OGG fading in it
 *
 *  PARAMS:
 *      mod pointer
 *      number of loops (-1 infinite loops)
 *      ms  microsends of fadding
 *
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */



static int fxi_fade_music_in (INSTANCE * my, int * params)
{		
	return (fade_music_in(params[0],params[1],params[2]));
}


/*
 *  FUNCTION : fxi_fade_music_off
 *
 *  Stop the play of a mod 
 *
 *  PARAMS:
 *      
 *  ms  microsends of fadding
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */

static int fxi_fade_music_off (INSTANCE * my, int * params)
{		
	return (fade_music_off(params[0]));
}


/*
 *  FUNCTION : fxi_load_wav
 *
 *  Load a WAV from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE:
 *
 *		wav id
 *
 */




static int fxi_load_wav (INSTANCE * my, int * params)
{
	int var;

	const char * filename ;

	filename = string_get (params[0]) ;
	if (!filename) return (-1) ;

	var=(load_wav(filename));
	string_discard(params[0]);
	return (var);

}


/*
 *  FUNCTION : fxi_play_mod
 *
 *  Play a WAV
 *
 *  PARAMS:
 *      wav id;
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


static int fxi_play_wav (INSTANCE * my, int * params)
{
	return(play_wav(params[0],params[1]));
}



/*
 *  FUNCTION : fxi_unload_wav
 *
 *  Frees the resources from a wav, unloading it
 *
 *  PARAMS:
 *
 *  mod id
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


static int fxi_unload_wav (INSTANCE * my, int * params)
{
	return(unload_wav(params[0]));
}


/*
 *  FUNCTION : fxi_stop_wav
 *
 *  Stop a wav playing
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */



static int fxi_stop_wav (INSTANCE * my, int * params)
{
	return(stop_wav(params[0]));
}



/*
 *  FUNCTION : fxi_pause_wav
 *
 *  Pause a wav playing, you can resume it after
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */


static int fxi_pause_wav (INSTANCE * my, int * params)
{
	return (pause_wav(params[0]));
}


/*
 *  FUNCTION : resume_wav
 *
 *  Resume a wav playing, paused before
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if all goes ok
 */

static int fxi_resume_wav (INSTANCE * my, int * params)
{
	return (resume_wav(params[0]));
}



/*
 *  FUNCTION : is_playing_wav
 *
 *  Check a wav playing
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  TRUE OR FALSE if there is no error
 */


static int fxi_is_playing_wav(INSTANCE * my, int * params) {
    return (is_playing_wav(params[0]));
}




/*
 *  FUNCTION : fxi_set_wav_volume
 *
 *  Set the volume for a wav playing (0-128)
 *
 *  PARAMS:
 *
 *  wav id
 *  int volume
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  0 if there is no error
 */


static int fxi_set_wav_volume(INSTANCE * my, int * params) {
	return(set_wav_volume(params[0],params[1]));
}

/*
 *  FUNCTION : fxi_set_panning
 *
 *  Set the panning for a wav channel 
 *
 *  PARAMS:
 *
 *  channel
 *  left volume (0-255)
 *  right volume (0-255)
 *  
 *
 *	
 * 
 */


static int fxi_set_panning(INSTANCE * my, int * params) {
	return(set_panning(params[0],params[1],params[2]));
}

/*
 *  FUNCTION : fxi_set_position
 *
 *  Set the position of a channel. (angle) is an integer from 0 to 360 
 *
 *  PARAMS:
 *
 *  channel
 *  angle (0-360)
 *  distance (0-255)
 *  
 *
 *	
 * 
 */


static int fxi_set_position(INSTANCE * my, int * params) {
	return(set_position(params[0],params[1],params[2]));
}


/*
 *  FUNCTION : fxi_set_distance
 *
 *  Set the "distance" of a channel. (distance) is an integer from 0 to 255
 *  that specifies the location of the sound in relation to the listener.
 *
 *  PARAMS:
 *
 *  channel
 *  
 *  distance (0-255)
 *  
 *
 *	
 * 
 */


static int fxi_set_distance(INSTANCE * my, int * params) {
	return(set_distance(params[0],params[1]));
}


/*
 *  FUNCTION : fxi_reverse_stereo
 *
 *  Causes a channel to reverse its stereo. 
 *  
 *
 *  PARAMS:
 *
 *  channel
 *  
 *  flip 0 normal != reverse
 *  
 *
 *	
 * 
 */


static int fxi_reverse_stereo(INSTANCE * my, int * params) {
	return(reverse_stereo(params[0],params[1]));
}

/* Sonido de CD */

static int fxi_play_cd (INSTANCE * my, int * params)
{
	cd_play (params[0], params[1]) ;
	return 1 ;
}

static int fxi_is_playing_cd (INSTANCE * my, int * params)
{
	return cd_playing() ;
}

static int fxi_stop_cd (INSTANCE * my, int * params)
{
	cd_stop () ;
	return 1 ;
}

/* Mode 7 */

static int fxi_start_mode7 (INSTANCE * my, int * params)
{
	gr_mode7_start (params[0], params[1], params[2], params[3], params[4], params[5]) ;
	return 1 ;
}

static int fxi_stop_mode7 (INSTANCE * my, int * params)
{
	gr_mode7_stop (params[0]) ;
	return 1 ;
}

/* Scroll */

static int fxi_start_scroll (INSTANCE * my, int * params)
{
	object_list_dirty = 1;
	gr_scroll_start (params[0], params[1], params[2], params[3], params[4], params[5]) ;
	return 1 ;
}

static int fxi_stop_scroll (INSTANCE * my, int * params)
{
	object_list_dirty = 1;
	gr_scroll_stop (params[0]) ;
	return 1 ;
}

static int fxi_move_scroll (INSTANCE * my, int * params)
{
	//gr_scroll_draw (params[0], 0, 0) ;
	return 1 ;
}

/* Reproducción de FLI */

FLIC * current_fli = 0 ;
int    current_fli_x = 0 ;
int    current_fli_y = 0 ;

/*
 *  FUNCTION : fxi_start_fli
 *
 *  Load & start playing a FLI/FLC animation
 *
 *  PARAMS:
 *
 *		file : filename/path for the FLI/FLC file
 *		x,y	 : screen position
 *  
 */

static int fxi_start_fli (INSTANCE * my, int * params)
{
	const char * str = string_get(params[0]) ;

	if (!str) return 0 ;
	if (current_fli) flic_destroy (current_fli) ;
	current_fli = flic_open (str) ;
	current_fli_x = params[1] ;
	current_fli_y = params[2] ;
	object_list_dirty = 1;
	string_discard (params[0]) ;
	if (current_fli) return current_fli->header.frames ;
	return 1 ;
}


/*
 *  FUNCTION : fxi_reset_fli
 *
 *  Reset current FLI/FLC animation to frame 0
 *
 *  PARAMS:
 *
 *		No params
 *  
 */

static int fxi_reset_fli (INSTANCE * my, int * params)
{
	if (current_fli) flic_reset (current_fli) ;
	return 1 ;
}

/*
 *  FUNCTION : fxi_end_fli
 *
 *  Stop current FLI/FLC animation
 *
 *  PARAMS:
 *
 *		No params
 *  
 */

static int fxi_end_fli (INSTANCE * my, int * params)
{
	if (current_fli)
	{
		object_list_dirty = 1;
		flic_destroy (current_fli) ;
		current_fli = 0 ;
	}
	return 1 ;
}

/*
 *  FUNCTION : fxi_frame_fli
 *
 *  Check status for the current FLI/FLC animation
 *
 *	RETURN VALUE:
 *
 *		Current frame or 0 if animation has ended
 *
 *  PARAMS:
 *
 *		No params
 *  
 */

static int fxi_frame_fli (INSTANCE * my, int * params)
{
	if (current_fli) 
		return current_fli->finished ? 0 : current_fli->current_frame ;
	return 0 ;
	  
}

/* STRINGS */

/** LEN (STRING SOURCE)
 *  Returns the size of a string
 */

static int fxi_strlen (INSTANCE * my, int * params)
{
	const char * str = string_get(params[0]) ;
	int r = str ? strlen(str) : 0 ;
	string_discard (params[0]) ;
	return r ;
}

/** STRING UCASE (STRING SOURCE)
 *  Converts a string to upper-case
 */

static int fxi_strupper (INSTANCE * my, int * params)
{
	int r = string_ucase(params[0]) ;
	string_discard (params[0]) ;
	string_use     (r) ;
	return r ;
}

/** STRING LCASE (STRING SOURCE)
 *  Converts a string to lower-case
 */

static int fxi_strlower (INSTANCE * my, int * params)
{
	int r = string_lcase(params[0]) ;
	string_discard (params[0]) ;
	string_use     (r) ;
	return r ;
}

/** STRCASECMP (STRING S1, STRING S2)
 *  Compares two strings, case-insensitive
 */

static int fxi_strcasecmp (INSTANCE * my, int * params)
{
	int r = string_casecmp(params[0],params[1]) ;
	string_discard (params[0]) ;
	string_discard (params[1]) ;
	return r ;
}

/** SUBSTR (STRING SOURCE, INT FIRST_CHARACTER, INT COUNT)
 *  Returns part of a given string, starting at the given character position
 *  and returning a string limited to COUNT characters 
 */

static int fxi_substr (INSTANCE * my, int * params)
{
	int r = string_substr(params[0],params[1],
		params[2] > 0 ? params[1]+params[2]-1 : 
	    params[2] < 0 ? params[2]:-1) ;
	string_discard (params[0]) ;
	string_use     (r) ;
	return r ;
}

/** SUBSTR (STRING SOURCE, INT FIRST_CHARACTER)
 *  Returns a substring, from the character given to the end of the source string
 */

static int fxi_substr2 (INSTANCE * my, int * params)
{
	int r = string_substr(params[0],params[1],-1) ;
	string_discard (params[0]) ;
	string_use     (r) ;
	return r ;
}

/** FIND (STRING SOURCE, STRING SUBSTRING)
 *  Searchs a substring in a string, and returns its position
 */

static int fxi_strfind (INSTANCE * my, int * params)
{
	int r = string_find(params[0],params[1], 0) ;
	string_discard (params[0]) ;
	string_discard (params[1]) ;
	return r ;
}

/** FIND (STRING SOURCE, STRING SUBSTRING, INT FIRST)
 *  Searchs a substring in a string, starting from the given position, and returns its position
 */

static int fxi_strfindSSI (INSTANCE * my, int * params)
{
	int r = string_find(params[0],params[1], params[2]) ;
	string_discard (params[0]) ;
	string_discard (params[1]) ;
	return r ;
}

/** STRING LPAD (STRING SOURCE, LENGTH)
 *  Expands the string up to the given length, adding spaces at the left
 */

static int fxi_lpad (INSTANCE * my, int * params)
{
	int r = string_pad(params[0], params[1], 0);
	string_discard(params[0]);
	string_use(r);
	return r;
}

/** STRING RPAD (STRING SOURCE, LENGTH)
 *  Expands the string up to the given length, adding spaces at the right
 */

static int fxi_rpad (INSTANCE * my, int * params)
{
	int r = string_pad(params[0], params[1], 1);
	string_discard(params[0]);
	string_use(r);
	return r;
}

/** ITOA (INT VALUE)
 *  Converts an integer to string
 */

static int fxi_itos (INSTANCE * my, int * params)
{
	int r = string_itoa (params[0]) ;
	string_use(r) ;
	return r ;
}

/** FTOA (FLOAT VALUE)
 *  Converts a floating-point number to string
 */

static int fxi_ftos (INSTANCE * my, int * params)
{
	int r = string_ftoa (*(float *)&params[0]) ;
	string_use(r) ;
	return r ;
}

/** ATOI (STRING VALUE)
 *  Converts a string to integer
 */

static int fxi_stoi (INSTANCE * my, int * params)
{
	const char * str = string_get(params[0]) ;
	int r = str ? atoi(str) : 0 ;
	string_discard (params[0]) ;
	return r ;
}

/** ATOF (STRING VALUE)
 *  Converts a string to floating-point number
 */

static int fxi_stof (INSTANCE * my, int * params)
{
	const char * str = string_get(params[0]) ;
	float res = (float)(str ? atof(str) : 0 );
	string_discard (params[0]) ;
	return *(Sint32 *)&res ;
}

/** ASC(STRING C)
 *  Return the ASCII code of the first character at the string
 */

static int fxi_asc (INSTANCE * my, int * params)
{
	const Uint8 * str = string_get(params[0]) ;
	int r = str ? *str : 0 ;
	string_discard (params[0]) ;
	return r ;
}

/** CHR(ASCII)
 *  Returns a string of length 1, with the character of the given ASCII code
 */

static int fxi_chr (INSTANCE * my, int * params)
{
	char buffer[2] = " " ; int r ;
	buffer[0] = params[0] ;
	r = string_new (buffer) ;
	string_use (r) ;
	return r ;
}

/** STRING TRIM(STRING SOURCE)
 *  Returns the given string, stripping any space characters at the beginning or the end
 */

static int fxi_trim (INSTANCE * my, int * params)
{
	int r = string_strip (params[0]) ;
	string_discard (params[0]) ;
	string_use (r) ;
	return r;
}

#ifndef WIN32
extern char *strrev(char *);
#endif

/** STRING STRREV (STRING SOURCE)
 *  Returns the reverse of the source string
 */

static int fxi_strrev (INSTANCE * my, int * params)
{
	int r = string_new (string_get(params[0]));
	string_discard (params[0]) ;
	string_use (r) ;
	strrev ((char *) string_get(r));
	return r;
}

/** FORMAT (INT VALUE)
 *  Converts a given integer value to string form
 */

static int fxi_formatI (INSTANCE * my, int * params)
{
	int r = string_format (params[0], 0, '.', ',');
	string_use(r) ;
	return r;
}

/** FORMAT (FLOAT VALUE)
 *  Converts a given value to string form
 */

static int fxi_formatF (INSTANCE * my, int * params)
{
	int r = string_format (*(float *)&params[0], -1, '.', ',');
	string_use(r) ;
	return r;
}

/** FORMAT (INT VALUE, INT DECIMALS)
 *  Converts a given integer value to string form. Uses a fixed number
 *  of decimals, as given with the second parameter.
 */

static int fxi_formatFI (INSTANCE * my, int * params)
{
	int r = string_format (*(float *)&params[0], params[1], '.', ',');
	string_use(r) ;
	return r;
}

/*
 * Dynamic memory
 */

/* Linux utility function */

#ifdef TARGET_linux
int kernel_version_type(void)
{
	struct utsname sysinf;
	int kernel_v[3];
	int i,t,fv = 0;

	if(uname(&sysinf) == -1)
		return -1;

	bzero((int*)kernel_v, sizeof(int)*3);

	for(i=0, t=0; i<=2; i++) 
	{
		if(sysinf.release[t])
		{
			kernel_v[i] = atoi(&sysinf.release[t]);
			while (sysinf.release[++t] && sysinf.release[t] != '.')
				;
			t++;
		}
	}

	if(!fv && kernel_v[0] > KERNELC_V_1) fv = 1;
	if(!fv && kernel_v[0] < KERNELC_V_1) fv = 2;
	if(!fv && kernel_v[1] > KERNELC_V_2) fv = 1;
	if(!fv && kernel_v[1] < KERNELC_V_2) fv = 2;
	if(!fv && kernel_v[2] > KERNELC_V_3) fv = 1;
	if(!fv && kernel_v[2] < KERNELC_V_3) fv = 2;

	return fv;
}
#endif

/* MEMORY_FREE()
 *  Returns the number of free bytes (physycal memory only)
 *  This value is intended only for informational purposes
 *  and may or may not be an approximation.
 */

static int fxi_memory_free (INSTANCE * my, int * params)
{
#if defined(WIN32)
	MEMORYSTATUS mem ;
	float result ;

	GlobalMemoryStatus(&mem) ;
	result = (float)mem.dwAvailPhys ;

	return *(int *)&result ;
#elif defined(TARGET_linux)
	struct sysinfo meminf;
	int fv;

	if(sysinfo(&meminf) == -1)
		return -1;

	if(!(fv = kernel_version_type()))
		return -1;

	if(fv == 1)
	{
		gr_con_printf ("Returning %d", meminf.freeram * meminf.mem_unit);
		return meminf.freeram * meminf.mem_unit;
	}
	else
	{
		gr_con_printf ("Returning %d", meminf.freeram);
		return meminf.freeram;
	}

	return -1;
#elif defined(TARGET_BeOS)
	float result;
	system_info info;
	
	get_system_info(&info);
	result = B_PAGE_SIZE * (info.max_pages-info.used_pages);
	
	return *(int*)&result;
#endif
}

/* MEMORY_TOTAL();
 *  Return total number of bytes of physical memory
 */

static int fxi_memory_total (INSTANCE * my, int * params)
{
#if defined(WIN32)
	MEMORYSTATUS mem ;
	float result ;

	GlobalMemoryStatus(&mem) ;
	result = mem.dwTotalPhys ;

	return mem.dwTotalPhys ;
#elif defined(TARGET_linux)
	struct sysinfo meminf;
	int fv;

	if(sysinfo(&meminf) == -1)
		return -1;

	if(!(fv = kernel_version_type()))
		return -1;

	if(fv == 1)
		return meminf.totalram * meminf.mem_unit;
	else
		return meminf.totalram;

	return -1;
#elif defined(TARGET_BeOS)
	float memory;
	system_info info;
	
	get_system_info(&info);
	memory = B_PAGE_SIZE * (info.max_pages);
	return *(int*)&memory;
#endif
}

static int fxi_memcopy (INSTANCE * my, int * params)
{
	memmove (VP(params[0]), VP(params[1]), params[2]) ;
	return 1 ;
}

static int fxi_memset (INSTANCE * my, int * params)
{
	memset (VP(params[0]), params[1], params[2]) ;
	return 1 ;
}

static int fxi_memsetw (INSTANCE * my, int * params)
{
	Uint16 * ptr = (Uint16 *)VP(params[0]) ;
	const Uint16 b = params[1] ;
	int n ;

	for (n = params[2] ; n ; n--) *ptr++ = b ;
	return 1 ;
}

static int fxi_alloc (INSTANCE * my, int * params)
{
	void * ptr = vm_malloc (params[0] ? params[0] : 1) ;
	if (!ptr) gr_error ("ALLOC: no hay memoria libre suficiente") ;
	return (int)(intptr_t)ptr ;
}

static int fxi_realloc (INSTANCE * my, int * params)
{
	void * old = VP(params[0]) ;
	size_t n = params[1] ? (size_t)params[1] : 1 ;
	size_t oldsz = vm_alloc_size (old) ;
	void * ptr = vm_malloc (n) ;
	if (!ptr) gr_error ("REALLOC: no hay memoria libre suficiente") ;
	if (old && oldsz)
		memcpy (ptr, old, oldsz < n ? oldsz : n) ;
	return (int)(intptr_t)ptr ;
}

static int fxi_free (INSTANCE * my, int * params)
{
	(void) my ;
	(void) params ;
	return 1 ;
}

/* Hora del día */

static int fxi_time (INSTANCE * my, int * params)
{
	return time(0) ;
}

/*
 *  FUNCTION : fxi_ftime
 *
 *  Returns parts of the date
 *
 *  PARAMS:
 *      no params
 *
 *  RETURN VALUE:
 *      pointer to a float value...
 *
 */

static int
fxi_ftime (INSTANCE * my, int * params)
{
	char buffer[128] ;
	char * format ;
	struct tm * t ;
	int ret ;
	time_t tim ;
	char * base ;

#ifndef LINUX
	/* aux buffer to make all changes... */
	char aux[128] ;
	unsigned char pos ;

#endif

	format = base = strdup(string_get(params[0])) ;

#ifndef LINUX
	/* Addapting win32 strftime formats to linux formats */
	/* HEAVY PATCH... :( */
	pos=0 ;
	while (*format && pos<127) {
		switch (*format) {
			case '%': /* MIGHT NEED CONVERSION... */
				aux[pos] = *format ;
				pos++ ;
				format++ ;
				switch (*format) {
					case 'e':	aux[pos] = '#' ;
								pos++ ;
								aux[pos] = 'd' ;
								break ;
					case 'l':   aux[pos] = '#' ;
								pos++ ;
								aux[pos] = 'I' ;
								break ;
					case 'k':	aux[pos]='#' ;
								pos++ ;
								aux[pos] = 'H' ;
								break ;
					case 'P':	aux[pos] = 'p' ;
								break ;

					case 'C':	aux[pos++] = '%' ;
								aux[pos++] = *format ;
								aux[pos++] = '%' ;
								aux[pos] = 'Y' ;
								break ;

					case 'u':	aux[pos++] = '%' ;
								aux[pos++] = *format ;
								aux[pos++] = '%' ;
								aux[pos] = 'w' ;
								break ;

					case '%':	//MUST BE %%%% TO KEEP 2 IN POSTPROCESS
								aux[pos++] = '%' ;
								aux[pos++] = '%' ;
								aux[pos] = '%' ;
								break ;

					default:	aux[pos] = *format ;
								break ;
				}
				break ;

			default: aux[pos] = *format ;
					 break ;
			}
		format++ ;
		pos++ ;
	}
	aux[pos]=0 ;
	free(base) ;
	format = aux ;
#endif

	tim = (time_t) params[1] ;
	t = localtime(&tim) ;
	strftime (buffer, 128, format, t) ;
	string_discard(params[0]) ;

#ifndef LINUX
	/* win32 postprocess */
	aux[0] = '\0' ;
	format = buffer ;
	pos = 0 ;
	while (*format) {
		switch (*format) {
			case '%':	format++ ;
						switch (*format) {
							case 'u':	format++ ;
										if (*format=='0') *format='7' ;
										aux[pos] = *format ;
										break ;

							case 'C':	format++ ;
										aux[pos] = *format ;
										pos++ ;
										format++ ;
										aux[pos] = *format ;
										format++ ;
										format++ ;
										break ;

							default:	aux[pos] = *format ;
										break ;
						}
						break ;
			default:	aux[pos] = *format ;
						break ;
		}
		format++ ;
		pos++;
	}
	aux[pos] = '\0' ;
	strcpy(buffer,aux) ;
#endif

	ret = string_new(buffer) ;
	string_use(ret) ;
	return ret ;
}


/* Window Manager */


static int fxi_set_title (INSTANCE * my, int * params) {

	apptitle = strdup(string_get(params[0])) ;
	return 1 ;
}

static int fxi_set_icon (INSTANCE * my, int * params) {

	icono = bitmap_get(params[0],params[1]) ;
	return 1 ;
}


/*
 * Auxiliary QSort functions
 *
 */

double GetData(Uint8 *Data,int pos,int *params)
{
	int i; Uint16 w; float f;
	if(params[4]==1){ return Data[pos*params[1]+params[3]];}
	if(params[4]==2){ memcpy(&w,&Data[pos*params[1]+params[3]],params[4]); return w;}
	if(params[4]==4 && params[5]==0){ memcpy(&i,&Data[pos*params[1]+params[3]],params[4]); return i;}
	if(params[4]==4 && params[5]==1){ memcpy(&f,&Data[pos*params[1]+params[3]],params[4]); return f;}
	return 1 ;
}

void QuickSort(Uint8 *Data,int inf, int sup, int *params)
{
	register int izq,der;
	double mitad;
	Uint8* x=(Uint8*)malloc(params[1]);
	izq=inf;
	der=sup;
	mitad=GetData(Data,(izq+der)>>1,params);
	do
	{
		while(GetData(Data,izq,params) < mitad && izq<sup)
			izq++;
		while(mitad<GetData(Data,der,params) && der>inf)
			der--;

		if(izq<=der)
		{
			memcpy(x,&Data[izq*params[1]],params[1]);
			memcpy(&Data[izq*params[1]],&Data[der*params[1]],params[1]);
			memcpy(&Data[der*params[1]],x,params[1]);
			izq++;
			der--;
		}
	}while(izq<=der);

	if(inf<der)
		QuickSort(Data,inf,der,params);

	if(izq<sup)
		QuickSort(Data,izq,sup,params);
}

/*
 *	QSort:
 */

static int fxi_quicksort(INSTANCE *my, int *params)
{ //punteroalarray,tamañodato,numdatos,offsetadatoordenador,tamañodatoaordenar,tipodato(int,float)
	Uint8 *Data=(Uint8 *)VP(params[0]);
	QuickSort(Data,0,params[2]-1,params);
	return 1 ;
}

static int fxi_filter (INSTANCE *my, int *params)
{ //fpg,map,tabla10

	GRAPH * map = bitmap_get (params[0], params[1]), *map2=bitmap_clone(map);
	int *tabla=(int*)VP(params[2]);
	int x,y,i,j;
	int r,g,b,r2,g2,b2,c;
	float r1,g1,b1;

	if (map->depth != 16)
		gr_error ("Intento de usar filter con un gráfico de 8 bits") ;

	r1=0;g1=0;b1=0,c=0;
	for(i=1;i<map->width-1;i++)
	{
		for(j=1;j<map->height-1;j++)
		{
			for(y=j-1;y<j+2;y++)
			{
				for(x=i-1;x<i+2;x++)
				{
					gr_get_rgb(gr_get_pixel(map,x,y),&r2,&g2,&b2);
					r1+=(float)(r2*tabla[c]);g1+=(float)(g2*tabla[c]);b1+=(float)(b2*tabla[c]);
					c++;
				}
			}
			r1/=tabla[9];g1/=tabla[9];b1/=tabla[9];
			r=(int)r1;
			g=(int)g1;
			b=(int)b1;

			if(r<0)r=0;
			if(g<0)g=0;
			if(b<0)b=0;

			if(!r && !g && !b)
				c=0;
			else
				c=gr_rgb(r,g,b);
			gr_put_pixel(map2,i,j,c);
			r1=0;g1=0;b1=0;c=0;
		}
	}
	memcpy(map->data,map2->data,map->height*map->pitch);
	bitmap_destroy(map2);

	return 1 ;
}

static int fxi_blur (INSTANCE *my, int *params) // fpg,map,tipo
{

	GRAPH * map = bitmap_get (params[0], params[1]), *map2;

	int x,y,i,j,c;
	int r,g,b,r2,g2,b2;

	if (map->depth != 16)
		gr_error ("Intento de usar blur con un gráfico de 8 bits") ;

	switch(params[2])
	{

		case 0:
			//METODO 1 "RAPIDO" izq y arriba
			for(i=0;i<map->width;i++)
				for(j=0;j<map->height;j++)
				{
					gr_get_rgb(gr_get_pixel(map,i,j),&r,&g,&b);
					if(!r && !g && !b)continue;
					if(i>0){
						gr_get_rgb(gr_get_pixel(map,i-1,j),&r2,&g2,&b2);
						r+=r2;g+=g2;b+=b2;
					}
					if(j>0){
						gr_get_rgb(gr_get_pixel(map,i,j-1),&r2,&g2,&b2);
						r+=r2;g+=g2;b+=b2;
					}
					r/=3;g/=3;b/=3;
					gr_put_pixel(map,i,j,gr_rgb(r,g,b));
				}
			break;

		case 1:
			// METODO2 LENTO 3x3
			r=0;g=0;b=0;c=0;
			for(i=0;i<map->width;i++)
				for(j=0;j<map->height;j++)
				{
					gr_get_rgb(gr_get_pixel(map,j,i),&r,&g,&b);
					if(!r && !g && !b)continue;
					for(x=i-1;x<i+2;x++)
					{
						for(y=j-1;y<j+2;y++)
						{
							if(x<0 || x>map->width-1 || y<0 || y>map->height-1)continue;
							gr_get_rgb(gr_get_pixel(map,x,y),&r2,&g2,&b2);
							r+=r2;g+=g2;b+=b2;
							c++;
						}
					}
					r/=c;g/=c;b/=c;
					gr_put_pixel(map,i,j,gr_rgb(r,g,b));
					r=0;g=0;b=0;c=0;
				}
			break;

		case 2:
			// METODO3 aun mas LENTO 5x5
			r=0;g=0;b=0;c=0;
			for(i=0;i<map->width;i++)
			{
				for(j=0;j<map->height;j++)
				{
					gr_get_rgb(gr_get_pixel(map,j,i),&r,&g,&b);
					if(!r && !g && !b)continue;
					for(x=i-2;x<i+3;x++)
					{
						for(y=j-2;y<j+3;y++)
						{
							if(x<0 || x>map->width-1 || y<0 || y>map->height-1)continue;
							gr_get_rgb(gr_get_pixel(map,x,y),&r2,&g2,&b2);
							r+=r2;g+=g2;b+=b2;
							c++;
						}
					}
					r/=c;g/=c;b/=c;
					gr_put_pixel(map,i,j,gr_rgb(r,g,b));
					r=0;g=0;b=0;c=0;
				}
			}
			break;

		case 3:
			// METODO4 5x5 mapa adicional
			map2 = bitmap_clone(map);
			r=0;g=0;b=0;c=0;
			for(i=0;i<map->width;i++)
			{
				for(j=0;j<map->height;j++)
				{
					gr_get_rgb(gr_get_pixel(map,j,i),&r,&g,&b);
					if(!r && !g && !b)continue;
					for(x=i-2;x<i+3;x++)
					{
						for(y=j-2;y<j+3;y++)
						{
							if(x<0 || x>map->width-1 || y<0 || y>map->height-1)continue;
							gr_get_rgb(gr_get_pixel(map,x,y),&r2,&g2,&b2);
							r+=r2;g+=g2;b+=b2;
							c++;
						}
					}
					r/=c;g/=c;b/=c;
					gr_put_pixel(map2,i,j,gr_rgb(r,g,b));
					r=0;g=0;b=0;c=0;
				}
			}
			memcpy(map->data,map2->data,map->height*map->pitch*2);
			bitmap_destroy(map2);
		break;
		default: break;

	}
	return 1 ;
}

static int fxi_grayscale (INSTANCE *my, int *params) //fpg,map,tipo
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	int i,j,c;
	int r,g,b;

	if (!map) return -1 ;
    if (map->depth != 16)
		gr_error ("Intento de usar grayscale con un gráfico de 8 bits") ;

	for(i=0;i<map->height;i++)
		for(j=0;j<map->width;j++){
			gr_get_rgb(gr_get_pixel(map,j,i),&r,&g,&b);
			if(!r && !g && !b)continue;
			c=(int)(0.3*r + 0.59*g + 0.11*b);
			switch(params[2]){
				case 0: // RGB
					c=gr_rgb(c,c,c);
					break;
				case 1: // R
					c=gr_rgb(c,0,0);
					break;
				case 2: // G
					c=gr_rgb(0,c,0);
					break;
				case 3: // B
					c=gr_rgb(0,0,c);
					break;
				case 4: // RG
					c=gr_rgb(c,c,0);
					break;
				case 5: // RB
					c=gr_rgb(c,0,c);
					break;
				case 6: // GB
					c=gr_rgb(0,c,c);
					break;
				default:
					c=gr_rgb(r,g,b);
			}
			gr_put_pixel(map,j,i,c);
		}

	return 1 ;
}

static int fxi_rgbscale (INSTANCE *my, int *params) //fpg, map, r, g, b
{
	GRAPH * map = bitmap_get (params[0], params[1]) ;
	int i,j,c;
	int r,g,b;

	if (!map) return -1 ;
	if (map->depth != 16)
		gr_error ("Intento de usar rgbscale con un gráfico de 8 bits") ;

	for(i=0;i<map->height;i++)
		for(j=0;j<map->width;j++){
			gr_get_rgb(gr_get_pixel(map,j,i),&r,&g,&b);
			if(!r && !g && !b)continue;
			c=(int)(0.3*r + 0.59*g + 0.11*b);
			c=gr_rgb((int)(c**(float *)(&params[2])),
				     (int)(c**(float *)(&params[3])),
					 (int)(c**(float *)(&params[4])));
			gr_put_pixel(map,j,i,c);
		}
	return 1 ;
}

#include "regex.h"

/** REGEX (STRING pattern, STRING string)
 *  Match a regular expresion to the given string. Fills the
 *  REGEX_REG global variables and returns the character position
 *  of the match or -1 if none found.
 */

static int fxi_regex (INSTANCE * my, int * params)
{
	const char * reg = string_get(params[0]);
	const char * str = string_get(params[1]);
	int result = -1;
	unsigned n;

	struct re_pattern_buffer pb;
	struct re_registers re;
	int start[16];
	int end[16];

	/* Alloc the pattern resources */

	memset (&pb, 0, sizeof(pb));
	memset (&re, 0, sizeof(re));
	pb.buffer = malloc(4096);
	pb.allocated = 4096;
	pb.fastmap = malloc(256);
	pb.regs_allocated = 16;
	re.num_regs = 16;
	re.start = start;
	re.end = end;

	re_syntax_options = RE_SYNTAX_POSIX_MINIMAL_EXTENDED | REG_ICASE;

	/* Match the regex */

	if (re_compile_pattern (reg, strlen(reg), &pb) == 0)
	{
		result = re_search (&pb, str, strlen(str), 0, strlen(str), &re);

		if (result != -1)
		{
			/* Fill the regex_reg global variables */

			for (n = 0 ; n < 16 && n <= pb.re_nsub ; n++)
			{
				string_discard (GLODWORD(REGEX_REG + 4*n));
				GLODWORD(REGEX_REG + 4*n) = string_newa (
					str + re.start[n], re.end[n] - re.start[n]);
				string_use (GLODWORD(REGEX_REG + 4*n));
			}
		}
	}

	/* Free the resources */
	free (pb.buffer);
	free (pb.fastmap);
	string_discard(params[0]);
	string_discard(params[1]);

	return result;
}

/** REGEX_REPLACE (STRING pattern, STRING string, STRING replacement)
 *  Match a regular expresion to the given string. For each
 *  match, substitute it with the given replacement. \0 - \9
 *  escape sequences are accepted in the replacement.
 *  Returns the resulting string. REGEX_REG variables are
 *  filled with information about the first match.
 */

static int fxi_regex_replace (INSTANCE * my, int * params)
{
	const char * reg = string_get(params[0]);
	const char * rep = string_get(params[1]);
	const char * str = string_get(params[2]);

	unsigned reg_len = strlen(reg);
	unsigned str_len = strlen(str);
	unsigned rep_len = strlen(rep);
	char * replacement;
	unsigned replacement_len;
	int fixed_replacement = strchr(rep, '\\') ? 0:1;
	
	struct re_pattern_buffer pb;
	struct re_registers re;
	int start[16];
	int end[16];

	unsigned startpos = 0;
	unsigned nextpos;
	int regex_filled = 0;

	char * result = 0;
	unsigned result_allocated = 0;
	int result_string = 0;

	unsigned n;

	/* Alloc a buffer for the resulting string */

	result = malloc(128);
	result_allocated = 128;
	*result = 0;

	/* Alloc the pattern resources */

	memset (&pb, 0, sizeof(pb));
	memset (&re, 0, sizeof(re));
	pb.buffer = malloc(4096);
	pb.allocated = 4096;
	pb.used = 0;
	pb.fastmap = malloc(256);
	pb.translate = NULL;
	pb.fastmap_accurate = 0;
	pb.regs_allocated = 16;
	re.start = start;
	re.end = end;

	re_syntax_options = RE_SYNTAX_POSIX_MINIMAL_EXTENDED;

	/* Run the regex */

	if (re_compile_pattern (reg, reg_len, &pb) == 0)
	{
		startpos = 0;
		
		while (startpos < str_len)
		{
			nextpos = re_search (&pb, str, str_len, startpos, 
				str_len - startpos, &re);
			if ((int)nextpos < 0) break;

			/* Fill the REGEX_REG global variables */

			if (regex_filled == 0)
			{
				regex_filled = 1;
				for (n = 0 ; n < 16 && n <= pb.re_nsub ; n++)
				{
					string_discard (GLODWORD(REGEX_REG + 4*n));
					GLODWORD(REGEX_REG + 4*n) = string_newa (
						str + re.start[n], re.end[n] - re.start[n]);
					string_use (GLODWORD(REGEX_REG + 4*n));
				}
			}

			/* Prepare the replacement string */

			if (fixed_replacement == 0)
			{
				int total_length = rep_len;
				const char * bptr;
				char *  ptr;
				
				/* Count the size */

				ptr = strchr(rep, '\\');
				while (ptr)
				{
					if (ptr[1] >= '0' && ptr[1] <= '9')
						total_length += re.end[ptr[1]-'0'] - re.start[ptr[1]-'0'] - 2;
					ptr = strchr(ptr+1, '\\');
				}

				/* Fill the replacement string */

				replacement = calloc (total_length+1, 1);

				bptr = rep;
				ptr = strchr(rep, '\\');
				while (ptr)
				{
					if (ptr[1] >= '0' && ptr[1] <= '9')
					{
						strncpy (replacement+strlen(replacement), bptr, ptr-bptr);
						strncpy (replacement+strlen(replacement), 
							str + re.start[ptr[1]-'0'], re.end[ptr[1]-'0'] - re.start[ptr[1]-'0']);
						bptr = ptr+2;
					}
					ptr = strchr (ptr+1, '\\');
				}
				strcat (replacement, bptr);
				replacement_len = strlen(replacement);
			}
			else
			{
				replacement = (char *)rep;
				replacement_len = rep_len;
			}

			/* Fill the resulting string */

			if (result_allocated < strlen(result)+(nextpos-startpos)+1+replacement_len)
			{
				result_allocated += ((nextpos-startpos+1+replacement_len) & ~127) + 128;
				result = realloc(result, result_allocated);
			}
			result[strlen(result)+(nextpos-startpos)] = 0;
			memcpy (result + strlen(result), str+startpos, nextpos-startpos);
			strcat (result, replacement);

			if (fixed_replacement == 0)
				free (replacement);

			/* Continue the search */

			startpos = nextpos+re_match(&pb, str, str_len, nextpos, 0);
			if (startpos <  nextpos) break;
			if (startpos == nextpos) startpos++;
		}
	}
	
	/* Copy remaining characters */

	nextpos = str_len;
	if (result_allocated < strlen(result)+(nextpos-startpos)+1)
	{
		result_allocated += ((nextpos-startpos+1) & ~127) + 128;
		result = realloc(result, result_allocated);
	}
	result[strlen(result)+(nextpos-startpos)] = 0;
	memcpy (result + strlen(result), str+startpos, nextpos-startpos);

	/* Free resources */

	free (pb.buffer);
	free (pb.fastmap);
	string_discard(params[0]);
	string_discard(params[1]);
	string_discard(params[2]);

	/* Return the new string */

	result_string = string_new(result);
	string_use(result_string);
	free(result);

	return result_string;
}

/** SPLIT (STRING regex, STRING string, STRING POINTER array, INT array_size)
 *  Fills the given array with sections of the given string, using
 *  the given regular expression as separators. Returns the number
 *  of elements filled in the array.
 *
 */

int fxi_split (INSTANCE * my, int * params)
{
	const char * reg = string_get(params[0]);
	const char * str = string_get(params[1]);
	int * result_array = (int *)VP(params[2]);
	int result_array_size = params[3];
	int count = 0;
	int pos, lastpos = 0;

	struct re_pattern_buffer pb;
	struct re_registers re;
	int start[16];
	int end[16];

	/* Alloc the pattern resources */

	memset (&pb, 0, sizeof(pb));
	memset (&re, 0, sizeof(re));
	pb.buffer = malloc(4096);
	pb.allocated = 4096;
	pb.fastmap = malloc(256);
	pb.regs_allocated = 16;
	re.num_regs = 16;
	re.start = start;
	re.end = end;

	re_syntax_options = RE_SYNTAX_POSIX_MINIMAL_EXTENDED;

	/* Match the regex */

	if (re_compile_pattern (reg, strlen(reg), &pb) == 0)
	{
		for (;;)
		{
			pos = re_search (&pb, str, strlen(str), lastpos, strlen(str), &re);
			if (pos == -1) break;
			*result_array = string_newa (str + lastpos, pos-lastpos);
			string_use(*result_array);
			result_array++;
			count++;
			result_array_size--;
			if (result_array_size == 0) break;
			lastpos = pos + re_match (&pb, str, strlen(str), pos, 0);
			if (lastpos < pos) break;
			if (lastpos == pos) lastpos++;
		}
		if (result_array_size > 0)
		{
			*result_array = string_new (str + lastpos);
			string_use (*result_array);
			count++;
		}
	}

	/* Free the resources */
	free (pb.buffer);
	free (pb.fastmap);
	string_discard(params[0]);
	string_discard(params[1]);

	return count;
}

/** JOIN (STRING separator, STRING POINTER array, INT array_size)
 *  Joins an array of strings, given a separator. Returns the
 *  resulting string.
 */

int fxi_join (INSTANCE * my, int * params)
{
	const char * sep = string_get(params[0]);
	int * string_array = (int *)VP(params[1]);
	int count = params[2] ;
	int total_length = 0;
	int sep_len = strlen(sep);
	int n;
	char * buffer;
	char * ptr;
	int result;

	for (n = 0 ; n < count ; n++)
	{
		total_length += strlen(string_get(string_array[n]));
		if (n < count-1) total_length += sep_len;
	}

	buffer = malloc(total_length+1);
	ptr = buffer;

	for (n = 0 ; n < count ; n++)
	{
		memcpy (ptr, string_get(string_array[n]), 
			  strlen(string_get(string_array[n])));
		ptr += strlen(string_get(string_array[n]));
		if (n < count-1)
		{
			memcpy (ptr, sep, sep_len);
			ptr += sep_len;
		}
	}
	*ptr = 0;
	result = string_new(buffer);
	free(buffer);
	string_use(result);
	return result;
}

/* DIRECTORY FUNCTIONS */

static int
fxi_cd(INSTANCE * my, int * params) {

	char *d = dir_current() ;
	int r = string_new(d) ;
	string_use(r) ;
	free(d) ;
	return r ;

}

static int
fxi_chdir(INSTANCE * my, int * params) {

	
	const char *d = string_get(params[0]) ;
	int r = dir_change(d) ;	

	return r ;

}

static int
fxi_mkdir(INSTANCE * my, int * params) {

	
	const char *d = string_get(params[0]) ;
	int r = dir_create(d) ;
	
	return r ;

}

static int
fxi_rmdir(INSTANCE * my, int * params) {

	
	const char *d = string_get(params[0]) ;
	int r = dir_delete(d) ;

	return r ;

}

/*  string GLOB (STRING path)
 *
 *  Given a path with wildcards ('*' or '?' characters), returns the first
 *  file that matches and, in every next call, all matching files found
 *	until no more files exists. It then returns NIL.
 */

static int fxi_glob (INSTANCE * my, int * params)
{
#ifdef WIN32
	static int last_call = 0;
	static HANDLE handle;
	const char * path = string_get(params[0]);
	WIN32_FIND_DATA data;
	SYSTEMTIME time;
	int result;
	char filename[MAX_PATH];
	char fullname[MAX_PATH];
	char buffer[128];
	char * ptr;
	
	if (last_call == params[0])
	{
		/* Continue last search */
		if (!FindNextFile (handle, &data))
		{
			/* No more matches found */
			string_discard (params[0]);
			last_call = 0;
			return 0;
		}
	}
	else
	{
		/* New search */
		handle = FindFirstFile (path, &data);
		if (handle == INVALID_HANDLE_VALUE)
		{
			/* No matches found */
			string_discard (params[0]);
			last_call = 0;
			return 0;
		}
		last_call = params[0];
	}

	/* Fill the FILEINFO struct */
	strcpy (filename, path);
	ptr = filename + strlen(filename);
	while (ptr >= filename)
	{
		if (*ptr == '\\' || *ptr == '/')
		{
			ptr[1] = 0;
			break;
		}
		ptr--;
	}
	strcat (filename, data.cFileName);
	GetFullPathName (filename, MAX_PATH, fullname, &ptr);
	if (ptr) *ptr = 0;
	
	/* Store the file path */
	if (GLODWORD(FILE_PATH))
		string_discard (GLODWORD(FILE_PATH));
	GLODWORD(FILE_PATH) = string_new (fullname);
	string_use (GLODWORD(FILE_PATH));
	
	/* Store the file name */
	if (GLODWORD(FILE_NAME))
		string_discard (GLODWORD(FILE_NAME));
	GLODWORD(FILE_NAME) = string_new (data.cFileName);
	result = GLODWORD(FILE_NAME);
	string_use (result);
	string_use (result);

	/* Store integer and boolean variables */
	GLODWORD(FILE_DIRECTORY) = ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1:0);
	GLODWORD(FILE_HIDDEN)    = ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? 1:0);
	GLODWORD(FILE_READONLY)  = ((data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? 1:0);
	GLODWORD(FILE_SIZE) = data.nFileSizeLow;

	/* Format and store the creation time */
	FileTimeToSystemTime (&data.ftCreationTime, &time);
	sprintf (buffer, "%02d/%02d/%4d %02d:%02d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute);
	if (GLODWORD(FILE_CREATED))
		string_discard(GLODWORD(FILE_CREATED));
	GLODWORD(FILE_CREATED) = string_new(buffer);
	string_use (GLODWORD(FILE_CREATED));

	/* Format and store the last write time */
	FileTimeToSystemTime (&data.ftLastWriteTime, &time);
	sprintf (buffer, "%02d/%02d/%4d %02d:%02d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute);
	if (GLODWORD(FILE_MODIFIED))
		string_discard(GLODWORD(FILE_MODIFIED));
	GLODWORD(FILE_MODIFIED) = string_new(buffer);
	string_use (GLODWORD(FILE_MODIFIED));

	string_discard (params[0]);
	return result;

#else
#  warning compiling Linux glob version, please check Mac/BeOS versions too

	const char * path = string_get(params[0]);
	char * path_final;
	const char * ptr;
	char * fptr;
	static int lastPath = 0;
	static int currentFile = 0;
	static glob_t globd;
	struct stat s;
	struct tm s_tm;
	int result;
	char buffer[128];

	/* Clean the path creating a case-insensitive match pattern */
	path_final = malloc(strlen(path)*4);
	fptr = path_final;
	ptr = path;
	while (*ptr)
	{
		if (*ptr == '\\')
			*fptr++ = '/';
		else if (*ptr >= 'a' && *ptr <= 'z')
		{
			*fptr++ = '[';
			*fptr++ = *ptr;
			*fptr++ = toupper(*ptr);
			*fptr++ = ']';
		}
		else if (*ptr >= 'A' && *ptr <= 'Z')
		{
			*fptr++ = '[';
			*fptr++ = tolower(*ptr);
			*fptr++ = *ptr;
			*fptr++ = ']';
		}
		else
			*fptr++ = *ptr;
		ptr++;
	}
	*fptr = 0;
	// Convert *.* to *
	if (fptr > path_final+2 && fptr[-1] == '*' && fptr[-2] == '.' && fptr[-3] == '*')
		fptr[-2] = 0;

	if (lastPath != params[0] && strcasecmp(path_final, string_get(lastPath)) != 0)
	{
		/* Using a different path */
		if (lastPath != 0)
		{
			string_discard(lastPath);
			globfree (&globd);
		}

		lastPath = params[0];
		string_use(params[0]);
#ifdef TARGET_MAC
		glob (path_final, GLOB_ERR | GLOB_NOSORT, NULL, &globd);
#endif
#ifdef TARGET_BeOS
		glob (path_final, GLOB_ERR | GLOB_NOSORT, NULL, &globd);
#endif
#ifdef WIN32
		glob (path_final, GLOB_ERR | GLOB_PERIOD | GLOB_NOSORT, NULL, &globd);
#endif
		currentFile = 0;
	}

	if (currentFile == globd.gl_pathc)
	{
		/* Last file reached */
		lastPath = 0;
		string_discard (params[0]);
		free (path_final);
		return 0;
	}

	stat (globd.gl_pathv[currentFile], &s);
	gr_con_printf ("%s (size %d) es un %s", 
		globd.gl_pathv[currentFile], 
		s.st_size,
		S_ISDIR(s.st_mode) ? "directorio":"fichero");

	/* Store the file name and path */
	if (GLODWORD(FILE_NAME))
		string_discard (GLODWORD(FILE_NAME));
	if (GLODWORD(FILE_PATH))
		string_discard (GLODWORD(FILE_PATH));
	ptr = strrchr (globd.gl_pathv[currentFile], '/');
	if (!ptr) 
	{
		result = string_new (ptr = globd.gl_pathv[currentFile]);
		GLODWORD(FILE_NAME) = result;
		GLODWORD(FILE_PATH) = 0;
		string_use(result);
	}
	else
	{
		GLODWORD(FILE_NAME) = result = string_new(ptr+1);
		string_use(GLODWORD(FILE_NAME));
		fptr = globd.gl_pathv[currentFile];
		GLODWORD(FILE_PATH) = string_newa(fptr, ptr-fptr);
		string_use(GLODWORD(FILE_PATH));
		ptr++;
	}

	/* Store integer and boolean variables */
	GLODWORD(FILE_DIRECTORY) = (S_ISDIR(s.st_mode) ? 1:0);
	GLODWORD(FILE_HIDDEN)    = (*ptr == '.');
	GLODWORD(FILE_READONLY)  = !(s.st_mode & 0444);
	GLODWORD(FILE_SIZE)      = s.st_size;

	/* Store file times */
	strftime (buffer, 100, "%d/%m/%Y %H:%M", localtime(&s.st_mtime));
	if (GLODWORD(FILE_MODIFIED))
		string_discard(GLODWORD(FILE_MODIFIED));
	GLODWORD(FILE_MODIFIED) = string_new(buffer);
	string_use (GLODWORD(FILE_MODIFIED));

	strftime (buffer, 100, "%d/%m/%Y %H:%M", localtime(&s.st_ctime));
	if (GLODWORD(FILE_CREATED))
		string_discard(GLODWORD(FILE_CREATED));
	GLODWORD(FILE_CREATED) = string_new(buffer);
	string_use (GLODWORD(FILE_CREATED));

	/* Return */
	currentFile++;
	string_use(result);
	string_discard (params[0]);
	free (path_final);
	return result;
#endif
}

/* ---------------------------------------------------------------------- */

#include "sysprocs.h"

static SYSPROC * sysproc_tab[256+MAX_SYSPROCS] ;

int sysproc_add (char * name, char * paramtypes, int type, void * func)
{
	static SYSPROC * last = 0 ;
	static int sysproc_count = 0 ;

	if (!last)
	{
		last = sysprocs ;
		sysproc_count++ ;
		while (last[1].func) last++, sysproc_count++ ;
	}
	last[1].code = last[0].code + 1 ;
	last[1].name = name ;
	last[1].paramtypes = paramtypes ;
	last[1].params = strlen(paramtypes) ;
	last[1].type = type ;
	last[1].func = func ;
	last[1].id   = 0 ;
	last++ ;
	sysproc_count++ ;
	if (sysproc_count == MAX_SYSPROCS)
		gr_error ("Demasiadas funciones del sistema") ;
	last[1].func = 0 ;
	return last->code ;
}

SYSPROC * sysproc_get (int code)
{
	return sysproc_tab[code] ;
}

void sysproc_init()
{
	SYSPROC       * proc = sysprocs ;
	void          * library ;
	dlfunc          RegisterFunctions ;
	const char    * filename;
	unsigned int    n ;

#ifdef TARGET_linux
	char soname[1024];
	char * ptr;
#elif defined(TARGET_BeOS)
	char soname[1024];
	char * ptr;
#endif

	for (n = 0 ; n < dcb.NImports ; n++)
	{
		filename = string_get(dcb.imports[n]) ;

#ifdef TARGET_linux
		snprintf (soname, 1024, "./%s.so", filename);

		/* Clean the name (strip .DLL, and use lowercase) */

		for (ptr = soname ; *ptr ; ptr++)
			*ptr = TOLOWER(*ptr);
		if (strlen(soname) > 7 && strcmp(ptr-7, ".dll.so") == 0)
			strcpy (ptr-7, ".so");

		library  = dlopen (soname, RTLD_NOW | RTLD_GLOBAL) ;
#elif defined(TARGET_BeOS)
		snprintf (soname, 1024, "./%s.so", filename);

		/* Clean the name (strip .DLL, and use lowercase) */

		for (ptr = soname ; *ptr ; ptr++)
			*ptr = TOLOWER(*ptr);
		if (strlen(soname) > 7 && strcmp(ptr-7, ".dll.so") == 0)
			strcpy (ptr-7, ".so");

		library  = dlopen (soname, RTLD_NOW | RTLD_GLOBAL) ;
#else
		library  = dlopen (filename, RTLD_NOW | RTLD_GLOBAL) ;
#endif
		if (!library) gr_error (dlerror()) ;

		RegisterFunctions = dlsym (library, "RegisterFunctions") ;
		if (!RegisterFunctions) gr_error("Error in %s", filename) ;

        	(*RegisterFunctions)(fnc_import, sysproc_add) ;
	}

	while (proc->func)
	{
		sysproc_tab[proc->code] = proc ;
		proc++ ;
	}

	fnc_show_information();
}
