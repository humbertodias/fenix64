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

/*
 * FILE        : g_pal.c
 * DESCRIPTION : Palette and color functions
 *
 * HISTORY:      0.82 - Blendops functions moved to g_blendop.c
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <SDL.h>

#ifdef BeIDE
#include <BeOS.h>
#endif

#include "fxi.h"

extern GRAPH * gr_read_png (const char * filename) ;

SDL_Color  palette[256] ;
SDL_Color vpalette[256] ;

Uint16     colorequiv[256] ;	/* Equivalencia paleta -> pantalla   */
Uint16     colorghost[65536] ;	/* Deja un color a 50% de intensidad */

int palette_loaded = 0 ;
int palette_changed = 0 ;

static int fade_inc = 0 ;
int        fade_on = 0 ;
int        fade_step = 64 ;
SDL_Color  fade_from ;
SDL_Color  fade_to ;
SDL_Color  fade_pos = { 100, 100, 100 };

/* Lista ordenada de colores */

int color_list[256] ;

/* Tabla de transparencias */

Uint8 trans_table[256][256] ;
int trans_table_updated = 0 ;

/* Paleta de colores por defecto */

int default_palette[] = 
{
        0,  0,  0,  16, 16, 16,  32, 32, 32,  48, 48, 48,
       64, 64, 64,  84, 84, 84, 100,100,100, 116,116,116,
      132,132,132, 148,148,148, 168,168,168, 184,184,184,
      200,200,200, 216,216,216, 232,232,232, 252,252,252,
       40,  0,  0,  72,  0,  0, 108,  0,  0, 144,  0,  0,
      180,  0,  0, 216,  0,  0, 252,  0,  0, 252, 32,  0,
      252, 64,  0, 252, 96,  0, 252,128,  0, 252,160,  0,
      252,180, 56, 252,200,112, 252,220,168, 252,240,224,
        0, 40,  0,   0, 60,  0,   0, 84,  0,   0,108,  0,
        0,132,  0,   0,156,  0,   0,180,  0,   0,204,  0,
        0,228,  0,   0,252,  0,  48,252, 32,  96,252, 64,
      144,252, 96, 192,252,132, 216,252,176, 240,252,220,
        0,  0, 40,   0,  0, 72,   0,  0,104,   0,  0,140,
        0,  0,172,   0,  0,208,   0,  0,252,   0, 48,252,
        0,100,252,   0,148,252,   0,200,252,   0,252,252,
       56,252,252, 112,252,252, 168,252,252, 224,252,252,
       28, 28,  0,  52, 52,  0,  76, 76,  0, 100,100,  0,
      124,124,  0, 152,152,  0, 176,176,  0, 200,200,  0,
      224,224,  0, 252,252,  0, 252,252, 36, 252,252, 72,
      252,252,108, 252,252,144, 252,252,180, 252,252,220,
       28,  0, 28,  52,  0, 52,  76,  0, 76, 100,  0,100,
      124,  0,124, 152,  0,152, 176,  0,176, 200,  0,200,
      224,  0,224, 252,  0,252, 252, 36,252, 252, 72,252,
      252,112,252, 252,148,252, 252,184,252, 252,224,252,
        0, 20, 20,   0, 40, 40,   0, 60, 60,   0, 80, 80,
        0,104,100,   0,124,120,   0,144,144,   0,164,164,
        0,188,184,   0,208,204,   0,228,224,   0,252,248,
       44,252,248,  92,252,248, 140,252,248, 188,252,248,
       24, 12,  0,  44, 24,  0,  64, 36,  0,  84, 48,  0,
      108, 60,  0, 128, 72,  0, 148, 84,  0, 168, 96,  0,
      192,112,  0, 196,128, 28, 204,144, 60, 212,160, 92,
      216,180,120, 224,196,152, 232,212,184, 240,232,216,
       24, 12, 12,  40, 20, 20,  60, 32, 32,  80, 44, 44,
       96, 52, 52, 116, 64, 64, 136, 76, 76, 156, 88, 88,
      176,104,104, 196,120,120, 216,136,136, 240,152,152,
      240,168,168, 244,188,188, 244,204,204, 248,224,224,
       24, 20, 12,  44, 36, 24,  68, 52, 36,  88, 72, 48,
      112, 88, 60, 132,104, 72, 156,124, 88, 172,140,100,
      188,156,112, 204,176,124, 220,192,136, 240,212,152,
      240,216,168, 244,224,188, 244,232,204, 248,240,224,
       32,  8,  0,  60, 16,  0,  88, 28,  0, 120, 36,  0,
      148, 48,  0, 176, 56,  0, 208, 68,  0, 216, 88,  0,
      224,112,  0, 232,136,  0, 240,160,  0, 252,184,  0,
      252,200, 56, 252,216,112, 252,232,168, 252,252,224,
       20, 12, 12,  36, 24, 24,  56, 36, 36,  72, 48, 48,
       92, 64, 64, 108, 76, 76, 128, 88, 88, 144,100,100,
      164,116,116, 172,132,132, 184,148,148, 192,164,164,
      204,180,180, 212,196,196, 224,212,212, 236,232,232,
       12, 20, 12,  24, 36, 24,  36, 56, 36,  48, 72, 48,
       64, 92, 64,  76,108, 76,  88,128, 88, 100,144,100,
      116,164,116, 132,172,132, 148,184,148, 164,192,164,
      180,204,180, 196,212,196, 212,224,212, 232,236,232,
       12, 12, 16,  24, 24, 32,  36, 36, 48,  48, 48, 64,
       64, 64, 80,  76, 76, 96,  88, 88,112, 100,100,128,
      116,116,148, 132,132,160, 148,148,172, 164,164,184,
      180,180,196, 196,196,208, 212,212,220, 232,232,236,
       40,  0,  0,  80,  0,  0, 124,  0,  0, 164,  0,  0,
      208,  0,  0, 252,  0,  0, 252, 40,  0, 252, 84,  0,
      252,124,  0, 252,168,  0, 252,208,  0, 252,252,  0,
      252,252, 44, 252,252, 92, 252,252,140, 252,252,188,
        0,  0,  0,   0,  0, 88,   0,  0,128,   0,  0,168,
        0,  0,208,   0,  0,248,  40,  0,248,  84,  0,248,
      124,  0,248, 168,  0,248, 208,  0,248, 252,  0,252,
      252, 52,252, 252,108,252, 252,164,252, 252,220,252
} ;


static void activate_vpalette()
{
	int n ;

	if (!screen) return ;

	if (enable_16bits)
	{
		for (n = 0 ; n < 256 ; n++)
			colorequiv[n] = SDL_MapRGB (screen->format,
				palette[n].r, palette[n].g, palette[n].b) ;
	}
	else 
	{
		for (n = 0 ; n < 256 ; n++)
		{
			if (fade_pos.r <= 100)
				vpalette[n].r = palette[n].r * fade_pos.r / 100;
			else
				vpalette[n].r = palette[n].r + (255-palette[n].r) * (fade_pos.r-100) / 100;

			if (fade_pos.g <= 100)
				vpalette[n].g = palette[n].g * fade_pos.g / 100;
			else
				vpalette[n].g = palette[n].g + (255-palette[n].g) * (fade_pos.g-100) / 100;

			if (fade_pos.b <= 100)
				vpalette[n].b = palette[n].b * fade_pos.b / 100;
			else
				vpalette[n].b = palette[n].b + (255-palette[n].b) * (fade_pos.b-100) / 100;
		}

		SDL_SetColors (screen, vpalette, 0, 256) ;
	}
}

void gr_roll_palette (int color0, int num, int inc)
{
	SDL_Color backup[256] ;

	if (!scr_initialized) gr_init (320, 200) ;

	if (color0 < 0 || color0 > 255) return ;
	if (num+color0 > 255) num = 256-color0 ;
	while (inc >= num) inc -= num ;
	while (inc <= -num) inc += num ;
	if (!inc) return ;

        memcpy  (&backup[color0],  &palette[color0],     sizeof(SDL_Color)*num) ;

	if (inc < 0) /* Derecha */
	{
		inc = -inc ;
		memmove (&palette[color0+inc], &palette[color0], sizeof(SDL_Color)*(num-inc)) ;
		memcpy  (&palette[color0], &backup[color0+num-inc], sizeof(SDL_Color)*inc) ;
	}
	else /* Izquierda */
	{
		memmove (&palette[color0], &palette[color0+inc], sizeof(SDL_Color)*(num-inc)) ;
		memcpy  (&palette[color0+num-inc], &backup[color0], sizeof(SDL_Color)*inc) ;
	}

	palette_changed = 1 ;
}

void gr_fade_init (int r, int g, int b, int speed)
{
	fade_inc  = speed;
	fade_step = 0;
	fade_on   = 1 ;
	fade_from = fade_pos;
	fade_to.r = r ;
	fade_to.g = g ;
	fade_to.b = b ;

	GLODWORD(FADING) = 1 ;
}

void gr_fade_step()
{
	if (fade_on != 0)
	{
        GLODWORD(FADING) = 1 ;

		fade_step += fade_inc ;
		if (fade_step < 0) 
		{
		    GLODWORD(FADING) = 0 ;
			fade_step = 0 ;
			fade_on = 0 ;
		}
		if (fade_step >= 64) 
		{
	        GLODWORD(FADING) = 0 ;
			fade_step = 64 ;
			fade_on = 0 ;
		}

		fade_pos.r = (fade_to.r * fade_step + fade_from.r * (64-fade_step)) / 64;
		fade_pos.g = (fade_to.g * fade_step + fade_from.g * (64-fade_step)) / 64;
		fade_pos.b = (fade_to.b * fade_step + fade_from.b * (64-fade_step)) / 64;

		if ((fade_step + fade_inc < 0 || fade_step + fade_inc > 64) &&
			(fade_pos.r == 100 && fade_pos.g == 100 && fade_pos.b == 100))
		{
	        GLODWORD(FADING) = 0 ;
			fade_on = 0;
		}

		activate_vpalette() ;

		if (scrbitmap->depth == 16)
		{
			gr_fade16 (scrbitmap,  fade_pos.r, fade_pos.g, fade_pos.b);
		}
	}
}

int gr_read_pal (file * fp)
{
	unsigned char colors[256][3] ;
	int i ;

	if (!file_read (fp, colors, 3 * 256))
		return 0 ;

	/* Ignora definiciones de gama */
	file_seek (fp, 576, SEEK_CUR) ;

	for (i = 0 ; i < 256 ; i++)
	{
		palette[i].r = colors[i][0] << 2 ;
		palette[i].g = colors[i][1] << 2 ;
		palette[i].b = colors[i][2] << 2 ;
	}

	palette_loaded = 1 ;
	palette_changed = 1 ;

	return 1 ;
}

/*
 *  FUNCTION : gr_save_pal
 *
 *  Saves the current palette to the given file
 *
 *  PARAMS : 
 *		filename		Name of file to create
 *
 *  RETURN VALUE : 
 *      1 if succeded, 0 if error
 */

int gr_save_pal (const char * filename)
{
	file * fp = file_open (filename, "wb");
	char header[8] = "pal\x1A\x0D\x0A";
	unsigned char colors[256][3] ;
	int i;

	if (!fp)
		return 0;

	for (i = 0 ; i < 256 ; i++)
	{
		colors[i][0] = palette[i].r / 4 ;
		colors[i][1] = palette[i].g / 4 ;
		colors[i][2] = palette[i].b / 4 ;
	}

	file_write (fp, header, 8);
	file_write (fp, colors, 768);

	memset (colors, 0, 576);
	file_write (fp, colors, 576);
	file_close (fp);

	return 1;
}

int gr_load_pal (const char * filename)
{
	file * fp = file_open (filename, "rb") ;
	char header[8] ;
	int r = 0 ;

	if (!fp) return -1 ;
	file_read (fp, header, 8) ;
	if (strcmp (header, "map\x1A\x0D\x0A") == 0)
	{
		file_seek (fp, 48, SEEK_SET) ;
		r = gr_read_pal (fp) ;
	}
	else if (strcmp (header, "fpg\x1A\x0D\x0A") == 0 ||
	    strcmp (header, "fnt\x1A\x0D\x0A") == 0 ||
	    strcmp (header, "pal\x1A\x0D\x0A") == 0)
	{
		r = gr_read_pal (fp) ;
	}
	else if (memcmp (header, "\x89PNG", 4) == 0)
	{
		GRAPH * graph ;

		file_close (fp);
		palette_loaded = 0;
		graph = gr_read_png(filename);
		if (graph) bitmap_destroy(graph);
		return 1;
	}
	file_close(fp) ;
	return r;
}

int gr_rgb (int r, int g, int b)
{
	int color ;

	if (!scr_initialized) gr_init (320, 200) ;

	r >>= screen->format->Rloss ;
	g >>= screen->format->Gloss ;
	b >>= screen->format->Bloss ;

	color= (r << screen->format->Rshift) |
	       (g << screen->format->Gshift) |
	       (b << screen->format->Bshift) ;
	if (!color) return 1 ;
	return color ;
}

void gr_get_rgb (int color, int *r, int *g, int *b)
{
	if (!scr_initialized) gr_init (320, 200) ;

	if (!enable_16bits)
	{
		color &= 0xFF ;
		(*r) = palette[color].r ;
		(*g) = palette[color].g ;
		(*b) = palette[color].b ;
		return ;
	}

	(*r) = ((color & screen->format->Rmask) >> screen->format->Rshift) ;
	(*g) = ((color & screen->format->Gmask) >> screen->format->Gshift) ;
	(*b) = ((color & screen->format->Bmask) >> screen->format->Bshift) ;

        (*r) <<= screen->format->Rloss ;
        (*g) <<= screen->format->Gloss ;
        (*b) <<= screen->format->Bloss ;
}

/* Busca el color más cercano en la paleta a uno determinado */

/* Emplea una caché interna que acelera considerablemente el proceso,
 * a cambio de perder una parte de precisión */

Uint8 nearest_table[64][64][64] ;

inline int find_nearest_color (const int first, const int last, SDL_Color key)
{
	int n, diff ;
	int best, bestdiff;
	SDL_Color * c ;

        n = nearest_table[key.r/4][key.g/4][key.b/4] ;
        if (!n && key.r/4 > 0) n = nearest_table[key.r/4-1][key.g/4][key.b/4] ;
        if (!n && key.g/4 > 0) n = nearest_table[key.b/4][key.g/4-1][key.b/4] ;
        if (!n && key.b/4 > 0) n = nearest_table[key.b/4][key.g/4][key.b/4-1] ;
        if (n) return n ;

        //printf ("*** %02d %02d %02d NOT FOUND ****\n", key.r/8, key.g/8, key.b/8) ;

	bestdiff = 999 ;
	
	for (n = first ; n <= last ; n++)
	{
		c = &palette[color_list[n]] ;
		diff =  (key.r > c->r ? key.r-c->r : c->r-key.r)  
		     +  (key.g > c->g ? key.g-c->g : c->g-key.g)  
		     +  (key.b > c->b ? key.b-c->b : c->b-key.b) ;
		if (diff < bestdiff) {
			bestdiff = diff ;
			best     = n ;
		}
	}

	return nearest_table[key.r/4][key.g/4][key.b/4] = color_list[best] ;
}

int gr_find_nearest_color (int r, int g, int b)
{
	SDL_Color c ;
	c.r = r ;
	c.g = g ;
	c.b = b ;

	return find_nearest_color(0, 255, c) ;
}

Uint16 gr_get_color (int r, int g, int b)
{
	return SDL_MapRGB (screen->format, (Uint8)r, (Uint8)g, (Uint8)b) ;
}

/* ----------------------------------------------------------------------
 * Función que rellena la tabla de color más cercano (nearest_table)
 * ---------------------------------------------------------------------- */

#define DUP 1
#define DDOWN 2
#define DRIGHT 3
#define DLEFT 4
#define DIN 5
#define DOUT 6

typedef struct __fillpoint {
        Uint8 r, g, b, dir, color ;
        struct __fillpoint * next ;
        struct __fillpoint * prev ;
} fillpoint ;

static fillpoint * filling = 0 ;

inline fillpoint * add_filling(int r, int g, int b, int color, int dir) 
{ 
        fillpoint * n ;
        
        if (nearest_table[r*2][g*2][b*2] != 0)
                return 0 ;

        //printf ("%02d %02d %02d -> %d\n", r, g, b, color) ;

        nearest_table[r*2+0][g*2+0][b*2+0] = color ;
        nearest_table[r*2+0][g*2+0][b*2+1] = color ;
        nearest_table[r*2+0][g*2+1][b*2+0] = color ;
        nearest_table[r*2+0][g*2+1][b*2+1] = color ;
        nearest_table[r*2+1][g*2+0][b*2+0] = color ;
        nearest_table[r*2+1][g*2+0][b*2+1] = color ;
        nearest_table[r*2+1][g*2+1][b*2+0] = color ;
        nearest_table[r*2+1][g*2+1][b*2+1] = color ;

        n = malloc(sizeof(fillpoint)) ;
        n->r = r ;
        n->g = g ;
        n->b = b ;
        n->color = color ;
        n->dir = dir ;
        n->next = filling ; 
        if (filling)
                filling->prev = n ;
        n->prev = 0 ;
        return filling = n ; 
}

inline void del_filling(fillpoint * n)
{
        if (n->prev)
                n->prev->next = n->next ;
        else
                filling = n->next ;
        if (n->next)
                n->next->prev = n->prev ;
        free (n) ;
}

void gr_fill_nearest_table()
{
        int n ;
        SDL_Color c ;
        register fillpoint * ptr ;
        fillpoint * next ;

        for (n = 1 ; n < 256 ; n++)
        {
		c = palette[n] ;
                c.r /= 8 ;
                c.g /= 8 ;
                c.b /= 8 ;
                add_filling (c.r, c.g, c.b, n, DRIGHT) ;
                if (c.r < 31) add_filling (c.r+1, c.g,   c.b,   n, DRIGHT) ;
                if (c.r > 0)  add_filling (c.r-1, c.g,   c.b,   n, DLEFT) ;
                if (c.g < 31) add_filling (c.r,   c.g+1, c.b,   n, DDOWN) ;
                if (c.g > 0)  add_filling (c.r,   c.g-1, c.b,   n, DUP) ;
                if (c.b < 31) add_filling (c.r,   c.g,   c.b+1, n, DIN) ;
                if (c.b > 0)  add_filling (c.r,   c.g,   c.b-1, n, DOUT) ;
        }

        while (filling)
        {
                ptr = filling ;
                while (ptr)
                {
                        if (ptr->r < 31) add_filling(ptr->r+1, ptr->g,   ptr->b,   ptr->color, DRIGHT) ;
                        if (ptr->r > 0)  add_filling(ptr->r-1, ptr->g,   ptr->b,   ptr->color, DLEFT) ;
                        if (ptr->g < 31) add_filling(ptr->r,   ptr->g+1, ptr->b,   ptr->color, DUP) ;
                        if (ptr->g > 0)  add_filling(ptr->r,   ptr->g-1, ptr->b,   ptr->color, DDOWN) ;
                        if (ptr->b < 31) add_filling(ptr->r,   ptr->g,   ptr->b+1, ptr->color, DIN) ;
                        if (ptr->b > 0)  add_filling(ptr->r,   ptr->g,   ptr->b-1, ptr->color, DOUT) ;
                        next = ptr->next ;
                        del_filling (ptr) ;
                        ptr = next ;
                }
        }

}

/* ----------------------------------------------------------------------
 * Rellena el contenido de la tabla de transparencias
 * ---------------------------------------------------------------------- */

static int compare_colors (const void * p1, const void * p2)
{
	SDL_Color * c1 = &palette[*(int *)p1] ;
	SDL_Color * c2 = &palette[*(int *)p2] ;

	return (c1->r + c1->g + c1->b) - (c2->r + c2->g + c2->b) ;
}

void gr_make_trans_table()
{
	int ticks1, ticks2 ;
	int a, b, c1, c2, n  ;
	SDL_Color base, dest ;

	if (trans_table_updated)
		return;

	ticks1 = SDL_GetTicks() ;

	memset (nearest_table, 0, sizeof(nearest_table)) ;
	for (n = 1 ; n < 256 ; n++) 
	{
		SDL_Color *c = &palette[n] ;
		nearest_table[c->r/4][c->g/4][c->b/4] = n ;
	}

	for (n = 0 ; n < 256 ; n++) color_list[n] = n ;
	qsort (color_list, 256, sizeof(int), compare_colors) ;

	gr_fill_nearest_table() ;

	for (a = 0 ; a < 256 ; a++)
	{
		c1 = color_list[a] ;
		base.r = palette[c1].r/2 ;
		base.g = palette[c1].g/2 ;
		base.b = palette[c1].b/2 ;

		for (b = 0 ; b < a ; b++)
		{
			c2 = color_list[b] ;
			dest.r = base.r + palette[c2].r/2 ;
			dest.g = base.g + palette[c2].g/2 ;
			dest.b = base.b + palette[c2].b/2 ;

			trans_table[c1][c2] = 
			trans_table[c2][c1] = 
				find_nearest_color (b, a, dest) ;
		}

		trans_table[c1][c1] = c1 ;
		trans_table[0][c1]  = c1 ;
	}

        trans_table_updated = 1 ;

	ticks2 = SDL_GetTicks() ;
        if (report_graphics)
                gr_con_printf ("[GRAPH] Palette analysis done in %d ms\n", 
                                ticks2-ticks1) ;
}

void gr_refresh_palette()
{
    int n ;

	if (!scr_initialized) return ;

	memset (nearest_table, 0, sizeof(nearest_table)) ;

	for (n = 0 ; n < 256 ; n++) color_list[n] = n ;
	qsort (color_list, 256, sizeof(int), compare_colors) ;

	trans_table_updated = 0 ;

	/* Actualiza la paleta */

	palette_changed = 0 ;

	if (fade_step != 100) 
	{
		memcpy (vpalette, palette, sizeof(vpalette)) ;
		activate_vpalette() ;
	}
	else 
	{
		if (enable_16bits)
			for (n = 0 ; n < 256 ; n++)
				colorequiv[n] = SDL_MapRGB (screen->format,
					palette[n].r, palette[n].g, palette[n].b) ;
		else
			SDL_SetColors (screen, palette, 0, 256) ;
	}
}

void gr_set_rgb (int color, int r, int g, int b)
{
	if (color < 0 || color > 255) return ;
	palette[color].r = r << 2 ;
	palette[color].g = g << 2 ;
	palette[color].b = b << 2 ;
	palette_changed  = 1 ;
}

/* Rutinas para guardar y recibir trozos de paleta desde un buffer */

void gr_get_colors (int color, int num, Uint8 * pal)
{
	if (num < 1 || color < 0 || color > 255) return ;
	if (color+num >= 256) num = 256-color ;

	while (num--)
	{
		*pal++ = palette[color  ].r ;
		*pal++ = palette[color  ].g ;
		*pal++ = palette[color++].b ;
	}
}

void gr_set_colors (int color, int num, Uint8 * pal)
{
	if (num < 1 || color < 0 || color > 255) return ;
	if (color+num >= 256) num = 256-color ;

	while (num--)
	{
		palette[color  ].r = *pal++ ;
		palette[color  ].g = *pal++ ;
		palette[color++].b = *pal++ ;
	}
	palette_changed = 1 ;
}

