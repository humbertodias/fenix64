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

#ifndef __SOUND_H
#define __SOUND_H

#include "SDL_mixer.h"

void            sound_init      ();
void            sound_close     ();

extern int		sound_active	 ;

extern int      load_song        (const char * filename);
extern int      play_song        (int id , int loops);
extern int      unload_song      (int id);
extern int      stop_song        ();
extern int      pause_song       ();
extern int      resume_song      ();
extern int      is_playing_song  ();
extern int      is_paused_song   ();
extern int      set_song_volume  (int volume);

extern int      load_wav			(const char * filename);
extern int      play_wav			(int id , int loops, int channel);
extern int      unload_wav			(int id);
extern int      stop_wav			(int id);
extern int      pause_wav			(int id);
extern int      resume_wav			(int id);
extern int      is_playing_wav		(int id);
extern int      set_wav_volume		(int id,int volume);
extern int      set_channel_volume  (int id,int volume);
extern int      reserve_channels    (int id);
extern int		set_panning			(int canal,int left, int right);
extern int		set_position		(int canal,int angle, int dist);
extern int		set_distance		(int canal,int dist);
extern int		reverse_stereo		(int canal,int flip);

extern int		fade_music_in	 (int id, int loops, int ms);
extern int		fade_music_off	 (int ms);

/* ------------ */
/* CD-ROM Audio */
/* ------------ */
extern void     ini_musiccd     ();
extern void     cd_play         (int track, int continuous);
extern void     cd_stop         ();
extern int      cd_playing      ();

#endif
