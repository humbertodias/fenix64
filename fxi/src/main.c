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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include "fxi.h"
#include "font.h"
#include "dcb.h"

/*
 *  GLOBAL VARIABLES
 */

int debug = 0 ;		/* EXTERN defined in fxi.h */
int fxi   = 0 ;		/* EXTERN defined in fxi.h */

extern int full_screen, double_buffer ;

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
	if (keytab_initialized) keytab_free() ;	
	if (selected_joystick!=NULL) SDL_JoystickClose(selected_joystick) ;
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

main (int argc, char **argv)
{
	char * filename = 0 ;
	char dcbname[256] ;
	INSTANCE * mainproc_running ;
	int i, j ;
	int norun = 0 ;
	const SDL_version * sdl_version ;
	char * ptr ;

    gr_con_printf ("¬15" VERSION) ;

	/* Find out if we are calling fxi.exe or whatever.exe */
 	ptr = argv[0] + strlen(argv[0]) ;
 	while (ptr > argv[0] && ptr[-1] != '\\' && ptr[-1] != '/') 
 		ptr-- ;
 	fxi = (strncmp(ptr,"fxi",3) == 0) || (strncmp(ptr,"FXI",3) == 0) ;
 
	/* Init RAND generator */
	srand (time(NULL)) ;

	/* Init SDL info */
	if ( SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_CDROM) < 0 ) {
		printf ("SDL Init Error: %s\n", SDL_GetError()) ;
		do_exit(1) ;
	}

	if (fxi) {
		/* we are calling FXI.EXE */
		sdl_version = SDL_Linked_Version();
		gr_con_printf ("¬14SDL: %d.%d.%d (DLL loaded: %d.%d.%d)", 
				SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL,
				sdl_version->major, sdl_version->minor, sdl_version->patch) ;
	}

	SDL_JoystickEventState (SDL_ENABLE) ;

 	if (!fxi) {
		/* calling WHATEVER.EXE must guess DCB/DAT filename */
 		filename = ptr ;
 		while (*ptr != '.') ptr++ ;
 		*ptr = 0 ;
	} else {
		/* Calling FXI.EXE so we must get all command line params */
		for (i = 1 ; i < argc ; i++) {
			if (argv[i][0] == '-') {
				j = 1 ;
				while (argv[i][j]) {
#ifdef DEBUG
					if (argv[i][j] == 'd') debug = 1 ;
#endif
					if (argv[i][j] == 'r') norun = 1 ;
					//if (argv[i][j] == 'b') double_buffer = 1 ;
					if (argv[i][j] == 'f') enable_filtering = 1 ;
					if (argv[i][j] == 'i') {
						if (argv[i][j+1] == 0) {
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
			} else {
				if (!filename) {
					filename = argv[i] ;
					if (i < argc-1)
						memmove (&argv[i], &argv[i+1],
							sizeof(char*) * (argc-i-1)) ;
					argc-- ;
					i-- ;
				}
			}
		}

		if (!filename) {
			gr_error (VERSION "\nCopyright(C) 2002 Fenix Team\nCopyright (C)1999 Jose Luis Cebrian\n"
				"Fenix comes with ABSOLUTELY NO WARRANTY; see COPYING for details\n\n"
				"Usage: %s [options] file.dcb\n\n"
#ifdef DEBUG
				"   -d       Activate DEBUG mode\n"
#endif
//				"   -w       Execute in WINDOWED mode\n"
				"   -f       16bpp Filter ON (only 16bpp color mode)\n"
//				"   -b       Double buffer ON\n\n"
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

	if (!file_exists(dcbname))
	{
		/* Hack: con FXI renombrado, se permite que el fichero
		 * de datos tenga una extensión .DAT en vez de .DCB */

 		if (!fxi)
 		{
 			strcat (dcbname, ".dat") ;
 			if (!file_exists(dcbname))
 			{
 				strcpy (dcbname, filename) ;
 				strcat (dcbname, ".dcb") ;
 			}
 		}
 		else
 		{
 			strcat (dcbname, ".dcb") ;
 		}
	}

	if (!dcb_load (dcbname))
		return -1 ;

	/* If the dcb is not in debug mode, switch off the profiler */

	if (dcb.NID == 0)
		gprof_toggle();

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

