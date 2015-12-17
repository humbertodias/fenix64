/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.75
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
 * FILE        : map.c
 * DESCRIPTION : MAP Utility
 *
 * HISTORY:
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <zlib.h>
#include <png.h>
#include <gif_lib.h>

typedef unsigned int	Uint32 ;
typedef signed   int	Sint32 ;
typedef unsigned short	Uint16 ;
typedef signed   short	Sint16 ;
typedef unsigned char	Uint8  ;
typedef signed   char	Sint8  ;

#define MAXPATH  1024
#define MAXFILES  256
#define MAXPARAMS  64

char files[MAXFILES][MAXPATH] ;
int  n_files = 0 ;
char * policy = "wb" ;

#define AC_LIST    1
#define AC_PALETTE 2
#define AC_16BITS  3
#define AC_TO_PNG  4
#define AC_TO_MAP  5
#define AC_NOTRANS 6
#define AC_UPDATE  7

int action = AC_LIST ;

int n_commands ;
char * commands[256] ;

/* "Traducción" de un color determinado a 0, en PNGs de 16 bits */
int do_zeroc = 0 ;
Uint16 zeroc[2][3] = { { 0, 0, 0 }, { 0, 0, 0 } } ;

void fatal_error (char * fmt, ...)
{
	va_list ap ;
	fflush (stdout) ;
	va_start (ap, fmt) ;
	vfprintf (stderr, fmt, ap) ;
	fprintf (stderr, "\n") ;
	va_end (ap) ;
	exit (1) ;
}

void set_extension (const char * filename, const char * ext, char * buffer)
{
	char       * ptr ;
	const char * fptr ;

	/* Concatena la extensión al nombre de fichero */

	strcpy (buffer, filename) ;
	ptr = strchr (buffer, '.') ;

	if (ptr) strcpy (ptr, ext) ;
	else 	 strcat (buffer, ext) ;

	/* Pone la extensión en mayúsculas si el nombre lo está */

	for (fptr = filename ; *fptr ; fptr++)
		if (*fptr >= 'a' && *fptr <= 'z') break ;

	if (!*fptr)
	{
		for (ptr = buffer ; *ptr ; ptr++)
			*ptr = toupper(*ptr) ;
	}
}

void help ()
{
	printf ("MAP Utility v0.75\nCopyright (C) 1999 José Luis Cebrián Pagüe\nCopyright (C) 2002 Fenix Team\n"
		"This utility comes with ABSOLUTELY NO WARRANTY; map -h for details\n\n") ;

	printf ("Usage: map [option] file \n"
		"\n"
		"    -l      Describes MAP file (default option)\n"
		"    -p      Extract palette file (.PAL) from the graphic\n"
		"    -c      Convert MAP file to 16bpp format\n"
		"    -g      Convert MAP file to PNG file\n"
		"    -m      Convert PNG or GIF file to MAP file\n"
		"    -s      Modify MAP file parameters\n"
		"    -n      Remove MAP file transparency\n"
 		"    -z=#    PNG transparent color ('0' color) range\n"
		"    -#      Compresion level (0-9 NOTE: 0 DIV compatible for 8bpps MAP files)\n"
		"\n"
		"Options for -l/-m/-c: (# means integer value)\n"
		"\n"
		"    +name=...       Change the name for the MAP (up to 32 characters)\n"
		"    +id=#           Change MAP code (0-999))\n"
		"    +center=[#,#]   Change center for the MAP (control point 0)\n"
 		"    +animation=...  Animation sequence for ANIMATED MAPS\n"
		"    +speed=#        Frame duration (ms) for each animation frame\n"
		"    +#=[#,#]        Control point definition\n"
		"\n") ;
}

/* ----------------------------------------------------------------- */

typedef struct
{
	Uint8	magic[8] ;
	Uint16	width ;
	Uint16	height ;
	Uint32	code ;
	Sint8	name[32] ;
}
#ifdef __GNU_C__
__attribute__ ((packed))
#endif
MAP_HEADER ;

#define F_NCPOINTS	0x0FFF
#define F_ANIMATION	0x1000

typedef struct
{
	MAP_HEADER	header ;
	Uint8 *		palette ;
	void *		data ;
	Sint16 *	cpoints ;
	Sint16 *	animation ;
	Sint16		n_flags ;
	char		filename[12] ;
	int		depth ;
	int		frames ;
	int		animation_length ;
	int		animation_speed ;
}
MAP ;

#define PALETTE_SIZE (768+576)

#define MAP_MAGIC "map\x1A\x0D\x0A"
#define M16_MAGIC "m16\x1A\x0D\x0A"
#define PAL_MAGIC "pal\x1A\x0D\x0A"
#define FNT_MAGIC "fnt\x1A\x0D\x0A"
#define FPG_MAGIC "fpg\x1A\x0D\x0A"

void save_map (const char * filename, MAP * map)
{
	long   len ;
	gzFile * file = gzopen (filename, policy) ;

	if (!file) fatal_error ("%s: error al crear", filename) ;
	gzwrite (file, &map->header, sizeof(MAP_HEADER)) ;
	if (map->depth == 8) gzwrite (file, map->palette, PALETTE_SIZE) ;
	gzwrite (file, &map->n_flags, 2) ;
	if (map->n_flags & F_NCPOINTS)
		gzwrite (file, map->cpoints, 4 * (map->n_flags & F_NCPOINTS)) ;
	if (map->n_flags & F_ANIMATION)
	{
		gzwrite (file, &map->frames, 2) ;
		gzwrite (file, &map->animation_length, 2) ;
		gzwrite (file, &map->animation_speed, 2) ;
		gzwrite (file, map->animation, 2 * map->animation_length) ;
	}
	len = map->header.width * map->header.height * map->frames ;
	if (map->depth == 16) len *= 2 ;
	gzwrite (file, map->data, len) ;
	gzclose (file) ;
	printf (" => %s\n", filename) ;
}

MAP * load_gif (const char * filename)
{
	int i ;
	GifFileType * file ;
	MAP * map ;
	ColorMapObject * pal ;

	file = DGifOpenFileName (filename) ;
	if (!file) return NULL ;
	DGifSlurp (file) ;

	map = (MAP *) malloc(sizeof(MAP)) ;
	map->frames = file->ImageCount ;
	map->depth = 8 ;
	map->cpoints = 0 ;
	map->n_flags = 0 ;
	map->animation_speed  = 250 ;
	memcpy (map->header.magic, MAP_MAGIC, 8) ;
	strncpy (map->header.name, filename, 32) ;
	map->header.name[31] = 0 ;

	/* Paleta de colores */

	map->palette = (void *)malloc(PALETTE_SIZE) ;
	pal = file->SColorMap ;
	if (!pal)
		pal = file->SavedImages[0].ImageDesc.ColorMap ;
	if (!pal)
		fatal_error ("El GIF no contiene una paleta de colores") ;
	if (pal->ColorCount > 255)
		fatal_error ("El GIF tiene más de 256 colores") ;

	memset (map->palette, 0, PALETTE_SIZE) ;
	for (i = 0 ; i < pal->ColorCount ; i++)
	{
		map->palette[i*3  ] = ((pal->Colors[i].Red >> 2) & 0x3F) ;
		map->palette[i*3+1] = ((pal->Colors[i].Green >> 2)& 0x3F)  ;
		map->palette[i*3+2] = ((pal->Colors[i].Blue >> 2)& 0x3F)  ;
	}

	/* Tamaño de la imagen */

	map->header.width = file->SWidth ;
	map->header.height = file->SHeight ;
	for (i = 0 ; i < map->frames ; i++)
	{
		int w, h ;
		w =  file->SavedImages[i].ImageDesc.Width ;
		h =  file->SavedImages[i].ImageDesc.Height ;
		w += file->SavedImages[i].ImageDesc.Left ;
		h += file->SavedImages[i].ImageDesc.Top ;
		if (w > map->header.width)  map->header.width  = w ;
		if (h > map->header.height) map->header.height = h ;
	}

	/* Procesa las imágenes del GIF */

	map->data = malloc(map->header.width * map->header.height * map->frames) ;
	memset (map->data, 0, map->header.width * map->header.height * map->frames) ;

	for (i = 0 ; i < map->frames ; i++)
	{
		Uint8 * line = (Uint8 *)map->data + map->header.width * i ;
		Uint8 * base, * bptr, * ptr ;
		int x, y, bw, bg ;

		line += file->SavedImages[i].ImageDesc.Top
			* map->header.width * map->frames ;

		base = file->SavedImages[i].RasterBits ;
		bw =   file->SavedImages[i].ImageDesc.Width ;
		bg =   file->SBackGroundColor ;

		/* Busca e interpreta la extensión de control de animaciones */

		for (x = 0 ; x < file->SavedImages[i].ExtensionBlockCount ; x++)
		{
			ExtensionBlock * ext = &file->SavedImages[i].ExtensionBlocks[x] ;

			if (ext->Function == 0xf9)
			{
				bg = (Uint8)ext->Bytes[3] ;
				map->animation_speed = 10 *
					(((int)ext->Bytes[2] << 8) + ext->Bytes[1]) ;
			}
			else
				printf ("Unknown function 0x%02X at frame %d\n", ext->Function, x) ;
		}

		map->palette[bg*3  ] = map->palette[0] ;
		map->palette[bg*3+1] = map->palette[1] ;
		map->palette[bg*3+2] = map->palette[2] ;

		/* Copia la primera imagen sobre el resto */

		if (i > 0)
		for (y = 0 ; y < map->header.height ; y++ )
		{
			ptr  = (Uint8 *)map->data + map->header.width * map->frames * y ;
			bptr = ptr + map->header.width * i ;
			memcpy (bptr, ptr, map->header.width) ;
		}

		/* Copia la imagen */

		for (y = 0 ; y < file->SavedImages[i].ImageDesc.Height ; y++ )
		{
			bptr = base + bw*y ;
			ptr  = line + file->SavedImages[i].ImageDesc.Left ;
			for (x = 0 ; x < bw ; x++)
			{
				if ((*bptr) == bg)
					*ptr++ = 0 ;
				else if (*bptr == 0)
					*ptr++ = bg ;
				else
					*ptr++ = *bptr ;
				bptr++ ;
			}
			line += map->header.width * map->frames ;
		}
	}

	/* Animación */

	map->animation_length  = 0 ;
	map->animation = NULL ;
	if (map->frames > 1)
	{
		map->n_flags |= F_ANIMATION ;
		map->animation_length = map->frames ;
		map->animation = malloc(map->frames * 2) ;
		for (i = 0 ; i < map->frames ; i++)
			map->animation[i] = i+1 ;
	}

	return map ;
}

MAP * load_map (const char * filename)
{
	gzFile file = gzopen (filename, "rb") ;
	MAP * map ;
	int error = 0, len, clen ;

	if (!file) fatal_error ("%s: fichero no encontrado", filename) ;

	map = (MAP *) malloc(sizeof(MAP)) ;
	gzread (file, &map->header, sizeof(map->header)) ;
	strncpy (map->filename, filename, 12) ;

	/* Extensión: ficheros MAP de 16 bits */

	if (strcmp (map->header.magic, M16_MAGIC) == 0)
	{
		len          = map->header.width * map->header.height * 2 ;
		map->depth   = 16 ;
		map->palette = 0 ;
	}
	else
	{
		if (strcmp (map->header.magic, MAP_MAGIC) != 0)
			fatal_error ("%s: no es un fichero MAP", filename) ;

		len          = map->header.width * map->header.height ;
		map->depth   = 8 ;
		map->palette = (void *)malloc(PALETTE_SIZE) ;
		if (gzread (file, map->palette, PALETTE_SIZE) < PALETTE_SIZE)
			error = 1 ;
	}

	gzread (file, &map->n_flags, 2) ;
	clen = (map->n_flags & F_NCPOINTS) * 4 ;
	map->cpoints = (Sint16 *)malloc(clen+4) ;
	map->animation_length  = 0 ;
	map->animation_speed  = 50 ;
	map->frames  = 1 ;
	map->animation = NULL ;

	if (map->n_flags & F_NCPOINTS)
		if (gzread (file, map->cpoints, clen) < clen) error = 1 ;

	if (map->n_flags & F_ANIMATION)
	{
		if (gzread (file, &map->frames, 2) < 2) error = 1 ;
		if (gzread (file, &map->animation_length, 2) < 2) error = 1 ;
		if (gzread (file, &map->animation_speed, 2) < 2) error = 1 ;
		clen = 2 * map->animation_length ;
		map->animation = (Sint16 *)malloc(clen+2) ;
		if (gzread (file, map->animation, clen) < clen) error = 1 ;
		len *= map->frames ;
	}

	map->data    = malloc(len) ;
	if (gzread (file, map->data,    len ) < len ) error = 1 ;
	gzclose (file) ;

	if (error) fatal_error ("%s: fichero truncado", filename) ;
	return map ;
}

void save_png (const char * filename, MAP * map)
{
	FILE * file = fopen (filename, "wb") ;

	png_structp	png_ptr ;
	png_infop	info_ptr ;
	png_uint_32	k, i ;
	png_bytep	*rowpointers ;
	png_colorp	palette ;
	Uint32      * data, * ptr ;
	Uint16		* orig ;

	if (!file) fatal_error ("%s: error al crear", filename) ;

    rowpointers = malloc( sizeof(png_bytep) * map->header.height );
    if ( !rowpointers ) {
        fclose(file) ;
        return ;
    }

	png_ptr  = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0) ;
	info_ptr = png_create_info_struct  (png_ptr) ;

	if (!png_ptr || !info_ptr) {
        free ( rowpointers ) ;
	    fatal_error ("Sin memoria") ;
	}

	if (setjmp(png_ptr->jmpbuf))
	{
        free ( rowpointers ) ;
		fclose (file) ;
		png_destroy_write_struct (&png_ptr, NULL) ;
		fatal_error ("%s: error al escribir", filename) ;
	}

	png_init_io (png_ptr, file) ;

	if (map->depth == 8)
	{
		png_set_IHDR (png_ptr, info_ptr, map->header.width,
			map->header.height, 8, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
			PNG_FILTER_TYPE_BASE) ;

		palette = (png_colorp) png_malloc (png_ptr, 256*sizeof(png_color)) ;
		if (!palette) {
            free ( rowpointers ) ;
            fclose (file);
		    fatal_error ("Sin memoria") ;
		}
		for (k = 0 ; k < 256 ; k++)
		{
			palette[k].red   = map->palette[k*3]*4 ;
			palette[k].green = map->palette[k*3+1]*4 ;
			palette[k].blue  = map->palette[k*3+2]*4 ;
		}
		png_set_PLTE (png_ptr, info_ptr, palette, 256) ;
		png_write_info (png_ptr, info_ptr) ;
		for (k = 0 ; k < map->header.height ; k++)
			rowpointers[k] = (Uint8 *)map->data + map->header.width*k ;
		png_write_image (png_ptr, rowpointers) ;
		free (info_ptr->palette) ;
		info_ptr->palette = NULL ;
	}
	else
	{
		png_set_IHDR (png_ptr, info_ptr, map->header.width,
                      map->header.height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                      PNG_FILTER_TYPE_BASE) ;
		png_write_info (png_ptr, info_ptr) ;

		data = malloc (map->header.width * map->header.height * map->frames * 4) ;
		if (!data) {
            free ( rowpointers ) ;
            fclose (file);
		    fatal_error ("Sin memoria") ;
		}
		for (k = 0 ; k < map->header.height ; k++)
		{
			ptr  = data + map->header.width * k ;
			orig = (Uint16 *)map->data + map->header.width * map->frames * k ;
			rowpointers[k] = (Uint8 *)ptr ;
			for (i = 0 ; i < map->header.width ; i++)
			{
				if (*orig == 0) *ptr = 0x00000000 ;
				else
					*ptr = ((*orig & 0xf800) >> 8) |
					       ((*orig & 0x07e0) << 5) |
					       ((*orig & 0x001f) << 19)|
					       0xFF000000 ;

				orig++ ;
				ptr++  ;
			}
		}
		png_write_image (png_ptr, rowpointers) ;
		free (data) ;
		data = NULL ;
	}

	png_write_end (png_ptr, info_ptr) ;

    free ( rowpointers ) ;
    fclose (file);

	printf (" => %s\n", filename) ;
}

int png_code = 1 ;

MAP * load_png (const char * filename)
{
	MAP * bitmap ;
	unsigned int n, x ;
	int k ;
	Uint16 * ptr ;
	Uint32 * orig ;
	Uint32 * row ;
	FILE * png ;

	png_bytep	*rowpointers ;

	png_structp	png_ptr ;
	png_infop	info_ptr, end_info ;
	png_uint_32    	width, height, rowbytes ;
	png_colorp	palette ;
	int		depth, color ;

	bitmap = load_gif (filename) ;
	if (bitmap) return bitmap ;

	/* Abre el fichero y se asegura de que screen está inicializada */

	png = fopen (filename, "rb") ;
	if (!png) fatal_error ("No existe %s\n", filename) ;

	/* Prepara las estructuras internas */

	png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0) ;
	if (!png_ptr) fatal_error ("Error al cargar PNG") ;
	info_ptr = png_create_info_struct (png_ptr) ;
	end_info = png_create_info_struct (png_ptr) ;
	if (!info_ptr || !end_info) fatal_error ("Error al cargar PNG") ;

	/* Rutina de error */

	if (setjmp (png_ptr->jmpbuf))
	{
		png_destroy_read_struct (&png_ptr, &info_ptr, &end_info) ;
		fclose (png) ;
		return 0 ;
	}

	/* Recupera información sobre el PNG */

	png_init_io (png_ptr, png) ;
	png_read_info (png_ptr, info_ptr) ;
	png_get_IHDR (png_ptr, info_ptr, &width, &height, &depth, &color, 0, 0 , 0) ;

    row = malloc( sizeof(Uint32) * width );
    if ( !row ) {
		fclose (png) ;
        return 0 ;
    }

    rowpointers = malloc( sizeof(png_bytep) * height );
    if ( !rowpointers ) {
		fclose (png) ;
        free ( row ) ;
        return 0 ;
    }

	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
		fatal_error ("No se soportan PNG en escala de grises") ;

	/* Configura los distintos modos disponibles */

	if (depth < 8)
		png_set_expand(png_ptr) ;
	if (depth == 16)
		png_set_strip_16(png_ptr) ;
	if (color == PNG_COLOR_TYPE_RGB)
		png_set_filler (png_ptr, 0xFF, PNG_FILLER_AFTER) ;
	png_set_bgr(png_ptr) ;

	/* Recupera el fichero, convirtiendo a 16 bits si es preciso */

	rowbytes = png_get_rowbytes (png_ptr, info_ptr) ;

	bitmap = (MAP *) malloc(sizeof(MAP)) ;
	if (!bitmap) {
		fclose (png) ;
        free ( rowpointers ) ;
        free ( row ) ;
	    return 0 ;
	}
	bitmap->palette       = (void *)malloc(PALETTE_SIZE) ;
	bitmap->data          = malloc(width*height*(color == PNG_COLOR_TYPE_PALETTE ? 1:2)) ;
	bitmap->cpoints       = 0 ;
	bitmap->n_flags       = 0 ;
	bitmap->frames        = 1 ;
	bitmap->header.width  = (Uint16) width ;
	bitmap->header.height = (Uint16) height ;
	bitmap->header.code   = png_code++ ;

	bitmap->animation_speed = 250 ;

	strncpy (bitmap->header.name, filename, 32) ;
	bitmap->header.name[31] = 0 ;
	if (strchr(bitmap->header.name, '.'))
		*(strchr(bitmap->header.name, '.')) = 0 ;
	strncpy (bitmap->filename, bitmap->header.name, 12) ;

	if (color == PNG_COLOR_TYPE_PALETTE)
	{
		memcpy (bitmap->header.magic, MAP_MAGIC, 8) ;
		bitmap->depth = 8 ;

		palette = (png_colorp) png_malloc (png_ptr, 256*sizeof(png_color)) ;
		if (!palette) fatal_error ("Sin memoria") ;
		png_get_PLTE (png_ptr, info_ptr, &palette, &k) ;
		for (k-- ; k >= 0 ; k--)
		{
			bitmap->palette[k*3]   = palette[k].red/4   ;
			bitmap->palette[k*3+1] = palette[k].green/4 ;
			bitmap->palette[k*3+2] = palette[k].blue/4  ;
		}

		for (n = 0 ; n < height ; n++)
			rowpointers[n] = ((Uint8*)bitmap->data) + n*width ;
		png_read_image (png_ptr, rowpointers) ;

 		if (do_zeroc)
 		{
 			char tr[256] ;
 			for (k = 0 ; k < 256 ; k++)
 				if (palette[k].red   >= zeroc[0][0] &&
 				    palette[k].red   <= zeroc[1][0] &&
 				    palette[k].green >= zeroc[0][1] &&
 				    palette[k].green <= zeroc[1][1] &&
 				    palette[k].blue  >= zeroc[0][2] &&
 				    palette[k].blue  <= zeroc[1][2])
 					tr[k] = 0 ;
 				else
 					tr[k] = k ;
 			for (n = 0 ; n < height ; n++)
 				for (x = 0 ; x < width ; x++)
 					rowpointers[n][x] = tr[rowpointers[n][x]] ;
 		}
	}
	else
	{
		memcpy (bitmap->header.magic, M16_MAGIC, 8) ;
		bitmap->depth = 16 ;

		ptr = (Uint16*) bitmap->data ;

		for (n = 0 ; n < height ; n++)
		{
			rowpointers[0] = (void *)row ;
			png_read_rows (png_ptr, rowpointers, 0, 1) ;

			orig = row ;
			for (x = 0 ; x < width ; x++)
			{
 				if (do_zeroc)
 				if (((*orig & 0x0000FF) >>  0) >= zeroc[0][0] &&
 				    ((*orig & 0x0000FF) >>  0) <= zeroc[1][0] &&
 				    ((*orig & 0x00FF00) >>  8) >= zeroc[0][1] &&
 				    ((*orig & 0x00FF00) >>  8) <= zeroc[1][1] &&
 				    ((*orig & 0xFF0000) >> 16) >= zeroc[0][2] &&
 				    ((*orig & 0xFF0000) >> 16) <= zeroc[1][2])
 					*orig = 0x00000000 ;

				if ((*orig) & 0x80000000)
				{
					*ptr = ((*orig & 0x0000F8) >> 3)|
					       ((*orig & 0x00FC00) >> 5)|
					       ((*orig & 0xF80000) >> 8)  ;
					if (!*ptr) (*ptr)++ ;
				}
				else *ptr = 0 ;
				ptr++, orig++ ;
			}
		}
	}

	/* Fin */

	png_read_end (png_ptr, 0) ;
	fclose (png) ;
    free ( rowpointers ) ;
    free ( row ) ;
	return bitmap ;
}

void free_map (MAP * map)
{
	if (map->depth == 8 && map->palette) free (map->palette) ;
	if (map->n_flags & F_NCPOINTS) free (map->cpoints) ;
	free (map->data) ;
	free (map) ;
}

void data_notrans (MAP * map)
{
	int n = map->header.width * map->header.height * map->frames ;
	Uint8 * palette = map->palette ;

	if (map->depth == 16)
	{
		Uint16 * ptr = (Uint16 *) map->data ;
		while (n--)
		{
			if (*ptr == 0) *ptr = 1 ;
			ptr++ ;
		}
	}
	else if (map->palette)
	{
		Uint8 * ptr = (Uint8 *) map->data ;
		int i, b = 1, bc, c ;

		bc = palette[3] + palette[4] + palette[5] ;
		for (i = 1 ; i < 256 ; i++)
		{
			c = palette[i*3] + palette[i*3+1] + palette[i*3+2] ;
			if (c < bc) bc = c, b = i ;
		}

		while (n--)
		{
			if (*ptr == 0) *ptr = b ;
			ptr++ ;
		}
	}
}

Uint16 * data_8to16 (Uint8 * palette, Uint8 * data, int len)
{
	Uint16 convert[256] ;
	Uint16 * buffer, * ptr ;
	int    n, r, g, b ;

	buffer = (Uint16 *) malloc (len * 2) ;
	if (!buffer) fatal_error ("Error: sin memoria") ;
	for (n = 1 ; n < 256 ; n++)
	{
		r = ((palette[n*3] >> 1) << 11) ;
		g = (palette[n*3+1] << 5) ;
		b = (palette[n*3+2] >> 1) ;
		assert ((r & 0xF800) == r) ;
		assert ((g & 0x07E0) == g) ;
		assert ((b & 0x001F) == b) ;
		convert[n] = (r|g|b) ;
		if (convert[n] == 0) convert[n]++ ;
	}
	convert[0] = 0 ;
	for (n = 0, ptr = buffer ; n < len ; n++)
	{
		*ptr++ = convert[*data++] ;
	}
	return buffer ;
}

void convert_map (MAP * map)
{
	Uint16 * data ;
	int len ;

	if (map->depth == 16) return ;

	len  = map->header.width * map->header.height * map->frames ;
	data = data_8to16 (map->palette, map->data, len) ;
	free (map->data) ;
	map->data = data ;

	memcpy (map->header.magic, M16_MAGIC, 8) ;
	map->depth = 16 ;
}

void save_pal (const char * filename, char * palette)
{
	FILE * file = fopen (filename, "wb") ;

	if (!file) fatal_error ("%s: error al crear", filename) ;
	fwrite (PAL_MAGIC, 8, 1, file) ;
	fwrite (palette, PALETTE_SIZE, 1, file) ;
	fclose (file) ;
	printf (" => %s\n", filename) ;
}

char * load_pal (const char * filename)
{
	gzFile * file = gzopen (filename, "rb") ;
	char header[8] ;
	char * here = malloc (PALETTE_SIZE) ;

	if (!file) fatal_error ("%s: fichero no existente", filename) ;
	if (!here) fatal_error ("Sin memoria") ;

	gzread (file, header, 8) ;
	if (strcmp (header, MAP_MAGIC) == 0)
		gzseek (file, 48, SEEK_SET) ;
	else if (strcmp (header, FPG_MAGIC) != 0 &&
	    strcmp (header, FNT_MAGIC) != 0 &&
	    strcmp (header, PAL_MAGIC) != 0)
		fatal_error ("%s: no es un fichero de paleta válido", filename) ;

	gzread (file, here, PALETTE_SIZE) ;
	gzclose (file) ;
	return here ;
}

void list_map (const char * filename, MAP * map, int verbose)
{
	int   i ;
	char  buffer[32], name[36] = "" ;

	memcpy (buffer, map->header.name, 31) ;
	buffer[31] = 0 ;
	if (buffer[0]) sprintf (name, "\"%s\" ", buffer) ;

	printf ("%s: %s%dx%d pixels (%d bits)", filename, name,
		map->header.width, map->header.height, map->depth) ;
	if (verbose)
	{
		printf ("\n") ;
		for (i = 0 ; i < (map->n_flags & F_NCPOINTS) ; i++)
		{
			if (map->cpoints[2*i] == -1 && map->cpoints[2*i+1] == -1)
				continue ;
			if (i == 0) printf ("    centro: ") ;
			else        printf ("    pto%3d: ", i) ;
			printf ("%3d,%-3d\n", map->cpoints[2*i], map->cpoints[2*i+1]) ;
		}
		if (map->n_flags & F_ANIMATION)
		{
			printf ("   %d frames (%dms): ", map->frames,
				map->animation_speed) ;
			for (i = 0 ; i < map->animation_length ; i++)
				printf ("%3d", map->animation[i]) ;
			printf ("\n") ;
		}
	}
}

void set_cpoint (MAP * map, int no, int x, int y)
{
	int n ;

	if (!map->cpoints)
	{
		map->cpoints = (Sint16 *) malloc(4*(no+1)) ;
		for (n = 0 ; n < no ; n++)
			map->cpoints[n*2] = map->cpoints[n*2+1] = -1 ;
		map->n_flags = ((map->n_flags & ~F_NCPOINTS) | (no+1)) ;
	}
	else if ((map->n_flags & F_NCPOINTS) < no+1)
	{
		map->cpoints = (Sint16 *) realloc(map->cpoints, 4*(no+1)) ;
		for (n = (map->n_flags & F_NCPOINTS) ; n < no ; n++)
			map->cpoints[n*2] = map->cpoints[n*2+1] = -1 ;
		map->n_flags = ((map->n_flags & ~F_NCPOINTS) | (no+1)) ;
	}
	map->cpoints[no*2]   = x ;
	map->cpoints[no*2+1] = y ;
}

/* ----------------------------------------------------------------- */

void parse_rgbrange (char * text, Uint16 * range)
{
	Uint16 n = 0, *c = range ;
	while (*text && !isdigit(*text)) text++ ;
	while (*text && n < 6)
	{
 		*c = atoi(text) ;
 		while (isdigit(*text)) text++ ;
 		while (*text && !isdigit(*text)) text++ ;
 		n++, c++ ;
 	}
 	if (n == 3)
 	{
 		range[3] = range[0] ;
 		range[4] = range[1] ;
 		range[5] = range[2] ;
 	}
 	printf ("Rango de color: #%02X%02X%02X - #%02X%02X%02X\n",
 		range[0], range[1], range[2], range[3], range[4], range[5]) ;
}

void parse_coords (char * text, int * x, int * y)
{
	char * comma ;

	if (!*text) { *x = *y = -1 ; return ; }
	comma = strchr (text, ',') ;
	if (!comma) fatal_error ("Se requieren dos coordenadas") ;
	*x = atoi(text) ;
	*y = atoi(comma+1) ;
	if (*x < 0 || *y < 0) fatal_error ("Coordenadas incorrectas") ;
}

void parse_animation (char * text, MAP * map)
{
	Sint16 frames[1024], n_frames = 0, max_frame = 1 ;

	if (map->animation)
	{
		free (map->animation) ;
		map->animation = NULL ;
		map->header.width *= map->frames ;
		map->frames = 1 ;
	}

	while (isspace(*text)) text++ ;
	while (n_frames < 1024 && isdigit(*text))
	{
		frames[n_frames] = atoi(text) ;
		if (frames[n_frames] < 1)
			frames[n_frames] = 1 ;
		if (frames[n_frames] > max_frame)
			max_frame = frames[n_frames] ;
		n_frames++ ;

		text = strchr (text, ',') ;
		if (!text) break ;
		text++ ;
		while (isspace(*text)) text++ ;
	}

	if (!n_frames)
	{
		map->n_flags &= ~F_ANIMATION ;
		return ;
	}

	map->n_flags |= F_ANIMATION ;
	map->frames = max_frame ;
	map->animation_length = n_frames ;
	map->animation = (Sint16 *) malloc(n_frames * 2) ;
	memcpy (map->animation, frames, n_frames * 2) ;
	if ((map->header.width / map->frames) * map->frames != map->header.width)
		fatal_error ("El ancho no es múltiplo de %d\n", map->frames) ;
	map->header.width /= map->frames ;
}

void do_command (char * command, MAP * map)
{
	char * equal ;
	char name[128] ;
	char param[128] ;
	int  x, y ;

	equal = strchr (command, '=') ;
	if (!equal) equal = strchr (command, ':') ;
	if (!equal) fatal_error ("Comando '%s' sin signo '='\n", command) ;
	strncpy (name, command, 128) ;
	name[equal-command] = 0 ;
	strncpy (param, equal+1, 128) ;
	param[127] = 0 ;

	if (strcmp (name, "name") == 0)
	{
		strncpy (map->header.name, param, 31) ;
	}
	else if (strcmp (name, "center") == 0)
	{
		parse_coords (param, &x, &y) ;
		set_cpoint (map, 0, x, y) ;
	}
	else if (strcmp (name, "animation") == 0)
	{
		parse_animation (param, map) ;
	}
	else if (strcmp (name, "id") == 0)
	{
		map->header.code = atoi(param) ;
	}
	else if (strcmp (name, "speed") == 0)
	{
		map->animation_speed = atoi(param) ;
	}
	else if (*name >= '0' && *name <= '9')
	{
		parse_coords (param, &x, &y) ;
		set_cpoint (map, atoi(name), x, y) ;
	}
	else	fatal_error ("Comando '%s' desconocido", command) ;
}

void do_commands (MAP * map)
{
	int i ;

	for (i = 0 ; i < n_commands ; i++)
		do_command (commands[i], map) ;
}

/* ----------------------------------------------------------------- */

int main (int argc, char ** argv)
{
	int i ;
	char * ptr ;
	char filename[MAXPATH] ;
	MAP * map ;

	for (i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '+')
		{
			if (n_commands == 256)
				fatal_error ("Demasiados comandos") ;
			commands[n_commands++] = argv[i]+1 ;

			if (argc > i)
				memcpy (&argv[i], &argv[i+1], sizeof(char *) * (argc-i)) ;
			argc--, i-- ;
		}
		else if (argv[i][0] == '-')
		{
			ptr = argv[i] + 1 ;

			while (*ptr)
                        if (isdigit(*ptr))
                        {
                                policy = strdup ("wb ") ;
                                policy[2] = *ptr++ ;
                        }
                        else
                        switch (tolower(*ptr++))
			{
				case 'p':
					action = AC_PALETTE ;
					break ;
				case 'c':
					action = AC_16BITS ;
					break ;
				case 's':
					action = AC_UPDATE ;
					break ;
				case 'm':
					action = AC_TO_MAP ;
					break ;
				case 'g':
					action = AC_TO_PNG ;
					break ;
				case 'l':
					action = AC_LIST ;
					break ;
				case 'n':
					action = AC_NOTRANS ;
					break ;
				case 'h':
					help() ;
					return -1 ;
				case 'z':
					do_zeroc = 1 ;
					parse_rgbrange (ptr, (Uint16*)&zeroc) ;
					while (*ptr) ptr++ ;
					break ;
				default:
					fatal_error ("Error: opción -%c no reconocida\n", *ptr) ;
			}

			if (argc > i)
				memcpy (&argv[i], &argv[i+1], sizeof(char *) * (argc-i)) ;
			argc-- ;
			i-- ;
		}
	}

	if (argc <= 1)
	{
		help() ;
		exit(0) ;
	}

	switch (action)
	{
		case AC_LIST:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_map (argv[i]) ;
				if (!map) continue ;
				if (n_commands)
				{
					do_commands (map) ;
					list_map (argv[i], map, 1) ;
					save_map (argv[i], map) ;
				}
				else
					list_map (argv[i], map, 1) ;
				free_map (map) ;
			}
			break ;

		case AC_PALETTE:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_map (argv[i]) ;
				if (!map) continue ;
				list_map (argv[i], map, 0) ;
				if (map->depth != 8)
					fatal_error (": No contiene ninguna paleta") ;
				set_extension (argv[i], ".pal", filename) ;
				save_pal (filename, map->palette) ;
				free_map (map) ;
			}
			break ;

		case AC_16BITS:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_map (argv[i]) ;
				if (!map) continue ;
				do_commands(map) ;
				list_map (argv[i], map, 0) ;
				convert_map (map) ;
				save_map (argv[i], map) ;
				free_map (map) ;
			}
			break ;

		case AC_NOTRANS:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_map (argv[i]) ;
				if (!map) continue ;
				list_map (argv[i], map, 1) ;
				do_commands(map) ;
				data_notrans(map) ;
				save_map (argv[i], map) ;
				free_map (map) ;
			}
			break ;

		case AC_UPDATE:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_map (argv[i]) ;
				if (!map) continue ;
				do_commands(map) ;
				list_map (argv[i], map, 1) ;
				save_map (argv[i], map) ;
				free_map (map) ;
			}
			break ;

		case AC_TO_MAP:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_png (argv[i]) ;
				if (!map) continue ;
				do_commands(map) ;
				list_map (argv[i], map, 1) ;
				set_extension (argv[i], ".map", filename) ;
				save_map (filename, map) ;
				free_map (map) ;
			}
			break ;

		case AC_TO_PNG:
			for (i = 1 ; i < argc ; i++)
			{
				map = load_map (argv[i]) ;
				if (!map) continue ;
				do_commands(map) ;
				list_map (argv[i], map, 0) ;
				set_extension (argv[i], ".png", filename) ;
				save_png (filename, map) ;
				free_map (map) ;
			}
			break ;
	}

	exit(0);
}

