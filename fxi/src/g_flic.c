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
#include <stdlib.h>
#include <string.h>
#include <posix/assert.h>

#include "fxi.h"

#include <SDL.h>

extern SDL_Color palette[256] ;

/* --------------------------------------------------------------------- */
/* Librería para reproducir ficheros FLI directamente desde el disco     */
/* --------------------------------------------------------------------- */

void flic_destroy (FLIC * flic)
{
	file_close (flic->fp) ;

	if (flic->bitmap) 
		bitmap_destroy (flic->bitmap) ;

	free (flic->frame) ;
	free (flic) ;
	flic = NULL ;
}

FLIC * flic_open (const char * filename)
{
	FLIC * flic ;

	flic = (FLIC *) malloc (sizeof(FLIC)) ;
	if (!flic) return 0 ;

	flic->fp = file_open (filename, "rb") ;
	if (!flic->fp)
	{
		free (flic) ;
		return 0 ;
	}
	flic->frame_reserved = 8192 ;
	flic->frame = (FLIC_FRAME *) malloc(8192) ;
	if (!flic->frame)
	{
		file_close (flic->fp) ;
		free (flic) ;
		return 0 ;
	}

	if (!file_read (flic->fp, &flic->header, sizeof(FLIC_HEADER)))
	{
		flic_destroy (flic) ;
		return 0 ;
	}

	if (flic->header.type != 0xAF11 && flic->header.type != 0xAF12)
	{
		/* Tipo de FLIC no reconocido */
		flic_destroy (flic) ;
		return 0 ;
	}

	flic->bitmap = bitmap_new (0, flic->header.width, flic->header.height, 8) ;
	if (!flic->bitmap)
	{
		/* Tamaño incorrecto */
		flic_destroy (flic) ;
		return 0 ;
	}

	if (!flic->header.oframe1)
		flic->header.oframe1 = file_pos (flic->fp) ;

	flic->current_frame = 0 ;
	flic->finished      = 0 ;
	flic->last_frame_ms = SDL_GetTicks() ;

	if (flic->header.type == 0xAF11)
		flic->speed_ms = (int)((1000.0F/70.0F) * flic->header.speed) ;
	else
		flic->speed_ms = flic->header.speed ;

	return flic ;
}

static FLIC * flic_do_delta (FLIC * flic)
{
	GRAPH     * bitmap = flic->bitmap ;
	int          first_line, line_count ;
	Uint8        * ptr, * optr, * loptr, packet_count ;

	first_line = flic->chunk->delta_fli.first_line ;
	line_count = flic->chunk->delta_fli.line_count ;
	ptr        = flic->chunk->delta_fli.data ;

	optr = (Uint8 *)bitmap->data + bitmap->pitch * first_line ;

	while (line_count-- > 0)
	{
		packet_count = *ptr++ ;
		loptr        = optr ;

		while (packet_count--)
		{
			optr += *ptr++ ;

			if (*ptr == 0)
			{
				ptr++ ;
			}
			else if ((Sint8)*ptr > 0)
			{
				memcpy (optr, ptr+1, *ptr) ;
				optr += *ptr ;
				ptr += 1 + *ptr ;
			}
			else
			{
				memset (optr, ptr[1], -(Sint8)*ptr) ;
				optr += -(Sint8)*ptr ;
				ptr += 2 ;
			}
		}

		optr = loptr + bitmap->width ;
	}

	return flic ;
}

static FLIC * flic_do_delta_flc (FLIC * flic)
{
	GRAPH     * bitmap = flic->bitmap ;
	int          line_count ;
	Uint16     * ptr, opcode, line_skip ;
	Uint8      * optr , * loptr;
	Uint8        skip_count ;
	Sint8        data_count ;

	ptr = (Uint16 *)flic->chunk->raw.data ;
	line_count = *ptr++ ;

	optr = bitmap->data ;

	while (line_count > 0)
	{
		opcode = *ptr++ ;

		if ((opcode & 0xC000) == 0x8000)
		{
			optr[bitmap->width-1] = (opcode & 0xFF) ;
			continue ;
		}
		if ((opcode & 0xC000) == 0xC000)
		{
			line_skip = -(Sint16)opcode;
			optr += bitmap->pitch * line_skip ;
			continue ;
		}
		if ((opcode & 0xC000) == 0x4000)
		{
			flic_destroy (flic) ;
			return 0 ;
		}

		loptr = optr ;

		while (opcode-- > 0)
		{
			skip_count = *(Uint8 *)ptr ;
			data_count = *((Sint8 *)ptr + 1) ;
			optr += skip_count ;
			ptr++ ;

			if (data_count > 0)
			{
				memcpy (optr, ptr, (int)data_count*2) ;
				ptr  += data_count ;
				optr += 2*(int)data_count ;
			}
			else
			{
				data_count = -data_count ;
				while (data_count--)
				{
					*optr++ = (*ptr & 0xFF) ;
					*optr++ = (*ptr >> 8) ;
				}
				ptr ++ ;
			}
		}

		optr = loptr ;
		optr += bitmap->width ;
		line_count-- ;
	}

	return flic ;
}

static FLIC * flic_do_color (FLIC * flic)
{
	Uint8 * ptr ;
	int packet_count ;
	int copy_count ;
	SDL_Color * color ;

	ptr        = flic->chunk->raw.data ;

	packet_count = *(Uint16 *)ptr ;
	ptr += 2 ;

	while (packet_count-- > 0)
	{
		color = &palette[*ptr++] ;
		copy_count = *ptr++ ;
		if (copy_count < 1) copy_count = 256 ;

		if (flic->chunk->header.type == CHUNK_COLOR_64)
		{
			while (copy_count--)
			{
				color->r = (*ptr++ << 2) ;
				color->g = (*ptr++ << 2) ;
				color->b = (*ptr++ << 2) ;
				color++ ;
			}
		}
		else
		{
			while (copy_count--)
			{
				color->r = *ptr++ ;
				color->g = *ptr++ ;
				color->b = *ptr++ ;
				color++ ;
			}
		}
	}

	palette_loaded  = 1 ;
	palette_changed = 1 ;
	return flic ;
}

static FLIC * flic_do_brun (FLIC * flic)
{
	GRAPH     * bitmap = flic->bitmap ;
	int          line_count ;
	Uint8      * ptr, * optr, * loptr, packet_count ;
	Uint16	     remaining_width ;

	ptr        = flic->chunk->raw.data ;
	optr       = bitmap->data ;
	line_count = bitmap->height ;

	while (line_count-- > 0)
	{
		packet_count = *ptr++ ;
		loptr        = optr ;

		remaining_width = bitmap->width ;

		while (remaining_width > 0)
		{
			if (*ptr == 0)
			{
				ptr++ ;
			}
			else if ((Sint8)*ptr < 0)
			{
				memcpy (optr, ptr+1, -(Sint8)*ptr) ;
				optr += -(Sint8)*ptr ;
				remaining_width -= -(Sint8)*ptr ;
				ptr += 1 + -(Sint8)*ptr ;
			}
			else
			{
				memset (optr, ptr[1], *ptr) ;
				optr += *ptr ;
				remaining_width -= *ptr ;
				ptr += 2 ;
			}
		}
		assert (remaining_width ==0 );

		optr = loptr + bitmap->width ;
	}

	return flic ;
}

static FLIC * flic_do_chunk (FLIC * flic)
{
	int y;

	/* Procesa el contenido del chunk actual */

	switch (flic->chunk->header.type)
	{
		case CHUNK_BLACK:
			for (y = 0 ; y < flic->bitmap->height ; y++)
			{
				memset ((Uint8 *)flic->bitmap->data + flic->bitmap->pitch * y,
						0, flic->bitmap->height) ;
			}
			break ;

		case CHUNK_FLI_COPY:
			for (y = 0 ; y < flic->bitmap->height ; y++)
			{
				memcpy ((Uint8 *)flic->bitmap->data + flic->bitmap->pitch * y, 
						flic->chunk->raw.data + flic->bitmap->width * y,
						flic->bitmap->height) ;
			}
			break ;

		case CHUNK_DELTA_FLI:
			if (!flic_do_delta(flic)) return 0 ;
			break ;

		case CHUNK_DELTA_FLC:
			if (!flic_do_delta_flc(flic)) return 0 ;
			break ;

		case CHUNK_BYTE_RUN:
			if (!flic_do_brun(flic)) return 0 ;
			break ;

		case CHUNK_COLOR_256:
		case CHUNK_COLOR_64:
			if (!flic_do_color(flic)) return 0 ;
			break ;

		case CHUNK_STAMP:
			break ;

		default:
			/* Tipo de chunk desconocido */
			flic_destroy (flic) ;
			return 0 ;
	}

	return flic ;
}

FLIC * flic_do_frame (FLIC * flic)
{
	int chunkno = 0 ;
	int ms ;

	ms = SDL_GetTicks() ;

	if (flic->last_frame_ms + flic->speed_ms > ms || flic->finished)
		return flic ;

	/* Cuenta el frame y vuelve al inicio si es necesario */

	flic->current_frame++ ;

	if (flic->current_frame >= flic->header.frames)
	{
		file_seek (flic->fp, flic->header.oframe1, SEEK_SET) ;
		flic->current_frame = 1 ;
		flic->finished = 1 ;
		return flic ;
	} else {

	do
	{
		/* Recupera información del siguiente chunk del fichero */

		if (!file_read (flic->fp, flic->frame, sizeof(FLIC_FRAME)))
		{
			flic_destroy (flic) ;
			return 0 ;
		}
		if (flic->frame->type != CHUNK_FRAME && 
		    flic->frame->type != CHUNK_PREFIX)
		{
			/* Tipo de frame incorrecto */
			flic_destroy (flic) ;
			return 0 ;
		}
		if (flic->frame->size < sizeof(FLIC_FRAME))
		{
			/* Fichero corrupto */
			flic_destroy (flic) ;
			return 0;
		}

		/* Rserva la memoria necesaria y carga el chunk */

		if (flic->frame_reserved < flic->frame->size)
		{
			flic->frame_reserved = flic->frame->size ;
			flic->frame = (FLIC_FRAME *) realloc (flic->frame, 
					flic->frame_reserved) ;

			if (!flic->frame)
			{
				/* Error: sin memoria */
				file_close (flic->fp) ;
				free (flic) ;
				return 0 ;
			}
		}

		if (flic->frame->size > sizeof(FLIC_FRAME))
		if (!file_read (flic->fp, &flic->frame[1],
				flic->frame->size - sizeof(FLIC_FRAME)) )
		{
			flic_destroy (flic) ;
			return 0 ;
		}
	}
	while (flic->frame->type != CHUNK_FRAME) ;

	/* Procesa cada sub-chunk */

	flic->chunk = (FLIC_CHUNK *) &flic->frame[1] ;

	for (chunkno = 0 ; chunkno < flic->frame->chunks ; chunkno++)
	{
		if (flic_do_chunk (flic) == 0)
			return 0 ;

		flic->chunk = (FLIC_CHUNK *) 
			(  ((Uint8 *)flic->chunk) + flic->chunk->header.size  ) ;
	}

	flic->last_frame_ms += flic->speed_ms ;
	ms = SDL_GetTicks() ;
	if (flic->last_frame_ms < ms)
	{
		return flic_do_frame (flic) ;
	}
	return flic ;
	}
}

void flic_reset (FLIC * flic)
{
	flic->current_frame = 0 ;
	flic->finished = 0 ;
	file_seek (flic->fp, flic->header.oframe1, SEEK_SET) ;
}
