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

#include "fxi.h"

/* Indicadores de bloqueo */
#define GRAPH_HWRAP 1
#define GRAPH_VWRAP 2
#define BACK_HWRAP 4
#define BACK_VWRAP 8

scrolldata scrolls[10] ;

typedef struct _scroll_Extra_data
{
	Sint32	x0 ;
	Sint32	y0 ;
	Sint32	x1 ;
	Sint32	y1 ;
	Sint32	z ;
	Sint32  camera ;
	Sint32  ratio ;
	Sint32  speed ;
	Sint32  region1 ;
	Sint32  region2 ;
	Sint32  flags1 ;
	Sint32  flags2 ;
	Sint32  follows ;
	Sint32  reserved[7] ;
}
__PACKED
SCROLL_EXTRA_DATA ;

void gr_scroll_bbox (int n, REGION * r)
{
	if (n >= 0 && n <= 9)
		*r = *scrolls[n].region;
}

void scroll_region (int n, REGION * r)
{
	if (n < 0 || n > 9) return ;
	/* Corrected from x,y... to posx,posy... so out_region works fine */
	r->x  -= scrolls[n].posx0 ;
	r->y  -= scrolls[n].posy0 ;
	r->x2 -= scrolls[n].posx0 ;
	r->y2 -= scrolls[n].posy0 ;
}

/* Averigua si las regiones de scroll ocupan toda la pantalla */

static int scroll_fills_region (REGION r)
{
	int n ;
	REGION r2 ;
	SCROLL_EXTRA_DATA * data ;
    GRAPH * graph ;
    int flags ;

	if (r.x > r.x2 || r.y > r.y2) return 1 ;

	for (n = 0 ; n < 10 ; n++)
	{
		data = &((SCROLL_EXTRA_DATA *)&GLODWORD(SCROLLS))[n] ;
		if (!scrolls[n].active) continue ;
                if (scrolls[n].back)
                {
                        graph = scrolls[n].back ;
                        flags = data->flags2 ;
                }
                else
                {
                        graph = scrolls[n].graph ;
                        flags = data->flags1 ;
                }
                if (!graph) continue ;
		if (flags & B_TRANSLUCENT) continue ;

		// Descarta el scroll si puede verse a través

		if (!(flags & B_NOCOLORKEY))
		{
			if (graph->modified)
				bitmap_analize(graph) ;
			if (!(graph->info_flags & GI_NOCOLORKEY))
				continue ;
		}

		if (scrolls[n].region->x <= r.x && scrolls[n].region->y <= r.y &&
		    scrolls[n].region->x2 > r.x && scrolls[n].region->y2 > r.y)
		{
			r2 = r ; r2.x = scrolls[n].region->x2+1 ;
			if (!scroll_fills_region(r2)) return 0 ;
			r2 = r ; r2.y = scrolls[n].region->y2+1 ;
			r2.x2 = scrolls[n].region->x2 ;
			return scroll_fills_region(r2) ;
		}
	}
	return 0 ;
}

int gr_scroll_is_fullscreen ()
{
	REGION base ;

	base.x = 0 ;
	base.y = 0 ;
	base.x2 = scrbitmap->width-1 ;
	base.y2 = scrbitmap->height-1 ;
	return scroll_fills_region(base) ;
}

void gr_scroll_start (int n, int fileid, int graphid, int backid, int region, int flags)
{
	if (n >= 0 && n <= 9)
	{
		object_list_dirty = 1;

		if (region < 0 || region > 31) region = 0 ;

		scrolls[n].active  = 1 ;
		scrolls[n].fileid  = fileid ;
		scrolls[n].graphid = graphid ;
		scrolls[n].backid  = backid ;
		scrolls[n].region  = &regions[region] ;
		scrolls[n].flags   = flags ;

		scrolls[n].graph = graphid ? bitmap_get (fileid, graphid) : 0 ;
		scrolls[n].back  = backid  ? bitmap_get (fileid, backid)  : 0 ;

		if (!graphid || !scrolls[n].graph)
			gr_error ("El fondo de scroll %d:%d no existe\n", fileid, graphid) ;
		if ( backid && !scrolls[n].back )
			gr_error ("Grafico %d:%d no existe\n", fileid, backid) ;
	}
}

void gr_scroll_stop (int n)
{
	if (n >= 0 && n <= 9)
	{
		object_list_dirty = 1;
		scrolls[n].active = 0 ;
	}
}

static int compare_instances (const void * ptr1, const void * ptr2)
{
	const INSTANCE * i1 = *(const INSTANCE **)ptr1 ;
	const INSTANCE * i2 = *(const INSTANCE **)ptr2 ;

	return LOCDWORD(i2,COORDZ) - LOCDWORD(i1,COORDZ);
}

void gr_scroll_draw (int n, int do_drawing, REGION * clipping)
{
	int nproc, x, y, x0, y0, x1, y1, cx, cy, w, h, speed ;

	static INSTANCE ** proclist = 0  ;
	static int proclist_reserved = 0 ;
	int proclist_count ;
	REGION r;

	SCROLL_EXTRA_DATA * data ;
	INSTANCE * i ;

	if (n < 0 || n > 9) return ;

	data = &((SCROLL_EXTRA_DATA *)&GLODWORD(SCROLLS))[n] ;

	if (!scrolls[n].active || !scrolls[n].region || !scrolls[n].graph)
		return ;

	w = scrolls[n].region->x2 - scrolls[n].region->x + 1 ;
	h = scrolls[n].region->y2 - scrolls[n].region->y + 1 ;

	scrolls[n].z       = data->z ;
	scrolls[n].ratio   = data->ratio ;
	scrolls[n].camera  = instance_get(data->camera) ;
	scrolls[n].speed   = data->speed ;

	if (data->follows >= 0 && data->follows <= 9)
		scrolls[n].follows = &scrolls[data->follows] ;
	else
		scrolls[n].follows = 0 ;

	if (data->region1 < 0  || data->region1 > 31)
		scrolls[n].region1 = 0 ;
	else
		scrolls[n].region1 = &regions[data->region1] ;

	if (data->region2 < 0  || data->region2 > 31)
		scrolls[n].region2 = 0 ;
	else
		scrolls[n].region2 = &regions[data->region2] ;

	/* Actualiza las variables globales (perseguir la camara, etc) */

	if (scrolls[n].follows)
	{
		if (scrolls[n].ratio)
		{
			data->x0 = scrolls[n].follows->x0 * 100 / scrolls[n].ratio ;
			data->y0 = scrolls[n].follows->y0 * 100 / scrolls[n].ratio ;
		}
		else
		{
			data->x0 = scrolls[n].follows->x0 ;
			data->y0 = scrolls[n].follows->y0 ;
		}
	}

	if (scrolls[n].camera)
	{
		instance_update_bbox (scrolls[n].camera) ;

		/* Mira a ver si entra dentro de la region o la 2 */

		speed = scrolls[n].speed ;
		if (scrolls[n].speed == 0) speed = 9999999 ;

		x0 = LOCDWORD(scrolls[n].camera,BOX_X0) - data->x0 ;
		y0 = LOCDWORD(scrolls[n].camera,BOX_Y0) - data->y0 ;
		x1 = LOCDWORD(scrolls[n].camera,BOX_X1) - data->x0 ;
		y1 = LOCDWORD(scrolls[n].camera,BOX_Y1) - data->y0 ;

		if (scrolls[n].region1)
		{
			if (x0 < scrolls[n].region1->x2 &&
			    y0 < scrolls[n].region1->y2 &&
			    x1 >= scrolls[n].region1->x &&
			    y1 > scrolls[n].region1->y)
			{
				speed = 0 ;
			}
		}

		if (scrolls[n].region2)
		{
			if (x0 < scrolls[n].region1->x2 &&
			    y0 < scrolls[n].region1->y2 &&
			    x1 >= scrolls[n].region1->x &&
			    y1 > scrolls[n].region1->y)
			{
				speed = 99999999 ;
			}
		}

		/* Forzar a que esté en el centro de la ventana */

		cx = LOCDWORD(scrolls[n].camera,COORDX) ;
		cy = LOCDWORD(scrolls[n].camera,COORDY) ;

        RESOLXY(scrolls[n].camera, cx, cy);

		cx -= w/2 ;
		cy -= h/2 ;

		if (data->x0 < cx) data->x0 = MIN(data->x0+speed, cx) ;
		if (data->y0 < cy) data->y0 = MIN(data->y0+speed, cy) ;
		if (data->x0 > cx) data->x0 = MAX(data->x0-speed, cx) ;
		if (data->y0 > cy) data->y0 = MAX(data->y0-speed, cy) ;
    }

    /* Scrolls no cíclicos y posición del background */

    if (scrolls[n].graph)
    {
		if (!(scrolls[n].flags & GRAPH_HWRAP))
			data->x0 = MAX (0, MIN (data->x0, (int)scrolls[n].graph->width - w)) ;
		if (!(scrolls[n].flags & GRAPH_VWRAP))
			data->y0 = MAX (0, MIN (data->y0, (int)scrolls[n].graph->height - h)) ;
	}

	if (scrolls[n].ratio)
	{
		data->x1 = data->x0 * 100 / scrolls[n].ratio ;
		data->y1 = data->y0 * 100 / scrolls[n].ratio ;
        }

        if (scrolls[n].back)
        {
		if (!(scrolls[n].flags & BACK_HWRAP))
			data->x1 = MAX (0, MIN (data->x1, (int)scrolls[n].back->width - w)) ;
		if (!(scrolls[n].flags & BACK_VWRAP))
			data->y1 = MAX (0, MIN (data->y1, (int)scrolls[n].back->height - h)) ;
	}

	/* Actualiza la posición del scroll según las variables globales */

	scrolls[n].posx0 = data->x0 ;
	scrolls[n].posy0 = data->y0 ;
	scrolls[n].x0 = data->x0 % scrolls[n].graph->width  ;
	scrolls[n].y0 = data->y0 % scrolls[n].graph->height ;
	if (scrolls[n].x0 < 0) scrolls[n].x0 += scrolls[n].graph->width ;
	if (scrolls[n].y0 < 0) scrolls[n].y0 += scrolls[n].graph->height ;

	if (scrolls[n].back)
	{
		scrolls[n].posx1 = data->x1 ;
		scrolls[n].posy1 = data->y1 ;
		scrolls[n].x1 = data->x1 % scrolls[n].back->width ;
		scrolls[n].y1 = data->y1 % scrolls[n].back->height ;
		if (scrolls[n].x1 < 0) scrolls[n].x1 += scrolls[n].back->width ;
		if (scrolls[n].y1 < 0) scrolls[n].y1 += scrolls[n].back->height ;
	}

	if (!do_drawing) return ;

	/* Dibuja el fondo */

	r = *scrolls[n].region;
	if (clipping)
		region_union(&r, clipping);

	if (scrolls[n].back)
	{
		if (scrolls[n].back->ncpoints > 0 &&
			scrolls[n].back->cpoints[0].x >= 0)
		{
			cx = scrolls[n].back->cpoints[0].x ;
			cy = scrolls[n].back->cpoints[0].y ;
		}
		else
		{
			cx = scrolls[n].back->width / 2 ;
			cy = scrolls[n].back->height / 2 ;
		}
		y = scrolls[n].region->y - scrolls[n].y1 ;

		while (y < scrolls[n].region->y2)
		{
			x = scrolls[n].region->x - scrolls[n].x1 ;
			while (x < scrolls[n].region->x2)
			{
				gr_blit (0, &r, x+cx, y+cy, data->flags2, scrolls[n].back) ;
				x += scrolls[n].back->width ;
			}
			y += scrolls[n].back->height ;
		}
	}

	/* Dibuja el primer plano */

	if (scrolls[n].graph->ncpoints > 0 &&
		scrolls[n].graph->cpoints[0].x >= 0)
	{
		cx = scrolls[n].graph->cpoints[0].x ;
		cy = scrolls[n].graph->cpoints[0].y ;
	}
	else
	{
		cx = scrolls[n].graph->width / 2 ;
		cy = scrolls[n].graph->height / 2 ;
	}
	y = scrolls[n].region->y - scrolls[n].y0 ;

	while (y < scrolls[n].region->y2)
	{
		x = scrolls[n].region->x - scrolls[n].x0 ;
		while (x < scrolls[n].region->x2)
		{
			gr_blit (0, &r, x+cx, y+cy, data->flags1, scrolls[n].graph) ;
			x += scrolls[n].graph->width ;
		}
		y += scrolls[n].graph->height ;
	}

	/* Crea una lista ordenada de instancias a dibujar */

	i = first_instance ;
	proclist_count = 0 ;
	while (i)
	{
		if (((LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK) == STATUS_RUNNING ||
		     (LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK) == STATUS_FROZEN     ) &&
		    LOCDWORD(i,CTYPE) == 1)
		{
			if (LOCDWORD(i,CFLAGS) && !(LOCDWORD(i,CFLAGS) & (1<<n)))
			{
				i = i->next ;
				continue ;
			}

			if (proclist_count == proclist_reserved)
			{
				proclist_reserved += 16 ;
				proclist = (INSTANCE **) realloc (proclist, sizeof(INSTANCE *) * proclist_reserved) ;
			}

			proclist[proclist_count++] = i ;
		}
		i = i->next ;
	}

	/* Ordena la listilla */

	qsort (proclist, proclist_count, sizeof(INSTANCE *), compare_instances) ;

	/* Visualiza los procesos */

	for (nproc = 0 ; nproc < proclist_count ; nproc++)
	{
		x = LOCDWORD (proclist[nproc], COORDX) ;
		y = LOCDWORD (proclist[nproc], COORDY) ;

        RESOLXY(proclist[nproc], x, y);

		draw_instance_at (proclist[nproc], &r,
				          x - scrolls[n].posx0 + scrolls[n].region->x,
				          y - scrolls[n].posy0 + scrolls[n].region->y) ;
	}
}

int gr_scroll_active (int n)
{
	return scrolls[n].active ;
}
