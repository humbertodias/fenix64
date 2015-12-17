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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include "fxi.h"

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

	flic->bitmap = bitmap_new (0, flic->header.width, flic->header.height, 8, 1) ;
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
	GRAPH       * bitmap = flic->bitmap ;
	int         first_line, line_count ;
	Uint8       * ptr, * optr, * loptr, packet_count ;
	int         size;

	first_line = flic->chunk->delta_fli.first_line ;
	line_count = flic->chunk->delta_fli.line_count ;
	ptr        = flic->chunk->delta_fli.data ;

	loptr = (Uint8 *)bitmap->data + bitmap->pitch * first_line ;

	while (line_count-- > 0)
	{
        optr = loptr ;
        loptr += bitmap->pitch ;
		packet_count = *ptr++ ;

		while (packet_count--) {
			optr += *ptr++ ;
			size = *(Sint8 *)ptr++ ;
            if (size > 0) {
				memcpy (optr, ptr, size) ;
			    ptr += size ;
			} else if (size < 0) {
                size = -size ;
				memset (optr, *ptr++, size) ;
			}
			optr += size ;
		}
	}

	return flic ;
}

static FLIC * flic_do_delta_flc (FLIC * flic)
{
	GRAPH   * bitmap = flic->bitmap ;
	int     line_count ;
	Uint16  * ptr, opcode ;
	Uint8   * optr, * loptr ;
	Sint8   data_count ;

	ptr = (Uint16 *)flic->chunk->raw.data ;

	line_count = *ptr++ ;

	optr = bitmap->data ;

	while (line_count > 0)
	{
		opcode = *ptr++ ;

		switch (opcode & 0xC000) {
		    case 0x0000:
		        loptr = optr ;
        		while (opcode-- > 0) {
        			optr += *(Uint8 *)ptr ;
        			data_count = *((Sint8 *)ptr + 1) ;
        			ptr++ ;

        			if (data_count > 0) {
        				memcpy (optr, ptr, (int)data_count * 2) ;
        				ptr += data_count ;
        			    optr += data_count * 2 ;
        			} else if (data_count < 0) {
        				data_count = -data_count ;
        				while (data_count--)
        				{
        					*optr++ = (*ptr & 0xFF) ;
        					*optr++ = (*ptr >> 8) ;
        				}
        				ptr++ ;
        			}
        		}

                optr = loptr ;
                optr += bitmap->width ;
        		line_count-- ;
                break ;

		    case 0x4000:
    			flic_destroy (flic) ;
    			return 0 ;

		    case 0x8000:
    			optr[bitmap->width-1] = (opcode & 0xFF) ;
    			break ;

		    case 0xC000:
                optr += bitmap->pitch * -(Sint16)opcode;
    			break ;
        }
	}

	return flic ;
}

static FLIC * flic_do_color (FLIC * flic)
{
	Uint8 * ptr ;
	int packet_count ;
	int copy_count ;
	SDL_Color * color ;

	ptr = flic->chunk->raw.data ;

	packet_count = *(Uint16 *)ptr ;
	ptr += 2 ;

	while (packet_count-- > 0)
	{
		color = &palette[*ptr++] ;
		copy_count = *ptr++ ;
		if (copy_count < 1) copy_count = 256 ;

		if (flic->chunk->header.type == CHUNK_COLOR_64) {
			while (copy_count--) {
				color->r = (*ptr++ << 2) ;
				color->g = (*ptr++ << 2) ;
				color->b = (*ptr++ << 2) ;
				color++ ;
			}
		} else {
			while (copy_count--) {
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
	GRAPH       * bitmap = flic->bitmap ;
	int         line_count ;
	Uint8       * ptr, * optr, * loptr ;
	Uint16	    remaining_width ;
	int         size;

	ptr        = flic->chunk->raw.data ;
	loptr      = bitmap->data ;
	line_count = bitmap->height ;

	while (line_count--)
	{
        optr = loptr;
		loptr += bitmap->pitch ;
        ptr++ ;
		remaining_width = bitmap->width ;

		while (remaining_width > 0) {
            size = *(Sint8 *)ptr++;
            if (size < 0) {
                size = -size;
				memcpy (optr, ptr, size) ;
				ptr += size ;
			} else if (size > 0) {
				memset (optr, *ptr++, size) ;
			}
			optr += size ;
			remaining_width -= size ;
		}
	}

	return flic ;
}

static FLIC * flic_do_chunk (FLIC * flic)
{
	Uint32 y;

	/* Procesa el contenido del chunk actual */

	switch (flic->chunk->header.type)
	{
		case CHUNK_BLACK:
			for (y = 0 ; y < flic->bitmap->height ; y++)
			{
				memset ((Uint8 *)flic->bitmap->data + flic->bitmap->pitch * y, 0, flic->bitmap->pitch) ;
			}
			break ;

		case CHUNK_FLI_COPY:
			for (y = 0 ; y < flic->bitmap->height ; y++)
			{
				memcpy ((Uint8 *)flic->bitmap->data + flic->bitmap->pitch * y,
						flic->chunk->raw.data + flic->bitmap->width * y,
						flic->bitmap->width) ;
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

    		if (!file_read (flic->fp, flic->frame, sizeof(FLIC_FRAME))) {
    			flic_destroy (flic) ;
    			return 0 ;
    		}

    		if (flic->frame->type != CHUNK_FRAME &&
    		    flic->frame->type != CHUNK_PREFIX) {
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

    		/* Reserva la memoria necesaria y carga el chunk */

    		if (flic->frame_reserved < flic->frame->size)
    		{
    			flic->frame_reserved = flic->frame->size ;
    			flic->frame = (FLIC_FRAME *) realloc (flic->frame, flic->frame_reserved) ;

    			if (!flic->frame)
    			{
    				/* Error: sin memoria */
    				file_close (flic->fp) ;
    				free (flic) ;
    				return 0 ;
    			}
    		}

            /* If it's a prefix frame, skip it. */

    		if (flic->frame->size > sizeof(FLIC_FRAME)) {
        		if (!file_read (flic->fp, &flic->frame[1], flic->frame->size - sizeof(FLIC_FRAME)))
        		{
        			flic_destroy (flic) ;
        			return 0 ;
        		}
        	}
    	}
    	while (flic->frame->type != CHUNK_FRAME) ;

    	/* Procesa cada sub-chunk */

    	flic->chunk = (FLIC_CHUNK *) &flic->frame[1] ;

    	for (chunkno = 0 ; chunkno < flic->frame->chunks ; chunkno++)
    	{
    		if (!flic_do_chunk(flic))
    			return 0 ;

    		flic->chunk = (FLIC_CHUNK *) (((Uint8 *)flic->chunk) + flic->chunk->header.size) ;
    	}

    	flic->last_frame_ms += flic->speed_ms ;
    	return flic ;
	}
}

void flic_reset (FLIC * flic)
{
	flic->current_frame = 0 ;
	flic->finished = 0 ;
	file_seek (flic->fp, flic->header.oframe1, SEEK_SET) ;
}
