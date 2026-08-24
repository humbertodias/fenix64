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

/* HISTORY
 *
 *	REWRITEN ALL SOUND CODE USING SDL_MIXER
 *
 *
 *
 */


#pragma comment (lib, "SDL_mixer")




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fxi.h"
#include <SDL.h>

#ifdef BeIDE
#include <BeOS.h>
#endif

int sound_active=0;     //variable para comprobar si el sonido está activado.

static SDL_CD* cdrom ;



/*
 *  FUNCTION : sound_init
 *
 *  Set the SDL_Mixer library
 *
 *  PARAMS:
 *      no params
 *
 *  RETURN VALUE: 
 *      
 *		no return
 */


void sound_init () 
{
	int audio_rate;	
	Uint16 audio_format;
	int audio_channels;
	int audio_buffers;

	if (!audio_initialized)
	{
		audio_initialized = 1;
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			gr_con_printf ("[SOUND] Sonido no disponible: %s", SDL_GetError()) ;
			return;
		}
	}

	/* Initialize variables: but limit quality to some fixed options */

	audio_rate = GLODWORD(SOUND_FREQ);

	if (audio_rate > 22050)
		audio_rate = 44100;
	else if (audio_rate > 11025)
		audio_rate = 22050;
	else
		audio_rate = 11025;

	audio_format = AUDIO_S16;
	audio_channels = GLODWORD(SOUND_MODE)+1;
	audio_buffers = 1024*audio_rate/22050;

	/* Open the audio device */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {		
		gr_con_printf ("[SOUND] No se pudo inicializar el audio: %s\n",SDL_GetError()) ;
		sound_active=0;
		return;
	} else {
		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
		gr_con_printf ("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
			(audio_format&0xFF),
			(audio_channels > 1) ? "stereo" : "mono", 
			audio_buffers );
		sound_active=1;	 		
		ini_musiccd();		
		return;
	}
}



/*
 *  FUNCTION : sound_close
 *
 *  Close all the audio set 
 *
 *  PARAMS:
 *      no params
 *
 *  RETURN VALUE: 
 *      
 *		no return
 */


void sound_close() 
{
	if (audio_initialized == 0)
		sound_init();
    if (sound_active==0) return;
	
	SDL_CDClose(cdrom);
	
	//falta por comprobar que todo esté descargado

	Mix_CloseAudio();
	
	sound_active=0;
}


/* ------------------ */
/* Sonido MOD y OGG   */
/* ------------------ */



/*
 *  FUNCTION : load_song
 *
 *  Load a MOD/OGG from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE: 
 *      
 *		mod pointer
 *  
 */


int load_song (const char * filename)
{	
	Mix_Music *music = NULL;		

	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return(-1);

	music = Mix_LoadMUS(filename);	

	if ( music == NULL ) {
			gr_con_printf("Couldn't load %s: %s\n",filename, SDL_GetError());
			return(-1);
	} else {
		return ((int)music);
	}

}


/*
 *  FUNCTION : play_song
 *
 *  Play a MOD/OGG
 *
 *  PARAMS:
 *      mod pointer
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */



int play_song (int id, int loops)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) 
	{
		gr_error ("Sound is not active");
		return (-1);			
	}
		
	if (((Mix_Music *)id!=NULL)){
		int result = Mix_PlayMusic((Mix_Music *)id,loops);
		if (result == -1) {
			gr_error ("%s", Mix_GetError());
		}
		return result;
	} else {
		gr_error ("Play song called with invalid handle");
		return(-1);
	}

}


/*
 *  FUNCTION : fade_music_in
 *
 *  Play a MOD/OGG fading in it
 *
 *  PARAMS:
 *      mod pointer
 *      number of loops (-1 infinite loops)
 *      ms  microsends of fadding
 *
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */



int fade_music_in (int id, int loops, int ms)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
		
	if (((Mix_Music *)id!=NULL)){
		return(Mix_FadeInMusic((Mix_Music *)id,loops, ms));
	} else {
		return(-1);
	}

}


/*
 *  FUNCTION : fade_music_off
 *
 *  Stop the play of a mod 
 *
 *  PARAMS:
 *      
 *  ms  microsends of fadding
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */

int fade_music_off (int ms)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	return (Mix_FadeOutMusic(ms));
}


/*
 *  FUNCTION : unload_song
 *
 *  Play a MOD
 *
 *  PARAMS:
 *      
 *  mod id
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int unload_song (int id)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			

	if ((Mix_Music *)id!=NULL) {
		Mix_FreeMusic((Mix_Music *)id);
		return (0) ;
	} else {
		return (-1);
	}
	
}



/*
 *  FUNCTION : stop_song
 *
 *  Stop the play of a mod 
 *
 *  PARAMS:
 *      
 *  no params
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */

int stop_song (void)
{	
	if (audio_initialized == 0)
		sound_init();
    if (sound_active==0) return (-1);			
	return (Mix_HaltMusic());
}



/*
 *  FUNCTION : pause_song
 *
 *  Pause the mod in curse, you can resume it after
 *
 *  PARAMS:
 *      
 *  no params
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int pause_song (void)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	Mix_PauseMusic();
	return (0) ;			
}


/*
 *  FUNCTION : resume_song
 *
 *  Resume the mod, paused before
 *
 *  PARAMS:
 *      
 *  no params
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int resume_song (void)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	Mix_ResumeMusic();
	return(0) ;
}


/*
 *  FUNCTION : is_playing_song
 *
 *  Check if there is any mod playing
 *
 *  PARAMS:
 *      
 *  no params
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  TRUE OR FALSE if there is no error
 */


int is_playing_song(void) 
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
    return Mix_PlayingMusic();
}


/*
 *  FUNCTION : set_song_volume
 *
 *  Set the volume for mod playing (0-128)
 *
 *  PARAMS:
 *      
 *  int volume 
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  0 if there is no error
 */


int set_song_volume (int volume)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			

	if (volume<0) volume=0;
	if (volume>128) volume=128;

	Mix_VolumeMusic(volume);
	return 0;
}



/* ------------ */
/* Sonido WAV   */
/* ------------ */


/*
 *  FUNCTION : load_wav
 *
 *  Load a WAV from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE: 
 *      
 *		wav pointer
 *  
 */


int load_wav (const char * filename)
{	
	static Mix_Chunk *music = NULL;

	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return(-1);

	music = Mix_LoadWAV(filename);
	
	if ( music == NULL ) {
		gr_con_printf("Couldn't load %s: %s\n",filename, SDL_GetError());
		return(0);
	} else {
		return ((int)music);
	}

}



/*
 *  FUNCTION : play_wav
 *
 *  Play a WAV
 *
 *  PARAMS:
 *      wav pointer;
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  else channel where the music plays
 */


int play_wav (int id, int loops)
{	
	int canal;

	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
		
	if ((Mix_Chunk *)id!=NULL) {				
		canal=Mix_PlayChannel(-1,(Mix_Chunk *)id,loops);
		return(canal);
	}
	else {
		return (-1);
	}

}





/*
 *  FUNCTION : unload_wav
 *
 *  Frees the resources from a wav, unloading it
 *
 *  PARAMS:
 *      
 *  wav pointer
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int unload_wav (int id)
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			

	if ((Mix_Chunk *)id!=NULL) {
		Mix_FreeChunk((Mix_Chunk *)id);
		return (0) ;
	} else return (-1);
	

}



/*
 *  FUNCTION : stop_wav
 *
 *  Stop a wav playing
 *
 *  PARAMS:
 *      
 *  int channel
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 * 
 */

int stop_wav (int canal)
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	
	if (Mix_Playing(canal))
		return(Mix_HaltChannel(canal));
	return (-1) ;
}



/*
 *  FUNCTION : pause_wav
 *
 *  Pause a wav playing, you can resume it after
 *
 *  PARAMS:
 *      
 *  int channel
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int pause_wav (int canal)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	
	if (Mix_Playing(canal)) 
	{
		Mix_Pause(canal);
		return (0) ;
	}
	return (-1) ;
}

		


/*
 *  FUNCTION : resume_wav
 *
 *  Resume a wav playing, paused before
 *
 *  PARAMS:
 *      
 *  int channel
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int resume_wav (int canal)
{	
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	
	if (Mix_Playing(canal)) 
	{
		Mix_Resume(canal);				
		return (0) ;
	} 
	
	return (-1) ;
}


/*
 *  FUNCTION : is_playing_wav
 *
 *  Check a wav playing
 *
 *  PARAMS:
 *      
 *  int channel
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  TRUE OR FALSE if there is no error
 */


int is_playing_wav(int canal) 
{
	if (audio_initialized == 0)
		sound_init();
	
	if (sound_active==0) return (-1);			
	return(Mix_Playing(canal));
}



/*
 *  FUNCTION : set_wav_volume
 *
 *  Set the volume for wav playing (0-128)
 *
 *  PARAMS:
 *      
 *  channel id
 *  int volume 
 *      
 *  RETURN VALUE: 
 *      
 *	-1 if there is any error
 *  
 */


int	 set_wav_volume	(int canal,int volume) 
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			

	if (volume<0) volume=0;
	if (volume>128) volume=128;	    
				
	return(Mix_Volume(canal,volume));
}


/*
 *  FUNCTION : set_panning
 *
 *  Set the panning for a wav channel 
 *
 *  PARAMS:
 *
 *  channel
 *  left volume (0-255)
 *  right volume (0-255)
 *  
 *
 *	
 * 
 */


int set_panning(int canal, int left, int right) 
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	
	if (Mix_Playing(canal)) 
	{		
		Mix_SetPanning(canal,(Uint8)left,(Uint8)right);
		return (0) ;
	} 
	
	return (-1) ;
}

/*
 *  FUNCTION : set_position
 *
 *  Set the position of a channel. (angle) is an integer from 0 to 360 
 *
 *  PARAMS:
 *
 *  channel
 *  angle (0-360)
 *  distance (0-255)
 *  
 *
 *	
 * 
 */


int set_position(int canal, int angle, int dist) 
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	
	if (Mix_Playing(canal)) {		
		Mix_SetPosition(canal,(Sint16)angle,(Uint8)dist);
		return (0) ;
	} 
	
	return (-1) ;
	
}


/*
*  FUNCTION : set_distance
*
*  Set the "distance" of a channel. (distance) is an integer from 0 to 255
*  that specifies the location of the sound in relation to the listener.
*
*  PARAMS:
*
*  channel
*  
*  distance (0-255)
 *  
 *
 *	
 * 
 */


int set_distance(int canal, int dist) 
{
	if (audio_initialized == 0)
		sound_init();
    if (sound_active==0) return (-1);			

	if (Mix_Playing(canal)) {		
		Mix_SetDistance(canal,(Uint8)dist);
		return (0) ;
	} 

	return (-1) ;
}



/*
 *  FUNCTION : reverse_stereo
 *
 *  Causes a channel to reverse its stereo. 
 *
 *  PARAMS:
 *
 *  channel
 *  flip  0 normal  != reverse
 *  
 *  
 *
 *	
 * 
 */


int reverse_stereo(int canal, int flip) 
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);			
	
	if (Mix_Playing(canal)) {		
		Mix_SetReverseStereo(canal,flip);
		return (0) ;
	} 
	
	return (-1) ;
}


/* ------------ */
/* Sonido de CD */
/* ------------ */


void ini_musiccd()
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return;
		
	if (cd_playing()) cd_stop();
	cdrom = SDL_CDOpen (0) ;
	if (!cdrom) {
		gr_con_printf ("[SOUND] No se pudo inicializar el CDROM: %s\n",SDL_GetError()) ;
	}
}


void cd_play (int track, int continuous)
{
	if (audio_initialized == 0)
		sound_init();
	if (cdrom && CD_INDRIVE(SDL_CDStatus(cdrom)))
		SDL_CDPlayTracks (cdrom, track, 0, continuous ? 0:1, 0) ;
}

void cd_stop ()
{
	if (audio_initialized == 0)
		sound_init();
	if (cdrom) SDL_CDStop (cdrom) ;
}

int cd_playing ()
{
	if (audio_initialized == 0)
		sound_init();
	if (!cdrom) return 0 ;
	switch (SDL_CDStatus (cdrom))
	{
		case CD_PLAYING:
		case CD_PAUSED:
			return 1;
		default:
			return 0;
	}
}

