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
 * FILE        : fgc.c
 * DESCRIPTION : Fenix Graphic Collection handling functions
 *
 * HISTORY:      0.85 - first version
 */

#include <string.h>
#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include "fxi.h"
#include "fbm.h"
#include "fgc.h"

const char * fgc_error = "";

/*
 *  FUNCTION : fgc_load
 *
 *  Loads an FGC file, given its name.
 *
 *  PARAMS :
 *		filename		Name of the file
 *
 *  RETURN VALUE :
 *      Number (identifier) of the new library
 *		-1 if error (fgc_error will have a description in this case)
 *
 */

int fgc_load (const char * filename)
{
	file *		fp;
	int			id;
	GRLIB *		lib;
	GRAPH *     graph;
	FGC_HEADER	header;
	Uint32 *	offsets;
	Uint32		i;
	PALETTE *   pal = NULL;

	// Open the file and check the header and version

	fp = file_open(filename, "rb");

	if (!fp)
	{
		fgc_error = "Error al abrir el fichero";
		return -1;
	}

	if (file_read (fp, &header, sizeof(FGC_HEADER)) != sizeof(FGC_HEADER))
	{
		fgc_error = "Fichero FGC corrupto o truncado";
		file_close(fp);
		return -1;
	}

	ARRANGE_DWORD(&header.version);
	ARRANGE_DWORD(&header.depth);
	ARRANGE_DWORD(&header.count);
	ARRANGE_DWORD(&header.palette);

	if (strcmp (header.magic, FGC_MAGIC) != 0 || (header.version & 0xFF00) != 0x0100)
	{
		fgc_error = "No es un fichero FGC o compatible";
		file_close(fp);
		return -1;
	}

	// Create the new graphic library

	id = grlib_new();
	lib = grlib_get(id);

	offsets = (Uint32 *)malloc(4*header.count);

	if (offsets == NULL)
	{
		fgc_error = "No hay memoria libre suficiente";
		file_close(fp);
		return -1;
	}

	if (file_read (fp, offsets, 4*header.count) != 4*(int)header.count)
	{
		fgc_error = "Fichero FGC corrupto o truncado";
		free(offsets);
		file_close(fp);
		return -1;
	}

	ARRANGE_DWORDS(offsets, header.count);

	// Load the graphic palette

	if (header.depth == 8)
	{
		static char color_palette[768];

		file_seek(fp, header.palette, SEEK_SET);

		if (file_read(fp, color_palette, 768) != 768)
		{
			fgc_error = "PALETA - Fichero FGC truncado";
			free(offsets);
			file_close(fp);
			return -1;
		}

        pal = pal_new2(color_palette);

		if (!palette_loaded)
		{
			for (i = 0 ; i < 256 ; i++)
			{
				palette[i].r = color_palette[3*i + 0];
				palette[i].g = color_palette[3*i + 1];
				palette[i].b = color_palette[3*i + 2];
			}
			palette_loaded = 1 ;
			palette_changed = 1 ;
		}
	}

	// Load the individual graphics

	for (i = 0 ; i < header.count ; i++)
	{
		file_seek(fp, offsets[i], SEEK_SET);

		graph = fbm_load_from(fp, header.depth);
		if (graph == NULL)
		{
			fgc_error = fbm_error;
			break;
		}
		grlib_add_map (id, graph);
		if (graph->depth == 8 && !graph->palette) pal_map_assign(id, graph->code, pal);
	}

	free(offsets);
	file_close(fp);

    pal_destroy(pal); // Elimino la instancia inicial

	return (i == header.count ? id : -1);
}

/*
 *  FUNCTION : fgc_save
 *
 *  Save a FGC file, given its name.
 *
 *  PARAMS :
 *		id				ID of the library (must be > 0)
 *		filename		Name of the file
 *
 *  RETURN VALUE :
 *		1 if error, 0 otherwise (fgc_error will have a description in this case)
 *
 */

int fgc_save (int id, const char * filename)
{
	GRLIB *     lib;
	FGC_HEADER	header;
	file *		fp;
	int			i, n;
	Uint32 *    offsets;
	int         palette_saved = 0;

	// Check the parameters

	lib = grlib_get(id);
	if (lib == NULL || !lib->maps)
	{
		fgc_error = "Número de librería incorrecta";
		return 0;
	}

	// Check the library and prepare the header data

	strcpy (header.magic, FGC_MAGIC);
	strncpy (header.name, lib->name, 64);
	header.version = 0x0100;
	header.depth = 0;
	header.count = 0;

	for (i = 0 ; i < lib->map_reserved ; i++)
	{
		if (lib->maps[i] == NULL) continue;

		// Maps with code > 999 are not inside a collection
		if (lib->maps[i]->code > 999) continue;

		header.count++;
		if (header.depth == 0)
		{
			header.depth = lib->maps[i]->depth;
			continue;
		}
		if (header.depth != lib->maps[i]->depth)
		{
			fgc_error = "La librería contiene mapas de diferente profundidad";
			return 0;
		}
	}

	// Corrected palette_offset information

	header.palette = sizeof(FGC_HEADER) + 4 * header.count ;

	// Create the file

	fp = file_open (filename, "wb9");
	if (fp == NULL)
	{
		fgc_error = "Error al crear fichero FGC";
		return 0;
	}

	offsets = (Uint32 *)malloc(4*header.count + 4);
	if (offsets == NULL)
	{
		fgc_error = "No hay memoria libre suficiente";
		file_close(fp);
		return -1;
	}

	// Calculate the offsets

	for (i = n = 0 ; i < lib->map_reserved ; i++)
	{
		if (lib->maps[i] == NULL) continue;
		// Maps with code > 999 are not inside a collection
		if (lib->maps[i]->code > 999) continue;

		if (n == 0)
		{
			offsets[n] = sizeof(FGC_HEADER) + 4*header.count;
			if (header.depth == 8) offsets[n] += 768;
		}

		offsets[n+1] = offsets[n] + fbm_size(lib->maps[i], 0, 0);

		n++;
	}

	// Write the FGC file header

	ARRANGE_DWORDS(offsets, header.count);
	ARRANGE_DWORD(&header.depth);
	ARRANGE_DWORD(&header.version);
	ARRANGE_DWORD(&header.count);
	ARRANGE_DWORD(&header.palette);

	if (file_write(fp, &header, sizeof(FGC_HEADER)) != sizeof(FGC_HEADER))
	{
		fgc_error = "Error escribiendo en fichero FGC";
		file_close (fp);
		return 0;
	}

	ARRANGE_DWORD(&header.count);
	ARRANGE_DWORD(&header.depth);

	if (file_write(fp, offsets, 4*header.count) != 4*(int)header.count)
	{
		fgc_error = "Error escribiendo en fichero FGC";
		file_close (fp);
		return 0;
	}

	// Write each graphic

	for (i = n = 0 ; i < lib->map_reserved ; i++)
	{
		if (lib->maps[i] == NULL) continue;
		// Maps with code > 999 are not inside a collection
		if (lib->maps[i]->code > 999) continue;

    	// Write the graphic palette

    	if (!palette_saved && header.depth == 8)
    	{
    		static char color_palette[768];
		    SDL_Color * gpal = palette ;

            if (lib->maps[i]->palette) gpal = lib->maps[i]->palette->rgb; else gpal = palette;

    		for (i = 0 ; i < 256 ; i++)
    		{
    			color_palette[3*i    ] = gpal[i].r;
    			color_palette[3*i + 1] = gpal[i].g;
    			color_palette[3*i + 2] = gpal[i].b;
    		}
    		if (file_write(fp, color_palette, 768) != 768)
    		{
    			fgc_error = "Error al escribir, FGC truncado";
    			free(offsets);
    			file_close(fp);
    			return -1;
    		}
    		palette_saved = 1;
    	}

		assert (file_pos(fp) == (int)offsets[n]);

		if (!fbm_save_to (lib->maps[i], fp, 0))
		{
			fgc_error = fbm_error;
			free(offsets);
			file_close(fp);
			return 0;
		}
		n++;
	}

	free(offsets);
	file_close(fp);
	return 1;
}
