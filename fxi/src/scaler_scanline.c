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

/* By Splinter 2007 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "fxi.h"

void scanline2x(Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height)
{
	const Uint32 nextlineSrc = srcPitch / sizeof(Uint16);
	const Uint16 *p = (const Uint16 *)srcPtr;

	const Uint32 nextlineDst = dstPitch / sizeof(Uint16);
	Uint16 *q = (Uint16 *)dstPtr;

	while (height--) {
		int tmpWidth = width;
		while (tmpWidth--) {
            *q=*p;
            *(q+nextlineDst)=0;
            q++;
            *q=*p;
            *(q+nextlineDst)=0;
            q++;
            p++;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 2;
	}
}
