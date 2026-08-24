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
 * FILE        : g_texts.c
 * DESCRIPTION : Text manipulation routines
 *
 * HISTORY:	0.82 - New extended FONT format. Font functions moved to g_font.
 *          0.74 - gr_text_new_var modified to support data types
 *			0.74 - gr_text_put_all modified to suppor new data types 
 *			0.72 - Corrected gr_text_bitmap (WRITE_IN_MAP)
 */

#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "fxi.h"

#define MAX_TEXTS 512

int fntcolor8 = -1 ;
Uint16 fntcolor16 = 0xFFFF ;

#define TEXT_TEXT		1
#define TEXT_STRING		2
#define TEXT_INTEGER	3
#define TEXT_FLOAT		4
#define TEXT_WORD		5
#define TEXT_BYTE		6
#define TEXT_CHARARRAY	7

typedef struct _text 
{
	int on ;		/* 1 - Texto ; 2 - VarSTR; 3 - VarINT; 4 - VarFLOAT; 5 - VarWORD; 6 - VarBYTE */
	int fontid ;
	int x ;
	int y ;
	int z ;
	int alignment ;
	int color8 ;
	int color16 ;
	int objectid ;
	int last_value ;
	int owner ;             /* PROCESS_ID that created this text, or 0 */
	char * text ;           /* Memoria dinámica */
	const void * var  ;		/* CHANGED TO VOID to allow diff. data types */
} TEXT;

TEXT texts[MAX_TEXTS] ;

int text_nextid = 1 ;
int text_count  = 0 ;

static int text_creator = 0 ;

void gr_text_set_creator (int process_id)
{
	text_creator = process_id ;
}

#ifndef WIN32
char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}
#endif

/*
 *  FUNCTION : get_text
 *
 *  Returns the character string of a given text 
 *  (may be the representation of a integer or float value)
 *
 *  PARAMS : 
 *		text			Pointer to the text object
 *
 *  RETURN VALUE : 
 *      String contained within the text
 *
 */

static const char * get_text (TEXT * text)
{
	static char buffer[128];
	char * aux;
	int w;

	switch (text->on) 
	{
		case TEXT_TEXT:
			return text->text;
		case TEXT_STRING:
			return string_get(*(int*)text->var) ;
		case TEXT_INTEGER:
			sprintf (buffer, "%d", *(int *)text->var) ;
			return buffer ;
		case TEXT_FLOAT:
			sprintf (buffer, "%f", *(float *)text->var) ;
			/* REMOVING TRAILING 0s */
			aux = strrev(buffer) ;
			w=0 ;
			while (*aux=='0') w++, aux++;
			if (*aux=='.') aux-- ;
			strcpy(buffer,aux) ;
			strrev(buffer) ;
			return buffer ;
		case TEXT_BYTE:
			sprintf (buffer, "%d", *(Uint8 *)text->var) ;
			return buffer ;
		case TEXT_WORD:
			sprintf (buffer, "%d", *(Uint16 *)text->var) ;
			return buffer ;
		case TEXT_CHARARRAY:
			return (const char *)(text->var);
	}

	return "";
}

/*
 *  FUNCTION : info_text
 *
 *  Returns information about a text object
 *
 *  PARAMS : 
 *		text			Pointer to the text object
 *		bbox			Region to update with the text bounding box
 *
 *  RETURN VALUE : 
 *      1 if the text has changed since last frame
 *
 */

static int info_text (TEXT * text, REGION * bbox)
{
	const char * str = get_text(text);
	int x, y, width, height;
	REGION prev = *bbox;
	FONT * font;

	/* Calculate the text dimensions */

	x = text->x;
	y = text->y;
	width = gr_text_width (text->fontid, str);
	height = gr_text_height (text->fontid, str);

	/* Update the font's maxheight (if needed) */

	font = gr_font_get (text->fontid);
	if (font == NULL)
		return 0;

	if (font->maxheight == 0)
	{
		int c;

		for (c = 0 ; c < 256 ; c++)
		{
			if (font->glyph[c].bitmap == NULL)
				continue;
			if (font->maxheight < font->glyph[c].bitmap->height + font->glyph[c].yoffset)
				font->maxheight = font->glyph[c].bitmap->height + font->glyph[c].yoffset;
		}
	}

	/* Adjust top-left coordinates for text alignment */

	switch (text->alignment)
	{
		case 1:
		case 4:
		case 7:
			x -= width/2;
			break;
		case 2:
		case 5:
		case 8:
			x -= width-1;
			break;
	}

	switch (text->alignment)
	{
		case 3:
		case 4:
		case 5:
			y -= font->maxheight/2;
			break;
		case 6:
		case 7:
		case 8:
			y -= font->maxheight-1;
			break;
	}

	/* Fill the bounding box */

	bbox->x = x;
	bbox->y = y;
	bbox->x2 = x+width-1;
	bbox->y2 = y+height-1;

	/* Check if the var has changed since last call */

	if (bbox->x  != prev.x  || bbox->y  != prev.y ||
		bbox->x2 != prev.x2 || bbox->y2 != prev.y2)
		return 1;

	switch (text->on)
	{
		case TEXT_TEXT:
			return 0;
		case TEXT_STRING:
		case TEXT_FLOAT:
		case TEXT_INTEGER:
			if (text->last_value == *(int *)text->var)
				return 0;
			text->last_value = *(int *)text->var;
			return 1;
		case TEXT_BYTE:
			if (text->last_value == *(Uint8 *)text->var)
				return 0;
			text->last_value = *(Uint8 *)text->var;
			return 1;
		case TEXT_WORD:
			if (text->last_value == *(Uint16 *)text->var)
				return 0;
			text->last_value = *(Uint16 *)text->var;
			return 1;
		case TEXT_CHARARRAY:
			return 1;
	}
	return 0;
}

/*
 *  FUNCTION : draw_text
 *
 *  Draws a text object
 *
 *  PARAMS : 
 *		text			Pointer to the text object
 *		clip			Clipping region
 *
 *  RETURN VALUE : 
 *      None
 *
 */

void draw_text (TEXT * text, REGION * clip)
{
	const char * str = get_text(text);
	int save8, save16;
	int x, y, width, height;
	FONT * font;

	/* Calculate the text dimensions */

	x = text->x;
	y = text->y;
	width = gr_text_width (text->fontid, str);
	height = gr_text_height (text->fontid, str);

	/* Update the font's maxheight (if needed) */

	font = gr_font_get (text->fontid);
	if (font == NULL)
		return;

	if (font->maxheight == 0)
	{
		int c;

		for (c = 0 ; c < 256 ; c++)
		{
			if (font->glyph[c].bitmap == NULL)
				continue;
			if (font->maxheight < font->glyph[c].bitmap->height + font->glyph[c].yoffset)
				font->maxheight = font->glyph[c].bitmap->height + font->glyph[c].yoffset;
		}
	}

	/* Adjust top-left coordinates for text alignment */

	switch (text->alignment)
	{
		case 1:
		case 4:
		case 7:
			x -= width/2;
			break;
		case 2:
		case 5:
		case 8:
			x -= width-1;
			break;
	}

	switch (text->alignment)
	{
		case 3:
		case 4:
		case 5:
			y -= font->maxheight/2;
			break;
		case 6:
		case 7:
		case 8:
			y -= font->maxheight-1;
			break;
	}

	/* Draw the text */

	save8 = fntcolor8;
	save16 = fntcolor16;
	fntcolor8 = text->color8;
	fntcolor16 = text->color16;

	gr_text_put (0, clip, text->fontid, x, y, str) ;

	fntcolor8 = save8;
	fntcolor16 = save16;
}

/*
 *  FUNCTION : gr_text_new
 *
 *  Create a new text, using a fixed text string
 *
 *  PARAMS : 
 *		fontid			Font number
 *		x, y			Screen coordinates
 *		alignment		Alignment
 *		text			Pointer to text
 *
 *  RETURN VALUE : 
 *      None
 *
 */

int gr_text_new (int fontid, int x, int y, int alignment, const char * text)
{
	int textid = text_nextid ;

	if (text_nextid == MAX_TEXTS)
	{
		for (textid = 1 ; textid < MAX_TEXTS ; textid++)
			if (!texts[textid].on) break ;
		if (textid == MAX_TEXTS)
			gr_error ("Demasiados textos en pantalla") ;
	}
	else text_nextid++ ;
	text_count++ ;

	texts[textid].on = 1 ;
	texts[textid].fontid = fontid ;
	texts[textid].x = x ;
	texts[textid].y = y ;
	texts[textid].z = GLODWORD(TEXTZ) ;
	texts[textid].alignment = alignment ;
	texts[textid].text = text ? strdup(text) : 0 ;
	texts[textid].color8 = fntcolor8 ;
	texts[textid].color16 = fntcolor16 ;
	texts[textid].objectid = gr_new_object (texts[textid].z, info_text, draw_text, &texts[textid]);
	texts[textid].last_value = 0 ;
	texts[textid].owner = text_creator ;
	return textid ;
}

int gr_text_new_var (int fontid, int x, int y, int alignment, const void * var, int type)
{
	int textid = gr_text_new (fontid, x, y, alignment, 0) ;

	if (textid < 0) return -1 ;
	texts[textid].on = type ;
	if (type > 1)
		texts[textid].var = var ;
	
	return textid ;
}

void gr_text_move (int textid, int x, int y)
{
	if (textid > 0 && textid < text_nextid)
	{
		texts[textid].x = x ;
		texts[textid].y = y ;
	}
}

void gr_text_destroy (int textid)
{
	gr_text_destroy_from (textid, 0) ;
}

void gr_text_destroy_from (int textid, int caller_id)
{
	if (textid == 0)
	{
		for (textid = 1 ; textid < text_nextid ; textid++)
		{
			if (texts[textid].on)
			{
				/* HFF: dying menu does delete_text(0) in the same frame
				 * that main PROC's the next screen. Keep texts written by a
				 * newer process that is still running. */
				if (caller_id && texts[textid].owner > caller_id)
				{
					INSTANCE * owner = instance_get (texts[textid].owner) ;
					if (owner && LOCDWORD(owner, STATUS) == STATUS_RUNNING)
						continue ;
				}
				gr_destroy_object (texts[textid].objectid);
				free (texts[textid].text) ;
				texts[textid].on = 0 ;
				texts[textid].owner = 0 ;
			}
		}
		if (!caller_id)
		{
			text_count = 0 ;
			text_nextid = 1 ;
		}
		else
		{
			text_count = 0 ;
			for (textid = 1 ; textid < text_nextid ; textid++)
				if (texts[textid].on) text_count++ ;
			while (text_nextid > 1 && !texts[text_nextid-1].on)
				text_nextid-- ;
		}
		return ;
	}
	if (textid > 0 && textid < text_nextid)
	{
		if (!texts[textid].on) return ;

		gr_destroy_object (texts[textid].objectid);
		free (texts[textid].text) ;
		texts[textid].on = 0 ;
		texts[textid].owner = 0 ;
		if (textid == text_nextid-1)
		{
			while (text_nextid > 1 && !texts[text_nextid-1].on)
				text_nextid-- ;
		}
		text_count-- ;
	}
}

int gr_text_width (int fontid, const unsigned char * text)
{
	return gr_text_widthn (fontid, text, strlen(text));
}

int gr_text_widthn (int fontid, const unsigned char * text, int n)
{
	int l = 0 ;
	FONT * f ;

	if (fontid < 0 || fontid > 255 || !fonts[fontid])
		gr_error ("Tipo de letra incorrecto (%d)", fontid) ;
	
	f = fonts[fontid] ;
	
	while (*text && n--) 
	{
		switch (f->charset)
		{
			case CHARSET_ISO8859:
				l += f->glyph[dos_to_win[*text]].xadvance ;
				break;
			case CHARSET_CP850:
				l += f->glyph[*text].xadvance ;
				break;
		}
		text++;
	}
	return l ;
}

int gr_text_margintop (int fontid, const unsigned char * text)
{
	int minyoffset = 0x7FFFFFFF ;
	FONT * f ;

	if (fontid < 0 || fontid > 255 || !fonts[fontid])
	{
		gr_error ("Tipo de letra incorrecto (%d)", fontid) ;
		return 0;
	}
	
	f = fonts[fontid] ;

	if (!*text)
		return 0;
	while (*text) 
	{
		switch (f->charset)
		{
			case CHARSET_ISO8859:
				if (minyoffset > f->glyph[dos_to_win[*text]].yoffset)
					minyoffset = f->glyph[dos_to_win[*text]].yoffset;
				break;
			case CHARSET_CP850:
				if (minyoffset > f->glyph[*text].yoffset)
					minyoffset = f->glyph[*text].yoffset;
				break;
		}
		text++ ;
	}
	return minyoffset ;
}

int gr_text_height (int fontid, const unsigned char * text)
{
	int l = 0;
	int margin ;
	FONT * f ;

	if (fontid < 0 || fontid > 255 || !fonts[fontid])
	{
		gr_error ("Tipo de letra incorrecto (%d)", fontid) ;
		return 0;
	}

	margin = gr_text_margintop (fontid, text);
	f = fonts[fontid] ;

	if (!*text)
		return 0;

	while (*text) 
	{
		if (f->glyph[*text].bitmap)
		{
			switch (f->charset)
			{
				case CHARSET_ISO8859:
					if (l < f->glyph[dos_to_win[*text]].yoffset 
						+ f->glyph[dos_to_win[*text]].bitmap->height)
						l = f->glyph[dos_to_win[*text]].yoffset 
						+ f->glyph[dos_to_win[*text]].bitmap->height ;
					break;
				case CHARSET_CP850:
					if (f->glyph[*text].yoffset + f->glyph[*text].bitmap->height > l)
						l = f->glyph[*text].yoffset + f->glyph[*text].bitmap->height ;
					break;
			}
		}
		text++ ;
	}
	return l - margin ;
}

void gr_text_put (GRAPH * dest, REGION * clip, int fontid, int x, int y, const unsigned char * text)
{
	GRAPH * ch ;
	FONT   * f ;
	Uint8   current_char;
	int flags ;
	int save16, save8;

	if (!dest) dest = scrbitmap ;

        if (fontid < 0 || fontid > 255 || !fonts[fontid])
                gr_error ("Tipo de letra incorrecto (%d)", fontid) ;

        f = fonts[fontid] ;

	/* Se permite imprimir texto antes de inicializar los globales */

	flags = globaldata ? GLODWORD(TEXT_FLAGS) : 0 ;
	

	save8 = syscolor8;
	save16 = syscolor16;
	
	if (fntcolor8 == -1)
	{
		gr_setcolor (dest->depth == 8 ? gr_find_nearest_color(255,255,255):gr_rgb(255,255,255));
	}
	else
	{
		syscolor8 = fntcolor8;
		syscolor16 = fntcolor16;
	}

	while (*text)
	{
		switch (f->charset)
		{
			case CHARSET_ISO8859:
				current_char = dos_to_win[*text];
				break;
			case CHARSET_CP850:
				current_char = *text;
				break;
			default:
				current_char = 0;
				break;
		}

		ch = f->glyph[current_char].bitmap ;
		if (ch) 
		{
			gr_blit (dest, clip, x + f->glyph[current_char].xoffset, 
				              y + f->glyph[current_char].yoffset, 
							  flags, ch) ;
		}
		x += f->glyph[current_char].xadvance ;
		text++ ;
	}

	syscolor8 = save8;
	syscolor16 = save16;
}

GRAPH * gr_text_bitmap (int fontid, const char * text, int alignment)
{
	GRAPH * gr ;
	int x, y ;
	FONT   * f ;
        
        if (fontid < 0 || fontid > 255 || !fonts[fontid])
                gr_error ("Tipo de letra incorrecto (%d)", fontid) ;

        f = fonts[fontid] ;
	
	/* Un refresco de paleta en mitad de gr_text_put puede provocar efectos
	 * desagradables al modificar el tipo de letra del sistema */

	if (palette_changed) gr_refresh_palette() ;

	gr = bitmap_new_syslib (gr_text_width (fontid, text),
		gr_text_height (fontid, text), enable_16bits ? 16:8) ;
	assert (gr) ;
	gr_clear (gr) ;
	gr_text_put (gr, 0, fontid, 0, -gr_text_margintop(fontid, text), text) ;

	switch (alignment)
	{
		case 0:
		case 1:
		case 2:
			y = 0 ;
			break ;
		case 3:
		case 4:
		case 5:
			y = gr->height/2 ;
			break ;
		default:
			y = gr->height-1 ;
			break ;

	}

	switch (alignment)
	{
		case 0:
		case 3:
		case 6:
			x = 0 ;
			break ;
		case 1:
		case 4:
		case 7:
			x = gr->width/2 ;
			break ;
		default:
			x = gr->width-1 ;
			break ;

	}

	bitmap_add_cpoint (gr, x, y) ;
	return gr ;
}

void gr_text_setcolor (int c)
{
	int r, g, b;

	if (c == 0)
	{
		fntcolor8 = 0;
		fntcolor16 = 0;
	}
	else if (!enable_16bits)
	{
		fntcolor8 = c ;
	}
	else
	{
		gr_get_rgb (c, &r, &g, &b);
		fntcolor8 = gr_find_nearest_color (r, g, b);
		fntcolor16 = c ;
	}
}

int gr_text_getcolor()
{	
	return ((enable_16bits)?fntcolor16:fntcolor8) ;
}
