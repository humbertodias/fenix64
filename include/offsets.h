
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
#define ARGC			4*358
#define ARGV_TABLE		4*359	/* 32+1 cadenas */
#define SPEED_GAUGE		4*392
#define FRAME_TIME		4*393
#define REGEX_REG		4*394	/* 16 cadenas */
#define ALPHA_STEPS		4*410

/* FILEINFO struct */
#define FILE_PATH		4*411
#define FILE_NAME		4*412
#define FILE_DIRECTORY	4*413
#define FILE_HIDDEN		4*414
#define FILE_READONLY	4*415
#define FILE_SIZE		4*416
#define FILE_CREATED	4*417
#define FILE_MODIFIED	4*418

#define FULL_SCREEN		4*419

/* ----------------------------------------- */
/* Offsets de los datos locales predefinidos */
/* ----------------------------------------- */

#define PROCESS_ID		4*0
#define ID_SCAN			4*1
#define PROCESS_TYPE	4*2
#define TYPE_SCAN		4*3
#define STATUS			4*4
#define CHANGED         4*5
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
#define FLAGS   		4*27
#define GRAPHSIZE    	4*28
#define ANGLE   		4*29
#define REGIONID    	4*30
#define FILEID  		4*31
#define XGRAPH  		4*32
#define HEIGHT  		4*33
#define RESOLUTION		4*34
#define ALPHA           4*35
#define GRAPHSIZEX      4*36
#define GRAPHSIZEY      4*37
#define BLENDOP			4*38
#define SAVED_X			4*49
#define SAVED_Y			4*40
#define SAVED_GRAPH		4*41
#define SAVED_ANGLE		4*42
#define SAVED_ALPHA		4*43
#define SAVED_BLENDOP	4*44
#define SAVED_SIZE		4*45
#define SAVED_SIZEX		4*46
#define SAVED_SIZEY		4*47
#define SAVED_FLAGS		4*48

/* Algunas definiciones comunes */

#define STATUS_DEAD		0
#define STATUS_KILLED	1
#define STATUS_RUNNING	2
#define STATUS_SLEEPING	3
#define STATUS_FROZEN 	4

