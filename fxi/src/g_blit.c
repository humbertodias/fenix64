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
 *  Copyright ù 1999 Josù Luis Cebriùn Pagùe
 *  Copyright ù 2002 Fenix Team
 *
 */

/*
 * FILE        : g_blit.c
 * DESCRIPTION : New blitter (gr_rotated_blit functions)
 *
 * HISTORY:      0.82 - Performance/prec. improvements & MMX versions
 *               0.81 - First version of a new blitter
 */

#include "fxi.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

/* Matemùticas de punto fijo, basadas en Allegro */

#include "fmath.h"
#include <math.h>

/* Define some constants and structs used by the blitter */

typedef struct
{
    int x, y;
}
_POINT;

typedef struct
{
    float x, y;
}
_POINTF;

typedef struct
{
    int x;
    int y;
    float s;
    float t;
}
VERTEX;

typedef _POINTF VECTOR;

typedef void (DRAW_SPAN) (GRAPH*,GRAPH*,int,int,int,int,int,int,int);
typedef void (DRAW_HSPAN)(void *,void *,int,int);
typedef Uint16 (ADDITIVE_BLEND)(Uint16,Uint16);

#define HSPAN8_INIT    Uint8  *scr = (Uint8  *)vscr, *tex = (Uint8  *)vtex
#define HSPAN816_INIT  Uint16 *scr = (Uint16 *)vscr; Uint8 *tex = (Uint8 *)vtex
#define HSPAN16_INIT   Uint16 *scr = (Uint16 *)vscr, *tex = (Uint16 *)vtex


/* Conversion tables used by transparency/blending
 *
 * In 16 bits, this two lookup tables will be used as:
 *
 *      Dest_color = ghost1[screen_color] + ghost2[graphic_color]
 *
 * In transparency mode, both tables are the assigned to the ghostcolor
 * global table (a table that reduces all color components to half)
 */

Sint16  * ghost1;
Sint16  * ghost2;
Uint8   * ghost8;

Uint16 * pcolorequiv = NULL ;

/*
    Calculates additive blend value
*/
ADDITIVE_BLEND *ablend;
int bt;
#ifdef MMX_FUNCTIONS
Uint16 MMX_additive_blend(Uint16 A, Uint16 B){
    Uint8 m[4], n[4];
    if (!enable_16bits){
        A &= 0xFF ;
        m[0] = palette[A].r ;
        m[1] = palette[A].g ;
        m[2] = palette[A].b ;

        B &= 0xFF ;
        n[0] = palette[B].r ;
        n[1] = palette[B].g ;
        n[2] = palette[B].b ;
    }else{
        m[0] = ((A & screen->format->Rmask) >> screen->format->Rshift) ;
        m[1] = ((A & screen->format->Gmask) >> screen->format->Gshift) ;
        m[2] = ((A & screen->format->Bmask) >> screen->format->Bshift) ;
        m[0] <<= screen->format->Rloss ;
        m[1] <<= screen->format->Gloss ;
        m[2] <<= screen->format->Bloss ;

        n[0] = ((B & screen->format->Rmask) >> screen->format->Rshift) ;
        n[1] = ((B & screen->format->Gmask) >> screen->format->Gshift) ;
        n[2] = ((B & screen->format->Bmask) >> screen->format->Bshift) ;
        n[0] <<= screen->format->Rloss ;
        n[1] <<= screen->format->Gloss ;
        n[2] <<= screen->format->Bloss ;
    }

    if(!bt){
  #ifdef __GNUC__
        __asm__ __volatile__(
            "movd %0, %%mm0\n"
            "movd %1, %%mm1\n"
            "paddusb %%mm1, %%mm0\n"
            "movd %%mm0, %0\n"
            "emms\n"
            :
            : "m" (m), "m" (n)
            : "mm0", "mm1", "cc"
        );
  #else
        _asm{
            movd mm0,m
            movd mm1,n
            paddusb mm0,mm1
            movd m,mm0
            emms
        }
  #endif
    }else{
  #ifdef __GNUC__
        __asm__ __volatile__(
            "movd %0, %%mm0\n"
            "movd %1, %%mm1\n"
            "psubusb %%mm1, %%mm0\n"
            "movd %%mm0, %0\n"
            "emms\n"
            :
            : "m" (m), "m" (n)
            : "mm0", "mm1", "cc"
        );
  #else
        _asm{
            movd mm0,m
            movd mm1,n
            psubusb mm0,mm1
            movd m,mm0
            emms
        }
  #endif
    }
    if(enable_16bits)
        return gr_rgb(m[0],m[1],m[2]);
    else
        return gr_find_nearest_color(m[0],m[1],m[2]);
}
#endif

Uint16 additive_blend(Uint16 A, Uint16 B){
    Sint32 r,g,b,r2,g2,b2;

    gr_get_rgb(B,&r,&g,&b);
    gr_get_rgb(A,&r2,&g2,&b2);

    if(!bt){
        r+=r2;
        if(r>255)r=255;
        g+=g2;
        if(g>255)g=255;
        b+=b2;
        if(b>255)b=255;
    }else{
        r+=(r2-256);
        if(r<0)r=0;
        g+=(g2-256);
        if(g<0)g=0;
        b+=(b2-256);
        if(b<0)b=0;
    }
    if(enable_16bits)
        return gr_rgb(r,g,b);
    else
        return gr_find_nearest_color(r,g,b);

}


/* Routine to sort vertexes in y, x order */
static int compare_vertex_y (const void * a, const void * b)
{
    const VERTEX * va = a;
    const VERTEX * vb = b;

    if (va->y == vb->y)
        return va->x - vb->x;

    return va->y - vb->y;
}


/*
 *  FUNCTION : gr_draw_span_XXX
 *
 *  Draw a textures span line into a bitmap. Those functions
 *  represent the inner loop of the blitter.
 *
 *  This file includes unoptimized C versions of those functions
 *
 *  There is one version of this function for each bit depth
 *  and blend effect configuration
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      orig            Pointer to the graphic object to draw
 *      x, y            Pixel coordinates of the destination leftmost point
 *      pixels          Number of pixels to draw
 *      s, t            Texture coordinates of the leftmost point
 *      s2, t2          Texture coordinates of the rightmost point
 *
 *  RETURN VALUE :
 *      None
 *
 */

/*
void draw_span_1to1(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                    int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x/8 ;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16)/8 + (cs >> 16)/8;
        Uint8   mask = (0x80 >> ((cs >> 16)/8 & 7));

        *ptr |= (*tex & mask);

        if (ct%8==7) ptr++;

        cs += incs, ct += inct;
    }
}

void draw_span_1to1_nocolorkey(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                    int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x/8 ;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16)/8 + (cs >> 16)/8;
        Uint8   mask = (0x80 >> ((cs >> 16)/8 & 7));

        *ptr &= ~mask;
        *ptr |= (*tex & mask);

        if (ct%8==7) ptr++;

        cs += incs, ct += inct;
    }
}
*/

void draw_span_1to8(GRAPH * dest, GRAPH * orig, int x, int y, int pixels, int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x ;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16)/8;
        Uint8   mask = (0x80 >> ((cs >> 16) & 7));
        if (*tex & mask) *ptr++ = syscolor8; else ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to8_nocolorkey(GRAPH * dest, GRAPH * orig, int x, int y, int pixels, int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        *ptr++ = *tex;
        cs += incs, ct += inct;
    }
}

void draw_span_8to8_translucent(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                                int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = ghost8[(*tex << 8) + *ptr];
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to8(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                    int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = *tex;
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to8_ablend(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                            int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);

        if (*tex != 0) *ptr = (Uint8)ablend(*tex,*ptr);
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to8_tablend(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                            int s, int t, int incs, int inct)
{
    Uint8 * ptr = (Uint8 *)dest->data + dest->pitch*y + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = (Uint8)ablend(ghost8[(*tex << 8) + *ptr],*ptr);
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_1to16(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                    int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x ;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16)/8;
        Uint8   mask = (0x80 >> ((cs >> 16) & 7));
        if (*tex & mask) *ptr++ = syscolor16; else ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to16(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                    int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = pcolorequiv[*tex];
        ptr++;
        cs += incs, ct += inct;
    }
}


void draw_span_8to16_ablend (GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
     int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = ablend(pcolorequiv[*tex],*ptr);
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to16_tablend(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                            int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = ablend((Uint16)(ghost1[pcolorequiv[*tex]] + ghost2[*ptr]),*ptr);
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to16_translucent(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                                 int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        if (*tex != 0) *ptr = ghost1[pcolorequiv[*tex]] + ghost2[*ptr];
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_8to16_nocolorkey(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                                int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint8 * tex = (Uint8 *)orig->data + orig->pitch*(ct >> 16) + (cs >> 16);
        *ptr++ = pcolorequiv[*tex];
        cs += incs, ct += inct;
    }
}

void draw_span_16to16(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                      int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint16 * tex = (Uint16 *)orig->data + orig->pitch*(ct >> 16)/2 + (cs >> 16);
        if (*tex != 0) *ptr = *tex;
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_16to16_ablend(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                             int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint16 * tex = (Uint16 *)orig->data + orig->pitch*(ct >> 16)/2 + (cs >> 16);
        if (*tex != 0) *ptr = ablend(*tex,*ptr);
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_16to16_tablend(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                              int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint16 * tex = (Uint16 *)orig->data + orig->pitch*(ct >> 16)/2 + (cs >> 16);
        if (*tex != 0) *ptr = ablend((Uint16)(ghost1[*tex] + ghost2[*ptr]),*ptr);
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_16to16_translucent(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                                  int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint16 * tex = (Uint16 *)orig->data + orig->pitch*(ct >> 16)/2 + (cs >> 16);
        if (*tex != 0) *ptr = ghost1[*tex] + ghost2[*ptr];
        ptr++;
        cs += incs, ct += inct;
    }
}

void draw_span_16to16_nocolorkey(GRAPH * dest, GRAPH * orig, int x, int y, int pixels,
                                 int s, int t, int incs, int inct)
{
    Uint16 * ptr = (Uint16 *)dest->data + dest->pitch*y/2 + x;
    int cs = s, ct = t, i;

    for (i = 0 ; i < pixels ; i++)
    {
        Uint16 * tex = (Uint16 *)orig->data + orig->pitch*(ct >> 16)/2 + (cs >> 16);
        *ptr++ = *tex;
        cs += incs, ct += inct;
    }
}

/*
 *  FUNCTION : gr_draw_hspan_XXX
 *
 *  Draw a textures span line into a bitmap. Those functions
 *  represent the inner loop of the blitter, but in an
 *  unscaled, non-rotated case (for gr_blit). Texture/screen
 *  coordinates are already calculated in origin/dest pointers.
 *
 *  This file includes unoptimized C versions of those functions
 *
 *  There is one version of this function for each bit depth
 *  and blend effect configuration
 *
 *  PARAMS :
 *      dest            Destination pointer
 *      tex             Origin pointer
 *      pixels          Number of pixels to draw
 *      incs            Texture increment: must be 1 or -1
 *
 *  RETURN VALUE :
 *      None
 *
 */

/* Parameter for 1to8 and 16to8 */
static int posx;

void draw_hspan_1to8 (void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
    int i;
    int mask = (0x80 >> (posx & 7));

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex & mask) *scr++ = syscolor8; else scr++;
        if (incs < 0) { if (mask == 0x80) mask = 0x01, tex--; else mask <<= 1; }
        else          { if (mask == 0x01) mask = 0x80, tex++; else mask >>= 1; }
    }
}

void draw_hspan_8to8_nocolorkey(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        *scr++ = *tex;
        tex += incs;
    }
}

void draw_hspan_8to8_translucent(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = ghost8[(*tex << 8) + *scr];
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to8_tablend(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = (Uint8)ablend(ghost8[(*tex << 8) + *scr],*scr);
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to8_ablend(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = (Uint8)ablend(*tex, *scr);
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to8(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN8_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = *tex;
        scr++;
        tex += incs;
    }
}

void draw_hspan_1to16(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN816_INIT;
    int i;
    int mask = (0x80 >> (posx & 7));

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex & mask) *scr++ = syscolor16; else scr++;
        if (incs < 0) { if (mask == 0x80) mask = 0x01, tex--; else mask <<= 1; }
        else          { if (mask == 0x01) mask = 0x80, tex++; else mask >>= 1; }
    }
}

void draw_hspan_8to16(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN816_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = pcolorequiv[*tex];
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to16_ablend(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN816_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = ablend(pcolorequiv[*tex], *scr);
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to16_tablend(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN816_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = ablend((Uint16)(ghost1[pcolorequiv[*tex]] + ghost2[*scr]),*scr);
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to16_translucent(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN816_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = ghost1[pcolorequiv[*tex]] + ghost2[*scr];
        scr++;
        tex += incs;
    }
}

void draw_hspan_8to16_nocolorkey(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN816_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        *scr++ = pcolorequiv[*tex];
        tex += incs;
    }
}

void draw_hspan_16to16(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex) *scr = *tex;
        scr++ ;
        tex += incs;
    }
}

void draw_hspan_16to16_ablend(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex) *scr = ablend(*tex, *scr);
        scr++ ;
        tex += incs;
    }
}

void draw_hspan_16to16_tablend(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = ablend((Uint16)(ghost1[*tex] + ghost2[*scr]),*scr);
        scr++;
        tex += incs;
    }
}


void draw_hspan_16to16_translucent(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        if (*tex != 0) *scr = ghost1[*tex] + ghost2[*scr];
        scr++;
        tex += incs;
    }
}

void draw_hspan_16to16_nocolorkey(void * vscr, void * vtex, int pixels, int incs)
{
    HSPAN16_INIT;
    int i;

    for (i = 0 ; i < pixels ; i++)
    {
        *scr++ = *tex;
        tex += incs;
    }
}

/*
 *  FUNCTION : gr_calculate_corners
 *
 *  Calculate the screen coordinates of the corners of a graphic
 *  when rotated and scaled in an specific angle
 *
 *  PARAMS :
 *      graph           Pointer to the graphic object
 *      x, y            Pixel coordinates of the center on screen
 *      angle           Angle of rotation in miliangles
 *      scalex, scaley  Scaling ratio in percentaje (100 for original size)
 *      corners         Pointer to the output array of 4 points
 *
 *  RETURN VALUE :
 *      The screen coordinates of the points will be written
 *      into the "corners" array
 *
 */

void gr_calculate_corners (GRAPH * dest, int screen_x, int screen_y, int flags,
                          int angle, int scalex, int scaley, _POINTF * corners)
{
    float center_x, center_y;
    float top_y, bot_y;
    float lef_x, rig_x;
    float scalexf, scaleyf;
    float cos_angle = (float) cos(angle * M_PI / -180000.0);
    float sin_angle = (float) sin(angle * M_PI / -180000.0);

    /* Adjust size to prevent integer conversion errors */

    if (scalex < 0)
        scalex = 0;
    if (scalex > 100 && scalex < 250)
        scalex  = ((scalex - 4) & ~3) + 6;

    if (scaley < 0)
        scaley = 0;
    if (scaley > 100 && scaley < 250)
        scaley  = ((scaley - 4) & ~3) + 6;

    scalexf = scalex / 100.0 ;
    scaleyf = scaley / 100.0 ;

    /* Calculate the graphic center */

    if (dest->ncpoints && dest->cpoints[0].x != CPOINT_UNDEFINED)
    {
        center_x = dest->cpoints[0].x;
        center_y = dest->cpoints[0].y;

        if (flags & B_HMIRROR)
            center_x = dest->width - 1 - center_x;
        if (flags & B_VMIRROR)
            center_y = dest->height - 1 - center_y;
    }
    else
    {
        center_x = dest->width / 2.0;
        center_y = dest->height / 2.0;
    }

    /* Calculate the non-rotated non-translated coordinates */

    lef_x = - center_x * scalexf;
    rig_x = + (dest->width - center_x) * scalexf - 1;
    top_y = - center_y * scaleyf;
    bot_y = + (dest->height - center_y) * scaleyf - 1;

    /* Rotate and scale the coordinates */

    /* Top-left, top-right, bottom-left, bottom-right */

    corners[0].x = screen_x  + (cos_angle * lef_x - sin_angle * top_y) + 0.5 ;
    corners[0].y = screen_y  + (sin_angle * lef_x + cos_angle * top_y) + 0.5 ;
    corners[1].x = screen_x  + (cos_angle * rig_x - sin_angle * top_y) + 0.5 ;
    corners[1].y = screen_y  + (sin_angle * rig_x + cos_angle * top_y) + 0.5 ;

    corners[2].x = screen_x  + (cos_angle * lef_x - sin_angle * bot_y) + 0.5 ;
    corners[2].y = screen_y  + (sin_angle * lef_x + cos_angle * bot_y) + 0.5 ;
    corners[3].x = screen_x  + (cos_angle * rig_x - sin_angle * bot_y) + 0.5 ;
    corners[3].y = screen_y  + (sin_angle * rig_x + cos_angle * bot_y) + 0.5 ;

}

/*
 *  FUNCTION : gr_get_bbox
 *
 *  Calculate the bounding box of a graphic, when drawing it with
 *  the gr_rotated_blit function
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      scrx, scry      Pixel coordinates of the center on screen
 *      angle           Angle of rotation in miliangles
 *      scalex, scaley  Scaling ratio in percentaje (100 for original size)
 *      gr              Pointer to the graphic object to draw
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_get_bbox (REGION * dest, REGION * clip, int x, int y, int flags,
                  int angle, int scalex, int scaley, GRAPH * gr)
{
    _POINTF  corners[4];
    _POINT   min, max;
    int     i;

    /* Calculate the coordinates of each corner in the graphic */

    gr_calculate_corners (gr, x, y, flags, angle, scalex, scaley, corners);

    /* Calculate the bounding box */

    min.x = max.x = corners[0].x;
    min.y = max.y = corners[0].y;

    for (i = 1 ; i < 4 ; i++)
    {
        if (min.x > corners[i].x) min.x = (int)corners[i].x;
        if (max.x < corners[i].x) max.x = (int)corners[i].x;
        if (min.y > corners[i].y) min.y = (int)corners[i].y;
        if (max.y < corners[i].y) max.y = (int)corners[i].y;
    }

    dest->x  = min.x;
    dest->y  = min.y;
    dest->x2 = max.x;
    dest->y2 = max.y;

}

/*
 *  FUNCTION : gr_rotated_blit
 *
 *  Draw a rotated and/or scaled bitmap
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      scrx, scry      Pixel coordinates of the center on screen
 *      angle           Angle of rotation in miliangles
 *      scalex, scaley  Scaling ratio in percentaje (100 for original size)
 *      gr              Pointer to the graphic object to draw
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_rotated_blit  (GRAPH * dest, REGION * clip,
                       int scrx, int scry, int flags,
                       int angle, int scalex, int scaley,
                       GRAPH * gr)
{
    _POINTF  corners[4];
    _POINT   min, max;
    VERTEX  vertex[4];
    int     i;
    float   half_texel_size;

    /* Data for the left line */
    int     left_steps;
    int     left_pos;
    VECTOR  left_texture_increment;
    VERTEX* left_start;
    VERTEX* left_end;

    /* Data for the right line */
    int     right_steps;
    int     right_pos;
    VECTOR  right_texture_increment;
    VERTEX* right_start;
    VERTEX* right_end;

    /* Pointer to the line drawing function */
    DRAW_SPAN *draw_span = NULL;

    if (!dest) dest = scrbitmap;
    if (!dest->data || !gr->data) return;
    if (scalex <= 0) return;
    if (scaley <= 0) return;

    /* Update the parameters if the graphic is animated */

	if(gr->current_keyframe >= 0) {
        angle += gr->keyframes[gr->current_keyframe].angle;
        flags ^= gr->keyframes[gr->current_keyframe].flags;
    }

    /* Calculate the coordinates of each corner in the graphic */

    gr_calculate_corners (gr, scrx, scry, flags, angle, scalex, scaley, corners);

    /* Calculate the clipping coordinates */

    if (clip)
    {
        min.x = MAX(clip->x, 0 );
        min.y = MAX(clip->y, 0 );
        max.x = MIN(clip->x2, (int)dest->width - 1);
        max.y = MIN(clip->y2, (int)dest->height - 1);
    }
    else
    {
        min.x = 0;
        min.y = 0;
        max.x = (int)dest->width - 1;
        max.y = (int)dest->height - 1;
    }

    /* The texture coordinates of each corner point are displaced
       to the center of the texel to sidestep precision errors */

    half_texel_size = 50.0 / scalex;

    /* Fill the vertex array with the four obtained points */

    for (i = 0; i < 4; i++)
    {
        vertex[i].x = (int)(corners[i].x);
        vertex[i].y = (int)(corners[i].y);
    }
    if (flags & B_HMIRROR)
    {
        vertex[1].s = vertex[3].s = half_texel_size ;
        vertex[0].s = vertex[2].s = gr->width - half_texel_size;
    }
    else
    {
        vertex[0].s = vertex[2].s = half_texel_size;
        vertex[1].s = vertex[3].s = gr->width - half_texel_size;
    }
    if (flags & B_VMIRROR)
    {
        vertex[2].t = vertex[3].t = half_texel_size;
        vertex[0].t = vertex[1].t = gr->height - half_texel_size;
    }
    else
    {
        vertex[0].t = vertex[1].t = half_texel_size;
        vertex[2].t = vertex[3].t = gr->height - half_texel_size;
    }

    /* Sort the vertex list in y coordinate order */

    qsort (vertex, 4, sizeof(VERTEX), compare_vertex_y);

    /* Analize the bitmap if needed (find if no color key used */

    if (gr->modified) bitmap_analize (gr) ;

    if (gr->info_flags & GI_NOCOLORKEY) flags |= B_NOCOLORKEY ;

    /* Setup the 16 bits translucency tables if necessay */

    if (gr->blend_table)
    {
        ghost1 = gr->blend_table ;
        ghost2 = gr->blend_table + 65536 ;
        flags |= B_TRANSLUCENT ;
    }
    else if (flags & B_ALPHA)
    {
        if (dest->depth == 16)
        {
            ghost1 = gr_alpha16((flags & B_ALPHA_MASK) >> B_ALPHA_SHIFT);
            ghost2 = gr_alpha16(255-((flags & B_ALPHA_MASK) >> B_ALPHA_SHIFT));
        }
        else if (dest->depth == 8)
        {
            ghost8 = gr_alpha8((flags & B_ALPHA_MASK) >> B_ALPHA_SHIFT);
        }
        flags |= B_TRANSLUCENT ;
    }
    else if (flags & B_TRANSLUCENT)
    {
        ghost1 = ghost2 = colorghost ;
        ghost8 = (Uint8 *)trans_table ;
    }
/*
    if ((flags & B_TRANSLUCENT) && !trans_table_updated) gr_make_trans_table() ;
*/
    #ifdef MMX_FUNCTIONS
    if (MMX_available) {
        ablend = MMX_additive_blend;
    }else{
    #endif
    ablend = additive_blend;
    #ifdef MMX_FUNCTIONS
    }
    #endif

    /* Choose a line drawing function */

    if (dest->depth == 8 && gr->depth == 8)
    {
        if (flags & B_TRANSLUCENT){
            if(flags & B_ABLEND){
                bt=0;
                draw_span = draw_span_8to8_tablend;
            }else if(flags & B_SBLEND){
                bt=1;
                draw_span = draw_span_8to8_tablend;
            }else
                draw_span = draw_span_8to8_translucent;
        }
        else if (flags & B_ABLEND){
            bt=0;
            draw_span = draw_span_8to8_ablend;
        }else if (flags & B_SBLEND){
            bt=1;
            draw_span = draw_span_8to8_ablend;
        }
        else if (flags & B_NOCOLORKEY)
            draw_span = draw_span_8to8_nocolorkey;
        else
            draw_span = draw_span_8to8;
    }
    else if (dest->depth == 16 && gr->depth == 8)
    {
        pcolorequiv = gr->palette ? gr->palette->colorequiv : colorequiv ;

        if (flags & B_TRANSLUCENT){
            if(flags & B_ABLEND){
                bt=0;
                draw_span = draw_span_8to16_tablend;
           }else if(flags & B_SBLEND){
                bt=1;
                draw_span = draw_span_8to16_tablend;
            }else
                draw_span = draw_span_8to16_translucent;
        }
        else if (flags & B_ABLEND){
            bt=0;
            draw_span = draw_span_8to16_ablend;
        }else if (flags & B_SBLEND){
            bt=1;
            draw_span = draw_span_8to16_ablend;
        }
        else if (flags & B_NOCOLORKEY)
            draw_span = draw_span_8to16_nocolorkey;
        else
            draw_span = draw_span_8to16;
    }
    else if (dest->depth == 16 && gr->depth == 16)
    {
        if (flags & B_TRANSLUCENT){
            if(flags & B_ABLEND){
                bt=0;
                draw_span = draw_span_16to16_tablend;
            }else if(flags & B_SBLEND){
                bt=1;
                draw_span = draw_span_16to16_tablend;
            }else
                draw_span = draw_span_16to16_translucent;
        }
        else if (flags & B_ABLEND){
            bt=0;
            draw_span = draw_span_16to16_ablend;
        }else if (flags & B_SBLEND){
            bt=1;
            draw_span = draw_span_16to16_ablend;
        }
        else if (flags & B_NOCOLORKEY)
            draw_span = draw_span_16to16_nocolorkey;
        else
            draw_span = draw_span_16to16;
    }
    else if (dest->depth == 8 && gr->depth == 1)
        draw_span = draw_span_1to8;
    else if (dest->depth == 16 && gr->depth == 1)
        draw_span = draw_span_1to16;
    else
        gr_error ("Profundidad de color no soportada\n(gr_rotated_blit)") ;

    /* Fix por problemas de visualizacion */

    if ((vertex[0].y>>1) == (vertex[1].y>>1)) {
        if ( (vertex[0].x > vertex[1].x && vertex[2].x < vertex[3].x) ||
             (vertex[0].x < vertex[1].x && vertex[2].x > vertex[3].x) ) {
            VERTEX  aux = vertex[0];
            vertex[0]=vertex[1];
            vertex[1]=aux;
        }
    }

    if ((vertex[2].y>>1) == (vertex[3].y>>1)) {
        if ( (vertex[0].x > vertex[1].x && vertex[2].x < vertex[3].x) ||
             (vertex[0].x < vertex[1].x && vertex[2].x > vertex[3].x) ) {
            VERTEX  aux = vertex[2];
            vertex[2]=vertex[3];
            vertex[3]=aux;
        }
    }

    /* Create the first two left and right line segments */

    left_start = &vertex[0];
    left_end   = (vertex[1].x < vertex[2].x)? &vertex[1] : &vertex[2];

    right_start = &vertex[0];
    right_end   = (vertex[1].x < vertex[2].x) ? &vertex[2] : &vertex[1];
    if ((right_start->y>>1) == (right_end->y>>1))
    {
        right_start = right_end;
        right_end   = &vertex[3];
    }

    left_steps = left_end->y - left_start->y;
    left_texture_increment.x  = (left_end->s - left_start->s)  / (float)(left_steps);
    left_texture_increment.y  = (left_end->t - left_start->t)  / (float)(left_steps);

    right_steps = right_end->y - right_start->y;
    right_texture_increment.x = (right_end->s - right_start->s) / (float)(right_steps);
    right_texture_increment.y = (right_end->t - right_start->t) / (float)(right_steps);

    left_pos = -1;
    right_pos = -1;

    /* Draw the graphic a line each time, navigating through the
     * left and right line segments until reaching the graphic bottom */

    for (i = vertex[0].y ; i <= vertex[3].y ; i++)
    {
        int   x, x2;
        float s, t, s2, t2;

        left_pos++;
        right_pos++;

        /* Calculate the left point coordinates in screen and texture */
        x  = left_start->x;
        s  = left_start->s;
        t  = left_start->t;
        if (left_pos > 0 && left_steps > 0)
        {
            x += (left_end->x - left_start->x) * left_pos / left_steps;
            s += left_texture_increment.x * left_pos ;
            t += left_texture_increment.y * left_pos ;
        }

        /* Calculate the right point coordinates in screen and texture */
        x2 = right_start->x;
        s2 = right_start->s;
        t2 = right_start->t;
        if (right_pos > 0 && right_steps > 0)
        {
            x2 += (right_end->x - right_start->x) * right_pos / right_steps;
            s2 += right_texture_increment.x * right_pos ;
            t2 += right_texture_increment.y * right_pos ;
        }

        /* We don't calculate y/y2 coordinate positions, because they
           should be equal to i. However, precision errors may prevent it. */

        /* Clip the resulting coordinates */
        if (i >= min.y && i <= max.y)
        {
            if (x < min.x && x2 >= x)
            {
                s += (min.x-x) * ((s2-s) / (x2-x+1));
                t += (min.x-x) * ((t2-t) / (x2-x+1));
                x  = min.x;
            }
            if (x2 > max.x && x2 >= x)
            {
                s2 -= (x2-max.x) * ((s2-s) / (x2-x+1));
                t2 -= (x2-max.x) * ((t2-t) / (x2-x+1));
                x2  = max.x;
            }

            /* Draw the resulting line */
            if (x2 > x)
                draw_span (dest, gr, x, i, x2-x+1,
                           (int)(s * 65536), (int)(t * 65536),
                           (int)((s2-s)/(x2-x) * 65536),
                           (int)((t2-t)/(x2-x) * 65536));
        }

        /* If there is no more steps in the left vector,
         * advance to the next segment */
        if (left_pos == left_steps)
        {
            left_start = left_end;
            left_end   = &vertex[3];
            left_pos   = 0;
            left_steps = left_end->y - left_start->y;
            if (left_steps > 1) {
                left_texture_increment.x = (left_end->s - left_start->s)  / (float)(left_steps);
                left_texture_increment.y = (left_end->t - left_start->t)  / (float)(left_steps);
            }
        }

        /* Same for the right vector */

        if (right_pos == right_steps)
        {
            right_start = right_end;
            right_end   = &vertex[3];
            right_pos   = 0;
            right_steps = right_end->y - right_start->y;
            if (right_steps > 1) {
                right_texture_increment.x = (right_end->s - right_start->s) / (float)(right_steps);
                right_texture_increment.y = (right_end->t - right_start->t) / (float)(right_steps);
            }
        }
    }

    dest->modified = 1 ;

}

/*
 *  FUNCTION : gr_blit
 *
 *  Draw a bitmap (with no rotation or scaling, but with flags & clipping)
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      scrx, scry      Pixel coordinates of the center on screen
 *      gr              Pointer to the graphic object to draw
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_blit (GRAPH * dest, REGION * clip, int scrx, int scry, int flags, GRAPH * gr)
{
    _POINT   min, max;
    _POINT   center;
    int     x, y, s, t, p, l, i;
    void *  tex;
    void *  scr;
    int     tex_inc;
    int     scr_inc;
    int     direction;

    DRAW_HSPAN  * draw_hspan = NULL;

    if (!dest) dest = scrbitmap ;
    if (!dest->data || !gr->data) return;

    /* Update the parameters if the graphic is animated */

	if(gr->current_keyframe >= 0) {
        flags ^= gr->keyframes[gr->current_keyframe].flags;
    }

    /* Calculate the clipping coordinates */

    if (clip)
    {
        min.x = MAX(clip->x, 0);
        min.y = MAX(clip->y, 0);
        max.x = MIN(clip->x2, (int)dest->width - 1);
        max.y = MIN(clip->y2, (int)dest->height - 1);
    }
    else
    {
        min.x = 0;
        min.y = 0;
        max.x = dest->width - 1;
        max.y = dest->height - 1;
    }

    /* Analize the bitmap if needed (find if no color key used */

    if (gr->modified) bitmap_analize (gr) ;

    if (gr->info_flags & GI_NOCOLORKEY) flags |= B_NOCOLORKEY ;

    /* Setup the 16 bits translucency tables if necessay */

    if (gr->blend_table)
    {
        ghost1 = gr->blend_table ;
        ghost2 = gr->blend_table + 65536 ;
        flags |= B_TRANSLUCENT ;
    }
    else if (flags & B_ALPHA)
    {
        if (dest->depth == 16)
        {
            ghost1 = gr_alpha16((flags & B_ALPHA_MASK) >> B_ALPHA_SHIFT);
            ghost2 = gr_alpha16(255-((flags & B_ALPHA_MASK) >> B_ALPHA_SHIFT));
        }
        else if (dest->depth == 8)
        {
            ghost8 = gr_alpha8((flags & B_ALPHA_MASK) >> B_ALPHA_SHIFT);
        }
        flags |= B_TRANSLUCENT ;
    }
    else if (flags & B_TRANSLUCENT)
    {
        ghost1 = ghost2 = colorghost ;
        ghost8 = (Uint8 *)trans_table ;
    }

/*
    if ((flags & B_TRANSLUCENT) && !trans_table_updated) gr_make_trans_table() ;
*/
    #ifdef MMX_FUNCTIONS
    if (MMX_available) {
        ablend = MMX_additive_blend;
    }else{
    #endif
    ablend = additive_blend;
    #ifdef MMX_FUNCTIONS
    }
    #endif

    /* Choose a line drawing function */

    if (dest->depth == 8 && gr->depth == 8)
    {
        if (flags & B_TRANSLUCENT){
            if(flags & B_ABLEND){
                bt=0;
                draw_hspan = draw_hspan_8to8_tablend;
            }else if(flags & B_SBLEND){
                bt=1;
                draw_hspan = draw_hspan_8to8_tablend;
            }else {
                #ifdef MMX_FUNCTIONS
                if (MMX_available){
                    draw_hspan = MMX_draw_hspan_8to8_translucent;
                } else {
                #endif
                draw_hspan = draw_hspan_8to8_translucent;
                #ifdef MMX_FUNCTIONS
                }
                #endif
            }
        }
        else if(flags & B_ABLEND){
            bt=0;
            draw_hspan = draw_hspan_8to8_ablend;
        }else if (flags & B_SBLEND){
            bt=1;
            draw_hspan = draw_hspan_8to8_ablend;
        }
        else if (flags & B_NOCOLORKEY) {
            #ifdef MMX_FUNCTIONS
            if (MMX_available){
                draw_hspan = MMX_draw_hspan_8to8_nocolorkey;
            } else {
            #endif
            draw_hspan = draw_hspan_8to8_nocolorkey;
            #ifdef MMX_FUNCTIONS
            }
            #endif
        }
        else {
            #ifdef MMX_FUNCTIONS
            if (MMX_available){
                draw_hspan = MMX_draw_hspan_8to8;
            } else {
            #endif
            draw_hspan = draw_hspan_8to8;
            #ifdef MMX_FUNCTIONS
            }
            #endif
        }
    }
    else if (dest->depth == 16 && gr->depth == 8)
    {
        pcolorequiv = gr->palette ? gr->palette->colorequiv : colorequiv ;

        if (flags & B_TRANSLUCENT){
            if(flags & B_ABLEND){
                bt=0;
                draw_hspan = draw_hspan_8to16_tablend;
            }else if(flags & B_SBLEND){
                bt=1;
                draw_hspan = draw_hspan_8to16_tablend;
            }else
                draw_hspan = draw_hspan_8to16_translucent;
        }
        else if(flags & B_ABLEND){
            bt=0;
            draw_hspan = draw_hspan_8to16_ablend;
        }else if (flags & B_SBLEND){
            bt=1;
            draw_hspan = draw_hspan_8to16_ablend;
        }
        else if (flags & B_NOCOLORKEY)
            draw_hspan = draw_hspan_8to16_nocolorkey;
        else
            draw_hspan = draw_hspan_8to16;
    }
    else if (dest->depth == 16 && gr->depth == 16)
    {
        if (flags & B_TRANSLUCENT){
            if(flags & B_ABLEND){
                bt=0;
                draw_hspan = draw_hspan_16to16_tablend;
            }else if(flags & B_SBLEND){
                bt=1;
                draw_hspan = draw_hspan_16to16_tablend;
            }else{
                draw_hspan = draw_hspan_16to16_translucent;
            }
        }
        else if(flags & B_ABLEND){
            bt=0;
            draw_hspan = draw_hspan_16to16_ablend;
        }else if (flags & B_SBLEND){
            bt=1;
            draw_hspan = draw_hspan_16to16_ablend;
        }
        else if (flags & B_NOCOLORKEY) {
            #ifdef MMX_FUNCTIONS
            if (MMX_available){
                draw_hspan = MMX_draw_hspan_16to16_nocolorkey;
            } else {
            #endif
            draw_hspan = draw_hspan_16to16_nocolorkey;
            #ifdef MMX_FUNCTIONS
            }
            #endif
        } else {
            #ifdef MMX_FUNCTIONS
            if (MMX_available){
                draw_hspan = MMX_draw_hspan_16to16;
            } else {
            #endif
            draw_hspan = draw_hspan_16to16;
            #ifdef MMX_FUNCTIONS
            }
            #endif
        }
    }
    else if (dest->depth == 8 && gr->depth == 1)
        draw_hspan = draw_hspan_1to8;
    else if (dest->depth == 16 && gr->depth == 1)
        draw_hspan = draw_hspan_1to16;
    else
        gr_error ("Profundidad de color no soportada\n(gr_blit)") ;

    /* Calculate the graphic center */

    if (gr->ncpoints && gr->cpoints[0].x != CPOINT_UNDEFINED)
    {
        center.x = gr->cpoints[0].x ;
        center.y = gr->cpoints[0].y ;

        if (flags & B_HMIRROR)
            center.x = gr->width - 1 - center.x;
        if (flags & B_VMIRROR)
            center.y = gr->height - 1 - center.y;
    }
    else
    {
        center.x = gr->width / 2.0;
        center.y = gr->height / 2.0;
    }

    /* Calculate the initial texture and screen coordinates */

    x = scrx - center.x;
    y = scry - center.y;
    p = gr->width;
    l = gr->height;
    s = t = 0;

    /* Clip the coordinates */

    if (y < min.y)
    {
        l -= min.y - y;
        t += min.y - y;
        y  = min.y;
    }
    if (y+l-1 > max.y)
    {
        l -= y + l - 1 - max.y ;
    }
    if (x < min.x)
    {
        p -= min.x - x;
        s += min.x - x;
        x  = min.x;
    }
    if (x+p > max.x)
    {
        p -= x + p - 1 - max.x;
    }

    /* Mirror the texture coordinates if needed */

    if (flags & B_HMIRROR) s = gr->width - 1 - s;
    if (flags & B_VMIRROR) t = gr->height - 1 - t;

    /* Calculate the initial pointers and advances */

    posx      = s;
    scr       = (Uint8 *)dest->data + dest->pitch*y + x*dest->depth/8 ;
    tex       = (Uint8 *)gr->data + gr->pitch*t + s*gr->depth/8 ;
    scr_inc   = dest->pitch ;
    tex_inc   = gr->pitch ;
    direction = 1;

    if (flags & B_VMIRROR) tex_inc = -tex_inc;
    if (flags & B_HMIRROR) direction = -1;

    if (p > 0)
    {
        for (i = 0; i < l; i++)
        {
            (*draw_hspan) (scr, tex, p, direction);
            scr = (Uint8 *)scr + scr_inc;
            tex = (Uint8 *)tex + tex_inc;
        }
    }

    dest->modified = 1 ;
}

