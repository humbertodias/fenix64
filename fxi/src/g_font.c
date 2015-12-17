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
 * FILE        : g_font.c
 * DESCRIPTION : Font loading and manipulation
 *
 * HISTORY:	0.82 - First version (used to be in g_texts.c)
 */

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "fxi.h"

FONT * fonts[256] = { 0 } ;
int    font_count = 0 ;		/* Fuente 0 reservada para sistema */

/*
 *  FUNCTION : gr_font_new
 *
 *  Create a new font, with no characters in it.
 *  The font uses the MS-DOS charset and 8bpp by default.
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      Code of the new font or -1 if error
 *      The font data is in the global array fonts[index]
 *
 */

int gr_font_new ()
{
	FONT * f = (FONT *)malloc(sizeof(FONT));

	if (f == NULL)
	{
		gr_error ("gr_font_new: sin memoria");
		return -1;
	}
	if (font_count == 255)
	{
		gr_error ("gr_font_new: demasiadas fuentes");
		return -1;
	}

	memset (f, 0, sizeof(FONT));
	f->charset = CHARSET_CP850;
	f->bpp = 8;
	f->maxwidth = 0;
	f->maxheight = 0;

	fonts[font_count] = f ;
	return font_count++ ;
}

/*
 *  FUNCTION : gr_font_newfrombitmap
 *
 *  Create a new font using a bitmap as source. The bitmap should be
 *  in black and white (1 bit per pixel) with a fixed character size
 *  and a character width of 8, 16, 24...
 *
 *  PARAMS :
 *		chardata		Pointer to the bitmap
 *		width			Width of each character, in bits (pixels)
 *		height			Height of each character
 *		options			Can be 0 or a combination of the following flags:
 *
 *	NFB_FIXEDWIDTH		Create a fixed width font
 *						(the default is a propotional width one)
 *
 *  RETURN VALUE :
 *      1 if MMX available, 0 otherwise
 *
 */

static void align_bitmap_char_left (unsigned char *data, int width, int height);
static int get_bitmap_char_width (unsigned char *data, int width, int height);

int gr_font_newfrombitmap (char * chardata, int width, int height, int options)
{
	FONT * f ;
	char * ptr, * charptr ;
	int i, y, id ;
	int charsize ;
	int linesize ;
	GRAPH * bitmap;

	id = gr_font_new() ;
	if (id == -1) return -1;
	f = fonts[id];
	f->bpp = 1;

	charsize = width/8 * height;
	linesize = width/8;

	for (charptr = chardata, i = 0 ; i < 256 ; i++, charptr += charsize)
	{
		if (options != NFB_FIXEDWIDTH)
			align_bitmap_char_left (charptr, width, height) ;

		bitmap = bitmap_new (i, width, height, 1, 1);
		if (bitmap == NULL)
		{
			gr_error ("gr_font_newfrombitmap: sin memoria");
			return id;
		}

		f->glyph[i].bitmap = bitmap;
		f->glyph[i].xoffset = 0;
		f->glyph[i].yoffset = 0;

		if (options != NFB_FIXEDWIDTH)
			f->glyph[i].xadvance = get_bitmap_char_width (charptr, width, height);
		else
			f->glyph[i].xadvance = width;

		bitmap_add_cpoint (bitmap, 0, 0) ;
		ptr = bitmap->data ;
		for (y = 0 ; y < height ; y++, ptr += bitmap->pitch)
			memcpy (ptr, charptr + linesize*y, linesize);

		if (bitmap->modified > 0)
			bitmap->modified = 0 ;
		bitmap->info_flags = 0 ;
	}

	/* Set a reasonable size for the space */

	f->glyph[32].xadvance = 5 ;
	f->maxwidth = width;
	f->maxheight = height;

	return id ;
}

/* Utility function used by gr_new_fontfrombitmap to align characters */

static void align_bitmap_char_left (unsigned char *data, int width, int height)
{
	int        leftest, n, c ;
	static int leftest_table[256] = { 0 } ;

	if (leftest_table[0] == 0)
	{
		for (n = 0 ; n < 256 ; n++)
		{
			if (n & 0x80) leftest_table[n] = 0 ;
			else if (n & 0x40) leftest_table[n] = 1 ;
			else if (n & 0x20) leftest_table[n] = 2 ;
			else if (n & 0x10) leftest_table[n] = 3 ;
			else if (n & 0x08) leftest_table[n] = 4 ;
			else if (n & 0x04) leftest_table[n] = 5 ;
			else if (n & 0x02) leftest_table[n] = 6 ;
			else if (n & 0x01) leftest_table[n] = 7 ;
			else               leftest_table[n] = 8 ;
		}
	}

	leftest = 8 ;
	for (n = 0 ; n < height ; n++)
	{
		for (c = 0 ; c < width ; c += 8)
		{
			if (leftest > c+leftest_table[data[(width*n+c)/8]])
				leftest = c+leftest_table[data[(width*n+c)/8]] ;
		}
	}

	if (leftest > 7)
	{
		if (width > leftest)
		{
			for (n = 0 ; n < height ; n++)
			{
				memmove (data + n*width/8, data + (n*width+leftest)/8,
					(width - leftest)/8);
			}
		}
		leftest &= 7;
	}

	for (n = 0 ; n < height*width/8 ; n++)
		data[n] <<= leftest ;
}

/* Utility function used by gr_new_fontfrombitmap to calculate char widths */

static int get_bitmap_char_width (unsigned char *data, int width, int height)
{
	int x, c, d, max = 0 ;

	while (height--)
	{
		for (x = 0 ; x < width ; x += 8)
		{
			c = *data++ ;
			for (d = 8 ; d > 0 ; d--, c >>= 1)
			{
				if (c & 0x01) break ;
			}
      		if (x*8+d > max) max = x*8+d ;
		}
	}
	return max+1 ;
}


/*
 *  FUNCTION : gr_font_systemfont
 *
 *  Create the system font. This function should be called once.
 *
 *  PARAMS :
 *		chardata		Pointer to the system font data
 *
 *  RETURN VALUE :
 *      Always returns 1
 *
 */

int gr_font_systemfont (char * chardata)
{
	int last_count = font_count ;
	if (fonts[0]) gr_font_destroy(0) ;
	font_count = 0 ;
	gr_font_newfrombitmap (chardata, 8, 8, 0);
	if (last_count) font_count = last_count ;

	return 1;
}


/*
 *  FUNCTION : gr_font_destroy
 *
 *  Destroy a font and all the internal bitmap data
 *
 *  PARAMS :
 *		fontid		ID of the font
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_font_destroy (int fontid)
{
	int n ;

	if (fontid >= 0 && fontid < font_count)
	{
		if (!fonts[fontid]) return ;
		for (n = 0 ; n < 256 ; n++)
		{
			if (fonts[fontid]->glyph[n].bitmap)
				bitmap_destroy (fonts[fontid]->glyph[n].bitmap) ;
		}
		free (fonts[fontid]) ;
		fonts[fontid] = NULL ;
		while (font_count > 0 && fonts[font_count-1] == 0)
			font_count-- ;
	}
}


/*
 *  FUNCTION : gr_font_load
 *
 *  Load a font from a given file, in FNT (DIV) format
 *
 *  PARAMS :
 *		filename		Name of the file
 *
 *  RETURN VALUE :
 *      ID of the new font, or -1 if error
 *
 */

static int gr_font_loadfrom (file * fp);

int gr_font_load (char * filename)
{
	file * fp ;
	int result;

	fp = file_open (filename, "rb") ;
	if (!fp) return -1 ;
	result = gr_font_loadfrom(fp);
	file_close (fp);
	return result;
}

static int gr_font_loadfrom (file * fp)
{
	char header[8];
	int bpp;
	int types, i, id;
	Uint32 y;
	FONT * f;

	struct
	{
		int	width ;
		int	height ;
		int	yoffset ;
		int	fileoffset ;
	}
	oldchardata[256];

	struct
	{
		int	width;
		int	height;
		int xadvance;
		int yadvance;
		int xoffset;
		int	yoffset;
		int	fileoffset ;
	}
	chardata[256] ;

	if (font_count == 256) return -1 ;

	/* Read the file header */

	if (!file_read(fp, header,8)) return -1;

	if (memcmp (header, "fnt\x1a\x0d\x0a", 7) != 0 &&
		memcmp (header, "fnx\x1a\x0d\x0a", 7) != 0)
	{
		gr_error ("gr_font_load: formato desconocido");
		return -1;
	}

	bpp = header[7];
	if (bpp == 0) bpp = 8;

	/* Read or ignore the palette */

	if (bpp == 8)
	{
		if (palette_loaded)
			file_seek (fp, 576 + 768, SEEK_CUR) ;
		else
			if (!gr_read_pal (fp)) return -1 ;
	}

	/* Read the character data (detect old format) */

	if (header[2] == 'x')
	{
		if (!file_readSint32(fp, &types)) return -1 ;
		if (!file_read(fp, chardata, sizeof(chardata)))
			return -1 ;
		for (i = 0 ; i < 256 ; i++)
		{
			ARRANGE_DWORD (&chardata[i].width);
			ARRANGE_DWORD (&chardata[i].height);
			ARRANGE_DWORD (&chardata[i].xadvance);
			ARRANGE_DWORD (&chardata[i].yadvance);
			ARRANGE_DWORD (&chardata[i].xoffset);
			ARRANGE_DWORD (&chardata[i].yoffset);
			ARRANGE_DWORD (&chardata[i].fileoffset);
		}
	}
	else
	{
		if (!file_readSint32(fp, &types)) return -1 ;
		if (!file_read(fp, oldchardata, sizeof(oldchardata)))
			return -1 ;
		for (i = 0 ; i < 256 ; i++)
		{
			ARRANGE_DWORD (&oldchardata[i].width);
			ARRANGE_DWORD (&oldchardata[i].height);
			ARRANGE_DWORD (&oldchardata[i].yoffset);
			ARRANGE_DWORD (&oldchardata[i].fileoffset);

			chardata[i].width      = oldchardata[i].width;
			chardata[i].height     = oldchardata[i].height;
			chardata[i].xoffset    = 0;
			chardata[i].yoffset    = oldchardata[i].yoffset;
			chardata[i].xadvance   = oldchardata[i].width;
			chardata[i].yadvance   = oldchardata[i].height + oldchardata[i].yoffset;
			chardata[i].fileoffset = oldchardata[i].fileoffset;
		}
	}

	/* Create the font */

	id = gr_font_new() ;
	if (id == -1) return -1;
	f = fonts[id];
	assert (f != 0) ;

	if (header[2] == 'x')
	{
		f->bpp = header[7];
		f->charset = types;
	}
	else
	{
		f->bpp = 8;
		f->charset = CHARSET_CP850;
	}

	/* Load the character bitmaps */

	for (i = 0 ; i < 256 ; i++)
	{
		GRAPH * gr;
		Uint8 * ptr;

		f->glyph[i].xadvance = chardata[i].xadvance ;
		f->glyph[i].yadvance = chardata[i].yadvance ;

		if (chardata[i].fileoffset == 0 ||
		    chardata[i].width      == 0 ||
		    chardata[i].height     == 0) continue ;

		f->glyph[i].xoffset = chardata[i].xoffset ;
		f->glyph[i].yoffset = chardata[i].yoffset ;

		file_seek (fp, chardata[i].fileoffset, SEEK_SET) ;
		f->glyph[i].bitmap = gr = bitmap_new (i, chardata[i].width,
			chardata[i].height, f->bpp, 1) ;
		assert (gr) ;
		bitmap_add_cpoint (gr, 0, 0) ;

		for (y = 0, ptr = gr->data ; y < gr->height ; y++, ptr += gr->pitch)
		{
			if (!file_read (fp, ptr, gr->widthb))
				break ;
			if (gr->depth == 16)
				gr_convert16_565ToScreen ((Uint16 *)ptr, gr->width);
		}

		f->glyph[i].yoffset = chardata[i].yoffset ;
	}
	if (f->glyph[32].xadvance == 0)
		f->glyph[32].xadvance = 4 ;

	return id ;
}


/*
 *  FUNCTION : gr_font_save
 *
 *  Write a font to disk, in FNT/FNX format
 *
 *  PARAMS :
 *		fontid			ID of the font to save
 *		filename		Name of the file to create
 *
 *  RETURN VALUE :
 *      1 if succeded or 0 otherwise
 *
 */

int gr_font_save (int fontid, const char * filename)
{
	char     fullname[1024];
	char *   ptr;
	gzFile * file;
	int      n;
	Uint32   y;
	long     offset;
	Uint8 *  block = NULL ;
	Uint8 *  lineptr;

	FONT *   font;
	Uint8    header[8];
	struct
	{
		int	width;
		int	height;
		int xadvance;
		int yadvance;
		int xoffset;
		int	yoffset;
		int	fileoffset ;
	}
	chardata[256] ;

	if (fontid < 0 || fontid > 255 || !fonts[fontid])
	{
		gr_error ("gr_font_save: fuente incorrecta");
		return 0;
	}
	font = fonts[fontid];

	/* If the file does not have an extension, append ".FNT" */

	memset (fullname, 0, sizeof(fullname));
	strncpy (fullname, filename, 1000);
	ptr = fullname + strlen(fullname) - 1;
	while (ptr > fullname && !strchr("/\\.", *ptr))
		ptr--;
	if (*ptr != '.')
		strcat (fullname, ".fnt");

	/* Open the file */

	file = gzopen (fullname, "wb");
	if (!file)
	{
		gr_error ("gr_font_save: no se pudo crear el fichero %s", fullname);
		return 0;
	}

	/* Write the header */

	memcpy (header, "fnx\x1a\x0d\x0a\x00", 7);
	header[7] = font->bpp;
	gzwrite (file, &header, 8);

	/* Write the palette */

	if (font->bpp == 8)
	{
		Uint8   colors[256][3];
		Uint8 * block = calloc(576,1) ;

		for (n = 1 ; n < 256 ; n++)
		{
			colors[n][0] = palette[n].r >> 2 ;
			colors[n][1] = palette[n].g >> 2 ;
			colors[n][2] = palette[n].b >> 2 ;
		}
		gzwrite (file, &colors, 768) ;
		gzwrite (file, block, 576) ;
		free(block) ;
	}

	/* Write the character information */

	memset (chardata, 0, sizeof(chardata));
	offset = 8 + 4 + (font->bpp == 8 ? 576+768:0) + sizeof(chardata);

	for (n = 0 ; n < 256 ; n++)
	{
		chardata[n].xadvance   = font->glyph[n].xadvance;
		chardata[n].yadvance   = font->glyph[n].yadvance;

		if (font->glyph[n].bitmap)
		{
			chardata[n].width      = font->glyph[n].bitmap->width;
			chardata[n].height     = font->glyph[n].bitmap->height;
			chardata[n].xadvance   = font->glyph[n].xadvance;
			chardata[n].yadvance   = font->glyph[n].yadvance;
			chardata[n].xoffset    = font->glyph[n].xoffset;
			chardata[n].yoffset    = font->glyph[n].yoffset;
			chardata[n].fileoffset = offset;

			offset += font->glyph[n].bitmap->widthb
				    * chardata[n].height;
		}

		ARRANGE_DWORD (&chardata[n].xadvance);
		ARRANGE_DWORD (&chardata[n].yadvance);
		ARRANGE_DWORD (&chardata[n].width);
		ARRANGE_DWORD (&chardata[n].width);
		ARRANGE_DWORD (&chardata[n].xoffset);
		ARRANGE_DWORD (&chardata[n].yoffset);
		ARRANGE_DWORD (&chardata[n].fileoffset);
	}

	ARRANGE_DWORD (&font->charset);
	gzwrite (file, &font->charset, 4);
	ARRANGE_DWORD (&font->charset);

	gzwrite (file, &chardata, sizeof(chardata));

	/* Write the character bitmaps */

	for (n = 0 ; n < 256 ; n++)
	{
		if (font->glyph[n].bitmap)
		{
			GRAPH * gr = font->glyph[n].bitmap;

			if (gr->depth != font->bpp)
			{
				gr_error ("gr_font_save: fuente corrupta");
				gzclose (file);
				return 0;
			}

			if (gr->depth == 16)
			{
				if ( (block = malloc(gr->widthb)) == NULL)
				{
					gr_error ("gr_font_save: sin memoria");
					gzclose (file);
					return 0;
				}
			}

			lineptr = gr->data;

			for (y = 0 ; y < gr->height ; y++, lineptr += gr->pitch)
			{
				if (gr->depth == 16)
				{
					memcpy (block, lineptr, gr->widthb);
					ARRANGE_WORDS (block, (int)gr->width);
					gr_convert16_ScreenTo565 ((Uint16 *)block, gr->width);
					gzwrite (file, block, gr->widthb);
				}
				else
				{
					gzwrite (file, lineptr, gr->widthb);
				}
			}

			if (gr->depth == 16)
				free(block);
		}
	}

	gzclose(file);
	return 1;
}

/*
 *  FUNCTION : gr_load_bdf
 *
 *  Load a BDF font from disk. This is a very simple loader that ignores
 *  anything that is not relevant to screen display or non-horizontal
 *  writing fonts.
 *
 *  PARAMS :
 *		filename		Name of the BDF file
 *
 *  RETURN VALUE :
 *      ID of the font if succeded or -1 otherwise
 *
 */

int gr_load_bdf (const char * filename)
{
	file * fp;
	Uint8 line[2048];
	Uint8 * ptr;
	Uint8 * optr;
	FONT * font;
	int id, x, y, i;
	int error = 0;

	Uint8 nibbleh[256];
	Uint8 nibblel[256];

	int default_xadvance = 0;
	int default_yadvance = 0;
	int in_char = 0;
	int encoding = -1;
	int width = 0;
	int height = 0;
	int xoffset = 0;
	int yoffset = 0;
	int xadvance = 0;
	int yadvance = 0;
	int minyoffset = 0;

	/* Arrays used to convert hex ASCII to binary */

	memset (nibbleh, 0, 256);
	memset (nibblel, 0, 256);

	for (i = '0' ; i <= '9' ; i++)
	{
		nibbleh[i] = ((i - '0') << 4);
		nibblel[i] = i - '0';
	}
	for (i = 10 ; i <= 15 ; i++)
	{
		nibbleh['A' + i - 10] = (i << 4);
		nibbleh['a' + i - 10] = (i << 4);
		nibblel['A' + i - 10] = i;
		nibblel['a' + i - 10] = i;
	}

	/* Open the file and create the font */

	fp = file_open (filename, "r");
	if (!fp) return -1;

	id = gr_font_new ();
	if (id < 0) return -1;
	font = fonts[id];
	font->bpp = 1;
	font->charset = CHARSET_ISO8859;
	font->maxwidth = 0;
	font->maxheight = 0;

	/* Process the file, a line each time */

	for (line[2047] = 0 ; ; )
	{
		if (!file_gets (fp, line, 2047))
			break;

		/* Handle global-level commands */

		if (strncmp (line, "DWIDTH ", 7) == 0 && !in_char)
		{
			default_xadvance = atoi(line+7);
			ptr = strchr (line+7, ' ');
			if (ptr) default_yadvance = atoi(ptr+1);
		}
		else if (strncmp (line, "STARTCHAR", 9) == 0)
		{
			in_char = 1;
			encoding = -1;
			height = 0;
			xadvance = default_xadvance;
			yadvance = default_yadvance;
		}
		else if (strncmp (line, "ENDCHAR", 7) == 0)
		{
			in_char = 0;
		}

		/* Handle character-level commands */

		else if (strncmp (line, "DWIDTH ", 7) == 0 && in_char)
		{
			xadvance = atoi(line+7);
			ptr = strchr (line+7, ' ');
			if (ptr) yadvance = atoi(ptr+1);
		}
		else if (strncmp (line, "ENCODING ", 9) == 0 && in_char)
		{
			encoding = atoi(line+9);
			if (encoding == -1)
			{
				ptr = strchr (line+7, ' ');
				if (ptr) encoding = atoi(ptr+1);
			}
		}
		else if (strncmp (line, "BBX ", 4) == 0 && in_char)
		{
			width = atoi(line+4);
			if (width & 7) width = (width & ~7)+8;
			if ((ptr = strchr (line+4, ' ')) == NULL) continue;
			height = atoi(ptr+1);
			if ((ptr = strchr (ptr+1, ' ')) == NULL) continue;
			xoffset = atoi(ptr+1);
			if ((ptr = strchr (ptr+1, ' ')) == NULL) continue;
			yoffset = atoi(ptr+1);
		}
		else if (strncmp (line, "BITMAP", 6) == 0)
		{
			/* Read bitmap data */
			if (encoding >= 0 && encoding < 256 && height > 0)
			{
				font->glyph[encoding].xadvance = xadvance;
				font->glyph[encoding].yadvance = yadvance;
				font->glyph[encoding].xoffset  = xoffset;
				font->glyph[encoding].yoffset  = -yoffset-height;

				if (minyoffset > -yoffset-height)
					minyoffset = -yoffset-height;

				error = 1;
				font->glyph[encoding].bitmap = bitmap_new (encoding, width, height, 1, 1);
				if (font->glyph[encoding].bitmap == 0)
					break;
				bitmap_add_cpoint (font->glyph[encoding].bitmap, 0, 0) ;

				if (font->maxwidth < width)
					font->maxwidth = width;
				if (font->maxheight < height)
					font->maxheight = height;

				for (y = 0 ; y < height ; y++)
				{
					if (!file_gets (fp, line, 2047))
						break;
					ptr  = line;
					optr = (Uint8 *)font->glyph[encoding].bitmap->data +
						   font->glyph[encoding].bitmap->pitch * y;

					for (x = 0 ; x < width ; x += 8)
					{
						if (!ptr[0] || !ptr[1]) break;
						*optr++ = nibbleh[ptr[0]] | nibblel[ptr[1]];
						ptr += 2;
					}
				}
				if (y != height) break;
				error = 0;
			}
		}
	}

	file_close (fp);

	if (error)
	{
		gr_error ("gr_load_bdf: incorrect BDF file");
		return -1;
	}

	/* Adjust yoffsets to positive */

	for (i = 0 ; i < 256 ; i++)
		font->glyph[i].yoffset -= minyoffset;

	if (font->glyph[32].xadvance == 0)
		font->glyph[32].xadvance = font->glyph['j'].xadvance;

	fonts[font_count] = font ;
	return font_count++ ;
}

/*
 *  FUNCTION : gr_font_get
 *
 *  Return a font object, given an ID
 *
 *  PARAMS :
 *		id		id of the font
 *
 *  RETURN VALUE :
 *      Pointer to the font object or NULL if it does not exist
 *
 */

FONT * gr_font_get (int id)
{
	if (id >= 0 && id <= 255)
		return fonts[id];
	return NULL;
}
