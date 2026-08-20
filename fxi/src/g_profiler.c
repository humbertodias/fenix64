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
 * FILE        : g_profiler.c
 * DESCRIPTION : Profiler module
 *
 * HISTORY:      0.81 - First version
 */

#include <string.h>
#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <math.h>
#include <limits.h>

#include <fxi.h>

typedef struct
{
	const char * name;

	int		valid;
	int		active;
	int		called;

	long	start;
	long	accumulator;
	long	children;
	int     parent;
	int		parent_count;
}
PROFILE_SAMPLE;

typedef struct
{
	const char * name;

	int		valid;
	double  total;
	double	average;
	double  min;
	double  max;
	double  last;
}
PROFILE_HISTORY;

static PROFILE_SAMPLE  * samples = NULL;
static PROFILE_HISTORY * history = NULL;
static long  gprof_frame_start = 0;
static long  gprof_frame_count = 0;
static int   gprof_allocated = 0;
static int   gprof_sample_count = 0;
static int   gprof_history_count = 0;
static int   gprof_active = 0;
static int   gprof_activate = 1;

/*
 *  FUNCTION : gprof_allocate
 *
 *  Internal function used to allocate more profile samples. The number
 *  of profile entries has no internal limit.
 *
 *  PARAMS :
 *		count			New value for gprof_allocated
 *
 *  RETURN VALUE :
 *      None
 */

static int gprof_allocate (int count)
{
	int i;

	assert (count > gprof_allocated);

	/* Alloc more dynamic memory */

	samples = (PROFILE_SAMPLE  *) realloc (samples, count * sizeof(PROFILE_SAMPLE));
	history = (PROFILE_HISTORY *) realloc (history, count * sizeof(PROFILE_HISTORY));

	if (samples == NULL || history == NULL)
	{
		gr_error ("gprof_allocate: sin memoria");
		samples = NULL;
		history = NULL;
		return 0;
	}

	/* Initialize the new slots */

	for (i = gprof_sample_count ; i < gprof_allocated ; i++)
		samples[i].valid = 0;

	for (i = gprof_history_count ; i < gprof_allocated ; i++)
		history[i].valid = 0;

	gprof_allocated = count;
	return 1;
}

/*
 *  FUNCTION : gprof_compare
 *
 *  Internal function used to sort the list of sample history.
 *  A sample is drawn first if
 *
 *		- Both are top-level samples and a < b
 *		- Both are children of the same parent and a.average < b.average
 *		- The parent of a should be drawn before b or its parent
 *
 *  PARAMS :
 *		a				Pointer to a sample number
 *		b				Pointer to a sample number
 *
 *  RETURN VALUE :
 *      < 1 if sample a should be drawn first
 *        0 if it does not matter
 *		> 1 if sample b should be drawn first
 */

static int gprof_compare (const void * va, const void * vb)
{
	const int * a = va;
	const int * b = vb;
	if (samples[*b].parent == samples[*a].parent)
	{
		int timediff;

		/* Both are top level */
		if (samples[*a].parent == -1)
			return *a - *b ;

		/* Children of the same parent */
		timediff = (int)((history[*a].average - history[*b].average) * 100.0);
		if (timediff == 0)
			return strcmp (samples[*a].name, samples[*b].name);
		return -timediff;
	}

	/* Neither are top-level */
	if (samples[*b].parent != -1 && samples[*a].parent != -1)
	{
		return samples[*a].parent - samples[*b].parent;
	}

	/* b is top level, a is not */
	if (samples[*a].parent != -1)
	{
		if (*b == samples[*a].parent)
			return 1;
		return samples[*a].parent - *b;
	}

	/* a is top level, b is not */
	if (samples[*b].parent != -1)
	{
		if (*a == samples[*b].parent)
			return -1;
		return *a - samples[*b].parent;
	}

	assert (!"Not reached");
	return 0;
}

/*
 *  FUNCTION : gprof_store
 *
 *  Internal function used to store a sample into the history
 *
 *  PARAMS :
 *		sample			Pointer to the sample object
 *      total			Time elapsed after frame start
 *
 *  RETURN VALUE :
 *      None
 */

static void gprof_store (int i, int total)
{
	double value;
	PROFILE_SAMPLE * sample = &samples[i];

	if (!gprof_active) return;

	assert (total > 0);

	/* Get a % */

	assert (sample->accumulator >= 0);
	value = sample->accumulator * 100.0 / total;

	/* Initialize history */

	if (!history[i].valid)
	{
		history[i].valid = 1;
		history[i].name = sample->name;
		history[i].total = 0;
	}

	/* Update history */

	if (history[i].total == 0)
	{
		history[i].total = value;
		history[i].max   = value;
		history[i].min   = value;
	}
	else
	{
		history[i].total += value;
		if (history[i].min > value)
			history[i].min = value;
		if (history[i].max < value)
			history[i].max = value;
	}

	history[i].last    = value;
	history[i].average = (double)history[i].total / gprof_frame_count;
}

/*
 *  FUNCTION : gprof_init
 *
 *  Initialize the profiler. This should be called at program startup once.
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_init()
{
	int i;

	samples = (PROFILE_SAMPLE  *) malloc(32 * sizeof(PROFILE_SAMPLE));
	history = (PROFILE_HISTORY *) malloc(32 * sizeof(PROFILE_HISTORY));

	if (samples == NULL || history == NULL)
	{
		gr_error ("gprof_init: sin memoria");
		return;
	}

	gprof_allocated = 32;

	for (i = 0 ; i < gprof_allocated ; i++)
	{
		samples[i].valid = 0;
		history[i].valid = 0;
	}
}

/*
 *  FUNCTION : gprof_begin
 *
 *  Starts the timing of a program block
 *
 *  PARAMS :
 *		name		Name of the block (it must the same pointer each time,
 *					use a constant string or a constant pointer)
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_begin (const char * name)
{
	static int last_entry = -1;
	int i, available = -1;

	if (!gprof_active) return;

	/* Search for already-existing profile */

	for (i = 0 ; i < gprof_sample_count ; i++)
	{
		if (samples[i].valid)
		{
			if (strcmp(samples[i].name, name) == 0)
			{
				if (samples[i].active) {
//					gr_con_printf ("gprof_begin: bloque %s ya activo", name);
					return;
				}
				samples[i].called++;
				samples[i].active++;
				samples[i].start = SDL_GetTicks();
				last_entry = i;
				return;
			}
		}
		else if (available == -1)
		{
			available = i;
		}
	}

	/* No available slot */

	if (available == -1)
	{
		if (gprof_sample_count == gprof_allocated)
		{
			if (!gprof_allocate (gprof_allocated + 32))
				return;
		}
		available = gprof_sample_count++;
	}

	samples[available].name = name;
	samples[available].valid = 1;
	samples[available].parent_count = 0;
	samples[available].children = 0;
	samples[available].start = SDL_GetTicks();
	samples[available].active = 1;
	samples[available].parent = -1;
	samples[available].called = 1;

	/* Count parents (any active sample) */

	for (i = 0 ; i < gprof_sample_count ; i++)
	{
		if (samples[i].active && i != available)
		{
			samples[available].parent_count++;
			if (samples[available].parent_count == 1)
				samples[available].parent = i;
			else if (samples[samples[available].parent].accumulator > samples[i].accumulator)
				samples[available].parent = i;
		}
	}
}

/*
 *  FUNCTION : gprof_end
 *
 *  End the timing of a program block
 *
 *  PARAMS :
 *		name		Name of the block (same pointer used in gprof_start!)
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_end (const char * name)
{
	int  i;
	long now;
	long elapsed;

	if (!gprof_active) return;

	/* Search the profile */

	for (i = 0 ; i < gprof_sample_count ; i++)
	{
		if (samples[i].valid && strcmp (samples[i].name, name) == 0)
		{
			if (samples[i].active < 1)
			{
//				gr_con_printf ("gprof_end: bloque %s inactivo", name);
				return;
			}
			break;
		}
	}
	if (i == gprof_sample_count)
	{
//		gr_con_printf ("gprof_end: no se encontró el bloque %s", name);
		return;
	}

	/* Stop the timing */

	now = SDL_GetTicks();
	elapsed = now - samples[i].start;

	/* TODO: Fix this assert */
	//assert (samples[i].children <= elapsed);

	samples[i].active--;
	samples[i].accumulator = elapsed ; /* - samples[i].children? */

	/* Search for parents */

	if (samples[i].parent != -1)
	{
		int parent = samples[i].parent;
		samples[parent].children += elapsed;
	}
}

/*
 *  FUNCTION : gprof_frame
 *
 *  Call this function at frame start time. It will close any opened
 *  profile samples and update the profiler history.
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_frame()
{
	int  i;
	int  count;
	int  min_parent_count;
	long now;
	long elapsed;

	if (gprof_activate != gprof_active)
	{
		if (gprof_activate != 0)
		{
			gprof_active = 1;
			if (gprof_activate < 0)
				gprof_activate++;
		}
		else gprof_active = 0;
	}

	if (!gprof_active) return;

	/* Close any open samples, childs first */

	do
	{
		min_parent_count = -1;
		count = 0;

		/* Count active samples and find those with less parents */

		for (i = 0 ; i < gprof_sample_count ; i++)
		{
			if (samples[i].valid && samples[i].active)
			{
				if (min_parent_count < samples[i].parent_count)
					min_parent_count = samples[i].parent_count;
				count++;
			}
		}

		if (min_parent_count == -1)
			break;

		/* Stop found samples */

		for (i = 0 ; i < gprof_sample_count ; i++)
		{
			if (samples[i].valid && samples[i].active)
			{
				if (min_parent_count == samples[i].parent_count)
				{
					gprof_end (samples[i].name);
					count--;
				}
			}
		}
	}
	while (count > 0);

	/* First frame? */

	if (gprof_frame_count == 0)
	{
		for (i = 0 ; i < gprof_sample_count ; i++)
		{
			samples[i].called = 0;
			samples[i].accumulator = 0;
			samples[i].children = 0;
		}

		gprof_frame_start = SDL_GetTicks();
		gprof_frame_count++;
		return;
	}
/*
	if (gprof_activate != gprof_active)
	{
		if (gprof_activate != 0)
		{
			gprof_active = 1;
			if (gprof_activate < 0)
				gprof_activate++;
		}
		else gprof_active = 0;
	}

	if (!gprof_active) return;
*/
	/* Update frame information */

	now = SDL_GetTicks();
	elapsed = now - gprof_frame_start;
	if (elapsed < 1)
		return;

	gprof_frame_start = now;
	gprof_frame_count++;

	/* Store sample history */

	for (i = 0 ; i < gprof_sample_count ; i++)
	{
		if (samples[i].called)
		{
			gprof_store (i, elapsed);

			samples[i].called = 0;
			samples[i].accumulator = 0;
			samples[i].children = 0;
		}
	}
}

/*
 *  FUNCTION : gprof_dump
 *
 *  Dumps profiler history to a text file (in append mode)
 *
 *  PARAMS :
 *		filename		Name of the file
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_dump (const char * filename)
{
	int    i = 0;
	int    n;
	int    parents_of = -1;
	FILE * f = fopen (filename, "a");

	if (f == NULL) return;

	fputs ("\n", f);
	fputs ("-------------------------------------------------------\n", f);
	fputs ("  Avg  :  Min  :  Max  :  Last :  Profile name         \n", f);
	fputs ("-------------------------------------------------------\n", f);

	for (;;)
	{
		if (i == gprof_sample_count)
		{
			if (parents_of == -1)
				break;
			i = parents_of;
			parents_of = samples[i].parent;
			i++;
			continue;
		}

		if (samples[i].parent != parents_of)
		{
			i++;
			continue;
		}

		/* Write the history information */

		fprintf (f, "%6.02f :%6.02f :%6.02f :%6.02f :  ", history[i].average, history[i].min,  history[i].max, history[i].last);

		for (n = 0 ; n < samples[i].parent_count ; n++)
			fputs ("  ", f);

		fputs (samples[i].name, f);
		fputs ("\n", f);

		/* Now, search children */

		parents_of = i;
		i = 0;
	}

	fputs ("-------------------------------------------------------\n", f);
	fclose (f);
}

/*
 *  FUNCTION : gprof_reset
 *
 *  Reset the profile history
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_reset ()
{
	int i;

	for (i = 0 ; i < gprof_sample_count ; i++)
		history[i].valid = 0;

	gprof_frame_count = 0;

	if (!gprof_active)
	{
		gprof_activate = -2;
	}
}

/*
 *  FUNCTION : gprof_draw
 *
 *  Draw the current profile history to a bitmap
 *
 *  PARAMS :
 *		dest			Destination graphic
 *
 *  RETURN VALUE :
 *      None
 */

#define CHARWIDTH 6
#define CHARHEIGHT 8

/* This macro is used to find worthless samples. A worthless sample is one
 * that could be ignored if there is not enough screen space the full list */

#define ISWORTHLESS(i) (samples[i].parent != -1)

void gprof_draw (GRAPH * dest)
{
	int    x, y;
	int    i, c;
	int    n;
	char   buffer[100];
	int    count;
	int    count_worthless = 0;
	int    show_worthless = 0;
	int  * list;

	if (!gprof_active) return;

	/* Count total history lines */
	for (i = count = count_worthless = 0 ; i < gprof_sample_count ; i++)
	{
		if (history[i].valid)
		{
			count++;
			if (ISWORTHLESS(i)) count_worthless++;
		}
	}

	if (count == 0) return;

	gprof_begin ("Profiler");

	x = (dest->width - 50*CHARWIDTH)/2 ;
	y = 5;

	/* Sort the list */

	list = malloc(sizeof(int) * count);
	if (list == NULL) return;
	for (i = c = 0 ; i < gprof_sample_count ; i++)
	{
		if (history[i].valid)
			list[c++] = i;
	}
	assert (c == count);
	qsort (list, count, sizeof(int), gprof_compare);

	show_worthless = (dest->height/CHARHEIGHT - 7) - (count - count_worthless);

	/* Draw the header */

	gr_sys_color (0xFFFFFF, 0x404040);
	gr_sys_puts (dest, x, y+=CHARHEIGHT, "\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02",50);
	gr_sys_puts (dest, x, y+=CHARHEIGHT, "  Avg   Min   Max  Last : Profile name            ",50);
	gr_sys_puts (dest, x, y+=CHARHEIGHT, "\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02",50);

	/* Draw the profiler information box */

	for (c = 0 ; c < count ; c++)
	{
		i = list[c];
		if (!history[i].valid) continue;

		if (show_worthless > 0 || !ISWORTHLESS(i))
		{
			if (ISWORTHLESS(i))
				show_worthless--;

			/* Write the history information */

			_snprintf (buffer, 100, "%5.01f %5.01f %5.01f %5.01f : ",
				history[i].average, history[i].min, history[i].max, history[i].last);

			for (n = 0 ; n < samples[i].parent_count ; n++)
				strncat (buffer, "  ", 99-strlen(buffer));

			strncat (buffer, samples[i].name, 99-strlen(buffer));
			gr_sys_puts (dest, x, y+=CHARHEIGHT, buffer, 50);
		}
		else if (ISWORTHLESS(i))
		{
			if (show_worthless == 0)
				gr_sys_puts (dest, x, y+=CHARHEIGHT, "   -     -     -     -  :   .......               ",50);
			show_worthless--;
		}
	}

	gr_sys_puts (dest, x, y+=CHARHEIGHT, "\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02",50);

	free (list);

	gprof_end ("Profiler");
}

/*
 *  FUNCTION : gprof_toggle
 *
 *  Toggles the profiler
 *
 *  PARAMS :
 *		dest			Destination graphic
 *
 *  RETURN VALUE :
 *      None
 */

void gprof_toggle()
{
	background_dirty = 1;
	gprof_activate = !gprof_activate;
}
