/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.82
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
 *				 0.82 - added gr_save_map
 */

#include <posix/assert.h>
#include <string.h>
#include <stdlib.h>

#include <SDL.h>

#include "fxi.h"

extern SDL_Surface * screen ;

extern GRAPH * gr_read_png (const char * filename) ;
extern GRAPH * gr_read_pcx (const char * filename) ;

static int free_map_code = 1000 ;

/* Returns the code of a new system library graph (1000+). Searchs
   for free slots if the program creates too many system maps */

static int get_free_map_code()
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

GRAPH * bitmap_new (int code, int w, int h, int depth)
{
	GRAPH  * gr ;
	int      bytesPerRow, wb ;

	if (depth != 8 && depth != 16 && depth != 1)
	{
		gr_error ("Profundidad de color no soportada\n(new bitmap)") ;
		return NULL;
	}

	/* Calculate the row size (dword-aligned) */

	wb = w * depth / 8;
	if (wb*8/depth < w)
		wb++;

	bytesPerRow = wb;
	if (bytesPerRow & 0x03)
		bytesPerRow = (bytesPerRow & ~3) + 4;

	/* Create and fill the struct */

	gr = (GRAPH *) malloc (sizeof(GRAPH)) ;
	if (!gr) 
	{
		gr_error ("bitmap_new(%dx%dx%d): sin memoria", w, h, depth) ; ;
		return NULL;
	}

	gr->width       = w ;
	gr->widthb      = wb ;
	gr->pitch       = bytesPerRow ;
	gr->height      = h ;
	gr->depth       = depth ;
	gr->name[0]     = 0 ;
	gr->code        = code ;
	gr->data        = (char *) malloc (h * gr->pitch) ;
	gr->offset      = 0 ;
	gr->flags       = 0 ;
	gr->modified    = 0 ;
	gr->info_flags  = 0 ;
	gr->cpoints     = NULL ;
	gr->animation   = NULL ;
	gr->blend_table = NULL ;

	if (!gr->data) 
	{
		free (gr);
		gr_error ("bitmap_new: sin memoria en calloc(%d, %d)", h, gr->pitch) ; ;
		return NULL;
	}
	return gr ;
}

GRAPH * bitmap_clone (GRAPH * this)
{
	GRAPH * gr ;
	int y;
	int w;

	w = this->animation ? this->animation->frames * this->width : this->width;
	gr = bitmap_new_syslib(w, this->height, this->depth) ;
	if (gr == NULL) return NULL;
	for (y = 0 ; y < this->height ; y++)
	{
		memcpy ((Uint8*)gr->data + gr->pitch * y,
			    (Uint8*)this->data + gr->pitch * y,
				gr->widthb);
	}
	if (this->animation)
	{
		gr->animation = malloc(sizeof(ANIMATION)) ;
		memcpy (gr->animation, this->animation, sizeof(ANIMATION)) ;
		gr->animation->order = malloc(2 * (this->animation->length+1)) ;
		memcpy (gr->animation->order, this->animation->order,
			2 * (this->animation->length)) ;
	}
	if (this->cpoints)
	{
		gr->cpoints = malloc(4 * (this->flags & F_NCPOINTS)) ;
		memcpy (gr->cpoints, this->cpoints, 4 * (this->flags & F_NCPOINTS)) ;
	}
	gr->width = this->width ;
	gr->flags = this->flags ;
	gr->offset = this->offset ;
	gr->info_flags = this->info_flags ;
	gr->modified = this->modified ;
	memcpy (gr->name, this->name, sizeof(this->name)) ;
	return gr ;
}


void bitmap_add_cpoint (GRAPH * map, int x, int y)
{
	int n = (map->flags & F_NCPOINTS) ;
	map->flags = ((map->flags & ~F_NCPOINTS) | (n+1)) ;
	map->cpoints = (CPOINT *) realloc (map->cpoints,
			(n+1) * sizeof(CPOINT)) ;
	map->cpoints[n].x = x ;
	map->cpoints[n].y = y ;
}

void bitmap_destroy (GRAPH * map)
{
	if (map->cpoints) free(map->cpoints) ;
        if (map->animation) {
                free (map->animation->order) ;
                free (map->animation) ;
        }
	free (map->data) ;
	free (map) ;
}


static GRAPH * gr_read_map (file * fp)
{
	char header[8] ;
	unsigned short int w, h, c ;
	int y ;
	int depth, code ;
	GRAPH * gr ;
	int len;

	/* Carga los datos de cabecera */

	file_read (fp, header, 8) ;
	if (strcmp (header, M16_MAGIC) == 0) 
		depth = 16 ;
	else if (strcmp (header, MAP_MAGIC) == 0) 
		depth = 8 ;
	else if (strcmp (header, M01_MAGIC) == 0) 
		depth = 1 ;
	else
		return 0 ;

	file_readUint16 (fp, &w) ;
	file_readUint16 (fp, &h) ;
	file_readSint32 (fp, &code) ;

	gr = bitmap_new (code, w, h, depth) ;
	if (!gr) return 0 ;
	file_read (fp, gr->name, 32) ;
	gr->name[31] = 0 ;

	/* Datos de paleta */

	if (gr->depth == 8)
	{
		if (palette_loaded)
			file_seek (fp, 576 + 768, SEEK_CUR) ;
		else
			if (!gr_read_pal (fp)) return 0 ;
	}

	/* Puntos de control */

	file_readUint16 (fp, &c) ;
	gr->flags = c ;

	if (gr->flags & F_NCPOINTS)
	{
		gr->cpoints = (CPOINT *) malloc ((c & F_NCPOINTS) * sizeof(CPOINT)) ;
		if (!gr->cpoints) { free(gr) ; return 0 ; }
		for (c = 0 ; c < (gr->flags & F_NCPOINTS) ; c++)
		{
			file_readUint16 (fp, &w) ;
			file_readUint16 (fp, &h) ;
			gr->cpoints[c].x = w ;
			gr->cpoints[c].y = h ;
		}
	}
	else	gr->cpoints = 0 ;

	len = gr->widthb;

	if (gr->flags & F_ANIMATION)
	{
		if (!gr->animation) gr->animation = malloc(sizeof(ANIMATION)) ;
		if (!gr->animation) { free(gr) ; return 0 ; }
		file_readUint16 (fp, &c) ; gr->animation->frames = c ;
		file_readUint16 (fp, &c) ; gr->animation->length = c ;
		file_readUint16 (fp, &c) ; gr->animation->speed = c ;
		gr->animation->remaining = gr->animation->speed ;
		gr->animation->order = (Sint16 *) malloc(gr->animation->length * 2) ;
		if (!gr->animation->order) { free(gr) ; return 0 ; }
		file_read (fp, gr->animation->order, gr->animation->length * 2) ;
		ARRANGE_WORDS (gr->animation->order, gr->animation->length);
		gr->pitch *= gr->animation->frames ;
		len *= gr->animation->frames;
	}

	/* Datos del gráfico */

	for (y = 0 ; y < gr->height ; y++)
	{
		Uint8 * line = (Uint8 *)gr->data + gr->pitch*y;
		if (!file_read (fp, line, len))
		{
			free (gr->data) ;
			free (gr) ;
			return 0 ;
		}
		if (gr->depth == 16) 
		{
			ARRANGE_WORDS (line, len/2);
			gr_convert16_565ToScreen ((Uint16 *)line, len/2);
		}
	}

	gr->modified = 1 ;
	return gr ;
}

/* Funciones de carga de nivel superior */

int gr_load_png (const char * mapname)
{
	GRAPH * gr ;

	gr = gr_read_png (mapname) ;
	if (!gr) return -1 ;
	gr->code = get_free_map_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr->code ;
}

int gr_load_pcx (const char * mapname)
{
	GRAPH * gr ;

	gr = gr_read_pcx (mapname) ;
	if (!gr) return -1 ;
	gr->code = get_free_map_code() ;
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
		gr_error ("Mapa %s no encontrado\n", mapname) ;
		return -1 ;
	}
	gr = gr_read_map (fp) ;
	file_close (fp) ;
	if (!gr) return -1 ;
	gr->code = get_free_map_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr->code ;
}

// Saving a map

int gr_save_map (GRAPH * gr, const char * filename)
{
	MAP_HEADER  	mh ;
	char			fname[1024] ;
	gzFile *		file ;
	unsigned char	colors[256][3] ;
	unsigned char * block ;
	int				i;
	int				linesize;
	Uint8 *         lineptr;
	Sint16          flags;

	strncpy (fname, filename, 1000);
	if (!strrchr(fname,'.')) 
		strcat (fname,".map") ;

	file = gzopen(fname,"wb") ;
	if (!file) 
	{
		gr_error ("No se pudo crear el fichero %s.\n", fname) ;
		return -1 ;
	}
	
	if (gr->depth == 8)
		strcpy(mh.magic,MAP_MAGIC) ;
	else if (gr->depth == 16)
		strcpy(mh.magic,M16_MAGIC) ;
	else if (gr->depth == 1)
		strcpy(mh.magic,M01_MAGIC) ;
	else
	{
		gr_error ("gr_save_map: profundidad de color no soportada");
		return -1;
	}

	mh.width = gr->width ;
	mh.height = gr->height ;
	mh.code = gr->code ;
	strncpy (mh.name, gr->name, sizeof(mh.name)) ;

	ARRANGE_WORD (&mh.width);
	ARRANGE_WORD (&mh.height);
	ARRANGE_DWORD (&mh.code);

	gzwrite (file, &mh, sizeof(MAP_HEADER)) ;

	// Palette...

	if (gr->depth == 8) 
	{
		block = calloc(576,1) ;
		for (i=0 ; i<256 ; i++) 
		{
			colors[i][0] = palette[i].r >> 2 ;
			colors[i][1] = palette[i].g >> 2 ;
			colors[i][2] = palette[i].b >> 2 ;
		}
		gzwrite (file, &colors, 768) ;		
		gzwrite (file, block, 576) ;		
		free(block) ;
	}
	
	flags = gr->flags;
	ARRANGE_WORD (&flags);
	gzwrite (file, &flags, 2) ;	

	if (gr->flags & F_NCPOINTS) 
	{
		ARRANGE_WORDS ((Sint16 *)gr->cpoints, gr->flags & F_NCPOINTS);
		gzwrite (file, gr->cpoints, 4 * (gr->flags & F_NCPOINTS)) ;	
		ARRANGE_WORDS ((Sint16 *)gr->cpoints, gr->flags & F_NCPOINTS);
	}

	// Animation

	linesize = gr->widthb;

	if (gr->flags & F_ANIMATION) 
	{
		linesize *= gr->animation->frames;

		ARRANGE_WORD  (&gr->animation->frames);
		ARRANGE_WORD  (&gr->animation->length);
		ARRANGE_WORD  (&gr->animation->speed);
		ARRANGE_WORDS (&gr->animation->order, gr->animation->length);

		gzwrite (file, &gr->animation->frames, 2) ;
		gzwrite (file, &gr->animation->length, 2) ;
		gzwrite (file, &gr->animation->speed, 2) ;
		gzwrite (file, &gr->animation->order, 2 * gr->animation->length) ;

		ARRANGE_WORD  (&gr->animation->frames);
		ARRANGE_WORD  (&gr->animation->length);
		ARRANGE_WORD  (&gr->animation->speed);
		ARRANGE_WORDS (&gr->animation->order, gr->animation->length);
	}	

	// Pixbuffer

	lineptr = gr->data;

	if (gr->depth == 16)
	{
		if ( (block = malloc(gr->widthb)) == NULL)
		{
			gr_error ("gr_save_map: sin memoria");
			gzclose (file);
			return 0;
		}
	}

	for (i = 0 ; i < gr->height ; i++, lineptr += gr->pitch)
	{
		if (gr->depth == 16)
		{
			memcpy (block, lineptr, gr->widthb);
			gr_convert16_ScreenTo565 ((Uint16 *)block, gr->width);
			ARRANGE_WORDS (block, gr->width);
			gzwrite (file, block, gr->widthb);
		}
		else
		{
			gzwrite (file, lineptr, gr->widthb);
		}
	}

	if (gr->depth == 16)
		free(block);

	gzclose (file) ;
	return 1 ;	
}

GRAPH * bitmap_new_syslib (int w, int h, int depth)
{
	GRAPH * gr ;
	
	gr = bitmap_new (0, w, h, depth) ;
	gr->code = get_free_map_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr ;
}

/* Análisis */

void bitmap_analize (GRAPH * bitmap)
{
	int x, y;

	Uint32 pixels = bitmap->pitch * bitmap->height ;

	if (bitmap->modified > 0)
		bitmap->modified = 0 ;
	bitmap->info_flags = 0 ;

	/* Search for transparent pixels (value 0).
	 * If none found, set the flag GI_NOCOLORKEY */

	if (bitmap->depth == 8)
	{
		for (y = 0 ; y < bitmap->height ; y++)
		{
			Uint8 * ptr = memchr ((Uint8 *)bitmap->data + bitmap->pitch*y, 
								  0, bitmap->width) ;
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
	if (pos > bitmap->animation->length) 
		pos = bitmap->animation->length - 1 ;
	if (pos < 0)
		pos = 0 ;

	if (bitmap->depth == 16)
	{
		bitmap->data = (Uint16 *)bitmap->data - bitmap->offset ;
		bitmap->offset = bitmap->pitch * (bitmap->animation->order[pos]-1) ;
		bitmap->data = (Uint16 *)bitmap->data + bitmap->offset ;
	}
	else
	{
		bitmap->data = (Uint8 *)bitmap->data - bitmap->offset ;
		bitmap->offset = bitmap->pitch * (bitmap->animation->order[pos]-1) ;
		bitmap->data = (Uint8 *)bitmap->data + bitmap->offset ;
	}

	bitmap->animation->pos = pos ;
	bitmap->animation->speed = speed ;
	bitmap->animation->remaining = speed ;
}

extern int frame_count ;

void bitmap_animate (GRAPH * bitmap)
{
	if (!(bitmap->flags & F_ANIMATION))
		return ;
        if (!bitmap->animation)
                return ;
	if (bitmap->animation->last_frame == frame_count)
		return ;
	if (!bitmap->animation->speed)
		return ;

	bitmap->animation->last_frame = frame_count ;
	bitmap->animation->remaining -= last_frame_ms ;

	while (bitmap->animation->remaining < 0)
	{
		if (bitmap->animation->speed < 1) 
			bitmap->animation->speed = last_frame_ms ;
		if (bitmap->animation->speed < 10) 
			bitmap->animation->speed = 10 ;

		bitmap->animation->remaining += bitmap->animation->speed ;

		if (bitmap->depth == 16)
		{
			bitmap->data = (Uint16 *)bitmap->data - bitmap->offset ;
			bitmap->offset = bitmap->pitch * 
                                (bitmap->animation->order[bitmap->animation->pos]-1) ;
			bitmap->data = (Uint16 *)bitmap->data + bitmap->offset ;
		}
		else
		{
			bitmap->data = (Uint8 *)bitmap->data - bitmap->offset ;
			bitmap->offset = bitmap->pitch * 
                                (bitmap->animation->order[bitmap->animation->pos]-1) ;
			bitmap->data = (Uint8 *)bitmap->data + bitmap->offset ;
		}

		bitmap->animation->pos++ ;
		if (bitmap->animation->pos == bitmap->animation->length)
			bitmap->animation->pos = 0 ;
	}
}
