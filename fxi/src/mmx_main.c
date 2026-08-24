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
 * FILE        : mmx_main.c
 * DESCRIPTION : MMX support routines (including CPUID)
 *
 * HISTORY:      0.82 - First version 
 */

#include <SDL.h>

#include "fxi.h"


int MMX_available = 0;

/*
 *  FUNCTION : MMX_init
 *
 *  See if the current processor supports MMX and update the
 *  global variable mmx_available with either 1 or 0
 *
 *  PARAMS : 
 *              None
 *
 *  RETURN VALUE : 
 *      1 if MMX available, 0 otherwise
 *
 */

void MMX_init()
{
#if defined(__i386__) || defined(_M_IX86)
        int cpuid_processor;
        int cpuid_features;

#ifdef __GNUC__
       __asm__ __volatile__ (
               "movl $1, %%eax \n"
               "cpuid \n"
               : "=a" (cpuid_processor), "=d" (cpuid_features)
               );
#else
        _asm
        {
                mov     eax,1
                cpuid
                mov     cpuid_processor, eax
                mov     cpuid_features, edx
        }
#endif

        MMX_available = (cpuid_features & (1 << 23)) ? 1:0;
#else
        MMX_available = 0;
#endif
}
