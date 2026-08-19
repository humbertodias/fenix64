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
 * HISTORY: Eliminated string_internal as stated in the new strings lib thugh
 * full strings library is not yet implemented.
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <string.h>

#include "fxi.h"
#include "dcb.h"

void mnemonic_dump (int i, int param);

/* ---------------------------------------------------------------------- */
/* Módulo principal del intérprete: función de ejecución de bloques       */
/* ---------------------------------------------------------------------- */

int must_exit = 0 ;
int force_debug = 0 ;
int debug_next = 0 ;
int debug_on_frame = 0;

int trace_sentence  = -1;
INSTANCE * trace_instance = NULL;
INSTANCE * last_instance_run = NULL;

int stack[16384] ;
int * stack_gptr = stack ;

#ifdef DEBUG
void stack_dump()
{
    int * ptr = stack  ;
    int i=0;

    while (ptr < stack_gptr)
    {
        printf ( "%08X ", *ptr++) ;
        if (i==4){
            i=0;
            printf("\n");
        } else {
            i++;
        }
        if (ptr > stack_gptr + 4)
        {
            printf ( " ...") ;
            break ;
        }
    }

    if ( i ){
        printf("\n");
    }

    return ;
}
#endif

static int freevars(void * data, DCB_VAR * var, int nvars);
static int freetype(int * data, DCB_TYPEDEF * var);
/*static */void freestrings( PROCDEF * proc, int * data );

static int freevars(void * data, DCB_VAR * var, int nvars)
{
	int result = 0;
	int partial;

	for (; nvars > 0; nvars--, var++)
	{
		partial = freetype (data, &var->Type);
		data = (Uint8*) data + partial;
		result += partial;
	}
	return result;
}


static int freetype(int * data, DCB_TYPEDEF * var)
{
	int n = 0;
	int count = 1;
	int result = 0;
	int partial;

	for (;n<MAX_TYPECHUNKS;)
	{
		switch (var->BaseType[n])
		{
			case    TYPE_FLOAT:

			case    TYPE_INT:
			case    TYPE_DWORD:
			case    TYPE_POINTER:
				    for (; count ; count--) {
					    result += 4;
					    data++; // = (Uint8*)data + 4;
				    }
				    break;

			case    TYPE_SHORT:
			case    TYPE_WORD:
				    for (; count ; count--) {
					    result += 2;
					    data = (int *)((Uint8*)data + 2);
				    }
				    break;

			case    TYPE_BYTE:
			case    TYPE_SBYTE:
		    case    TYPE_CHAR:
				    result += count;
				    break;

			case    TYPE_STRING:
				    for (; count ; count--) {
                        string_discard(*(Uint32 *)data);
                        data++; //   = (Uint8*)data + 4;
				        result += 4;
                    }
				    break;

			case    TYPE_ARRAY:
				    count *= var->Count[n];
				    n++;
				    continue;

			case    TYPE_STRUCT:
				    for (; count ; count--) {
					    partial = freevars(data, dcb.varspace_vars[var->Members], dcb.varspace[var->Members].NVars);
					    data = (int *)((Uint8*)data + partial);
					    result += partial;
				    }
				    break;

			default:
				    break;
		}
		break;
	}

	return result;
}

void freestrings(PROCDEF * proc, int * data)
{
    // Splinter, descarto todas las strings privadas
    int i, s = 0, r;

    for ( i=0; s < proc->private_size; i++ ){
        r = freetype((int *)((Uint8 *)data + s), &dcb.proc[proc->type].privar[i].Type);
        s += r;
    }
}

void instance_go_all ()
{
    INSTANCE * i = NULL;
    int i_count = 0 ;

    must_exit = 0 ;

    gprof_begin ("Interpreter");

    while (first_instance)
    {
        if (show_console) {
            gr_wait_frame() ;
            gr_draw_frame() ;

            if (must_exit) break ;

        } else {
            if (last_instance_run) {
                if (instance_exists(last_instance_run)) {
                    i = last_instance_run;
                } else {
                    last_instance_run = NULL;
                    i = instance_next_by_priority();
                }
            } else {
                i = instance_next_by_priority();
                i_count = 0 ;
            }

            while (i)
            {
                if (LOCDWORD(i, STATUS) == STATUS_KILLED || LOCDWORD(i, STATUS) == STATUS_DEAD ||
                    last_instance_run)
                {
                    // Run instance
                } else if (LOCDWORD(i, STATUS) == STATUS_RUNNING && LOCDWORD(i, FRAME_PERCENT) < 100) {
                    LOCDWORD(i, TYPE_SCAN) = 0;
                    LOCDWORD(i, ID_SCAN) = 0;
                } else {
                    i = instance_next_by_priority();
                    last_instance_run = NULL;
                    continue;
                }

                i_count++;

                last_instance_run = NULL;

                instance_go (i);

                if (force_debug) {
                    show_console = 1;
                    last_instance_run  = trace_instance;
                    break;
                }

                if (must_exit) break;

                i = instance_next_by_priority();
            }

            if (must_exit) break ;

            /* Si no se ejecutó nada: Dibujar, actualizar variables, etc. */

            if (!i_count && !force_debug)
            {
                /* Honors the signal-changed status of the process and
                 * saves so it is used in this loop the next frame
                 */
                i = first_instance ;
                while (i)
                {
                    INSTANCE * next = i->next ;

                    if (LOCDWORD(i,STATUS) == STATUS_RUNNING)
                    {
                        LOCDWORD(i,FRAME_PERCENT) -= 100 ;
                    }

                    LOCDWORD(i,SAVED_STATUS) = LOCDWORD(i, STATUS);

                    if (LOCDWORD(i,STATUS) == STATUS_RUNNING)
                    {
                        if (LOCDWORD(i, SAVED_PRIORITY) != LOCDWORD(i, PRIORITY))
                        {
                            LOCDWORD(i, SAVED_PRIORITY) = LOCDWORD(i, PRIORITY);
                            instance_dirty(i);
                        }
                    }

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
                gr_advance_timers() ;

                if (debug_on_frame) {
                    debug_on_frame = 0;
                    show_console = 1;
                }
                continue ;
            }

            /* NOTA: los procesos pueden haberse autodestruido tras su ejecución */
        }
    }

    gprof_end ("Interpreter");
}

int instance_go (INSTANCE * r)
{
    if (!r) return 0 ;

    register int * ptr = r->codeptr ;
    register int * stack_ptr = stack_gptr ;
             int * stack_begin = stack_ptr ;
    int n ;
    int return_value = LOCDWORD(r, PROCESS_ID) ;
    int was_visible;
    SYSPROC * p = NULL ;
    INSTANCE * i = NULL ;
    PROCDEF * proc = r->proc;
    static char buffer[16], * str = NULL ;

    /* This variable contains a pointer to the private area at the stack.
       It is 0 if the current process uses the instance's private area instead */
    int * private_data = r->pridata;

   /* Pointer to the current process's code (it may be a called one) */
    int * base_code = r->code;

    int switchval = 0, switchval_string = 0, cased = 0 ;

    int child_is_alive = 0;

    /* ------------------------------------------------------------------------------- */
    /* Restauro si salio por debug */
#if 0
    if (r->inpridata) {
        private_data = r->inpridata;
        base_code = r->inproc->code;
        proc = r->inproc;
    }
#endif

    switchval = r->switchval;
    switchval_string = r->switchval_string;
    cased = r->cased;

    if (r->stack)
    {
        /* Restore a saved stack, if present */
        if ((*r->stack) & STACK_SIZE_MASK) memcpy (stack_ptr, r->stack+1, ((*r->stack) & STACK_SIZE_MASK) * 4);
        stack_ptr += ((*r->stack) & STACK_SIZE_MASK);
        free (r->stack);
        r->stack = NULL;
    }

    if (debug) {
        printf ("***** INSTANCE %s(%d) ENTRY\n", proc->name, LOCDWORD(r,PROCESS_ID)) ;
    }

    gprof_begin (proc->name);

    was_visible = instance_visible(r);

    LOCDWORD(r, PREV_Z) = LOCDWORD(r, COORDZ);

    if((proc->breakpoint || r->breakpoint) && trace_instance != r) {
        debug_next = 1;
    }

    trace_sentence = -1;

    while(!must_exit /*&&
          (LOCDWORD(r, STATUS) == STATUS_RUNNING ||
           LOCDWORD(r, STATUS) == STATUS_DEAD) &&
          !(LOCDWORD(r, STATUS) & STATUS_WAITING_MASK)
         */)
    {
        if (debug_next && trace_sentence != -1) {
            force_debug = 1;
            debug_next = 0;
            r->codeptr = ptr ;
            stack_gptr = stack_ptr ;
            return_value = LOCDWORD(r, PROCESS_ID);
#if 0
            if (private_data != r->pridata) {
                r->inpridata = private_data;
                r->inproc = proc;
            }
#endif
            r->switchval = switchval;
            r->switchval_string = switchval_string;
            r->cased = cased;

            break;
        }

#ifdef DEBUG
        if (debug)
        {
            stack_gptr = stack_ptr ;
            stack_dump();
            printf ("%45s [%4u] ", " ", (ptr - base_code)) ;
            mnemonic_dump (*ptr, ptr[1]) ;
            printf("\n");
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
            case MN_INDEX | MN_UNSIGNED:
            case MN_INDEX | MN_STRING:
            case MN_INDEX | MN_WORD:
            case MN_INDEX | MN_WORD | MN_UNSIGNED:
            case MN_INDEX | MN_BYTE:
            case MN_INDEX | MN_BYTE | MN_UNSIGNED:
            case MN_INDEX | MN_FLOAT: /* Splinter, se agrega float, no se por que no estaba */
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
                ptr = base_code + ptr[1] ;
                continue ;

            case MN_CALL:
            case MN_PROC:
                proc = procdef_get (ptr[1]) ;
                if (!proc) gr_error ("Error: Procedimiento desconocido\n") ;

#if 0
                if (proc->flags & (PROC_USES_FRAME | PROC_USES_LOCALS | PROC_USES_PUBLICS))
#endif
                {
                    /* Process uses FRAME or locals, must create an instance */
                    i = instance_new (proc, r) ;

                    for (n = 0 ; n < proc->params ; n++)
                        PRIDWORD(i,4*n) = stack_ptr[-proc->params+n] ;

                    stack_ptr -= proc->params ;
                    stack_gptr = stack_ptr ;

                    gprof_end (proc->name);

                    /* Por default, me pongo en espera... */
                    LOCDWORD(r,STATUS) |= STATUS_WAITING_MASK;
                    i->called_by   = r;

                    // Ejecuto la funcion/processo...

                    if (*ptr == MN_CALL) {
                        *stack_ptr++ = instance_go (i);
                    } else {
                        instance_go (i);
                    }

                    child_is_alive = instance_exists(i);

                    gprof_begin (proc->name);

                    ptr += 2 ;

                    /* If the process is a function in a frame, save the stack and leave */
                    /* Si sigue corriendo la funcion/proceso que lance, es porque esta en un frame.
                       Si esta ejecutando codigo, es porque su STATUS es RUNNING */
                    if (child_is_alive &&
                        ((LOCDWORD(r,STATUS) & STATUS_WAITING_MASK) ||
                         (LOCDWORD(r,STATUS) & ~STATUS_WAITING_MASK) == STATUS_FROZEN ||
                         (LOCDWORD(r,STATUS) & ~STATUS_WAITING_MASK) == STATUS_SLEEPING
                        )
                       )
                    {
                        /* En este caso me pongo a dormir y retorno */
                        i->called_by   = r;

                        /* Guardo todo el contenido del stack actual (mi stack) */
                        r->stack       = (int *) malloc(4*(stack_ptr - stack_begin + 2));
                        /* Guardo size del stack */
                        r->stack[0]    = stack_ptr - stack_begin;

                        if (stack_ptr > stack_begin)
                            memcpy (r->stack+1, stack_begin, 4*(stack_ptr-stack_begin));

                        /* Guardo el puntero de instruccion */
                        /* Esta instancia no va a ejecutar otro codigo hasta que retorne el hijo */
                        r->codeptr = ptr ;

                        /* Apunto global stack a stack_begin (inicio del stack original) */
                        stack_gptr = stack_begin;

                        /* Si no fue un call, seteo un flag en la len para no retornar valor */
                        if (ptr[-2] != MN_CALL) {
                            r->stack[0] |= STACK_NO_RETURN_VALUE;
                        }

                        if (debug_next && trace_sentence != -1) {
                            force_debug = 1;
                            debug_next = 0;
                        }

                        return 0;
                    }

                    /* Me despierto */
                    LOCDWORD(r, STATUS) &= ~STATUS_WAITING_MASK;
                    if (child_is_alive) i->called_by = NULL;
                    proc = r->proc;
                }
#if 0
                else
                {
                    /* This process can be called locally: create a private memory area using the stack */
                    /* Solo privadas, si usa otro tipo de variables es una instancia */

                    int * old_data      = private_data;
                    private_data = stack_ptr - proc->params;
                    if (stack_ptr > stack + sizeof(stack)/4 - proc->private_size/4 - 128) gr_error ("Stack overflow!");

                    stack_ptr += ( proc->private_size/4 - proc->params ) ;

                    if (proc->private_size > proc->params*4) {
                        memcpy ((Uint32 *)private_data + proc->params, proc->pridata + proc->params, proc->private_size - proc->params*4);
                    }

                    /* Save the previous process frame (see MN_RETURN) */
                    *stack_ptr++ = *ptr;                            // Instruccion Actual (stack_ptr[-6] in return)
                    *stack_ptr++ = (Sint32) base_code;              // Base Code Actual   (stack_ptr[-5] in return)
                    *stack_ptr++ = (Sint32) proc;                   // Proc Actual        (stack_ptr[-4] in return)
                    *stack_ptr++ = (Sint32) (ptr+2);                //                    (stack_ptr[-3] in return)
                    *stack_ptr++ = (Sint32) old_data;               //                    (stack_ptr[-2] in return)
                    base_code = ptr = proc->code;
                }
#endif
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

            /* Acceso a direcciones de variables */

            case MN_PRIVATE:
            case MN_PRIVATE | MN_UNSIGNED:
            case MN_PRIVATE | MN_WORD:
            case MN_PRIVATE | MN_BYTE:
            case MN_PRIVATE | MN_WORD | MN_UNSIGNED:
            case MN_PRIVATE | MN_BYTE | MN_UNSIGNED:
            case MN_PRIVATE | MN_STRING:
            case MN_PRIVATE | MN_FLOAT:
                *stack_ptr++ = (int) ((Uint8 *)private_data + ptr[1]);
                ptr += 2 ;
                break ;

            case MN_PUBLIC:
            case MN_PUBLIC | MN_UNSIGNED:
            case MN_PUBLIC | MN_WORD:
            case MN_PUBLIC | MN_BYTE:
            case MN_PUBLIC | MN_WORD | MN_UNSIGNED:
            case MN_PUBLIC | MN_BYTE | MN_UNSIGNED:
            case MN_PUBLIC | MN_STRING:
            case MN_PUBLIC | MN_FLOAT:
                *stack_ptr++ = (int) &PUBDWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_LOCAL:
            case MN_LOCAL | MN_UNSIGNED:
            case MN_LOCAL | MN_WORD:
            case MN_LOCAL | MN_BYTE:
            case MN_LOCAL | MN_WORD | MN_UNSIGNED:
            case MN_LOCAL | MN_BYTE | MN_UNSIGNED:
            case MN_LOCAL | MN_STRING:
            case MN_LOCAL | MN_FLOAT:
                *stack_ptr++ = (int) &LOCDWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_GLOBAL:
            case MN_GLOBAL | MN_UNSIGNED:
            case MN_GLOBAL | MN_WORD:
            case MN_GLOBAL | MN_BYTE:
            case MN_GLOBAL | MN_WORD | MN_UNSIGNED:
            case MN_GLOBAL | MN_BYTE | MN_UNSIGNED:
            case MN_GLOBAL | MN_STRING:
            case MN_GLOBAL | MN_FLOAT:
                *stack_ptr++ = (int) &GLODWORD(ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_REMOTE:
            case MN_REMOTE | MN_UNSIGNED:
            case MN_REMOTE | MN_WORD:
            case MN_REMOTE | MN_BYTE:
            case MN_REMOTE | MN_WORD | MN_UNSIGNED:
            case MN_REMOTE | MN_BYTE | MN_UNSIGNED:
            case MN_REMOTE | MN_STRING:
            case MN_REMOTE | MN_FLOAT:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = (int) &LOCDWORD(i, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_REMOTE_PUBLIC:
            case MN_REMOTE_PUBLIC | MN_UNSIGNED:
            case MN_REMOTE_PUBLIC | MN_WORD:
            case MN_REMOTE_PUBLIC | MN_BYTE:
            case MN_REMOTE_PUBLIC | MN_WORD | MN_UNSIGNED:
            case MN_REMOTE_PUBLIC | MN_BYTE | MN_UNSIGNED:
            case MN_REMOTE_PUBLIC | MN_STRING:
            case MN_REMOTE_PUBLIC | MN_FLOAT:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = (int) &PUBDWORD(i, ptr[1]) ;
                ptr += 2 ;
                break ;

            /* Acceso a variables tipo DWORD */

            case MN_GET_PRIV:
            case MN_GET_PRIV | MN_FLOAT:
            case MN_GET_PRIV | MN_UNSIGNED:
                *stack_ptr++ = *(int *) ((Uint8 *)private_data + ptr[1]);
                ptr += 2 ;
                break ;

            case MN_GET_PUBLIC:
            case MN_GET_PUBLIC | MN_FLOAT:
            case MN_GET_PUBLIC | MN_UNSIGNED:
                *stack_ptr++ = PUBDWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_GET_LOCAL:
            case MN_GET_LOCAL | MN_FLOAT:
            case MN_GET_LOCAL | MN_UNSIGNED:
                *stack_ptr++ = LOCDWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_GET_GLOBAL:
            case MN_GET_GLOBAL | MN_FLOAT:
            case MN_GET_GLOBAL | MN_UNSIGNED:
                *stack_ptr++ = GLODWORD(ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_GET_REMOTE:
            case MN_GET_REMOTE | MN_FLOAT:
            case MN_GET_REMOTE | MN_UNSIGNED:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = LOCDWORD(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_GET_REMOTE_PUBLIC:
            case MN_GET_REMOTE_PUBLIC | MN_FLOAT:
            case MN_GET_REMOTE_PUBLIC | MN_UNSIGNED:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = PUBDWORD(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_PTR:
            case MN_PTR | MN_UNSIGNED:
            case MN_PTR | MN_FLOAT:
                stack_ptr[-1] = *(Sint32 *)stack_ptr[-1] ;
                ptr++ ;
                break ;

            /* Acceso a variables tipo STRING */

            case MN_PUSH | MN_STRING:
                *stack_ptr++ = ptr[1] ;
                string_use ( stack_ptr[-1] ) ;
                ptr += 2 ;
                break ;

            case MN_GET_PRIV | MN_STRING:
                *stack_ptr++ = *(int *) ((Uint8 *)private_data + ptr[1]);
                string_use     ( stack_ptr[-1] ) ;
                ptr += 2 ;
                break ;

            case MN_GET_PUBLIC | MN_STRING:
                *stack_ptr++ = PUBDWORD(r, ptr[1]) ;
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
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = LOCDWORD(i,ptr[1]) ;
                string_use     ( stack_ptr[-1] ) ;
                ptr += 2 ;
                break ;

            case MN_GET_REMOTE_PUBLIC | MN_STRING:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = PUBDWORD(i,ptr[1]) ;
                string_use     ( stack_ptr[-1] ) ;
                ptr += 2 ;
                break ;

            case MN_STRING | MN_PTR:
                stack_ptr[-1] = *(Sint32 *)stack_ptr[-1] ;
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
                *stack_ptr++ = *(Sint16 *) ((Uint8 *)private_data+ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_PRIV | MN_UNSIGNED:
                *stack_ptr++ = *(Uint16 *) ((Uint8 *)private_data+ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_PUBLIC:
                *stack_ptr++ = (Sint16) PUBWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_PUBLIC | MN_UNSIGNED:
                *stack_ptr++ = PUBWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_LOCAL:
                *stack_ptr++ = (Sint16) LOCWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_LOCAL | MN_UNSIGNED:
                *stack_ptr++ = LOCWORD(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_GLOBAL:
                *stack_ptr++ = GLOWORD(ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_GLOBAL | MN_UNSIGNED:
                *stack_ptr++ = GLOWORD(ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_REMOTE:
            case MN_WORD | MN_GET_REMOTE | MN_UNSIGNED:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = LOCWORD(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_GET_REMOTE_PUBLIC:
            case MN_WORD | MN_GET_REMOTE_PUBLIC | MN_UNSIGNED:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = PUBWORD(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_WORD | MN_PTR:
            case MN_WORD | MN_PTR | MN_UNSIGNED:
                stack_ptr[-1] = *(Sint16 *)stack_ptr[-1] ;
                ptr++ ;
                break ;

/*            case MN_WORD | MN_NOT:
            case MN_WORD | MN_NOT | MN_UNSIGNED:
                stack_ptr[-1] = !(stack_ptr[-1]) ;
                ptr++ ;
                break ; */

            /* Acceso a variables tipo BYTE */

            case MN_BYTE | MN_GET_PRIV:
                *stack_ptr++ = *(Sint8 *) ((Uint8 *)private_data+ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_PRIV | MN_UNSIGNED:
                *stack_ptr++ = *(Uint8 *) ((Uint8 *)private_data+ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_PUBLIC:
                *stack_ptr++ = (Sint8) PUBBYTE(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_PUBLIC | MN_UNSIGNED:
                *stack_ptr++ = PUBBYTE(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_LOCAL:
                *stack_ptr++ = (signed char) LOCBYTE(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_LOCAL | MN_UNSIGNED:
                *stack_ptr++ = LOCBYTE(r, ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_GLOBAL:
                *stack_ptr++ = (signed char) GLOBYTE(ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_GLOBAL | MN_UNSIGNED:
                *stack_ptr++ = GLOBYTE(ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_REMOTE:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = (signed char) LOCBYTE(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_REMOTE | MN_UNSIGNED:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = LOCBYTE(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_REMOTE_PUBLIC:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = (signed char) PUBBYTE(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_GET_REMOTE_PUBLIC | MN_UNSIGNED:
                i = instance_get (stack_ptr[-1]) ;
                if (!i)
                    gr_error ("Error de ejecucion en proceso %s(%d):\nProcedimiento %d no activo\n", proc->name, LOCDWORD(r,PROCESS_ID), stack_ptr[-1]) ;
                else
                    stack_ptr[-1] = PUBBYTE(i,ptr[1]) ;
                ptr += 2 ;
                break ;

            case MN_BYTE | MN_PTR:
                stack_ptr[-1] = *((Sint8 *)stack_ptr[-1]) ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_PTR | MN_UNSIGNED:
                stack_ptr[-1] = *((Uint8 *)stack_ptr[-1]) ;
                ptr++ ;
                break ;

/*            case MN_BYTE | MN_NOT:
            case MN_BYTE | MN_NOT | MN_UNSIGNED:
                stack_ptr[-1] = !(stack_ptr[-1]) ;
                ptr++ ;
                break ; */

            /* Operaciones matemáticas  en coma floatante */

            case MN_FLOAT | MN_NEG:
                *(float *)&stack_ptr[-1] = -*((float *)&stack_ptr[-1]) ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_NOT:
                *(float *)&stack_ptr[-1] = (float) !*((float *)&stack_ptr[-1]) ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_ADD:
                *(float *)&stack_ptr[-2] += *((float *)&stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_SUB:
                *(float *)&stack_ptr[-2] -= *((float *)&stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_MUL:
                *(float *)&stack_ptr[-2] *= *((float *)&stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_DIV:
                if (*((float *)&stack_ptr[-1]) == 0.0) gr_error ("Error: Division por cero\n") ;
                *(float *)&stack_ptr[-2] /= *((float *)&stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT2INT:
                *(Sint32 *)&(stack_ptr[-ptr[1]-1]) = (Sint32) *(float *)&(stack_ptr[-ptr[1]-1]) ;
                ptr+=2 ;
                break ;

            case MN_INT2FLOAT:
            case MN_INT2FLOAT | MN_UNSIGNED:
                *(float *)&(stack_ptr[-ptr[1]-1]) = (float) *(Sint32 *)&(stack_ptr[-ptr[1]-1]) ;
                ptr+=2 ;
                break ;

            case MN_INT2WORD:
            case MN_INT2WORD | MN_UNSIGNED:
                *(int *)&(stack_ptr[-ptr[1]-1]) = (Sint32)(Uint16) *(Sint32 *)&(stack_ptr[-ptr[1]-1]) ;
                ptr += 2;
                break;
            case MN_INT2BYTE:
            case MN_INT2BYTE | MN_UNSIGNED:
                *(int *)&(stack_ptr[-ptr[1]-1]) = (Sint32)(Uint8) *(Sint32 *)&(stack_ptr[-ptr[1]-1]) ;
                ptr += 2;
                break;

            /* Operaciones matemáticas */

            case MN_NEG:
            case MN_NEG | MN_UNSIGNED:
                stack_ptr[-1] = -stack_ptr[-1] ;
                ptr++ ;
                break ;

            case MN_NOT:
            case MN_NOT | MN_UNSIGNED:
                stack_ptr[-1] = !(stack_ptr[-1]) ;
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
            case MN_MUL | MN_BYTE:
            case MN_MUL:
                stack_ptr[-2] *= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_MUL | MN_WORD | MN_UNSIGNED:
            case MN_MUL | MN_BYTE | MN_UNSIGNED:
            case MN_MUL | MN_UNSIGNED:
                stack_ptr[-2] = (Uint32)stack_ptr[-2] * (Uint32)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_DIV | MN_WORD:
            case MN_DIV | MN_BYTE:
            case MN_DIV:
                if (stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                stack_ptr[-2] /= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_DIV | MN_WORD | MN_UNSIGNED:
            case MN_DIV | MN_BYTE | MN_UNSIGNED:
            case MN_DIV | MN_UNSIGNED:
                if (stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                stack_ptr[-2] = (Uint32)stack_ptr[-2] / (Uint32)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_MOD | MN_WORD:
            case MN_MOD | MN_BYTE:
            case MN_MOD:
                if (stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                stack_ptr[-2] %= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_MOD | MN_WORD | MN_UNSIGNED:
            case MN_MOD | MN_BYTE | MN_UNSIGNED:
            case MN_MOD | MN_UNSIGNED:
                if (stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                stack_ptr[-2] = (Uint32)stack_ptr[-2] % (Uint32)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            /* Operaciones a nivel de bit */

            case MN_ROR:
                (stack_ptr[-2]) = (Sint32)(stack_ptr[-2]) >> stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_ROR | MN_UNSIGNED:
                stack_ptr[-2] = (Uint32)(stack_ptr[-2]) >> stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_ROR:
                stack_ptr[-2] = (Sint16)(stack_ptr[-2]) >> stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_ROR | MN_UNSIGNED:
                stack_ptr[-2] = (Uint16)(stack_ptr[-2]) >> stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_ROR:
                stack_ptr[-2] = (Sint8)(stack_ptr[-2]) >> stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_ROR | MN_UNSIGNED:
                stack_ptr[-2] = (Uint8)(stack_ptr[-2]) >> stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_ROL:
                (stack_ptr[-2]) = (Sint32)(stack_ptr[-2]) << stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            /* Todos estos ROL siguientes no serian necesarios, pero bueno... */
            case MN_ROL | MN_UNSIGNED:
                (stack_ptr[-2]) = (Uint32)(stack_ptr[-2]) << stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_ROL:
                (stack_ptr[-2]) = (Sint16)(stack_ptr[-2]) << stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_ROL | MN_UNSIGNED:
                (stack_ptr[-2]) = (Uint16)(stack_ptr[-2]) << stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_ROL:
                (stack_ptr[-2]) = (Sint8)(stack_ptr[-2]) << stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_ROL | MN_UNSIGNED:
                (stack_ptr[-2]) = (Uint8)(stack_ptr[-2]) << stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BAND:
            case MN_BAND | MN_UNSIGNED:
                stack_ptr[-2] &= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BOR:
            case MN_BOR | MN_UNSIGNED:
                stack_ptr[-2] |= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BXOR:
            case MN_BXOR | MN_UNSIGNED:
                stack_ptr[-2] ^= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BNOT:
            case MN_BNOT | MN_UNSIGNED:
                stack_ptr[-1] = ~(stack_ptr[-1]) ;
                ptr++ ;
                break ;

            /* Operaciones logicas */

            case MN_AND:
                stack_ptr[-2] = stack_ptr[-2] && stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_OR:
                stack_ptr[-2] = stack_ptr[-2] || stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_XOR:
                stack_ptr[-2] = (stack_ptr[-2]!=0) ^ (stack_ptr[-1]!=0) ;
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

            case MN_GTE | MN_UNSIGNED:
                stack_ptr[-2] = ((Uint32)stack_ptr[-2] >= (Uint32)stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_LTE:
                stack_ptr[-2] = (stack_ptr[-2] <= stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_LTE | MN_UNSIGNED:
                stack_ptr[-2] = ((Uint32)stack_ptr[-2] <= (Uint32)stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_LT:
                stack_ptr[-2] = (stack_ptr[-2] < stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_LT | MN_UNSIGNED:
                stack_ptr[-2] = ((Uint32)stack_ptr[-2] < (Uint32)stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_GT:
                stack_ptr[-2] = (stack_ptr[-2] > stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_GT | MN_UNSIGNED:
                stack_ptr[-2] = ((Uint32)stack_ptr[-2] > (Uint32)stack_ptr[-1]) ;
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
                n = *(Sint32 *)(stack_ptr[-2]) ;
                *(Sint32 *)(stack_ptr[-2]) = string_add (n, stack_ptr[-1]) ;
                string_use ( *(Sint32 *)(stack_ptr[-2]) ) ;
                string_discard (n) ;
                string_discard (stack_ptr[-1]) ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_LETNP | MN_STRING:
                string_discard ( *(Sint32 *)(stack_ptr[-2]) ) ;
                (*(Sint32 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-=2 ;
                ptr++ ;
                break ;

            case MN_LET | MN_STRING:
                string_discard ( *(Sint32 *)(stack_ptr[-2]) ) ;
                (*(Sint32 *)(stack_ptr[-2])) = stack_ptr[-1] ;
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

            case MN_INT2STR | MN_WORD:
                stack_ptr[-ptr[1]-1] = string_itoa((Sint16)stack_ptr[-ptr[1]-1]) ;
                string_use (stack_ptr[-ptr[1]-1]) ;
                ptr += 2 ;
                break ;

            case MN_INT2STR:
                stack_ptr[-ptr[1]-1] = string_itoa(stack_ptr[-ptr[1]-1]) ;
                string_use (stack_ptr[-ptr[1]-1]) ;
                ptr += 2 ;
                break ;

            case MN_INT2STR | MN_BYTE:
                stack_ptr[-ptr[1]-1] = string_itoa((Sint8)stack_ptr[-ptr[1]-1]) ;
                string_use (stack_ptr[-ptr[1]-1]) ;
                ptr += 2 ;
                break ;

            case MN_INT2STR | MN_UNSIGNED | MN_BYTE:
                stack_ptr[-ptr[1]-1] = string_uitoa((Uint8) stack_ptr[-ptr[1]-1]) ;
                string_use (stack_ptr[-ptr[1]-1]) ;
                ptr += 2 ;
                break ;

            case MN_INT2STR | MN_UNSIGNED:
            case MN_INT2STR | MN_UNSIGNED | MN_WORD:
                stack_ptr[-ptr[1]-1] = string_uitoa((unsigned) stack_ptr[-ptr[1]-1]) ;
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

            case MN_STR2CHR:
                n = stack_ptr[-ptr[1]-1] ;
                stack_ptr[-1] = *string_get(n) ;
                string_discard(n);
                ptr += 2 ;
                break ;

            case MN_POINTER2STR:
                stack_ptr[-ptr[1]-1] = string_ptoa(*(void **)&stack_ptr[-ptr[1]-1]) ;
                string_use (stack_ptr[-ptr[1]-1]) ;
                ptr += 2 ;
                break ;
/*
            case MN_POINTER2BOL:
                stack_ptr[-ptr[1]-1] = (stack_ptr[-ptr[1]-1]) ? 1:0 ;
                ptr += 2 ;
                break ;
*/
            case MN_STR2FLOAT:
                n = stack_ptr[-ptr[1]-1] ;
                str = (char *)string_get(n) ;
                *(float *)(&stack_ptr[-ptr[1]-1]) = str ? (float)atof(str) : 0.0f ;
                string_discard (n) ;
                ptr += 2 ;
                break ;

            case MN_STR2INT:
                n = stack_ptr[-ptr[1]-1] ;
                str = (char *)string_get(n) ;
                stack_ptr[-ptr[1]-1] = str ? atoi(str) : 0 ;
                string_discard (n) ;
                ptr += 2 ;
                break ;

            /* Operaciones con cadenas de ancho fijo */

            case MN_A2STR:
                str = *(char **)(&stack_ptr[-ptr[1]-1]) ;
                n = string_new(str);
                string_use(n);
                stack_ptr[-ptr[1]-1] = n ;
                ptr+=2 ;
                break ;

            case MN_STR2A:
                n = stack_ptr[-1];
                strncpy (*(char **)(&stack_ptr[-2]), string_get(n), ptr[1]) ;
                ((char *)(stack_ptr[-2]))[ptr[1]] = 0;
                stack_ptr[-2] = stack_ptr[-1];
                stack_ptr--;
                ptr+=2 ;
                break ;

            case MN_STRACAT:
                n = stack_ptr[-1];
                strncat (*(char **)(&stack_ptr[-2]), string_get(n),ptr[1] - strlen(*(char **)(&stack_ptr[-2]))) ;
                ((char *)(stack_ptr[-2]))[ptr[1]] = 0;
                stack_ptr[-2] = stack_ptr[-1];
                stack_ptr--;
                ptr+=2 ;
                break ;

            /* Operaciones directas con variables tipo DWORD */

            case MN_LETNP:
            case MN_LETNP | MN_UNSIGNED:
                (*(Sint32 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-=2 ;
                ptr++ ;
                break ;

            case MN_LET:
            case MN_LET | MN_UNSIGNED:
                (*(Sint32 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_INC:
            case MN_INC | MN_UNSIGNED:
                (*(Sint32 *)(stack_ptr[-1])) += ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_DEC:
            case MN_DEC | MN_UNSIGNED:
                (*(Sint32 *)(stack_ptr[-1])) -= ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_POSTDEC:
            case MN_POSTDEC | MN_UNSIGNED:
                (*(Sint32 *)(stack_ptr[-1])) -= ptr[1] ;
                stack_ptr[-1] = *(Sint32 *)(stack_ptr[-1]) + ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_POSTINC:
            case MN_POSTINC | MN_UNSIGNED:
                *((Sint32 *)(stack_ptr[-1])) += ptr[1] ;
                stack_ptr[-1] = *(Sint32 *)(stack_ptr[-1]) - ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_VARADD:
            case MN_VARADD | MN_UNSIGNED:
                *(Sint32 *)(stack_ptr[-2]) += stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARSUB:
            case MN_VARSUB | MN_UNSIGNED:
                *(Sint32 *)(stack_ptr[-2]) -= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARMUL:
            case MN_VARMUL | MN_UNSIGNED:
                *(Sint32 *)(stack_ptr[-2]) *= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARDIV:
            case MN_VARDIV | MN_UNSIGNED:
                if (stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                *(Sint32 *)(stack_ptr[-2]) /= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARMOD:
            case MN_VARMOD | MN_UNSIGNED:
                if (stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                *(Sint32 *)(stack_ptr[-2]) %= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VAROR:
            case MN_VAROR | MN_UNSIGNED:
                *(Sint32 *)(stack_ptr[-2]) |= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARXOR:
            case MN_VARXOR | MN_UNSIGNED:
                *(Sint32 *)(stack_ptr[-2]) ^= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARAND:
            case MN_VARAND | MN_UNSIGNED:
                *(Sint32 *)(stack_ptr[-2]) &= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARROR:
                *(Sint32 *)(stack_ptr[-2]) >>= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARROR | MN_UNSIGNED:
                *(Uint32 *)(stack_ptr[-2]) >>= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARROL:
                *(Sint32 *)(stack_ptr[-2]) <<= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_VARROL | MN_UNSIGNED:
                *(Uint32 *)(stack_ptr[-2]) <<= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            /* Operaciones directas con variables tipo WORD */

            case MN_WORD | MN_LETNP:
            case MN_WORD | MN_LETNP | MN_UNSIGNED:
                (*(Sint16 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-=2 ;
                ptr++ ;
                break ;

            case MN_WORD | MN_LET:
            case MN_WORD | MN_LET | MN_UNSIGNED:
                (*(Sint16 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_INC:
            case MN_WORD | MN_INC | MN_UNSIGNED:
                (*(Sint16 *)(stack_ptr[-1])) += ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_WORD | MN_DEC:
            case MN_WORD | MN_DEC | MN_UNSIGNED:
                (*(Sint16 *)(stack_ptr[-1])) -= ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_WORD | MN_POSTDEC:
            case MN_WORD | MN_POSTDEC | MN_UNSIGNED:
                (*(Sint16 *)(stack_ptr[-1])) -= ptr[1] ;
                stack_ptr[-1] = *(Sint16 *)(stack_ptr[-1]) + ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_WORD | MN_POSTINC:
            case MN_WORD | MN_POSTINC | MN_UNSIGNED:
                *((Sint16 *)(stack_ptr[-1])) += ptr[1] ;
                stack_ptr[-1] = *(Sint16 *)(stack_ptr[-1]) - ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_WORD | MN_VARADD:
            case MN_WORD | MN_VARADD | MN_UNSIGNED:
                *(Sint16 *)(stack_ptr[-2]) += stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARSUB:
            case MN_WORD | MN_VARSUB | MN_UNSIGNED:
                *(Sint16 *)(stack_ptr[-2]) -= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARMUL:
            case MN_WORD | MN_VARMUL | MN_UNSIGNED:
                *(Sint16 *)(stack_ptr[-2]) *= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARDIV:
            case MN_WORD | MN_VARDIV | MN_UNSIGNED:
                if ((Sint16)stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                *(Sint16 *)(stack_ptr[-2]) /= (Sint16)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARMOD:
            case MN_WORD | MN_VARMOD | MN_UNSIGNED:
                if ((Sint16)stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                *(Sint16 *)(stack_ptr[-2]) %= (Sint16)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VAROR:
            case MN_WORD | MN_VAROR | MN_UNSIGNED:
                *(Sint16 *)(stack_ptr[-2]) |= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARXOR:
            case MN_WORD | MN_VARXOR | MN_UNSIGNED:
                *(Sint16 *)(stack_ptr[-2]) ^= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARAND:
            case MN_WORD | MN_VARAND | MN_UNSIGNED:
                *(Sint16 *)(stack_ptr[-2]) &= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARROR:
                *(Sint16 *)(stack_ptr[-2]) >>= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARROR | MN_UNSIGNED:
                *(Uint16 *)(stack_ptr[-2]) >>= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARROL:
                *(Sint16 *)(stack_ptr[-2]) <<= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_WORD | MN_VARROL | MN_UNSIGNED:
                *(Uint16 *)(stack_ptr[-2]) <<= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            /* Operaciones directas con variables tipo BYTE */

            case MN_BYTE | MN_LETNP:
            case MN_BYTE | MN_LETNP | MN_UNSIGNED:
                (*(Uint8 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-=2 ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_LET:
            case MN_BYTE | MN_LET | MN_UNSIGNED:
                (*(Uint8 *)(stack_ptr[-2])) = stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_INC:
            case MN_BYTE | MN_INC | MN_UNSIGNED:
                (*(Uint8 *)(stack_ptr[-1])) += ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_BYTE | MN_DEC:
            case MN_BYTE | MN_DEC | MN_UNSIGNED:
                (*(Uint8 *)(stack_ptr[-1])) -= ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_BYTE | MN_POSTDEC:
            case MN_BYTE | MN_POSTDEC | MN_UNSIGNED:
                (*(Uint8 *)(stack_ptr[-1])) -= ptr[1] ;
                stack_ptr[-1] = *(Uint8 *)(stack_ptr[-1]) + ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_BYTE | MN_POSTINC:
            case MN_BYTE | MN_POSTINC | MN_UNSIGNED:
                *((Uint8 *)(stack_ptr[-1])) += ptr[1] ;
                stack_ptr[-1] = *(Uint8 *)(stack_ptr[-1]) - ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_BYTE | MN_VARADD:
            case MN_BYTE | MN_VARADD | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) += stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARSUB:
            case MN_BYTE | MN_VARSUB | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) -= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARMUL:
            case MN_BYTE | MN_VARMUL | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) *= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARDIV:
            case MN_BYTE | MN_VARDIV | MN_UNSIGNED:
                if ((Uint8)stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                *(Uint8 *)(stack_ptr[-2]) /= (Uint8)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARMOD:
            case MN_BYTE | MN_VARMOD | MN_UNSIGNED:
                if ((Uint8)stack_ptr[-1] == 0) gr_error ("Error: Division por cero\n") ;
                *(Uint8 *)(stack_ptr[-2]) %= (Uint8)stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VAROR:
            case MN_BYTE | MN_VAROR | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) |= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARXOR:
            case MN_BYTE | MN_VARXOR | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) ^= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARAND:
            case MN_BYTE | MN_VARAND | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) &= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARROR:
                *(Sint8 *)(stack_ptr[-2]) >>= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARROR | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) >>= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARROL:
                *(Sint8 *)(stack_ptr[-2]) <<= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_BYTE | MN_VARROL | MN_UNSIGNED:
                *(Uint8 *)(stack_ptr[-2]) <<= stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            /* Operaciones directas con variables tipo FLOAT */
            case MN_FLOAT | MN_LETNP:
                (*(float *)(stack_ptr[-2])) = *(float *)&stack_ptr[-1] ;
                stack_ptr-=2 ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_LET :
                (*(float *)(stack_ptr[-2])) = *(float *)&stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_INC:
                (*(float *)(stack_ptr[-1])) += ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_FLOAT | MN_DEC:
                (*(float *)(stack_ptr[-1])) -= ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_FLOAT | MN_POSTDEC:
                (*(float *)(stack_ptr[-1])) -= ptr[1] ;
                stack_ptr[-1] = *(Uint32 *)(stack_ptr[-1]) + ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_FLOAT | MN_POSTINC:
                *((float *)(stack_ptr[-1])) += ptr[1] ;
                stack_ptr[-1] = *(Uint32 *)(stack_ptr[-1]) - ptr[1] ;
                ptr+=2 ;
                break ;

            case MN_FLOAT | MN_VARADD:
                *(float *)(stack_ptr[-2]) += *(float *)&stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_VARSUB:
                *(float *)(stack_ptr[-2]) -= *(float *)&stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_VARMUL:
                *(float *)(stack_ptr[-2]) *= *(float *)&stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            case MN_FLOAT | MN_VARDIV:
                if (*(float *)&stack_ptr[-1] == 0.0) gr_error ("Error: Division por cero\n") ;
                *(float *)(stack_ptr[-2]) /= *(float *)&stack_ptr[-1] ;
                stack_ptr-- ;
                ptr++ ;
                break ;

            /* Saltos */

            case MN_JUMP:
                ptr = base_code + ptr[1] ;
                continue ;

            case MN_JTRUE:
                stack_ptr-- ;
                if (*stack_ptr)
                {
                    ptr = base_code + ptr[1] ;
                    continue ;
                }
                ptr += 2 ;
                break ;

            case MN_JFALSE:
                stack_ptr-- ;
                if (!*stack_ptr)
                {
                    ptr = base_code + ptr[1] ;
                    continue ;
                }
                ptr += 2 ;
                break ;

            case MN_JTTRUE:
                if (stack_ptr[-1])
                {
                    ptr = base_code + ptr[1] ;
                    continue ;
                }
                ptr += 2 ;
                break ;

            case MN_JTFALSE:
                if (!stack_ptr[-1])
                {
                    ptr = base_code + ptr[1] ;
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
                if (switchval_string != 0) string_discard (switchval_string);
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
                    ptr = base_code + ptr[1] ;
                    continue ;
                }
                ptr += 2 ;
                break ;

            /* Control de procesos */

            case MN_TYPE:
                {
                PROCDEF * proct = procdef_get (ptr[1]) ;
                if (!proct) gr_error ("Error: Procedimiento desconocido\n") ;
                *stack_ptr++ = proct->type ;
                ptr += 2 ;
                break ;
                }

            case MN_FRAME:
                LOCDWORD(r,FRAME_PERCENT) += stack_ptr[-1];
                stack_ptr-- ;
                r->codeptr = ptr+1 ;
                stack_gptr = stack_ptr ;
                return_value = LOCDWORD(r, PROCESS_ID);

                if (!(r->proc->flags & PROC_FUNCTION) &&
                    r->called_by && instance_exists(r->called_by) &&
                    (LOCDWORD(r->called_by, STATUS) & STATUS_WAITING_MASK))
                {
                    /* We're returning and the parent is waiting: wake it up */
                    if (r->called_by->stack && !(r->called_by->stack[0] & STACK_NO_RETURN_VALUE)) {
                        r->called_by->stack[r->called_by->stack[0] & STACK_SIZE_MASK] = return_value;
                    }

                    LOCDWORD(r->called_by,STATUS) &= ~STATUS_WAITING_MASK;
                    r->called_by = NULL;
                }
                goto break_all ;

            case MN_END:
#if 0
                if (private_data == r->pridata) /* Esto es una nueva instancia */
#endif
                {
                    if (LOCDWORD(r,STATUS) != STATUS_DEAD) LOCDWORD(r,STATUS) = STATUS_KILLED ;
                    stack_gptr = stack_ptr ;
                    goto break_all ;
                }
#if 0
                return_value = LOCDWORD(r, PROCESS_ID);
                *stack_ptr++ = return_value;
#endif
            case MN_RETURN:
#if 0
                if (private_data == r->pridata) /* Esto es una nueva instancia */
#endif
                {
                    if (LOCDWORD(r,STATUS) != STATUS_DEAD) LOCDWORD(r,STATUS) = STATUS_KILLED ;
                    stack_ptr-- ;
                    stack_gptr = stack_ptr ;
                    return_value = *stack_ptr ;
                    goto break_all ;
                }
#if 0
                /* The process is inside a stack-call */
                proc = (PROCDEF *)stack_ptr[-4]; // stack_ptr[-4] = Recupero el proc de la instancia actual
                freestrings(proc, private_data);

                return_value = stack_ptr[-1];
                ptr = (int *)stack_ptr[-3];
                base_code = (int *)stack_ptr[-5];

                int * temp = private_data;
                private_data = (int *)stack_ptr[-2];

                if (stack_ptr[-6] == MN_CALL) {
                    /* Save a return value */
                    stack_ptr = temp;
                    *stack_ptr++ = return_value;
                } else {
                    stack_ptr = temp;
                }

                r->inpridata = NULL;
                r->inproc = NULL;
#endif
                break;

            /* Otros */

            case MN_DEBUG:
                if (dcb.data.NID) {
                    gr_con_printf ("\n----- DEBUG from %s(%d) -----\n", proc->name, LOCDWORD(r,PROCESS_ID) ) ;
                    debug_next = 1;
                }
                ptr++;
                break ;

            case MN_SENTENCE:
                trace_sentence     = ptr[1];
                trace_instance     = r;

                if (debug)
                {
                    printf ("%d: ", trace_sentence & 0xFFFFFF);
                    if (dcb.sourcecount[trace_sentence >> 24]) {
                        printf (" %s", dcb.sourcelines[trace_sentence >> 24] [(trace_sentence & 0xFFFFFF)-1]) ;
                    }
                    printf ("\n");
                    fflush(stdout) ;
                }

                ptr += 2 ;
                break ;

            default:
                gr_error ("Error: Mnemonico 0x%02X no implementado en %s\n", *ptr, proc->name) ;

        }

        /* Si me killearon o estoy en waiting salgo */
        if ((LOCDWORD(r,STATUS) & ~STATUS_WAITING_MASK) == STATUS_KILLED || (LOCDWORD(r,STATUS) & STATUS_WAITING_MASK)) {
            r->codeptr = ptr;
            stack_gptr = stack_ptr ;
            return_value = LOCDWORD(r, PROCESS_ID);
            goto break_all ;
        }

#ifdef EXIT_ON_EMPTY_STACK
        if (stack_ptr == stack)
        {
            r->codeptr = ptr ;
            if (LOCDWORD(r,STATUS) != STATUS_RUNNING && LOCDWORD(r,STATUS) != STATUS_DEAD)
                break ;
        }
#endif

    }

/* *** SALIDA GENERAL *** */
break_all:

    gprof_end (proc->name);

    if (!*ptr || *ptr == MN_RETURN || *ptr == MN_END || LOCDWORD(r, STATUS) == STATUS_KILLED) {
        /* Check for waiting parent */
        if (r->called_by && instance_exists(r->called_by) && (LOCDWORD(r->called_by, STATUS) & STATUS_WAITING_MASK))
        {
            /* We're returning and the parent is waiting: wake it up */
            if (r->called_by->stack && !(r->called_by->stack[0] & STACK_NO_RETURN_VALUE)) {
                r->called_by->stack[r->called_by->stack[0] & STACK_SIZE_MASK] = return_value;
            }

            LOCDWORD(r->called_by, STATUS) &= ~STATUS_WAITING_MASK;
        }
        r->called_by = NULL;

        /* The process should be destroyed immediately, it is a function-type one */
        /* Ejecuto ONEXIT */
        if ((LOCDWORD(r,STATUS) & ~STATUS_WAITING_MASK) != STATUS_DEAD && r->exitcode) {
            LOCDWORD(r,STATUS) = STATUS_DEAD;
            r->codeptr = r->exitcode;

            instance_go(r);
        } else {
            instance_destroy(r);
        }

        /* Apunto global stack a stack_begin (inicio del stack original) */
        stack_gptr = stack_begin;

        if (was_visible) object_list_dirty = 1;
    }
    else if (LOCDWORD(r, STATUS) != STATUS_KILLED && r->first_run)
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
        object_list_unsorted = 1;
    }

//    if (switchval_string != 0) string_discard(switchval_string);

    if (debug_next && trace_sentence != -1) {
        force_debug = 1;
        debug_next = 0;
    }

    /* Apunto global stack a stack_begin (inicio del stack original) */
/*    stack_gptr = stack_begin; */

    return return_value;
}
