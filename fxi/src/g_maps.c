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
 * FILE        : g_maps.c
 * DESCRIPTION : bitmap & pixbuff functions
 *
 * HISTORY:      0.81 - patched bitmap_new to initializa to 0 values
 *               0.82 - added gr_save_map
 */

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "fxi.h"

extern SDL_Surface * screen ;

extern GRAPH * gr_read_png (const char * filename) ;
extern GRAPH * gr_read_pcx (const char * filename) ;

static int free_map_code = 1000 ;

static GRAPH * first = NULL;

/* Returns the code of a new system library graph (1000+). Searchs
   for free slots if the program creates too many system maps */

int bitmap_next_code()
{
    int i;

    if (free_map_code > 2000)
    {
        for (i = free_map_code-1 ; i >= 1000 ; i--)
        {
            if (bitmap_get(0, i) == 0)
                return i;
        }
    }
    return free_map_code++;
}

GRAPH * bitmap_new (int code, int w, int h, int depth, int frames)
{
    GRAPH  * gr ;
    int      bytesPerRow, wb ;

    if (depth != 8 && depth != 16 && depth != 1)
    {
        gr_con_printf ("Profundidad de color no soportada (new bitmap)\n") ;
        return NULL;
    }

    if (frames < 1) frames = 1;

    /* Calculate the row size (dword-aligned) */

    wb = w * depth / 8;
    if ((wb * 8 / depth) < w) wb++;

    bytesPerRow = wb;
    if (bytesPerRow & 0x03) bytesPerRow = (bytesPerRow & ~3) + 4;

    /* Create and fill the struct */

    gr = (GRAPH *) malloc (sizeof(GRAPH)) ;
    if (!gr)
    {
        gr_con_printf ("bitmap_new(%dx%dx%d): sin memoria", w, h, depth) ; ;
        return NULL;
    }

    memset (gr, 0, sizeof(GRAPH));

    gr->width            = w ;
    gr->widthb           = wb ;
    gr->pitch            = bytesPerRow ;
    gr->frames           = frames;
    gr->height           = h ;
    gr->depth            = depth ;
    gr->code             = code ;
    gr->data_start       = (char *) malloc (h * gr->pitch * frames) ;
    gr->data             = gr->data_start;
    gr->cpoints          = NULL ;
    gr->blend_table      = NULL ;
    gr->keyframes        = (FBM_KEYFRAME *) malloc(sizeof(FBM_KEYFRAME));
    gr->sequences        = (FBM_SEQUENCE *) malloc(sizeof(FBM_SEQUENCE));
    gr->next             = first;
    gr->prev             = NULL;
    gr->current_keyframe = -1;

    gr->palette          = NULL ;

    if (first) first->prev = gr;
    first = gr;

    if (!gr->data || !gr->keyframes || !gr->sequences)
    {
        if (gr->data) free(gr->data);
        if (gr->keyframes) free(gr->keyframes);
        if (gr->sequences) free(gr->sequences);
        free (gr);

        gr_con_printf ("bitmap_new: sin memoria en calloc(%d, %d)", h, gr->pitch) ; ;
        return NULL;
    }

    // Fills default data for the animation

    gr->keyframes->angle = 0;
    gr->keyframes->flags = 0;
    gr->keyframes->frame = 0;
    gr->keyframes->pause = 25;

    gr->sequences->first_keyframe = 0;
    gr->sequences->last_keyframe = 0;
    gr->sequences->next_sequence = 0;
    gr->sequences->name[0] = 0;

    return gr ;
}

GRAPH * bitmap_clone (GRAPH * map)
{
    GRAPH * gr ;
    Uint32 y;

    gr = bitmap_new_syslib(map->width, map->height, map->depth, 1) ;
    if (gr == NULL) return NULL;
    for (y = 0 ; y < map->height ; y++)
    {
        memcpy ((Uint8*)gr->data_start + gr->pitch * y,
                (Uint8*)map->data + gr->pitch * y,
                gr->widthb);
    }
    if (map->cpoints)
    {
        gr->cpoints = malloc(4 * map->ncpoints) ;
        memcpy (gr->cpoints, map->cpoints, 4 * map->ncpoints) ;
        gr->ncpoints = map->ncpoints ;
    }
    gr->width = map->width;
    gr->blend_table = map->blend_table;
    gr->offset = map->offset ;
    gr->info_flags = map->info_flags ;
    gr->modified = map->modified ;
    gr->palette = map->palette ;
    pal_use(map->palette);
    memcpy (gr->name, map->name, sizeof(map->name)) ;
    return gr ;
}

void bitmap_add_cpoint (GRAPH * map, int x, int y)
{
    map->cpoints = (CPOINT *) realloc (map->cpoints, (map->ncpoints+1) * sizeof(CPOINT)) ;
    map->cpoints[map->ncpoints].x = x ;
    map->cpoints[map->ncpoints].y = y ;
    map->ncpoints++;
}


/*
 *  FUNCTION : bitmap_set_cpoint
 *
 *  Set a control point in a graphic
 *
 *  PARAMS :
 *      map             Pointer to the bitmap
 *      point           Control point index
 *      x               New X coordinate or CPOINT_UNDEFINED to unset
 *      y               New Y coordinate or CPOINT_UNDEFINED to unset
 *
 *  RETURN VALUE :
 *      None
 *
 */

void bitmap_set_cpoint (GRAPH * map, Uint32 point, int x, int y)
{
    Uint32 n;

    if (point < 0)
        return;

    if (map->ncpoints <= point)
    {
        map->cpoints = (CPOINT *) realloc (map->cpoints, (point+1) * sizeof(CPOINT)) ;
        for (n = map->ncpoints; n < point; n++)
        {
            map->cpoints[n].x = CPOINT_UNDEFINED;
            map->cpoints[n].y = CPOINT_UNDEFINED;
        }
        map->ncpoints = point+1 ;
    }
    map->cpoints[point].x = x;
    map->cpoints[point].y = y;
}

void bitmap_destroy (GRAPH * map)
{
    if (!map) return;

    if (map->prev) map->prev->next = map->next;
    if (map->next) map->next->prev = map->prev;
/*  if (map->next_time) map->next->prev = map->next; */
    if (first == map) first = map->next;

    if (map->cpoints) free(map->cpoints) ;
    if (map->keyframes) free(map->keyframes);
    if (map->sequences) free(map->sequences);

    pal_destroy(map->palette);

    free (map->data) ;
    free (map) ;
}

void bitmap_destroy_fake (GRAPH * map)
{
    if (!map) return;

    if (map->prev) map->prev->next = map->next;
    if (map->next) map->next->prev = map->prev;
/*  if (map->next_time) map->next->prev = map->next; */
    if (first == map) first = map->next;

    if (map->cpoints) free(map->cpoints);
    if (map->keyframes) free(map->keyframes);
    if (map->sequences) free(map->sequences);

    pal_destroy(map->palette);

    free (map) ;
}

static GRAPH * gr_read_map (file * fp)
{
    char header[8] ;
    unsigned short int w, h, c ;
    Uint32 y ;
    int depth, code ;
    GRAPH * gr ;
    int len;
    PALETTE * pal = NULL;

    /* Carga los datos de cabecera */

    file_read (fp, header, 8) ;
    if (strcmp (header, M16_MAGIC) == 0)
        depth = 16 ;
    else if (strcmp (header, MAP_MAGIC) == 0)
        depth = 8 ;
    else if (strcmp (header, M01_MAGIC) == 0)
        depth = 1 ;
    else
        return NULL ;

    file_readUint16 (fp, &w) ;
    file_readUint16 (fp, &h) ;
    file_readSint32 (fp, &code) ;

    gr = bitmap_new (code, w, h, depth, 1) ;
    if (!gr) return NULL ;
    file_read (fp, gr->name, 32) ;
    gr->name[31] = 0 ;

    /* Datos de paleta */

    if (gr->depth == 8)
        if (!(pal = gr_read_pal_with_gamma (fp))) {
            bitmap_destroy(gr);
            return NULL ;
        }

    /* Puntos de control */

    file_readUint16 (fp, &c) ;
    gr->ncpoints = c ;

    if (gr->ncpoints)
    {
        gr->cpoints = (CPOINT *) malloc (c * sizeof(CPOINT)) ;
        if (!gr->cpoints) {
            bitmap_destroy(gr);
            pal_destroy(pal);
            return NULL ;
        }

        for (c = 0 ; c < gr->ncpoints ; c++)
        {
            file_readUint16 (fp, &w) ;
            file_readUint16 (fp, &h) ;
            if ((short int) w == -1 && (short int) h == -1)
            {
                w = CPOINT_UNDEFINED;
                h = CPOINT_UNDEFINED;
            }
            gr->cpoints[c].x = w ;
            gr->cpoints[c].y = h ;
        }
    }
   else
        gr->cpoints = 0 ;

    len = gr->widthb;

    /* Datos del gráfico */

    for (y = 0 ; y < gr->height ; y++)
    {
        Uint8 * line = (Uint8 *)gr->data + gr->pitch*y;
        if (!file_read (fp, line, len))
        {
            bitmap_destroy(gr);
            pal_destroy(pal);
            return NULL ;
        }
        if (gr->depth == 16)
        {
            ARRANGE_WORDS (line, len/2);
            if (scr_initialized) gr_convert16_565ToScreen ((Uint16 *)line, len/2);
        }
    }

	if (gr->depth == 8 && !gr->palette) gr->palette = pal;

    gr->modified = 1 ;

    return gr ;
}

/* Funciones de carga de nivel superior */

int gr_load_png (const char * mapname)
{
    GRAPH * gr ;

    gr = gr_read_png (mapname) ;
    if (!gr) return -1 ;
    gr->code = bitmap_next_code() ;
    assert (syslib) ;
    grlib_add_map (0, gr) ;
    return gr->code ;
}

int gr_load_pcx (const char * mapname)
{
    GRAPH * gr ;

    gr = gr_read_pcx (mapname) ;
    if (!gr) return -1 ;
    gr->code = bitmap_next_code() ;
    assert (syslib) ;
    grlib_add_map (0, gr) ;
    return gr->code ;
}

int gr_load_map (const char * mapname)
{
    GRAPH * gr ;
    file * fp = file_open (mapname, "rb") ;

    if (!fp)
    {
        gr_con_printf ("Mapa %s no encontrado\n", mapname) ;
        return -1 ;
    }

    gr = fbm_load_from (fp, 0);
    if (!gr)
    {
        file_close(fp);
        fp = file_open(mapname, "rb");
        if (fp) gr = gr_read_map (fp) ;
    }
    file_close (fp) ;

    if (!gr) return -1 ;

    gr->code = bitmap_next_code() ;
    assert (syslib) ;
    grlib_add_map (0, gr) ;
    return gr->code ;
}

GRAPH * bitmap_new_syslib (int w, int h, int depth, int frames)
{
    GRAPH * gr ;

    gr = bitmap_new (0, w, h, depth, frames) ;
    if (!gr) return NULL;
    gr->code = bitmap_next_code() ;
    assert (syslib) ;
    grlib_add_map (0, gr) ;
    return gr ;
}

/* Análisis */

void bitmap_analize (GRAPH * bitmap)
{
    Uint32 x, y;

    if (bitmap->modified > 0)
        bitmap->modified = 0 ;
    bitmap->info_flags = 0 ;

    /* Search for transparent pixels (value 0).
     * If none found, set the flag GI_NOCOLORKEY */

    if (bitmap->depth == 8)
    {
        for (y = 0 ; y < bitmap->height ; y++)
        {
            Uint8 * ptr = memchr ((Uint8 *)bitmap->data + bitmap->pitch*y, 0, bitmap->width) ;
            if (ptr) break;
        }
        if (y == bitmap->height)
            bitmap->info_flags |= GI_NOCOLORKEY ;
    }
    else
    {
        Uint16 * ptr = (Uint16 *)bitmap->data ;
        for (y = 0 ; y < bitmap->height ; y++, ptr += bitmap->pitch/2)
        {
            for (x = 0 ; x < bitmap->width ; x++)
                if (ptr[x] == 0) break;
            if (x != bitmap->width)
                break;
        }
        if (y == bitmap->height)
            bitmap->info_flags |= GI_NOCOLORKEY ;
    }
}

/* Animación */

void bitmap_animate_to (GRAPH * bitmap, int pos, int speed)
{
}

/*
 *  FUNCTION : bitmap_animate
 *
 *  Update the current bitmap frame using the current_time global,
 *  advancing the bitmap animation if needed
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      scrx, scry      Pixel coordinates of the center on screen
 *      gr              Pointer to the graphic object to draw
 *
 *  RETURN VALUE :
 *      None
 *
 */

void bitmap_animate (GRAPH * map)
{
    if (map->last_frame == frame_count || map->next_time > current_time || current_time == 0)
        return;

    // Check for internal error conditions that shouldn't happen

    if (map->current_keyframe > map->max_keyframe) map->current_keyframe = 0;
    if (map->current_sequence > map->max_sequence) map->current_sequence = 0;

    map->last_frame = frame_count;

    // Update the animation

    map->end_of_sequence = 0;

    // Advance to the next keyframe

    map->current_keyframe++;
    map->next_time += map->keyframes[map->current_keyframe].pause;

    // At end of sequence, move to the next sequence of mark the map
    // as if it is at the end of the animation

    if (map->current_keyframe > map->sequences[map->current_sequence].last_keyframe)
    {
        if (map->sequences[map->current_sequence].next_sequence >= 0)
        {
            map->current_sequence = map->sequences[map->current_sequence].next_sequence;
            if (map->current_sequence > map->max_sequence) map->current_sequence = 0;

            map->current_keyframe = map->sequences[map->current_sequence].first_keyframe;
        }
        else
        {
            map->current_keyframe--;
            map->end_of_sequence = 1;
        }
    }

    // This is an internal error condition; shouldn't happen
    if (map->current_keyframe > map->max_keyframe) map->current_keyframe = 0;

    map->current_frame = map->keyframes[map->current_keyframe].frame;
    if (map->current_frame > map->frames-1) map->current_frame = 0;

    // Calculate the next time, only if there was a long wait since
    // the last update

    map->next_time = current_time + map->keyframes[map->current_keyframe].pause;

    // Adjust the graphic data

    map->data = (Uint8*)map->data_start + map->height * map->pitch * map->current_frame;

}

/*
 *  FUNCTION : bitmap_16bits_conversion
 *
 *  When 16 bits mode is initialized for the first time, this
 *  function will convert every 16 bit bitmap in memory to
 *  the screen format
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      None
 *
 */

void bitmap_16bits_conversion()
{
    GRAPH * map = first;

    while (map)
    {
        if (map->depth == 16)
            gr_convert16_565ToScreen (map->data_start, map->width * map->height * map->frames);

        map = map->next;
    }
}
