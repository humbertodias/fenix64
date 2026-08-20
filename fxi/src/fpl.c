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
#include "fpl.h"

const char * fpl_error = "";

/*
 *  FUNCTION : fpl_load_from
 *
 *  Load a FPL file from an already opened file
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		fgc_depth		Color depth if the graphic is part of a FGC file or 0 if it isn't
 *						Part of the header are supposed to not be there if this
 *						parameter is greater than 0 (see fbm.h for details)
 *
 *  RETURN VALUE :
 *		0 if error (fpl_error will have a description in this case)
 *		1 on SUCCESS
 *
 */

int fpl_load_from (file * fp)
{
	FPL_HEADER			 header;
	static unsigned char color_palette[768];
	int n ;

	// Read the header and check the file type and version

	if (file_read (fp, &header, sizeof(FPL_HEADER)) != sizeof(FPL_HEADER)) {
		fpl_error = "Error de lectura" ;
		return 0 ;
	}

    if (strncmp(header.magic, FPL_MAGIC, strlen(FPL_MAGIC)) == 0) {
		fpl_error = "Fichero FPL inválido" ;
		return 0 ;
    }

	ARRANGE_DWORD(&file_header.depth);
	ARRANGE_DWORD(&file_header.version);

	if (!FPL_VALID_DEPTH(header.depth)) {
		fpl_error = "Profundidad de color no válida en el fichero FPL" ;
		return 0 ;
	}

	if ((header.version & 0xFF00) != 0x0100) {
		fpl_error = "El fichero FPL es de una versión posterior incompatible";
		return 0;
	}

	if (file_read(fp, color_palette, 768) != 768) {
		fpl_error = "Fichero FPL truncado";
		return 0;
	}

	for (n = 0 ; n < 256 ; n++) {
		palette[n].r = color_palette[3*n    ];
		palette[n].g = color_palette[3*n + 1];
		palette[n].b = color_palette[3*n + 2];
	}

	palette_loaded = 1 ;
	palette_changed = 1 ;

	return 1 ;

}


/*
 *  FUNCTION : fpl_save_to
 *
 *  Save palette data
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *
 *  RETURN VALUE :
 *      1 if no error, 0 otherwise (fpl_error will have a description in this case)
 *
 */

int fpl_save_to (file * fp)
{
	static char			color_palette[768];
	int n ;

	// Write the color palette

	for (n = 0 ; n < 256 ; n++)
	{
		color_palette[3*n    ] = palette[n].r;
		color_palette[3*n + 1] = palette[n].g;
		color_palette[3*n + 2] = palette[n].b;
	}

	if (file_write (fp, &color_palette, 768) != 768)
	{
		fbm_error = "Error escribiendo fichero FPL";
		return 0;
	}

	return 1;
}

/*
 *  FUNCTION : fpl_save
 *
 *  Save a palette to a new FPL file
 *
 *  PARAMS :
 *		filename		Name of the file to be created
 *
 *  RETURN VALUE :
 *      1 if no error, 0 otherwise (fpl_error will have a description in this case)
 *
 */

int fpl_save (const char * filename)
{
	FPL_HEADER	file_header;
	file *		fp;
	int			return_value;

	// Create the file

	fp = file_open (filename, "wb9");
	if (fp == NULL)
	{
		fpl_error = "Error al crear fichero FPL";
		return 0;
	}

	// Write the FPL file header

	strcpy (file_header.magic, FPL_MAGIC);
	file_header.depth = 8;
	file_header.version = 0x0100;

	ARRANGE_DWORD(&file_header.depth);
	ARRANGE_DWORD(&file_header.version);

	if (file_write (fp, &file_header, sizeof(FPL_HEADER)) != sizeof(FPL_HEADER))
	{
		fpl_error = "Error escribiendo en fichero FPL";
		file_close (fp);
		return 0;
	}

	// Use fbm_save_to to save the rest of the data

	return_value = fpl_save_to(fp);
	file_close(fp);
	return return_value;
}

/*
 *  FUNCTION : fpl_load
 *
 *  Load a palette from a FPL file
 *
 *  PARAMS :
 *		filename		Name of the file to read
 *
 *  RETURN VALUE :
 *      1 if no error, 0 otherwise (fpl_error will have a description in this case)
 *
 */

int fpl_load (const char * filename)
{
	file *		fp;
	int			return_value;

	// Open the file

	fp = file_open (filename, "rb9");
	if (fp == NULL)
	{
		fpl_error = "Error al abrir fichero FPL";
		return 0;
	}

	// Use fbm_load_from to load the rest of the data

	return_value = fpl_load_from(fp);
	file_close(fp);
	return return_value;
}

