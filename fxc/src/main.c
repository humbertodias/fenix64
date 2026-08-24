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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fxc.h"

#define VERSION "FXC 0.83 (" __DATE__ " " __TIME__ ")"

#include "messages.c"

/* ---------------------------------------------------------------------- */

int debug = 0 ;

#define MAX_FILES 64

int n_files = 0 ;
char files[MAX_FILES][256] ;

int load_file (char * filename)
{
	long   size ;
	file * fp = file_open (filename, "rb0") ;
	char * source ;

        if (n_files == MAX_FILES)
                compile_error (MSG_TOO_MANY_FILES) ;
        strcpy (files[n_files++], filename) ;

	if (!fp)
	{
		fprintf (stdout, MSG_FILE_NOT_FOUND "\n", filename) ;
		exit (1) ;
	}

	size = file_size (fp) ;

	source = (char *) malloc(size+1) ;
	if (!source)
	{
		fprintf (stdout, MSG_FILE_TOO_BIG "\n", filename) ;
		exit (1) ;
	}
	if (size == 0)
	{
		fprintf (stdout, MSG_FILE_EMPTY "\n", filename) ;
		exit (1) ;
	}

	if (!file_read (fp, source, size))
	{
		fprintf (stdout, MSG_READ_ERROR "\n", filename) ;
		exit (1) ;
	}
	source[size] = 0 ;
	file_close (fp) ;

	token_init (source, n_files-1) ;
        return n_files-1 ;
}

extern int full_screen, double_buffer ;

int dcb_options = 0 ;

int main (int argc, char **argv)
{
	char * filename = 0 ;
	char dcbname[256] ;
	int i, j ;
 
	printf (VERSION " - Copyright (C) 1999 José Luis Cebrián Pagüe\n") ;
	printf ("Fenix comes with ABSOLUTELY NO WARRANTY; see COPYING for details\n\n") ;

	for (i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			j = 1 ;
			while (argv[i][j])
			{
#ifdef DEBUG
				if (argv[i][j] == 'd') debug = 1 ;
#endif
				if (argv[i][j] == 'c') dos_chars = 1 ;
				if (argv[i][j] == 'a') autoinclude = 1 ;
				if (argv[i][j] == 'g') dcb_options |= DCB_DEBUG ;
				if (argv[i][j] == 'f')
                                {
                                        if (argv[i][j+1]) 
                                                dcb_add_file (argv[i]+j+1) ;
                                        else while (argv[i+1])
                                        {
                                                if (argv[i+1][0] == '-')
                                                        break ;
                                                dcb_add_file (argv[i+1]) ;
                                                i++ ;
                                        }
                                        break ;
                                }
				if (argv[i][j] == 'i')
				{
					if (argv[i][j+1] == 0)
					{
						if (i == argc-1)
						{
							printf (MSG_DIRECTORY_MISSING "\n") ;
							exit (1) ;
						}
						file_addp (argv[i+1]);
						i++ ;
						break ;
					}
					file_addp (argv[i]+j+1) ;
					break ;
				}
				j++ ;
			}
		}
		else
		{
			if (filename)
			{
				printf (MSG_TOO_MANY_FILES "\n") ;
				return 0 ;
			}
			filename = argv[i] ;
		}
	}
	if (!filename)
	{
		printf (MSG_USING
#ifdef DEBUG
			MSG_OPTION_D
#endif
                        MSG_OPTIONS,
			argv[0]) ;
		return 0 ;
	}

	strcpy (dcbname, filename) ;
	if (strrchr(dcbname, '.')) 
		*strrchr(dcbname, '.') = 0 ;
	strcat (dcbname, ".dcb") ;

	memset (&dcb, 0, sizeof(dcb));
	init_c_type() ;

	identifier_init() ;
	constants_init() ;
	string_init () ;
	compile_init () ;
	div_init() ;

	load_file (filename) ;
	compile_program() ;
	dcb_save (dcbname, dcb_options) ;

	return 1 ;
}

