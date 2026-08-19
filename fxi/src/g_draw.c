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

#include <string.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "fxi.h"

#include "files.h"
#include "grlib.h"

int      syscolor8 = 15 ;
Uint16   syscolor16 = 0xFFFF ;
Uint16   syscolor16_alpha = 0xFFFF ;
int      drawing_alpha = 255;
Uint16 * drawing_alpha16 = NULL ;
Uint8  * drawing_alpha8 = NULL ;
Uint32   drawing_stipple = 0xFFFFFFFF;

DRAWING_OBJECT * drawing_objects = NULL;

#ifdef __GNUC__
#define _inline inline
#endif

_inline void _HLine8_nostipple (Uint8 * ptr, Uint32 length)
{
    if (drawing_alpha == 255)
    {
        memset (ptr, syscolor8, length) ;
    }
    else
    {
        register int n;

        for (n = length; n; n--, ptr++) *ptr = drawing_alpha8[(syscolor8 << 8) + *ptr];
    }
}

_inline void _HLine8_stipple (Uint8 * ptr, Uint32 length)
{
    register int n;

    if (drawing_alpha == 255)
    {
        for (n = length; n; n--, ptr++)
        {
            if (drawing_stipple & 1) *ptr = syscolor8;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
        }
    }
    else
    {
        for (n = length; n; n--, ptr++)
        {
            if (drawing_stipple & 1) *ptr = drawing_alpha8[(syscolor8 << 8) + *ptr];
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
        }
    }
}

_inline void _HLine16_nostipple (Uint16 * ptr, Uint32 length)
{
    register int n ;

    if (drawing_alpha == 255)
    {
        for (n = length; n; n--) *ptr++ = syscolor16 ;
    }
    else
    {
        for (n = length; n; n--, ptr++) *ptr = drawing_alpha16[*ptr] + syscolor16_alpha ;
    }
}

_inline void _HLine16_stipple (Uint16 * ptr, Uint32 length)
{
    register int n ;

    if (drawing_alpha == 255)
    {
        for (n = length; n; n--, ptr++)
        {
            if (drawing_stipple & 1) *ptr = syscolor16 ;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
        }
    }
    else
    {
        for (n = length; n; n--, ptr++)
        {
            if (drawing_stipple & 1) *ptr = drawing_alpha16[*ptr] + syscolor16_alpha ;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
        }
    }
}

_inline void _Pixel8 (Uint8 * ptr, int color)
{
    if (drawing_alpha == 255)
        *ptr = color ;
    else
        *ptr = drawing_alpha8[(color << 8) + *ptr];
}

_inline void _Pixel16 (Uint16 * ptr, Uint16 color, Uint16 color_alpha)
{
    if (drawing_alpha == 255)
        *ptr = color ;
    else
        *ptr = drawing_alpha16[*ptr] + color_alpha;
}

/*
 *  FUNCTION : gr_get_pixel
 *
 *  Read a pixel from a bitmap
 *
 *  PARAMS :
 *      dest            Destination bitmap
 *      x, y            Pixel coordinates
 *
 *  RETURN VALUE :
 *      1, 8 or 16-bit integer with the pixel value
 *
 */

int gr_get_pixel (GRAPH * dest, int x, int y)
{
    if (x < 0 || y < 0 || x >= (int)dest->width || y >= (int)dest->height) return -1 ;

    switch (dest->depth)
    {
        case 8:
            return ((Uint8 *)dest->data) [x + dest->pitch * y] ;
        case 16:
            return ((Uint16 *)dest->data)[x + dest->pitch * y / 2] ;
        case 1:
            return (((Uint8 *)dest->data)[x / 8 + dest->pitch * y] & (0x80 >> (x & 7))) ? 1:0;
        default:
            gr_error ("gr_get_pixel: Profundidad de color no soportada");
            return 0;
    }
}

/*
 *  FUNCTION : gr_put_pixel
 *
 *  Paint a pixel with no clipping whatsoever, except by
 *  the bitmap's dimensions
 *
 *  PARAMS :
 *      dest            Destination bitmap
 *      x, y            Pixel coordinates
 *      color           1, 8 or 16-bit pixel value
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_put_pixel (GRAPH * dest, int x, int y, int color)
{
    if (x < 0 || y < 0 || x >= (int)dest->width || y >= (int)dest->height) return ;

    dest->modified = 1 ;

    switch (dest->depth)
    {
        case 8:
            _Pixel8(((Uint8 *)dest->data)+ x + dest->pitch * y, color);
            break;

        case 16:
            _Pixel16(((Uint16 *)dest->data) + x + dest->pitch * y / 2, color, gr_alpha16(drawing_alpha)[color]) ;
            break;

        case 1:
            if (color)
                (((Uint8 *)dest->data)[x / 8 + dest->pitch * y]) |= (0x80 >> (x & 7)) ;
            else
                (((Uint8 *)dest->data)[x / 8 + dest->pitch * y]) &= ~(0x80 >> (x & 7)) ;
            break;

        default:
            gr_error ("gr_put_pixel: Profundidad de color no soportada");
    }
}

/*
 *  FUNCTION : gr_put_pixel
 *
 *  Paint a pixel with no clipping whatsoever, using
 *  a clipping region
 *
 *  PARAMS :
 *      dest            Destination bitmap
 *      x, y            Pixel coordinates
 *      color           1, 8 or 16-bit pixel value
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_put_pixelc (GRAPH * dest, REGION * clip, int x, int y, int color)
{
    if (clip && x >= clip->x && x <= clip->x2 && y >= clip->y && y <= clip->y2)
        gr_put_pixel (dest, x, y, color);

    dest->modified = 1;
}

/*
 *  FUNCTION : gr_clear
 *
 *  Clear a bitmap (paint all pixels as 0 [transparent])
 *
 *  PARAMS :
 *      dest            Bitmap to clear
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_clear (GRAPH * dest)
{
    dest->modified = 1 ;

    memset (dest->data, 0, dest->pitch * dest->height) ;

    if (dest == background) background_is_black = 1;
}

/*
 *  FUNCTION : gr_clear_as
 *
 *  Clear a bitmap (paint all pixels as the given color)
 *
 *  PARAMS :
 *      dest            Bitmap to clear
 *      color           8 or 16-bit color value
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_clear_as (GRAPH * dest, int color)
{
    Uint32 y;

    dest->modified = 1 ;

    switch (dest->depth)
    {
        case 8:
        {
            memset (dest->data, color, dest->pitch * dest->height) ;
/*
            Uint8 * data = dest->data ;
            for (y = 0; y < dest->height; y++) {
                memset (data, color, dest->pitch) ;
                data += dest->pitch ;
            }
*/
            break;
        }

        case 16:
        {
            Uint8 * data = dest->data ;
            for (y = 0; y < dest->height; y++)
            {
                Uint16 * ptr = (Uint16 *)data ;
                int n ;
                for (n = 0; n < dest->width; n++) *ptr++ = color ;
                data += dest->pitch ;
            }
            break;
        }

        case 1:
        {
            int c = color ? 0xFF : 0 ;
            memset (dest->data, c, dest->pitch * dest->height) ;
/*
            Uint8 * data = dest->data ;
            int c = color ? 0xFF : 0 ;
            for (y = 0; y < dest->height; y++) {
                memset (data, c, dest->pitch) ;
                data += dest->pitch ;
            }
*/
            break;
        }

        default:
            gr_error ("gr_clear_as: Profundidad de color no soportada");
    }

    if (dest == background && !color) background_is_black = 1;
}

/*
 *  FUNCTION : gr_clear_region
 *
 *  Clear a region bitmap (paint all pixels as 0 [transparent])
 *
 *  PARAMS :
 *      dest            Bitmap to clear
 *      region          Region to clear or NULL for the whole screen
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_clear_region (GRAPH * dest, REGION * region)
{
    REGION base_region ;
    int y, n, l ;

    if (!dest) dest = scrbitmap ;
    dest->modified = 1 ;

    if (!region)
    {
        region = &base_region ;
        region->x = 0 ;
        region->y = 0 ;
        region->x2 = dest->width - 1 ;
        region->y2 = dest->height - 1 ;
    }
    else
    {
        base_region = *region ;
        region = &base_region ;
        region->x = MAX(MIN(region->x, region->x2), 0) ;
        region->y = MAX(MIN(region->y, region->y2), 0) ;
        region->x2 = MIN(MAX(region->x, region->x2), dest->width - 1) ;
        region->y2 = MIN(MAX(region->y, region->y2), dest->height - 1) ;
    }

    switch (dest->depth)
    {
        case 8:
        {
            Uint8 * data = ((Uint8 *)dest->data) + dest->pitch * region->y + region->x ;
            l = region->x2 - region->x + 1;
            for (y = region->y; y <= region->y2; y++) {
                memset (data, 0, l) ;
                data += dest->pitch ;
            }

            break ;
        }

        case 16:
        {
            Uint8 * data = ((Uint8 *)dest->data) + dest->pitch * region->y + region->x * 2;
            l = region->x2 - region->x + 1;
            for (y = region->y; y <= region->y2; y++) {
                Uint16 * ptr = (Uint16 *)data ;
                for (n = 0; n < l; n++) *ptr++ = 0 ;
                data += dest->pitch ;
            }
            break ;
        }

        case 1:
        {
            Uint8 * data = ((Uint8 *)dest->data) + region->x / 8 ;
            l = (region->x2 - region->x - 1) / 8 + 1 ;
            for (y = region->y; y <= region->y2; y++) {
                /* Esta debe ser cambiada, por bits */
                memset (data, 0, l) ;
                data += dest->pitch ;
            }
            break ;
        }
    }

}

/*
 *  FUNCTION : gr_vline
 *
 *  Draw a vertical rectangle
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the top-left pixel
 *      h               Height in pixels
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_vline (GRAPH * dest, REGION * clip, int x, int y, int h)
{
    REGION base_clip ;
    int old_stipple = drawing_stipple;

    if (!dest) dest = scrbitmap ;
    dest->modified = 1 ;

    if (!clip)
    {
        clip = &base_clip ;
        clip->x = 0 ;
        clip->y = 0 ;
        clip->x2 = dest->width - 1 ;
       clip->y2 = dest->height - 1 ;
    }

    if (h < 0) h = -h, y -= (h - 1) ;
    if (x < clip->x || x > clip->x2) return ;

    if (y < clip->y) h += y - clip->y, y = clip->y;
    if (y + h > clip->y2) h = clip->y2 - y + 1 ;

    if (h < 0) return ;

    if (dest->depth == 8)
    {
        Uint8 * ptr = dest->data ;
        ptr += dest->pitch * y + x ;
        if (drawing_stipple != 0xFFFFFFFF)
        {
            while (h--)
            {
                if (drawing_stipple & 1) _Pixel8 (ptr, syscolor8) ;
                drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
                ptr += dest->pitch ;
            }
        }
        else
        {
            while (h--)
            {
                _Pixel8 (ptr, syscolor8) ;
                ptr += dest->pitch ;
            }
        }
    }
    else if (dest->depth == 16)
    {
        Uint16 * ptr = dest->data ;
        int inc = dest->pitch / 2 ;
        ptr += inc * y + x ;
        if (drawing_stipple != 0xFFFFFFFF)
        {
            while (h--)
            {
                if (drawing_stipple & 1) _Pixel16 (ptr, syscolor16, syscolor16_alpha) ;
                drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
                ptr += inc ;
            }
        }
        else
        {
            while (h--)
            {
                _Pixel16 (ptr, syscolor16, syscolor16_alpha) ;
                ptr += inc ;
            }
        }
    }
    else if (dest->depth == 1)
    {
        Uint8 * ptr = dest->data;
        int mask ;
        ptr += dest->pitch * y + x / 8;
        mask = (1 << (7-(x & 7)));
        if (drawing_stipple != 0xFFFFFFFF)
        {
            while (h--)
            {
                if (drawing_stipple & 1) {
                    if (!syscolor8)
                        *ptr &= ~mask;
                    else
                        *ptr |= mask;
                }
                drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
                ptr += dest->pitch ;
            }
        } else {
            while (h--)
            {
                if (!syscolor8)
                    *ptr &= ~mask;
                else
                    *ptr |= mask;
                ptr += dest->pitch ;
            }
        }
    }

    drawing_stipple = old_stipple;

}

/*
 *  FUNCTION : gr_gline
 *
 *  Draw an horizontal line
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the top-left pixel
 *      w               Width in pixels
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_hline (GRAPH * dest, REGION * clip, int x, int y, int w)
{
    REGION base_clip ;
    int old_stipple = drawing_stipple;

    if (!dest) dest = scrbitmap ;
    if (!clip)
    {
        clip = &base_clip ;
        clip->x = 0 ;
        clip->y = 0 ;
        clip->x2 = dest->width - 1 ;
        clip->y2 = dest->height - 1 ;
    }

    dest->modified = 1 ;

    if (w < 0) w = -w, x -= (w - 1) ;
    if (y < clip->y || y > clip->y2) return ;

    if (x < clip->x) w += x-clip->x, x = clip->x ;
    if (x + w > clip->x2) w = clip->x2 - x + 1 ;

    if (w < 0) return ;

    if (dest->depth == 8)
    {
        Uint8 * ptr = dest->data ;
        ptr += dest->pitch * y + x ;
        if (drawing_stipple == 0xFFFFFFFF)
            _HLine8_nostipple (ptr, w) ;
        else
            _HLine8_stipple (ptr, w) ;
    }
    else if (dest->depth == 16)
    {
        Uint16 * ptr = dest->data ;
        ptr += dest->pitch * y / 2 + x ;
        if (drawing_stipple == 0xFFFFFFFF)
            _HLine16_nostipple (ptr, w) ;
        else
            _HLine16_stipple (ptr, w) ;
    }
    else if (dest->depth == 1)
    {
        Uint8 * ptr = dest->data;
        int mask ;
        ptr += dest->pitch * y + x / 8;
        mask = (1 << (7-(x & 7)));

        if (drawing_stipple != 0xFFFFFFFF)
        {
            while (w--)
            {
                if (drawing_stipple & 1) {
                    if (!syscolor8)
                        *ptr &= ~mask;
                    else
                        *ptr |= mask;
                }
                mask >>= 1;
                if (!mask)
                {
                    mask = 0x80;
                    ptr++;
                }
                drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            }
        } else {
            while (w--)
            {
                if (!syscolor8)
                    *ptr &= ~mask;
                else
                    *ptr |= mask;
                mask >>= 1;
                if (!mask)
                {
                    mask = 0x80;
                    ptr++;
                }
            }
        }
    }

    drawing_stipple = old_stipple;
}

/*
 *  FUNCTION : gr_box
 *
 *  Draw a filled rectangle
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the top-left pixel
 *      w               Width in pixels
 *      h               Height in pixels
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_box (GRAPH * dest, REGION * clip, int x, int y, int w, int h)
{
    REGION base_clip ;

    if (!dest) dest = scrbitmap ;
    dest->modified = 1 ;

    if (!clip)
    {
        clip = &base_clip ;
        clip->x = 0 ;
        clip->y = 0 ;
        clip->x2 = dest->width - 1 ;
        clip->y2 = dest->height - 1 ;
    }
    else
    {
        base_clip = *clip ;
        clip = &base_clip ;
        clip->x = MAX(MIN(clip->x, clip->x2), 0) ;
        clip->y = MAX(MIN(clip->y, clip->y2), 0) ;
        clip->x2 = MIN(MAX(clip->x, clip->x2), dest->width - 1) ;
        clip->y2 = MIN(MAX(clip->y, clip->y2), dest->height - 1) ;
    }

    if (w < 0) w = -w, x -= (w - 1) ;
    if (h < 0) h = -h, y -= (h - 1) ;

    if (x < clip->x) w += x - clip->x, x = clip->x ;
    if (y < clip->y) h += y - clip->y, y = clip->y ;

    if (x + w > clip->x2) w = clip->x2 - x + 1 ;
    if (y + h > clip->y2) h = clip->y2 - y + 1 ;

    if (w < 0 || h < 0) return ;

    if (!w) w++;
    if (!h) h++;

    if (dest->depth == 8)
    {
        Uint8 * ptr = dest->data ;
        ptr += dest->pitch * y + x ;
        while (h--)
        {
            _HLine8_nostipple (ptr, w) ;
            ptr += dest->pitch ;
        }
    }
    else if (dest->depth == 16)
    {
        Uint16 * ptr = dest->data ;
        int inc = dest->pitch / 2 ;
        ptr += dest->pitch / 2 * y + x ;
        while (h--)
        {
            _HLine16_nostipple (ptr, w) ;
            ptr += inc ;
        }
    }
    else if (dest->depth == 1)
    {
        int old_stipple = drawing_stipple;
        drawing_stipple = 0xFFFFFFFF;

        while (h--)
            gr_hline (dest, clip, x, y + h, w);

        drawing_stipple = old_stipple;
    }
}

/*
 *  FUNCTION : gr_rectangle
 *
 *  Draw a rectangle (non-filled)
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the top-left pixel
 *      w               Width in pixels
 *      h               Height in pixels
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_rectangle (GRAPH * dest, REGION * clip, int x, int y, int w, int h)
{
    int stipple = drawing_stipple ;

    dest->modified = 1;

    if (w < 0) w = -w, x -= (w - 1);
    if (h < 0) h = -h, y -= (h - 1);

    if (w)                 gr_hline (dest, clip, x        , y        , w     ) ;
    if ((w - 1) && h && w) gr_vline (dest, clip, x + w - 1, y        , h     ) ;
    if ((h - 1) && w && h) gr_hline (dest, clip, x + w - 1, y + h - 1, -w    ) ;
    if (h)                 gr_vline (dest, clip, x        , y + h - 1, -h    ) ;

    drawing_stipple = stipple ;
}

/*
 *  FUNCTION : gr_circle
 *
 *  Draw a circle
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the center
 *      r               Radius, in pixels
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_circle (GRAPH * dest, REGION * clip, int x, int y, int r)
{
    int cx = 0, cy = r ;
    int lcy = -1;
    int df = 1-r, de = 3, dse = -2*r + 5 ;
    int old_stipple = drawing_stipple;
    REGION base_clip ;
    int color = 0;

    if (!dest) dest = scrbitmap ;
    if (!clip)
    {
        clip = &base_clip ;
        clip->x = 0 ;
        clip->y = 0 ;
        clip->x2 = dest->width-1 ;
        clip->y2 = dest->height-1 ;
    }

    dest->modified = 1 ;

    if (dest->depth == 8 || dest->depth == 1) {
        color = syscolor8;
    } else if (dest->depth == 16) {
        color = syscolor16;
    }

    do {
        if (drawing_stipple & 1) {
            gr_put_pixelc (dest, clip, x-cx, y-cy, color) ;
            if (cx) gr_put_pixelc (dest, clip, x+cx, y-cy, color) ;

            if (cy) {
                gr_put_pixelc (dest, clip, x-cx, y+cy, color) ;
                if (cx) gr_put_pixelc (dest, clip, x+cx, y+cy, color) ;
            }

            if (cx != cy) {
                gr_put_pixelc (dest, clip, x-cy, y-cx, color) ;
                if (cy) gr_put_pixelc (dest, clip, x+cy, y-cx, color) ;
            }

            if (cx && cy != cx) {
                gr_put_pixelc (dest, clip, x-cy, y+cx, color) ;
                if (cy) gr_put_pixelc (dest, clip, x+cy, y+cx, color) ;
            }
        }
        drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));

        lcy = cy;
        cx++ ;
        if (df < 0) df += de,  de += 2, dse += 2 ;
        else        df += dse, de += 2, dse += 4, cy-- ;
    } while (cx <= cy) ;

    drawing_stipple = old_stipple;

}


/*
 *  FUNCTION : gr_fcircle
 *
 *  Draw a filled circle
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the center
 *      r               Radius, in pixels
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_fcircle (GRAPH * dest, REGION * clip, int x, int y, int r)
{
    int cx = 0, cy = r ;
    int df = 1 - r, de = 3, dse = -2*r + 5 ;
    int old_stipple = drawing_stipple;
    drawing_stipple = 0xFFFFFFFF;

    dest->modified = 1;
    do
    {
        if (cx != cy) {
            gr_hline (dest, clip, x-cy, y-cx, 2*cy+1) ;
            if (cx) gr_hline (dest, clip, x-cy, y+cx, 2*cy+1) ;
        }
        if (df < 0) {
            df += de,  de += 2, dse += 2 ;
        } else {
            df += dse, de += 2, dse += 4;
            gr_hline (dest, clip, x-cx, y-cy, 2*cx+1) ;
            if (cy) gr_hline (dest, clip, x-cx, y+cy, 2*cx+1) ;
            cy-- ;
        }
        cx++ ;
    }
    while (cx <= cy) ;

    drawing_stipple = old_stipple;
}

/*
 *  FUNCTION : gr_line
 *
 *  Draw a line
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      x, y            Coordinates of the first point
 *      w, h            Distance to the second point
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_line (GRAPH * dest, REGION * clip, int x, int y, int w, int h)
{
    int n, m, hinc, vinc ;
    int i1, i2, dd ;
    REGION base_clip ;
    int old_stipple = drawing_stipple;

    if (!w) {
        gr_vline (dest, clip, x, y, h) ;
        return ;
    }

    if (!h) {
        gr_hline (dest, clip, x, y, w) ;
        return ;
    }

    if (!dest) dest = scrbitmap ;
    if (!clip)
    {
        clip = &base_clip ;
        clip->x = 0 ;
        clip->y = 0 ;
        clip->x2 = dest->width - 1 ;
        clip->y2 = dest->height - 1 ;
    }

    dest->modified = 1 ;

    /* Clipping de la línea - INCORRECTO pero funcional */

/* TODO: SE NECESITA CORREGIR CLIPPING EN LINE */
#if 0
    if (x < clip->x) /* izquierda */
    {
        if ((x + w) < clip->x) return ;
        n = clip->x - x ;
        m = w ? n * h / ABS(w) : 0 ;
        x += n, w -= n, y += m, h -= m ;
        if (w == 0) return ;
    }
    if ((x + w) < clip->x) /* w < 0 */
    {
        n = clip->x - (x + w) ;
        m = w ? n * h / ABS(w) : 0 ;
        w += n, h -= m ;
        if (w == 0) return ;
    }
    if (y < clip->y) /* arriba */
    {
        if ((y + h) < clip->y) return ;
        m = clip->y - y ;
        n = h ? m * w / ABS(h) : 0 ;
        x += n, w -= n, y += m, h -= m ;
        if (h == 0) return ;
    }
    if ((y + h) < clip->y) /* h < 0 */
    {
        m = clip->y - (y + h) ;
        n = h ? m * w / ABS(h) : 0 ;
        w -= n, h += m ;
        if (h == 0) return ;
    }
    if (x > clip->x2) /* derecha */
    {
        if ((x + w) > clip->x2) return ;
        n = x - clip->x2 ;
        m = w ? n * h / ABS(w) : 0 ;
        x -= n, w += n, y += m, h -= m ;
        if (w == 0) return ;
    }
    if ((x + w) > clip->x2) /* w > 0 */
    {
        n = (x + w) - clip->x2 ;
        m = w ? n * h / ABS(w) : 0 ;
        w -= n, h -= m ;
        if (w == 0) return ;
    }
    if (y > clip->y2) /* abajo */
    {
        if ((y + h) > clip->y2) return ;
        m = y - clip->y2 ;
        n = m * w / ABS(h) ;
        x += n, w -= n, y -= m, h += m ;
        if (h == 0) return ;
    }

    if ((y + h) > clip->y2) /* h > 0 */
    {
        m = (y + h) - clip->y2 ;
        n = h ? m * w / ABS(h) : 0 ;
        w -= n, h -= m ;
        if (h == 0) return ;
    }
#endif
    hinc = (w > 0) ? 1 : -1 ;
    vinc = (h > 0) ? dest->pitch : -(int)dest->pitch ;
    if (dest->depth == 16) vinc /= 2;

    /* Aquí va una implementación deprisa y corriendo de Bresenham */

    w = ABS(w) ;
    h = ABS(h) ;

    if (w > h) {
        i1 = 2 * h ; dd = i1 - w ; i2 = dd - w ;
    } else {
        i1 = 2 * w ; dd = i1 - h ; i2 = dd - h ;
    }

    if (dest->depth == 8)
    {
        Uint8 * ptr = (Uint8 *)dest->data + dest->pitch * y + x;

        if (w > h) while (w--)
        {
            if (drawing_stipple & 1) _Pixel8(ptr, syscolor8) ;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            if (dd >= 0) ptr += hinc + vinc, dd += i2 ;
            else         ptr += hinc,        dd += i1 ;
        }
        else while (h--)
        {
            if (drawing_stipple & 1) _Pixel8(ptr, syscolor8) ;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            if (dd >= 0) ptr += vinc + hinc, dd += i2 ;
            else         ptr += vinc,        dd += i1 ;
        }
    }
    else if (dest->depth == 16)
    {
        Uint16 * ptr = (Uint16 *)dest->data + dest->pitch * y / 2 + x;

        if (w > h) while (w--)
        {
            if (drawing_stipple & 1) _Pixel16(ptr, syscolor16, syscolor16_alpha) ;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            if (dd >= 0) ptr += hinc + vinc, dd += i2 ;
            else         ptr += hinc,        dd += i1 ;
        }
        else while (h--)
        {
            if (drawing_stipple & 1) _Pixel16(ptr, syscolor16, syscolor16_alpha) ;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            if (dd >= 0) ptr += vinc + hinc, dd += i2 ;
            else         ptr += vinc,        dd += i1 ;
        }
    }
    else if(dest->depth == 1)
    {
        Uint8 * ptr = dest->data + dest->pitch * y + x / 8;
        Uint8 mask, rmask ;
        mask = (1 << (7-(x & 7)));

        if (hinc < 0) rmask = 0x01 ; else rmask = 0x80 ;

        if (w > h) while (w--)
        {
            if (drawing_stipple & 1) if (!syscolor8) *ptr &= ~mask; else *ptr |= mask;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            if (dd >= 0) ptr += vinc, dd += i2 ;
            else                      dd += i1 ;
            if (hinc < 0) mask <<= 1; else mask >>= 1; if (!mask) mask = rmask, ptr += hinc ;
        }
        else while (h--)
        {
            if (drawing_stipple & 1) if (!syscolor8) *ptr &= ~mask ; else *ptr |= mask;
            drawing_stipple = ((drawing_stipple << 1) | ((drawing_stipple & 0x80000000) ? 1:0));
            if (dd >= 0) { ptr += vinc, dd += i2 ; if (hinc < 0) mask <<= 1; else mask >>= 1; if (!mask) mask = rmask, ptr += hinc ; }
            else           ptr += vinc, dd += i1 ;
        }
    }

    drawing_stipple = old_stipple;

}

/*
 *  FUNCTION : gr_bezier
 *
 *  Draw a bezier curve
 *
 *  PARAMS :
 *      dest            Destination bitmap or NULL for screen
 *      clip            Clipping region or NULL for the whole screen
 *      params          Pointer to an integer array with the parameters:
 *
 *          x1, y1
 *          x2, y2
 *          x3, y3
 *          x4, y4      Curve points
 *          level       Curve smoothness (1 to 15, 15 is more)
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_bezier (GRAPH * dest, REGION * clip, int * params)
{
    const int x1 = params[0];
    const int y1 = params[1];
    const int x2 = params[2];
    const int y2 = params[3];
    const int x3 = params[4];
    const int y3 = params[5];
    const int x4 = params[6];
    const int y4 = params[7];

    int level = params[8];

    float x = (float)x1, y = (float)y1;
    float xp = x, yp = y;
    float delta;
    float dx, d2x, d3x;
    float dy, d2y, d3y;
    float a, b, c;
    int i;
    int n = 1;

    dest->modified = 1;

    /* Compute number of iterations */

    if(level < 1) level=1;
    if(level >= 15) level=15;
    while (level-- > 0) n*= 2;
    delta = 1.0f / (float)n;

    /* Compute finite differences */
    /* a, b, c are the coefficient of the polynom in t defining the parametric curve */
    /* The computation is done independently for x and y */

    a = (float)(-x1 + 3*x2 - 3*x3 + x4);
    b = (float)(3*x1 - 6*x2 + 3*x3);
    c = (float)(-3*x1 + 3*x2);

    d3x = 6 * a * delta*delta*delta;
    d2x = d3x + 2 * b * delta*delta;
    dx = a * delta*delta*delta + b * delta*delta + c * delta;

    a = (float)(-y1 + 3*y2 - 3*y3 + y4);
    b = (float)(3*y1 - 6*y2 + 3*y3);
    c = (float)(-3*y1 + 3*y2);

    d3y = 6 * a * delta*delta*delta;
    d2y = d3y + 2 * b * delta*delta;
    dy = a * delta*delta*delta + b * delta*delta + c * delta;

    for (i = 0; i < n; i++)
    {
        x += dx; dx += d2x; d2x += d3x;
        y += dy; dy += d2y; d2y += d3y;
        if((Sint16)(xp) != (Sint16)(x) || (Sint16)(yp) != (Sint16)(y))
        {
            gr_line(dest,clip,(Sint16)xp,(Sint16)yp,(Sint16)x-(Sint16)xp,(Sint16)y-(Sint16)yp);
        }
        xp = x; yp = y;
    }
}


/*
 *  FUNCTION : info_object
 *
 *  Internal function used to return information about a primitive object
 *
 *  PARAMS :
 *      dr          Drawing object
 *      bbox        Pointer to a REGION to be filled with the bounding box
 *
 *  RETURN VALUE :
 *      1 if the primitive changed since last frame
 *
 */

int info_object (DRAWING_OBJECT * dr, REGION * clip)
{
    REGION newclip;

    switch (dr->type)
    {
        case DRAWOBJ_CIRCLE:
        case DRAWOBJ_FCIRCLE:
            newclip.x  = dr->x1-dr->x2;
            newclip.y  = dr->y1-dr->x2;
            newclip.x2 = dr->x1+dr->x2;
            newclip.y2 = dr->y1+dr->x2;
            break;
        case DRAWOBJ_CURVE:
            newclip.x  = dr->x1;
            newclip.y  = dr->y1;
            newclip.x2 = dr->x4;
            newclip.y2 = dr->y4;
            break;
        default:
            newclip.x  = dr->x1;
            newclip.y  = dr->y1;
            newclip.x2 = dr->x2;
            newclip.y2 = dr->y2;
            break;
    }

    if (newclip.x  != clip->x  || newclip.y  != clip->y ||
        newclip.x2 != clip->x2 || newclip.y2 != clip->y2)
    {
        *clip = newclip;
        return 1;
    }
    return 0;
}

/*
 *  FUNCTION : draw_object
 *
 *  Internal function used to draw a primitive object
 *
 *  PARAMS :
 *      dr          Drawing object
 *
 *  RETURN VALUE :
 *      None
 *
 */

void draw_object (DRAWING_OBJECT * dr, REGION * clip)
{
    int b8 = syscolor8;
    int b16 = syscolor16;

    syscolor8 = dr->color8;
    syscolor16 = dr->color16;

    if (drawing_alpha!=255) gr_setalpha(drawing_alpha);

    switch (dr->type)
    {
        case DRAWOBJ_LINE:
            gr_line (scrbitmap, clip, dr->x1, dr->y1, dr->x2-dr->x1, dr->y2-dr->y1);
            break;
        case DRAWOBJ_RECT:
            gr_rectangle (scrbitmap, clip, dr->x1, dr->y1, dr->x2-dr->x1, dr->y2-dr->y1) ;
            break;
        case DRAWOBJ_BOX:
            gr_box (scrbitmap, clip, dr->x1, dr->y1, dr->x2-dr->x1, dr->y2-dr->y1) ;
            break;
        case DRAWOBJ_CIRCLE:
            gr_circle (scrbitmap, clip, dr->x1, dr->y1, dr->x2);
            break;
        case DRAWOBJ_FCIRCLE:
            gr_fcircle (scrbitmap, clip, dr->x1, dr->y1, dr->x2);
            break;
        case DRAWOBJ_CURVE:
            gr_bezier (scrbitmap, clip, &dr->x1);
            break;
    }

    syscolor8 = b8;
    syscolor16 = b16;
}

/*
 *  FUNCTION : gr_drawing_new
 *
 *  Create a new on-screen drawing object
 *
 *  PARAMS :
 *      drawing         Object type and coordinates
 *      z               Z Coordinate
 *
 *  RETURN VALUE :
 *      An integer identifier, representing the new object, or -1 if error
 *
 */

int gr_drawing_new (DRAWING_OBJECT drawing, int z)
{
    DRAWING_OBJECT * dr = malloc (sizeof(DRAWING_OBJECT));
    if (!dr) return -1;

    /* Fill the struct and register the new object */

    *dr = drawing;
    dr->next = drawing_objects;

    dr->color8 = syscolor8;
    dr->color16 = syscolor16;
    dr->id = gr_new_object (z, info_object, draw_object, dr);

    drawing_objects = dr;

    return dr->id;
}

/*
 *  FUNCTION : gr_drawing_destroy
 *
 *  Destroy a new on-screen drawing object
 *
 *  PARAMS :
 *      id              Object id returned by gr_drawing_new
 *                      or 0 to destroy every object
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_drawing_destroy (int id)
{
    DRAWING_OBJECT * dr=drawing_objects;
    DRAWING_OBJECT * ldr=NULL;
    DRAWING_OBJECT * ndr=NULL;

    while (dr) {
        ndr = dr->next;
        if(!id || dr->id == id){
            gr_destroy_object (dr->id);
            if(ldr) {
                ldr->next = dr->next;
            }
            free(dr);
            if (drawing_objects == dr) drawing_objects = ndr;
            if(id) return;
            dr = ldr;
        }
        ldr = dr;
        dr = ndr;
    }
}

/*
 *  FUNCTION : gr_drawing_move
 *
 *  Move an on-screen drawing object to a new coordinates,
 *  relative to the first point in the primitive
 *
 *  PARAMS :
 *      id              Object id returned by gr_drawing_new
 *      x, y            New coordinates
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_drawing_move (int id, int x, int y)
{
    DRAWING_OBJECT * dr=drawing_objects;

    while (dr) {
        if (dr->id == id)
        {
            int incx = x - dr->x1;
            int incy = y - dr->y1;

            dr->x1 += incx;
            dr->y1 += incy;
            if (dr->type == DRAWOBJ_CIRCLE || dr->type == DRAWOBJ_FCIRCLE) return;
            dr->x2 += incx;
            dr->y2 += incy;
            dr->x3 += incx;
            dr->y3 += incy;
            dr->x4 += incx;
            dr->y4 += incy;
            return;
        }

        dr = dr->next;
    }
}

/*
 *  FUNCTION : gr_drawing_alpha
 *
 *  Sets the drawing alpha value for primitives
 *
 *  PARAMS :
 *      alpha           New alpha (0-255)
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_setalpha (int value)
{
    value &= 0xFF;
    drawing_alpha = value;

    if (enable_16bits)
    {
        drawing_alpha16 = gr_alpha16(255-value);
        drawing_alpha8 = gr_alpha8(value);
        syscolor16_alpha = gr_alpha16(value)[syscolor16];
    }
    else
    {
        drawing_alpha8 = gr_alpha8(value);
    }
}

/*
 *  FUNCTION : gr_drawing_alpha
 *
 *  Sets the drawing color
 *
 *  PARAMS :
 *      alpha           New color (16 bits if enable_16bits == 1)
 *
 *  RETURN VALUE :
 *      None
 *
 */

void gr_setcolor (int c)
{
    int r, g, b;

    if (c == 0)
    {
        syscolor8 = 0;
        syscolor16 = 0;
    }
    else if (!enable_16bits)
    {
        syscolor8 = c;
    }
    else
    {
        if (c > 255 || c < 0)
        {
            gr_get_rgb (c, &r, &g, &b);
            syscolor8 = gr_find_nearest_color (r, g, b);
        }
        else
        {
            syscolor8 = c;
        }
        syscolor16 = c ;
    }

    if (drawing_alpha != 255) syscolor16_alpha = gr_alpha16(drawing_alpha)[syscolor16];
}

