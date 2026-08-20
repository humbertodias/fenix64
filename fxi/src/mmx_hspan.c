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
 * FILE        : mmx_hspan.c
 * DESCRIPTION : Accelerated MMX routines for horizontal texture span
 *               drawing (used by g_blit.c)
 *
 * HISTORY:      0.82 - First version
 */

#include "fxi.h"

//#define DEBUGMMX

#ifdef MMX_FUNCTIONS

/* Parameters for hspan_16to16_translucent */
extern Sint16 * ghost1;
extern Sint16 * ghost2;

/* Parameters for hspan_8to8_translucent */
extern Sint8 (* ghost8)[256][256];

#define HSPAN8_INIT    Uint8  *scr = (Uint8  *)vscr, *tex = (Uint8  *)vtex
#define HSPAN16_INIT   Uint16 *scr = (Uint16 *)vscr, *tex = (Uint16 *)vtex

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

void MMX_draw_hspan_8to8_nocolorkey(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
#ifdef DEBUGMMX
    printf("MMX_draw_hspan_8to8_nocolorkey\n");
    fflush(stdout);
#endif

  #ifdef __GNUC__
    __asm__ __volatile__(
//        "movl %1, %%esi \n"           //      mov  esi, tex
//        "movl %0, %%edi \n"           //      mov  edi, scr
        "movl %3, %%edx \n"           //      mov  edx, incs
        "cmp $0, %%edx \n"            //      cmp edx, 0
        "jng 1f \n"                   //      jng inverse
        "movl %2, %%ecx \n"           //      mov  ecx, pixels
        "sar $3, %%ecx \n"            //      sar ecx, 3
        "jz 3f \n"                    //      jz  last_pixels
        "sal $3, %%edx \n"            //      sal edx, 3
        "movl %2, %%eax \n"           //      mov  eax, pixels
        "and $0x7, %%eax \n"          //      and eax, 07h
        "movl %%eax, %2 \n"           //      mov  pixels, eax
     "0: \n" // main_loop             //  main_loop:
        "movq 0(%%esi), %%mm0 \n"     //      movq    mm0, [esi]
        "add %%edx, %%esi \n"         //      add esi, edx
        "movq 0(%%edi), %%mm0 \n"     //      movq    [edi], mm0
        "add $8, %%edi \n"            //      add edi, 8
        "dec %%ecx \n"                //      dec ecx
        "jnz 0b \n"                   //      jnz main_loop
        "jmp 3f \n"                   //      jmp last_pixels
   "1: \n" // inverse                 //  inverse:
        "movl %2, %%ecx \n"           //      mov ecx, pixels
        "sar $3, %%ecx \n"            //      sar ecx, 3
        "jz 3f \n"                    //      jz  last_pixels
        "sal $3, %%edx \n"            //      sal edx, 3
        "movl %2, %%eax \n"           //      mov  eax, pixels
        "and $0x7, %%eax \n"          //      and eax, 07h
        "movl %%eax, %2 \n"           //      mov  pixels, eax
  "2: \n" // inverse_loop             //  inverse_loop:
        "movl -3(%%esi), %%eax \n"    //      mov  eax, [esi-3]
        "movl -7( %%esi), %%ebx \n"   //      mov  ebx, [esi-7]
        "sub $8, %%esi \n"            //      sub esi, 8
        "rol $8, %%ax \n"             //      rol ax, 8
        "rol $8, %%bx \n"             //      rol bx, 8
        "add $8, %%edi \n"            //      add edi, 8
        "rol $16, %%eax \n"           //      rol eax, 16
        "rol $16, %%ebx \n"           //      rol ebx, 16
        "rol $8, %%ax \n"             //      rol ax, 8
        "rol $8, %%bx \n"             //      rol bx, 8
        "dec %%ecx \n"                //      dec ecx
        "movl %%eax, -8(%%edi) \n"    //      mov  [edi-8], eax
        "movl %%ebx, -4( %%edi ) \n"  //      mov  [edi-4], ebx
        "jnz 2b \n"                   //      jnz inverse_loop
  "3: \n" // last_pixels              //  last_pixels:
        "movl %2, %%ecx \n"           //      mov  ecx, pixels
        "test %%ecx, %%ecx \n"        //      test    ecx, ecx
        "jz 5f \n"                    //      jz  ending
  "4: \n" // last_pixel_loop          //  last_pixel_loop:
        "movb (%%esi), %%al \n"       //      mov  al, [esi]
        "add %3, %%esi \n"            //      add esi, incs
        "movb %%al, ( %%edi) \n"      //      mov [edi], al
        "inc %%edi \n"                //      inc edi
        "dec %%ecx \n"                //      dec ecx
        "jnz 4b \n"                   //      jnz last_pixel_loop
 "5: \n" // ending:                   //  ending:
        "emms \n"                     //      emms
        :
        : "D" (scr), "S" (tex), "m" (pixels), "m" (incs)
        : "mm0", "mm1", "cc" // "ax", "esi", "edi", "edx", "ecx", "eax", "memory"
    );
  #else
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
  #endif
}

void MMX_draw_hspan_8to8_translucent(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
	int counter;
#ifdef DEBUGMMX
    printf("MMX_draw_hspan_8to8_translucent\n");
    fflush(stdout);
#endif

#ifdef __GNUC__
    __asm__ __volatile__(
//        "movl %1, %%esi \n"             //        mov     esi, tex
//        "movl %0, %%edi \n"             //        mov     edi, scr
        "movl %2, %%edx \n"             //        mov     edx, pixels
        "movl %3, %%ecx \n"             //        mov     ecx, incs
        "add %%ecx, %%ecx \n"           //        add     ecx, ecx
        "movl %%ecx, %3 \n"             //        mov     incs, ecx
        "xor %%eax, %%eax \n"           //        xor     eax, eax
        "xor %%ebx, %%ebx \n"           //        xor     ebx, ebx
        "sar $1, %%edx \n"              //        sar     edx, 1
        "jz 0f \n"                      //        jz      last_pixel
        "movl %5, %%edx \n"             //        mov     counter, edx
        "movl (%4), %%edx \n"           //        mov     edx, [ghost8]
  "1: \n" // main_loop                  //    main_loop:
        "movw (%%esi), %%ax \n"         //        mov     ax, [esi]
        "movw (%%edi), %%bx \n"         //        mov     bx, [edi]
        "xchg %%bh, %%al \n"            //        xchg    al, bh
        "add %3, %%esi \n"              //        add     esi, incs
        "movb  (%%edx ,%%eax), %%ch \n" //        mov     ch, byte ptr [edx + eax]
        "add $2, %%edi \n"              //        add     edi, 2
        "movb  (%%edx ,%%ebx), %%cl \n" //        mov     cl, byte ptr [edx + ebx]
        "decl %5 \n"                    //        dec     counter
        "movw %%cx, -2( %%edi) \n"      //        mov     [edi-2], cx
        "jnz 1b \n"                     //        jnz     main_loop
    "0: \n" // last_pixel               //    last_pixel:
        "movl %2,  %%ecx \n"            //        mov     ecx, pixels
        "test  $1,  %%ecx \n"           //        test    ecx, 1
        "jz 2f \n"                      //        jz      ending
        "movl ( %4 ), %%edx \n"         //        mov     edx, [ghost8]
        "movb ( %%esi), %%ah \n"        //        mov     ah, [esi]
        "movb ( %%edi ), %%al \n"       //        mov     al, [edi]
        "movb (%%edx, %%eax), %%al  \n" //        mov     al, byte ptr [edx + eax]
        "movb   %%al,  ( %%edi )  \n"   //        mov     [edi], al
   "2: \n" // ending:                   //    ending:
        :
        : "D" (scr), "S" (tex), "m" (pixels), "m" (incs), "m" (ghost8), "m" (counter)
        : "cc" // "ax", "bx","esi", "edi", "edx", "ecx", "eax", "memory"
    );
#else
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
#endif
}

void MMX_draw_hspan_8to8(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
#ifdef DEBUGMMX
    printf( "MMX_draw_hspan_8to8\n");
    fflush(stdout);
#endif

  #ifdef __GNUC__
    __asm__ __volatile__(
//        "movl %1, %%esi  \n"             //  mov     esi, tex
//        "movl %0, %%edi \n"              //  mov     edi, scr
        "movl %2, %%ecx \n"              //  mov     ecx, pixels
        "movl %3, %%edx \n"              //  mov     edx, incs
        "add %%edx, %%edx\n"             //  add     edx, edx
        "add %%edx, %%edx\n"             //  add     edx, edx
        "sar $2, %%ecx \n"               //  sar     ecx, 2
        "jz 0f \n"                       //  jz      last_pixel
        "cmp $0, %%edx \n"               //  cmp     edx, 0
        "jl 3f \n"                       //  jl      main_loop
        "add %%edx, %%edx \n"            //  add     edx, edx
        "sar $1, %%ecx \n"               //  sar     ecx, 1
        "jz 0f \n"                       //  jz      last_pixel
    "1: \n" //mmx_loop:                  //mmx_loop:
        "movq (%%esi), %%mm0 \n"         //  movq    mm0, [esi]
        "pxor   %%mm2, %%mm2 \n"         //  pxor    mm2, mm2
        "add $8, %%esi \n"               //  add     esi, 8
        "movq (%%edi), %%mm1 \n"         //  movq    mm1, [edi]
        "pcmpeqb %%mm2,  %%mm0 \n"       //  pcmpeqb mm2, mm0
        "add $8, %%edi \n"               //  add     edi, 8
        "pand %%mm2, %%mm1 \n"           //  pand    mm1, mm2
        "pandn %%mm0,  %%mm2 \n"         //  pandn   mm2, mm0
        "dec %%ecx \n"                   //  dec     ecx
        "por %%mm1, %%mm2 \n"            //  por     mm2, mm1
        "movq %%mm2, -8(%%edi) \n"       //  movq    [edi-8], mm2
        "jnz 1b \n"                      //  jnz     mmx_loop
        "movl %2, %%ecx \n"              //  mov     ecx, pixels
        "and $7, %%ecx \n"               //  and     ecx, 7
        "jmp 2f \n"                      //  jmp     last_pixel_loop
    "3: \n" //main_loop:                 //main_loop:
        "movl -3(%%esi), %%eax  \n"      //  mov     eax, [esi-3]
        "movl (%%edi), %%ebx \n"         //  mov     ebx, [edi]
        "xchg %%ah, %%al \n"             //  xchg    ah, al
        "rol $16, %%eax  \n"             //  rol     eax, 16
        "xchg %%al, %%ah \n"             //  xchg    ah, al
        "test %%al, %%al \n"             //  test    al, al
        "jnz 4f \n"                      //  jnz     next_pixel
        "movb %%bl, %%al \n"             //  mov     al, bl
    "4: \n" //next_pixel:                //next_pixel:
        "test %%ah, %%ah \n"             //  test    ah, ah
        "jnz 5f \n"                      //  jnz     next_pixel2
        "movb %%bh, %%ah \n"              //  mov     ah, bh
    "5: \n" //next_pixel2:               //next_pixel2:
        "ror $16, %%eax \n"              //  ror     eax, 16
        "ror $16, %%ebx \n"              //  ror     ebx, 16
        "test %%al, %%al \n"             //  test    al, al
        "jnz 6f \n"                      //  jnz     next_pixel3
        "movb %%bl, %%al \n"              //  mov     al, bl
    "6: \n" //next_pixel3:               //next_pixel3:
        "test %%ah, %%ah \n"             //  test    ah, ah
        "jnz 7f \n"                      //  jnz     next_step
        "movb %%bh, %%ah \n"              //  mov     ah, bh
    "7: \n"  //next_step:                //next_step:
        "ror $16,   %%eax \n"            //  ror     eax, 16
        "movl %%eax, (%%edi)  \n"         //  mov     [edi], eax
        "add $4, %%edi \n"               //  add     edi, 4
        "add %%edx, %%esi \n"            //  add     esi, edx
        "dec %%ecx \n"                   //  dec     ecx
        "jnz 3b \n"                      //  jnz     main_loop
        "movl %2, %%ecx \n"              //  mov     ecx, pixels
        "and $0x3,  %%ecx \n"            //  and     ecx, 03h
        "jz 8f \n"                       //  jz      ending
        "jmp 2f \n"                      //  jmp     last_pixel_loop
    "0: \n"  //last_pixel:               //last_pixel:
        "movl %2, %%ecx \n"              //  mov     ecx, pixels
    "2: \n"  //last_pixel_loop:          //last_pixel_loop:
        "dec %%ecx \n"                   //  dec     ecx
        "jl 8f \n"                       //  jl      ending
        "movb (%%esi), %%al \n"          //  mov     al, [esi]
        "inc %%edi \n"                   //  inc     edi
        "add %3, %%esi \n"               //  add     esi, incs
        "test %%al, %%al \n"             //  test    al, al
        "jz 2b \n"                       //  jz      last_pixel_loop
        "movb %%al, -1(%%edi) \n"        //  mov     [edi-1], al
        "jmp 2b \n"                      //  jmp     last_pixel_loop
     "8: \n" //ending:                   //ending:
         "emms \n"                       //  emms
        :
        : "D" (scr), "S" (tex), "m" (pixels), "m" (incs)
        :  "mm0", "mm1", "cc" // "ax", "bx", "esi", "edi", "edx", "ecx", "eax", "memory"
    );
  #else
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
	#endif
}

void MMX_draw_hspan_16to16(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
#ifdef DEBUGMMX
    printf( "MMX_draw_hspan_16to16\n");
    fflush(stdout);
#endif

  #ifdef __GNUC__
    __asm__ __volatile__(
//        "movl  %1,%%esi \n"               //    mov     esi, tex
//        "movl  %0, %%edi \n"              //    mov     edi, scr
        "movl  %2, %%ecx \n"              //    mov     ecx, pixels
        "movl  %3, %%edx \n"             //    mov     edx, incs
        "add    %%edx ,%%edx \n"     //    add     edx, edx
        "sar  $1, %%ecx \n"                //    sar     ecx, 1
        "jz 4f \n"                         //    jz      last_pixel
        "cmp $0, %%edx \n"                 //    cmp     edx, 0
        "jl 1f \n"                         //    jl      main_loop
        "sal $2,%%edx  \n"                 //    sal     edx, 2
        "sar    $2, %%ecx  \n"             //    sar     ecx, 2
        "jz 4f \n"                         //    jz      last_pixel
    "0: \n" // mmx_loop                    //mmx_loop:
        "movq (%%esi), %%mm0   \n"         //    movq    mm0, [esi]
        "pxor %%mm2, %%mm2 \n"             //    pxor    mm2, mm2
        "movq 8(%%esi), %%mm3  \n"         //    movq    mm3, [esi+8]
        "pxor %%mm5, %%mm5 \n"             //    pxor    mm5, mm5
        "add  $16, %%esi \n"               //    add     esi, 16
        "movq (%%edi), %%mm1  \n"          //    movq    mm1, [edi]
        "pcmpeqw %%mm0, %%mm2 \n"          //    pcmpeqw mm2, mm0
        "movq 8(%%edi), %%mm4   \n"        //    movq    mm4, [edi+8]
        "pcmpeqw %%mm3,  %%mm5 \n"         //    pcmpeqw mm5, mm3
        "add $16, %%edi   \n"              //    add     edi, 16
        "pand %%mm2, %%mm1 \n"             //    pand    mm1, mm2
        "pand %%mm5, %%mm4 \n"             //    pand    mm4, mm5
        "pandn %%mm0, %%mm2 \n"            //    pandn   mm2, mm0
        "pandn %%mm3, %%mm5 \n"            //    pandn   mm5, mm3
        "dec %%ecx \n"                     //    dec     ecx
        "por %% mm1, %%mm2 \n"             //    por     mm2, mm1
        "por %%mm4, %%mm5 \n"              //    por     mm5, mm4
        "movq %%mm2, -16(%%edi)  \n"       //    movq    [edi-16], mm2
        "movq %%mm5, -8(%%edi)  \n"        //    movq    [edi-8], mm5
        "jnz    0b \n"                     //    jnz     mmx_loop
        "movl %2, %%ecx  \n"               //    mov     ecx, pixels
        "and $7, %%ecx  \n"                //    and     ecx, 7
        "jz 7f \n"                         //    jz      ending
        "movl  %3, %%edx  \n"              //    mov     edx, incs
        "add %%edx, %%edx \n"              //    add     edx, edx
        "jmp 5f \n"                        //    jmp     last_pixel_loop
    "1: \n" // main_loop                   //main_loop:
        "movw (%%esi),  %%ax   \n"         //    mov     ax, [esi]
        "movw (%%esi,%%edx), %%bx   \n"    //    mov     bx, [esi+edx]
        "add %%edx, %%esi  \n"             //    add     esi, edx
        "test %%ax, %%ax \n"               //    test    ax, ax
        "jz 2f \n"                         //    jz      next_pixel
        "movw  %%ax, (%%edi)  \n"          //    mov     [edi], ax
    "2: \n" // next_pixel                  //next_pixel:
        "test %%bx , %%bx \n"              //    test    bx, bx
        "jz 3f \n"                         //    jz      next_step
        "movw   %%bx,   2(%%edi) \n"       //    mov     [edi+2], bx
    "3: \n" // next_step                   //next_step:
        "add  $4, %%edi  \n"               //    add     edi, 4
        "add %%edx,  %%esi \n"             //    add     esi, edx
        "dec %%ecx \n"                     //    dec     ecx
        "jnz 1b \n"                        //    jnz     main_loop
        "movl %2, %%ecx  \n"               //    mov     ecx, pixels
        "and $0x01,    %%ecx  \n"          //    and     ecx, 01h
        "jz 7f \n"                         //    jz      ending
        "movl %3, %%edx \n"                //    mov     edx, incs
        "add %%edx,  %%edx \n"             //    add     edx, edx
        "jmp 5f \n"                        //   jmp     last_pixel_loop
    "4: \n" // last_pixel                  //last_pixel:
        "movl %2, %%ecx \n"                //    mov     ecx, pixels
        "movl %3, %%edx \n"                //    mov     edx, incs
        "add  %%edx , %%edx \n"            //    add     edx, edx
    "5: \n" // last_pixel_loop             //last_pixel_loop:
        "movw (%%esi), %%ax   \n"          //    mov     ax, [esi]
        "add  $2, %%edi  \n"               //    add     edi, 2
        "add  %%edx, %%esi   \n"           //    add     esi, edx
        "test %%ax , %%ax \n"              //    test    ax, ax
        "jz 6f \n"                         //    jz      n
        "movw  %%ax, -2(%%edi) \n"         //    mov     [edi-2], ax
    "6: \n" // n:                          //n:
        "dec %%ecx \n"                     //    dec     ecx
        "jnz 5b \n"                        //    jnz     last_pixel_loop
      "7: \n" // ending                    //ending:
          "emms \n"                        //    emms
        :
        : "D" (scr), "S" (tex), "m" (pixels), "m" (incs)
        : "mm0", "mm1", "cc" // "ax", "bx", "esi", "edi", "edx", "ecx", "eax", "memory"
    );
  #else
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
	#endif
}


void MMX_draw_hspan_16to16_translucent(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
#ifdef DEBUGMMX
    printf("MMX_draw_hspan_16to16_translucent\n");
    fflush(stdout);
#endif

#ifdef __GNUC__
    __asm__ __volatile__(
//        "movl  %1, %%esi \n"                    //    mov     esi, tex
//        "movl   %0, %%edi \n"                   //    mov     edi, scr
        "movl %4, %%ecx  \n"                    //    mov     ecx, ghost1
        "movl %5, %%edx \n"                     //    mov     edx, ghost2
        "xor %%eax, %%eax \n"                   //    xor     eax, eax
        "xor %%ebx, %%ebx \n"                   //    xor     ebx, ebx
        "cmp $1, %3 \n"                         //    cmp incs, 1
        "jne 2f  \n"                            //    jne     loop_backward
    "0: \n" // loop_forward                     //loop_forward:
        "movw (%%esi), %%ax  \n"                //    mov     ax, word ptr [esi]
        "movw (%%edi), %%bx \n"                 //    mov     bx, word ptr [edi]
        "add $2, %%esi \n"                      //    add     esi, 2
        "test %%ax, %%ax \n"                    //    test    ax, ax
        "jz 1f \n"                              //    jz      n1
        "movw (%%ecx,%%eax,2), %%ax  \n"        //    mov     ax, [ecx+eax*2]
        "add $2, %%edi  \n"                     //    add     edi, 2
        "add (%%edx,%%ebx,2), %%ax  \n"         //    add     ax, [edx+ebx*2]
        "decl %2 \n"                            //    dec     pixels
        "movw  %%ax, -2(%%edi) \n"              //    mov     [edi-2], ax
        "jnz 0b \n"                             //    jnz     loop_forward
        "jmp 4f  \n"                            //    jmp     ending
    "1: \n" // n1:                              //n1:
        "add  $2, %%edi \n"                     //    add     edi, 2
        "decl %2 \n"                            //    dec     pixels
        "jnz 0b \n"                             //    jnz     loop_forward
        "jmp 4f \n"                             //    jmp     ending
    "2: \n" // loop_backward                    //loop_backward:
        "movw (%%esi), %%ax  \n"                //    mov     ax, word ptr [esi]
        "movw (%%edi), %%bx \n"                 //    mov     bx, word ptr [edi]
        "sub $2, %%esi  \n"                     //    sub     esi, 2
        "test %%ax, %%ax \n"                    //    test    ax, ax
        "jz 3f \n"                              //    jz      n2
        "movw (%%ecx,%%eax,2), %%ax  \n"        //    mov     ax, [ecx+eax*2]
        "add $2, %%edi  \n"                     //    add     edi, 2
        "add (%%edx,%%ebx,2), %%ax  \n"         //    add     ax, [edx+ebx*2]
        "decl %2 \n"                            //    dec     pixels
        "movw %%ax, -2(%%edi) \n"               //    mov     [edi-2], ax
        "jnz 2b \n"                             //    jnz     loop_backward
        "jmp 4f \n"                             //    jmp     ending
   "3: \n" // n2:                               //n2:
        "add $2, %%edi  \n"                     //        add     edi, 2
        "decl %2 \n"                            //    dec     pixels
        "jnz 2b \n"                             //    jnz     loop_backward
   "4: \n" // ending                            //ending:
        :
        : "D" (scr), "S" (tex), "m" (pixels), "m" (incs), "m" (ghost1), "m" (ghost2)
        :  "cc" // "ax", "bx", "esi", "edi", "edx", "ecx", "eax", "memory"
    );
#else
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
#endif
}

void MMX_draw_hspan_16to16_nocolorkey(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
#ifdef DEBUGMMX
    printf("MMX_draw_hspan_16to16_nocolorkey\n");
    fflush(stdout);
#endif

  #ifdef __GNUC__
    __asm__ __volatile__(
//        "movl  %1, %%esi \n"                    //    mov     esi, tex
//        "movl   %0, %%edi \n"                   //    mov     edi, scr
        "movl %2, %%ecx \n"                 //      mov ecx, pixels
        "cmp $0, %3 \n"                     //      cmp incs, 0
        "jl 1f \n"                          //      jl  last_pixel
        "cmp $8, %%ecx \n"                  //      cmp ecx, 8
        "jl 1f \n"                          //      jl  last_pixel
        "sar $3, %%ecx \n"                  //      sar ecx, 3
        "xor %%edx, %%edx \n"               //      xor edx, edx
    "0: \n" // main_loop                    //   main_loop:
        "movq (%%esi,%%edx), %%mm0 \n"      //       movq    mm0, [esi+edx]
        "movq 8(%%esi,%%edx), %%mm1  \n"    //       movq    mm1, [esi+edx+8]
        "movq %%mm0, (%%edi,%%edx) \n"      //       movq    [edi+edx], mm0
        "movq %%mm1, 8(%%edi,%%edx) \n"     //       movq    [edi+edx+8], mm1
        "add $16, %%edx \n"                 //       add edx, 16
        "dec %%ecx \n"                      //       dec ecx
        "jnz 0b \n"                         //       jnz main_loop
        "movl %2, %%ecx \n"                 //       mov ecx, pixels
        "and $7, %%ecx \n"                  //       and ecx, 7
        "jz 3f \n"                          //       jz  ending
        "add %%edx, %%esi \n"               //       add esi, edx
        "add %%edx, %%edi \n"               //       add edi, edx
    "1: \n" // last_pixel                   //   last_pixel:
        "movl %3, %%edx \n"                 //       mov edx, incs
        "add %%edx, %%edx \n"               //       add edx, edx
    "2: \n" // last_pixel_loop              //   last_pixel_loop:
        "movw (%%esi), %%ax  \n"            //       mov ax, [esi]
        "add %%edx, %%esi \n"               //       add esi, edx
        "movw %%ax, (%%edi)  \n"            //       mov [edi], ax
        "add $2, %%edi \n"                  //       add edi, 2
        "dec %%ecx \n"                      //       dec ecx
        "jnz 2b \n"                         //       jnz last_pixel_loop
    "3: \n" // ending:                      //   ending:
        "emms \n" // emms                   //       emms
        :
        : "D" (scr), "S" (tex), "m" (pixels), "m" (incs)
        : "mm0", "mm1", "cc" // "ax", "esi", "edi", "edx", "ecx", "eax", "memory"
    );
  #else
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
    #endif
}

#endif
