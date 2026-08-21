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
 * FILE        : instance.c
 * DESCRIPTION : Implements FENIX language function handlers
 *
 * HISTORY:  0.80 - Solved leak problem in instance_destroy()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif


#include "fxi.h"

#define MIN_PRIORITY	-2048
#define MAX_PRIORITY	 2048
#define PRIORITIES		(MAX_PRIORITY - MIN_PRIORITY + 1)

static uint8_t * vm_base = NULL ;
static size_t    vm_arena_size = 64 * 1024 * 1024 ;
static size_t    vm_used = 0 ;

void vm_arena_init (void)
{
	if (!vm_base)
		vm_base = (uint8_t *) malloc (vm_arena_size) ;
	assert (vm_base != 0) ;
	vm_used = 16 ;
	memset (vm_base, 0, 16) ;
}

void * vm_malloc (size_t n)
{
	size_t * hdr ;

	if (!vm_base) vm_arena_init () ;
	n = (n + 15) & ~(size_t)15 ;
	if (vm_used + 16 + n > vm_arena_size)
		gr_error ("vm_malloc: arena llena") ;
	hdr = (size_t *)(vm_base + vm_used) ;
	*hdr = n ;
	vm_used += 16 + n ;
	return (uint8_t *)hdr + 16 ;
}

int vm_in_arena (const void * p)
{
	const uint8_t * q = (const uint8_t *) p ;
	return vm_base && q >= vm_base && q < vm_base + vm_arena_size ;
}

size_t vm_alloc_size (const void * p)
{
	if (!vm_in_arena (p) || (const uint8_t *)p < vm_base + 16)
		return 0 ;
	return *(const size_t *)((const uint8_t *)p - 16) ;
}

static void * vm_from_base (const void * base, size_t size, uint32_t t)
{
	uintptr_t b, full, high ;
	uint32_t low ;

	if (!base || size == 0) return NULL ;
	b = (uintptr_t) base ;
	high = b & ~(uintptr_t)0xFFFFFFFFULL ;
	low = (uint32_t) b ;
	full = high | t ;
	if (t < low)
		full += (uintptr_t)1 << 32 ;
	if (full >= b && full < b + size)
		return (void *) full ;
	return NULL ;
}

void * vm_ptr (INSTANCE * my, int p32)
{
	uint32_t t = (uint32_t) p32 ;
	void * p ;

	if (!p32) return NULL ;

	if (vm_base) {
		p = vm_from_base (vm_base, vm_arena_size, t) ;
		if (p) return p ;
	}

	p = vm_from_base (stack, sizeof(stack), t) ;
	if (p) return p ;

	if (my) {
		if (my->pridata) {
			p = vm_from_base (my->pridata, (size_t)my->private_size + 4, t) ;
			if (p) return p ;
		}
		if (my->locdata) {
			p = vm_from_base (my->locdata, (size_t)local_size + 4, t) ;
			if (p) return p ;
		}
		if (my->pubdata) {
			p = vm_from_base (my->pubdata, (size_t)my->public_size + 4, t) ;
			if (p) return p ;
		}
	}

	return (void *)(uintptr_t) t ;
}

/* ---------------------------------------------------------------------- */
/* Módulo de gestión de instancias, con las funciones de incialización y  */
/* destrucción, duplicado, etc.                                           */
/* ---------------------------------------------------------------------- */

INSTANCE * first_instance = NULL ;
INSTANCE * last_instance  = NULL ;

/* Priority lists */

static INSTANCE * first_by_priority     = NULL ;
static INSTANCE * iterator_by_priority  = NULL ;
static int        iterator_reset        = 1;

/* Dirty list: a list of all instances that need an update because
 * they changed its priority since the last execution
 */

static INSTANCE * dirty_list = NULL;

static int instance_maxid =  FIRST_INSTANCE_ID-2 ;

/*
 *  FUNCTION : instance_getid
 *
 *  Allocate and return a free instance identifier code.
 *  It must be an even number. It should reuse already freed
 *  identifiers, but there is little point currently because
 *  an array is not used to access them.
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      A new, unused instance identifier
 */

int instance_getid()
{
	instance_maxid += 2 ;
	return instance_maxid ;
}

/*
 *  FUNCTION : instance_duplicate
 *
 *  Create a copy of an instance, using the same local/private data
 *    - Updates the instance list, adding the new instance
 *    - Marks all local and private strings
 *    - Updates all parents local family variables
 *
 *  The new instance uses the same code pointer as the father
 *  (it begins execution at the same line, instead of the first one)
 *
 *  PARAMS :
 *      father			Pointer to the original instance
 *
 *  RETURN VALUE :
 *      Pointer to the new instance
 */

INSTANCE * instance_duplicate (INSTANCE * father)
{
	INSTANCE * r, * brother ;
	int n ;

	r = (INSTANCE *) malloc (sizeof(INSTANCE)) ;
	assert (r != 0) ;

	r->pridata      = (int *) vm_malloc (father->private_size + 4) ;
	r->pubdata      = (int *) vm_malloc (father->public_size + 4) ;
	r->locdata      = (int *) vm_malloc (local_size + 4) ;
	r->code         = father->code ;
	r->codeptr      = father->codeptr ;
	r->exitcode     = father->exitcode ;
	r->proc         = father->proc ;

	r->inpridata    = NULL ;
	r->inproc       = NULL ;

    r->switchval        = 0;
    r->switchval_string = 0;
    r->cased            = 0;

	r->breakpoint   = 0 ;

	r->private_size = father->private_size ;
	r->public_size  = father->public_size ;
	r->first_run    = 1 ;

	if ( father->private_size > 0 ) {
	    memcpy (r->pridata, father->pridata, father->private_size) ;
	}
	if ( father->public_size > 0 ) {
	    memcpy (r->pubdata, father->pubdata, father->public_size) ;
	}
	if ( local_size > 0 ) {
	    memcpy (r->locdata, father->locdata, local_size) ;
	}

	/* Inicializa datos DIV */

	/* Crea el proceso clónico como si lo hubiera llamado el padre */
	/* No sé si es eso lo que hace DIV */

	LOCDWORD(r, PROCESS_ID)   = instance_getid() ;
    LOCDWORD(r, PROCESS_TYPE) = LOCDWORD(father, PROCESS_TYPE) ;
	LOCDWORD(r, SON)          = 0 ;
	LOCDWORD(r, SMALLBRO)     = 0 ;

	LOCDWORD(r, FATHER)     = LOCDWORD(father, PROCESS_ID) ;
	brother = instance_get(LOCDWORD(father, SON)) ;
	if (brother) {
	    LOCDWORD(r, BIGBRO)         = LOCDWORD(brother, PROCESS_ID) ;
	    LOCDWORD(brother, SMALLBRO) = LOCDWORD(r, PROCESS_ID) ;
	} else {
	    LOCDWORD(r, BIGBRO)         = 0 ;
	}
	LOCDWORD(father,SON)    = LOCDWORD(r,PROCESS_ID) ;

	/* Actualiza las cuentas de uso de las cadenas */

	for (n = 0 ; n < r->proc->string_count ; n++)
		string_use (PRIDWORD(r, r->proc->strings[n])) ;

	for (n = 0 ; n < r->proc->pubstring_count ; n++)
		string_use (PUBDWORD(r, r->proc->pubstrings[n])) ;

	for (n = 0 ; n < local_strings ; n++)
		string_use (LOCDWORD(r, localstr[n])) ;

	/* Añade la instancia al final de la lista */

	r->next    = 0 ;
	r->prev    = last_instance ;
	if (r->prev) r->prev->next = r ;

	r->next_by_priority = NULL;
	r->prev_by_priority = NULL;

	last_instance = r ;
	if (!first_instance) {
	    first_instance      = r ;
		LOCDWORD(r, FATHER) = 0 ;
	}

	/* The called_by pointer should be set only when the caller
	 * is waiting for this process to return */

	r->called_by = NULL;
	r->stack = NULL;

	/* Initialize list pointers */

	r->next_dirty = dirty_list;
	dirty_list    = r;
	r->is_dirty   = 1;

	LOCDWORD(r, STATUS) = STATUS_RUNNING;

	return r ;
}

/*
 *  FUNCTION : instance_new
 *
 *  Create a new instance, using the default local/private data
 *    - Updates the instance list, adding the new instance
 *    - Marks all local and private strings
 *    - Updates all parents local family variables
 *
 *  PARAMS :
 *      proc			Pointer to the procedure definition
 *		father			Pointer to the father instance (may be NULL)
 *
 *  RETURN VALUE :
 *      Pointer to the new instance
 */

INSTANCE * instance_new (PROCDEF * proc, INSTANCE * father)
{
	INSTANCE * r, * brother ;
	int n ;

	r = (INSTANCE *) malloc (sizeof(INSTANCE)) ;
	assert (r != 0) ;

	r->pridata      = (int *) vm_malloc (proc->private_size + 4) ;
	r->pubdata      = (int *) vm_malloc (proc->public_size + 4) ;
	r->locdata      = (int *) vm_malloc (local_size + 4) ;
	r->code         = proc->code ;
	r->codeptr      = proc->code ;
	r->exitcode     = proc->exitcode ;
	r->proc         = proc ;

	r->inpridata    = NULL ;
	r->inproc       = NULL ;

    r->switchval        = 0;
    r->switchval_string = 0;
    r->cased            = 0;

	r->breakpoint   = 0 ;

	r->private_size = proc->private_size ;
	r->public_size  = proc->public_size ;
	r->first_run    = 1 ;

    if ( proc->private_size > 0 ) {
	    memcpy (r->pridata, proc->pridata, proc->private_size) ;
    }
    if ( proc->public_size > 0 ) {
 	    memcpy (r->pubdata, proc->pubdata, proc->public_size) ;
    }
    if ( local_size > 0 ) {
	    memcpy (r->locdata, localdata, local_size) ;
    }

	/* Inicializa datos DIV */

	LOCDWORD(r, PROCESS_ID)   = instance_getid() ;
	LOCDWORD(r, PROCESS_TYPE) = proc->type ;
	LOCDWORD(r, SON)          = 0 ;
	LOCDWORD(r, SMALLBRO)     = 0 ;

	if (father)
	{
		LOCDWORD(r, FATHER)     = LOCDWORD(father, PROCESS_ID) ;
		brother = instance_get(LOCDWORD(father, SON)) ;
		if (brother) {
			LOCDWORD(r, BIGBRO)         = LOCDWORD(brother, PROCESS_ID) ;
    		LOCDWORD(brother, SMALLBRO) = LOCDWORD(r, PROCESS_ID) ;
    	} else {
    		LOCDWORD(r, BIGBRO)         = 0 ;
    	}
		LOCDWORD(father,SON)    = LOCDWORD(r, PROCESS_ID) ;
	} else {
		LOCDWORD(r, FATHER)     = 0 ;
		LOCDWORD(r, BIGBRO)     = 0 ;
	}

	/* Cuenta los usos de las variables tipo cadena */

	for (n = 0 ; n < proc->string_count ; n++)
		string_use (PRIDWORD(r, proc->strings[n])) ;

	for (n = 0 ; n < proc->pubstring_count ; n++)
		string_use (PUBDWORD(r, proc->pubstrings[n])) ;

	for (n = 0 ; n < local_strings ; n++)
		string_use (LOCDWORD(r, localstr[n])) ;

	/* Añade la instancia al final de la lista */

	r->next    = 0 ;
	r->prev    = last_instance ;
	if (r->prev) r->prev->next = r ;

	r->next_by_priority = NULL;
	r->prev_by_priority = NULL;

	last_instance = r ;
	if (!first_instance) {
	    first_instance      = r ;
		LOCDWORD(r, FATHER) = 0 ;
	}

	/* The called_by pointer should be set only when the caller
	 * is waiting for this process to return */

	r->called_by = NULL;
	r->stack = NULL;

	/* Initialize list pointers */

	r->next_dirty = dirty_list;
	dirty_list    = r;
	r->is_dirty   = 1;

	LOCDWORD(r, STATUS) = STATUS_RUNNING;

	return r ;
}

/*
 *  FUNCTION : instance_get
 *
 *  Returns a instance, given its ID. This is actually
 *  slow, it should use a better search method.
 *
 *  PARAMS :
 *      id				Integer ID of the instance
 *
 *  RETURN VALUE :
 *      Pointer to the found instance or NULL if not found
 */

INSTANCE * instance_get(int id)
{
	INSTANCE * i = first_instance ;

	while (i)
	{
		if (LOCDWORD(i, PROCESS_ID) == id) break ;
		i = i->next ;
	}
	return i ;
}

INSTANCE * instance_getfather (INSTANCE * i)
{
	return instance_get(LOCDWORD(i, FATHER)) ;
}

INSTANCE * instance_getson (INSTANCE * i)
{
	return instance_get(LOCDWORD(i, SON)) ;
}

INSTANCE * instance_getbigbro (INSTANCE * i)
{
	return instance_get(LOCDWORD(i, BIGBRO)) ;
}

INSTANCE * instance_getsmallbro (INSTANCE * i)
{
	return instance_get(LOCDWORD(i, SMALLBRO)) ;
}

/*
 *  FUNCTION : instance_destroy_all
 *
 *  Destroy all instances. Simply calls instance_destroy
 *  for any and every instance in existence.
 *
 *  PARAMS :
 *      except			Don't destroy this instance (used for LET_ME_ALONE)
 *
 *  RETURN VALUE :
 *      None
 */

void instance_destroy_all (INSTANCE * except)
{
	INSTANCE * i, * next ;

	i = first_instance ;
	while (i)
	{
		next = i->next ;
		if (i != except) instance_destroy (i) ;
		i = next ;
	}
}

/*
 *  FUNCTION : instance_destroy
 *
 *  Destroy an instance, effectively
 *    - Updates any instance list, removing the given instance
 *    - Discards all local and private strings
 *    - Updates all parents local family variables
 *    - Frees any memory involved
 *
 *  PARAMS :
 *      r			Pointer to the instance
 *
 *  RETURN VALUE :
 *      None
 */

void instance_destroy (INSTANCE * r)
{
	INSTANCE * father, * bigbro, * smallbro, * smallson, * bigson, * smallerbro=NULL;
	int n ;

	if (LOCDWORD(r, GRAPHID) != 0) object_list_dirty = 1;

	LOCDWORD(r, STATUS) = STATUS_RUNNING;

	if (LOCDWORD(r, BOX_X0) != -2) gr_mark_instance(r);

	/* Actualiza la cuenta de referencia de las variables tipo string */

	for (n = 0 ; n < r->proc->string_count ; n++)
		string_discard (PRIDWORD(r, r->proc->strings[n])) ;

	for (n = 0 ; n < r->proc->pubstring_count ; n++)
		string_discard (PUBDWORD(r, r->proc->pubstrings[n])) ;

	for (n = 0 ; n < local_strings ; n++)
		string_discard (LOCDWORD(r, localstr[n])) ;

	/* Actualiza árbol DIV */

    /* Si tengo hermano mayor */
	bigbro = instance_get(LOCDWORD(r,BIGBRO)) ;
	if (bigbro) LOCDWORD(bigbro,SMALLBRO) = LOCDWORD(r,SMALLBRO) ;

    /* Si tengo un hermano */
	smallbro = instance_get(LOCDWORD(r,SMALLBRO)) ;
	if (smallbro) LOCDWORD(smallbro,BIGBRO) = LOCDWORD(r,BIGBRO) ;

    /* Yo, ya estoy fuera */

	father = instance_get(LOCDWORD(r,FATHER)) ;
    if ( father ) {
        smallerbro = instance_get(LOCDWORD(father,SON));
        if ( smallerbro == r ) {
            smallerbro = bigbro;
        }
    } else {
        if ( smallbro ) {
            smallerbro = smallbro;
	        while (LOCDWORD(smallerbro,SMALLBRO))
		    {
    			smallerbro = instance_get(LOCDWORD(smallerbro,SMALLBRO)) ;
			    assert ( smallerbro != 0) ;
		    }
		} else {
            smallerbro = bigbro;
        }
    }

	/* Tengo hijos */
	smallson = instance_get(LOCDWORD(r,SON)) ;
	if (smallson)
	{
		/* Busca el primer y el último hijo */

		bigson = smallson ;
        /* Asignar los procesos huerfanos al padre, si es que tengo padre */
	    LOCDWORD(bigson,FATHER) = father ? LOCDWORD(father,PROCESS_ID) : 0 ;
		while (LOCDWORD(bigson,BIGBRO))
		{
			bigson = instance_get(LOCDWORD(bigson,BIGBRO)) ;
			assert ( bigson != 0) ;
            /* Asignar los procesos huerfanos al padre, si es que tengo padre */
			LOCDWORD(bigson,FATHER) = father ? LOCDWORD(father,PROCESS_ID) : 0 ;
		}

		/* Si tengo hermanos, pongo mis hijos como hermanos menores del menor de mis hermanos */
        if ( smallerbro ) {
            LOCDWORD(smallerbro,SMALLBRO)   = LOCDWORD(bigson,PROCESS_ID) ;
	        LOCDWORD(bigson,BIGBRO)         = LOCDWORD(smallerbro,PROCESS_ID) ;
        }

		/* Si tengo padre */
		if (father) {
			LOCDWORD(father,SON) = LOCDWORD(smallson,PROCESS_ID) ;
        }
	} else if (father) { /* Si tengo padre */
	    /* Si soy el primer hijo, asigno a mi hermano mayor como hijo primero a mi padre */
		if (LOCDWORD(father,SON) == LOCDWORD(r, PROCESS_ID))
		{
		    LOCDWORD(father,SON) = LOCDWORD(r,BIGBRO) ;
		}
	}

	/* Quita la instancia de la lista */

	if (r->prev) r->prev->next  = r->next ;

	if (r->next) r->next->prev = r->prev ;

	if (first_instance == r) first_instance = r->next ;

	if (last_instance == r) last_instance = r->prev ;

	/* Remove the instance from the priority list */
	if (first_by_priority == r) first_by_priority = r->next_by_priority;

	if (r->prev_by_priority) r->prev_by_priority->next_by_priority = r->next_by_priority;
	if (r->next_by_priority) r->next_by_priority->prev_by_priority = r->prev_by_priority;

	/* Remove the instance from the dirty list */

	if (dirty_list == r) {
		dirty_list = r->next_dirty;
    } else if (r->is_dirty) {
		INSTANCE * i = dirty_list;
		while (i) {
			if (i->next_dirty == r) {
				i->next_dirty = r->next_dirty;
				break;
			}
			i = i->next_dirty;
		}
	}

	if (r->stack) free (r->stack) ;

	/* loc/pub/pri live in the VM arena; do not pass them to libc free */
	free (r) ;
}

/*
 *  FUNCTION : instance_update_bbox
 *
 *  Update the internal bounding box local variables in a instance
 *  (Offsets BBOX_X0, BBOX_Y0, BBOX_X1, BBOX_Y1).
 *
 *  PARAMS :
 *      i			Pointer to the instance
 *
 *  RETURN VALUE :
 *      None
 */

void instance_update_bbox (INSTANCE * i)
{
	REGION dest, * r ;
	GRAPH * gr ;
	int x, y, n, * ptr ;
	int scalex, scaley ;

	gr = instance_graph (i) ;

	if (!gr) return ;

	if (LOCDWORD(i,REGIONID) >= 0 && LOCDWORD(i,REGIONID) <= 31)
		r = &regions[LOCDWORD(i,REGIONID)] ;
	else
		r = &regions[0] ;

	x = LOCDWORD(i, COORDX) ;
	y = LOCDWORD(i, COORDY) ;

    RESOLXY(i, x, y);

	scalex = LOCDWORD(i,GRAPHSIZEX);
	scaley = LOCDWORD(i,GRAPHSIZEY);
	if (scalex == 100 && scaley == 100)
	    scalex = scaley = LOCDWORD(i,GRAPHSIZE);

	gr_get_bbox (&dest, r, x, y, LOCDWORD(i,FLAGS) ^ LOCDWORD(i,XGRAPH_FLAGS),
                        		 (LOCDWORD(i,XGRAPH)) ? 0 : LOCDWORD(i,ANGLE),
                        		 scalex, scaley, gr) ;

	if (LOCDWORD(i, CTYPE) == 1)	/* c_scroll */
	{
		n = 0 ;
		if (LOCDWORD(i, CFLAGS))
		{
			while (!(LOCDWORD(i, CFLAGS) & (1 << n))) n++ ;
			n++ ;
		}
		ptr = &GLODWORD(SCROLLS) + 20 * n ;
		dest.x  -= ptr[0] ;
		dest.y  -= ptr[1] ;
		dest.x2 -= ptr[0] ;
		dest.y2 -= ptr[1] ;
	}

	LOCDWORD(i, BOX_X0) = dest.x ;
	LOCDWORD(i, BOX_Y0) = dest.y ;
	LOCDWORD(i, BOX_X1) = dest.x2 ;
	LOCDWORD(i, BOX_Y1) = dest.y2 ;
}

/*
 *  FUNCTION : instance_visible
 *
 *  Returns 1 if the instance is visible (has a valid visible graphic)
 *
 *  PARAMS :
 *      i			Pointer to the instance
 *
 *  RETURN VALUE :
 *      1 if the instance is visible, 0 otherwise
 */

int instance_visible (INSTANCE * i)
{
	if ((LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK) == STATUS_SLEEPING ||
	    (LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK) == STATUS_RUNNING)
	{
		if (instance_graph(i)) return 1;
	}

	return 0;
}

/*
 *  FUNCTION : instance_posupdate
 *
 *  Updates the internal position variables of the instance. These variables
 *  are used to detect any aspect change in a instance. Use
 *  instance_poschanged() to compare current position with the stored one.
 *
 *  PARAMS :
 *      i			Pointer to the instance
 *
 *  RETURN VALUE :
 *      None
 */

void instance_posupdate (INSTANCE * i)
{
	LOCDWORD(i,SAVED_X)       = LOCDWORD(i,COORDX);
	LOCDWORD(i,SAVED_Y)       = LOCDWORD(i,COORDY);
	LOCDWORD(i,SAVED_GRAPH)   = LOCDWORD(i,GRAPHID);
	LOCDWORD(i,SAVED_ANGLE)   = LOCDWORD(i,ANGLE);
	LOCDWORD(i,SAVED_ALPHA)   = LOCDWORD(i,ALPHA);
	LOCDWORD(i,SAVED_BLENDOP) = LOCDWORD(i,BLENDOP);
	LOCDWORD(i,SAVED_SIZE)    = LOCDWORD(i,GRAPHSIZE);
	LOCDWORD(i,SAVED_SIZEX)   = LOCDWORD(i,GRAPHSIZEX);
	LOCDWORD(i,SAVED_SIZEY)   = LOCDWORD(i,GRAPHSIZEY);
	LOCDWORD(i,SAVED_FLAGS)   = LOCDWORD(i,FLAGS);
	LOCDWORD(i,SAVED_FILE)    = LOCDWORD(i,FILEID);
	LOCDWORD(i,SAVED_XGRAPH)  = LOCDWORD(i,XGRAPH);
}

/*
 *  FUNCTION : instance_poschanged
 *
 *  Compares the internal position variables of the instance with its
 *  currents values, and returns 1 if there is any difference. Used
 *  to detect changes in a visible process's aspect or position.
 *
 *  PARAMS :
 *      i			Pointer to the instance
 *
 *  RETURN VALUE :
 *      1 if there is any change, 0 otherwise
 */

int instance_poschanged (INSTANCE * i)
{
	GRAPH * graph = instance_graph(i);

	if (graph && (graph->modified || (graph->frames > 1 && graph->next_time < current_time)))
	    return 1;

	return  LOCDWORD(i,SAVED_X)       != LOCDWORD(i,COORDX)		||
    		LOCDWORD(i,SAVED_Y)       != LOCDWORD(i,COORDY)		||
    		LOCDWORD(i,SAVED_GRAPH)   != LOCDWORD(i,GRAPHID)	||
    		LOCDWORD(i,SAVED_ANGLE)   != LOCDWORD(i,ANGLE)		||
    		LOCDWORD(i,SAVED_ALPHA)   != LOCDWORD(i,ALPHA)		||
    		LOCDWORD(i,SAVED_BLENDOP) != LOCDWORD(i,BLENDOP)	||
    		LOCDWORD(i,SAVED_SIZE)    != LOCDWORD(i,GRAPHSIZE)	||
    		LOCDWORD(i,SAVED_SIZEX)   != LOCDWORD(i,GRAPHSIZEX) ||
    		LOCDWORD(i,SAVED_SIZEY)   != LOCDWORD(i,GRAPHSIZEY) ||
    		LOCDWORD(i,SAVED_FLAGS)   != LOCDWORD(i,FLAGS)		||
    		LOCDWORD(i,SAVED_FILE)	  != LOCDWORD(i,FILEID)		||
    		LOCDWORD(i,SAVED_XGRAPH)  != LOCDWORD(i,XGRAPH)		||
    		LOCDWORD(i,PREV_Z)        != LOCDWORD(i,COORDZ)		;
}

/*
 *  FUNCTION : instance_graph
 *
 *  Returns the instance graphic or NULL if there is none
 *
 *  PARAMS :
 *      i			Pointer to the instance
 *
 *  RETURN VALUE :
 *      Pointer to the graphic or NULL if none
 */

GRAPH * instance_graph (INSTANCE * i)
{
	int * xgraph, c, a ;

	if (LOCDWORD(i,XGRAPH))
	{
		xgraph = (int *) vm_ptr (i, LOCDWORD(i,XGRAPH)) ;
		if (!xgraph) return 0 ;
		c = *xgraph++;
		if (c)
		{
			a = LOCDWORD(i,ANGLE) % 360000 ;
			if (a < 0) a += 360000 ;
			c = xgraph[a*c/360000] ;
			LOCDWORD(i,XGRAPH_FLAGS) = 0;
			if (c < 0)
			{
				c = -c;
				LOCDWORD(i, XGRAPH_FLAGS) = B_HMIRROR;
			}
			return bitmap_get(LOCDWORD(i,FILEID), c) ;
		}
	}

	if (LOCDWORD(i,GRAPHID)) return bitmap_get(LOCDWORD(i,FILEID), LOCDWORD(i,GRAPHID)) ;

	return 0 ;
}

/*
 *  FUNCTION : instance_exists
 *
 *  Given an instance pointer, returns TRUE if it is still valid.
 *
 *  PARAMS :
 *      i				Pointer to the instance
 *
 *  RETURN VALUE :
 *      1 if the instance pointer is in the global instance list, 0 otherwise
 */

int instance_exists (INSTANCE * i)
{
	INSTANCE * pos = first_instance;

	while (pos)
	{
		if (pos == i) return 1;
		pos = pos->next;
	}
	return 0;
}

/*
 *  FUNCTION : instance_next_by_priority
 *
 *  Gets the next instance pointer until no more instances are
 *  returned. Instances are returned sorted by priority.
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      Pointer to the next priority on the list or NULL
 *		if there is no more instances. The next call in this
 *		case will return a pointer to the first instance
 *		(the one with the lower priority)
 */

INSTANCE * instance_next_by_priority()
{
	INSTANCE * i = NULL ;
	INSTANCE * j = NULL ;
	INSTANCE * best_prev = NULL ;
	INSTANCE * best_next = NULL ;
	if (!iterator_by_priority) {
		// NULL will be returned once and then the list will be reset

		if (!iterator_reset) {
			iterator_reset = 1;
			return NULL;
		}
		iterator_reset = 0;

		// Add all dirty instances to its place at the list

		i = dirty_list;
		while (i != NULL) {
			// Check the priority value

			if (LOCDWORD(i, PRIORITY) < MIN_PRIORITY) LOCDWORD(i, PRIORITY) = MIN_PRIORITY;
			if (LOCDWORD(i, PRIORITY) > MAX_PRIORITY) LOCDWORD(i, PRIORITY) = MAX_PRIORITY;

			// Remove the instance from the list

			if (i->prev_by_priority) i->prev_by_priority->next_by_priority = i->next_by_priority;
			if (i->next_by_priority) i->next_by_priority->prev_by_priority = i->prev_by_priority;

			if (first_by_priority == i) first_by_priority = i->next_by_priority;

			// Add the instance to the list. The easy case is when there is
			// already some instance with the same priority.

			j = first_by_priority;
			best_prev = best_next = NULL;

            i->prev_by_priority = NULL;
			i->next_by_priority = NULL;

			while (j) {
				if (LOCDWORD(j, PRIORITY) == LOCDWORD(i, PRIORITY)) {
					i->prev_by_priority = j;
					i->next_by_priority = j->next_by_priority;
					j->next_by_priority = i;

					if (i->next_by_priority) i->next_by_priority->prev_by_priority = i;
					break;
				} else if (LOCDWORD(j, PRIORITY) > LOCDWORD(i, PRIORITY)) {
					best_prev = j;
				} else if (!best_next) {
					best_next = j;
				}

				j = j->next_by_priority;
			}

			if (!j) {
				// No best case
				if (best_prev) {
					// But some instance was found with a lower priority

					i->next_by_priority = best_prev->next_by_priority;
					best_prev->next_by_priority = i;

					if (i->next_by_priority) i->next_by_priority->prev_by_priority = i;

				    i->prev_by_priority = best_prev; /* Splinter */
				} else if (best_next) {
					// But some instance was found with a higher priority

					i->prev_by_priority = best_next->prev_by_priority;
					best_next->prev_by_priority = i;

					if (i->prev_by_priority) {
						i->prev_by_priority->next_by_priority = i;
					} else {
						first_by_priority = i;
					}

				    i->next_by_priority = best_next; /* Splinter */
				} else {
					// There are no instances in the list

					first_by_priority   = i;
				}
			}
			i->is_dirty = 0;
			i = i->next_dirty;
		}

		// Reset the dirty list

		dirty_list = NULL;

		if (first_by_priority)
		    iterator_by_priority = first_by_priority->next_by_priority;
		else
		    iterator_by_priority = NULL;

		return first_by_priority;
	}

	i = iterator_by_priority;

	if (iterator_by_priority)
	    iterator_by_priority = iterator_by_priority->next_by_priority;

	return i;
}


/*
 *  FUNCTION : instance_dirty
 *
 *  Adds an instance to the dirty instances list. This is a list of all
 *  instances which priority changed since the last execution.
 *
 *  PARAMS :
 *      i				Pointer to the instance
 *
 *  RETURN VALUE :
 *      None
 */

void instance_dirty (INSTANCE * i)
{
	i->next_dirty = dirty_list;
	dirty_list = i;
}
