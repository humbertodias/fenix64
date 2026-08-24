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
 * FILE        : g_fpg.c
 * DESCRIPTION : Graphic library (FPG) functions
 *
 * HISTORY:      0.82 - First version
 */

#include <string.h>
#include <stdlib.h>

#include <SDL.h>

#ifdef BeIDE
#include <BeOS.h>
#endif

#include "fxi.h"

#define MAXLIBS 128
static GRLIB * libs[MAXLIBS] ;
GRLIB * syslib = 0 ;
static int lib_nextid = 0 ;
int lib_count = 0 ;

/*
 *  FUNCTION : grlib_new
 *
 *  Create a new FPG library
 *
 *  PARAMS : 
 *		None
 *
 *  RETURN VALUE : 
 *      ID of the new library or -1 if error
 */

static GRLIB * grlib_create ();

int grlib_new ()
{
	GRLIB * lib  = grlib_create();
	int i ;

	if (lib_nextid == MAXLIBS && lib_count == lib_nextid) return -1 ;

	lib_count++ ;

	if (lib_nextid == MAXLIBS)
	{
		for (i = 0 ; i < lib_nextid ; i++)
		{
			if (!libs[i]) 
				break ;
		}
		if (i == lib_nextid)
		{
			gr_error ("grlib_new: sin memoria");
			return -1;
		}
		libs[i] = lib ;
		return i ;
	}

	libs[lib_nextid] = lib ;
	return lib_nextid++ ;
}

/* Static convenience function */

static GRLIB * grlib_create ()
{
	GRLIB * lib ;

	lib = (GRLIB *) malloc (sizeof(GRLIB)) ;
	if (!lib) return 0 ;
	lib->maps = (GRAPH **) malloc (16 * sizeof(GRAPH *)) ;
	if (!lib->maps) return 0 ;
	
	lib->map_reserved = 16 ;
	memset (lib->maps, 0, 16 * sizeof(GRAPH *));
	return lib ;
}

/*
 *  FUNCTION : grlib_get
 *
 *  Get a pointer to an FPG in memory given the internal ID
 *  returned by gr_load_fpg
 *
 *  PARAMS : 
 *		libid			Library code 
 *
 *  RETURN VALUE : 
 *      Pointer to the object required or NULL if it does not exist
 */

GRLIB * grlib_get (int libid)
{
	if (libid < 0 || libid >= lib_nextid)
		return 0 ;
	return libs[libid] ;
}

/*
 *  FUNCTION : grlib_destroy
 *
 *  Remove a FPG and all its maps from memory
 *
 *  PARAMS : 
 *		libid			ID of the library
 *
 *  RETURN VALUE : 
 *      0 if there wasn't a map with this code, 1 otherwise
 */

void grlib_destroy (int libid)
{
	int i ;
	GRLIB * lib = grlib_get (libid) ;
	if (lib == 0) return ;

	libs[libid] = 0 ;
	if (lib_nextid == libid+1) lib_nextid-- ;
	lib_count-- ;
	if (!lib_count) lib_nextid = 0 ;

	for (i = 0 ; i < lib->map_reserved ; i++)
		if (lib->maps[i] != NULL)
			bitmap_destroy (lib->maps[i]) ;

	free (lib->maps) ;
	free (lib) ;
}

/*
 *  FUNCTION : grlib_unload_map
 *
 *  Remove a map from a library, if present
 *
 *  PARAMS : 
 *		libid			ID of the library
 *		mapcode			ID of the map
 *
 *  RETURN VALUE : 
 *      0 if there wasn't a map with this code, 1 otherwise
 */

int grlib_unload_map (int libid, int mapcode)
{
	GRLIB * lib;

	if (libid == 0 && mapcode > 999)
		lib = syslib;
	else
		lib = grlib_get(libid);
	if (lib == NULL) return 0;

	if (lib->map_reserved <= mapcode)
		return 0;
	if (lib->maps[mapcode] == 0)
		return 0;

	bitmap_destroy (lib->maps[mapcode]) ;
	lib->maps[mapcode] = 0 ;
	return 1 ;
}

/*
 *  FUNCTION : grlib_add_map
 *
 *  Add a map to a graphic library. If any map with the same code
 *  is in the library, it will be unload first
 *
 *  PARAMS : 
 *		lib				Pointer to the FPG
 *		map				Pointer to the graphic
 *						(fill map->code first!)
 *
 *  RETURN VALUE : 
 *      -1 if error, a number >= 0 otherwise
 */

int grlib_add_map (int libid, GRAPH * map)
{
	GRLIB * lib = grlib_get(libid);
	if (map->code > 999) lib = syslib;
	if (!lib) return -1 ;	
	if (map->code < 0) return -1 ;

	grlib_unload_map (libid, map->code);

	if (lib->map_reserved <= map->code)
	{
		int new_reserved = (map->code & ~0x001F) + 32;
		lib->maps = (GRAPH **) realloc (lib->maps, sizeof(GRAPH*) * new_reserved) ;
		if (!lib->maps) gr_error ("grlib_add_map: sin memoria\n") ;
		memset (lib->maps + lib->map_reserved, 0, 
			(new_reserved - lib->map_reserved) * sizeof(GRAPH *));
		lib->map_reserved = new_reserved;
	}
	lib->maps[map->code] = map ;
	return map->code ;
}

/*
 *  FUNCTION : bitmap_get
 *
 *  Get a bitmap using the DIV syntax (libcode, mapcode)
 *
 *  PARAMS : 
 *		libid			Library code or 0 for system/global bitmap
 *		mapcode			Code of the bitmap
 *
 *  RETURN VALUE : 
 *      Pointer to the graphic required or NULL if not found
 *
 *		(0, -1) gets the scrbitmap graphic (undocumented!)
 *		(0,  0) gets the background
 *
 */

GRAPH * bitmap_get (int libid, int mapcode)
{
	GRLIB * lib ;

	/* Invalid map code used */

	if (libid == 0 || mapcode > 999)
	{
		/* Using (0, 0) we can get the background map */

		if (mapcode == 0)
		{
			background_is_black = 0 ;

			if (enable_16bits && background_8bits_used)
				return background_8bits ;
			return background ;
		}

		/* Using (0, -1) we can get the screen bitmap (undocumented bug/feature) */

		if (mapcode == -1)
		{
			if (!scr_initialized) gr_init(320,200) ;
			if (!scrbitmap) gr_lock_screen() ;
			return scrbitmap ;
		}

		/* Get the map from the system library 
		 * (the only one that can have more than 1000 maps)
		 */

		if (syslib && syslib->map_reserved > mapcode && syslib->maps[mapcode])
			return syslib->maps[mapcode] ;
	}

	/* Get the map from a FPG */

	lib = grlib_get(libid) ;
	if (lib && lib->map_reserved > mapcode)
		return lib->maps[mapcode];
	return 0 ;
}

/*
 *  FUNCTION : gr_load_fpg
 *
 *  Load a FPG file
 *
 *  PARAMS : 
 *		libname			Name of the file
 *
 *  RETURN VALUE : 
 *      Internal ID of the library or -1 if invalid file
 *
 */

static int gr_read_lib (file * fp);

int gr_load_fpg (const char * libname)
{
	int libid ;
	file * fp = file_open (libname, "rb") ;

	if (!fp) 
	{
		gr_error ("Libreria %s no encontrada\n", libname) ;
		return -1 ;
	}
	libid = gr_read_lib (fp) ;
	file_close (fp) ;
	return libid ;
}

/* Static convenience function */
static int gr_read_lib (file * fp)
{
	char header[8] ;
	short int px, py ;
	int bpp, libid, c, len, y ;
	GRLIB * lib ;
	GRAPH * gr ;

	struct 
	{
		int	code ;
		int	regsize ;
		char	name[32] ;
		char	fpname[12] ;
		int	width ;
		int	height ;
		int	flags ;

	} chunk ;

	libid = grlib_new() ;
	if (libid < 0) return 0 ;
	lib = libs[libid] ;
	if (lib == 0) return 0;

	file_read (fp, header, 8) ;
	if (strcmp (header, "f16\x1A\x0D\x0A") == 0) 
		bpp = 16 ;
	else if (strcmp (header, "fpg\x1A\x0D\x0A") == 0) 
		bpp = 8 ;
	else if (strcmp (header, "f01\x1A\x0D\x0A") == 0) 
		bpp = 1 ;
	else
		return 0 ;

	if (bpp > 8 && (!enable_16bits && (GLODWORD(GRAPH_MODE) & 0x01) != 0x01)) {
		gr_error ("Tried to load a 16bpp FPG file in 8bpp mode \n(gr_read_lib)") ;
	}

	if (bpp == 8)
	{
		if (palette_loaded)
			file_seek (fp, 576 + 768, SEEK_CUR) ;
		else if (!gr_read_pal (fp)) 
			return 0 ;
	}

	while (!file_eof(fp))
	{
		if (!file_read (fp, &chunk, 64)) break ;

		ARRANGE_DWORD (&chunk.code);
		ARRANGE_DWORD (&chunk.regsize);
		ARRANGE_DWORD (&chunk.width);
		ARRANGE_DWORD (&chunk.height);
		ARRANGE_DWORD (&chunk.flags);

		/* Cabecera del gráfico */

		gr = bitmap_new (chunk.code, chunk.width, chunk.height, bpp);
		if (!gr) return 0 ;
		memcpy (gr->name, chunk.name, 32) ;
		gr->name[31] = 0 ;
		gr->flags = chunk.flags ;
		gr->modified = 1 ;

		/* Puntos de control */

		if (gr->flags & F_NCPOINTS)
		{
			gr->cpoints = (CPOINT *) malloc 
				((gr->flags & F_NCPOINTS) * sizeof(CPOINT)) ;
			if (!gr->cpoints) { free(gr) ; return 0 ; }
			for (c = 0 ; c < (gr->flags & F_NCPOINTS) ; c++)
			{
				file_readSint16 (fp, &px) ;
				file_readSint16 (fp, &py) ;
				gr->cpoints[c].x = px ;
				gr->cpoints[c].y = py ;
			}
		}
		else	gr->cpoints = 0 ;

		/* Animación */

		len = gr->widthb;

		if (gr->flags & F_ANIMATION)
		{
			if (!gr->animation) gr->animation = malloc(sizeof(ANIMATION)) ;
			file_readSint16 (fp, &gr->animation->frames) ;
			file_readSint16 (fp, &gr->animation->length) ;
			file_readSint16 (fp, &gr->animation->speed) ;
			gr->animation->order = malloc(gr->animation->length * 2) ;
			file_read (fp, gr->animation->order, gr->animation->length * 2) ;
			ARRANGE_WORDS (gr->animation->order, gr->animation->length);
			gr->pitch *= gr->animation->frames ;
			len *= gr->animation->frames;
			gr->data = realloc(gr->data, len) ;
		}

		/* Datos del gráfico */

		for (y = 0 ; y < gr->height ; y++)
		{
			Uint8 * ptr = (Uint8 *)gr->data + gr->pitch*y;
			if (!file_read (fp, ptr, len))
			{
				free (gr) ;
				break ;
			}
			if (bpp == 16) 
			{
				ARRANGE_WORDS (ptr, len/2);
				gr_convert16_565ToScreen ((Uint16 *)ptr, len/2);
			}
		}


		grlib_add_map (libid, gr) ;
	}
	return libid ;
}

/*
 *  FUNCTION : grlib_init
 *
 *  Create the system library. This should be called at program startup.
 *
 *  PARAMS : 
 *		None
 *
 *  RETURN VALUE : 
 *      None
 *
 */

void grlib_init()
{
	if (!syslib)
	{
		syslib = grlib_create() ;
	}
}

/*
 *  FUNCTION : gr_save_fpg
 *
 *  Save a FPG file
 *
 *  PARAMS : 
 *		filename		Name of the file
 *
 *  RETURN VALUE : 
 *      1 if succeded or 0 otherwise
 *
 */

int gr_save_fpg (int libid, const char * filename)
{
	file  * fp;
	GRLIB * lib;
	Uint8 * block;
	int     n, y, c;
	int     bpp;

	struct 
	{
		int		code ;
		int		regsize ;
		char	name[32] ;
		char	fname[12] ;
		int		width ;
		int		height ;
		int		flags ;
		
	} chunk ;
	
	/* Get the library and open the file */
	
	lib = grlib_get (libid);
	if (!lib) return 0;
	
	fp = file_open (filename, "wb");
	if (!fp) return 0;
	
	/* Guess the FPG depth */
	
	for (bpp = n = 0 ; n < lib->map_reserved ; n++)
	{
		if (lib->maps[n])
		{
			if (bpp && lib->maps[n]->depth != bpp)
			{
				gr_error ("gr_save_map: no puede guardarse una librería con mapas de distinto tipo");
				file_close(fp);
				return 0;
			}
			bpp = lib->maps[n]->depth;
		}
	}
	
	if (bpp == 0)
	{
		gr_error ("gr_save_fpg: intento de guardar un FPG vacío");
		file_close (fp);
		return 0;
	}
	
	/* Write the header */
	
	if (bpp == 16)
		file_write (fp, "f16\x1A\x0D\x0A\x00", 8);
	else if (bpp == 8)
		file_write (fp, "fpg\x1A\x0D\x0A\x00", 8);
	else if (bpp == 1)
		file_write (fp, "f01\x1A\x0D\x0A\x00", 8);
	else
	{
		gr_error ("gr_save_fpg: profundidad de bit no soportada");
		file_close (fp);
		return 0;
	}
	
	/* Write the color palette */
	
	if (bpp == 8) 
	{
		Uint8   colors[256][3] ;
		
		block = calloc(576,1) ;
		if (block == NULL)
		{
			gr_error ("gr_save_fpg: sin memoria");
			file_close (fp);
			return 0;
		}
		for (n = 0 ; n < 256 ; n++) 
		{
			colors[n][0] = palette[n].r >> 2 ;
			colors[n][1] = palette[n].g >> 2 ;
			colors[n][2] = palette[n].b >> 2 ;
		}
		file_write (fp, &colors, 768) ;		
		file_write (fp, block, 576) ;		
		free(block) ;
	}
	
	/* Write each map */
	
	for (n = 0 ; n < lib->map_reserved ; n++)
	{
		GRAPH * gr = lib->maps[n];
		
		if (gr == NULL)
			continue;
		
		/* Write the bitmap header */
		
		chunk.code = n;
		chunk.regsize = 0;
		chunk.width = gr->width;
		chunk.height = gr->height;
		chunk.flags = gr->flags;
		chunk.fname[0] = 0;
		strncpy (chunk.name,  gr->name,  sizeof(chunk.name));
		
		ARRANGE_DWORD (&chunk.code);
		ARRANGE_DWORD (&chunk.regsize);
		ARRANGE_DWORD (&chunk.width);
		ARRANGE_DWORD (&chunk.height);
		ARRANGE_DWORD (&chunk.flags);
		
		file_write (fp, &chunk, 64);
		
		/* Write the control points */
		
		if (gr->flags & F_NCPOINTS)
		{
			for (c = 0 ; c < (gr->flags & F_NCPOINTS) ; c++)
			{
				file_writeSint16 (fp, &gr->cpoints[c].x);
				file_writeSint16 (fp, &gr->cpoints[c].y);
			}
		}
		
		/* Write the animation data */
		
		if (gr->flags & F_ANIMATION)
		{
			file_writeSint16 (fp, &gr->animation->frames) ;
			file_writeSint16 (fp, &gr->animation->length) ;
			file_writeSint16 (fp, &gr->animation->speed) ;
			ARRANGE_WORDS (gr->animation->order, gr->animation->length);
			file_write (fp, gr->animation->order, gr->animation->length * 2) ;
			ARRANGE_WORDS (gr->animation->order, gr->animation->length);
		}
		
		/* Write the bitmap pixel data */
		
		if (gr->depth == 16)
		{
			if ((block = malloc(gr->widthb)) == NULL)
			{
				gr_error ("gr_save_fpg: sin memoria");
				file_close (fp);
				return 0;
			}
		}
		
		for (y = 0 ; y < gr->height ; y++)
		{
			if (bpp == 16)
			{
				memcpy (block, (Uint8 *)gr->data + gr->pitch*y, gr->widthb);
				gr_convert16_ScreenTo565 ((Uint16 *)block, gr->width);
				ARRANGE_WORDS (block, gr->width);
				file_write (fp, block, gr->widthb);
			}
			else
			{
				file_write (fp, (Uint8 *)gr->data + gr->pitch*y, gr->widthb);
			}
		}
		
		if (bpp == 16) free(block);
	}
	
	file_close(fp);
	return 1;
}
