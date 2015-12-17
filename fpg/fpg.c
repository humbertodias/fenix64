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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <png.h>

#define MAXPATH  1024
#define MAXFILES  256
#define MAXPARAMS  64

char files[MAXFILES][MAXPATH] ;
int  n_files = 0 ;
char * palette_file = 0 ;
char * policy = "wb" ;

#define AC_LIST 	1
#define AC_PALETTE 	2
#define AC_ADD   	3
#define AC_DELETE	4
#define AC_EXTRACT      5
#define AC_16BITS	6
#define AC_CREATE	7

int action = AC_LIST ;
int preserve = 1 ;
int show_cpoints = 0 ;
int create_16bits = 1 ;

typedef unsigned int	Uint32 ;
typedef signed   int	Sint32 ;
typedef unsigned short	Uint16 ;
typedef signed   short	Sint16 ;
typedef unsigned char	Uint8  ;
typedef signed   char	Sint8  ;

#define PALETTE_SIZE (768+576)

typedef struct
{
	Uint32	code ;
	Uint32	regsize ;
	Uint8	name[32] ;
	Uint8	filename[12] ;
	Sint32	width ;
	Sint32	height ;
	Sint32	n_flags ;
}
FPG_MAPHEADER ;

#define F_NCPOINTS	0x0FFF
#define F_ANIMATION	0x1000

typedef struct
{
	Uint8	magic[8] ;
	Sint16	width ;
	Sint16	height ;
	Uint32	code ;
	Sint8	name[32] ;
}
MAP_HEADER ;

typedef struct
{
	MAP_HEADER	header ;
	Uint8 *		palette ;
	void *		data ;
	int  * 		cpoints ;
	Sint16 *	animation ;
	Sint16		n_flags ;
	char		filename[12] ;
	int		depth ;
	Uint16		frames ;
	Uint16		animation_length ;
	Uint16		animation_speed ;
}
MAP ;

void fatal_error (char * fmt, ...)
{
	va_list ap ;
	va_start (ap, fmt) ;
	vfprintf (stderr, fmt, ap) ;
	va_end (ap) ;
	printf ("\n") ;
	exit (1) ;
}

/* Rutina de utilidad para crear un nombre de fichero a partir de otro,
 * cambiándole la extensión en el proceso */

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

/* Rutina de carga de ficheros PNG */

int png_code = 1 ;

MAP * png_load (const char * filename)
{
	MAP * bitmap ;
	unsigned int n, x ;
	Uint16 * ptr ;
	Uint32 * orig ;
	Uint32 *row ;
	FILE * png ;

	png_bytep	*rowpointers ;

	png_structp	png_ptr ;
	png_infop	info_ptr, end_info ;
	png_uint_32    	width, height, rowbytes;
	int		depth, color ;

	/* Opción de especificar un número de código */

	if (strchr(filename, ':') && isdigit(*filename))
	{
		png_code = atoi(filename) ;
		filename = strchr(filename, ':')+1 ;
	}

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

	//bitmap = bitmap_new (0, width, height, color == PNG_COLOR_TYPE_PALETTE ? 8 : 16) ;
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
	bitmap->frames        = 1 ;
	bitmap->header.width  = (short)width ;
	bitmap->header.height = (short)height ;
	bitmap->header.code   = png_code++ ;
	bitmap->n_flags       = 0 ;

	strncpy (bitmap->header.name, filename, 32) ;
	bitmap->header.name[31] = 0 ;
	if (strchr(bitmap->header.name, '.'))
		*(strchr(bitmap->header.name, '.')) = 0 ;
	strncpy (bitmap->filename, bitmap->header.name, 12) ;

	if (color == PNG_COLOR_TYPE_PALETTE)
	{
                png_colorp pal ;
                int numcolors ;

		memcpy (bitmap->header.magic, "map\x1A\x0D\x0A", 8) ;
		bitmap->depth = 8 ;

                png_get_PLTE (png_ptr, info_ptr, &pal, &numcolors) ;
                for (n = 0 ; n < numcolors ; n++)
                {
                        bitmap->palette[n*3+0] = pal[n].red / 4 ;
                        bitmap->palette[n*3+1] = pal[n].green / 4 ;
                        bitmap->palette[n*3+2] = pal[n].blue / 4 ;
                }

		for (n = 0 ; n < height ; n++)
			rowpointers[n] = ((Uint8*)bitmap->data) + n*width ;
		png_read_image (png_ptr, rowpointers) ;
	}
	else
	{
		memcpy (bitmap->header.magic, "m16\x1A\x0D\x0A", 8) ;
		bitmap->depth = 16 ;

		ptr = (Uint16*) bitmap->data ;
		for (n = 0 ; n < height ; n++)
		{
			rowpointers[0] = (void *)row ;
			png_read_rows (png_ptr, rowpointers, 0, 1) ;

			orig = row ;
			for (x = 0 ; x < width ; x++)
			{
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

/* Rutina de carga de ficheros MAP */

MAP * load_map (const char * filename)
{
	gzFile * file ;
	MAP * map ;
	int error = 0, len, clen, force_code = -1 ;

	if (strstr(filename, ".png") || strstr(filename, ".PNG"))
	{
		map = png_load (filename) ;
		if (map) return map ;
	}

	if (strchr(filename, ':') && isdigit(*filename))
	{
		force_code = atoi(filename) ;
		filename = strchr(filename, ':')+1 ;
	}

	file = gzopen (filename, "rb") ;
	if (!file) fatal_error ("%s: fichero no encontrado\n", filename) ;

	map = (MAP *) malloc(sizeof(MAP)) ;
	gzread (file, &map->header, sizeof(map->header)) ;
	strncpy (map->filename, filename, 12) ;

	/* Extensión: ficheros MAP de 16 bits */

	if (strcmp (map->header.magic, "m16\x1A\x0D\x0A") == 0)
	{
		len          = map->header.width * map->header.height * 2 ;
		map->depth   = 16 ;
		map->palette = 0 ;
	}
	else
	{
		if (strcmp (map->header.magic, "map\x1A\x0D\x0A") != 0)
		{
			free (map) ;
			gzclose (file) ;
			return map ;
		}

		len          = map->header.width * map->header.height ;
		map->depth   = 8 ;
		map->palette = (void *)malloc(PALETTE_SIZE) ;
		if (gzread (file, map->palette, PALETTE_SIZE) < PALETTE_SIZE) error = 1 ;
	}

	gzread (file, &map->n_flags, 2) ;

	if (map->n_flags & F_NCPOINTS)
	{
		clen = (map->n_flags & F_NCPOINTS) * 4 ;
		map->cpoints = (int *)malloc(clen+4) ;
		if (gzread (file, map->cpoints, clen) < clen   ) error = 1 ;
	}

	map->frames = 1 ;

	if (map->n_flags & F_ANIMATION)
	{
		if (gzread (file, &map->frames, 2) < 2) error = 1 ;
		if (gzread (file, &map->animation_length, 2) < 2) error = 1 ;
		if (gzread (file, &map->animation_speed, 2) < 2) error = 1 ;
		clen = 2 * map->animation_length ;
		map->animation = (Sint16 *)malloc(clen+2) ;
		if (gzread (file, map->animation, clen) < clen) error = 1 ;
	}

	len = map->header.width * map->header.height * map->frames ;
	if (map->depth == 16) len *= 2 ;

	map->data    = malloc(len) ;
	if (gzread (file, map->data   , len ) < len    ) error = 1 ;

	gzclose (file) ;

	if (error) fatal_error ("%s: fichero truncado\n", filename) ;
	if (force_code >= 0) map->header.code = force_code ;
	return map ;
}

/* Rutina para convertir un MAP de 8 a 16 bits */

Uint16 * data_8to16 (Uint8 * palette, Uint8 * data, int len)
{
	Uint16 convert[256] ;
	Uint16 * buffer, * ptr ;
	int    n, r, g, b ;

	buffer = (Uint16 *) malloc (len * 2) ;
	if (!buffer) fatal_error ("Error: out of memory\n") ;
	for (n = 0 ; n < 256 ; n++)
	{
		r = palette[n*3] ;
		g = palette[n*3+1] ;
		b = palette[n*3+2] ;
		convert[n] = ((r&~1) << 10) | (g << 5) | (b >> 1) ;
		if (convert[n] == 0) convert[n]++ ;
		if (n == 0) convert[n] = 0 ;
	}
	for (n = 0, ptr = buffer ; n < len ; n++)
	{
		*ptr++ = convert[*data++] ;
	}
	return buffer ;
}

void map_8to16 (MAP * map)
{
	Uint16 * data ;
	int len ;

	len  = map->header.width * map->header.height * map->frames ;
	data = data_8to16 (map->palette, map->data, len) ;
	free (map->data) ;
        map->depth = 16 ;
	map->data = data ;
}

/* Rutina para abrir un FPG */

gzFile * fpg_open (const char * filename, int * is_16bits)
{
	gzFile * file = gzopen (filename, "rb") ;
	char   buffer[8] ;

	if (!file) fatal_error ("%s: fichero no encontrado\n", filename) ;

	gzread (file, buffer, 8) ;
	if (strcmp (buffer, "f16\x1A\x0D\x0A") == 0)
	{
		*is_16bits = 1 ;
		return file ;
	}
	if (strcmp (buffer, "fpg\x1A\x0D\x0A") == 0)
	{
		*is_16bits = 0 ;
		return file ;
	}

	fatal_error ("%s: no es un fichero fpg\n", filename) ;
	return file ;
}

/* Rutina para listar el contenido de un FPG */

void fpg_list (const char * filename)
{
	gzFile * file ;
	FPG_MAPHEADER map ;
	char name[128], fname[128], buffer[128] ;
	int n, is_16bits ;
	Uint16 x, y, frames ;

	file = fpg_open (filename, &is_16bits) ;
	if (!file)
	{
		printf ("\n") ;
		return ;
	}

	/* Salta la paleta de colores */

	if (!is_16bits) gzseek (file, PALETTE_SIZE, SEEK_CUR) ;

	printf ("# Contenido del fichero %s (%d bits):\n\n",
			filename, is_16bits ? 16 : 8) ;

	while (!gzeof(file))
	{
		n = gzread (file, &map, sizeof(FPG_MAPHEADER)) ;
		if (n < (int)sizeof(FPG_MAPHEADER))
			break ;

		strncpy (name, map.name, 32) ;
		strncpy (fname, map.filename, 12) ;
		name[32] = 0 ;
		fname[12] = 0 ;
		frames = 1 ;

		if (strchr(fname, ' '))
		{
			sprintf (buffer, "\"%s\"", fname) ;
			strcpy (fname, buffer) ;
		}
		if (strchr(name, ' '))
		{
			sprintf (buffer, "\"%s\"", name) ;
			strcpy (name, buffer) ;
		}

		printf ("%3d: %-14s %-32s # %4dx%-4d bitmap\n",
			map.code, fname, name, map.width, map.height) ;
		if (map.n_flags & F_NCPOINTS)
		{
			for (n = 0 ; n < (map.n_flags & F_NCPOINTS) ; n++)
			{
				gzread (file, &x, 2) ;
				gzread (file, &y, 2) ;
				if (x == 65535 && y == 65535) continue ;
				if (show_cpoints)
					printf ("     CPOINT %d: %d, %d\n", n, x, y) ;
			}
		}

		frames = 1 ;

		if (map.n_flags & F_ANIMATION)
		{
			gzread (file, &frames, 2) ;
			gzread (file, &x, 2) ;
			gzread (file, &y, 2) ;
			if (show_cpoints)
			{
				printf ("     FRAMES: %d\n", frames) ;
				printf ("     SPEED: %d\n", y) ;
				printf ("     SCRIPT: ") ;
			}
			for (n = 0 ; n < x ; n++)
			{
				gzread (file, &y, 2) ;
				if (show_cpoints) {
					if (n > 0) printf (", ") ;
					printf ("%d", y) ;
				}
			}
			if (show_cpoints) printf ("\n") ;
		}

		gzseek (file, (is_16bits ? 2:1) * frames * map.width * map.height, SEEK_CUR) ;
	}
	printf ("\n") ;
	gzclose (file) ;
}

/* Rutina que comprueba si un código de gráfico entra dentro de los
 * especificados en la línea de comandos */

int matches (FPG_MAPHEADER * map, char * param)
{
	unsigned int base, count = 1 ;
	char * ptr ;

	base = atoi(param) ;
	ptr = strchr(param, '-') ;
	if (ptr)
	{
		count = atoi(ptr+1)-base+1 ;
		if (count < 0) count = 1 ;
	}

	if (map->code >= base && map->code < base+count)
		return 1 ;

	ptr = strchr(param,',') ;
	if (ptr) return matches (map, ptr+1) ;
	return 0 ;
}

/* Creación de un nuevo FPG */

void load_pal (void * here, const char * filename)
{
	gzFile * file = gzopen (filename, "rb") ;
	char header[8] ;
	const char *ptr ;

	if (!file) {
		ptr = filename ;
		while (isdigit(*ptr)) ptr++ ;
		if (*ptr == ':')
			file = gzopen (ptr+1, "rb") ;
		if (!file)
			fatal_error ("%s: fichero no existente", filename) ;
	}

	gzread (file, header, 8) ;
	if (strcmp (header, "map\x1A\x0D\x0A") == 0)
		gzseek (file, 48, SEEK_SET) ;
	else if (strcmp (header, "fpg\x1A\x0D\x0A") != 0 &&
	    strcmp (header, "fnt\x1A\x0D\x0A") != 0 &&
	    strcmp (header, "pal\x1A\x0D\x0A") != 0)
		fatal_error ("%s: no es un fichero de paleta válido", filename) ;

	gzread (file, here, PALETTE_SIZE) ;
	gzclose (file) ;
}

void fpg_new (const char * filename, const char * palfile, int depth)
{
	FILE * file ;
	char palette[PALETTE_SIZE] ;

	if (palfile && depth == 8) load_pal (palette, palfile) ;

	file = fopen (filename, "wb") ;
	if (!file) fatal_error ("%s: error al escribir", filename) ;

	if (depth == 8)
	{
		fwrite ("fpg\x1A\x0D\x0A", 1, 8, file) ;
		if (!palfile) fatal_error ("Necesita una paleta de colores para crear un FPG de 8 bits");
		fwrite (palette, 1, PALETTE_SIZE, file) ;
	}
	else
		fwrite ("f16\x1A\x0D\x0A", 1, 8, file) ;
	fclose (file) ;
}

/* Estados de los 1000 códigos de MAP disponibles */

#define ST_PRESENT	1
#define ST_TO_ADD	2
#define ST_DELETE	2

/* Función principal, que procesa un FPG creando una copia del mismo
 * que va alterando en función de la acción a realizar */

void fpg_process(const char * filename, int * nfile)
{
	gzFile * file ;
	gzFile * ofile ;
	FILE * ofile2 ;
	FPG_MAPHEADER map ;
	char name[128], fname[MAXPATH], fname2[MAXPATH] ;
	char output_filename[MAXPATH] ;
	int len, n, i, is_16bits ;
	Uint16 frames ;

	MAP * maps[128] ;
	int status[1000] ;
	int n_maps = 0 ;

	static char * buffer = 0, * ptr ;
	static int    buffer_len = 0 ;

	static char * palette ;

	/* Abre o crea el fichero */

	if (action == AC_CREATE)
	{
		fpg_new (filename, palette_file ? palette_file : 0,
				   create_16bits ? 16 : 8) ;
		action = AC_ADD ;
		if(!create_16bits)n_files--;
	}

	file = fpg_open (filename, &is_16bits) ;
	if (!file) fatal_error ("%s: no es un FPG\n", file) ;

	/* Carga los .MAP */

	if (action == AC_ADD || action == AC_CREATE)
	{
		for (n = (*nfile)+1 ; n < n_files ; n++)
		{
			maps[n_maps] = load_map (files[n]) ;
			if (maps[n_maps])
			{
				if (!palette_file) palette_file = files[n] ;
				if (maps[n_maps]->header.code > 0 &&
				    maps[n_maps]->header.code < 1000)
				{
					while (status[maps[n_maps]->header.code] == ST_TO_ADD)
					{
						maps[n_maps]->header.code++ ;
						if (maps[n_maps]->header.code == 1000)
							fatal_error ("No quedan identificadores disponibles") ;
					}
					status[maps[n_maps]->header.code] = ST_TO_ADD ;
				}
				if (is_16bits && maps[n_maps]->depth == 8)
					map_8to16 (maps[n_maps]) ;
				if (!is_16bits && maps[n_maps]->depth == 16)
					fatal_error ("Error: intento de añadir un gráfico de 16 bits a un FPG de 8\n") ;
				n_maps++ ;
			}
		}
		*nfile = n_files ;
		if (n_maps == 0 && action == AC_ADD)
			printf ("Aviso: el fichero FPG se creará vacío\n") ;
	}

	/* Recupera la paleta de colores */

	if (!is_16bits)
	{
		palette = (char *) malloc (PALETTE_SIZE) ;

		if (gzread (file, palette, PALETTE_SIZE) < (int)PALETTE_SIZE)
		{
			fatal_error ("%s: fichero truncado\n", filename) ;
			gzclose (file) ;
			return ;
		}
	}

	/* Graba la paleta y acaba si es eso lo que se solicitó */

	if (action == AC_PALETTE)
	{
		if (is_16bits)
			fatal_error ("Error: intento de extraer la paleta a un FPG de 16 bits\n") ;

		set_extension (filename, ".pal", fname2) ;
		ofile2 = fopen (fname2, "wb") ;
		if (ofile2)
		{
			fwrite ("pal\x1A\x0D\x0A", 1, 8, ofile2) ;
			fwrite (palette, 1, PALETTE_SIZE, ofile2) ;
			fclose (ofile2) ;

			printf ("%s -> %s\n", filename, fname2) ;
			gzclose (file) ;
			return ;
		}
		else
		{
			fatal_error ("%s: error al crear\n", fname2) ;
			return ;
		}
	}

	/* Crea un nuevo FPG a partir de éste */

	set_extension (filename, ".$$$", output_filename) ;

	ofile = gzopen (output_filename, policy) ;
	if (!ofile)
	{
		fatal_error ("%s: error al crear\n", output_filename) ;
		gzclose (file) ;
		return ;
	}

	if (!is_16bits && action != AC_16BITS)
	{
		gzwrite (ofile, "fpg\x1A\x0D\x0A", 8) ;
		if (gzwrite (ofile, palette, PALETTE_SIZE) < PALETTE_SIZE)
		{
			fatal_error ("%s: error de escritura\n", output_filename) ;
			gzclose (file) ;
			gzclose (ofile) ;
			return ;
		}
	}
	else
	{
		gzwrite (ofile, "f16\x1A\x0D\x0A", 8) ;
	}

	/* Gráficos incluídos */

	printf ("%s:\n\n", filename) ;

mainloop:
	if (action != AC_CREATE)
	while (!gzeof(file))
	{
		if (gzread (file, &map, sizeof(FPG_MAPHEADER))
				< (int)sizeof(FPG_MAPHEADER))
			break ;

		if (map.code < 0 || map.code >= 1000)
		{
			fatal_error ("Aviso: código %d erróneo\n", map.code) ;
			map.code = 0 ;
		}

		strncpy (name, map.name, 32) ;
		strncpy (fname, map.filename, 12) ;
		name[32] = 0 ;
		fname[12] = 0 ;

		/* Extrae gráficos */

		if (action == AC_EXTRACT)
		for (n = (*nfile)+1 ; n_files < 2 || n < n_files ; n++)
		{
			MAP_HEADER h ;

			if (n_files > 1 && !matches(&map, files[n]))
				continue ;

			set_extension (fname, ".map", fname2) ;

			ofile2 = gzopen (fname2, policy) ;
			if (ofile2)
			{
				if (is_16bits)
					strcpy (h.magic, "m16\x1A\x0D\x0A") ;
				else
					strcpy (h.magic, "map\x1A\x0D\x0A") ;

				h.width = map.width ;
				h.height = map.height ;
				h.code = map.code ;
				strncpy (h.name, map.name, 32) ;

				/* Palette */
				gzwrite (ofile2, &h, sizeof(h)) ;
				if (!is_16bits)
					gzwrite (ofile2, palette, PALETTE_SIZE) ;
				gzwrite (ofile2, &map.n_flags, 2) ;

				/* Control points */
				len = (map.n_flags & F_NCPOINTS) * 4 ;
				if (len > 0)
				{
					if (buffer_len < len)
					{
						buffer = (char *)realloc(buffer, len) ;
						buffer_len = len ;
					}
					if (gzread (file, buffer, len) < len)
					{
						fatal_error ("%s: fichero truncado\n", filename) ;
						break ;
					}
					gzwrite (ofile2, buffer, len) ;
				}

				len = map.width * map.height * (is_16bits? 2:1);

				/* Animation */
				if (map.n_flags & F_ANIMATION)
				{
					Uint16 frames, speed, length ;
					gzread (file, &frames, 2) ;
					gzread (file, &length, 2) ;
					gzread (file, &speed, 2) ;
					if (buffer_len < length * 2)
					{
						buffer = (char *)realloc(buffer, length * 2) ;
						buffer_len = length * 2 ;
					}
					gzwrite (ofile2, &frames, 2) ;
					gzwrite (ofile2, &length, 2) ;
					gzwrite (ofile2, &speed, 2) ;
					gzread (file, buffer, length * 2) ;
					gzwrite (ofile2, buffer, length * 2) ;
					len *= frames ;
				}

				/* Pixel Data */
				if (buffer_len < len)
				{
					buffer = (char *)realloc(buffer, len) ;
					buffer_len = len ;
				}
				if (gzread (file, buffer, len) < len)
				{
					fatal_error ("%s: fichero truncado\n", filename) ;
					break ;
				}
				if (gzwrite (ofile2, buffer, len) < len)
				{
					fatal_error ("%s: error de escritura\n", output_filename) ;
					break ;
				}
				gzclose (ofile2) ;

				printf ("  %03d: %-32s -> %s\n",
						map.code, map.name, fname2) ;
				goto mainloop ;
			}
			else
				fatal_error ("%s: error al abrir", fname2) ;

			if (n_files < 2) break ;
		}

		/* Borra gráficos */

		if (action == AC_DELETE)
		{
			for (n = (*nfile)+1 ; n < n_files ; n++)
			{
				if (matches(&map, files[n]))
				{
					status[map.code] = ST_DELETE ;
					break ;
				}
			}
		}

		if (status[map.code] == ST_TO_ADD || status[map.code] == ST_DELETE)
		{
			gzseek (file, 4 * (map.n_flags & F_NCPOINTS), SEEK_CUR) ;
			if (map.n_flags & F_ANIMATION)
			{
				Uint16 frames, length ;
				gzread (file, &frames, 2) ;
				gzread (file, &length, 2) ;
				gzseek (file, 2 + length*2, SEEK_CUR) ;
				gzseek (file, (is_16bits?2:1)*frames*map.width*map.height, SEEK_CUR) ;
			}
			else
				gzseek (file, (is_16bits?2:1)*map.width*map.height, SEEK_CUR) ;
			printf ("  %03d: %-32s ELIMINADO\n", map.code, name) ;
			continue ;
		}
		else 	status[map.code] = ST_PRESENT ;

		/* Pasa los gráficos al nuevo fichero */

		if (gzwrite (ofile, &map, sizeof(map))
				< (int)sizeof(map))
		{
			fatal_error ("%s: error de escritura\n", output_filename) ;
			break ;
		}

		/* Puntos de control */

		len = (map.n_flags & F_NCPOINTS) * 4 ;
		if (len)
		{
			if (buffer_len < len)
			{
				buffer = (char *)realloc(buffer, len) ;
				buffer_len = len ;
			}
			if (gzread (file, buffer, len) < len)
			{
				fatal_error ("%s: fichero truncado %d\n", filename, len) ;
				break ;
			}
			if (gzwrite (ofile, buffer, len) < len)
			{
				fatal_error ("%s: error de escritura\n", output_filename) ;
				break ;
			}
		}

		/* Animación */

		frames = 1 ;
		if (map.n_flags & F_ANIMATION)
		{
			Uint16 length, speed ;
			gzread (file, &frames, 2) ;
			gzread (file, &length, 2) ;
			gzread (file, &speed, 2) ;
			gzwrite (ofile, &frames, 2) ;
			gzwrite (ofile, &length, 2) ;
			gzwrite (ofile, &speed, 2) ;
			len = 2 * length ;
			if (buffer_len < len)
			{
				buffer = (char *)realloc(buffer, len) ;
				buffer_len = len ;
			}
			gzread (file, buffer, len) ;
			gzwrite (ofile, buffer, len) ;
		}

		/* Datos del gráfico */

		len = map.width * map.height * frames ;
		if (is_16bits) len *= 2 ;
		if (buffer_len < len)
		{
			buffer = (char *)realloc(buffer, len) ;
			buffer_len = len ;
		}
		if (gzread (file, buffer, len) < len)
		{
			fatal_error ("%s: fichero truncado\n", filename) ;
			break ;
		}
		if (!is_16bits && action == AC_16BITS)
		{
			ptr = (char *) data_8to16 (palette, buffer, len) ;
			len *= 2 ;
			if (buffer_len < len)
			{
				free (buffer) ;
				buffer = ptr ;
				buffer_len = len ;
			}
			else
			{
				memcpy (buffer, ptr, len) ;
				free (ptr) ;
			}

			printf ("  %03d: %-32s CONVERTIDO\n", map.code, map.name) ;
		}
		if (gzwrite (ofile, buffer, len) < len)
		{
			fatal_error ("%s: error de escritura\n", output_filename) ;
			break ;
		}
	}

	/* Añade gráficos al fpg */

	if (action == AC_ADD)
	{
		for (n = 0 ; n < n_maps ; n++)
		{
			MAP * bitmap = maps[n] ;

			if (bitmap->header.code < 1 || bitmap->header.code > 999)
			{
				for (i = 1 ; i < 1000 ; i++)
				{
					if (status[i] == 0) break ;
				}
				status[i] = ST_PRESENT ;
				bitmap->header.code = i ;
			}

			map.code = bitmap->header.code ;
			map.width = bitmap->header.width ;
			map.height = bitmap->header.height ;
			map.n_flags = bitmap->n_flags ;
			strncpy (map.filename, bitmap->filename, 12) ;
			strncpy (map.name, bitmap->header.name, 32) ;
			map.regsize = map.width + map.height + 4*(map.n_flags & F_NCPOINTS) + sizeof(map) ;

			gzwrite (ofile, &map, sizeof(map)) ;
			gzwrite (ofile, bitmap->cpoints, 4 * (map.n_flags & F_NCPOINTS)) ;
			if (!bitmap->frames) bitmap->frames = 1 ;
			if (bitmap->n_flags & F_ANIMATION)
			{
				gzwrite (ofile, &bitmap->frames, 2) ;
				gzwrite (ofile, &bitmap->animation_length, 2) ;
				gzwrite (ofile, &bitmap->animation_speed, 2) ;
				gzwrite (ofile, bitmap->animation, bitmap->animation_length * 2) ;
			}
			len = map.width * map.height * bitmap->frames * (is_16bits ? 2:1) ;
			gzwrite (ofile, bitmap->data, len) ;
			printf ("  %03d: %-32s AÑADIDO\n", map.code, map.name) ;
		}
		*nfile = n_files ;
	}

	printf ("\n") ;
	gzclose (ofile) ;
	gzclose (file) ;

	if (action == AC_ADD || action == AC_DELETE || action == AC_16BITS ||
	    (action == AC_EXTRACT && !preserve))
	{
		unlink (filename) ;
		rename (output_filename, filename) ;
	}
	else unlink (output_filename) ;

	if (action == AC_DELETE || action == AC_EXTRACT)
		*nfile = n_files ;
}

void help ()
{
	printf ("FPG Utility - Copyright (C) 1999 José Luis Cebrián Pagüe\n"
		"This utility comes with ABSOLUTELY NO WARRANTY; fpg -h for details\n\n") ;

	printf ("Uso: fpg [opcion] fichero [gráfico ...]\n"
		"\n"
		"    -l      Describe el FPG (opción por defecto): -v más extenso\n"
		"    -n      Crea un nuevo FPG, opcionalmente añadiendo MAPs\n"
		"    -o      Crea un nuevo FPG de 8 bits, opcionalmente añadiendo MAPs\n"
		"    -d      Elimina los gráficos indicados del FPG\n"
		"    -p      Extrae la paleta (.PAL) del fichero\n"
		"    -e      Extrae gráficos (.MAP) del FPG y los borra del mismo\n"
		"    -x      Extrae gráficos (.MAP) del FPG\n"
		"    -a      Añade  gráficos (.MAP) al FPG\n"
		"    -c      Convierte el .FPG a 16 bits\n"
		"    -#      Nivel de compresión (0 a 9)\n"
		"\n"
		"Las opciones -x y -e admiten indicar los gráficos mediante su código.\n"
		"Se pueden utilizar rangos a-b y varios gráficos separados por comas.\n"
		"Las opciones -p y -l admiten múltiples ficheros fpg\n\n") ;
}

int main (int argc, char ** argv)
{
	int i ;
	char * ptr ;

	for (i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '-' || argv[i][0] == '/')
		{
			ptr = argv[i] + 1 ;

			while (*ptr)
			{
				if (isdigit(*ptr))
				{
					policy = strdup ("wb ") ;
					policy[2] = *ptr ;
				}
				else
				switch (tolower(*ptr))
				{
					case 'n':
						action = AC_CREATE ;
						break ;
					case 'o':
						action = AC_CREATE ;
						create_16bits = 0 ;
						if (argc <= i+2)
							fatal_error ("Especifique nombre de paleta tras -o") ;
						palette_file = argv[i+2] ;
						break ;
					case 'a':
						action = AC_ADD ;
						break ;
					case 'd':
						action = AC_DELETE ;
						break ;
					case 'l':
						action = AC_LIST ;
						break ;
					case 'x':
						action = AC_EXTRACT ;
						break ;
					case 'e':
						action = AC_EXTRACT ;
						preserve = 0 ;
						break ;
					case 'p':
						if (action == AC_CREATE)
						{
							if (argc == i+1)
								fatal_error ("Especifique nombre de paleta tras -p") ;
							palette_file = argv[i+1] ;
							memcpy (&argv[i+1], &argv[i+2], sizeof(char *)*(argc-i)) ;
							argc-- ;
						}
						else action = AC_PALETTE ;
						break ;
					case 'v':
						show_cpoints = 1 ;
						break ;
					case 'c':
						action = AC_16BITS ;
						break ;
					case 'h':
						help() ;
						return -1 ;
					default:
						fatal_error ("Error: opción -%c no reconocida\n", *ptr) ;
				}
				ptr++ ;
			}
		}
		else
		{
			if (n_files == MAXFILES)
				fatal_error ("Error: demasiados ficheros\n") ;
			strcpy (files[n_files++], argv[i]) ;
		}
	}

	if (n_files == 0)
	{
		help() ;
		return -1 ;
	}

	for (i = 0 ; i < n_files ; i++)
	{
		switch (action)
		{
			case AC_LIST:
				fpg_list (files[i]) ;
				break ;
			case AC_CREATE:
			case AC_DELETE:
			case AC_PALETTE:
			case AC_ADD:
			case AC_EXTRACT:
			case AC_16BITS:
				fpg_process (files[i], &i) ;
				break ;
			default:
				fatal_error ("Error: Acción no disponible\n") ;
		}
	}

	exit(0);
}

