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
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <math.h>

#include "fxi.h"
#include "fmath.h"

#define FOCAL_DIST 128

extern Uint8 trans_table[256][256] ;
extern int trans_table_updated ;

typedef struct _mode7
{
    int     active ;

    REGION *    region ;
    GRAPH * indoor ;
    GRAPH * outdoor ;
}
MODE7 ;

typedef struct _mode7_info
{
    Uint32  camera_id ;
    Uint32  height ;
    Uint32  distance ;
    Uint32  horizon ;
    Uint32  focus ;
    Uint32  z ;
    Uint32  color ;
    Uint32  flags ;
    Uint32  reserved[2] ;
}
__PACKED
MODE7_INFO ;

MODE7 current_mode7[10] ;

void gr_mode7_bbox (int n, REGION * r)
{
    if (n >= 0 && n <= 9) *r = *current_mode7[n].region;
}

int gr_mode7_active (int n)
{
    return (n >= 0 && n <= 9) ? current_mode7[n].active : 0 ;
}

void gr_mode7_stop (int n)
{
    object_list_dirty = 1;
    current_mode7[n].active = 0 ;
}

void gr_mode7_start (int n, int fileid, int inid, int outid, int region, int horizon)
{
    MODE7_INFO * dat ;
    if (n < 0 || n > 9) return ;

    dat = &((MODE7_INFO *)&GLODWORD(M7STRUCTS))[n];
    dat->height     = 32;
    dat->distance   = 64;
    dat->horizon    = horizon;
    dat->focus      = 256;
    dat->z          = 256;
    dat->color      = 0;
    dat->flags      = 0;

    object_list_dirty = 1;

    current_mode7[n].active  = 1 ;
    current_mode7[n].indoor  = bitmap_get (fileid, inid) ;
    current_mode7[n].outdoor = bitmap_get (fileid, outid) ;
    current_mode7[n].outdoor = bitmap_get (fileid, outid) ;
    current_mode7[n].region  = region_get (region) ;

}

/* Funciones generales de rotación en 2D
 *
 *  x1  =  x * cos(angulo)  -  y * sin(angulo)
 *
 *  y1  =  x * sin(angulo)  +  y * cos(angulo)
 *
 * */

#define ROTATEDX(x,y,sina,cosa) (fmul(x,cosa) - fmul(y,sina))
#define ROTATEDY(x,y,sina,cosa) (fmul(x,sina) + fmul(y,cosa))

/* Esta estructura guarda información que se recalcula a cada frame.
 * Quizá no sería necesario recalcularlo todo. El incremento podría
 * quedar constante mientras no variara la altura, p.ej. */

typedef struct _lineinfo
{
    fixed   left_bmp_x ;
    fixed   left_bmp_y ;
    fixed   right_bmp_x ;
    fixed   right_bmp_y ;

    fixed   hinc, vinc ;
}
LINEINFO ;

static LINEINFO lines[1280] ;

static int compare_by_distance (const void * ptr1, const void * ptr2)
{
    const INSTANCE * i1 = *(const INSTANCE **)ptr1 ;
    const INSTANCE * i2 = *(const INSTANCE **)ptr2 ;

    return LOCDWORD(i2,DISTANCE_1) - LOCDWORD(i1,DISTANCE_1);
}

void gr_mode7_draw (int n)
{
    fixed   bmp_x, bmp_y ;
    fixed   base_x,   base_y,   base_z ;
    fixed   camera_x, camera_y, camera_z ;
    fixed   point_x,  point_y,  point_z ;
    fixed   cosa,  sina  ;
    fixed   angle ;
    fixed   hinc, vinc ;

    int horizon_y   = -1;
    int jump ;

    int outdoor_hmask = 0 ;
    int outdoor_vmask = 0 ;

    Uint32  sx, sy ;
    int     x, y, z, height, width ;
    Uint8   * ptr, * baseline, c ;

    MODE7 * mode7   = &current_mode7[n] ;
    GRAPH * indoor  = mode7->indoor ;
    GRAPH * outdoor = mode7->outdoor ;
    GRAPH * dest    = scrbitmap ;
    GRAPH * pgr ;

    MODE7_INFO * dat  = &((MODE7_INFO *)&GLODWORD(M7STRUCTS))[n] ;
    INSTANCE   * camera ;

    static INSTANCE ** proclist = 0  ;
    static int proclist_reserved = 0 ;
    int proclist_count, nproc ;

    INSTANCE * i ;

    if (!cos_table_initialized) init_cos_tables() ;

    /* Averigua la posición inicial de dibujo */

    camera = instance_get (dat->camera_id) ;
    if (!camera) return ;

    angle = LOCDWORD(camera, ANGLE) ;

    cosa = fcos (-angle) ;
    sina = fsin (-angle) ;

    /* Averigua la posición de inicio */

    camera_x = itofix(LOCDWORD (camera, COORDX)) ;
    camera_y = itofix(LOCDWORD (camera, COORDY)) ;
    camera_z = itofix(dat->height) ;

    RESOLXY(camera, camera_x, camera_y);

    camera_x -= cosa * dat->distance ;
    camera_y -= sina * dat->distance ;

    if (dat->flags & B_VMIRROR) camera_z = -camera_z ;

    /* Bucle para sacar las sub-posiciones de cada línea */

    width  = mode7->region->x2 - mode7->region->x + 1 ;
    height = mode7->region->y2 - mode7->region->y + 1 ;

    if (!height || !width) return ;

//    horizon_y = 0 ;

    for (y = height ; y >= 0 ; y--)
    {
        /* Representa en las 3D el punto (0, y) de pantalla */

        base_x = itofix(FOCAL_DIST) ;
        base_y = -itofix(dat->focus/2) ;
        base_z = itofix(dat->focus/2) - itofix(y * dat->focus / height) ;

        /* Rota dicho punto según el ángulo del proceso */

        point_x = ROTATEDX (base_x, base_y, sina, cosa) + camera_x ;
        point_y = ROTATEDY (base_x, base_y, sina, cosa) + camera_y ;
        point_z = base_z + camera_z ;

        /* Aplica la fórmula (ver mode7.txt) */

        if (point_z == camera_z)
        {
            horizon_y = y ;
            continue ;
        }
        //if (point_z >= camera_z) break ;

        lines[y].left_bmp_x = fdiv( fmul((point_x - camera_x), -camera_z), (point_z - camera_z) ) + camera_x ;
        lines[y].left_bmp_y = fdiv( fmul((point_y - camera_y), -camera_z), (point_z - camera_z) ) + camera_y ;

        /* Lo mismo para el punto (width,y) */

        base_x = itofix(FOCAL_DIST) ;
        base_y = itofix(dat->focus/2) ;
        base_z = itofix(dat->focus/2) - itofix(y * dat->focus / height) ;

        point_x = ROTATEDX (base_x, base_y, sina, cosa) + camera_x ;
        point_y = ROTATEDY (base_x, base_y, sina, cosa) + camera_y ;
        point_z = base_z + camera_z ;

        //if (point_z >= camera_z) break ;

        lines[y].right_bmp_x = fdiv(fmul((point_x - camera_x), -camera_z), (point_z - camera_z)) + camera_x ;
        lines[y].right_bmp_y = fdiv(fmul((point_y - camera_y), -camera_z), (point_z - camera_z)) + camera_y ;

        /* Averigua el incremento necesario para cada paso de la línea */

        lines[y].hinc = (lines[y].right_bmp_x - lines[y].left_bmp_x) / width ;
        lines[y].vinc = (lines[y].right_bmp_y - lines[y].left_bmp_y) / width ;
    }

    /* Bucle principal */

    if ( horizon_y == -1 ) return ;

    y = horizon_y ;
    ptr = (Uint8 *)dest->data + dest->pitch*y + mode7->region->x ;

    if (outdoor)
    {
        outdoor_hmask = outdoor_vmask = 0xFFFFFFFF ;
        while (~(outdoor_hmask << 1) < (int)outdoor->width-1) outdoor_hmask <<= 1 ;
        while (~(outdoor_vmask << 1) < (int)outdoor->height-1) outdoor_vmask <<= 1 ;
        outdoor_hmask = ~outdoor_hmask ;
        outdoor_vmask = ~outdoor_vmask ;
    }

    jump = (camera_z < 0) ? -1 : 1 ;
    ptr += ((jump > 0) ? dest->pitch : -(int)dest->pitch) ;
    y   += jump ;

    if (dest->depth != 8) gr_error ("Profundidad de color no soportada\n(mode7, dest)") ;
    if (outdoor && outdoor->depth != 8) gr_error ("Profundidad de color no soportada\n(mode7, out)") ;
    if (indoor && indoor->depth != 8) gr_error ("Profundidad de color no soportada\n(mode7, in)") ;

    if ((dat->flags & B_TRANSLUCENT) && !trans_table_updated) gr_make_trans_table() ;

    if (!(dat->flags & B_TRANSLUCENT))
        for ( ; y < height && y >= 0 ; y += jump)
        {
            if (dat->flags & B_HMIRROR)
            {
                bmp_x = lines[y].right_bmp_x ;
                bmp_y = lines[y].right_bmp_y ;
                hinc  = -lines[y].hinc ;
                vinc  = -lines[y].vinc ;
            }
            else
            {
                bmp_x = lines[y].left_bmp_x ;
                bmp_y = lines[y].left_bmp_y ;
                hinc  = lines[y].hinc ;
                vinc  = lines[y].vinc ;
            }

            baseline = ptr ;

            for (x = 0 ; x < width ; x++)
            {
                sx = fixtoi (bmp_x) ;
                sy = fixtoi (bmp_y) ;

                if (indoor && sx >= 0 && sx < indoor->width &&
                    sy >= 0 && sy < indoor->height)
                {
                    c = ((Uint8*)indoor->data)[indoor->pitch*sy + sx] ;
                    if (c > 0) *ptr   = c ;
                    ptr++ ;
                }
                else
                {
                    if (outdoor) c = ((Uint8*)outdoor->data)[outdoor->pitch*(sy & outdoor_vmask) + (sx & outdoor_hmask)] ;
                    else         c = dat->color ;
                    if (c > 0) *ptr   = c ;
                    ptr++ ;
                }

                bmp_x += hinc ;
                bmp_y += vinc ;
            }

            ptr = baseline + ((jump > 0) ? dest->pitch : -(int)dest->pitch) ;
        }
        else
            for ( ; y < height && y >= 0 ; y += jump)
            {
                if (dat->flags & B_HMIRROR)
                {
                    bmp_x = lines[y].right_bmp_x ;
                    bmp_y = lines[y].right_bmp_y ;
                    hinc  = -lines[y].hinc ;
                    vinc  = -lines[y].vinc ;
                }
                else
                {
                    bmp_x = lines[y].left_bmp_x ;
                    bmp_y = lines[y].left_bmp_y ;
                    hinc  = lines[y].hinc ;
                    vinc  = lines[y].vinc ;
                }

                baseline = ptr ;

                for (x = 0 ; x < width ; x++)
                {
                    sx = fixtoi (bmp_x) ;
                    sy = fixtoi (bmp_y) ;

                    if (indoor && sx >= 0 && sx < indoor->width && sy >= 0 && sy < indoor->height) {
                        *ptr = trans_table[((Uint8*)indoor->data)[indoor->pitch*sy + sx]][*ptr] ;
                        ptr++;
                    } else {
                        if (outdoor) {
                            *ptr = trans_table[((Uint8*)outdoor->data)[outdoor->pitch*(sy & outdoor_vmask) + (sx & outdoor_hmask)]][*ptr] ;
                            ptr++;
                        } else {
                            *ptr = trans_table[dat->color][*ptr] ;
                            ptr++;
                        }
                    }

                    bmp_x += hinc ;
                    bmp_y += vinc ;
                }

                ptr = baseline + ((jump > 0) ? dest->pitch : -(int)dest->pitch) ;
            }

    /* Crea una lista ordenada de instancias a dibujar */

    i = first_instance ;
    proclist_count = 0 ;
    while (i)
    {
        if (((LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK) == STATUS_RUNNING ||
             (LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK) == STATUS_FROZEN)
             && LOCDWORD(i,CTYPE) == 2)
        {
            if (LOCDWORD(i,CFLAGS) && !(LOCDWORD(i,CFLAGS) & (1<<(n-1))))
            {
                i = i->next ;
                continue ;
            }

            if (proclist_count == proclist_reserved)
            {
                proclist_reserved += 16 ;
                proclist = (INSTANCE **) realloc (proclist, sizeof(INSTANCE *) * proclist_reserved) ;
            }

            /* Averigua la distancia a la cámara */

            x = LOCDWORD(i, COORDX) ;
            y = LOCDWORD(i, COORDY) ;

            RESOLXY(i, x, y);

            x -= fixtoi(camera_x) ;
            y -= fixtoi(camera_y) ;

            LOCDWORD(i,DISTANCE_1) = ftofix(sqrt ((double)x*x + (double)y*y)) ;

            proclist[proclist_count++] = i ;
        }
        i = i->next ;
    }

    /* Ordena la listilla */

    qsort (proclist, proclist_count, sizeof(INSTANCE *), compare_by_distance) ;

    /* Visualiza los procesos */

    for (nproc = 0 ; nproc < proclist_count ; nproc++)
    {
        pgr = instance_graph(proclist[nproc]) ;
        if (!pgr) continue ;

        if (LOCDWORD (proclist[nproc], DISTANCE_1) <= 0) continue ;

        base_x = LOCDWORD(proclist[nproc], COORDX) ;
        base_y = LOCDWORD(proclist[nproc], COORDY) ;
        base_z = LOCDWORD(proclist[nproc], HEIGHT) ;

        RESOLXYZ(proclist[nproc], base_x, base_y, base_z);

        base_x = itofix(base_x) - camera_x ;
        base_y = itofix(base_y) - camera_y ;
        base_z = itofix(base_z) + dat->height - camera_z ;

        x = ROTATEDX(base_x,base_y,cosa,sina) ;
        y = ROTATEDY(base_x,base_y,cosa,sina) ;
        z = base_z ;

        if (y <= 0) continue ;

        bmp_x = (long) (-((float)FOCAL_DIST * fixtof(x) / fixtof(y)) * (float)width / (float) dat->focus) ;
        bmp_y = (long) (-((float)FOCAL_DIST * fixtof(z) / fixtof(y)) * (float)dat->height / (float) dat->focus) ;

        x = LOCDWORD(proclist[nproc], GRAPHSIZE) ;
        LOCDWORD(proclist[nproc], GRAPHSIZE) = dat->focus * 8 / fixtoi(LOCDWORD(proclist[nproc], DISTANCE_1)) ;
        draw_instance_at (proclist[nproc], mode7->region,
                          mode7->region->x + width/2  + bmp_x,
                          mode7->region->y + height/2 + bmp_y) ;
        LOCDWORD(proclist[nproc], GRAPHSIZE) = x ;

    }
}

