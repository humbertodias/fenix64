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
 * FILE        : mmx_hspan.c
 * DESCRIPTION : Accelerated MMX routines for horizontal texture span
 *               drawing (used by g_blit.c)
 *
 * HISTORY:      0.82 - First version 
 */

#include <SDL.h>

#include "fxi.h"

#if defined(_MSC_VER) && defined(_M_IX86)

/* Parameters for hspan_16to16_translucent */
extern Sint16 * ghost1;
extern Sint16 * ghost2;

/* Parameters for hspan_8to8_translucent */
extern Sint8 (* ghost8)[256][256];

/*
 *  FUNCTION : gr_draw_hspan_XXX
 *
 *  Draw a textures span line into a bitmap. Those functions
 *  represent the inner loop of the blitter, but in an
 *  unscaled, non-rotated case (for gr_blit). Texture/screen
 *  coordinates are already calculated in origin/dest pointers.
 *
 *  This file includes optimized Intel-MMX versions of those 
 *  functions, and compiles under Visual C++ 6.0
 *
 *  There is one version of this function for each bit depth
 *  and blend effect configuration
 *
 *  PARAMS : 
 *		dest			Destination pointer
 *		tex				Origin pointer
 *		pixels			Number of pixels to draw
 *		incs			Texture increment: must be 1 or -1
 *
 *  RETURN VALUE : 
 *      None
 *
 */

void MMX_draw_hspan_8to8_nocolorkey
	(Uint8 * scr, Uint8 * tex, int pixels, int incs)
{
	_asm
	{
		mov		esi, tex
		mov		edi, scr
		mov		edx, incs
		cmp		edx, 0
		jng		inverse

		mov		ecx, pixels
		sar		ecx, 3
		jz		last_pixels
		sal		edx, 3
		mov		eax, pixels
		and		eax, 07h
		mov		pixels, eax
main_loop:
		movq	mm0, [esi]
		add		esi, edx
		movq	[edi], mm0
		add		edi, 8
		dec		ecx
		jnz		main_loop
		jmp		last_pixels

inverse:

		mov		ecx, pixels
		sar		ecx, 3
		jz		last_pixels
		sal		edx, 3
		mov		eax, pixels
		and		eax, 07h
		mov		pixels, eax
inverse_loop:
		mov		eax, [esi-3]
		mov		ebx, [esi-7]
		sub		esi, 8
		rol		ax, 8
		rol		bx, 8
		add		edi, 8
		rol		eax, 16
		rol		ebx, 16
		rol		ax, 8
		rol		bx, 8
		dec		ecx
		mov		[edi-8], eax
		mov		[edi-4], ebx
		jnz		inverse_loop

last_pixels:
		mov		ecx, pixels
		test	ecx, ecx
		jz		ending
last_pixel_loop:
		mov		al, [esi]
		add		esi, incs
		mov		[edi], al
		inc		edi
		dec		ecx
		jnz		last_pixel_loop
ending:
		emms
	}
}

void MMX_draw_hspan_8to8_translucent
	(Uint8 * scr, Uint8 * tex, int pixels, int incs)
{
	int counter;

	_asm
	{
		mov		esi, tex
		mov		edi, scr
		mov		edx, pixels
		mov		ecx, incs
		add		ecx, ecx
		mov		incs, ecx
		xor		eax, eax
		xor		ebx, ebx
		sar		edx, 1
		jz		last_pixel
		mov		counter, edx
		mov		edx, [ghost8]
main_loop:
		mov		ax, [esi]
		mov		bx, [edi]
		xchg	al, bh
		add		esi, incs
		mov		ch, byte ptr [edx + eax]
		add		edi, 2
		mov		cl, byte ptr [edx + ebx]
		dec		counter
		mov		[edi-2], cx
		jnz		main_loop
last_pixel:
		mov		ecx, pixels
		test	ecx, 1
		jz		ending
		mov		edx, [ghost8]
		mov		ah, [esi]
		mov		al, [edi]
		mov		al, byte ptr [edx + eax]
		mov		[edi], al
ending:
	}
}

void MMX_draw_hspan_8to8
	(Uint8 * scr, Uint8 * tex, int pixels, int incs)
{
	_asm
	{
		mov		esi, tex
		mov		edi, scr
		mov		ecx, pixels
		mov		edx, incs
		add		edx, edx
		add		edx, edx
		sar		ecx, 2
		jz		last_pixel
		cmp		edx, 0
		jl		main_loop
		add		edx, edx
		sar		ecx, 1
		jz		last_pixel

mmx_loop:
		movq	mm0, [esi]
		pxor	mm2, mm2
		add		esi, 8
		movq	mm1, [edi]
		pcmpeqb mm2, mm0
		add		edi, 8
		pand	mm1, mm2
		pandn	mm2, mm0
		dec		ecx
		por		mm2, mm1
		movq	[edi-8], mm2
		jnz		mmx_loop
		mov		ecx, pixels
		and		ecx, 7
		jmp		last_pixel_loop

main_loop:
		mov		eax, [esi-3]
		mov		ebx, [edi]
		xchg	ah, al
		rol		eax, 16
		xchg	ah, al
		test	al, al
		jnz		next_pixel
		mov		al, bl
next_pixel:
		test	ah, ah
		jnz		next_pixel2
		mov		ah, bh
next_pixel2:
		ror		eax, 16
		ror		ebx, 16
		test	al, al
		jnz		next_pixel3
		mov		al, bl
next_pixel3:
		test	ah, ah
		jnz		next_step
		mov		ah, bh
next_step:
		ror		eax, 16
		mov		[edi], eax
		add		edi, 4
		add		esi, edx
		dec		ecx
		jnz		main_loop
		mov		ecx, pixels
		and		ecx, 03h
		jz		ending
		jmp		last_pixel_loop
last_pixel:
		mov		ecx, pixels
last_pixel_loop:
		dec		ecx
		jl		ending
		mov		al, [esi]
		inc		edi
		add		esi, incs
		test	al, al
		jz		last_pixel_loop
		mov		[edi-1], al
		jmp		last_pixel_loop
ending:
		emms
	}
}

void MMX_draw_hspan_16to16
	(Uint16 * scr, Uint16 * tex, int pixels, int incs)
{
	_asm
	{
		mov		esi, tex
		mov		edi, scr
		mov		ecx, pixels
		mov		edx, incs
		add		edx, edx
		sar		ecx, 1
		jz		last_pixel
		cmp		edx, 0
		jl		main_loop
		sal		edx, 2
		sar		ecx, 2
		jz		last_pixel

mmx_loop:
		movq	mm0, [esi]
		pxor	mm2, mm2
		movq	mm3, [esi+8]
		pxor	mm5, mm5
		add		esi, 16
		movq	mm1, [edi]
		pcmpeqw mm2, mm0
		movq	mm4, [edi+8]
		pcmpeqw mm5, mm3
		add		edi, 16
		pand	mm1, mm2
		pand	mm4, mm5
		pandn	mm2, mm0
		pandn	mm5, mm3
		dec		ecx
		por		mm2, mm1
		por		mm5, mm4
		movq	[edi-16], mm2
		movq	[edi-8], mm5
		jnz		mmx_loop
		mov		ecx, pixels
		and		ecx, 7
		jz		ending
		mov		edx, incs
		add		edx, edx
		jmp		last_pixel_loop

main_loop:
		mov		ax, [esi]
		mov		bx, [esi+edx]
		add		esi, edx
		test	ax, ax
		jz		next_pixel
		mov		[edi], ax
next_pixel:
		test	bx, bx
		jz		next_step
		mov		[edi+2], bx
next_step:
		add		edi, 4
		add		esi, edx
		dec		ecx
		jnz		main_loop
		mov		ecx, pixels
		and		ecx, 01h
		jz		ending
		mov		edx, incs
		add		edx, edx
		jmp		last_pixel_loop
last_pixel:
		mov		ecx, pixels
		mov		edx, incs
		add		edx, edx
last_pixel_loop:
		mov		ax, [esi]
		add		edi, 2
		add		esi, edx
		test	ax, ax
		jz		n
		mov		[edi-2], ax
n:		dec		ecx
		jnz		last_pixel_loop
ending:
		emms
	}
}

void MMX_draw_hspan_16to16_translucent
	(Uint16 * scr, Uint16 * tex, int pixels, int incs)
{
	_asm
	{
		mov		esi, tex
		mov		edi, scr
		mov		ecx, ghost1
		mov		edx, ghost2
		xor		eax, eax
		xor		ebx, ebx
		cmp		incs, 1
		jne		loop_backward

loop_forward:
		mov		ax, word ptr [esi]
		mov		bx, word ptr [edi]
		add		esi, 2
		test	ax, ax
		jz		n1
		mov		ax, [ecx+eax*2]
		add		edi, 2
		add		ax, [edx+ebx*2]
		dec		pixels
		mov		[edi-2], ax
		jnz		loop_forward
		jmp		ending

n1:		add		edi, 2
		dec		pixels
		jnz		loop_forward
		jmp		ending

loop_backward:
		mov		ax, word ptr [esi]
		mov		bx, word ptr [edi]
		sub		esi, 2
		test	ax, ax
		jz		n2
		mov		ax, [ecx+eax*2]
		add		edi, 2
		add		ax, [edx+ebx*2]
		dec		pixels
		mov		[edi-2], ax
		jnz		loop_backward
		jmp		ending

n2:		add		edi, 2
		dec		pixels
		jnz		loop_backward

ending:
	}
}

void MMX_draw_hspan_16to16_nocolorkey
	(Uint16 * scr, Uint16 * tex, int pixels, int incs)
{
	_asm
	{
		mov		esi, tex
		mov		edi, scr
		mov		ecx, pixels
		cmp		incs, 0
		jl		last_pixel
		cmp		ecx, 8
		jl		last_pixel
		sar		ecx, 3
		xor		edx, edx
main_loop:
		movq	mm0, [esi+edx]
		movq	mm1, [esi+edx+8]
		movq	[edi+edx], mm0
		movq	[edi+edx+8], mm1
		add		edx, 16
		dec		ecx
		jnz		main_loop
		mov		ecx, pixels
		and		ecx, 7
		jz		ending
		add		esi, edx
		add		edi, edx
last_pixel:
		mov		edx, incs
		add		edx, edx
last_pixel_loop:
		mov		ax, [esi]
		add		esi, edx
		mov		[edi], ax
		add		edi, 2
		dec		ecx
		jnz		last_pixel_loop
ending:
		emms
	}
}

#else

void MMX_draw_hspan_8to8_nocolorkey (Uint8 * scr, Uint8 * tex, int pixels, int incs)
{
	(void)scr; (void)tex; (void)pixels; (void)incs;
}

void MMX_draw_hspan_8to8_translucent (Uint8 * scr, Uint8 * tex, int pixels, int incs)
{
	(void)scr; (void)tex; (void)pixels; (void)incs;
}

void MMX_draw_hspan_8to8 (Uint8 * scr, Uint8 * tex, int pixels, int incs)
{
	(void)scr; (void)tex; (void)pixels; (void)incs;
}

void MMX_draw_hspan_16to16 (Uint16 * scr, Uint16 * tex, int pixels, int incs)
{
	(void)scr; (void)tex; (void)pixels; (void)incs;
}

void MMX_draw_hspan_16to16_translucent (Uint16 * scr, Uint16 * tex, int pixels, int incs)
{
	(void)scr; (void)tex; (void)pixels; (void)incs;
}

void MMX_draw_hspan_16to16_nocolorkey (Uint16 * scr, Uint16 * tex, int pixels, int incs)
{
	(void)scr; (void)tex; (void)pixels; (void)incs;
}

#endif
