/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.85
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
 * FILE        : f_cd.c
 * DESCRIPTION : FXI functions for CD handling
 */

#include "fxi.h"

static SDL_CD * sdl_cd = NULL;
static int      sdl_cdnum = -1;

/**
   int CD_DRIVES()
   Returns the number of CD drives in the system
 **/

int fxi_cd_drives (INSTANCE * my, int * params)
{
	return SDL_CDNumDrives();
}

/**
   int CD_STATUS (int CD)
   Returns the status of a CD (using SDL constants)
 **/

int fxi_cd_status (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return SDL_CDStatus(sdl_cd);

}

/**
   string CD_NAME (int CD)
   Returns a human-readable string with the name of a CD drive
 **/

int fxi_cd_name (INSTANCE * my, int * params)
{
	int result;

	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	result = string_new(SDL_CDName(params[0]));
	string_use(result);
	return result;
}

/**
   CD_GETINFO(int CD)
   Fills the global structure CD with information about the current CD
   Returns 1 if there is a valid CD in the drive or 0 otherwise
 **/

int fxi_cd_getinfo (INSTANCE * my, int * params)
{
	int i, pos, total = 0;

	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	GLODWORD(CD_TRACKS) = sdl_cd->numtracks;
	GLODWORD(CD_TRACK)  = sdl_cd->cur_track;
	FRAMES_TO_MSF (sdl_cd->cur_frame, &GLODWORD(CD_MINUTE), &GLODWORD(CD_SECOND), &GLODWORD(CD_SUBFRAME));

	for (i = 0, pos = CD_TRACKINFO ; i < sdl_cd->numtracks ; i++, pos += 16)
	{
		total += sdl_cd->track[i].length;
		GLODWORD(pos) = (sdl_cd->track[i].type == SDL_AUDIO_TRACK);
		FRAMES_TO_MSF (sdl_cd->track[i].length, &GLODWORD(pos+4), &GLODWORD(pos+8), &GLODWORD(pos+12));
	}
	FRAMES_TO_MSF (total, &GLODWORD(CD_MINUTES), &GLODWORD(CD_SECONDS), &GLODWORD(CD_FRAMES));
	return 1;
}

/**
   CD_PLAY (int CD, int TRACK)
   Starts playing a track of the given CD
 **/

int fxi_cd_play (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	if (CD_INDRIVE(SDL_CDStatus(sdl_cd)))
		return !SDL_CDPlayTracks(sdl_cd, params[1], 0, 1, 0);
	return 0;
}

/**
   CD_PLAY (int CD, int TRACK, int NUMTRACKS)
   Plays a series of tracks of the CD
 **/

int fxi_cd_playtracks (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	if (CD_INDRIVE(SDL_CDStatus(sdl_cd)))
		return !SDL_CDPlayTracks(sdl_cd, params[1], 0, params[2], 0);
	return 0;
}

/**
   CD_EJECT (int CD)
   Ejects a CD
 **/

int fxi_cd_eject (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return !SDL_CDEject(sdl_cd);
}

/**
   CD_PAUSE (int CD)
   Pauses the CD playing
 **/

int fxi_cd_pause (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return !SDL_CDPause(sdl_cd);
}

/**
   CD_RESUME (int CD)
   Resumes a CD in pause
 **/

int fxi_cd_resume (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return !SDL_CDResume(sdl_cd);
}

/**
   CD_STOP (int CD)
   Stops the CD
 **/

int fxi_cd_stop (INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return !SDL_CDStop(sdl_cd);
}

/*
int fxi_cd_numtracks(INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return sdl_cd->numtracks;
}


int fxi_cd_getcurtrack(INSTANCE * my, int * params)
{
	if (params[0] < 0 || params[0] >= SDL_CDNumDrives()) return 0;

	if (sdl_cd == NULL || sdl_cdnum != params[0])
	{
		if (sdl_cd) SDL_CDClose(sdl_cd);
		sdl_cd = SDL_CDOpen(params[0]);
		if (sdl_cd == NULL) return 0;
		sdl_cdnum = params[0];
	}

	return sdl_cd->cur_track;
}
*/

