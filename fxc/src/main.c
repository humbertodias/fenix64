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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#ifdef WIN32
	// NEEDED FOR LOCALE DETECTION
	#include <windows.h>
	#include <windef.h>
	#include <winnls.h>
	#include "Shlwapi.h"
#endif

#include "fxc.h"

#define FXC_VERSION "FXC " VERSION " (" __DATE__ " " __TIME__ ")"

#include "messages.c"
#include "errors.h"


/* ---------------------------------------------------------------------- */

int debug = 0 ;

#define MAX_FILES 64

int path_file = 5;
int n_files = 0 ;
char files[MAX_FILES][256] ;

char langinfo[64] ;

int load_file (char * filename)
{
#ifdef TARGET_MAC
    char fileMac[256] ;
    snprintf (fileMac, 256, "%s/%s", files[path_file], filename) ;
    snprintf (filename, 256, fileMac);
#endif

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
	char stubname[256] = "";
	int i, j ;

	printf (FXC_VERSION " - Copyright (C) 1999 JosÃ© Luis CebriÃ¡n PagÃ¼e\n") ;
	printf ("Fenix comes with ABSOLUTELY NO WARRANTY; see COPYING for details\n\n") ;

	// Default lang to EN
	strcpy(langinfo,"EN") ;
	// LANG detect
#ifdef WIN32
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SABBREVCTRYNAME,langinfo,64) ;
	strlwr(langinfo) ;
#else
	 if (getenv("LANG")!=NULL && strlen(getenv("LANG"))>=2)
		 strcpy(langinfo,getenv("LANG")) ;
#endif
	langinfo[2] = 0 ;

	srand (time(NULL)) ;

	/* Init SDL */

	if ( SDL_Init (SDL_INIT_VIDEO) < 0 )
	{
		printf ("SDL Init Error: %s\n", SDL_GetError()) ;
		exit(1) ;
	}

	/* Get command line parameters */

	for (i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			j = 1 ;
			while (argv[i][j])
			{
				if (argv[i][j] == 'd')
					debug = 1 ;

				if (argv[i][j] == 'c')
					dos_chars = 1 ;

				if (argv[i][j] == 'a')
					autoinclude = 1 ;

				if (argv[i][j] == 'g')
					dcb_options |= DCB_DEBUG ;

				if (argv[i][j] == 's')
				{
					// -s "stub": Use a stub

					if (argv[i][j+1])
						strncpy (stubname, argv[i]+j+1, 256);
					else if (argv[i+1] && argv[i+1][0] != '-')
						strncpy (stubname, argv[++i], 256);
					break;
				}

				if (argv[i][j] == 'f')
                {
					// -f "filename": Embed a file to the DCB

                    if (argv[i][j+1])
                           dcb_add_file (argv[i+j]+1) ;
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
					// -i "path": add a file to the path for include files

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

				if (argv[i][j] == 'l')
				{
					// -lLANG:	Set the language for errors and messages

					if (argv[i][j+1] == 0)
					{
						if (i != argc-1)
						{
							strcpy(langinfo,argv[i+1]) ;
						}
						i++ ;
						break ;
					}
					strcpy(langinfo,argv[i]+j+1) ;
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
		printf (MSG_USING MSG_OPTION_D MSG_OPTIONS, argv[0]) ;
		return 0 ;
	}

	strcpy (dcbname, filename) ;
	if (strrchr(dcbname, '.'))
		*strrchr(dcbname, '.') = 0 ;
	strcat (dcbname, ".dcb") ;

	memset (&dcb, 0, sizeof(dcb));

	/* build error messages list */
	err_buildErrorTable() ;

	init_c_type() ;
	identifier_init() ;
	constants_init() ;
	string_init () ;
	compile_init () ;
	div_init() ;

	load_file (filename) ;
	compile_program() ;

	if (stubname[0] != 0)
	{
		if (!file_exists(stubname))
		{
#ifdef WIN32
			char   exepath[MAX_PATH];

			GetModuleFileName(NULL, exepath, sizeof(exepath));
			PathRemoveFileSpec(exepath);
			strcat (exepath, "\\");
			memmove (stubname + strlen(exepath), stubname, strlen(stubname)+1);
			memcpy  (stubname, exepath, strlen(exepath));
#else
			const char * ptr = argv[0] + strlen(argv[0]);
			while (ptr > argv[0] && *ptr != '\\' && *ptr != '/') ptr--;
			if (*ptr == '\\' || *ptr == '/')
				ptr++;
			if (ptr > argv[0])
			{
				memmove (stubname + (ptr - argv[0]), stubname, strlen(stubname)+1);
				memcpy  (stubname, argv[0], ptr - argv[0]);
			}
#endif
			if (!file_exists(stubname))
			{
				strcat (stubname, ".exe");
				if (!file_exists(stubname))
				{
					compile_error ("Can't open stub file %s", stubname);
					return -1;
				}
			}
		}

#ifdef WIN32
		if (strchr(dcbname, '.'))
		{
			char * ptr = dcbname + strlen(dcbname);
			while (ptr > dcbname && *ptr != '.') ptr--;
			if (stricmp(ptr, ".dcb") == 0)
				strcpy(ptr, ".exe");
		}
#endif

		dcb_save (dcbname, dcb_options, stubname);
	}
	else
	{
		dcb_save (dcbname, dcb_options, NULL) ;
	}

	/* destroy error messages list */
	err_destroyErrorTable() ;

	return 1 ;
}

