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

/* Rutinas matemáticas de punto fijo, basadas en Allegro */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef MAX
#define MAX(a,b)        ((a)>(b) ? (a):(b))
#define MIN(a,b)        ((a)<(b) ? (a):(b))
#endif

typedef long int fixed ;

extern fixed cos_table[90001] ;
extern int   cos_table_initialized ;
extern void  init_cos_tables() ;

#ifndef __GNUC__
#define inline __inline
#endif

#ifdef DEBUG
# define __INLINE static
#else
# define __INLINE static inline
#endif

//#define FIXED_PREC 12

#define FIXED_PREC      10000
#define FIXED_PREC_MED  5000
#define FIXED_PREC_DEC  1000

__INLINE fixed ftofix(double x)
{
	return (long)(x * FIXED_PREC);

}

__INLINE float fixtof (fixed x)
{
	return ((float)x) / FIXED_PREC ;
}

__INLINE fixed itofix(int x)
{
	return x * FIXED_PREC ;
}

__INLINE int fixceil(fixed x)
{
    int xd;

    if (x<0) {
        xd = x % FIXED_PREC ;
        x -= (FIXED_PREC + xd) ;
    } else if (x>0) {
        xd = x % FIXED_PREC ;
        x += (FIXED_PREC - xd) ;
    }

	return x ;
}

__INLINE int fixtoi(fixed x)
{
	return x / FIXED_PREC ;
}

__INLINE fixed fcos(int x)
{
if (x < 0) x = -x ;
if (x >= 360000) x %= 360000 ;
if (x >= 270000) return cos_table[360000 - x] ;
if (x >= 180000) return -cos_table[x - 180000] ;
if (x >= 90000) return -cos_table[180000 - x] ;
return cos_table[x] ;
}

__INLINE fixed fsin(int x)
{
if (x < 0) return -fsin(-x) ;
if (x >= 360000) x %= 360000 ;
if (x >= 270000) return -cos_table[x - 270000] ;
if (x >= 180000) return -cos_table[270000 - x] ;
if (x >= 90000) return cos_table[x - 90000 ] ;
return cos_table[90000 - x] ;
}

__INLINE fixed fmul(int x, int y)
{
	return ftofix (fixtof(x) * fixtof(y)) ;
}

__INLINE fixed fdiv(int x, int y)
{
	return ftofix (fixtof(x) / fixtof(y)) ;
}
