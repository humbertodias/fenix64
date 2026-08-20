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
 * FILE        : main.c
 * DESCRIPTION : Main entry point for FXI
 *
 * HISTORY:	0.81 - Removed -w option
 *
 */

/*
 * INCLUDES
 */

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fxi.h"
#include "font.h"
#include "dcb.h"

/* os versions */
#ifdef WIN32
	#define OS_ID 0
#endif
#ifdef TARGET_Linux
	#define OS_ID 1
#endif
#ifdef TARGET_BEOS
	#define OS_ID 2
#endif
#ifdef TARGET_MAC
	#define OS_ID 3
#endif
#ifdef TARGET_GP32
	#define OS_ID 4
#endif
#ifdef TARGET_DC
	#define OS_ID 5
#endif
#ifdef TARGET_BSD
	#define OS_ID 6
#endif
#ifdef WIN32
#include <windows.h>
#endif

#define DCB_MAGIC	"DCB Stub\x1A\x0D\x0A"
/*
 *  GLOBAL VARIABLES
 */

int debug	 = 0;		/* 1 if running in debug mode					 */
int fxi		 = 0;		/* 1 only if this is an standalone interpreter   */
int embedded = 0;		/* 1 only if this is a stub with an embedded DCB */

#ifdef TARGET_MAC
    int current_file = 0;
    char files[][256];
#endif

extern int full_screen, double_buffer ;
extern char *apptitle;


/*
 *  FUNCTION : do_exit
 *
 *  Exits from the program cleanly ending operations
 *
 *  PARAMS:
 *      INT n: ERROR LEVEL to return to OS
 *
 *  RETURN VALUE:
 *      No value
 *
 */

void
do_exit(int n)
{
    int i;

	if (keytab_initialized) keytab_free() ;

	for (i=0;i<MAX_JOYS;i++)
	    if (_joysticks[i]!=NULL) SDL_JoystickClose(_joysticks[i]) ;

	SDL_Quit() ;
	exit(n) ;
}

/*
 *  FUNCTION : main
 *
 *  Main function for FXI
 *
 *  PARAMS:
 *      INT n: ERROR LEVEL to return to OS
 *
 *  RETURN VALUE:
 *      No value
 *
 */

int main (int argc, char **argv)
{
	char *				filename = 0 ;
	char				dcbname[256] ;
	INSTANCE *			mainproc_running ;
	int					i, j ;
	int					norun = 0 ;
	const SDL_version * sdl_version ;
	char *				ptr ;
	file *				fp = NULL;

	struct
	{
		char	magic[12];
		int		dcb_offset;
	}
	dcb_signature;

	/* Find out if we are calling fxi.exe or whatever.exe */

 	ptr = argv[0] + strlen(argv[0]) ;
 	while (ptr > argv[0] && ptr[-1] != '\\' && ptr[-1] != '/')
 		ptr-- ;
 	fxi = (strncmp(ptr,"fxi",3) == 0) || (strncmp(ptr,"FXI",3) == 0) ;

	/* Init RAND generator */

	srand (time(NULL)) ;

	/* Init SDL */

	if ( SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_CDROM) < 0 )
	{
		printf ("SDL Init Error: %s\n", SDL_GetError()) ;
		do_exit(1) ;
	}

	SDL_JoystickEventState (SDL_ENABLE) ;

    gr_con_printf ("\xAC" "15" FXI_VERSION) ;

	if (fxi)
	{
		/* Standalone interpreter, dump some info */

		sdl_version = SDL_Linked_Version();
		gr_con_printf ("\xAC" "14SDL: %d.%d.%d (DLL loaded: %d.%d.%d)",
				SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL,
				sdl_version->major, sdl_version->minor, sdl_version->patch) ;
	}
	else
	{
		/* Hand-made interpreter: search for DCB at EOF */

		if (!file_exists(argv[0]))
		{
#ifdef WIN32
			char   exepath[MAX_PATH];

			GetModuleFileName(NULL, exepath, sizeof(exepath));
			fp = file_open(exepath, "rb0");
#endif
		}
		else
			fp = file_open(argv[0], "rb0");

		if (fp != NULL)
		{
			file_seek (fp, -(int)sizeof(dcb_signature), SEEK_END);
			file_read (fp, &dcb_signature, sizeof(dcb_signature));

			if (strcmp(dcb_signature.magic, DCB_MAGIC) == 0)
			{
				ARRANGE_DWORD (&dcb_signature.dcb_offset);

				filename = argv[0];
				embedded = 1;
			}
		}

		if (!embedded)
		{
			/* No embedded DCB; search for a DCB with similar name */

			filename = ptr ;
	 		while (*ptr && *ptr != '.') ptr++ ;
	 		*ptr = 0 ;
		}
	}

 	if (fxi)
	{
		/* Calling FXI.EXE so we must get all command line params */

		for (i = 1 ; i < argc ; i++)
		{
			if (argv[i][0] == '-')
			{
				j = 1 ;
				while (argv[i][j])
				{
					if (argv[i][j] == 'd') debug = 1 ;
					if (argv[i][j] == 'r') norun = 1 ;
					//if (argv[i][j] == 'b') double_buffer = 1 ;
					if (argv[i][j] == 'f') enable_filtering = 1 ;

					if (argv[i][j] == 'i')
					{
						if (argv[i][j+1] == 0)
						{
							if (i == argc-1)
								gr_error ("You must provide a directory") ;
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
				if (!filename)
				{
					filename = argv[i] ;
					if (i < argc-1)
						memmove (&argv[i], &argv[i+1],
							sizeof(char*) * (argc-i-1)) ;
					argc-- ;
					i-- ;
				}
			}
		}

		if (!filename)
		{
			gr_error ( FXI_VERSION "\nCopyright(C) 2002 Fenix Team\nCopyright (C)1999 Jose Luis Cebrian\n"
				"Fenix comes with ABSOLUTELY NO WARRANTY; see COPYING for details\n\n"
				"Usage: %s [options] file.dcb\n\n"
				"   -d       Activate DEBUG mode\n"
				"   -f       16bpp Filter ON (only 16bpp color mode)\n"
				"This program is free software dsitributed under.\n\n"
				"GNU General Public License published by Free Software Foundation.\n"
				"Permission granted to distribute and/or modify as stated in the license\n"
				"agreement (GNU GPL version 2 or later).\n"
				"See COPYING for license details.\n",
				argv[0]) ;
			return 0 ;
		}
	}

	/* Initialization (modules needed before dcb_load) */

	gprof_init () ;
	string_init () ;
	init_c_type() ;

	/* Init application title for windowed modes */

	strcpy (dcbname, filename) ;
	apptitle = strdup(filename) ;

#ifdef TARGET_MAC
        strcpy (files[current_file], filename);
#endif

	if (!embedded)
	{
		/* First try to load directly (we expect fxi myfile.dcb) */
		if (!dcb_load(dcbname))
		{
			/* not successful... try for DCB */

			strcat (dcbname, ".dcb") ;

			if (!dcb_load(dcbname))
			{
				/* not successful... try for DAT */

				strcpy (dcbname, filename) ;
 				strcat (dcbname, ".dat") ;

				if (!dcb_load(dcbname))
				{
					gr_error ("%s: no existe o no es un DCB version %d o compatible", filename, DCB_VERSION >> 8) ;
					return -1 ;
				}
			}
		}
	}
	else
	{
		dcb_load_from(fp, dcb_signature.dcb_offset);
/*		file_close (fp); */
	}

	/* If the dcb is not in debug mode, switch off the profiler */

	if (dcb.data.NID == 0)
	{
		gprof_toggle();
		debug = 0;
	}

	/* Initialization (modules needed after dcb_load) */

	fnc_init();
	#ifdef MMX_FUNCTIONS
		MMX_init();
	#endif

	grlib_init () ;
	sysproc_init () ;
	gr_font_systemfont (default_font);

	if (argc > 32) argc = 32 ;
	for (i = 0 ; i < argc ; i++)
	{
		int * ptr = &GLODWORD(ARGV_TABLE+i*4) ;
		*ptr = string_new(argv[i]) ;
		string_use (*ptr) ;
	}
	GLODWORD(ARGC) = argc-1 ;
	GLODWORD(FXI_OS) = OS_ID ;
	if (mainproc)
	{
		mainproc_running = instance_new (mainproc, 0) ;
		instance_go_all () ;
	}

	//if (cd_playing()) cd_stop() ;
	//string_dump() ;
	//gprof_dump ("profile.txt");

	do_exit(0);
	return 0;
}

