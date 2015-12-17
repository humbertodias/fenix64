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



int sound_active=0;     //variable para comprobar si el sonido está activado.

/* ------------------------------------- */
/* Interfaz SDL_RWops Fenix              */
/* ------------------------------------- */

static int SDLCALL fenix_seek(SDL_RWops *context, int offset, int whence)
{
    if (file_seek(context->hidden.unknown.data1, offset, whence)<0) {
        return (-1);
    }

    return(file_pos(context->hidden.unknown.data1));
}

static int SDLCALL fenix_read(SDL_RWops *context, void *ptr, int size, int maxnum)
{
	int ret = file_read(context->hidden.unknown.data1, ptr, size*maxnum);
    if (ret>0)
        ret /= size;
	return(ret);
}

static int SDLCALL fenix_write(SDL_RWops *context, const void *ptr, int size, int num)
{
    int ret = file_write(context->hidden.unknown.data1, (void *)ptr, size*num);
    if (ret>0)
        ret /= size;
	return(ret);
}

static int SDLCALL fenix_close(SDL_RWops *context)
{
	if ( context ) {
	    file_close(context->hidden.unknown.data1);
		SDL_FreeRW(context);
	}
	return(0);
}

SDL_RWops *SDL_RWFromFenixFP(file *fp)
{
	SDL_RWops *rwops = NULL;

	rwops = SDL_AllocRW();

	if ( rwops != NULL ) {
		rwops->seek = fenix_seek;
		rwops->read = fenix_read;
		rwops->write = fenix_write;
		rwops->close = fenix_close;
		rwops->hidden.unknown.data1 = fp;
	}
	return(rwops);
}


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
	int audio_mix_channels ;

	if (!audio_initialized)
	{
		audio_initialized = 1;
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			gr_con_printf ("[SOUND] Sonido no disponible: %s", SDL_GetError()) ;
			return;
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
    		GLODWORD(SOUND_CHANNELS)<=32?Mix_AllocateChannels(GLODWORD(SOUND_CHANNELS)):Mix_AllocateChannels(32) ;
    		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    		audio_mix_channels = Mix_AllocateChannels(-1) ;
    		GLODWORD(SOUND_CHANNELS) = audio_mix_channels ;
    		gr_con_printf ("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
    			(audio_format&0xFF),
    			(audio_channels > 1) ? "stereo" : "mono",
    			audio_buffers );
    		gr_con_printf ("Allocated %i audio mixing channels\n", audio_mix_channels) ;
    		// Set mixing channels

    		sound_active=1;
/*    		ini_musiccd(); */
    		return;
    	}
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
	file      *fp;

	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return(-1);

    fp=file_open(filename, "rb0");
    if (!fp)
        return (-1);

    music = Mix_LoadMUS_RW(SDL_RWFromFenixFP(fp));

	if ( music == NULL ) {
	    file_close(fp);
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
 *  FUNCTION : is_paused_song
 *  THIS FUNCTION IS INTENDED FOR FENIX INTERNAL USAGE ONLY
 *
 *  Check if there is a paused song
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


int is_paused_song(void)
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
	Mix_Chunk *music = NULL;
	file      *fp;

	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return(-1);

    fp=file_open(filename, "rb0");
    if (!fp)
        return (-1);

	music = Mix_LoadWAV_RW(SDL_RWFromFenixFP(fp),1);

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
 *      channel (-1 any channel)
 *
 *  RETURN VALUE:
 *
 *	-1 if there is any error
 *  else channel where the music plays
 */


int play_wav (int id, int loops, int channel)
{
	int canal;

	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);

	if ((Mix_Chunk *)id!=NULL) {
		canal=Mix_PlayChannel(channel,(Mix_Chunk *)id,loops);
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
 *  Set the volume for wav playing (0-128) IN SAMPLE
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


int	 set_wav_volume	(int sample,int volume)
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);

	if (volume<0) volume=0;
	if (volume>128) volume=128;

	if ((Mix_Chunk *)sample!=NULL)
		return(Mix_VolumeChunk((Mix_Chunk *)sample,volume));
	else
		return -1 ;
}


/*
 *  FUNCTION : set_channel_volume
 *
 *  Set the volume for wav playing (0-128) IN CHANNEL
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

int	 set_channel_volume	(int canal,int volume)
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);

	if (volume<0) volume=0;
	if (volume>128) volume=128;

	return(Mix_Volume(canal,volume));
}


/*
 *  FUNCTION : reserve_channels
 *
 *  Reserve the first channels (0 -> n-1) for the application, i.e. don't allocate
 *  them dynamically to the next sample if requested with a -1 value below.
 *
 *  PARAMS:
 *  number of channels to reserve.
 *
 *  RETURN VALUE:
 *  number of reserved channels.
 *	-1 if there is any error
 *
 */

int reserve_channels (int canales)
{
	if (audio_initialized == 0)
		sound_init();
	if (sound_active==0) return (-1);

    return Mix_ReserveChannels(canales);
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
