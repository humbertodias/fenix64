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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "fxc.h"
#include "messages.c"

void codeblock_postprocess (CODEBLOCK * code)
{
	int * ptr = code->data ;
	PROCDEF * proc ;

	while (ptr < code->data + code->current)
	{
		if (*ptr == MN_CALL || *ptr == MN_PROC || *ptr == MN_TYPE)
		{
			proc = procdef_search (ptr[1]) ;
			if (!proc || !proc->defined)
			{
				token.type = IDENTIFIER ;
				token.code = ptr[1] ;
				// Patch so linecount gets right
				line_count = identifier_line(token.code) ;
				current_file = identifier_file(token.code) ;
				compile_error (MSG_UNDEFINED_PROC) ;
			}
			ptr[1] = proc->typeid ;
		}
		if (*ptr == MN_JUMP    || *ptr == MN_JFALSE || 
		    *ptr == MN_JNOCASE || *ptr == MN_CLONE)
		{
			ptr++ ;
			*ptr = code->labels[*ptr] ;
			ptr++ ;
			continue ;
		}
		if (*ptr == MN_REFALSE)
		{
			*ptr++ = MN_JFALSE ;
			*ptr = code->loops[*ptr*2] ;
			ptr++ ;
			continue ;
		}
		if (*ptr == MN_REPEAT || *ptr == MN_RETRUE)
		{
			*ptr = (*ptr == MN_REPEAT ? MN_JUMP : MN_JTRUE) ;
			ptr++ ;
			*ptr = code->loops[*ptr*2] ;
			ptr++ ;
			continue ;
		}
		if (*ptr == MN_BREAK || *ptr == MN_BRFALSE)
		{
			*ptr = (*ptr == MN_BREAK ? MN_JUMP : MN_JFALSE) ;
			ptr++ ;
			*ptr = code->loops[*ptr*2 + 1] ;
			ptr++ ;
			continue ;
		}
		ptr+=MN_PARAMS(*ptr)+1 ;
	}
}

void codeblock_init(CODEBLOCK * c)
{
	c->data = (int *) malloc (64 * sizeof(int)) ;
	c->loops = (int *) malloc (16 * sizeof(int)) ;
	c->labels = (int *) malloc (16 * sizeof(int)) ;
	c->reserved = 64 ;
	c->loop_reserved = 8 ;
	c->loop_count = 1 ;
	c->loop_active = 0 ;
	c->label_count = 0 ;
	c->label_reserved = 16 ;
	c->current = 0 ;
	if (!c->data || !c->loops || !c->labels)
	{
		fprintf (stdout, "CODEBLOCK: sin memoria\n") ;
		exit (1) ;
	}
}

void codeblock_alloc (CODEBLOCK * c, int count)
{
	c->reserved += count ;
	c->data = (int *) realloc (c->data, c->reserved * sizeof(int)) ;
	if (!c->data)
	{
		fprintf (stdout, "CODEBLOCK: sin memoria\n") ;
		exit (1) ;
	}
}

void codeblock_loop_alloc (CODEBLOCK * c, int count)
{
	c->loop_reserved += count ;
	c->loops = (int *) realloc (c->loops, c->loop_reserved * sizeof(int) * 2) ;
	if (!c->loops)
	{
		fprintf (stdout, "CODEBLOCK: sin memoria\n") ;
		exit (1) ;
	}
}

extern int  codeblock_loop_add (CODEBLOCK * c) 
{
	return c->loop_count++ ;
}

void codeblock_loop_start (CODEBLOCK * c, int loop, int begin)
{
	if (c->loop_reserved <= loop)
		codeblock_loop_alloc (c, loop + 8 - c->loop_reserved) ;
	if (c->loop_count <= loop)
		c->loop_count = loop+1 ;
	c->loops[loop*2] = begin ;
}

void codeblock_loop_end (CODEBLOCK * c, int loop, int end)
{
	if (c->loop_reserved <= loop)
		codeblock_loop_alloc (c, loop + 8 - c->loop_reserved) ;
	if (c->loop_count <= loop)
		c->loop_count = loop+1 ;
	c->loops[loop*2+1] = end ;
}

void codeblock_label_alloc (CODEBLOCK * c, int count)
{
	c->label_reserved += count ;
	c->labels = (int *) realloc (c->labels, c->label_reserved * sizeof(int) * 2) ;
	if (!c->labels)
	{
		fprintf (stdout, "CODEBLOCK: sin memoria\n") ;
		exit (1) ;
	}
}

int codeblock_label_add (CODEBLOCK * c)
{
	c->label_count++ ;
	if (c->label_count == c->label_reserved)
		codeblock_label_alloc (c, c->label_count + 16) ;
	return c->label_count - 1 ;
}

void codeblock_label_set (CODEBLOCK * c, int label, int offset)
{
	c->labels[label] = offset ;
}

void codeblock_add (CODEBLOCK * c, int code, int param)
{
	if (!c) return ;

	if (MN_PARAMS(code) == 0 && param)
	{
		printf ("Aviso: mnemónico 0x%02X no recibe parámetros\n", code) ;
	}

	if (c->current > 0)
	{
		if (code == MN_ARRAY && c->data[c->previous] == MN_PUSH)
		{
			c->data[c->previous] = MN_INDEX ;
			c->data[c->previous+1] *= param ;
			return ;
		}
		else if (code == MN_ADD && c->data[c->previous] == MN_PUSH)
		{
			c->data[c->previous] = MN_INDEX ;
			return ;
		}
		else if (code == MN_SUB && c->data[c->previous] == MN_PUSH)
		{
			c->data[c->previous] = MN_INDEX ;
			c->data[c->previous+1] = -c->data[c->previous+1] ;
			return ;
		}
		else if (code == MN_INDEX)
		{
			if (c->data[c->previous] == MN_INDEX)
			{
				c->data[c->previous+1] += param ;
				return ;
			}
			if (c->data[c->previous] == MN_GLOBAL
			 || c->data[c->previous] == MN_LOCAL
			 || c->data[c->previous] == MN_PRIVATE
			 || c->data[c->previous] == MN_REMOTE)
			{
				c->data[c->previous+1] += param ;
				return ;
			}
		}
		else if (code == MN_POP)
		{
			switch (c->data[c->previous])
			{
				case MN_LET:
					c->data[c->previous] = MN_LETNP ;
					return ;
				case MN_CALL:
					c->data[c->previous] = MN_PROC ;
					return ;
				case MN_SYSCALL:
					c->data[c->previous] = MN_SYSPROC ;
					return ;
			}
		}
		else if ((code & 0xFF) == MN_PTR)
		{
			/* Mismo caso */

			switch (c->data[c->previous])
			{
				case MN_PRIVATE:
					c->data[c->previous] = MN_GET_PRIV | (code & ~0xFF) ;
					return ;
				case MN_LOCAL:
					c->data[c->previous] = MN_GET_LOCAL | (code & ~0xFF) ;
					return ;
				case MN_GLOBAL:
					c->data[c->previous] = MN_GET_GLOBAL | (code & ~0xFF) ;
					return ;
				case MN_REMOTE:
					c->data[c->previous] = MN_GET_REMOTE | (code & ~0xFF) ;
					return ;
			}
		}
	}

	c->previous = c->current ;

	c->data[c->current++] = code ;
	if (MN_PARAMS(code) > 0) 
		c->data[c->current++] = param ;

	if (c->current+2 >= c->reserved) codeblock_alloc (c, 32) ;
}

