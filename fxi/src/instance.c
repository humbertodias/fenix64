/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.8
 *  Last stable release   :
 *  Project documentation : http://fenix.sourceforge.net
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
 * FILE        : i_func.h
 * DESCRIPTION : Implements FENIX language function handlers
 *
 * HISTORY:  0.80 - Solved leak problem in instance_destroy()
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <stdint.h>

#include "fxi.h"
#include "dcb.h"

extern int stack[2048] ;
extern PROCDEF * procs ;
extern int procdef_count ;

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
	INSTANCE * i ;
	unsigned n ;

	if (!p32) return NULL ;

	if (vm_base) {
		p = vm_from_base (vm_base, vm_arena_size, t) ;
		if (p) return p ;
	}

	p = vm_from_base (stack, sizeof(stack), t) ;
	if (p) return p ;

	if (globaldata)
	{
		p = vm_from_base (globaldata, (size_t)dcb.SGlobal + 4, t) ;
		if (p) return p ;
	}

	if (localdata)
	{
		p = vm_from_base (localdata, (size_t)local_size + 4, t) ;
		if (p) return p ;
	}

	if (dcb.glovar && dcb.NGloVars)
	{
		p = vm_from_base (dcb.glovar, sizeof(DCB_VAR) * dcb.NGloVars, t) ;
		if (p) return p ;
	}

	if (dcb.locvar && dcb.NLocVars)
	{
		p = vm_from_base (dcb.locvar, sizeof(DCB_VAR) * dcb.NLocVars, t) ;
		if (p) return p ;
	}

	if (dcb.varspace_vars)
	{
		for (n = 0 ; n < dcb.NVarSpaces ; n++)
		{
			if (!dcb.varspace_vars[n] || !dcb.varspace[n].NVars)
				continue ;
			p = vm_from_base (dcb.varspace_vars[n],
				sizeof(DCB_VAR) * dcb.varspace[n].NVars, t) ;
			if (p) return p ;
		}
	}

	if (dcb.proc)
	{
		for (n = 0 ; n < dcb.NProcs ; n++)
		{
			if (!dcb.proc[n].privar || !dcb.proc[n].NPriVars)
				continue ;
			p = vm_from_base (dcb.proc[n].privar,
				sizeof(DCB_VAR) * dcb.proc[n].NPriVars, t) ;
			if (p) return p ;
		}
	}

	if (procs)
	{
		for (n = 0 ; n < (unsigned)procdef_count ; n++)
		{
			if (!procs[n].pridata)
				continue ;
			p = vm_from_base (procs[n].pridata,
				(size_t)procs[n].private_size + 4, t) ;
			if (p) return p ;
		}
	}

	if (my)
	{
		if (my->pridata)
		{
			p = vm_from_base (my->pridata, (size_t)my->private_size + 4, t) ;
			if (p) return p ;
		}
		if (my->locdata)
		{
			p = vm_from_base (my->locdata, (size_t)local_size + 4, t) ;
			if (p) return p ;
		}
	}

	for (i = first_instance ; i ; i = i->next)
	{
		if (i == my) continue ;
		if (i->pridata)
		{
			p = vm_from_base (i->pridata, (size_t)i->private_size + 4, t) ;
			if (p) return p ;
		}
		if (i->locdata)
		{
			p = vm_from_base (i->locdata, (size_t)local_size + 4, t) ;
			if (p) return p ;
		}
	}

	return (void *)(uintptr_t) t ;
}

/* ---------------------------------------------------------------------- */
/* Módulo de gestión de instancias, con las funciones de incialización y  */
/* destrucción, duplicado, etc.                                           */
/* ---------------------------------------------------------------------- */

INSTANCE * first_instance = 0 ;
INSTANCE * last_instance  = 0 ;

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

	r->pridata = (int *) vm_malloc (father->private_size + 4) ;
	r->locdata = (int *) vm_malloc (local_size + 4) ;
	r->code    = father->code ;
	r->codeptr = father->codeptr ;
	r->proc    = father->proc ;

	r->private_size = father->private_size ;

	memcpy (r->pridata, father->pridata, father->private_size) ;
	memcpy (r->locdata, father->locdata, local_size) ;

	/* Actualiza las cuentas de uso de las cadenas */

	for (n = 0 ; n < r->proc->string_count ; n++)
		string_use (PRIDWORD(r, r->proc->strings[n])) ;

	for (n = 0 ; n < local_strings ; n++)
		string_use (LOCDWORD(r, localstr[n])) ;

	/* Inicializa datos DIV */

	/* Crea el proceso clónico como si lo hubiera llamado el padre */
	/* No sé si es eso lo que hace DIV */

	LOCDWORD(r, PROCESS_ID)   = instance_getid() ;
	LOCDWORD(r, FATHER)       = LOCDWORD(father,PROCESS_ID) ;
	LOCDWORD(r, SON)          = 0 ;
	LOCDWORD(r, SMALLBRO)     = 0 ;
	LOCDWORD(r, BIGBRO)       = LOCDWORD(father,SON) ;

	brother = instance_get (LOCDWORD(father, SON)) ;
	if (brother) LOCDWORD(brother, SMALLBRO) = LOCDWORD(r,PROCESS_ID) ;

	LOCDWORD(father, SON)     = LOCDWORD(r,PROCESS_ID) ;

	/* Añade la instancia al final de la lista */

	r->next    = 0 ;
	r->prev    = last_instance ;
	if (r->prev) r->prev->next = r ;

	last_instance = r ;
	if (!first_instance) first_instance = r ;
	
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
	INSTANCE * r ;
	INSTANCE * brother ;
	int n ;

	r = (INSTANCE *) malloc (sizeof(INSTANCE)) ;
	assert (r != 0) ;

	r->pridata = (int *) vm_malloc (proc->private_size + 4) ;
	r->locdata = (int *) vm_malloc (local_size + 4) ;
	r->code    = proc->code ;
	r->codeptr = proc->code ;
	r->proc    = proc ;

	r->private_size = proc->private_size ;
	r->first_run    = 1 ;

	memcpy (r->pridata, proc->pridata, proc->private_size) ;
	memcpy (r->locdata, localdata, local_size) ;

	/* Inicializa datos DIV */

	LOCDWORD(r, PROCESS_ID)   = instance_getid() ;
	LOCDWORD(r, PROCESS_TYPE) = proc->type ;

	if (father)
	{
		LOCDWORD(r, FATHER)     = LOCDWORD(father, PROCESS_ID) ;
		brother = instance_get (LOCDWORD(father, SON)) ;
		if (brother)
		{
			LOCDWORD(r, BIGBRO)         = LOCDWORD(brother, PROCESS_ID) ;
			LOCDWORD(brother, SMALLBRO) = LOCDWORD(r, PROCESS_ID) ;
		}

		LOCDWORD(father, SON) = LOCDWORD(r, PROCESS_ID) ;
	}

	/* Cuenta los usos de las variables tipo cadena */

	for (n = 0 ; n < proc->string_count ; n++)
		string_use (PRIDWORD(r, proc->strings[n])) ;

	for (n = 0 ; n < local_strings ; n++)
		string_use (LOCDWORD(r, localstr[n])) ;

	/* Añade la instancia al final de la lista */

	r->next    = 0 ;
	r->prev    = last_instance ;
	if (r->prev) r->prev->next = r ;

	last_instance = r ;
	if (!first_instance) first_instance = r ;
	
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

INSTANCE * instance_get (int id)
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
	return instance_get (LOCDWORD(i, FATHER)) ;
}

INSTANCE * instance_getson (INSTANCE * i)
{
	return instance_get (LOCDWORD(i, SON)) ;
}

INSTANCE * instance_getbigbro (INSTANCE * i)
{
	return instance_get (LOCDWORD(i, BIGBRO)) ;
}

INSTANCE * instance_getsmallbro (INSTANCE * i)
{
	return instance_get (LOCDWORD(i, SMALLBRO)) ;
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
		if (i != except)
			instance_destroy (i) ;
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
 *      Integer value in milliseconds
 */

void instance_destroy (INSTANCE * r)
{
	INSTANCE * father, * brother, * son, * prime ;
	int n ;

	if (LOCDWORD(r, GRAPHID) != 0)
		object_list_dirty = 1;
	LOCDWORD(r,STATUS) = 2;
	if (LOCDWORD(r, BOX_X0) != -2)
		gr_mark_instance(r);

	/* Actualiza la cuenta de referencia de las variables tipo string */

	for (n = 0 ; n < r->proc->string_count ; n++)
		string_discard (PRIDWORD(r, r->proc->strings[n])) ;

	for (n = 0 ; n < local_strings ; n++)
		string_discard (LOCDWORD(r, localstr[n])) ;

	/* Quita la instancia de la lista */

	if (r->prev) r->prev->next  = r->next ;
	else         first_instance = r->next ;

	if (r->next) r->next->prev = r->prev ;
	else         last_instance = r->prev ;

	if (first_instance == r)
		first_instance = r->next ;

	/* Actualiza árbol DIV */

	brother = instance_get (LOCDWORD(r,BIGBRO)) ;
	if (brother) LOCDWORD(brother,SMALLBRO) = LOCDWORD(r,SMALLBRO) ;
	father = instance_get (LOCDWORD(r,FATHER)) ;
	if (father)
	{
		if (LOCDWORD(father, SON) == LOCDWORD(r, PROCESS_ID))
		{
			if (brother)
				LOCDWORD(father,SON) = LOCDWORD(brother,PROCESS_ID) ;
			else
				LOCDWORD(father,SON) = 0 ;
		}
	}

	brother = instance_get (LOCDWORD(r,SMALLBRO)) ;
	if (brother) LOCDWORD(brother,BIGBRO) = LOCDWORD(r,BIGBRO) ;

	son = instance_get (LOCDWORD(r,SON)) ;
	if (son)
	{
		/* Busca el primer y el último hijo */

		brother = son ;
		while (LOCDWORD(son,BIGBRO))
		{
			son = instance_get (LOCDWORD(son,BIGBRO)) ;
			assert (son != 0) ;
		}
		while (LOCDWORD(brother,SMALLBRO))
		{
			brother = instance_get (LOCDWORD(son,SMALLBRO)) ;
			assert (brother != 0) ;
		}

		/* Los procesos han quedado huérfanos; asígnalos al padre */

		prime = first_instance ;
		assert (prime != 0) ;

		while (son)
		{
			LOCDWORD(son,FATHER) = LOCDWORD(r,FATHER) ;
			son = instance_get (LOCDWORD(son,SMALLBRO)) ;
		}
	}
	
	/* loc/pri live in the VM arena; do not pass them to libc free */
	if (!vm_in_arena (r->locdata)) free (r->locdata) ;
	if (!vm_in_arena (r->pridata)) free (r->pridata) ;
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
 *      Integer value in milliseconds
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
	if (LOCDWORD(i, RESOLUTION))
	{
		x /= LOCDWORD(i, RESOLUTION) ;
		y /= LOCDWORD(i, RESOLUTION) ;
	}

	scalex = LOCDWORD(i,GRAPHSIZEX);
	scaley = LOCDWORD(i,GRAPHSIZEY);
	if (scalex == 100 && scaley == 100) 
		scalex = scaley = LOCDWORD(i,GRAPHSIZE);

	gr_get_bbox (&dest, r, x, y, LOCDWORD(i,FLAGS), 
		LOCDWORD(i,ANGLE), scalex, scaley, gr) ;

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
	if (LOCDWORD(i,STATUS) == STATUS_SLEEPING || LOCDWORD(i,STATUS) == STATUS_RUNNING)
	{
		if (instance_graph(i))
			return 1;
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

	if (graph && graph->modified)
		return 1;

	return
	    LOCDWORD(i,SAVED_X)       != LOCDWORD(i,COORDX)		||
		LOCDWORD(i,SAVED_Y)       != LOCDWORD(i,COORDY)		||
		LOCDWORD(i,SAVED_GRAPH)   != LOCDWORD(i,GRAPHID)	||
		LOCDWORD(i,SAVED_ANGLE)   != LOCDWORD(i,ANGLE)		||
		LOCDWORD(i,SAVED_ALPHA)   != LOCDWORD(i,ALPHA)		||
		LOCDWORD(i,SAVED_BLENDOP) != LOCDWORD(i,BLENDOP)	||
		LOCDWORD(i,SAVED_SIZE)    != LOCDWORD(i,GRAPHSIZE)	||
		LOCDWORD(i,SAVED_SIZEX)   != LOCDWORD(i,GRAPHSIZEX) ||
		LOCDWORD(i,SAVED_SIZEY)   != LOCDWORD(i,GRAPHSIZEY) ||
		LOCDWORD(i,SAVED_FLAGS)   != LOCDWORD(i,FLAGS)		;
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
		c      = *xgraph++ ;
		if (c)
		{
			a = LOCDWORD(i,ANGLE) % 360000 ;
			if (a < 0) a += 360000 ;
			c = xgraph[a*c/360000] ;
			return bitmap_get (LOCDWORD(i,FILEID), c) ;
		}
	}

	if (LOCDWORD(i,GRAPHID))
		return bitmap_get (LOCDWORD(i,FILEID), LOCDWORD(i,GRAPHID)) ;

	return 0 ;
}