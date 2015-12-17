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

#ifndef __GRLIB_H
#define __GRLIB_H

#include "instance_st.h"
#include "grlib_st.h"

#define MAX_JOYS    32

/* -------------------------------------------------------------------- */
/* Librería gráfica                                                     */
/* -------------------------------------------------------------------- */

extern int              scr_initialized ;
extern int              audio_initialized ;

extern int				enable_16bits ;      /* 1 = 16bpp MODE on                */
extern int				enable_filtering ;   /* 1 = 16bpp filter MODE on         */

extern Uint16			syscolor16 ;
extern int				syscolor8 ;			 /* Color for drawing primitives	 */
extern Uint16           fntcolor16 ;
extern int				fntcolor8 ;			 /* Color for drawing bitmap text	 */

extern Uint32           drawing_stipple;

extern REGION           regions[32] ;
extern SDL_Surface		* screen ;
extern GRAPH            * background ;
extern GRAPH            * scrbitmap ;
extern GRLIB            * syslib ;
extern GRAPH            * icono ;
extern SDL_Joystick		* _joysticks[MAX_JOYS];
extern SDL_Joystick		* selected_joystick;
extern scrolldata		scrolls[10] ;
extern int              last_frame_ms ;
extern char             * apptitle ;
extern key_equiv        key_table[] ;
extern unsigned char    * keystate ;
extern int              keytab_initialized ;

extern int              scr_width ;
extern int              scr_height ;

extern int              full_screen ;
extern int              frameless ;
extern int              double_buffer ;
extern int				grab_input ;
extern int				window_status ;
extern int				focus_status ;
extern int				mouse_status ;
extern int				exit_status ;
extern int				background_dirty ;

extern FONT * fonts[256] ;
extern int    font_count ;


/* Inicialización y control de tiempo */
/* ---------------------------------- */

extern void gr_init           (int w, int h) ;
extern void gr_set_fps        (int fps, int max_jump) ;
extern void gr_wait_frame     () ;
extern void gr_advance_timers () ;
extern void gr_draw_frame     () ;
extern void gr_draw_screen    (GRAPH * dest, int restore_type, int dump_type) ;
extern void gr_error          (const char *fmt, ...) ;
extern void keytab_init       () ;
extern void keytab_free       () ;
extern int  gr_key            (int code) ;
extern int  gr_timer          () ;

extern Uint32 frame_count;
extern Uint32 current_time;

/* Objetos definidos por DLL */
/* ------------------------- */

extern int  gr_new_object		(int z, int (*info)(void *, REGION *), void (*draw)(void *, REGION *), void * what);
extern void gr_hide_object		(int id, int hidden);
extern void gr_destroy_object	(int id);

extern int object_list_dirty;
extern int object_list_unsorted;

/* Paleta de colores */
/* ----------------- */

extern SDL_Color    vpalette[256] ;
extern SDL_Color    palette[256] ;

extern Uint16       colorequiv[256]   ;     /* Equivalencia paleta -> pantalla   */
extern Uint16       colorghost[65536] ;     /* Deja un color a 50% de intensidad */

extern GRAPH      * background_8bits ;
extern int          background_8bits_used ;
extern int          background_is_black ;

extern Uint8        trans_table[256][256] ; /* Tabla de transparencias 8 bits        */
extern int          trans_table_updated ;   /* 1 = La tabla es utilizable            */
extern Uint8        nearest_table[64][64][64] ; /* Conversión color -> paleta        */

extern void         gr_make_trans_table() ;
extern void         gr_fill_nearest_table() ;

extern int          palette_loaded ;        /* ¿Se ha cargado ya la paleta inicial ? */
extern int          palette_changed ;       /* Poner a 1 cuando se cambien colores   */
extern int          fade_on ;               /* ¿Hay un fade activo?                  */
extern int          fade_step ;             /* Si lo hay, posición (0=off)           */

extern int          gr_read_pal           (file * file) ;
extern void         gr_refresh_palette    () ;
extern void         gr_fade_init          (int pr, int pg, int pb, int speed) ;
extern void         gr_fade_step          () ;
extern void         gr_roll_palette       (int color0, int num, int inc) ;
extern int          gr_find_nearest_color (int r, int g, int b) ;
extern void         gr_set_rgb            (int c, int r, int g, int b) ;
extern int          gr_rgb                (int r, int g, int b) ;
extern void         gr_get_rgb            (int color, int *r, int *g, int *b) ;
extern void         gr_set_colors         (int color, int num, Uint8 * pal) ;
extern void         gr_get_colors         (int color, int num, Uint8 * pal) ;

/* Blend ops */
/* --------- */

extern Sint16 * blend_create       () ;
extern void     blend_free         () ;
extern void     blend_init         (Sint16 * blend) ;
extern void     blend_translucency (Sint16 * blend, float ammount) ;
extern void     blend_intensity    (Sint16 * blend, float ammount) ;
extern void     blend_tint         (Sint16 * blend, float ammount, Uint8 cr, Uint8 cg, Uint8 cb) ;
extern void     blend_swap         (Sint16 * blend) ;
extern void     blend_assign       (GRAPH * bitmap, Sint16 * blend) ;
extern void     blend_apply        (GRAPH * bitmap, Sint16 * blend) ;
extern void     blend_grayscale    (Sint16 * blend, int method) ;

/* FBM/FGC/FPL files */
/* ----------------- */

extern GRAPH *	fbm_load_from    (file * fp, int fgc_depth);
extern GRAPH *  fbm_load		 (const char * filename);
extern int 		fbm_save		 (GRAPH * graph, const char * filename);
extern int      fbm_save_to		 (GRAPH * map, file * fp, int with_palette);
extern Uint32   fbm_size		 (GRAPH * graph, int with_header, int with_palette);

extern int		fgc_load		 (const char * filename);
extern int		fgc_save		 (int id, const char * filename);

extern int		fpl_load_from	 (file * fp);
extern int		fpl_save		 (const char * filename);

extern const char * fbm_error;
extern const char * fgc_error;
extern const char * fpl_error;


/* Gestión de bitmaps y librerías de gráficos */
/* ------------------------------------------ */

extern int      gr_load_map      (const char * filename) ;
extern int      gr_load_png      (const char * filename) ;
extern int      gr_load_pcx      (const char * filename) ;
extern int      gr_load_fpg      (const char * filename) ;
extern int      gr_load_pal      (const char * filename) ;

extern int      gr_save_pal      (const char * filename) ;
extern int		gr_save_png		 (GRAPH * gr, const char * filename) ;

extern GRLIB  * grlib_get        (int libid) ;
extern void     grlib_init       () ;
extern int      grlib_new        () ;
extern void     grlib_destroy    (int libid) ;
extern int      grlib_add_map    (int libid, GRAPH * map) ;
extern int      grlib_unload_map (int libid, int mapcode) ;

extern GRAPH * bitmap_new        (int code, int w, int h, int depth, int frames) ;
extern GRAPH * bitmap_clone      (GRAPH * t) ;
extern GRAPH * bitmap_new_syslib (int w, int h, int depth, int frames) ;
extern GRAPH * bitmap_get        (int libid, int mapcode) ;
extern void    bitmap_destroy    (GRAPH * map) ;
extern void    bitmap_add_cpoint (GRAPH *map, int x, int y) ;
extern void    bitmap_set_cpoint (GRAPH * map, Uint32 point, int x, int y);
extern void    bitmap_analize    (GRAPH * bitmap) ;
extern void    bitmap_animate    (GRAPH * bitmap) ;
extern void    bitmap_animate_to (GRAPH * bitmap, int pos, int speed) ;
extern int     bitmap_next_code  ();

extern void	   bitmap_16bits_conversion();

/* Regiones */
/* -------- */

extern void     region_define   (int region, int x, int y, int width, int height) ;
extern void     region_union    (REGION * a, REGION * b) ;
extern int      region_is_empty (REGION * a) ;
extern int      region_is_out   (REGION * a, REGION * b) ;
extern REGION * region_new      (int x, int y, int width, int height);
extern void     region_destroy  (REGION *);
extern REGION * region_get      (int n);

/* Alto nivel */
/* ---------- */

extern void    draw_instance_at (INSTANCE * i, REGION * r, int x, int y) ;
extern void    draw_instance    (INSTANCE * i, REGION * clip) ;
extern void    instance_update_bbox(INSTANCE * i) ;
extern GRAPH * instance_graph   (INSTANCE * i) ;
extern int     instance_visible (INSTANCE * i);
extern void    scroll_region    (int nscroll, REGION * r) ;

/* Textos */
/* ------ */

/* Opciones para gr_font_newfrombitmap */
#define NFB_FIXEDWIDTH	1

extern int      gr_font_load       (char * filename) ;
extern int      gr_load_bdf        (const char * filename) ;
extern int      gr_load_ttf        (const char * filename, int size, int bpp, int fg, int bg) ;
extern int      gr_font_save       (int fontid, const char * filename) ;
extern int      gr_font_new        () ;
extern int      gr_font_newfrombitmap (char * chardata, int width, int height, int options) ;
extern int      gr_font_systemfont (char * chardata) ;
extern void     gr_font_destroy    (int fontid) ;
extern FONT *   gr_font_get		   (int id);

extern void		gr_text_setcolor   (int c) ;
extern int		gr_text_getcolor   () ;
extern int      gr_text_new        (int fontid, int x, int y, int centered, const char * text) ;
extern int      gr_text_new_var    (int fontid, int x, int y, int centered, const void * var, int type) ;
extern void     gr_text_move       (int textid, int x, int y) ;
extern void     gr_text_destroy    (int textid) ;
extern int      gr_text_margintop  (int fontid, const unsigned char * text) ;
extern int      gr_text_width      (int fontid, const unsigned char * text) ;
extern int      gr_text_widthn     (int fontid, const unsigned char * text, int n) ;
extern int      gr_text_height     (int fontid, const unsigned char * text) ;
extern int      gr_text_put        (GRAPH * dest, REGION * region, int fontid, int x, int y, const unsigned char * text) ;
extern GRAPH  * gr_text_bitmap     (int fontid, const char * text, int centered) ;

// Tipos para gr_text_new_var

#define TEXT_TEXT		1
#define TEXT_STRING		2
#define TEXT_INT		3
#define TEXT_FLOAT		4
#define TEXT_WORD		5
#define TEXT_BYTE		6
#define TEXT_CHARARRAY	7
#define TEXT_SHORT		8
#define TEXT_DWORD		9
#define TEXT_SBYTE		10
#define TEXT_CHAR		11

/* Bajo nivel */
/* ---------- */

/* Las funciones gráficas admiten dest=0 para referirse a la pantalla.
 * Para poder usar esta funcionalidad, debe estar bloqueada antes */

extern int  gr_lock_screen   () ;
extern void gr_unlock_screen () ;
extern void gr_mark_rect (int x, int y, int width, int height);
extern void gr_mark_instance (INSTANCE *);

/* Primitivas gráficas */

extern void gr_clear     (GRAPH * dest) ;
extern void gr_clear_as  (GRAPH * dest, int color) ;
extern void gr_put_pixel (GRAPH * dest, int x, int y, int color) ;
extern int  gr_get_pixel (GRAPH * dest, int x, int y) ;

extern void gr_setcolor  (int c) ;
extern void gr_setalpha  (int alpha);
extern void gr_vline     (GRAPH * dest, REGION * clip, int x, int y, int h) ;
extern void gr_hline     (GRAPH * dest, REGION * clip, int x, int y, int w) ;
extern void gr_line      (GRAPH * dest, REGION * clip, int x, int y, int w, int h) ;
extern void gr_box       (GRAPH * dest, REGION * clip, int x, int y, int w, int h) ;
extern void gr_rectangle (GRAPH * dest, REGION * clip, int x, int y, int w, int h) ;
extern void gr_circle    (GRAPH * dest, REGION * clip, int x, int y, int r) ;
extern void gr_fcircle   (GRAPH * dest, REGION * clip, int x, int y, int r) ;
extern void gr_bezier    (GRAPH * dest, REGION * clip, int * params) ;

extern int  gr_drawing_new     (DRAWING_OBJECT drawing, int z) ;
extern void gr_drawing_destroy (int id) ;
extern void gr_drawing_move    (int id, int x, int y) ;

/* Bitmaps */

extern void gr_blit          (GRAPH * dest, REGION * clip, int x, int y, int flags, GRAPH * gr) ;
extern void gr_get_bbox      (REGION * dest, REGION * clip, int x, int y, int flags, int angle, int scalex, int scaley, GRAPH * gr) ;
extern void gr_rotated_blit  (GRAPH * dest, REGION * clip, int x, int y, int flags, int angle, int scalex, int scaley, GRAPH * gr) ;

/* Scroll */
/* ------ */

extern void gr_scroll_start         (int n, int fileid, int graphid, int backid, int region, int flags) ;
extern void gr_scroll_stop          (int n) ;
extern void gr_scroll_draw          (int n, int do_drawing, REGION * clipping) ;
extern void gr_scroll_bbox			(int n, REGION * r) ;

extern int  gr_scroll_active        (int n) ;
extern int  gr_scroll_is_fullscreen () ;

/* Modo 7 */
/* ------ */

extern void gr_mode7_start  (int n, int fileid, int inid, int outid, int region, int inclination) ;
extern void gr_mode7_stop   (int n) ;
extern void gr_mode7_draw   (int n) ;
extern int  gr_mode7_active (int n) ;
extern void gr_mode7_bbox   (int n, REGION * r) ;

/* Consola del sistema */
/* ------------------- */

extern int  show_console ;

extern void gr_sys_color   (int cfg, int cbg) ;
extern void gr_sys_puts    (GRAPH * map, int x, int y, Uint8 * str, int len) ;
extern void gr_sys_putchar (GRAPH * map, int ox, int oy, Uint8 c) ;

extern void gr_con_printf  (const char *fmt, ...) ;
extern void gr_con_putline (char * text) ;
extern void gr_con_show    (int doit) ;
extern void gr_con_draw    () ;
extern void gr_con_getkey  (int key, int sym) ;
extern void gr_con_scroll  (int direction) ;
extern void gr_con_lateral_scroll (int direction);
extern void gr_con_do      (const char * command) ;

/* Profiler */
/* -------- */

extern void gprof_init  ();
extern void gprof_begin (const char * name);
extern void gprof_end   (const char * name);
extern void gprof_frame ();
extern void gprof_dump  (const char * filename);
extern void gprof_reset ();
extern void gprof_draw  (GRAPH * dest);
extern void gprof_toggle();

/* Versiones MMX de rutinas internas (no exportables) */
/* -------------------------------------------------- */

#ifdef WIN32
  #ifndef __GNUC__
  #define MMX_FUNCTIONS
  #endif

extern int  MMX_available;

extern void MMX_init();
extern void MMX_draw_hspan_8to8_nocolorkey (Uint8 * scr, Uint8 * tex, int pixels, int incs);
extern void MMX_draw_hspan_8to8_translucent	(Uint8 * scr, Uint8 * tex, int pixels, int incs);
extern void MMX_draw_hspan_8to8 (Uint8 * scr, Uint8 * tex, int pixels, int incs);
extern void MMX_draw_hspan_16to16 (Uint16 * scr, Uint16 * tex, int pixels, int incs);
extern void MMX_draw_hspan_16to16_translucent (Uint16 * scr, Uint16 * tex, int pixels, int incs);
extern void MMX_draw_hspan_16to16_nocolorkey (Uint16 * scr, Uint16 * tex, int pixels, int incs);
#endif

/* Rutinas de conversión entre formatos */

extern void     gr_convert16_ScreenTo565 (Uint16 * ptr, int len);
extern void     gr_convert16_565ToScreen (Uint16 * ptr, int len);
extern void     gr_fade16 (GRAPH * graph, int r, int g, int b);
extern Uint16 * gr_alpha16 (int alpha);
extern Uint8  * gr_alpha8  (int alpha);


/* Rutinas del Mame's 2xScale algorithm */

extern void scale2x(Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height);
/*extern void scale2x32(Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height);*/

/* Rutinas del ScummVM's HQ2x algorithm */

extern void hq2x(Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height);

/* Otras rutinas de SCALE */
extern void scale_normal2x(Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height);
extern void scanline2x(Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height);

#endif
