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
 * FILE        : fbm.c
 * DESCRIPTION : Fenix Bitmap handling functions
 *
 * HISTORY:      0.85 - first version
 */

#include <string.h>
#include <stdlib.h>

#include "fxi.h"
#include "fbm.h"

const char * fbm_error = "";

/*
 *  FUNCTION : arrange_sequences
 *             arrange_keyframes
 *
 *  In non-Intel platforms, correctly arranges all the contents
 *	of the graphic. If should be called just before writing to
 *	the file and once again thereafter to leave it as it was before
 *
 *  PARAMS :
 *		graph			Pointer to the graphic
 *
 *  RETURN VALUE :
 *      None
 *
 */

static void arrange_sequences (GRAPH * graph)
{
	Uint32 n;

	for (n = 0 ; n <= graph->max_sequence ; n++)
	{
		ARRANGE_DWORD(&graph->sequences[n].first_keyframe);
		ARRANGE_DWORD(&graph->sequences[n].last_keyframe);
		ARRANGE_DWORD(&graph->sequences[n].next_sequence);
	}
}

static void arrange_keyframes (GRAPH * graph)
{
	Uint32 n;

	for (n = 0 ; n <= graph->max_keyframe ; n++)
	{
		ARRANGE_DWORD(&graph->keyframes[n].frame);
		ARRANGE_DWORD(&graph->keyframes[n].angle);
		ARRANGE_DWORD(&graph->keyframes[n].flags);
		ARRANGE_DWORD(&graph->keyframes[n].pause);
	}
}








int fbm_insert_frame(GRAPH * gr, int frame, int where)
{
	(void)gr;
	(void)frame;
	(void)where;
	return 0;
}

int fbm_remove_frame(GRAPH * gr, int frame)
{
	(void)gr;
	(void)frame;
	return 0;
}

int fbm_insert_keyframe(GRAPH * gr, int keyframe, int where)
{
	(void)gr;
	(void)keyframe;
	(void)where;
	return 0;
}

int fbm_remove_keyframe(GRAPH * gr, int keyframe)
{
	(void)gr;
	(void)keyframe;
	return 0;
}

int fbm_insert_sequence(GRAPH * gr, int seq, int where)
{
	(void)gr;
	(void)seq;
	(void)where;
	return 0;
}

int fbm_remove_sequence(GRAPH * gr, int seq)
{
	(void)gr;
	(void)seq;
	return 0;
}










/*
 *  FUNCTION : fbm_load_from
 *
 *  Load a FBM file from an already opened file
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		fgc_depth		Color depth if the graphic is part of a FGC file or 0 if it isn't
 *						Part of the header are supposed to not be there if this
 *						parameter is greater than 0 (see fbm.h for details)
 *
 *  RETURN VALUE :
 *      Pointer to the newly allocated GRAPH
 *		NULL if error (fbm_error will have a description in this case)
 *
 */

GRAPH * fbm_load_from (file * fp, int fgc_depth)
{
	FBM_FILE_HEADER		file_header;
	FBM_HEADER			header;
	FBM_SEQUENCE *		sequences;
	FBM_KEYFRAME *		keyframes;
	FBM_CONTROL_POINT	control_point;

	GRAPH *				graph = NULL;
	int					error = 0;
	int					size;
	int					section;
	int					n, h;
	static char			color_palette[768];

	// Read the header and check the file type and version

	if (fgc_depth == 0)
	{
		if (file_read (fp, &file_header, sizeof(FBM_FILE_HEADER)) != sizeof(FBM_FILE_HEADER))
		{
			fbm_error = "Error de lectura";
			return NULL;
		}

		ARRANGE_DWORD (&file_header.version);
		ARRANGE_DWORD (&file_header.depth);

		if (strcmp (file_header.magic, FBM_MAGIC) != 0)
		{
			fbm_error = "El fichero no es un FBM válido";
			return NULL;
		}
		if ((file_header.version & 0xFF00) != 0x0100)
		{
			fbm_error = "El fichero FBM es de una versión posterior incompatible";
			return NULL;
		}
	}
	else
	{
		file_header.depth = fgc_depth;
	}

	// Read the graphic header

	if (file_read (fp, &header, sizeof(FBM_HEADER)) != sizeof(FBM_HEADER))
	{
		fbm_error = "Error de lectura";
		return NULL;
	}

	ARRANGE_DWORD (&header.width);
	ARRANGE_DWORD (&header.height);
	ARRANGE_DWORD (&header.flags);
	ARRANGE_DWORD (&header.code);
	ARRANGE_DWORD (&header.max_frame);
	ARRANGE_DWORD (&header.max_sequence);
	ARRANGE_DWORD (&header.max_keyframe);
	ARRANGE_DWORD (&header.max_point);
	ARRANGE_DWORD (&header.points);

	// Do some sanity checks on the file

	if (!FBM_VALID_DEPTH(file_header.depth))
	{
		fbm_error = "Fichero FBM corrupto";
		return NULL;
	}
/*
	if (header.width > FBM_MAX_WIDTH || header.height > FBM_MAX_HEIGHT)
	{
		fbm_error = "Fichero FBM corrupto";
		return NULL;
	}
*/
	// Allocate space in memory for the data

	sequences = (FBM_SEQUENCE *) malloc(sizeof(FBM_SEQUENCE) * (header.max_sequence+1));
	keyframes = (FBM_KEYFRAME *) malloc(sizeof(FBM_KEYFRAME) * (header.max_keyframe+1));

	if (sequences == NULL || keyframes == NULL)
	{
		fbm_error = "No hay memoria libre suficiente para cargar el FBM";
		error = 1;
	}

	// Create the graphic and fill its information in memory

	if (error == 0)
	{
		graph = bitmap_new (header.code, header.width, header.height, file_header.depth, header.max_frame+1);
		if (graph == NULL)
		{
			fbm_error = "Error al crear el gráfico en memoria";
			error = 1;
		}
		else
		{
			// Use our data instead of the default one
			free (graph->sequences);
			free (graph->keyframes);
			graph->sequences = sequences;
			graph->keyframes = keyframes;

			graph->max_sequence = header.max_sequence;
			graph->max_keyframe = header.max_keyframe;

			strncpy (graph->name, header.name, 64);
			graph->info_flags = header.flags;
		}
	}

	// Load each section sequentialy
	for (section = 0 ; error == 0 && section < 5 ; section++)
	{
		switch (section)
		{
			case 0:				// Read the palette

				if (file_header.depth == 8 && !fgc_depth)
				{
					if (file_read(fp, color_palette, 768) != 768)
					{
						fbm_error = "Fichero FBM truncado";
						error = 1;
						break;
					}
					if (!palette_loaded) {
						for (n = 0 ; n < 256 ; n++)
						{
							palette[n].r = color_palette[3*n + 0];
							palette[n].g = color_palette[3*n + 1];
							palette[n].b = color_palette[3*n + 2];
						}
                    }

                    graph->palette = pal_new2(color_palette);

                    palette_loaded = 1 ;
					palette_changed = 1 ;
				}
				break;

			case 1:				// Read the sequences

				size = sizeof(FBM_SEQUENCE) * (header.max_sequence+1);
				if (file_read(fp, sequences, size) != size)
				{
					fbm_error = "Fichero FBM truncado";
					error = 1;
				}
				else
				{
					arrange_sequences(graph);
				}
				break;

			case 2:				// Read the keyframes

				size = sizeof(FBM_KEYFRAME) * (header.max_keyframe+1);
				if (file_read(fp, keyframes, size) != size)
				{
					fbm_error = "Fichero FBM truncado";
					error = 1;
				}
				else
				{
					arrange_keyframes(graph);
				}
				break;

			case 3:				// Read the control points

				while (error == 0 && header.points-- > 0)
				{
                    if (file_read(fp, &control_point, size) != size)
					{
						fbm_error = "Fichero FBM truncado";
						error = 1;
					}
					else
					{
						ARRANGE_DWORD(&control_point.index);
						ARRANGE_DWORD(&control_point.x);
						ARRANGE_DWORD(&control_point.y);

						bitmap_set_cpoint (graph, control_point.index, control_point.x, control_point.y);
					}
				}
				break;

			case 4:				// Read the graphic data

            	if (graph->depth == 1)
            		size = (graph->width + 7)/8 ;
            	else
            		size = graph->width * graph->depth / 8;

                h = graph->height * graph->frames;

                for (n = 0; n < h; n++) {
    				if (file_read(fp, ((char *) graph->data_start) + n * graph->pitch, size) != size)
    				{
    					fbm_error = "Fichero FBM truncado";
    					error = 1;
    				}

                	if (graph->depth == 16)
                	{
						ARRANGE_WORDS (graph->data_start, size/2);
						if (scr_initialized) gr_convert16_565ToScreen ((Uint16*)graph->data_start, size/2);
            	    }
                }
		}
	}

	// Free the allocated memory and return

	if (error)
	{
		if (graph) {
		    bitmap_destroy(graph);
		}
		else
		{
        	if (sequences) free(sequences);
        	if (keyframes) free(keyframes);
        }

		return NULL;
	}

	return graph;
}

/*
 *  FUNCTION : fbm_load
 *
 *  Loads an FBM file, given its name. It is a shortcut for fbm_load_from
 *
 *  PARAMS :
 *		filename		Name of the file
 *
 *  RETURN VALUE :
 *      Pointer to the newly allocated GRAPH
 *		NULL if error (fbm_error will have a description in this case)
 *
 */

GRAPH * fbm_load (const char * filename)
{
	file *  fp = file_open(filename, "rb");
	GRAPH * result;

	if (!fp)
	{
		fbm_error = "Error al abrir fichero";
		return NULL;
	}

	result = fbm_load_from (fp, 0);
	file_close(fp);
	return result;
}

/*
 *  FUNCTION : fbm_save_to
 *
 *  Save all the data of a graphic to an already open FBM file
 *  This function is used by both fbm_save and fgc_save
 *
 *  PARAMS :
 *		map				Pointer to the graphic object
 *		fp				Pointer to the file object
 *		with_palette	1 to save the palette (FBM files), 0 inside FGC files
 *
 *  RETURN VALUE :
 *      1 if no error, 0 otherwise (fbm_error will have a description in this case)
 *
 */

int fbm_save_to (GRAPH * map, file * fp, int with_palette)
{
	FBM_HEADER			header;
	Sint32				size;
	Uint32				n;
	int                 h;
	static char			color_palette[768];
    char                * data_copy = NULL;
    char                * data = NULL;
    SDL_Color           * gpal = NULL;

	// Prepare the header

	header.code			= map->code;
	header.flags		= map->info_flags;
	header.height		= map->height;
	header.max_frame    = map->frames - 1;
	header.max_keyframe = map->max_keyframe;
	header.max_sequence = map->max_sequence;
	header.width		= map->width;

	strncpy (header.name, map->name, sizeof(header.name));

	// Calculate the biggest control point index
	// and number of control points in the graphic

	header.max_point = 0;
	header.points	 = 0;

	for (n = 0 ; n < map->ncpoints ; n++)
	{
		if (map->cpoints[n].x != CPOINT_UNDEFINED)
		{
			header.max_point = n;
			header.points++;
		}
	}

	ARRANGE_DWORD (&header.width);
	ARRANGE_DWORD (&header.height);
	ARRANGE_DWORD (&header.flags);
	ARRANGE_DWORD (&header.code);
	ARRANGE_DWORD (&header.max_frame);
	ARRANGE_DWORD (&header.max_sequence);
	ARRANGE_DWORD (&header.max_keyframe);
	ARRANGE_DWORD (&header.max_point);
	ARRANGE_DWORD (&header.points);

	// Write the header

	if (file_write (fp, &header, sizeof(FBM_HEADER)) != sizeof(FBM_HEADER))
	{
		fbm_error = "Error escribiendo fichero FBM";
		return 0;
	}

	// Write the color palette

	if (map->depth == 8 && with_palette)
	{
        if (map->palette) gpal = map->palette->rgb; else gpal = palette;

		for (n = 0 ; n < 256 ; n++)
		{
			color_palette[3*n    ] = gpal[n].r;
			color_palette[3*n + 1] = gpal[n].g;
			color_palette[3*n + 2] = gpal[n].b;
		}

		if (file_write (fp, &color_palette, 768) != 768)
		{
			fbm_error = "Error escribiendo fichero FBM";
			return 0;
		}
	}

	// Write the sequences

	arrange_sequences(map);
	size = sizeof(FBM_SEQUENCE) * (header.max_sequence+1);
	if (file_write (fp, map->sequences, size) != size)
	{
		arrange_sequences(map);
		fbm_error = "Error escribiendo fichero FBM";
		return 0;
	}
	arrange_sequences(map);

	// Write the keyframes

	arrange_keyframes(map);
	size = sizeof(FBM_KEYFRAME) * (header.max_keyframe+1);
	if (file_write (fp, map->keyframes, size) != size)
	{
		arrange_keyframes(map);
		fbm_error = "Error escribiendo fichero FBM";
		return 0;
	}
	arrange_keyframes(map);

	// Write the control points

	for (n = 0 ; n < map->ncpoints ; n++)
	{
		if (map->cpoints[n].x != CPOINT_UNDEFINED)
		{
			FBM_CONTROL_POINT cp;

			cp.index = n;
			cp.x     = map->cpoints[n].x;
			cp.y     = map->cpoints[n].y;

			ARRANGE_DWORD(&cp.index);
			ARRANGE_DWORD(&cp.x);
			ARRANGE_DWORD(&cp.y);

			if (file_write (fp, &cp, sizeof(cp)) != sizeof(cp))
			{
				fbm_error = "Error escribiendo fichero FBM";
				return 0;
			}
		}
	}

	// Write the graphic data

	if (map->depth == 1)
		size = (map->width + 7)/8 ;
	else
		size = map->width * map->depth / 8;

	if (map->depth == 16)
	{
		data_copy = (Uint16 *)malloc(size);

		if (data_copy == NULL)
		{
			fbm_error = "No hay memoria libre suficiente";
			return 0;
		}
	}

    h = map->height * map->frames;

    for (n = 0; n < h; n++) {
        data = ((char *)map->data_start) + n * map->pitch;

    	if (map->depth == 16)
    	{
    		memcpy (data_copy, data, size);
    		if (scr_initialized) gr_convert16_ScreenTo565(data_copy, size/2);
			ARRANGE_WORDS (data_copy, size/2);
    		data = data_copy ;
	    }

    	if (file_write(fp, data, size) != size)
	    {
    		fbm_error = "Error escribiendo fichero FBM";
		    free (data_copy);
		    return 0;
	    }
    }

    if (data_copy) free(data_copy);

	return 1;
}

/*
 *  FUNCTION : fbm_save
 *
 *  Save a graphic to a new FBM file
 *
 *  PARAMS :
 *		map				Pointer to the graphic object
 *		filename		Name of the file to be created
 *
 *  RETURN VALUE :
 *      1 if no error, 0 otherwise (fbm_error will have a description in this case)
 *
 */

int fbm_save (GRAPH * map, const char * filename)
{
	FBM_FILE_HEADER		file_header;
	file *				fp;
	int					return_value;

	// Create the file

	fp = file_open (filename, "wb9");
	if (fp == NULL)
	{
		fbm_error = "Error al crear fichero FBM";
		return 0;
	}

	// Write the FBM file header

	strcpy (file_header.magic, FBM_MAGIC);
	file_header.depth = map->depth;
	file_header.version = 0x0100;

	ARRANGE_DWORD(&file_header.depth);
	ARRANGE_DWORD(&file_header.version);

	if (file_write (fp, &file_header, sizeof(FBM_FILE_HEADER)) != sizeof(FBM_FILE_HEADER))
	{
		fbm_error = "Error escribiendo en fichero FBM";
		file_close (fp);
		return 0;
	}

	// Use fbm_save_to to save the rest of the graphic

	return_value = fbm_save_to(map, fp, 1);
	file_close(fp);
	return return_value;
}

/*
 *  FUNCTION : fbm_size
 *
 *  Calculate the size of a FBM file (optionally without
 *  header nor color palette) for the purposes of saving the
 *  graphic to a FGC collection
 *
 *  PARAMS :
 *		map				Pointer to the graphic object
 *		with_header		1 to count the FBM header, 0 inside FGC files
 *		with_palette	1 to count the color palette, 0 inside FGC files
 *
 *  RETURN VALUE :
 *      Size of the resulting file
 *
 */

Uint32 fbm_size (GRAPH * graph, int with_header, int with_palette)
{
	Uint32 result = 0;
	Uint32 size;
	Uint32 n;

	// Count the headers

	if (with_header)
		result += sizeof(FBM_FILE_HEADER);
	result += sizeof(FBM_HEADER);

	// Count the information

	result += sizeof(FBM_SEQUENCE) * (graph->max_sequence+1);
	result += sizeof(FBM_KEYFRAME) * (graph->max_keyframe+1);

	for (n = 0 ; n < graph->ncpoints ; n++)
		if (graph->cpoints[n].x != CPOINT_UNDEFINED)
			result += sizeof(FBM_CONTROL_POINT);

	if (graph->depth == 8 && with_palette)
		result += 768;

	// Count the data

	if (graph->depth == 1)
		size = (graph->width + 7)/8 * graph->height;
	else
		size = graph->width * graph->height * graph->depth / 8;

	// Return the result

	return result + size;
}

