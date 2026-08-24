/* Fenix - Compilador/intérprete de videojuegos
 * Copyright (C) 1999 José Luis Cebrián Pagüe
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * HISTORY: Eliminated string_internal as stated in the new strings lib thugh
 * full strings library is not yet implemented.
 */

#include <stdio.h>
#include <stdlib.h>
#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <string.h>

#include "fxi.h"
#include "dcb.h"

#define VA(p) vm_ptr(r, (int)(p))

/* ---------------------------------------------------------------------- */
/* Módulo principal del intérprete: función de ejecución de bloques       */
/* ---------------------------------------------------------------------- */

int must_exit = 0 ;

int stack[2048] ;
int * stack_gptr = stack ;

char * stack_dump()
{
	static char buffer[2048] ;
	int * ptr = stack  ;

	buffer[0] = 0 ;

	while (ptr < stack_gptr)
	{
		sprintf (buffer + strlen(buffer), "%08X ", *ptr++) ;
		if (ptr > stack_gptr + 4)
		{
			strcat (buffer, " ...") ;
			break ;
		}
	}
	return buffer ;
}

static int compare_instances (const void * ptr1, const void * ptr2)
{
	INSTANCE * i1 = *( (INSTANCE * *)ptr1 ) ;
	INSTANCE * i2 = *( (INSTANCE * *)ptr2 ) ;

	return LOCDWORD(i2,PRIORITY) - LOCDWORD(i1,PRIORITY) ;
}

void instance_go_all ()
{
	INSTANCE * i, * next ;
	INSTANCE * * ilist = 0 ;
	int i_reserved = 0 ;
	int i_count = 0 ;
	int n ;

	must_exit = 0 ;

	gprof_begin ("Interpreter");

	while (first_instance)
	{
		if (LOCDWORD(first_instance,STATUS) == 0 && 
		    !first_instance->next)
			break ;

		/* Crea una lista de procesos pendientes */

		i = first_instance ;
		i_count = 0 ;
		while (i)
		{
			/* Ignora procesos detenidos o dormidos */

			if (LOCDWORD(i,STATUS) != 2)
			{
				i = i->next ;
				continue ;
			}
			
			/* Cuenta procesos con frame no completado */

			if (LOCDWORD(i,FRAME_PERCENT) < 100)
			{
				if (i_reserved == i_count)
				{
					i_reserved += 16 ;
					ilist = (INSTANCE **) realloc(ilist, sizeof(INSTANCE*) * i_reserved) ;
					assert (ilist) ;
				}

				/* Inicializa colision antes de ejecutar */

				LOCDWORD(i,TYPE_SCAN) = 0 ;
				LOCDWORD(i,ID_SCAN) = 0 ;

				ilist[i_count++] = i ;
			}
			i = i->next ;
		}

		/* Si no hay nada pendiente: Dibujar, actualizar variables, etc. */

		if (!i_count)
		{
			/* Borra procesos tocados por s_kill */

			i = first_instance ;
			while (i)
			{
				next = i->next ;
				if (LOCDWORD(i,STATUS) == 1)
					instance_destroy (i) ;
				i = next ;
			}
			if (!first_instance) break ;

			/* Dibuja el frame */

			gprof_frame();
			gprof_begin ("Waiting for next frame");
				gr_wait_frame() ;
			gprof_end   ("Waiting for next frame");

			gprof_begin ("Drawing");
				gr_draw_frame() ;
			gprof_end   ("Drawing");

			gprof_begin ("Interpreter");
			
			/* Contabiliza los frame_percent */

			i = first_instance ;
			while (i)
			{
				if (LOCDWORD(i,STATUS) == 2)
				{
					LOCDWORD(i,FRAME_PERCENT) -= 100 ;
				}
				i = i->next ;
			}

			gr_advance_timers() ;

			if (must_exit) break ;

			continue ;
		}

		/* Ordena las instancias pendientes, por prioridad */

		qsort (ilist, i_count, sizeof(INSTANCE *), compare_instances) ;

		/* Ejecuta uno a uno todos los procesos */

		for (n = 0 ; n < i_count ; n++)
			if (LOCDWORD(ilist[n],STATUS) == 2)
				instance_go (ilist[n]) ;

		/* NOTA: los procesos pueden haberse autodestruido tras su ejecución */
	}
	if (ilist) free(ilist) ;
//	printf ("\n") ;

	gprof_end ("Interpreter");
}

int instance_go (INSTANCE * r)
{
	register int * ptr = r->codeptr ;
	register int * stack_ptr = stack_gptr ;

	int n ;
	int return_value = LOCDWORD(r, PROCESS_ID) ;
	int was_visible;
	SYSPROC * p ;
	INSTANCE * i ;
	PROCDEF * proc ;
	static char buffer[16], * str ;

	int switchval, switchval_string = 0, cased ;

	if (!r) return 0 ;
	if (debug) printf ("***** INSTANCE %d ENTRY\n", LOCDWORD(r,PROCESS_ID)) ;

	gprof_begin (getid(r->proc->id));

	was_visible = instance_visible(r);

	for (;;)
	{
#ifdef DEBUG
	if (debug)
	{
		stack_gptr = stack_ptr ;
		printf ("%-40s [%4d] ", stack_dump(), 
				(ptr - r->code)) ;
		mnemonic_dump (*ptr, ptr[1]) ;
		fflush(stdout) ;
	}
#endif

	switch (*ptr)
	{
		/* Manipulación de la pila */

		case MN_DUP:
			*stack_ptr = *(stack_ptr-1) ;
			stack_ptr++ ;
			ptr++ ;
			break ;

		case MN_PUSH:
		case MN_PUSH | MN_FLOAT:
			*stack_ptr++ = ptr[1] ;
			ptr += 2 ;
			break ;

		case MN_POP:
			stack_ptr-- ;
			ptr++ ;
			break ;

		case MN_INDEX:
			stack_ptr[-1] += ptr[1] ;
			ptr += 2 ;
			break ;

		case MN_ARRAY:
			stack_ptr[-2] += (ptr[1] * stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr += 2 ;
			break ;

		/* Llamadas a procesos */

		case MN_CLONE:
			i = instance_duplicate (r) ;
			i->codeptr = ptr+2 ;
			ptr = r->code + ptr[1] ;
			continue ;

		case MN_CALL:
			proc = procdef_get (ptr[1]) ;
			if (!proc) gr_error ("Error: Procedimiento desconocido\n") ;
			i = instance_new (proc, r) ;
			for (n = 0 ; n < proc->params ; n++)
				PRIDWORD(i,4*n) = stack_ptr[-proc->params+n] ;
			stack_ptr -= proc->params ;
			stack_gptr = stack_ptr ;
			gprof_end (getid(r->proc->id));
			*stack_ptr++ = instance_go (i) ;
			gprof_begin (getid(r->proc->id));
			ptr += 2 ;
			break ;

		case MN_PROC:
			proc = procdef_get (ptr[1]) ;
			if (!proc) gr_error ("Error: Procedimiento desconocido\n") ;
			i = instance_new (proc, r) ;
			for (n = 0 ; n < proc->params ; n++)
				PRIDWORD(i,4*n) = stack_ptr[-proc->params+n] ;
			stack_ptr -= proc->params ;

			/* Esto puede que no se haga así en DIV */
			stack_gptr = stack_ptr ;
			gprof_end (getid(r->proc->id));
			instance_go (i) ;
			gprof_begin (getid(r->proc->id));
			if (debug) printf ("***** INSTANCE %d RETURN\n", LOCDWORD(r,PROCESS_ID)) ;
			ptr += 2 ;
			break ;

		case MN_SYSCALL:
			p = sysproc_get (ptr[1]) ;
			if (!p) gr_error ("Error: Función del sistema desconocida\n") ;
			stack_ptr -= p->params ;
			*stack_ptr = (*p->func) (r, stack_ptr) ;
			stack_ptr++ ;
			ptr += 2 ;
			break ;

		case MN_SYSPROC:
			p = sysproc_get (ptr[1]) ;
			if (!p) gr_error ("Error: Procedimiento del sistema desconocido\n") ;
			stack_ptr -= p->params ;
			(*p->func) (r, stack_ptr) ;
			ptr += 2 ;
			break ;

		/* Acceso a variables tipo DWORD */

		case MN_PRIVATE:
			*stack_ptr++ = (int) &PRIDWORD(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_LOCAL:
			*stack_ptr++ = (int) &LOCDWORD(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_GLOBAL:
			*stack_ptr++ = (int) &GLODWORD(ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_REMOTE:
			i = instance_get (stack_ptr[-1]) ;
			if (i == 0)
				gr_error ("Error de ejecucion en proceso %d:\nProcedimiento %d no activo\n"
					  , LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
			else
				stack_ptr[-1] = (int) &LOCDWORD(i, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_GET_PRIV:
		case MN_GET_PRIV | MN_FLOAT:
			*stack_ptr++ = PRIDWORD(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_GET_LOCAL:
		case MN_GET_LOCAL | MN_FLOAT:
			*stack_ptr++ = LOCDWORD(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_GET_GLOBAL:
		case MN_GET_GLOBAL | MN_FLOAT:
			*stack_ptr++ = GLODWORD(ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_GET_REMOTE:
		case MN_GET_REMOTE | MN_FLOAT:
			i = instance_get (stack_ptr[-1]) ;
			if (i == 0)
				gr_error ("Error de ejecucion en proceso %d:\nProcedimiento %d no activo\n"
					  , LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
			else
				stack_ptr[-1] = LOCDWORD(i,ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_PTR:
		case MN_PTR | MN_FLOAT:
			stack_ptr[-1] = *(Sint32 *)VA(stack_ptr[-1]) ;
			ptr++ ;
			break ;

		/* Acceso a variables tipo STRING */

		case MN_PUSH | MN_STRING:
			*stack_ptr++ = ptr[1] ;
			string_use ( stack_ptr[-1] ) ;
			ptr += 2 ;
			break ;
		case MN_GET_PRIV | MN_STRING:
			*stack_ptr++ = PRIDWORD(r, ptr[1]) ;
			string_use     ( stack_ptr[-1] ) ;
			ptr += 2 ;
			break ;
		case MN_GET_LOCAL | MN_STRING:
			*stack_ptr++ = LOCDWORD(r, ptr[1]) ;
			string_use     ( stack_ptr[-1] ) ;
			ptr += 2 ;
			break ;
		case MN_GET_GLOBAL | MN_STRING:
			*stack_ptr++ = GLODWORD(ptr[1]) ;
			string_use     ( stack_ptr[-1] ) ;
			ptr += 2 ;
			break ;
		case MN_GET_REMOTE | MN_STRING:
			i = instance_get (stack_ptr[-1]) ;
			if (i == 0)
				gr_error ("Error de ejecucion en proceso %d:\nProcedimiento %d no activo\n"
					  , LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
			else
				stack_ptr[-1] = LOCDWORD(i,ptr[1]) ;
			string_use     ( stack_ptr[-1] ) ;
			ptr += 2 ;
			break ;
		case MN_STRING | MN_PTR:
			stack_ptr[-1] = *(Sint32 *)VA(stack_ptr[-1]) ;
			string_use     ( stack_ptr[-1] ) ;
			ptr++ ;
			break ;
		case MN_STRING | MN_POP:
			string_discard ( stack_ptr[-1] ) ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Acceso a variables tipo WORD */

		case MN_WORD | MN_GET_PRIV:
			*stack_ptr++ = PRIWORD(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_WORD | MN_GET_LOCAL:
			*stack_ptr++ = LOCWORD(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_WORD | MN_GET_GLOBAL:
			*stack_ptr++ = GLOWORD(ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_WORD | MN_GET_REMOTE:
			i = instance_get (stack_ptr[-1]) ;
			if (i == 0)
				gr_error ("Error de ejecucion en proceso %d:\nProcedimiento %d no activo\n"
					  , LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
			else
				stack_ptr[-1] = LOCWORD(i,ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_WORD | MN_PTR:
			stack_ptr[-1] = *(Sint16 *)VA(stack_ptr[-1]) ;
			ptr++ ;
			break ;
		// Added... now !<word_var> works fine
		case MN_WORD | MN_NOT:
			stack_ptr[-1] = ~(stack_ptr[-1]) ;
			ptr++ ;
			break ;

		/* Acceso a variables tipo BYTE */

		case MN_BYTE | MN_GET_PRIV:
			*stack_ptr++ = PRIBYTE(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_BYTE | MN_GET_LOCAL:
			*stack_ptr++ = LOCBYTE(r, ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_BYTE | MN_GET_GLOBAL:
			*stack_ptr++ = GLOBYTE(ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_BYTE | MN_GET_REMOTE:
			i = instance_get (stack_ptr[-1]) ;
			if (i == 0)
				gr_error ("Error de ejecucion en proceso %d:\nProcedimiento %d no activo\n"
					  , LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
			else
				stack_ptr[-1] = LOCBYTE(i,ptr[1]) ;
			ptr += 2 ;
			break ;
		case MN_BYTE | MN_PTR:
			stack_ptr[-1] = *(Uint8 *)VA(stack_ptr[-1]) ;
			ptr++ ;
			break ;
		// Added... now !<byte_var> works fine
		case MN_BYTE | MN_NOT:
			stack_ptr[-1] = ~(stack_ptr[-1]) ;
			ptr++ ;
			break ;

		/* Operaciones matemáticas  en coma floatante */

		case MN_FLOAT | MN_NEG:
			*(float *)&stack_ptr[-1] = -*(float *)&stack_ptr[-1] ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_NOT:
			*(float *)&stack_ptr[-1] = (float) !*(float *)&stack_ptr[-1] ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_ADD:
			*(float *)&stack_ptr[-2] += *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_SUB:
			*(float *)&stack_ptr[-2] -= *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_MUL:
			*(float *)&stack_ptr[-2] *= *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_DIV:
			if (*(float *)&stack_ptr[-1] == 0.0)
				gr_error ("Error: Division por cero\n") ;
			*(float *)&stack_ptr[-2] /= *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT2INT:
			*(Sint32 *)&stack_ptr[-ptr[1]-1] = (Sint32) *(float *)&stack_ptr[-ptr[1]-1] ;
			ptr+=2 ;
			break ;
		case MN_INT2FLOAT:
			*(float *)&stack_ptr[-ptr[1]-1] = (float) *(Sint32 *)&stack_ptr[-ptr[1]-1] ;
			ptr+=2 ;
			break ;

		/* Operaciones matemáticas */

		case MN_NEG:
			stack_ptr[-1] = -stack_ptr[-1] ;
			ptr++ ;
			break ;
		case MN_NOT:
			stack_ptr[-1] = ~(stack_ptr[-1]) ;
			ptr++ ;
			break ;
		case MN_ADD:
			stack_ptr[-2] += stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_SUB:
			stack_ptr[-2] -= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_MUL | MN_WORD:
		case MN_MUL| MN_BYTE:
		case MN_MUL:
			stack_ptr[-2] *= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_DIV | MN_WORD:
		case MN_DIV | MN_BYTE:
		case MN_DIV:
			if (stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			stack_ptr[-2] /= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_MOD | MN_WORD:
		case MN_MOD | MN_BYTE:
		case MN_MOD:
			if (stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			stack_ptr[-2] %= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Operaciones a nivel de bit */
			
		case MN_ROR:
			stack_ptr[-2] >>= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_ROL:
			stack_ptr[-2] <<= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_AND:
			stack_ptr[-2] &= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_OR:
			stack_ptr[-2] |= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_XOR:
			stack_ptr[-2] ^= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Comparaciones */

		case MN_EQ:
			stack_ptr[-2] = (stack_ptr[-2] == stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_NE:
			stack_ptr[-2] = (stack_ptr[-2] != stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_GTE:
			stack_ptr[-2] = (stack_ptr[-2] >= stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_LTE:
			stack_ptr[-2] = (stack_ptr[-2] <= stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_LT:
			stack_ptr[-2] = (stack_ptr[-2] < stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_GT:
			stack_ptr[-2] = (stack_ptr[-2] > stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Comparaciones con floats */

		case MN_EQ | MN_FLOAT:
			stack_ptr[-2] = (*(float *)&stack_ptr[-2] == *(float *)&stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_NE | MN_FLOAT:
			stack_ptr[-2] = (*(float *)&stack_ptr[-2] != *(float *)&stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_GTE | MN_FLOAT:
			stack_ptr[-2] = (*(float *)&stack_ptr[-2] >= *(float *)&stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_LTE | MN_FLOAT:
			stack_ptr[-2] = (*(float *)&stack_ptr[-2] <= *(float *)&stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_LT | MN_FLOAT:
			stack_ptr[-2] = (*(float *)&stack_ptr[-2] < *(float *)&stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_GT | MN_FLOAT:
			stack_ptr[-2] = (*(float *)&stack_ptr[-2] > *(float *)&stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Comparaciones con cadenas */

		case MN_EQ | MN_STRING :
			n = string_comp (stack_ptr[-2], stack_ptr[-1]) == 0 ;
			string_discard (stack_ptr[-2]);
			string_discard (stack_ptr[-1]);
			stack_ptr[-2] = n;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_NE | MN_STRING :
			n = string_comp (stack_ptr[-2], stack_ptr[-1]) != 0 ;
			string_discard (stack_ptr[-2]);
			string_discard (stack_ptr[-1]);
			stack_ptr[-2] = n;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_GTE | MN_STRING :
			n = string_comp (stack_ptr[-2], stack_ptr[-1]) >= 0 ;
			string_discard (stack_ptr[-2]);
			string_discard (stack_ptr[-1]);
			stack_ptr[-2] = n;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_LTE | MN_STRING :
			n = string_comp (stack_ptr[-2], stack_ptr[-1]) <= 0 ;
			string_discard (stack_ptr[-2]);
			string_discard (stack_ptr[-1]);
			stack_ptr[-2] = n;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_LT | MN_STRING :
			n = string_comp (stack_ptr[-2], stack_ptr[-1]) <  0 ;
			string_discard (stack_ptr[-2]);
			string_discard (stack_ptr[-1]);
			stack_ptr[-2] = n;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_GT | MN_STRING :
			n = string_comp (stack_ptr[-2], stack_ptr[-1]) >  0 ;
			string_discard (stack_ptr[-2]);
			string_discard (stack_ptr[-1]);
			stack_ptr[-2] = n;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Operaciones con cadenas */

		case MN_VARADD | MN_STRING:
			n = *(Sint32 *)VA(stack_ptr[-2]) ;
			*(Sint32 *)VA(stack_ptr[-2]) = string_add (n, stack_ptr[-1]) ;
			string_use ( *(Sint32 *)VA(stack_ptr[-2]) ) ;
			string_discard (n) ;
			string_discard (stack_ptr[-1]) ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		case MN_LET | MN_STRING:
			string_discard ( *(Sint32 *)VA(stack_ptr[-2]) ) ;
			(*(Sint32 *)VA(stack_ptr[-2])) = stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		case MN_ADD | MN_STRING:
			n = string_add (stack_ptr[-2], stack_ptr[-1]) ;
			string_use     ( n ) ;
			string_discard ( stack_ptr[-2] ) ;
			string_discard ( stack_ptr[-1] ) ;
			stack_ptr-- ;
			stack_ptr[-1] = n ;
			ptr++ ;
			break ;

		case MN_INT2STR:
			stack_ptr[-ptr[1]-1] = string_itoa(stack_ptr[-ptr[1]-1]) ;
			string_use (stack_ptr[-ptr[1]-1]) ;
			ptr += 2 ;
			break ;

		case MN_FLOAT2STR:
			stack_ptr[-ptr[1]-1] = string_ftoa(*(float *)&stack_ptr[-ptr[1]-1]) ;
			string_use (stack_ptr[-ptr[1]-1]) ;
			ptr += 2 ;
			break ;

		case MN_CHR2STR:
			buffer[0] = (Uint8)stack_ptr[-ptr[1]-1] ;
			buffer[1] = 0 ;
			stack_ptr[-ptr[1]-1] = string_new(buffer) ;
			string_use (stack_ptr[-ptr[1]-1]) ;
			ptr += 2 ;
			break ;

		case MN_CHRSTR:
			n = string_char (stack_ptr[-2], stack_ptr[-1]) ;
			string_use     ( n ) ;
			string_discard ( stack_ptr[-2] ) ;
			stack_ptr-- ;
			stack_ptr[-1] = n ;
			ptr++ ;
			break ;

		case MN_POINTER2STR:
			stack_ptr[-ptr[1]-1] = string_ptoa(*(void * *)&stack_ptr[-ptr[1]-1]) ;
			string_use (stack_ptr[-ptr[1]-1]) ;
			ptr += 2 ;
			break ;

		case MN_POINTER2BOL:
			stack_ptr[-ptr[1]-1] = stack_ptr[-ptr[1]-1] ? 1:0 ;
			ptr += 2 ;
			break ;

		case MN_STR2FLOAT:
			n = stack_ptr[-ptr[1]-1] ; str = (char *)string_get(n) ;
			*(float *)(&stack_ptr[-ptr[1]-1]) = str ? (float)atof(str) : 0.0f ;
			string_discard (n) ;
			ptr += 2 ;
			break ;

		case MN_STR2INT:
			n = stack_ptr[-ptr[1]-1] ; str = (char *)string_get(n) ;
			stack_ptr[-ptr[1]-1] = str ? atoi(str) : 0 ;
			string_discard (n) ;
			ptr += 2 ;
			break ;

		/* Operaciones con cadenas de ancho fijo */

		case MN_A2STR:
			str = (char *)VA(stack_ptr[-ptr[1]-1]) ;
			n = string_new(str);
			string_use(n);
			stack_ptr[-ptr[1]-1] = n ;
			ptr+=2 ;
			break ;

		case MN_STR2A:
			n = stack_ptr[-1];
			strncpy ((char *)VA(stack_ptr[-2]), string_get(n), ptr[1]) ;
			((char *)VA(stack_ptr[-2]))[ptr[1]] = 0;
			stack_ptr[-2] = stack_ptr[-1];
			stack_ptr--;
			ptr+=2 ;
			break ;

		case MN_STRACAT:
			n = stack_ptr[-1];
			strncat ((char *)VA(stack_ptr[-2]), string_get(n), 
				ptr[1] - strlen((char *)VA(stack_ptr[-2]))) ;
			((char *)VA(stack_ptr[-2]))[ptr[1]] = 0;
			stack_ptr[-2] = stack_ptr[-1];
			stack_ptr--;
			ptr+=2 ;
			break ;

		/* Operaciones directas con variables tipo DWORD */

		case MN_LETNP:
			(*(Sint32 *)VA(stack_ptr[-2])) = stack_ptr[-1] ;
			stack_ptr-=2 ;
			ptr++ ;
			break ;
		case MN_LET:
			(*(Sint32 *)VA(stack_ptr[-2])) = stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_INC:
			(*(Sint32 *)VA(stack_ptr[-1])) += ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_DEC:
			(*(Sint32 *)VA(stack_ptr[-1])) -= ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_POSTDEC:
			(*(Sint32 *)VA(stack_ptr[-1])) -= ptr[1] ;
			stack_ptr[-1] = *(Sint32 *)VA(stack_ptr[-1]) + ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_POSTINC:
			*((Sint32 *)VA(stack_ptr[-1])) += ptr[1] ;
			stack_ptr[-1] = *(Sint32 *)VA(stack_ptr[-1]) - ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_VARADD:
			*(Sint32 *)VA(stack_ptr[-2]) += stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARSUB:
			*(Sint32 *)VA(stack_ptr[-2]) -= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARMUL:
			*(Sint32 *)VA(stack_ptr[-2]) *= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARDIV:
			if (stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			*(Sint32 *)VA(stack_ptr[-2]) /= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARMOD:
			if (stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			*(Sint32 *)VA(stack_ptr[-2]) %= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VAROR:
			*(Sint32 *)VA(stack_ptr[-2]) |= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARXOR:
			*(Sint32 *)VA(stack_ptr[-2]) ^= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARAND:
			*(Sint32 *)VA(stack_ptr[-2]) &= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARROR:
			*(Sint32 *)VA(stack_ptr[-2]) >>= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_VARROL:
			*(Sint32 *)VA(stack_ptr[-2]) <<= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Operaciones directas con variables tipo WORD */

		case MN_WORD | MN_LET:
			(*(Sint16 *)VA(stack_ptr[-2])) = stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_INC:
			(*(Sint16 *)VA(stack_ptr[-1])) += ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_WORD | MN_DEC:
			(*(Sint16 *)VA(stack_ptr[-1])) -= ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_WORD | MN_POSTDEC:
			(*(Sint16 *)VA(stack_ptr[-1])) -= ptr[1] ;
			stack_ptr[-1] = *(Sint16 *)VA(stack_ptr[-1]) + ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_WORD | MN_POSTINC:
			*((Sint16 *)VA(stack_ptr[-1])) += ptr[1] ;
			stack_ptr[-1] = *(Sint16 *)VA(stack_ptr[-1]) - ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_WORD | MN_VARADD:
			*(Sint16 *)VA(stack_ptr[-2]) += stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARSUB:
			*(Sint16 *)VA(stack_ptr[-2]) -= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARMUL:
			*(Sint16 *)VA(stack_ptr[-2]) *= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARDIV:
			if ((Sint16)stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			*(Sint16 *)VA(stack_ptr[-2]) /= (Sint16)stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARMOD:
			if ((Sint16)stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			*(Sint16 *)VA(stack_ptr[-2]) %= (Sint16)stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VAROR:
			*(Sint16 *)VA(stack_ptr[-2]) |= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARXOR:
			*(Sint16 *)VA(stack_ptr[-2]) ^= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARAND:
			*(Sint16 *)VA(stack_ptr[-2]) &= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARROR:
			*(Sint16 *)VA(stack_ptr[-2]) >>= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_WORD | MN_VARROL:
			*(Sint16 *)VA(stack_ptr[-2]) <<= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Operaciones directas con variables tipo BYTE */

		case MN_BYTE | MN_LET:
			(*(Uint8 *)VA(stack_ptr[-2])) = stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_INC:
			(*(Uint8 *)VA(stack_ptr[-1])) += ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_BYTE | MN_DEC:
			(*(Uint8 *)VA(stack_ptr[-1])) -= ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_BYTE | MN_POSTDEC:
			(*(Uint8 *)VA(stack_ptr[-1])) -= ptr[1] ;
			stack_ptr[-1] = *(Uint8 *)VA(stack_ptr[-1]) + ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_BYTE | MN_POSTINC:
			*((Uint8 *)VA(stack_ptr[-1])) += ptr[1] ;
			stack_ptr[-1] = *(Uint8 *)VA(stack_ptr[-1]) - ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_BYTE | MN_VARADD:
			*(Uint8 *)VA(stack_ptr[-2]) += stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARSUB:
			*(Uint8 *)VA(stack_ptr[-2]) -= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARMUL:
			*(Uint8 *)VA(stack_ptr[-2]) *= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARDIV:
			if ((Uint8)stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			*(Uint8 *)VA(stack_ptr[-2]) /= (Uint8)stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARMOD:
			if ((Uint8)stack_ptr[-1] == 0)
				gr_error ("Error: Division por cero\n") ;
			*(Uint8 *)VA(stack_ptr[-2]) %= (Uint8)stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VAROR:
			*(Uint8 *)VA(stack_ptr[-2]) |= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARXOR:
			*(Uint8 *)VA(stack_ptr[-2]) ^= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARAND:
			*(Uint8 *)VA(stack_ptr[-2]) &= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARROR:
			*(Uint8 *)VA(stack_ptr[-2]) >>= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_BYTE | MN_VARROL:
			*(Uint8 *)VA(stack_ptr[-2]) <<= stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Operaciones directas con variables tipo FLOAT */

		case MN_FLOAT | MN_LET:
			(*(float *)VA(stack_ptr[-2])) = *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_INC:
			(*(float *)VA(stack_ptr[-1])) += ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_FLOAT | MN_DEC:
			(*(float *)VA(stack_ptr[-1])) -= ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_FLOAT | MN_POSTDEC:
			(*(float *)VA(stack_ptr[-1])) -= ptr[1] ;
			stack_ptr[-1] = *(Uint32 *)VA(stack_ptr[-1]) + ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_FLOAT | MN_POSTINC:
			*((float *)VA(stack_ptr[-1])) += ptr[1] ;
			stack_ptr[-1] = *(Uint32 *)VA(stack_ptr[-1]) - ptr[1] ;
			ptr+=2 ;
			break ;
		case MN_FLOAT | MN_VARADD:
			*(float *)VA(stack_ptr[-2]) += *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_VARSUB:
			*(float *)VA(stack_ptr[-2]) -= *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_VARMUL:
			*(float *)VA(stack_ptr[-2]) *= *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;
		case MN_FLOAT | MN_VARDIV:
			if (*(float *)&stack_ptr[-1] == 0.0)
				gr_error ("Error: Division por cero\n") ;
			*(float *)VA(stack_ptr[-2]) /= *(float *)&stack_ptr[-1] ;
			stack_ptr-- ;
			ptr++ ;
			break ;

		/* Saltos */

		case MN_JUMP:
			ptr = r->code + ptr[1] ;
			continue ;

		case MN_JTRUE:
			stack_ptr-- ;
			if ((*stack_ptr & 1) == 1)
			{
				ptr = r->code + ptr[1] ;
				continue ;
			}
			ptr += 2 ;
			break ;

		case MN_JFALSE:
			stack_ptr-- ;
			if ((*stack_ptr & 1) == 0)
			{
				ptr = r->code + ptr[1] ;
				continue ;
			}
			ptr += 2 ;
			break ;

		/* Switch */

		case MN_SWITCH:
			switchval = *--stack_ptr ;
			cased = 0 ;
			ptr++ ;
			break ;

		case MN_SWITCH | MN_STRING:
			if (switchval_string != 0)
				string_discard (switchval_string);
			switchval_string = *--stack_ptr;
			cased = 0;
			ptr++;
			break;

		case MN_CASE:
			if (switchval == *--stack_ptr) cased = 2 ;
			ptr++ ;
			break ;

		case MN_CASE | MN_STRING:
			if (string_comp (switchval_string, *--stack_ptr) == 0) cased = 2 ;
			string_discard (*stack_ptr);
			string_discard (stack_ptr[-1]);
			ptr++;
			break;

		case MN_CASE_R:
			stack_ptr -= 2 ;
			if (switchval >= stack_ptr[0] && 
			    switchval <= stack_ptr[1]) cased = 1 ;
			ptr++ ;
			break ;

		case MN_CASE_R | MN_STRING:
			stack_ptr -= 2;
			if (string_comp (switchval_string, stack_ptr[0]) >= 0 &&
				string_comp (switchval_string, stack_ptr[1]) <= 0)
				cased = 1;
			string_discard (stack_ptr[0]);
			string_discard (stack_ptr[1]);
			ptr++;
			break;

		case MN_JNOCASE:
			if (cased < 1)
			{
				ptr = r->code + ptr[1] ;
				continue ;
			}
			ptr += 2 ;
			break ;

		/* Control de procesos */

		case MN_TYPE:
			proc = procdef_get (ptr[1]) ;
			if (!proc)
				gr_error ("Error: Procedimiento desconocido\n") ;
			*stack_ptr++ = proc->type ;
			ptr += 2 ;
			break ;

		case MN_FRAME:
			LOCDWORD(r,FRAME_PERCENT) += stack_ptr[-1];
			stack_ptr-- ;
			r->codeptr = ptr+1 ;
			stack_gptr = stack_ptr ;
			goto break_all ;

		case MN_END:
			LOCDWORD (r,STATUS)=1 ;
			stack_gptr = stack_ptr ;
			goto break_all ;

		case MN_RETURN:
			LOCDWORD (r,STATUS) = 1 ;
			stack_ptr-- ;
			stack_gptr = stack_ptr ;
			return_value = *stack_ptr ;
			goto break_all ;

		/* Otros */

#ifdef DEBUG
		case MN_DEBUG:
			printf ("\n----- DEBUG -----\n") ;
			instance_dump_all() ;
			printf ("-----  END  -----\n") ;
			ptr++ ;
			break ;
#endif
                case MN_SENTENCE:
                        /*
                        if (dcb.sourcecount[ptr[1] >> 24])
                        printf ("%d: %s\n", ptr[1]  & 0xFFFFFF,
                                dcb.sourcelines [ptr[1] >> 24] [(ptr[1] & 0xFFFFFF)-1]) ;
                        */
                        ptr += 2 ;
                        break ;

		default:
			gr_error ("Error: Mnemonico 0x%02X no implementado\n", *ptr) ;

	}

	if (stack_ptr == stack)
	{
		r->codeptr = ptr ;
		if (LOCDWORD (r, STATUS) != 2)
			break ;
	}
	}

break_all:

	gprof_end (getid(r->proc->id));


	/* The process should be destroyed immediately,
	 * it is a function-type one */

	if (!*ptr || *ptr == MN_RETURN)
	{
		instance_destroy (r);
		if (was_visible)
			object_list_dirty = 1;
	}
	else if (LOCDWORD(r, STATUS) != 1 && r->first_run)
	{
		r->first_run = 0;
		object_list_dirty = 1;
	}
	else if (was_visible != instance_visible(r))
	{
		object_list_dirty = 1;
	}
	else if (LOCDWORD(r, COORDZ) != LOCDWORD(r, PREV_Z))
	{
		LOCDWORD(r, PREV_Z) = LOCDWORD(r, COORDZ);
		object_list_unsorted = 1;
	}

	if (switchval_string != 0)
		string_discard(switchval_string);

	return return_value;
}
