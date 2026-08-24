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
 * HISTORY: 0.81 - ALT+TAB patch at gr_unlock_screen
 *			0.81 - Added FULL_SCREEN variable
 *			0.76 - Patched DRAW_INSTANCE so XGRAPH does not rotate graphic
 *			0.76 - Patched DRAW_INSTANCE_AT so XGRAPH does not rotate graphic
 *			0.75 - Added icon WM capabilities on 8/16 bpp
 *			0.75 - New Keyboard system about 60/70% done
 *			0.74 - Added some more SDLK symbols to list
 *			0.74 - Added title WM capabilities... working in icon
 *			0.72 - Added SDLK_8 to keylistg
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#if defined(TARGET_BEOS) || defined(TARGET_BeOS)
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <SDL.h>
#include <SDL_keysym.h>

#ifdef TARGET_BeOS
#include "Be_compat/be_compat.h"
#endif

#include "fxi.h"
#include "dcb.h"

SDL_Surface * screen ;

GRAPH * background ;
GRAPH * scrbitmap = 0 ;
GRAPH * scrbitmap_extra = 0 ;
GRAPH * background_8bits ;
GRAPH * icono = 0 ;

int     background_8bits_used ;
int     enable_16bits = 0 ;
int		enable_filtering = 0 ;
int		enable_2xscale = 0 ;
int		background_is_black = 1 ;

char *	apptitle ;

int		scr_width = 320 ;
int		scr_height = 200 ;

int		scr_initialized = 0 ;
int     audio_initialized = 0 ;

int		full_screen = 1 ;
int		double_buffer = 0 ;
int     hardware_scr = 0 ;

static int scrbitmap_is_fake = 0 ;
static int show_console = 0 ;
static int show_profile = 0 ;

int report_graphics = 0 ;

static REGION last_mouse_bbox = { -2, -2, -2, -2 };

#define MAX_JOYSTICKS 8
int joy_x[8] = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
int joy_y[8] = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
int joy_b[8] = { 0, 0, 0, 0, 0, 0, 0, 0 } ;

static int sdl_equiv[SDLK_LAST+1] ; 
key_equiv key_table[127] ;  /* Now we have a search table with equivs */ 
unsigned char * keystate ;		  /* Pointer to key states */
int keystate_size = 0 ;

static int equivs[] =
{
	SDLK_ESCAPE,		1,
	SDLK_1,				2,
	SDLK_KP1,			2,
	SDLK_2,				3,
	SDLK_KP2,			3,
	SDLK_3,				4,
	SDLK_KP3,			4,
	SDLK_4,				5,
	SDLK_KP4,			5,
	SDLK_5,				6,
	SDLK_KP5,			6,
	SDLK_6,				7,
	SDLK_KP6,			7,
	SDLK_7,				8,
	SDLK_KP7,			8,
	SDLK_8,				9,
	SDLK_KP8,			9,
	SDLK_9,				10,
	SDLK_KP9,			10,
	SDLK_0,				11,
	SDLK_KP0,			11,
	SDLK_MINUS,			12,
	SDLK_EQUALS,		13,
	SDLK_BACKSPACE,		14,
	SDLK_TAB,			15,
	SDLK_q,				16,
	SDLK_w,				17,
	SDLK_e,				18,
	SDLK_r,				19,
	SDLK_t,				20,
	SDLK_y,				21,
	SDLK_u,				22,
	SDLK_i,				23,
	SDLK_o,				24,
	SDLK_p,				25,
	SDLK_LEFTBRACKET,	26,
	SDLK_RIGHTBRACKET,	27,
	SDLK_RETURN,		28,
	SDLK_KP_ENTER,		28,
	SDLK_LCTRL,			96,
	SDLK_RCTRL,			94,
	SDLK_LCTRL,			29,
	SDLK_RCTRL,			29,
	SDLK_a,				30,
	SDLK_s,				31,
	SDLK_d,				32,
	SDLK_f,				33,
	SDLK_g,				34,
	SDLK_h,				35,
	SDLK_j,				36,
	SDLK_k,				37,
	SDLK_l,				38,
	SDLK_SEMICOLON,		39,
	SDLK_QUOTE,			40,	
	SDLK_BACKQUOTE,     41,
	SDLK_LSHIFT,		42,	
	SDLK_BACKSLASH,		43,
	SDLK_z,				44,
	SDLK_x,				45,
	SDLK_c,				46,
	SDLK_v,				47,
	SDLK_b,				48,
	SDLK_n,				49,
	SDLK_m,				50,
	SDLK_COMMA,			51,
	SDLK_PERIOD,		52,
	SDLK_KP_PERIOD,		52,
	SDLK_SLASH,			53,
	SDLK_KP_DIVIDE,		53,
	SDLK_RSHIFT,		54,
/*	SDLK_PRINT,			55,*/
	SDLK_KP_MULTIPLY,	55,
	SDLK_LALT,			95,
	SDLK_RALT,			93,
	SDLK_LALT,			56,
	SDLK_RALT,			56,
	SDLK_RMETA,			56,
	SDLK_LMETA,			56,
	SDLK_SPACE,			57,
	SDLK_CAPSLOCK,		58,
	SDLK_F1,			59,
	SDLK_F2,			60,
	SDLK_F3,			61,
	SDLK_F4,			62,
	SDLK_F5,			63,
	SDLK_F6,			64,
	SDLK_F7,			65,
	SDLK_F8,			66,
	SDLK_F9,			67,
	SDLK_F10,			68,
	SDLK_NUMLOCK,		69,
	SDLK_SCROLLOCK,		70,
	SDLK_HOME,			71,
	SDLK_UP,			72,
	SDLK_PAGEUP,		73,
	SDLK_KP_MINUS,	    74,
	SDLK_LEFT,			75,
	SDLK_RIGHT,			77,
	SDLK_KP_PLUS,		78,
	SDLK_END,			79,
	SDLK_DOWN,			80,
	SDLK_PAGEDOWN,		81,
	SDLK_INSERT,		82,
	SDLK_DELETE,		83,
	SDLK_F11,			87,
	SDLK_F12,			88,
	SDLK_LESS,			89,
	SDLK_PLUS,			90,
	SDLK_GREATER,		91,
	SDLK_QUESTION,		92,
	SDLK_CARET,			93,
	SDLK_SYSREQ,        55,
	SDLK_PAUSE,         95,
	SDLK_MENU,          97,
	-1, -1
} ;

int keytab_initialized = 0 ;

int last_frame_ms = 0 ;
int frame_count = 0 ;

static int last_frame_ticks = 0 ;
static int next_frame_ticks = 0 ;
static int frame_ms = 40 ;
static int max_jump = 2 ;
static int current_jump = 0 ;
static int jump = 0 ;
static int timer_advance = 0 ;
static int FPS_count = 0 ;
static int FPS_curr  = 0 ;
static int FPS_init  = 0 ;

typedef struct _object
{
	int z ;
	int id ;
	int  (*info)(void * what, REGION * clip) ;
	void (*draw)(void * what, REGION * clip) ;
	void * what ;
	REGION bbox ;
	int changed ;
}
OBJECT ;

typedef struct _dll_object
{
	int id ;
	int hidden ;
	OBJECT x ;
	struct _dll_object * next ;
}
DLL_OBJECT ;

static DLL_OBJECT * first_dll_object = NULL;
static int          last_dll_object_id = 0;
static int          dll_object_count = 0;

static OBJECT * object_list = 0;
static int      object_list_allocated = 0;
static int      object_count = 0;
static int      object_id = 0;

int             object_list_dirty = 1;
int             object_list_unsorted = 0;

int				background_dirty = 0;

static REGION   updaterects[128];
static int      updaterects_count = 0;
static Uint8    zonearray[128/8];

extern int default_palette[];

/* ---------------------------------------------------------------------- */
/* Gestión de regiones                                                    */
/* ---------------------------------------------------------------------- */

REGION    regions[32] ;

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/*
 *  FUNCTION : region_define
 *
 *  Sets one of the 32 regions visible from the fenix
 *  language to the values given
 *
 *  The region 0 is always equal to the whole screen
 *  and cannot be changed
 *
 *  PARAMS : 
 *      n			Number of region to set (1 to 31)
 *		x, y		Top-Left coordinates
 *		width		Width in pixels
 *		height		Height in pixels
 *
 *  RETURN VALUE : 
 *      None
 */

void region_define (int region, int x, int y, int width, int height)
{
	if (region < 1 || region > 31)
		return ;

	regions[region].x = MAX (x, 0) ;
	regions[region].y = MAX (y, 0) ;
	regions[region].x2 = MIN (scr_width,  x+width)  - 1 ;
	regions[region].y2 = MIN (scr_height, y+height) - 1 ;
}

/*
 *  FUNCTION : region_union
 *
 *  Calculates a region as the intersection of another two
 *
 *  PARAMS : 
 *      dest		First region, and the one to contain the result
 *		b			Second region
 *
 *  RETURN VALUE : 
 *      None. The result will be stored in the first region
 *		used as parameter
 *
 */

void region_union (REGION * dest, REGION * b)
{
	dest->x  = MAX(dest->x,  b->x) ;
	dest->y  = MAX(dest->y,  b->y) ;
	dest->y2 = MIN(dest->y2, b->y2) ;
	dest->x2 = MIN(dest->x2, b->x2) ;
}

/*
 *  FUNCTION : region_is_empty
 *
 *  Returns TRUE if the region contains no pixels
 *
 *  PARAMS : 
 *      region		Region to check
 *
 *  RETURN VALUE : 
 *      1 if the region is empty, 0 otherwise
 *
 */

int region_is_empty (REGION * a)
{
	return (a->x2 < a->x) || (a->y2 < a->y) ;
}

/*
 *  FUNCTION : region_is_out
 *
 *  Returns TRUE if two regions overlap
 *
 *  PARAMS : 
 *      a			First region to check
 *		b			Second region to check
 *
 *  RETURN VALUE : 
 *      1 if there is at least one pixel in both regions, 0 otherwise
 *
 */

int region_is_out (REGION * a, REGION * b)
{
	return (b->x > a->x2 || b->y > a->y2 || b->x2 < a->x || b->y2 < a->y);
}

/*
 *  FUNCTION : region_new
 *
 *  Create a new region object. Only rectangular regions
 *  are supported in this library.
 *
 *  PARAMS : 
 *      x, y		Coordinates of the top-left pixel
 *		width		Width of the region in pixels
 *		height		Height of the region in pixels
 *
 *  RETURN VALUE : 
 *      Returns a pointer to the new object
 *
 */

REGION * region_new (int x, int y, int width, int height)
{
	REGION * region = malloc(sizeof(REGION)) ;

	region->x = MAX (x, 0) ;
	region->y = MAX (y, 0) ;
	region->x2 = MIN (scr_width,  x+width)  - 1 ;
	region->y2 = MIN (scr_height, y+height) - 1 ;
	return region ;
}

/*
 *  FUNCTION : region_get
 *
 *  Returns one of the 32 default regions visible from the fenix language
 *
 *  PARAMS : 
 *      n			Number of the region (0-31)
 *
 *  RETURN VALUE : 
 *      Returns the region object
 *
 */

REGION * region_get (int n)
{
	if (n < 0 || n > 31)
		return 0 ;

	return &regions[n] ;
}

/*
 *  FUNCTION : region_destroy
 *
 *  Free the memory allocated by a region
 *
 *  PARAMS : 
 *      region		Pointer to the region object
 *
 *  RETURN VALUE : 
 *		None
 *
 */

void region_destroy (REGION * region)
{
	free (region);
}

/* ---------------------------------------------------------------------- */
/* Gestión de eventos (ratón, teclado)                                    */
/* ---------------------------------------------------------------------- */

void add_key_equiv(int equiv, int keyconst) 
{
	key_equiv * curr = &key_table[keyconst] ;	
	if (curr->next != NULL) 
	{
		while (curr->next!=NULL) 
			curr=curr->next ;
	}
	if (curr->sdlk_equiv!=0) {
		curr->next = malloc(sizeof(key_equiv)) ;
		curr = curr->next ;
		curr->next = NULL ;
		}
	curr->sdlk_equiv = equiv ;
}

/* FREE used key_equivs... note base 127 equivs are static not allocated... */
void keytab_free() 
{
	int i ;
	key_equiv * aux ;
	key_equiv * curr = key_table ;

	for (i=0;i<127;i++) {
		if (curr->next!=NULL) {
			curr = curr->next ;
			while (curr->next!=NULL) {
				aux = curr ;
				curr = curr->next;
				free(aux) ;
			}
			free(curr) ;
		}
	}
}

void keytab_init()
{
	int * ptr = equivs ;

	memset (sdl_equiv, 0, sizeof(sdl_equiv));
	memset (key_table,  0, sizeof(key_table)) ;

	while (*ptr != -1)
	{
		sdl_equiv[*ptr] = ptr[1] ;
		add_key_equiv(ptr[0],ptr[1]) ;
		ptr += 2 ;
	}
	if (keystate == NULL)
	{
		SDL_GetKeyState (&keystate_size) ;
		keystate = malloc(keystate_size) ;
	}
	memcpy (keystate, SDL_GetKeyState(NULL), keystate_size) ;

	SDL_EnableUNICODE(1);
	keytab_initialized = 1 ;
}

/*
 *  FUNCTION : gr_key
 *
 *  Returns the current status of a key (pressed or not)
 *
 *  PARAMS : 
 *		key				Fenix code of the key (see fxdll.h for #defines)
 *
 *  RETURN VALUE : 
 *      A non-zero positive value if the key is pressed, 0 otherwise
 */

int gr_key (int code)
{
	key_equiv * curr ;
	int found = 0 ;

	if (!keystate) return 0;

	curr = &key_table[code] ;
	if (keytab_initialized==0) keytab_init() ;
	while (curr!=NULL && found==0) 
	{
		found = keystate[curr->sdlk_equiv] ;
		curr = curr->next ;
	}
	return found ;
}

/*
 *  FUNCTION : do_events
 *
 *  Process all pending SDL events, updating all global variables
 *  and handling debug key presses
 *  and cannot be changed
 *
 *  PARAMS : 
 *      None
 *
 *  RETURN VALUE : 
 *      None
 */

static void do_events ()
{
	SDL_Event e ;
	SDLMod m ;
	int k,asc ;
	int pressed ;
	key_equiv * curr ;

	int keypress ;
	static struct
	{
		int ascii ;
		int scancode ;
	}
	keyring [64] ;
	static int keyring_start = 0, keyring_tail = 0 ;

	static int last_mouse_x = -1, last_mouse_y = -1 ;

	if (!keytab_initialized) keytab_init() ;
	
	/* Actualizar eventos */
	
	keypress = 0 ;
	m = SDL_GetModState() ;

	/* El cambio de mouse.x/y afecta directamente al ratón */

	if (last_mouse_x != -1 && 
		(GLODWORD(MOUSEX) != last_mouse_x ||
		 GLODWORD(MOUSEY) != last_mouse_y))
	{
		SDL_WarpMouse ((Uint16)GLODWORD(MOUSEX), (Uint16)GLODWORD(MOUSEY)) ;
	}

	/* Procesa los eventos pendientes */
	/* Reset ascii and scancode if last key was released... */	
	/* must check all the linked equivs */

	if (show_console == 0)
		memcpy (keystate, SDL_GetKeyState(NULL), keystate_size) ;

	GLODWORD(MOUSEWHEELUP)   = 0 ;
	GLODWORD(MOUSEWHEELDOWN) = 0 ;

	pressed = 0 ;
	if (GLODWORD(SCANCODE)!=0) {
		curr = &key_table[GLODWORD(SCANCODE)] ;
		while (curr!=NULL && pressed == 0) {
			if (keystate[curr->sdlk_equiv]) pressed = 1 ;
			curr=curr->next ;
		}
	}
	if (pressed==0) {
		GLODWORD(ASCII) = 0 ;
		GLODWORD(SCANCODE) = 0 ;
	}
	while (SDL_PollEvent (&e)) 
	{
		switch (e.type)
		{
			case SDL_MOUSEMOTION:
				GLODWORD(MOUSEX) = e.motion.x ;
				GLODWORD(MOUSEY) = e.motion.y ;
				break ;
/*
			case SDL_JOYAXISMOTION:				
				if (e.jaxis.which < MAX_JOYSTICKS && 
					e.jaxis.axis == 0)
				{
					joy_x[e.jaxis.which] = e.motion.x ;
					joy_y[e.jaxis.which] = e.motion.y ;
				}
				break ;
*/
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == 1) GLODWORD(MOUSELEFT)     = 1 ;
				if (e.button.button == 2) GLODWORD(MOUSEMIDDLE)   = 1 ;
				if (e.button.button == 3) GLODWORD(MOUSERIGHT)    = 1 ;
				if (e.button.button == 4) GLODWORD(MOUSEWHEELUP)++ ;
				if (e.button.button == 5) GLODWORD(MOUSEWHEELDOWN)++ ;
				break ;
/*
			case SDL_JOYBUTTONDOWN:
				if (e.jbutton.which < MAX_JOYSTICKS)
				{
					joy_b[e.jbutton.which] |= 
						(1 << e.jbutton.button) ;
				}
				break ;
*/
			case SDL_MOUSEBUTTONUP:
				if (e.button.button == 1) GLODWORD(MOUSELEFT)      = 0 ;
				if (e.button.button == 2) GLODWORD(MOUSEMIDDLE)    = 0 ;
				if (e.button.button == 3) GLODWORD(MOUSERIGHT)     = 0 ;
				break ;
/*
			case SDL_JOYBUTTONUP:
				if (e.jbutton.which < MAX_JOYSTICKS)
				{
					joy_b[e.jbutton.which] &= 
						~(1 << e.jbutton.button) ;
				}
				break ;
*/
			case SDL_KEYDOWN:

				/* Teclas del sistema ALT+... (sólo modo debug) */

				if (dcb.NID != 0 && (e.key.keysym.mod & KMOD_LALT))
				{
					if (!full_screen && e.key.keysym.sym == SDLK_f)
					{
						GLODWORD(GRAPH_MODE) ^= 0x200;
						gr_init (scr_width, scr_height);
						break;
					}
					if (full_screen && e.key.keysym.sym == SDLK_w)
					{
						GLODWORD(GRAPH_MODE) ^= 0x200;
						gr_init (scr_width, scr_height);
						break;
					}
					if (e.key.keysym.sym == SDLK_z)
					{
						GLODWORD(GRAPH_MODE) ^= 2;
						gr_init (scr_width, scr_height);
						break;
					}
					if (e.key.keysym.sym == SDLK_x)
					{
						must_exit = 1 ;
						break;
					}
					if (e.key.keysym.sym == SDLK_c)
					{
						show_console = !show_console ;
						if (show_console)
							show_profile = 0;
						background_dirty = 1;
						break ;
					}
					if (e.key.keysym.sym == SDLK_r)
					{
						gprof_reset();
						break ;
					}
					if (e.key.keysym.sym == SDLK_p)
					{
						if (!show_console)
							show_profile = !show_profile;
						background_dirty = 1;
						break ;
					}
					if (e.key.keysym.sym == SDLK_s)
					{
						gprof_toggle();
						break ;
					}
					if (e.key.keysym.sym == SDLK_g)
					{
						int shot_num = 0;
						GRAPH * shot = bitmap_new (0, scr_width, scr_height, enable_16bits ? 16:8);

						if (!shot) break;
						gr_draw_screen (shot, 1, 1);
						for (shot_num = 0 ; shot_num < 9999 ; shot_num++)
						{
							char name[32] ;
							_snprintf (name, 32, "shot%04d.png", shot_num);
							if (!file_exists(name))
							{
								shot->flags |= GI_NOCOLORKEY;
								gr_save_png (shot, name);
								gr_con_printf ("[FXI] Screenshot %s grabado", name);
								break;
							}
						}
						bitmap_destroy (shot);
						break ;
					}
				}	

				/* Consola */

				if (show_console)
				{
					if (e.key.keysym.sym == SDLK_PAGEUP)
					{
						gr_con_scroll (-1) ;
						break ;
					}
					if (e.key.keysym.sym == SDLK_PAGEDOWN)
					{
						gr_con_scroll (1) ;
						break ;
					}
					if (!(m & KMOD_LALT)) 
					{
						gr_con_getkey (e.key.keysym.unicode,
							e.key.keysym.sym) ;
						break ;
					}
				}

				/* Almacena la pulsación de la tecla */

				k = sdl_equiv[e.key.keysym.sym];
				if (k == 0) 
				{
					gr_con_printf ("Warning: symbol %d not defined", 
						e.key.keysym.sym);
				}
				m = e.key.keysym.mod ;

                if (!keypress) 
				{
					GLODWORD(SCANCODE) = k ;
					if (e.key.keysym.unicode) 
					{
						asc = win_to_dos[e.key.keysym.unicode & 0xFF] ;
					    
						/* ascii mayusculas */
					    if (asc >= 0X61 && asc <= 0X7A && (m & KMOD_LSHIFT || m & KMOD_RSHIFT || keystate[SDLK_CAPSLOCK]))
							asc -= 0x20 ;
					} 
					else asc = 0 ;
					
					GLODWORD(ASCII) = asc ;
					keypress = 1 ;
				} 
				else 
				{
					keyring[keyring_tail].scancode = k ;
					if (e.key.keysym.unicode) {
						asc = win_to_dos[e.key.keysym.unicode & 0x7F] ;
					    /*ascii mayusculas */
					    if (asc >= 0X61 && GLODWORD(ASCII) <= 0X7A && (m & KMOD_LSHIFT || m & KMOD_RSHIFT || keystate[SDLK_CAPSLOCK]))
							asc -= 0x20 ;
					} else {
						asc = 0 ; /* NON PRINTABLE */
					}
					keyring[keyring_tail].ascii = asc ;
					if (++keyring_tail == 64) keyring_tail = 0 ;
				}

				break ;

			case SDL_KEYUP:
				/* Do nothing, fenix is key_up unsensitive */
				break ;

			case SDL_QUIT:
				if (dcb.NID > 0)
					do_exit (-1);
				break ;
		}
	}

	if (!keypress && keyring_start != keyring_tail)
	{
		GLODWORD(ASCII)    = keyring[keyring_start].ascii ;
		GLODWORD(SCANCODE) = keyring[keyring_start].scancode ;
		if (++keyring_start == 64) keyring_start = 0 ;
	}

	/* Now actualized every frame... */
	GLODWORD(SHIFTSTATUS) = 
		((m & KMOD_LSHIFT)                    ? 1 : 0) +
		((m & KMOD_RSHIFT)                    ? 2 : 0) +
		((m & KMOD_RCTRL) || (m & KMOD_LCTRL) ? 4 : 0) +
		((m & KMOD_LALT)  || (m & KMOD_RALT)  ? 8 : 0)  ;

	last_mouse_x = GLODWORD(MOUSEX) ;
	last_mouse_y = GLODWORD(MOUSEY) ;
}


/* ---------------------------------------------------------------------- */
/* Inicialización y controles de tiempo                                   */
/* ---------------------------------------------------------------------- */

/*
 *  FUNCTION : gr_set_fps
 *
 *  Change the game fps and frameskip values
 *
 *  PARAMS : 
 *      fps			New number of frames per second
 *		jump		New value of maximum frameskip
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_set_fps(int fps, int jump)
{
	frame_ms = fps ? 1000 / fps : 1 ;
	max_jump = jump ;

	next_frame_ticks = SDL_GetTicks() + frame_ms ;
}

/*
 *  FUNCTION : gr_advance_timers
 *
 *  Update the value of all global timers
 *
 *  PARAMS : 
 *      None
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_advance_timers()
{
	int * timer, i ;

	timer = &GLODWORD(TIMER) ;
	for (i = 0 ; i < 10 ; i++)
		timer[i] += timer_advance/10 ;
}

/*
 *  FUNCTION : gr_wait_frame
 *
 *  Wait for the next frame start. It also calls
 *  string_coalesce() from some garbage collection
 *
 *  PARAMS : 
 *      None
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_wait_frame()
{
	int frame_ticks, delay ;

	if (!scr_initialized) gr_init(320,200) ;

	frame_ticks = SDL_GetTicks() ;
	timer_advance %= 10 ;
	timer_advance += frame_ticks - last_frame_ticks ;

	last_frame_ms = frame_ticks - last_frame_ticks ;
	GLODWORD(SPEED_GAUGE) = (frame_ticks-last_frame_ticks)*100/frame_ms ;
	*(float *)&GLODWORD(FRAME_TIME) = (frame_ticks - last_frame_ticks)/1000.0f;
	last_frame_ticks = frame_ticks ;

	if (!jump) 
		FPS_count++ ;
	if (timer_advance > 9)
		FPS_curr += (int)100.0/(timer_advance/10) ;
	if (FPS_count && FPS_init < frame_ticks-1000)
	{
		GLODWORD(FPS) = FPS_count ;
		FPS_count = 0 ;
		FPS_curr = 0 ;
		FPS_init = frame_ticks ;
	}

	jump = 0 ;
	frame_count ++ ;

	if (frame_ticks > next_frame_ticks)
	{
		if (frame_ticks > next_frame_ticks + frame_ms)
			next_frame_ticks = frame_ticks + frame_ms ;
		else
			next_frame_ticks += frame_ms ;

		if (current_jump < max_jump)
		{
			current_jump++ ;
			last_frame_ticks = frame_ticks ;
			jump = 1 ;
		}
		else
		{
			//next_frame_ticks = frame_ticks + frame_ms ;
			current_jump = 0 ;
		}
	}
	else
	{
		delay = next_frame_ticks - frame_ticks  ;
		GLODWORD(SPEED_GAUGE) = 100 - delay*100/frame_ms ;
		next_frame_ticks += frame_ms ;

		if (delay) SDL_Delay (delay) ;

		current_jump = 0 ;
	}

	string_coalesce() ;	/* Libera cadenas temporales */
}

/*
 *  FUNCTION : gr_timer
 *
 *  Returns a milliseconds integer counter timer. This value
 *  updates only once for frame and is constant.
 *
 *  PARAMS : 
 *      None
 *
 *  RETURN VALUE : 
 *      Integer value in milliseconds
 */

int gr_timer()
{
	return last_frame_ticks;
}

/* Rutinas gráficas de alto nivel */

void draw_instance_at (INSTANCE * proc_ptr, REGION * region, int x, int y)
{
	INSTANCE * i = (INSTANCE *) proc_ptr ;
	GRAPH * map ;
	Sint16 * blend_table;
	int flags ;
	int scalex, scaley ;

	map = instance_graph (i) ;
	if (!map) return ;
	if (map->flags & F_ANIMATION) bitmap_animate(map) ;

	flags = LOCDWORD(i, FLAGS);
	if (LOCDWORD(i, ALPHA) != 255)
	{
		if (LOCDWORD(i, ALPHA) <= 0)
		    return;
		else if (LOCDWORD(i, ALPHA) < 255)
			flags |= B_ALPHA | ( (LOCDWORD(i,ALPHA)) << B_ALPHA_SHIFT );
	}

	scalex = LOCDWORD(i,GRAPHSIZEX);
	scaley = LOCDWORD(i,GRAPHSIZEY);
	if (scalex == 100 && scaley == 100) 
		scalex = scaley = LOCDWORD(i,GRAPHSIZE);

	if (LOCDWORD(i,BLENDOP) != 0)
	{
		blend_table = map->blend_table;
		map->blend_table = (Sint16 *) vm_ptr (i, LOCDWORD(i,BLENDOP));
	}

	// PATCH - XGRAPH DOES NOT ROTATE DESTINATION GRAPHIC
	if (LOCDWORD(i,ANGLE) || scalex != 100 || scaley != 100) {
		if (LOCDWORD(i,XGRAPH) && scalex == 100 && scaley == 100) {
			gr_blit (0, region, x, y, flags, map) ;
		} else {
			if (LOCDWORD(i,XGRAPH)) {
				gr_rotated_blit (0, region, x, y, flags, 0, scalex, scaley, map) ;
			} else {
				gr_rotated_blit (0, region, x, y, flags, LOCDWORD(i,ANGLE), scalex, scaley, map) ;
			}
		}
	} else {
		gr_blit (0, region, x, y, flags, map) ;
	}
	
	if (LOCDWORD(i,BLENDOP) != 0)
		map->blend_table = blend_table;
}

void draw_instance (INSTANCE * proc_ptr, REGION * clip)
{
	INSTANCE * i = (INSTANCE *) proc_ptr ;
	GRAPH * map ;
	Sint16 * blend_table;
	int x, y, r ;
	int flags;
	int scalex, scaley;
	REGION fclip;

	map = instance_graph (i) ;
	if (!map) return ;
	if (map->flags & F_ANIMATION) bitmap_animate(map) ;

	r = LOCDWORD(i,REGIONID);
	if (r < 1 || r > 31) r = 0 ;

	x = LOCDWORD(i, COORDX) ;
	y = LOCDWORD(i, COORDY) ;
	if (LOCDWORD(i, RESOLUTION))
	{
		x /= LOCDWORD(i, RESOLUTION) ;
		y /= LOCDWORD(i, RESOLUTION) ;
	}

	flags = LOCDWORD(i, FLAGS);
	if (LOCDWORD(i, ALPHA) != 255)
	{
		if (LOCDWORD(i, ALPHA) <= 0)
		    return;
		else if (LOCDWORD(i, ALPHA) < 255)
			flags |= B_ALPHA | ( (LOCDWORD(i,ALPHA)) << B_ALPHA_SHIFT );
	}

	scalex = LOCDWORD(i,GRAPHSIZEX);
	scaley = LOCDWORD(i,GRAPHSIZEY);
	if (scalex == 100 && scaley == 100) 
		scalex = scaley = LOCDWORD(i,GRAPHSIZE);

	if (LOCDWORD(i,BLENDOP) != 0)
	{
		blend_table = map->blend_table;
		map->blend_table = (Sint16 *) vm_ptr (i, LOCDWORD(i,BLENDOP));
	}

	fclip = regions[r];
	if (clip) region_union (&fclip, clip);

	if (LOCDWORD(i,ANGLE) || scalex != 100 || scaley != 100) {		
		if (LOCDWORD(i,XGRAPH) && scalex == 100 && scaley == 100) {
			gr_blit (0, &regions[r], x, y, flags, map) ;
		} else {
			if (LOCDWORD(i,XGRAPH)) {
				gr_rotated_blit (0, &fclip, x, y, flags, 0, scalex, scaley, map) ;
			} else {
				gr_rotated_blit (0, &fclip, x, y, flags, LOCDWORD(i,ANGLE), scalex, scaley, map) ;
			}
		}
	} else {
		gr_blit (0, &fclip, x, y, flags, map) ;
	}

	if (LOCDWORD(i,BLENDOP) != 0)
		map->blend_table = blend_table;
}

/*
 *  FUNCTION : info_mouse
 *
 *  Returns information about the mouse
 *
 *  PARAMS : 
 *      ptr				Void pointer, used for compatibility with DLL-type objects
 *		clip			Region to fill with bounding box
 *
 *  RETURN VALUE : 
 *      1 if the mouse has changed since last call
 */

int info_mouse (void * ptr, REGION * clip)
{
	GRAPH * map ;

	map = bitmap_get (GLODWORD(MOUSEFILE), GLODWORD(MOUSEGRAPH)) ;

	if (!map) 
	{
		clip->x = clip->x2 = -1;
		clip->y = clip->y2 = -1;
		return 0;
	}

	gr_get_bbox (&last_mouse_bbox, 0, GLODWORD(MOUSEX),
				 GLODWORD(MOUSEY), GLODWORD(MOUSEFLAGS),
				 GLODWORD(MOUSEANGLE), GLODWORD(MOUSESIZE), 
				 GLODWORD(MOUSESIZE), map) ;
	*clip = last_mouse_bbox;
	return 1;
}

/*
 *  FUNCTION : draw_mouse
 *
 *  Draws the mouse graphic at screen
 *
 *  PARAMS : 
 *      ptr				Void pointer, used for compatibility with DLL-type objects
 *		clip			Clipping region
 *
 *  RETURN VALUE : 
 *      None
 */

void draw_mouse (void * ptr, REGION * clip)
{
	GRAPH * map ;
	int r ;
	REGION region;

	map = bitmap_get (GLODWORD(MOUSEFILE), GLODWORD(MOUSEGRAPH)) ;

	if (!map) return ;
	if (map->flags & F_ANIMATION) bitmap_animate(map) ;

	r = GLODWORD(MOUSEREGION) ;
	if (r < 0 || r > 31) r = 0 ;

	region = regions[r];
	if (clip) region_union (&region, clip);

	if (GLODWORD(MOUSEANGLE) || GLODWORD(MOUSESIZE) != 100)
		gr_rotated_blit (0, &region, GLODWORD(MOUSEX),
				 GLODWORD(MOUSEY), GLODWORD(MOUSEFLAGS),
				 GLODWORD(MOUSEANGLE), GLODWORD(MOUSESIZE), 
				 GLODWORD(MOUSESIZE), map) ;
	else
		gr_blit (0, &region, GLODWORD(MOUSEX),
			 GLODWORD(MOUSEY), GLODWORD(MOUSEFLAGS), map) ;

}

/*
 *  FUNCTION : gr_new_object
 *
 *  Register a visible object with a Z coordinate to be drawn
 *  by an user-defined function. This is intended for DLL usage.
 *
 *  If your DLL does some screen operations, you should register
 *  a draw-type hook or an object. Any access to the screen
 *  surface outside those are invalid.
 *
 *  PARAMS : 
 *		z				Z value of the object to be drawn
 *		info			Pointer to the object information function
 *						(fills bounding box, returns 1 if changed since last frame)
 *		draw			Pointer to the object drawing function
 *		what			User-defined parameter that will be passed to "draw"
 *
 *  RETURN VALUE : 
 *      An integer ID that uniquely identifies the object, or -1
 *      if not enough memory
 */

int gr_new_object (int z, int (*info)(void *, REGION *), void (*draw)(void *, REGION *), void * what)
{
	DLL_OBJECT * object ;

	object = (DLL_OBJECT *) malloc(sizeof(DLL_OBJECT));
	if (object == NULL)
		return -1;

	object_list_dirty = 1;

	object->id = ++last_dll_object_id ;
	object->next = first_dll_object ;
	object->hidden = 0 ;
	object->x.info = info;
	object->x.draw = draw;
	object->x.what = what;
	object->x.z = z;
	object->x.bbox.x = -2;
	object->x.bbox.y = -2;
	object->x.bbox.x2= -2;
	object->x.bbox.y2= -2;
	object->x.id = object->id;
	first_dll_object = object;
	dll_object_count++;
	return object->id;
}

/*
 *  FUNCTION : gr_hide_object
 *
 *  Toggle the visibility of an object created by gr_new_object.
 *
 *  PARAMS : 
 *		id				ID returned by gr_new_object
 *		hidden			1 to hide the object, 0 to show it again
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_hide_object (int id, int hidden)
{
	DLL_OBJECT * object = first_dll_object ;

	object_list_dirty = 1;
	background_dirty = 1;

	while (object != NULL)
	{
		if (object->id == id)
		{
			object->hidden = hidden;
			break;
		}
		object = object->next;
	}
}

/*
 *  FUNCTION : gr_destroy_object
 *
 *  Unregister and remove a given object created by gr_new_object from memory
 *
 *  PARAMS : 
 *		id				ID returned by gr_new_object
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_destroy_object (int id)
{
	DLL_OBJECT * object = first_dll_object;
	DLL_OBJECT * next;

	object_list_dirty = 1;

	if (object->id == id)
	{
		first_dll_object = object->next;
		if (object->x.bbox.x == -2)
			(*object->x.info)(object->x.what, &object->x.bbox);
		gr_mark_rect (object->x.bbox.x, object->x.bbox.y,
			object->x.bbox.x2 - object->x.bbox.x + 1,
			object->x.bbox.y2 - object->x.bbox.y + 1);
		free (object);
		dll_object_count--;
	}
	else while (object->next != NULL)
	{
		if (object->next->id == id)
		{
			next = object->next;
			object->next = next->next;
			if (next->x.bbox.x == -2)
				(*next->x.info)(next->x.what, &next->x.bbox);
			gr_mark_rect (next->x.bbox.x, next->x.bbox.y,
				next->x.bbox.x2 - next->x.bbox.x + 1,
				next->x.bbox.y2 - next->x.bbox.y + 1);
			free (next);
			dll_object_count--;
			break;
		}
		object = object->next;
	}
}

int compare_actions (const OBJECT * a1, const OBJECT * a2)
{
	return (a1->z == a2->z ? a1->id - a2->id : a2->z - a1->z) ;
}

void draw_mode7 (void * ptr, REGION * clip)
{
	gr_mode7_draw ((int)ptr) ;
}

int  info_scroll (void * ptr, REGION * bbox)
{
	gr_scroll_bbox((int)ptr, bbox);
	return 1;
}

void draw_scroll (void * ptr, REGION * clip)
{
	gr_scroll_draw ((int)ptr, 1, clip) ;
}

int info_fli (void * ptr, REGION * clip)
{
	if (current_fli)
	{
		clip->x  = current_fli_x - current_fli->bitmap->width/2;
		clip->x2 = current_fli_x + current_fli->bitmap->width/2;
		clip->y  = current_fli_y - current_fli->bitmap->height/2;
		clip->y2 = current_fli_y + current_fli->bitmap->height/2;
		return 1;
	}
	clip->x = clip->x2 = -1;
	clip->y = clip->y2 = -1;
	return 0;
}

void draw_fli (void * ptr, REGION * clip)
{
	if ( (current_fli = flic_do_frame (current_fli)) )
	gr_blit (0, clip, current_fli_x + current_fli->bitmap->width/2, 
		       current_fli_y + current_fli->bitmap->height/2, 
		       0, current_fli->bitmap) ;
}

/*
 *  FUNCTION : gr_mark_rect
 *
 *  Updates the given rectangle as a dirty zone at the 128-bits array
 *
 *  PARAMS :
 *		x, y				Top-left coordinate
 *		width				Width in pixels
 *		height				Height in pixels
 *
 *  RETURN VALUE :
 *		None
 */

void gr_mark_rect (int x, int y, int width, int height)
{
	int cx, cy;
	int w, h;

	w = scr_width / 16;
	h = scr_height / 8;

	for (cx = x/w*w ; cx < x+width ; cx += w)
	{
		for (cy = y/h*h ; cy < y+height ; cy += h)
		{
			if (cx/w < 16 && cx/w >= 0)
				zonearray[cx/w] |= (1 << (cy / h));
		}
	}
}

/*
 *  FUNCTION : gr_mark_instance
 *
 *  Marks the zone where an instance was drawn last frame as dirty
 *
 *  PARAMS :
 *		r					Pointer to the instance
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_mark_instance (INSTANCE * r)
{
	int x, y, w, h;

	w = scr_width / 16;
	h = scr_height / 8;

	for (x = LOCDWORD(r, BOX_X0)/w*w ; x <= LOCDWORD(r, BOX_X1) ; x += w)
	{
		for (y = LOCDWORD(r, BOX_Y0)/h*h ; y <= LOCDWORD(r, BOX_Y1) ; y += h)
		{
			if (x/w < 16 && x/w >= 0)
				zonearray[x/w] |= (1 << (y / h));
		}
	}
}

/*
 *  FUNCTION : gr_mark_instances
 *
 *  Updates a 128-bits array of dirty screen zones, using the current object array.
 *  Changes to 1 all zone bits corresponding to all visible objects (or only to
 *  objects changed since the last frame).
 *
 *  PARAMS :
 *		onlychanged			If 1, mark as dirty only changed or moved visible instances
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_mark_instances (int onlychanged)
{
	int i = 0, x, y;
	int w, h;
	GRAPH * map = 0;
	static REGION mouser = { 0, 0, 0, 0 } ;
	REGION region;

	w = scr_width / 16;
	h = scr_height / 8;

	/* Mark all visible instances and DLL objects */

	for (i = 0 ; i < object_count ; i++)
	{
		if (object_list[i].draw == draw_instance)
		{
			INSTANCE * r = (INSTANCE *)(object_list[i].what);

			/* We assume r is visible, because it is at the array */

			if (onlychanged)
			{
				if ((LOCDWORD(r,CHANGED) = instance_poschanged(r)) == 0)
					continue;

				/* Mark the previous position */

				for (x = LOCDWORD(r, BOX_X0)/w*w ; x <= LOCDWORD(r, BOX_X1) ; x += w)
				{
					for (y = LOCDWORD(r, BOX_Y0)/h*h ; y <= LOCDWORD(r, BOX_Y1) ; y += h)
					{
						if (x/w < 16 && x/w >= 0)
							zonearray[x/w] |= (1 << (y / h));
					}
				}

				/* Update and mark the new position */
				instance_posupdate(r);
				instance_update_bbox (r);	

				for (x = LOCDWORD(r, BOX_X0)/w*w ; x <= LOCDWORD(r, BOX_X1) ; x += w)
				{
					for (y = LOCDWORD(r, BOX_Y0)/h*h ; y <= LOCDWORD(r, BOX_Y1) ; y += h)
					{
						if (x/w < 16 && x/w >= 0)
							zonearray[x/w] |= (1 << (y / h));
					}
				}
			}
			else
			{
				/* Mark the current position (next frame, the next will be marked */
				for (x = LOCDWORD(r, BOX_X0)/w*w ; x <= LOCDWORD(r, BOX_X1) ; x += w)
				{
					for (y = LOCDWORD(r, BOX_Y0)/h*h ; y <= LOCDWORD(r, BOX_Y1) ; y += h)
					{
						if (x/w < 16 && x/w >= 0)
							zonearray[x/w] |= (1 << (y / h));
					}
				}

				/* Update the bounding box */
				instance_update_bbox (r);			
			}
		}
		else
		{
			region = object_list[i].bbox;
			object_list[i].changed = (*object_list[i].info)(object_list[i].what, &object_list[i].bbox);

			/* Mark previous object position */

			if (!onlychanged || object_list[i].changed)
			{
				for (x = region.x/w*w ; x <= region.x2 ; x += w)
					for (y = region.y/h*h ; y <= region.y2 ; y += h)
					{
						if (x/w < 16 && x/w >= 0)
							zonearray[x/w] |= (1 << (y / h));
					}
			}

			/* Mark updated object position */

			if (object_list[i].changed)
			{
				for (x = object_list[i].bbox.x/w*w ; x <= object_list[i].bbox.x2 ; x += w)
					for (y = object_list[i].bbox.y/h*h ; y <= object_list[i].bbox.y2 ; y += h)
					{
						if (x/w < 16 && x/w >= 0)
							zonearray[x/w] |= (1 << (y / h));
					}
			}
		}
	}

	/* Mark the mouse 
	if (GLODWORD(MOUSEGRAPH))
		map = bitmap_get (GLODWORD(MOUSEFILE), GLODWORD(MOUSEGRAPH)) ;

	if (map)
	{
		r = GLODWORD(MOUSEREGION) ;
		if (r < 0 || r > 31) r = 0 ;

		for (x = mouser.x/w*w ; x <= mouser.x2 ; x += w)
		{
			for (y = mouser.y/h*h ; y <= mouser.y2 ; y += h)
			{
				if (x/w < 16 && x/w >= 0)
					zonearray[x/w] |= (1 << (y/h));
			}
		}

		gr_get_bbox (&mouser, &regions[r], GLODWORD(MOUSEX),
			GLODWORD(MOUSEY), GLODWORD(MOUSEFLAGS),
			GLODWORD(MOUSEANGLE), GLODWORD(MOUSESIZE), 
			GLODWORD(MOUSESIZE), map) ;
	}
	*/
}

/*
 *  FUNCTION : gr_mark_rects
 *
 *  Given a 128-bits array of dirty screen zones, create an array of SDL_Rect regions
 *
 *  PARAMS :
 *		zonearray		Pointer to a 128-bits array with dirty screen zones = 1
 *		rects			Pointer to a 128 REGION array
 *
 *  RETURN VALUE : 
 *      Number of rects filled
 */

int gr_mark_rects (REGION * rects)
{
	int count = 0, x, y;
	int w, h, cw, ch, x2;

	w = scr_width / 16;
	h = scr_height / 8;

	for (x = 0 ; x < 16 ; x++)
	{
		for (y = 0 ; y < 8 ; y++)
		{
			if (zonearray[x] & (1 << y))
			{
				zonearray[x] &= ~(1 << y);
				for (cw = 1 ; x+cw < 16 ; cw++)
				{
					if (zonearray[x+cw] & (1 << y))
						zonearray[x+cw] &= ~(1 << y);
					else
						break;
				}
				for (ch = 1 ; y+ch < 8 ; ch++)
				{
					for (x2 = x ; x2 < x+cw ; x2++)
						if (!(zonearray[x2] & (1 << (y+ch))))
							break;
					if (x2 < x+cw)
						break;
					for (x2 = x ; x2 < x+cw ; x2++)
						zonearray[x2] &= ~(1 << (y+ch));
				}
				rects[count].x  = w*x;
				rects[count].y  = h*y;
				rects[count].x2 = w*cw + rects[count].x - 1;
				rects[count].y2 = h*ch + rects[count].y - 1;
				count++;
			}
		}
	}
	return count;
}

/*
 *  FUNCTION : gr_draw_screen
 *
 *  Draw the current screen, using the given restore_type / dump_type parameters
 *
 *  PARAMS : 
 *		dest			Destination graphic (MUST have a correct size)
 *
 *		restore_type	
 *			-1			No background restore
 *			 0			Partial (old process bounding box) background restore
 *			 1			Full background restore
 *
 *		dump_type		
 *		     0			Partial (new process bounding box) drawing
 *			 1			Full drawing
 *
 *  RETURN VALUE : 
 *      None
 */

void gr_draw_screen (GRAPH * dest, int restore_type, int dump_type)
{
	INSTANCE * i ;
	int a, n ;
	DLL_OBJECT * object ;
	GRAPH * oldscrbitmap = scrbitmap;
	scrbitmap = dest;

	/* Create or update the object list */

	if (object_list_dirty)
	{
		gprof_begin("Listing objects");

		object_count = 0;

		i = first_instance ;
		while (i)
		{
			if ((LOCDWORD(i,GRAPHID) || LOCDWORD(i,XGRAPH)) && LOCDWORD(i,CTYPE) == 0 &&
				(LOCDWORD(i,STATUS) == STATUS_RUNNING ||
				LOCDWORD(i,STATUS) == STATUS_FROZEN))
			{
				if (object_list_allocated == object_count)
				{
					object_list_allocated += 16 ;
					object_list = (OBJECT *) realloc (object_list, sizeof(OBJECT) * object_list_allocated) ;
					assert (object_list) ;
				}
				object_list[object_count].z = LOCDWORD(i,COORDZ) ;
				object_list[object_count].draw = draw_instance ;
				object_list[object_count].what = i ;
				object_list[object_count].bbox.x  = -2 ;
				object_list[object_count].bbox.x2 = -2 ;
				object_list[object_count].bbox.y  = -2 ;
				object_list[object_count].bbox.y2 = -2 ;
				object_count++ ;
			}

			i = i->next ;
		}

		/* Añade los objetos creados por DLLs */

		if (object_list_allocated <= object_count+dll_object_count+32)
		{
			object_list_allocated = ((object_count+dll_object_count+32) & ~31) + 32 ;
			object_list = (OBJECT *) realloc (object_list, sizeof(OBJECT) * object_list_allocated) ;
			assert (object_list) ;
		}

		for (object = first_dll_object ; object != NULL ; object = object->next)
		{
			if (object->hidden == 0)
				object_list[object_count++] = object->x ;
		}

		/* Añade las acciones fijas internas, como dibujar textos */

		if (GLODWORD(MOUSEGRAPH))
		{
			object_list[object_count].z = GLODWORD(MOUSEZ) ;
			object_list[object_count].info = info_mouse;
			object_list[object_count].draw = draw_mouse ;
			object_list[object_count].bbox = last_mouse_bbox;
			object_count++ ;
		}

		if (current_fli)
		{
			object_list[object_count].z = 0 ;
			object_list[object_count].info = info_fli;
			object_list[object_count].draw = draw_fli ;
			object_list[object_count].bbox.x  = -2 ;
			object_list[object_count].bbox.x2 = -2 ;
			object_list[object_count].bbox.y  = -2 ;
			object_list[object_count].bbox.y2 = -2 ;
			object_count++ ;
		}

		/* Añade los planos de scroll que haya activos */

		for (n = 0 ; n < 10 ; n++)
		{
			if (gr_scroll_active(n))
			{
				object_list[object_count].z = *(int *)(&GLODWORD(SCROLLS) + 10*n + 4) ;
				object_list[object_count].draw = draw_scroll ;
				object_list[object_count].info = info_scroll ;
				object_list[object_count].what = (void *)n ;
				object_list[object_count].bbox.x  = -2 ;
				object_list[object_count].bbox.x2 = -2 ;
				object_list[object_count].bbox.y  = -2 ;
				object_list[object_count].bbox.y2 = -2 ;
				object_count++ ;
			}
		}

		for (n = 0 ; n < 10 ; n++)
		{
			if (gr_mode7_active(n))
			{
				object_list[object_count].z = *(int *)(&GLODWORD(M7STRUCTS) + 10*n + 5) ;
				object_list[object_count].draw = draw_mode7 ;
				object_list[object_count].what = (void *)n ;
				object_list[object_count].bbox.x  = -2 ;
				object_list[object_count].bbox.x2 = -2 ;
				object_list[object_count].bbox.y  = -2 ;
				object_list[object_count].bbox.y2 = -2 ;
				object_count++ ;
			}
		}

		/* Ordena por Z la lista y ejecuta cada acción */

		qsort (object_list, object_count, sizeof(OBJECT), compare_actions) ;

		object_list_dirty = 0;
		object_list_unsorted = 0;

		gprof_end("Listing objects");
	}
	else if (object_list_unsorted)
	{
		/* Update Z value of instances */
		for (a = 0 ; a < object_count ; a++)
		{
			if (object_list[a].draw == draw_instance)
				object_list[a].z = LOCDWORD(((INSTANCE *)object_list[a].what), COORDZ);
		}
		qsort (object_list, object_count, sizeof(OBJECT), compare_actions) ;
		object_list_unsorted = 0;
	}
	else
	{
		/* Check for unsorted entries, anyway */
		for (a = 0 ; a < object_count ; a++)
		{
			if (a > 0 && object_list[a].z > object_list[a-1].z)
			{
				qsort (object_list, object_count, sizeof(OBJECT), compare_actions) ;
				break;
			}
		}
	}

	/* Restore the background */

	gprof_begin("Background");

	updaterects_count = 1;
	updaterects[0].x = 0;
	updaterects[0].y = 0;
	updaterects[0].x2 = scr_width-1;
	updaterects[0].y2 = scr_height-1;

	if (enable_16bits && GLODWORD(FADING))
		restore_type = -1;

	if (!background_is_black)
	{
		if (enable_16bits && background_8bits_used)
			background_dirty = 1;
		else if (background->modified)
			background_dirty = 1;
	}

	if (restore_type == 1 || background_dirty)
	{
		/* COMPLETE_RESTORE */
		if (background_is_black)
			gr_clear (scrbitmap) ;
		else if (enable_16bits && background_8bits_used)
			gr_blit (scrbitmap, NULL, 0, 0, 128, background_8bits);
		else
			gr_blit (scrbitmap, NULL, 0, 0, 128, background);
		
		if (restore_type != 1)
		{
			for (a = 0 ; a < object_count ; a++)
				if (object_list[a].draw != draw_instance)
					(*object_list[a].info)(object_list[a].what, &object_list[a].bbox);
				else
					instance_update_bbox(object_list[a].what);
		}

		/* Reset the zone-to-update array for the next frame */
		memset (zonearray, 0, 128/8);
	}
	else if (restore_type == 0)
	{
		/* PARTIAL_RESTORE */
		gr_mark_instances (dump_type == 0);
		n = updaterects_count = gr_mark_rects (updaterects);

		/* Reset the zone-to-update array for the next frame */
		memset (zonearray, 0, 128/8);

		gr_setcolor (0);
		for (a = 0 ; a < n ; a++)
		{
			if (background_is_black)
				gr_box (dest, &updaterects[a], 0, 0, 9999, 9999) ;
			else if (enable_16bits && background_8bits_used)
				gr_blit (scrbitmap, &updaterects[a], 0, 0, 128, background_8bits);
			else
				gr_blit (scrbitmap, &updaterects[a], 0, 0, 128, background);
		}
	}

	gprof_end ("Background");

	/* Dump the objects */

	gprof_begin ("Objects");

	if (dump_type == 0 && !background_dirty)
	{
		/* Dump only changed instances and other objects */
		for (n = 0 ; n < updaterects_count ; n++)
		{
			for (a = 0 ; a < object_count ; a++)
			{
				if (object_list[a].draw == draw_instance)
				{
					INSTANCE * i = (INSTANCE *)object_list[a].what;
					if (LOCDWORD(i,BOX_X1) < updaterects[n].x ||
						LOCDWORD(i,BOX_X0) > updaterects[n].x2 ||
						LOCDWORD(i,BOX_Y1) < updaterects[n].y ||
						LOCDWORD(i,BOX_Y0) > updaterects[n].y2)
						continue;
				}
				else
				{
					if (object_list[a].bbox.x2 < updaterects[n].x ||
					    object_list[a].bbox.x  > updaterects[n].x2 ||
					    object_list[a].bbox.y2 < updaterects[n].y ||
					    object_list[a].bbox.y  > updaterects[n].y2)
						continue;
				}
				(*object_list[a].draw) (object_list[a].what, &updaterects[n]) ;
			}
		}
	}
	else
	{
		/* Dump everything */
		for (a = 0 ; a < object_count ; a++)
			(*object_list[a].draw) (object_list[a].what, 0) ;
	}

	background_dirty = 0;

	gprof_end ("Objects");

	scrbitmap = oldscrbitmap;
}

void gr_draw_frame ()
{
#ifdef TARGET_MAC
	jump = 0 ;
#endif
	if (jump)
	{
		do_events() ;		/* Recoge teclas y demás     */
		current_jump++ ;
		return ;
	}
	
	/* Actualiza la paleta */

	if (palette_changed)
		gr_refresh_palette() ;

	/* Bloquea el bitmap de pantalla */

	if (gr_lock_screen() < 0) return ;

	/* Dibuja la pantalla */

	gr_draw_screen (scrbitmap, GLODWORD(RESTORETYPE), GLODWORD(DUMPTYPE));

	/* Fading */

	if (fade_on != 0) 
	{
		gr_fade_step() ;
		background_dirty = 1;
	}

	/* Visualiza la consola */
	
	gr_con_show(show_console) ;
	gr_con_draw() ;

	if (show_profile)
		gprof_draw (scrbitmap);

	/* Actualiza la paleta y la pantalla */

	gr_unlock_screen() ;

	do_events() ;		/* Recoge teclas y demás     */
}

/* FUnción de inicialización de la librería gráfica */

static int screen_locked = 0 ;

#ifdef TARGET_MAC
/* RGB565, same masks as Fenix 16-bit and main's scale path.
 * Do not use a 32-bit surface with Amask 0: SDL 1.2 then picks an
 * alpha channel and scrambles R/B (wrong colors on Metal). */
static SDL_Surface * mac_565 = NULL ;
static SDL_Surface * mac_565s = NULL ;

static SDL_Surface * mac_rgb565_get (SDL_Surface ** slot, int w, int h)
{
	if (*slot && ((*slot)->w != w || (*slot)->h != h))
	{
		SDL_FreeSurface (*slot) ;
		*slot = NULL ;
	}
	if (!*slot)
	{
		*slot = SDL_CreateRGBSurface (SDL_SWSURFACE, w, h, 16,
			0xF800, 0x07E0, 0x001F, 0) ;
		if (*slot)
			SDL_SetAlpha (*slot, 0, 255) ;
	}
	return *slot ;
}

static void mac_present (void)
{
	SDL_Surface * fb, * scaled ;
	SDL_Color * pal ;
	SDL_Rect dstrect ;
	Uint16 mapped[256] ;
	int n, x, y, row, col, scale ;
	int src_w, src_h, dst_w, dst_h ;

	if (!screen || !scrbitmap || !scrbitmap->data) return ;

	src_w = scrbitmap->width ;
	src_h = scrbitmap->height ;
	if (src_w < 1 || src_h < 1) return ;

	fb = mac_rgb565_get (&mac_565, src_w, src_h) ;
	if (!fb || !fb->pixels) return ;

	pal = (fade_step < 64) ? vpalette : palette ;
	for (n = 0 ; n < 256 ; n++)
		mapped[n] = (Uint16) SDL_MapRGB (fb->format, pal[n].r, pal[n].g, pal[n].b) ;

	for (y = 0 ; y < src_h ; y++)
	{
		Uint16 * dst = (Uint16 *) ((Uint8 *) fb->pixels + y * fb->pitch) ;
		if (scrbitmap->depth == 8)
		{
			Uint8 * src = (Uint8 *) scrbitmap->data + y * scrbitmap->pitch ;
			for (x = 0 ; x < src_w ; x++)
				dst[x] = mapped[src[x]] ;
		}
		else
		{
			memcpy (dst, (Uint8 *) scrbitmap->data + y * scrbitmap->pitch,
				(size_t) src_w * 2) ;
		}
	}

	scale = screen->w / src_w ;
	if (screen->h / src_h < scale)
		scale = screen->h / src_h ;
	if (scale < 1) scale = 1 ;

	dst_w = src_w * scale ;
	dst_h = src_h * scale ;

	if (scale == 1)
		scaled = fb ;
	else
	{
		scaled = mac_rgb565_get (&mac_565s, dst_w, dst_h) ;
		if (!scaled || !scaled->pixels) return ;
		for (y = 0 ; y < src_h ; y++)
		{
			Uint16 * src = (Uint16 *) ((Uint8 *) fb->pixels + y * fb->pitch) ;
			for (row = 0 ; row < scale ; row++)
			{
				Uint16 * dst = (Uint16 *) ((Uint8 *) scaled->pixels
					+ (y * scale + row) * scaled->pitch) ;
				for (x = 0 ; x < src_w ; x++)
				{
					Uint16 p = src[x] ;
					for (col = 0 ; col < scale ; col++)
						*dst++ = p ;
				}
			}
		}
	}

	if (screen->w != dst_w || screen->h != dst_h)
		SDL_FillRect (screen, NULL, 0) ;

	dstrect.x = (Sint16) ((screen->w - dst_w) / 2) ;
	dstrect.y = (Sint16) ((screen->h - dst_h) / 2) ;
	dstrect.w = (Uint16) dst_w ;
	dstrect.h = (Uint16) dst_h ;
	SDL_BlitSurface (scaled, NULL, screen, &dstrect) ;
	SDL_Flip (screen) ;
	SDL_UpdateRect (screen, 0, 0, 0, 0) ;
}
#endif

int gr_lock_screen()
{
	if (screen_locked) return 1 ;

#ifdef TARGET_MAC
	/* Never map the game buffer onto the window surface: sdl12-compat
	 * (Metal) often has NULL pixels and 8-bit palettes do not present. */
	screen_locked = 1 ;
	scrbitmap_is_fake = 0 ;
	{
		int w = scr_width ? scr_width : 320 ;
		int h = scr_height ? scr_height : 200 ;
		int d = enable_16bits ? 16 : 8 ;

		if (scrbitmap && (scrbitmap->width != w || scrbitmap->height != h
				|| scrbitmap->depth != d || !scrbitmap->data))
		{
			bitmap_destroy (scrbitmap) ;
			scrbitmap = 0 ;
		}
		if (!scrbitmap)
		{
			scrbitmap = bitmap_new (0, w, h, d) ;
			bitmap_add_cpoint (scrbitmap, 0, 0) ;
			memset (scrbitmap->data, 0, (size_t)scrbitmap->pitch * scrbitmap->height) ;
		}
	}
	return 1 ;
#else

	screen_locked = 1 ;

	if (SDL_LockSurface (screen) < 0)
	{
		screen_locked = 0 ;
		return -1 ;
	}

	if (!enable_2xscale && !double_buffer &&
	    screen->format->BytesPerPixel == (enable_16bits ? 2 : 1))
	{
		if (!scrbitmap)
		{
			scrbitmap = bitmap_new (0, screen->w, screen->h, 
					enable_16bits ? 16:8) ;
			bitmap_add_cpoint (scrbitmap, 0, 0) ;
			free (scrbitmap->data) ;
		}

		scrbitmap_is_fake = 1 ;
		scrbitmap->data   = screen->pixels ;
		scrbitmap->height = screen->h ;

		if (enable_16bits)
		{
			scrbitmap->width  = screen->w ;
			scrbitmap->pitch  = screen->pitch ;
			scrbitmap->depth  = 16 ;
		}
		else
		{
			scrbitmap->width  = screen->w ;
			scrbitmap->pitch  = screen->pitch ;
			scrbitmap->depth  = 8 ;
		}
	}
	else
	{
		scrbitmap_is_fake = 0 ;

		if (!scrbitmap) 
		{
			if (enable_2xscale)
				scrbitmap = bitmap_new (0, screen->w/2, screen->h/2, 
					enable_16bits ? 16:8) ;
			else
				scrbitmap = bitmap_new (0, screen->w, screen->h, 
					enable_16bits ? 16:8) ;
			bitmap_add_cpoint (scrbitmap, 0, 0) ;
		}
	}

	return 1 ;
#endif
}

void gr_unlock_screen()
{
	int a ;

	if (!screen_locked) return ;
	screen_locked = 0 ;

#ifdef TARGET_MAC
	mac_present () ;
	return ;
#endif

	if (screen->pixels == 0) return ;

	if (enable_2xscale)
	{
		if (scrbitmap->depth == 8)
		{
			Uint8 * original;
			Uint16 * extra;
			int length = scrbitmap->width * scrbitmap->height, n;

			if (scrbitmap_extra == NULL 
				|| scrbitmap_extra->width != scrbitmap->width 
				|| scrbitmap_extra->height != scrbitmap->height)
			{
				if (scrbitmap_extra)
					bitmap_destroy (scrbitmap_extra);
				scrbitmap_extra = bitmap_new (0, scrbitmap->width, scrbitmap->height, 16);
			}

			original = scrbitmap->data;
			extra = scrbitmap_extra->data;
			for (n = 0 ; n < 256 ; n++)
				colorequiv[n] = SDL_MapRGB (screen->format,
				palette[n].r, palette[n].g, palette[n].b) ;
			while (length--)
				*extra++ = colorequiv[*original++];
#ifndef TARGET_MAC
			AdMame2x (scrbitmap_extra->data, scrbitmap_extra->pitch,
				      screen->pixels, screen->pitch,
					  scrbitmap->width, scrbitmap->height);
#endif
		}
		else
		{
#ifndef TARGET_MAC
			AdMame2x (scrbitmap->data, scrbitmap->pitch,
				      screen->pixels, screen->pitch,
					  scrbitmap->width, scrbitmap->height);
#endif
		}

		SDL_UnlockSurface (screen) ;
		SDL_UpdateRect (screen, 0, 0, 0, 0) ;
	}
	else if (scrbitmap_is_fake)
	{
		SDL_UnlockSurface (screen) ;
		scrbitmap->data = 0 ;
		if (double_buffer)
			SDL_Flip(screen) ;
		else
		{
#ifdef TARGET_MAC
			SDL_UpdateRect (screen, 0, 0, 0, 0) ;
#else
			if (updaterects_count == 0)
				/* Nothing to update! */ ;
			else
			{
				SDL_Rect rects[128];
				int i;

				for (i = 0 ; i < updaterects_count ; i++)
				{
					rects[i].x = updaterects[i].x;
					rects[i].y = updaterects[i].y;
					rects[i].w = updaterects[i].x2 - rects[i].x + 1;
					rects[i].h = updaterects[i].y2 - rects[i].y + 1;
				}
				SDL_UpdateRects (screen, updaterects_count, rects) ;
			}
#endif
		}
	}
	else
	{
		if (enable_16bits)
		{
			Uint16 * ptr, * orig ;

			orig = scrbitmap->data ;
			ptr  = screen->pixels ;

			if (enable_filtering)
			{
				int n;

				for (a = 0 ; a < screen->h ; a++)
				{
					ptr  = (Uint16 *)screen->pixels + screen->pitch * a / 2 ;

					for (n = 0 ; n < screen->w-1 ; n++)
					{
						*ptr = colorghost[orig[0]] 
						     + colorghost[orig[1]] ;

						ptr++, orig++ ;
					} 
					*ptr++ = *orig++ ;
				}
			}
			else
			{
				for (a = 0 ; a < screen->h ; a++)
				{
					memcpy (ptr, orig, screen->w * 2) ;
					orig += scrbitmap->pitch / 2 ;
					ptr  += screen->pitch / 2 ;
				}
			}
		}
		else if (!enable_16bits && screen->format->BytesPerPixel > 1)
		{
			Uint8 * orig, * dst ;
			int n, x, bpp ;
			Uint32 mapped[256] ;
			SDL_Color * pal ;

			pal = (fade_step < 64) ? vpalette : palette ;
			bpp = screen->format->BytesPerPixel ;
			for (n = 0 ; n < 256 ; n++)
				mapped[n] = SDL_MapRGB (screen->format,
					pal[n].r, pal[n].g, pal[n].b) ;
			for (a = 0 ; a < scrbitmap->height && a < screen->h ; a++)
			{
				orig = (Uint8 *)scrbitmap->data + scrbitmap->pitch * a ;
				dst = (Uint8 *)screen->pixels + screen->pitch * a ;
				if (bpp == 2)
				{
					Uint16 * ptr = (Uint16 *) dst ;
					for (x = 0 ; x < scrbitmap->width && x < screen->w ; x++)
						ptr[x] = (Uint16) mapped[orig[x]] ;
				}
				else if (bpp == 4)
				{
					Uint32 * ptr = (Uint32 *) dst ;
					for (x = 0 ; x < scrbitmap->width && x < screen->w ; x++)
						ptr[x] = mapped[orig[x]] ;
				}
				else
				{
					for (x = 0 ; x < scrbitmap->width && x < screen->w ; x++)
					{
						Uint32 c = mapped[orig[x]] ;
						dst[0] = (Uint8) (c) ;
						dst[1] = (Uint8) (c >> 8) ;
						dst[2] = (Uint8) (c >> 16) ;
						dst += 3 ;
					}
				}
			}
		}
		else
		{
			Uint8 * ptr, * orig ;

			orig = scrbitmap->data ;
			ptr  = screen->pixels ;
			for (a = 0 ; a < screen->h ; a++)
			{
				memcpy (ptr, orig, screen->w) ;
				orig += scrbitmap->pitch ;
				ptr  += screen->pitch ;
			}
		}

		SDL_UnlockSurface (screen) ;
		SDL_UpdateRect (screen, 0, 0, 0, 0) ;
	}
}

void gr_init(int width, int height)
{
	int n ;
	int sdl_flags = 0;
	SDL_Surface *ico ;

	if (globaldata)
	{
		/* Mode depth change! Background is no longer usable */
		if (enable_16bits != ((GLODWORD(GRAPH_MODE) & 0x0010) ? 1 : 0))
		{
			if (background) bitmap_destroy (background);
			if (background_8bits) bitmap_destroy (background_8bits);
			background = background_8bits = 0;
		}
		full_screen    = (GLODWORD(GRAPH_MODE) & 0x0200) ? 1 : 0 ;
		hardware_scr   = (GLODWORD(GRAPH_MODE) & 0x0800) ? 1 : 0 ;
		double_buffer  = (GLODWORD(GRAPH_MODE) & 0x0400) ? 1 : 0 ;
		enable_2xscale = (GLODWORD(GRAPH_MODE) & 0x0100) ? 1 : 0 ;
		enable_16bits  = (GLODWORD(GRAPH_MODE) & 0x0010) ? 1 : 0 ;		
		full_screen   |=  GLODWORD(FULL_SCREEN);
	}

	/* Inicializa el modo gráfico */

	if (scr_initialized && scrbitmap)
	{
		if (scrbitmap_is_fake)
			free (scrbitmap) ;
		else 
			bitmap_destroy (scrbitmap) ;

		scrbitmap = 0 ;
	}

	if (!scr_initialized) scr_initialized = 1 ;

	// Aqui las llamadas a SDL_WM funcs...
	SDL_WM_SetCaption(apptitle,"") ;
	// Icono...
	// Necesitamos crar una surface a partir de un MAP generico de 16x16...	
	if (icono) {				
		if (icono->depth == 8) {
			ico = SDL_CreateRGBSurfaceFrom(icono->data,32,32,8,32,0x00,0x00,0x00,0x00) ;
			SDL_SetPalette(ico, SDL_LOGPAL , palette, 0, 256);
		} else {
			ico = SDL_CreateRGBSurfaceFrom(icono->data,32,32,16,64,0xF800,0x07E0,0x001F,0x00) ;
		}
		SDL_SetColorKey(ico, SDL_SRCCOLORKEY, SDL_MapRGB(ico->format,0,0,0)) ;
		SDL_WM_SetIcon(ico, NULL);
		SDL_FreeSurface(ico) ;		
	}	

#ifdef TARGET_MAC
	/* mac_present integer-scales the native game buffer; do not also
	 * double width/height here or scr_width stays at 2x without a restore. */
	enable_2xscale = 0 ;
#endif
	if (width <= 400 && enable_2xscale)
	{
		width *= 2;
		height *= 2;
	}
	else 
	{
		enable_2xscale = 0 ;
	}

	/* Setup the SDL Video Mode */

	sdl_flags = SDL_HWPALETTE;
	if (double_buffer)
		sdl_flags |= SDL_DOUBLEBUF;
	if (full_screen)
		sdl_flags |= SDL_FULLSCREEN;
	if (hardware_scr)
		sdl_flags |= SDL_HWSURFACE;
	else
		sdl_flags |= SDL_SWSURFACE;

#ifdef TARGET_MAC
	/* Integer-scaled windowed 16-bit, same as main when SCALE is on.
	 * 8-bit palettes do not present on Metal. */
	{
		int scale = 2 ;
		int try_scale ;
		const SDL_VideoInfo * vi = SDL_GetVideoInfo () ;
		int dw = 1920, dh = 1080 ;

		if (vi && vi->current_w >= 320 && vi->current_h >= 200)
		{
			dw = vi->current_w ;
			dh = vi->current_h ;
		}
		scale = 1 ;
		for (try_scale = 2 ; try_scale <= 8 ; try_scale++)
		{
			if (width * try_scale <= dw - 64 && height * try_scale <= dh - 80)
				scale = try_scale ;
		}
		if (width <= 400 && scale < 2)
			scale = 2 ;

		sdl_flags = SDL_SWSURFACE | SDL_DOUBLEBUF ;
		screen = NULL ;
		while (scale >= 1 && !screen)
		{
			screen = SDL_SetVideoMode (width * scale, height * scale, 16, sdl_flags) ;
			if (!screen)
				screen = SDL_SetVideoMode (width * scale, height * scale, 0, sdl_flags) ;
			if (!screen)
				scale-- ;
		}
	}
#else
	screen = SDL_SetVideoMode (width, height, 
		  ((enable_16bits || enable_2xscale) ? 16:8), sdl_flags);
#endif

	if (enable_2xscale)
	{
		width /= 2;
		height /= 2;
	}

	if (!screen)
	{
		gr_con_printf ("Modo grafico %dx%d no disponible: %s\n",
				scr_width, scr_height, SDL_GetError()) ;

		do_exit(1);
	}

#ifdef TARGET_MAC
	fprintf (stderr, "[GRAPH] game %dx%d  window %dx%d %dbpp mask R=%08X G=%08X B=%08X  scale %dx\n",
		width, height, screen->w, screen->h, screen->format->BitsPerPixel,
		(unsigned) screen->format->Rmask,
		(unsigned) screen->format->Gmask,
		(unsigned) screen->format->Bmask,
		screen->w / (width ? width : 1)) ;
#endif

	if (enable_16bits)
	{
#ifndef TARGET_MAC
		if (screen->format->BytesPerPixel != 2)
		{
			printf ("Profundidad de color de 16 bits no soportada\n") ;
			do_exit(1) ;
		}
#endif
		for (n = 0 ; n < 65536 ; n++)
		{
			colorghost[n] = 
			     (((n & screen->format->Rmask) >> 1) & screen->format->Rmask) 
			   + (((n & screen->format->Gmask) >> 1) & screen->format->Gmask) 
			   + (((n & screen->format->Bmask) >> 1) & screen->format->Bmask) ;
		}
	}
	
	if (report_graphics)
		gr_con_printf ("[GRAPH] Graphic mode started - %dx%d - %s\n", 
			width, height, enable_16bits ? "16 bits":"8 bits") ;

	if (enable_16bits)
	{
		Uint32 m, Rbits = 0, Gbits = 0, Bbits = 0 ;
		
		for (m = screen->format->Rmask; m; m >>= 1) Rbits += (m&1) ;
		for (m = screen->format->Gmask; m; m >>= 1) Gbits += (m&1) ;
		for (m = screen->format->Bmask; m; m >>= 1) Bbits += (m&1) ;
		
		if (report_graphics)
			gr_con_printf ("[GRAPH] RGB %d%d%d: "
			"Masks R=0x%04X G=0x%04X B=0x%04X\n",
			Rbits, Gbits, Bbits,
			screen->format->Rmask,
			screen->format->Gmask,
			screen->format->Bmask) ;
	}
	if (report_graphics && (screen->flags & SDL_DOUBLEBUF))
		gr_con_printf ("[GRAPH] Doble buffer activo\n") ;
	fflush(stdout) ;

	SDL_ShowCursor (0) ;

	regions[0].x  = 0 ;
	regions[0].y  = 0 ;
	regions[0].x2 = width-1 ;
	regions[0].y2 = height-1 ;

	/* Bitmaps de fondo */

	if (!background || scr_width != width || scr_height != height)
	{
		if (background) bitmap_destroy (background);
		background = bitmap_new (0, width, height, enable_16bits ? 16:8) ;
		assert (background) ;
		gr_clear (background) ;
		bitmap_add_cpoint (background, 0, 0) ;

		background_is_black = 1 ;
	}

	if (enable_16bits)
	{
		if (!background_8bits || scr_width != width || scr_height != height)
		{
			if (background_8bits) bitmap_destroy (background_8bits) ;
			background_8bits = bitmap_new (0, width, height, 8) ;
			assert (background_8bits) ;
			gr_clear (background_8bits) ;
			bitmap_add_cpoint (background_8bits, 0, 0) ;
			background_8bits_used = 0 ;
		}
	}

	scr_width = width ;
	scr_height = height ;

	/* Paleta de colores por defecto */

	if (!palette_loaded)
	{
		for (n = 0 ; n < 256 ; n++) 
			gr_set_rgb (n, default_palette[n*3]/4, default_palette[n*3+1]/4, 
					default_palette[n*3+2]/4) ;
	}

	gr_refresh_palette() ;

	next_frame_ticks = SDL_GetTicks() + frame_ms ;

	scr_initialized = 2 ;

	memset (zonearray, 255, 128/8);	
}

#ifdef WIN32
#include <windows.h>
#endif

void gr_error (const char *fmt, ...)
{
	char text[4000] ;

	va_list ap;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	
#ifdef TARGET_BeOS
	if(full_screen == 0)
		be_alert(text) ;
	else
		fprintf (stderr, "%s\n", text) ;	
#else 
	fprintf (stderr, "%s\n", text) ;
#endif
	do_exit (1) ;
}

