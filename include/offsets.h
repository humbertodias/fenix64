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

#ifdef FENIX_RUNTIME_OFFSETS
#include "offsets_rt.h"
#else

/* ------------------------------------------ */
/* Offsets de los datos globales predefinidos */
/* ------------------------------------------ */

#define MOUSEX          4*0
#define MOUSEY          4*1
#define MOUSEGRAPH      4*2
#define MOUSEFILE       4*3
#define MOUSEZ          4*4
#define MOUSEANGLE      4*5
#define MOUSESIZE       4*6
#define MOUSEFLAGS      4*7
#define MOUSEREGION     4*8
#define MOUSELEFT       4*9
#define MOUSEMIDDLE     4*10
#define MOUSERIGHT      4*11
#define MOUSEWHEELUP	4*12
#define	MOUSEWHEELDOWN	4*13

#define SCROLLS			4*14

			/* Siguen 10 estructuras scroll de tamaño 20 */

#define JOYLEFT			4*214
#define JOYRIGHT		4*215
#define JOYUP			4*216
#define JOYDOWN			4*217
#define JOYBUTTON1		4*218
#define JOYBUTTON2		4*219
#define JOYBUTTON3		4*220
#define JOYBUTTON4		4*221
#define SETUPCARD		4*222
#define SETUPPORT		4*223
#define	SETUPIRQ		4*224
#define SETUPDMA		4*225
#define SETUPDMA2		4*226
#define SETUPMASTER		4*227
#define SETUPSOUNDFX	4*228
#define SETUPCDAUDIO	4*229
#define TIMER 			4*230

			/* Siguen 9 temporizadores adicionales */

#define TEXTZ			4*240
#define FADING			4*241
#define SHIFTSTATUS		4*242
#define ASCII			4*243
#define SCANCODE		4*244
#define JOYFILTER		4*245
#define JOYSTATUS		4*246
#define RESTORETYPE		4*247
#define DUMPTYPE		4*248
#define MAXPROCESSTIME	4*249

#define M7STRUCTS		4*250

			/* Siguen 10 estructuras de tamaño 10 */

/* Extras */

#define FPS				4*350
#define TEXT_FLAGS		4*351
#define PANSEP			4*352
#define REVERB			4*353
#define VOLUME			4*354
#define SOUND_FREQ		4*355
#define SOUND_MODE		4*356
#define GRAPH_MODE		4*357
#define SCALE_MODE		4*358
#define ARGC			4*359
#define ARGV_TABLE		4*360	/* 32+1 cadenas */
#define SPEED_GAUGE		4*393
#define FRAME_TIME		4*394
#define REGEX_REG		4*395	/* 16 cadenas */
#define ALPHA_STEPS		4*411

/* FILEINFO struct */
#define FILE_PATH		4*412
#define FILE_NAME		4*413
#define FILE_DIRECTORY	4*414
#define FILE_HIDDEN		4*415
#define FILE_READONLY	4*416
#define FILE_SIZE		4*417
#define FILE_CREATED	4*418
#define FILE_MODIFIED	4*419

#define FULL_SCREEN		4*420

#define EXIT_STATUS		4*421
#define WINDOW_STATUS	4*422
#define FOCUS_STATUS	4*423
#define MOUSE_STATUS    4*424
#define CD_TRACK		4*425
#define CD_FRAME		4*426
#define CD_TRACKS		4*427
#define CD_MINUTE		4*428
#define CD_SECOND		4*429
#define CD_SUBFRAME		4*430
#define CD_MINUTES      4*431
#define CD_SECONDS		4*432
#define CD_FRAMES		4*433
#define CD_TRACKINFO	4*434			// 400 INTs
#define FXI_OS			4*834
#define SOUND_CHANNELS	4*835

/* ----------------------------------------- */
/* Offsets de los datos locales predefinidos */
/* ----------------------------------------- */

#define PROCESS_ID		4*0
#define ID_SCAN			4*1
#define PROCESS_TYPE	4*2
#define TYPE_SCAN		4*3
#define STATUS			4*4
#define CHANGED         4*5
#define XGRAPH_FLAGS	4*6
#define SAVED_STATUS	4*7
#define PREV_Z			4*8
#define DISTANCE_1		4*9
#define DISTANCE_2		4*10
#define FRAME_PERCENT	4*11
#define BOX_X0			4*12
#define BOX_Y0			4*13
#define BOX_X1			4*14
#define BOX_Y1			4*15
#define FATHER			4*16
#define SON				4*17
#define SMALLBRO		4*18
#define BIGBRO			4*19
#define PRIORITY		4*20
#define CTYPE			4*21
#define CFLAGS  		4*22
#define COORDX       	4*23
#define COORDY       	4*24
#define COORDZ       	4*25
#define GRAPHID 		4*26
#define PALETTEID 		4*27
#define FLAGS   		4*28
#define GRAPHSIZE    	4*29
#define ANGLE   		4*30
#define REGIONID    	4*31
#define FILEID  		4*32
#define XGRAPH  		4*33
#define HEIGHT  		4*34
#define RESOLUTION		4*35
#define ALPHA           4*36
#define GRAPHSIZEX      4*37
#define GRAPHSIZEY      4*38
#define BLENDOP			4*39
#define SAVED_X			4*40
#define SAVED_Y			4*41
#define SAVED_GRAPH		4*42
#define SAVED_PALETTE	4*43
#define SAVED_ANGLE		4*44
#define SAVED_ALPHA		4*45
#define SAVED_BLENDOP	4*46
#define SAVED_SIZE		4*47
#define SAVED_SIZEX		4*48
#define SAVED_SIZEY		4*49
#define SAVED_FLAGS		4*50
#define SAVED_FILE		4*51
#define SAVED_XGRAPH	4*52
#define SAVED_PRIORITY  4*53

#endif /* FENIX_RUNTIME_OFFSETS */
