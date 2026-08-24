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
#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <stdlib.h>

#include "fxi.h"

typedef struct _node
{
	unsigned int x, y ;
        double f, g, h ;
	struct _node * parent ;
	struct _node * next ;
}
node ;

static int  * path_result = NULL ;
static int  * path_result_pointer = NULL ;

static node * open = NULL ;
static node * closed = NULL ;
static node * found = NULL ;

static int destination_x, destination_y ;
static int startup_x, startup_y ;

static GRAPH * map ;

static int block_if = 1 ;

double heuristic (int x, int y)
{
	int dx, dy ;
	Uint8 block = ((Uint8*)map->data)[map->pitch*y+x] ;

	if (x == destination_x && y == destination_y)
		return 0 ;
	if (block >= block_if) 
		return 1073741824.0 ;
	if (x < 0 || y < 0 || x >= map->width || y >= map->height) 
		return 1073741824.0 ;

	dx = abs(destination_x - x) ;
	dy = abs(destination_y - y) ;
	return (double)block + (double)dx*dx + (double)dy*dy ; 
}

int path_set_wall (int n)
{
	if (n >= 1) block_if = n ;
	return block_if ;
}

/* ---------------------------------------------------------------- */

/* Uso: open = add(open, this) ; */
/* La lista permanecerá ordenada */
node * node_add (node * list, node * this)
{
	node * curr = list ;
	if (!curr)
	{
		this->next = NULL ;
		return this ;
	}
	if (curr->f > this->f)
	{
		this->next = curr ;
		return this ;
	}

	for (;;)
	{
		if (!curr->next)
		{
			curr->next = this ;
			this->next = NULL ;
			return list ;
		}
		if (curr->next->f > this->f)
		{
			this->next = curr->next ;
			curr->next = this ;
			return list ;
		}
		curr = curr->next ;
	}
}

/* Uso: open = remove(open, this) ; */
node * node_remove (node * list, node * this)
{
	node * curr = list ;
	if (curr == this)
		return this->next ;
	while (curr)
	{
		if (curr->next == this)
		{
			curr->next = this->next ;
			return list ;
		}
		curr = curr->next ;
	}
	return list ;
}

node * node_find (node * list, int x, int y)
{
	while (list)
	{
		if (list->x == (unsigned)x && list->y == (unsigned)y)
			return list ;
		list = list->next ;
	}
	return NULL ;
}

node * node_reset (node * list)
{
	node * next ;
	while (list)
	{
		next = list->next ;
		free (list) ;
		list = next ;
	}
	return NULL ;
}

node * node_new (node * parent, int x, int y, int cost_inc)
{
	node * curr ;

	curr = (node *) malloc(sizeof(node)) ;
	if (!curr) return NULL ;

	curr->x = x ;
	curr->y = y ;
	curr->g = (parent ? parent->g : 0) + cost_inc ;
	curr->h = heuristic (x, y) ;
	curr->f = curr->g + curr->h ;

	curr->parent = parent ;
	curr->next   = NULL ;
	return curr ;
}

void node_push_succesor (node * parent, int ix, int iy, int cost)
{
	node * curr, * f_op, * f_cl ;
	
	curr = node_new (parent, parent->x+ix, parent->y+iy, cost) ;
	if (curr->h > 131072) { free(curr); return ; }

	f_cl = node_find (closed, curr->x, curr->y) ;
	if (f_cl) { free(curr); return ; }
	f_op = node_find (open, curr->x, curr->y) ;
	if (f_op && f_op->f <= curr->f) { free(curr); return ; }

	if (f_op) open = node_remove (open, f_op) ;
	if (f_cl) closed = node_remove (closed, f_cl) ;
	open = node_add (open, curr) ;
}

void node_push_succesors (node * parent, int options)
{
	node * prior = parent->parent ;
	if (!prior) prior = parent ;

	node_push_succesor (parent, 1, 0, prior->x < parent->x ? 9:10) ;
	node_push_succesor (parent, 0, 1, prior->x > parent->x ? 9:10) ;
	node_push_succesor (parent,-1, 0, prior->y < parent->y ? 9:10) ;
	node_push_succesor (parent, 0,-1, prior->y > parent->y ? 9:10) ;

	if (!(options & 1))
	{
		node_push_succesor (parent, 1, 1, 12) ;
		node_push_succesor (parent,-1,-1, 12) ;
		node_push_succesor (parent,-1, 1, 12) ;
		node_push_succesor (parent, 1,-1, 12) ;
	}
}

/* ---------------------------------------------------------------- */

int path_get (int * x, int * y)
{
	if (path_result_pointer)
	{
		(*x) = *path_result_pointer++ ;
		(*y) = *path_result_pointer++ ;
		if (*path_result_pointer == -1)
			path_result_pointer = NULL ;
		return 1 ;
	}
	return 0 ;
}

int path_find (GRAPH * bitmap, int sx, int sy, int dx, int dy, int options)
{
	node * curr ;

	startup_x = sx ;
	startup_y = sy ;
	map = bitmap ;
	destination_x = dx ;
	destination_y = dy ;

	open = node_reset (open) ;
	closed = node_reset (closed) ;

	curr = node_new (NULL, startup_x, startup_y, 0) ;
	curr->f = curr->h = 1 ;
	open = node_add (open, curr) ;

	while (open)
	{
		curr = open ;
		open = node_remove (open, curr) ;

		if (curr->x == (unsigned)destination_x && curr->y == (unsigned)destination_y)
		{
			int count = 1 ;

			found = curr ;
			while (curr->parent) {
				count++ ;
				curr = curr->parent ;
			}
			if (path_result) free(path_result) ;
			path_result = malloc(sizeof(int) * 2 * (count+4)) ;
			if (!(options & 2))
			{
				path_result_pointer = path_result + count*2 + 1;
				*path_result_pointer-- = -1 ;
				*path_result_pointer-- = -1 ;
				while (found) {
					*path_result_pointer-- = found->y ;
					*path_result_pointer-- = found->x ;
					found = found->parent ;
				}
				assert (path_result_pointer == path_result-1) ;
			}
			else
			{
				path_result_pointer = path_result ;
				while (found) {
					*path_result_pointer++ = found->x ;
					*path_result_pointer++ = found->y ;
					found = found->parent ;
				}
				*path_result_pointer++ = -1 ;
				*path_result_pointer++ = -1 ;
			}
			path_result_pointer = path_result ;
			return 1 ;
		}

		node_push_succesors (curr, options) ;

		closed = node_add (closed, curr) ;
	}

	return 0 ;
}
