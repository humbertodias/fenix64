/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.85
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
 *
 */

/*
 * FILE        : f_bgload.c
 * DESCRIPTION : FXI functions for background loading (THREAD BASED)
 */

#include <stdlib.h>
#include <stdint.h>
#include "fxi.h"


typedef struct {
	char *file;
	int *id;
	int (*fn)(const char *);
} bgdata ;

/**
 *	PREP
 *	Helper function preparing params
 **/

bgdata *prep(int *params){
	bgdata *t=(bgdata*)malloc(sizeof(bgdata));
	t->file=(char *)string_get(params[0]);
	t->id=(int *)(intptr_t)params[1];
	string_discard(params[0]);
	return t;
}


/**
 *	bgDoLoad
 *	Helper function executed in the new thread
 **/

int bgDoLoad(void *d){
	bgdata *t=(bgdata*)d;
	*(t->id)= -2 ; // WAIT STATUS
	*(t->id)=(*t->fn)(t->file);
	free(t);
	return 0;
}


/**
   int LOAD_FPG(STRING FICHERO, INT POINTER VARIABLE)
   Loads fpg file FICHERO on a separate thread
   VARIABLE is -2 while waiting, -1 on error, >=0 otherwise
 **/

int fxi_bgload_fpg(INSTANCE * my, int * params)
{
	bgdata *t=prep(params);
	t->fn=gr_load_fpg;
	SDL_CreateThread(bgDoLoad,(void *)t);
	return 0 ;
}

/**
   int LOAD_FGC(STRING FICHERO, INT POINTER VARIABLE)
   Loads fpg file FICHERO on a separate thread
   VARIABLE is -2 while waiting, -1 on error, >=0 otherwise
 **/

int fxi_bgload_fgc(INSTANCE * my, int * params)
{
	bgdata *t=prep(params);
	t->fn=fgc_load;
	SDL_CreateThread(bgDoLoad,(void *)t);
	return 0 ;
}
