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
 *  Copyright ù 1999 Josù Luis Cebriùn Pagùe
 *  Copyright ù 2002 Fenix Team
 *
 */

#ifndef __FXDLL_H
#define __FXDLL_H

#define FNXDLL

#ifdef TARGET_MAC
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include <instance_st.h>
#include <grlib_st.h>
#include <files_st.h>
#include <typedef_st.h>
#include <xctype_st.h>
#include <flic_st.h>
#include <i_procdef_st.h>
#include <offsets.h>
#include <pslang.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MAIN_FILE) || !defined(MULTIPLE_FILES)
#define FXEXTERN
#else
#define FXEXTERN extern
#endif

/*
 * Declaracion de funciones
 */

/*****************************************************
 * fxi.h
 *****************************************************/

FXEXTERN int * _debug ;            /* 1 = Actovate debug options       */
FXEXTERN int * _fxi ;              /* 1 = EXE module is FXI.EXE        */
FXEXTERN int * _enable_16bits ;    /* 1 = 16bpp MODE on                */
FXEXTERN int * _enable_filtering ; /* 1 = 16bpp filter MODE on         */

FXEXTERN Uint32 * _frame_count ;
FXEXTERN Uint32 * _current_time ;

FXEXTERN SDL_Surface	* * _screen ;

#define debug              ( * _debug             )
#define fxi                ( * _fxi               )
#define enable_16bits      ( * _enable_16bits     )
#define enable_filtering   ( * _enable_filtering  )
#define screen             ( * _screen            )
#define frame_count        ( * _frame_count       )
#define current_time       ( * _current_time      )

FXEXTERN int * _syscolor8 ;
FXEXTERN int * _syscolor16 ;
FXEXTERN int * _fntcolor8 ;
FXEXTERN int * _fntcolor16 ;

#define syscolor8  (*_syscolor8)
#define syscolor16 (*_syscolor16)
#define fntcolor8  (*_fntcolor8)
#define fntcolor16 (*_fntcolor16)

FXEXTERN int * _drawing_stipple;

#define drawing_stipple (*_drawing_stipple)

FXEXTERN int * _report_audio ;
FXEXTERN int * _report_string ;
FXEXTERN int * _report_graphics ;

#define report_audio        (* _report_audio    )
#define report_string       (* _report_string   )
#define report_graphics     (* _report_graphics )

FXEXTERN void   (* _gr_error      )(const char *fmt, ...) ;
FXEXTERN void   (* _do_exit       )(int n) ;
FXEXTERN int    (* _dcb_load      )(const char * filename) ;
FXEXTERN char * (* _getid         )(unsigned int code) ;
FXEXTERN int    (* _path_find     )(GRAPH * bitmap, int sx, int sy, int dx, int dy, int options) ;
FXEXTERN int    (* _path_get      )(int * x, int * y) ;
FXEXTERN int    (* _path_set_wall )(int n) ;

#define gr_error            (* _gr_error)
#define do_exit				(* _do_exit)
#define dcb_load			(* _dcb_load)
#define getid				(* _getid)
#define path_find			(* _path_find)
#define path_get			(* _path_get)
#define path_set_wall		(* _path_set_wall)

/*****************************************************
 * files.h
 *****************************************************/

FXEXTERN file * (*_file_open       )(const char * filename, char * mode) ;
FXEXTERN int    (*_file_read       )(file * fp, void * buffer, int len) ;
FXEXTERN int    (*_file_write      )(file * fp, const void * buffer, int len) ;
FXEXTERN int    (*_file_gets       )(file * fp, char * buffer, int len) ;
FXEXTERN int    (*_file_puts       )(file * fp, const char * buffer) ;
FXEXTERN int    (*_file_qgets      )(file * fp, char * buffer, int len) ;
FXEXTERN int    (*_file_qputs      )(file * fp, const char * buffer) ;
FXEXTERN int    (*_file_size       )(file * fp) ;
FXEXTERN int    (*_file_pos        )(file * fp) ;
FXEXTERN int    (*_file_seek       )(file * fp, int pos, int where) ;
FXEXTERN void   (*_file_addp       )(const char * path) ;
FXEXTERN void   (*_file_close      )(file * fp) ;
FXEXTERN int    (*_file_exists     )(const char * filename) ;
FXEXTERN void   (*_file_add_xfile  )(file * fp, long offset, char * name, int size) ;
FXEXTERN int    (*_file_eof        )(file * fp) ;
FXEXTERN FILE * (*_file_fp         )(file * fp) ;

#define file_open		(*_file_open)
#define file_read		(*_file_read)
#define file_write		(*_file_write)
#define file_gets		(*_file_gets)
#define file_puts		(*_file_puts)
#define file_qgets		(*_file_qgets)
#define file_qputs		(*_file_qputs)
#define file_size		(*_file_size)
#define file_pos		(*_file_pos)
#define file_seek		(*_file_seek)
#define file_addp		(*_file_addp)
#define file_close		(*_file_close)
#define file_exists		(*_file_exists)
#define file_add_xfile  (*_file_add_xfile)
#define file_eof		(*_file_eof)
#define file_fp			(*_file_fp)

/*****************************************************
 * strings.h
 *****************************************************/

FXEXTERN void         (*_string_init    )() ;
FXEXTERN const char * (*_string_get     )(int code) ;
FXEXTERN void         (*_string_dump    )() ;
FXEXTERN void         (*_string_load    )(file *) ;
FXEXTERN void         (*_string_save    )(file *) ;

FXEXTERN int          (*_string_new     )(const char * ptr) ;
FXEXTERN int          (*_string_newa    )(const char * ptr, unsigned count) ;
FXEXTERN void         (*_string_use     )(int code) ;
FXEXTERN void         (*_string_discard )(int code) ;
FXEXTERN int          (*_string_add     )(int code1, int code2) ;
FXEXTERN int          (*_string_compile )(const char * * source) ;
FXEXTERN int          (*_string_itoa    )(int n) ;
FXEXTERN int          (*_string_ftoa    )(float n) ;
FXEXTERN int          (*_string_ptoa    )(void * n) ;
FXEXTERN int          (*_string_comp    )(int code1, int code2) ;
FXEXTERN int          (*_string_casecmp )(int code1, int code2) ;
FXEXTERN int          (*_string_char    )(int n, int nchar) ;
FXEXTERN int          (*_string_substr  )(int code, int first, int len) ;
FXEXTERN int          (*_string_find    )(int code1, int code2, int first) ;
FXEXTERN int          (*_string_ucase   )(int code1) ;
FXEXTERN int          (*_string_lcase   )(int code1) ;
FXEXTERN int          (*_string_strip   )(int code1) ;
FXEXTERN int          (*_string_pad     )(int code, int length, int align) ;
FXEXTERN int			 (*_string_format  )(double number, int dec, char point, char thousands) ;
FXEXTERN void         (*_string_coalesce)() ;
FXEXTERN int          (*_string_concat  )(int code1, char * str2) ;

#define string_init		(*_string_init)
#define string_get		(*_string_get)
#define string_dump		(*_string_dump)
#define string_load		(*_string_load)
#define string_save		(*_string_save)
#define string_new		(*_string_new)
#define string_newa		(*_string_newa)
#define string_use		(*_string_use)
#define string_discard	(*_string_discard)
#define string_add		(*_string_add)
#define string_compile	(*_string_compile)
#define string_itoa		(*_string_itoa)
#define string_ftoa		(*_string_ftoa)
#define string_ptoa		(*_string_ptoa)
#define string_comp		(*_string_comp)
#define string_casecmp	(*_string_casecmp)
#define string_char		(*_string_char)
#define string_substr	(*_string_substr)
#define string_find		(*_string_find)
#define string_ucase	(*_string_ucase)
#define string_lcase	(*_string_lcase)
#define string_strip	(*_string_strip)
#define string_pad		(*_string_pad)
#define string_format	(*_string_format)
#define string_coalesce (*_string_coalesce)
#define string_concat   (*_string_concat)

/*****************************************************
 * xctype.h
 *****************************************************/

FXEXTERN int           * _dos_chars ;		/* 1 = Cùdigo fuente en caracteres MS-DOS */
FXEXTERN char          ** _c_type ;
FXEXTERN unsigned char ** _c_upper ;
FXEXTERN unsigned char ** _c_lower ;

FXEXTERN unsigned char (*_convert    )(unsigned char c) ;
FXEXTERN void          (*_init_c_type)() ;

FXEXTERN unsigned char ** _dos_to_win ;
FXEXTERN unsigned char ** _win_to_dos ;

#define dos_chars    (* _dos_chars )
#define c_type       (* _c_type    )
#define c_upper      (* _c_upper   )
#define c_lower      (* _c_lower   )
#define dos_to_win   (* _dos_to_win)
#define win_to_dos   (* _win_to_dos)
#define convert      (* _convert   )
#define init_c_type  (* _init_c_type)

/*
 * typedef.h
 */

FXEXTERN TYPEDEF      (*_typedef_new      )(BASETYPE type) ;
FXEXTERN TYPEDEF      (*_typedef_enlarge  )(TYPEDEF base) ;
FXEXTERN TYPEDEF      (*_typedef_reduce   )(TYPEDEF base) ;
FXEXTERN int          (*_typedef_size     )(TYPEDEF t) ;
FXEXTERN int          (*_typedef_subsize  )(TYPEDEF t, int c) ;
FXEXTERN void         (*_typedef_describe )(char * buffer, TYPEDEF t) ;
FXEXTERN TYPEDEF      (*_typedef_pointer  )(TYPEDEF to) ;

FXEXTERN TYPEDEF    * (*_typedef_by_name  )(int code) ;
FXEXTERN void         (*_typedef_name     )(TYPEDEF t, int code) ;

#define typedef_new			(*_typedef_new)
#define typedef_enlarge		(*_typedef_enlarge)
#define typedef_reduce		(*_typedef_reduce)
#define typedef_size		(*_typedef_size)
#define typedef_subsize		(*_typedef_subsize)
#define typedef_describe	(*_typedef_describe)
#define typedef_pointer		(*_typedef_pointer)
#define typedef_by_name		(*_typedef_by_name)
#define typedef_name		(*_typedef_name)

/*****************************************************
 * flic.h
 *****************************************************/

FXEXTERN FLIC * (*_flic_open     )(const char * filename) ;
FXEXTERN void   (*_flic_destroy  )(FLIC * flic) ;
FXEXTERN FLIC * (*_flic_do_frame )(FLIC * flic) ;
FXEXTERN void   (*_flic_reset    )(FLIC * flic) ;

FXEXTERN FLIC *  *_current_fli   ;
FXEXTERN int     *_current_fli_x ;
FXEXTERN int     *_current_fli_y ;

#define current_fli     (*_current_fli   )
#define current_fli_x   (*_current_fli_x )
#define current_fli_y   (*_current_fli_y )

#define flic_open		(*_flic_open)
#define flic_destroy	(*_flic_destroy)
#define flic_do_frame	(*_flic_do_frame)
#define flic_reset		(*_flic_reset)

/*****************************************************
 * grlib.h
 *****************************************************/

FXEXTERN int              * _scr_initialized ;
FXEXTERN REGION          ** _regions ;
FXEXTERN GRAPH           ** _background ;
FXEXTERN GRAPH           ** _scrbitmap ;
FXEXTERN GRLIB           ** _syslib ;
FXEXTERN GRAPH           ** _icono ;
FXEXTERN int              * _last_frame_ms ;
FXEXTERN char            ** _apptitle ;
FXEXTERN key_equiv       ** _key_table ;
FXEXTERN unsigned char   ** _keystate ;
FXEXTERN int              * _keytab_initialized ;

FXEXTERN int              * _scr_width ;
FXEXTERN int              * _scr_height ;

FXEXTERN int              * _full_screen ;
FXEXTERN int              * _double_buffer ;
FXEXTERN int              * _exit_status ;
FXEXTERN int              * _grab_input ;
FXEXTERN int              * _window_status ;
FXEXTERN int              * _focus_status ;
FXEXTERN int              * _mouse_status ;

#define scr_initialized      ( * _scr_initialized    )
#define regions              ( * _regions            )
#define background           ( * _background         )
#define scrbitmap            ( * _scrbitmap          )
#define syslib               ( * _syslib             )
#define icono                ( * _icono              )
#define last_frame_ms        ( * _last_frame_ms      )
#define joy_x                ( * _joy_x              )
#define joy_y                ( * _joy_y              )
#define joy_b                ( * _joy_b              )
#define apptitle             ( * _apptitle           )
#define key_table            ( * _key_table          )
#define keystate             ( * _keystate           )
#define keytab_initialized   ( * _keytab_initialized )
#define scr_width            ( * _scr_width          )
#define scr_height           ( * _scr_height         )
#define full_screen          ( * _full_screen        )
#define window_status        ( * _window_status      )
#define focus_status         ( * _focus_status       )
#define mouse_status         ( * _mouse_status       )
#define exit_status          ( * _exit_status        )
#define grab_input           ( * _grab_input         )
#define double_buffer        ( * _double_buffer      )

FXEXTERN char (*_nearest_table)[64][64][64];
FXEXTERN void (*_gr_fill_nearest_table)();

#define nearest_table (*_nearest_table)
#define gr_fill_nearest_table (*_gr_fill_nearest_table)

/* Inicializaciùn y control de tiempo */
/* ---------------------------------- */

FXEXTERN void (*_gr_init           )(int w, int h) ;
FXEXTERN void (*_gr_set_fps        )(int fps, int max_jump) ;
FXEXTERN void (*_gr_wait_frame     )() ;
FXEXTERN void (*_gr_advance_timers )() ;
FXEXTERN void (*_gr_draw_frame     )() ;
FXEXTERN void (*_gr_draw_screen    )(GRAPH * dest, int restore_type, int dump_type) ;
FXEXTERN void (*_keytab_init       )() ;
FXEXTERN void (*_keytab_free       )() ;
FXEXTERN int  (*_gr_key            )(int code) ;
FXEXTERN int  (*_gr_timer          )() ;

#define gr_init				(*_gr_init)
#define gr_set_fps			(*_gr_set_fps)
#define gr_wait_frame		(*_gr_wait_frame)
#define gr_advance_timers	(*_gr_advance_timers)
#define gr_draw_frame		(*_gr_draw_frame)
#define gr_draw_screen		(*_gr_draw_screen)
#define keytab_init			(*_keytab_init)
#define keytab_free			(*_keytab_free)
#define gr_key              (*_gr_key)
#define gr_timer            (*_gr_timer)

/* Objetos definidos por DLL */
/* ------------------------- */

FXEXTERN int  (*_gr_new_object	)(int z, int (*info)(void *, REGION *), void (*draw)(void *, REGION *), void * what);
FXEXTERN void (*_gr_hide_object	)(int id, int hidden);
FXEXTERN void (*_gr_destroy_object)(int id);

#define gr_new_object		(*_gr_new_object)
#define gr_hide_object		(*_gr_hide_object)
#define gr_destroy_object	(*_gr_destroy_object)

/* Paleta de colores */
/* ----------------- */

FXEXTERN SDL_Color   ** _vpalette ;
FXEXTERN SDL_Color   (* _palette)[] ;

FXEXTERN Uint16       * _colorequiv ;       /* Equivalencia paleta -> pantalla   */
FXEXTERN Uint16      (* _colorghost)[] ;       /* Deja un color a 50% de intensidad */

FXEXTERN GRAPH       ** _background_8bits ;
FXEXTERN int          * _background_8bits_used ;
FXEXTERN int          * _background_is_black ;

FXEXTERN Uint8        * _trans_table ;           /* Tabla de transparencias 8 bits        */
FXEXTERN int          * _trans_table_updated ;   /* 1 = La tabla es utilizable            */

FXEXTERN void  (*_gr_make_trans_table)() ;
#define gr_make_trans_table (*_gr_make_trans_table)

FXEXTERN int          * _palette_loaded ;        /* ùSe ha cargado ya la paleta inicial ? */
FXEXTERN int          * _palette_changed ;       /* Poner a 1 cuando se cambien colores   */
FXEXTERN int          * _fade_on ;               /* ùHay un fade activo?                  */
FXEXTERN int          * _fade_step ;             /* Si lo hay, posiciùn (0=off)           */

FXEXTERN PALETTE *  (*_gr_read_pal           )(file * file) ;
FXEXTERN PALETTE *  (*_gr_read_pal_with_gamma)(file * file) ;
FXEXTERN void       (*_gr_refresh_palette    )() ;
FXEXTERN void       (*_gr_fade_init          )(int r, int g, int b, int speed, int dir) ;
FXEXTERN void       (*_gr_fade_step          )() ;
FXEXTERN void       (*_gr_roll_palette       )(int color0, int num, int inc) ;
FXEXTERN int        (*_gr_find_nearest_color )(int r, int g, int b) ;
FXEXTERN void       (*_gr_set_rgb            )(int c, int r, int g, int b) ;
FXEXTERN int        (*_gr_rgb                )(int r, int g, int b) ;
FXEXTERN void       (*_gr_get_rgb            )(int color, int *r, int *g, int *b) ;
FXEXTERN void       (*_gr_set_colors         )(int color, int num, Uint8 * pal) ;
FXEXTERN void       (*_gr_get_colors         )(int color, int num, Uint8 * pal) ;

#define gr_read_pal				(*_gr_read_pal)
#define gr_read_pal_with_gamma  (*_gr_read_pal_with_gamma)
#define gr_refresh_palette		(*_gr_refresh_palette)
#define gr_fade_init			(*_gr_fade_init)
#define gr_fade_step			(*_gr_fade_step)
#define gr_roll_palette			(*_gr_roll_palette)
#define gr_find_nearest_color	(*_gr_find_nearest_color)
#define gr_set_rgb				(*_gr_set_rgb)
#define gr_rgb					(*_gr_rgb)
#define gr_get_rgb				(*_gr_get_rgb)
#define gr_set_colors			(*_gr_set_colors)
#define gr_get_colors			(*_gr_get_colors)

#define vpalette                     (* _vpalette                )
#define gpalette                     (* _palette                 )
#define colorequiv                   (  _colorequiv              )
#define colorghost                   (* _colorghost              )
#define background_8bits             (* _background_8bits        )
#define background_8bits_used        (* _background_8bits_used   )
#define background_is_black          (* _background_is_black     )
#define trans_table                  (  _trans_table             )
#define trans_table_updated          (* _trans_table_updated     )
#define palette_loaded               (* _palette_loaded          )
#define palette_changed              (* _palette_changed         )
#define fade_on                      (* _fade_on                 )
#define fade_step                    (* _fade_step               )

/* Blend ops */
/* --------- */

FXEXTERN Sint16 * (*_blend_create       )() ;
FXEXTERN void     (*_blend_free         )(Sint16 *blend) ;
FXEXTERN void     (*_blend_init         )(Sint16 * blend) ;
FXEXTERN void     (*_blend_translucency )(Sint16 * blend, float ammount) ;
FXEXTERN void     (*_blend_intensity    )(Sint16 * blend, float ammount) ;
FXEXTERN void     (*_blend_tint         )(Sint16 * blend, float ammount, Uint8 cr, Uint8 cg, Uint8 cb) ;
FXEXTERN void     (*_blend_swap         )(Sint16 * blend) ;
FXEXTERN void     (*_blend_assign       )(GRAPH * bitmap, Sint16 * blend) ;
FXEXTERN void     (*_blend_apply        )(GRAPH * bitmap, Sint16 * blend) ;
FXEXTERN void     (*_blend_grayscale    )(Sint16 * blend, int method) ;

#define blend_create		(*_blend_create)
#define blend_free			(*_blend_free)
#define blend_init			(*_blend_init)
#define blend_translucency	(*_blend_translucency)
#define blend_intensity		(*_blend_intensity)
#define blend_tint			(*_blend_tint)
#define blend_swap			(*_blend_swap)
#define blend_assign		(*_blend_assign)
#define blend_apply			(*_blend_apply)
#define blend_grayscale     (*_blend_grayscale)

/* Gestiùn de bitmaps y librerùas de grùficos */
/* ------------------------------------------ */

FXEXTERN int      (*_gr_load_map      )(const char * filename) ;
FXEXTERN int      (*_gr_load_png      )(const char * filename) ;
FXEXTERN int      (*_gr_load_pcx      )(const char * filename) ;
FXEXTERN int      (*_gr_load_fpg      )(const char * filename) ;
FXEXTERN int      (*_gr_load_pal      )(const char * filename) ;
FXEXTERN int	  (*_gr_save_pal      )(const char * filename) ;
FXEXTERN int	  (*_gr_save_map	  )(GRAPH * gr, const char * filename) ;
FXEXTERN int	  (*_gr_save_fpg	  )(GRAPH * gr, const char * filename) ;
FXEXTERN int	  (*_gr_save_png	  )(GRAPH * gr, const char * filename) ;
FXEXTERN int      (*_grlib_new        )() ;
FXEXTERN void     (*_grlib_destroy    )(int libid) ;
FXEXTERN int      (*_grlib_add_map    )(int libid, GRAPH * map) ;
FXEXTERN int      (*_grlib_unload_map )(int libid, int mapcode) ;
FXEXTERN GRAPH *  (*_bitmap_new       )(int code, int w, int h, int depth, int frames) ;
FXEXTERN GRAPH *  (*_bitmap_clone     )(GRAPH *) ;
FXEXTERN GRAPH *  (*_bitmap_new_syslib)(int w, int h, int depth,int frames) ;
FXEXTERN GRAPH *  (*_bitmap_get       )(int libid, int mapcode) ;
FXEXTERN void     (*_bitmap_destroy   )(GRAPH * map) ;
FXEXTERN void     (*_bitmap_add_cpoint)(GRAPH *map, int x, int y) ;
FXEXTERN void     (*_bitmap_set_cpoint)(GRAPH *map, int index, int x, int y) ;
FXEXTERN void     (*_bitmap_analize   )(GRAPH * bitmap) ;
FXEXTERN void     (*_bitmap_animate   )(GRAPH * bitmap) ;
FXEXTERN void     (*_bitmap_animate_to)(GRAPH * bitmap, int pos, int speed) ;

#define gr_load_map			(*_gr_load_map)
#define gr_load_png			(*_gr_load_png)
#define gr_load_pcx			(*_gr_load_pcx)
#define gr_load_fpg			(*_gr_load_fpg)
#define gr_load_pal			(*_gr_load_pal)
#define gr_save_pal         (*_gr_save_pal)
#define gr_save_map			(*_gr_save_map)
#define gr_save_fpg			(*_gr_save_fpg)
#define gr_save_png			(*_gr_save_png)
#define grlib_new			(*_grlib_new)
#define grlib_destroy		(*_grlib_destroy)
#define grlib_add_map		(*_grlib_add_map)
#define grlib_unload_map	(*_grlib_unload_map)
#define bitmap_new			(*_bitmap_new)
#define bitmap_clone		(*_bitmap_clone)
#define bitmap_new_syslib	(*_bitmap_new_syslib)
#define bitmap_get			(*_bitmap_get)
#define bitmap_destroy		(*_bitmap_destroy)
#define bitmap_add_cpoint	(*_bitmap_add_cpoint)
#define bitmap_set_cpoint	(*_bitmap_set_cpoint)
#define bitmap_analize		(*_bitmap_analize)
#define bitmap_animate		(*_bitmap_animate)
#define bitmap_animate_to	(*_bitmap_animate_to)

/* Regiones */
/* -------- */

FXEXTERN void     (*_region_define   )(int region, int x, int y, int width, int height) ;
FXEXTERN void     (*_region_union    )(REGION * a, REGION * b) ;
FXEXTERN int      (*_region_is_empty )(REGION * a) ;
FXEXTERN int      (*_region_is_out   )(REGION * a, REGION * b) ;
FXEXTERN REGION * (*_region_new      )(int x, int y, int width, int height);
FXEXTERN void     (*_region_destroy  )(REGION *);
FXEXTERN REGION * (*_region_get      )(int n);

#define region_define		(*_region_define)
#define region_union		(*_region_union)
#define region_is_empty		(*_region_is_empty)
#define region_is_out		(*_region_is_out)
#define region_new          (*_region_new)
#define region_destroy      (*_region_destroy)
#define region_get          (*_region_get)

/* Alto nivel */
/* ---------- */

FXEXTERN void     (*_draw_instance_at    )(INSTANCE * i, REGION * r, int x, int y) ;
FXEXTERN void     (*_draw_instance       )(void * i, REGION * clip) ;
FXEXTERN void     (*_instance_update_bbox)(INSTANCE * i) ;
FXEXTERN GRAPH  * (*_instance_graph      )(INSTANCE * i) ;
FXEXTERN void     (*_scroll_region       )(int nscroll, REGION * r) ;

#define draw_instance_at	 (*_draw_instance_at)
#define draw_instance   	 (*_draw_instance)
#define instance_update_bbox (*_instance_update_bbox)
#define instance_graph		 (*_instance_graph)
#define scroll_region		 (*_scroll_region)

/* Soporte DUMP_TYPE */
/* ----------------- */

FXEXTERN void	  (*_gr_mark_rect        )(int x, int y, int width, int height);
FXEXTERN void     (*_gr_mark_instance    )(INSTANCE * r);

#define gr_mark_rect         (*_gr_mark_rect)
#define gr_mark_instance	 (*_gr_mark_instance)

/* Textos */
/* ------ */

FXEXTERN int      (*_gr_load_bdf        )(const char * filename) ;

#define gr_load_bdf	(*_gr_load_bdf)

FXEXTERN int      (*_gr_font_load       )(char * filename) ;
FXEXTERN int      (*_gr_font_save       )(int fontid, const char * filename) ;
FXEXTERN int      (*_gr_font_new        )() ;
FXEXTERN int      (*_gr_font_newfrombitmap)(char * chardata, int width, int height, int options) ;
FXEXTERN int      (*_gr_font_systemfont )(char * chardata) ;
FXEXTERN void     (*_gr_font_destroy    )(int fontid) ;
FXEXTERN FONT *   (*_gr_font_get		   )(int id);

#define gr_font_load		(*_gr_font_load)
#define gr_font_save		(*_gr_font_save)
#define gr_font_new			(*_gr_font_new)
#define gr_font_newfrombitmap (*_gr_font_newfrombitmap)
#define gr_font_systemfont  (*_gr_font_systemfont)
#define gr_font_destroy		(*_gr_font_destroy)
#define gr_font_get			(*_gr_font_get)

FXEXTERN void     (*_gr_text_setcolor   )(int c) ;
FXEXTERN int      (*_gr_text_getcolor   )() ;
FXEXTERN int      (*_gr_text_new        )(int fontid, int x, int y, int centered, const char * text) ;
FXEXTERN int      (*_gr_text_new_var    )(int fontid, int x, int y, int centered, const void * var, int type) ;
FXEXTERN void     (*_gr_text_move       )(int textid, int x, int y) ;
FXEXTERN void     (*_gr_text_destroy    )(int textid) ;
FXEXTERN int      (*_gr_text_margintop  )(int fontid, const unsigned char * text) ;
FXEXTERN int      (*_gr_text_width      )(int fontid, const unsigned char * text) ;
FXEXTERN int      (*_gr_text_widthn     )(int fontid, const unsigned char * text, int n) ;
FXEXTERN int      (*_gr_text_height     )(int fontid, const unsigned char * text) ;
FXEXTERN int      (*_gr_text_put        )(GRAPH * dest, REGION * clip, int fontid, int x, int y, const unsigned char * text) ;
FXEXTERN GRAPH  * (*_gr_text_bitmap     )(int fontid, const char * text, int centered) ;

#define gr_text_setcolor		(*_gr_text_setcolor)
#define gr_text_getcolor		(*_gr_text_getcolor)
#define gr_text_new				(*_gr_text_new)
#define gr_text_new_var			(*_gr_text_new_var)
#define gr_text_move			(*_gr_text_move)
#define gr_text_destroy			(*_gr_text_destroy)
#define gr_text_margintop		(*_gr_text_margintop)
#define gr_text_width			(*_gr_text_width)
#define gr_text_height			(*_gr_text_height)
#define gr_text_put				(*_gr_text_put)
#define gr_text_bitmap          (*_gr_text_bitmap)
#define gr_text_widthn          (*_gr_text_widthn)

/* Bajo nivel */
/* ---------- */

/* Las funciones grùficas admiten dest=0 para referirse a la pantalla.
* Para poder usar esta funcionalidad, debe estar bloqueada antes */

FXEXTERN int  (*_gr_lock_screen   )() ;
FXEXTERN void (*_gr_unlock_screen )() ;

#define gr_lock_screen		(*_gr_lock_screen)
#define gr_unlock_screen	(*_gr_unlock_screen)

/* Primitivas grùficas */

FXEXTERN void (*_gr_clear     )(GRAPH * dest) ;
FXEXTERN void (*_gr_clear_as  )(GRAPH * dest, int color) ;
FXEXTERN void (*_gr_put_pixel )(GRAPH * dest, int x, int y, int color) ;
FXEXTERN int  (*_gr_get_pixel )(GRAPH * dest, int x, int y) ;

#define gr_clear		(*_gr_clear)
#define gr_clear_as		(*_gr_clear_as)
#define gr_get_pixel    (*_gr_get_pixel)
#define gr_put_pixel    (*_gr_put_pixel)

FXEXTERN void (*_gr_setcolor  )(int c) ;
FXEXTERN void (*_gr_setalpha  )(int a) ;
FXEXTERN void (*_gr_vline     )(GRAPH * dest, REGION * clip, int x, int y, int h) ;
FXEXTERN void (*_gr_hline     )(GRAPH * dest, REGION * clip, int x, int y, int w) ;
FXEXTERN void (*_gr_line      )(GRAPH * dest, REGION * clip, int x, int y, int w, int h) ;
FXEXTERN void (*_gr_box       )(GRAPH * dest, REGION * clip, int x, int y, int w, int h) ;
FXEXTERN void (*_gr_rectangle )(GRAPH * dest, REGION * clip, int x, int y, int w, int h) ;
FXEXTERN void (*_gr_circle    )(GRAPH * dest, REGION * clip, int x, int y, int r) ;
FXEXTERN void (*_gr_fcircle   )(GRAPH * dest, REGION * clip, int x, int y, int r) ;
FXEXTERN void (*_gr_bezier    )(GRAPH * dest, REGION * clip, int * params) ;

FXEXTERN int  (*_gr_drawing_new     )(DRAWING_OBJECT drawing, int z) ;
FXEXTERN void (*_gr_drawing_destroy )(int id) ;
FXEXTERN void (*_gr_drawing_move    )(int id, int x, int y) ;

#define gr_setcolor		(*_gr_setcolor)
#define gr_setalpha     (*_gr_setalpha)
#define gr_vline		(*_gr_vline)
#define gr_hline		(*_gr_hline)
#define gr_line			(*_gr_line)
#define gr_box			(*_gr_box)
#define gr_rectangle	(*_gr_rectangle)
#define gr_circle		(*_gr_circle)
#define gr_fcircle		(*_gr_fcircle)
#define gr_bezier       (*_gr_bezier)

#define gr_drawing_new     (*_gr_drawing_new)
#define gr_drawing_destroy (*_gr_drawing_destroy)
#define gr_drawing_move    (*_gr_drawing_toggle)

/* Bitmaps */

FXEXTERN void (*_gr_blit          )(GRAPH * dest, REGION * clip, int x, int y, int flags, GRAPH * gr) ;
FXEXTERN void (*_gr_get_bbox      )(REGION * dest, REGION * clip, int x, int y, int flags, int angle, int scalex, int scaley, GRAPH * gr) ;
FXEXTERN void (*_gr_rotated_blit  )(GRAPH * dest, REGION * clip, int scrx, int scry, int flags, int angle, int scalex, int scaley, GRAPH * gr) ;

#define gr_blit			(*_gr_blit)
#define gr_get_bbox		(*_gr_get_bbox)
#define gr_rotated_blit	(*_gr_rotated_blit)

FXEXTERN void (*_gr_convert16_ScreenTo565) (Uint16 * ptr, int len);
FXEXTERN void (*_gr_convert16_565ToScreen) (Uint16 * ptr, int len);
FXEXTERN void (*_gr_fade16) (GRAPH * graph, int r, int g, int b);

FXEXTERN Uint16 * (*_gr_alpha16)(int alpha);
FXEXTERN Uint8  * (*_gr_alpha8 )(int alpha);

#define gr_convert16_ScreenTo565 (*_gr_convert16_ScreenTo565)
#define gr_convert16_565ToScreen (*_gr_convert16_565ToScreen)
#define gr_fade16  (*_gr_fade16)
#define gr_alpha16 (*_gr_alpha16)
#define gr_alpha8  (*_gr_alpha8)

/* Scroll */
/* ------ */

FXEXTERN void (*_gr_scroll_start         )(int n, int fileid, int graphid, int backid, int region, int flags) ;
FXEXTERN void (*_gr_scroll_stop          )(int n) ;
FXEXTERN void (*_gr_scroll_draw          )(int n, int do_drawing, REGION * clipping) ;
FXEXTERN int  (*_gr_scroll_active        )(int n) ;
FXEXTERN int  (*_gr_scroll_is_fullscreen )() ;

#define gr_scroll_start			(*_gr_scroll_start)
#define gr_scroll_stop			(*_gr_scroll_stop)
#define gr_scroll_draw			(*_gr_scroll_draw)
#define gr_scroll_active		(*_gr_scroll_active)
#define gr_scroll_is_fullscreen (*_gr_scroll_is_fullscreen)

/* Modo 7 */
/* ------ */

FXEXTERN void (*_gr_mode7_start  )(int n, int fileid, int inid, int outid, int region, int inclination) ;
FXEXTERN void (*_gr_mode7_stop   )(int n) ;
FXEXTERN void (*_gr_mode7_draw   )(int n) ;
FXEXTERN int  (*_gr_mode7_active )(int n) ;

#define gr_mode7_start	(*_gr_mode7_start)
#define gr_mode7_stop	(*_gr_mode7_stop)
#define gr_mode7_draw	(*_gr_mode7_draw)
#define gr_mode7_active (*_gr_mode7_active)

/* Consola del sistema */
/* ------------------- */

FXEXTERN void (*_gr_sys_color   )(int cfg, int cbg) ;
FXEXTERN void (*_gr_sys_puts    )(GRAPH * map, int x, int y, Uint8 * str, int len) ;
FXEXTERN void (*_gr_sys_putchar )(GRAPH * map, int ox, int oy, Uint8 c) ;

#define gr_sys_color	(*_gr_sys_color)
#define gr_sys_puts		(*_gr_sys_puts)
#define gr_sys_putchar	(*_gr_sys_putchar)

FXEXTERN void (*_gr_con_printf  )(const char *fmt, ...) ;
FXEXTERN void (*_gr_con_putline )(char * text) ;
FXEXTERN void (*_gr_con_show    )(int doit) ;
FXEXTERN void (*_gr_con_draw    )() ;
FXEXTERN void (*_gr_con_getkey  )(int key, int sym) ;
FXEXTERN void (*_gr_con_scroll  )(int direction) ;
FXEXTERN void (*_gr_con_do      )(const char * command) ;

#define gr_con_printf	(*_gr_con_printf)
#define gr_con_putline	(*_gr_con_putline)
#define gr_con_show		(*_gr_con_show)
#define gr_con_draw		(*_gr_con_draw)
#define gr_con_getkey	(*_gr_con_getkey)
#define gr_con_scroll	(*_gr_con_scroll)
#define gr_con_do		(*_gr_con_do)

/* Profiler */
/* -------- */

FXEXTERN void (*_gprof_init   )();
FXEXTERN void (*_gprof_begin  )(const char * name);
FXEXTERN void (*_gprof_end    )(const char * name);
FXEXTERN void (*_gprof_frame  )();
FXEXTERN void (*_gprof_dump   )(const char * filename);
FXEXTERN void (*_gprof_reset  )();
FXEXTERN void (*_gprof_draw   )(GRAPH * dest);
FXEXTERN void (*_gprof_toggle )();

#define gprof_init		(*_gprof_init)
#define gprof_begin		(*_gprof_begin)
#define gprof_end		(*_gprof_end)
#define gprof_frame		(*_gprof_frame)
#define gprof_dump		(*_gprof_dump)
#define gprof_reset		(*_gprof_reset)
#define gprof_draw		(*_gprof_draw)
#define gprof_toggle	(*_gprof_toggle)

/*****************************************************
 * i_procdef.h
 *****************************************************/
FXEXTERN int *_local_strings ;
FXEXTERN int ** _localstr ;

#define local_strings   (* _local_strings )
#define localstr        (* _localstr      )

FXEXTERN PROCDEF **_mainproc ;
FXEXTERN PROCDEF *(*_procdef_get)(int n) ;

#define procdef_get (*_procdef_get)
#define mainproc    (*_mainproc)

FXEXTERN SYSPROC   * (*_sysproc_get )(int code) ;
FXEXTERN int         (*_sysproc_add )(char * name, char * paramtypes, int type, void * func) ;
FXEXTERN void        (*_sysproc_init)() ;

#define sysproc_get			(*_sysproc_get)
#define sysproc_add			(*_sysproc_add)
#define sysproc_init		(*_sysproc_init)

/*****************************************************
 * instance.h
 *****************************************************/

FXEXTERN void     **_globaldata ;
FXEXTERN void     **_localdata ;

FXEXTERN int       *_local_size ;

FXEXTERN INSTANCE **_first_instance ;
FXEXTERN INSTANCE **_last_instance ;

#define globaldata      (*_globaldata     )
#define localdata       (*_localdata      )
#define local_size      (*_local_size     )
#define first_instance  (*_first_instance )
#define last_instance   (*_last_instance  )

/*****************************************************
 * sound.h
 *****************************************************/

FXEXTERN void       (*_sound_init          )();
FXEXTERN void       (*_sound_close         )();

FXEXTERN int * _sound_active;

#define sound_init		(*_sound_init)
#define sound_close		(*_sound_close)
#define sound_active	(*_sound_active)

FXEXTERN int  (*_load_song)        (const char * filename);
FXEXTERN int  (*_play_song)        (int id , int loops);
FXEXTERN int  (*_unload_song)      (int id);
FXEXTERN int  (*_stop_song)        ();
FXEXTERN int  (*_pause_song)       ();
FXEXTERN int  (*_resume_song)      ();
FXEXTERN int  (*_is_playing_song)  ();
FXEXTERN int  (*_set_song_volume)  (int volume);

#define load_song		(*_load_song)
#define play_song		(*_play_song)
#define unload_song		(*_unload_song)
#define stop_song		(*_stop_song)
#define pause_song		(*_pause_song)
#define resume_song		(*_resume_song)
#define is_playing_song (*_is_playing_song)
#define set_song_volume (*_set_song_volume)

FXEXTERN int  (*_load_wav)			(const char * filename);
FXEXTERN int  (*_play_wav)			(int id , int loops, int channel);
FXEXTERN int  (*_unload_wav)		(int id);
FXEXTERN int  (*_stop_wav)			(int id);
FXEXTERN int  (*_pause_wav)			(int id);
FXEXTERN int  (*_resume_wav)		(int id);
FXEXTERN int  (*_is_playing_wav)	(int id);
FXEXTERN int  (*_set_wav_volume)	(int id,int volume);
FXEXTERN int  (*_set_channel_volume)(int id,int volume);
FXEXTERN int  (*_reserve_channels)  (int id);
FXEXTERN int  (*_set_panning)		(int canal,int left, int right);
FXEXTERN int  (*_set_position)		(int canal,int angle, int dist);
FXEXTERN int  (*_set_distance)		(int canal,int dist);
FXEXTERN int  (*_reverse_stereo)	(int canal,int flip);

#define load_wav			(*_load_wav)
#define play_wav			(*_play_wav)
#define unload_wav			(*_unload_wav)
#define stop_wav			(*_stop_wav)
#define pause_wav			(*_pause_wav)
#define resume_wav			(*_resume_wav)
#define is_playing_wav		(*_is_playing_wav)
#define set_wav_volume		(*_set_wav_volume)
#define set_channel_volume	(*_set_channel_volume)
#define reserve_channels	(*_reserve_channels)
#define set_panning			(*_set_panning)
#define set_position		(*_set_position)
#define set_distance		(*_set_distance)
#define reverse_stereo		(*_reverse_stereo)

FXEXTERN int  (*_fade_music_in)	(int id, int loops, int ms) ;
FXEXTERN int  (*_fade_music_off)(int ms) ;
FXEXTERN void (*_ini_musiccd)	() ;
FXEXTERN void (*_cd_play)		(int track, int continuous) ;
FXEXTERN void (*_cd_stop)		() ;
FXEXTERN int  (*_cd_playing)	() ;

#define fade_music_in	(*_fade_music_in)
#define fade_music_off	(*_fade_music_off)
#define ini_musiccd		(*_ini_musiccd)
#define cd_play			(*_cd_play)
#define cd_stop			(*_cd_stop)
#define cd_playing		(*_cd_playing)

/*****************************************************
 * instance.h
 *****************************************************/

FXEXTERN int * _must_exit ;
#define must_exit   (*_must_exit )

FXEXTERN int        (*_instance_getid       )() ;
FXEXTERN INSTANCE * (*_instance_get         )(int id) ;
FXEXTERN INSTANCE * (*_instance_getfather   )(INSTANCE * i) ;
FXEXTERN INSTANCE * (*_instance_getson      )(INSTANCE * i) ;
FXEXTERN INSTANCE * (*_instance_getbigbro   )(INSTANCE * i) ;
FXEXTERN INSTANCE * (*_instance_getsmallbro )(INSTANCE * i) ;
FXEXTERN INSTANCE * (*_instance_new         )(PROCDEF * proc, INSTANCE * father) ;
FXEXTERN INSTANCE * (*_instance_duplicate   )(INSTANCE * i) ;
FXEXTERN void       (*_instance_destroy     )(INSTANCE * r) ;
FXEXTERN void       (*_instance_posupdate   )(INSTANCE * i) ;
FXEXTERN int        (*_instance_poschanged  )(INSTANCE * i) ;
FXEXTERN void       (*_instance_dump        )(INSTANCE * father, int indent) ;
FXEXTERN void       (*_instance_dump_all    )() ;

#define instance_getid			(*_instance_getid)
#define instance_get			(*_instance_get)
#define instance_getfather		(*_instance_getfather)
#define instance_getson			(*_instance_getson)
#define instance_getbigbro		(*_instance_getbigbro)
#define instance_getsmallbro	(*_instance_getsmallbro)
#define instance_new			(*_instance_new)
#define instance_duplicate		(*_instance_duplicate)
#define instance_destroy		(*_instance_destroy)
#define instance_dump			(*_instance_dump)
#define instance_dump_all		(*_instance_dump_all)
#define instance_poschanged     (*_instance_poschanged)
#define instance_posupdate      (*_instance_posupdate)

/* Las siguientes funciones son el punto de entrada del intùrprete */
FXEXTERN int        (*_instance_go          )(INSTANCE * r) ;
FXEXTERN void       (*_instance_go_all      )() ;

#define instance_go		(*_instance_go)
#define instance_go_all (*_instance_go_all)

/*****************************************************
 * Otros
 *****************************************************/

FXEXTERN void (*_fnc_export) (const char * name, void * addr);

#define fnc_export	(*_fnc_export)

#ifndef WIN32
    #define FENIX_MainDLL       void
#else
    #define FENIX_MainDLL       void __stdcall
#endif

#define COMMON_PARAMS \
void *(*FENIX_import)(char *name),\
void  (*FENIX_export)(char * name, char * paramtypes, int type, void * func)

#define FENIX_DLLImport     \
	_gprof_init                 = FENIX_import ( "gprof_init" ); \
	_gprof_begin                = FENIX_import ( "gprof_begin" ); \
	_gprof_frame                = FENIX_import ( "gprof_frame" ); \
	_gprof_end                  = FENIX_import ( "gprof_end" ); \
	_gprof_dump                 = FENIX_import ( "gprof_dump" ); \
	_gprof_reset                = FENIX_import ( "gprof_reset" ); \
	_gprof_draw                 = FENIX_import ( "gprof_draw" ); \
	_gprof_toggle               = FENIX_import ( "gprof_toggle" ); \
    _file_open                  = FENIX_import ( "file_open" ); \
    _file_read                  = FENIX_import ( "file_read" ); \
    _file_write                 = FENIX_import ( "file_write" ); \
    _file_gets                  = FENIX_import ( "file_gets" ); \
    _file_puts                  = FENIX_import ( "file_puts" ); \
    _file_qgets                 = FENIX_import ( "file_qgets" ); \
    _file_qputs                 = FENIX_import ( "file_qputs" ); \
    _file_size                  = FENIX_import ( "file_size" ); \
    _file_pos                   = FENIX_import ( "file_pos" ); \
    _file_seek                  = FENIX_import ( "file_seek" ); \
    _file_addp                  = FENIX_import ( "file_addp" ); \
    _file_close                 = FENIX_import ( "file_close" ); \
    _file_exists                = FENIX_import ( "file_exists" ); \
    _file_add_xfile             = FENIX_import ( "file_add_xfile" ); \
    _file_eof                   = FENIX_import ( "file_eof" ); \
    _file_fp                    = FENIX_import ( "file_fp" ); \
    _string_init                = FENIX_import ( "string_init" ); \
    _string_get                 = FENIX_import ( "string_get" ); \
    _string_dump                = FENIX_import ( "string_dump" ); \
    _string_load                = FENIX_import ( "string_load" ); \
    _string_new                 = FENIX_import ( "string_new" ); \
    _string_newa                = FENIX_import ( "string_newa" ); \
    _string_use                 = FENIX_import ( "string_use" ); \
    _string_discard             = FENIX_import ( "string_discard" ); \
    _string_add                 = FENIX_import ( "string_add" ); \
    _string_itoa                = FENIX_import ( "string_itoa" ); \
    _string_ftoa                = FENIX_import ( "string_ftoa" ); \
    _string_ptoa                = FENIX_import ( "string_ptoa" ); \
    _string_comp                = FENIX_import ( "string_comp" ); \
    _string_casecmp             = FENIX_import ( "string_casecmp" ); \
    _string_char                = FENIX_import ( "string_char" ); \
    _string_substr              = FENIX_import ( "string_substr" ); \
    _string_find                = FENIX_import ( "string_find" ); \
    _string_ucase               = FENIX_import ( "string_ucase" ); \
    _string_lcase               = FENIX_import ( "string_lcase" ); \
    _string_strip               = FENIX_import ( "string_strip" ); \
    _string_pad                 = FENIX_import ( "string_pad" ); \
    _string_format              = FENIX_import ( "string_format" ); \
    _string_coalesce            = FENIX_import ( "string_coalesce" ); \
    _string_concat              = FENIX_import ( "string_concat" ); \
    _dos_chars                  = FENIX_import ( "dos_chars" ); \
    _c_type                     = FENIX_import ( "c_type" ); \
    _c_upper                    = FENIX_import ( "c_upper" ); \
    _c_lower                    = FENIX_import ( "c_lower" ); \
    _convert                    = FENIX_import ( "convert" ); \
    _init_c_type                = FENIX_import ( "init_c_type" ); \
    _dos_to_win                 = FENIX_import ( "dos_to_win" ); \
    _win_to_dos                 = FENIX_import ( "win_to_dos" ); \
    _flic_open                  = FENIX_import ( "flic_open" ); \
    _flic_destroy               = FENIX_import ( "flic_destroy" ); \
    _flic_do_frame              = FENIX_import ( "flic_do_frame" ); \
    _flic_reset                 = FENIX_import ( "flic_reset" ); \
    _current_fli                = FENIX_import ( "current_fli" ); \
    _current_fli_x              = FENIX_import ( "current_fli_x" ); \
    _current_fli_y              = FENIX_import ( "current_fli_y" ); \
    _debug                      = FENIX_import ( "debug" ); \
    _fxi                        = FENIX_import ( "fxi" ); \
    _enable_16bits              = FENIX_import ( "enable_16bits" ); \
    _enable_filtering           = FENIX_import ( "enable_filtering" ); \
    _report_string              = FENIX_import ( "report_string" ); \
    _report_graphics            = FENIX_import ( "report_graphics" ); \
    _gr_error                   = FENIX_import ( "gr_error" ); \
    _do_exit                    = FENIX_import ( "do_exit" ); \
    _dcb_load                   = FENIX_import ( "dcb_load" ); \
    _getid                      = FENIX_import ( "getid" ); \
    _path_find                  = FENIX_import ( "path_find" ); \
    _path_get                   = FENIX_import ( "path_get" ); \
    _path_set_wall              = FENIX_import ( "path_set_wall" ); \
    _scr_initialized            = FENIX_import ( "scr_initialized" ); \
    _regions                    = FENIX_import ( "regions" ); \
    _background                 = FENIX_import ( "background" ); \
    _scrbitmap                  = FENIX_import ( "scrbitmap" ); \
    _syslib                     = FENIX_import ( "syslib" ); \
    _icono                      = FENIX_import ( "icono" ); \
    _last_frame_ms              = FENIX_import ( "last_frame_ms" ); \
    _apptitle                   = FENIX_import ( "apptitle" ); \
    _key_table                  = FENIX_import ( "key_table" ); \
    _keystate                   = FENIX_import ( "keystate" ); \
    _keytab_initialized         = FENIX_import ( "keytab_initialized" ); \
	_gr_key                     = FENIX_import ( "gr_key" ); \
	_gr_timer                   = FENIX_import ( "gr_timer" ); \
    _scr_width                  = FENIX_import ( "scr_width" ); \
    _scr_height                 = FENIX_import ( "scr_height" ); \
    _full_screen                = FENIX_import ( "full_screen" ); \
    _double_buffer              = FENIX_import ( "double_buffer" ); \
    _gr_init                    = FENIX_import ( "gr_init" ); \
    _gr_set_fps                 = FENIX_import ( "gr_set_fps" ); \
    _gr_wait_frame              = FENIX_import ( "gr_wait_frame" ); \
    _gr_advance_timers          = FENIX_import ( "gr_advance_timers" ); \
    _gr_draw_frame              = FENIX_import ( "gr_draw_frame" ); \
	_gr_draw_screen             = FENIX_import ( "gr_draw_screen" ); \
    _gr_new_object              = FENIX_import ( "gr_new_object" ); \
    _gr_hide_object             = FENIX_import ( "gr_hide_object" ); \
    _gr_destroy_object          = FENIX_import ( "gr_destroy_object" ); \
    _keytab_init                = FENIX_import ( "keytab_init" ); \
    _keytab_free                = FENIX_import ( "keytab_free" ); \
    _vpalette                   = FENIX_import ( "vpalette" ); \
    _palette                    = FENIX_import ( "palette" ); \
    _colorequiv                 = FENIX_import ( "colorequiv" ); \
    _colorghost                 = FENIX_import ( "colorghost" ); \
    _background_8bits           = FENIX_import ( "background_8bits" ); \
    _background_8bits_used      = FENIX_import ( "background_8bits_used" ); \
    _background_is_black        = FENIX_import ( "background_is_black" ); \
    _trans_table                = FENIX_import ( "trans_table" ); \
    _trans_table_updated        = FENIX_import ( "trans_table_updated" ); \
    _gr_make_trans_table        = FENIX_import ( "gr_make_trans_table" ); \
    _palette_loaded             = FENIX_import ( "palette_loaded" ); \
    _palette_changed            = FENIX_import ( "palette_changed" ); \
    _fade_on                    = FENIX_import ( "fade_on" ); \
    _fade_step                  = FENIX_import ( "fade_step" ); \
    _gr_read_pal                = FENIX_import ( "gr_read_pal" ); \
    _gr_refresh_palette         = FENIX_import ( "gr_refresh_palette" ); \
    _gr_fade_init               = FENIX_import ( "gr_fade_init" ); \
    _gr_fade_step               = FENIX_import ( "gr_fade_step" ); \
    _gr_roll_palette            = FENIX_import ( "gr_roll_palette" ); \
    _gr_find_nearest_color      = FENIX_import ( "gr_find_nearest_color" ); \
    _gr_set_rgb                 = FENIX_import ( "gr_set_rgb" ); \
    _gr_rgb                     = FENIX_import ( "gr_rgb" ); \
    _gr_get_rgb                 = FENIX_import ( "gr_get_rgb" ); \
    _gr_set_colors              = FENIX_import ( "gr_set_colors" ); \
    _gr_get_colors              = FENIX_import ( "gr_get_colors" ); \
    _blend_create               = FENIX_import ( "blend_create" ); \
    _blend_free                 = FENIX_import ( "blend_free" ); \
    _blend_init                 = FENIX_import ( "blend_init" ); \
    _blend_translucency         = FENIX_import ( "blend_translucency" ); \
    _blend_intensity            = FENIX_import ( "blend_intensity" ); \
    _blend_tint                 = FENIX_import ( "blend_tint" ); \
    _blend_swap                 = FENIX_import ( "blend_swap" ); \
    _blend_assign               = FENIX_import ( "blend_assign" ); \
    _blend_apply                = FENIX_import ( "blend_apply" ); \
    _blend_grayscale            = FENIX_import ( "blend_grayscale" ); \
    _gr_load_map                = FENIX_import ( "gr_load_map" ); \
    _gr_load_png                = FENIX_import ( "gr_load_png" ); \
    _gr_load_pcx                = FENIX_import ( "gr_load_pcx" ); \
    _gr_load_fpg                = FENIX_import ( "gr_load_fpg" ); \
    _gr_load_pal                = FENIX_import ( "gr_load_pal" ); \
	_gr_save_pal                = FENIX_import ( "gr_save_pal" ); \
    _gr_save_map                = FENIX_import ( "gr_save_map" ); \
    _gr_save_fpg                = FENIX_import ( "gr_save_fpg" ); \
    _gr_save_png                = FENIX_import ( "gr_save_png" ); \
    _grlib_new                  = FENIX_import ( "grlib_new" ); \
    _grlib_destroy              = FENIX_import ( "grlib_destroy" ); \
    _grlib_add_map              = FENIX_import ( "grlib_add_map" ); \
    _grlib_unload_map           = FENIX_import ( "grlib_unload_map" ); \
    _bitmap_new                 = FENIX_import ( "bitmap_new" ); \
    _bitmap_clone               = FENIX_import ( "bitmap_clone" ); \
    _bitmap_new_syslib          = FENIX_import ( "bitmap_new_syslib" ); \
    _bitmap_get                 = FENIX_import ( "bitmap_get" ); \
    _bitmap_destroy             = FENIX_import ( "bitmap_destroy" ); \
    _bitmap_add_cpoint          = FENIX_import ( "bitmap_add_cpoint" ); \
    _bitmap_set_cpoint          = FENIX_import ( "bitmap_set_cpoint" ); \
    _bitmap_analize             = FENIX_import ( "bitmap_analize" ); \
    _bitmap_animate             = FENIX_import ( "bitmap_animate" ); \
    _bitmap_animate_to          = FENIX_import ( "bitmap_animate_to" ); \
    _region_define              = FENIX_import ( "region_define" ); \
    _region_union               = FENIX_import ( "region_union" ); \
    _region_is_empty            = FENIX_import ( "region_is_empty" ); \
    _region_is_out              = FENIX_import ( "region_is_out" ); \
    _region_new                 = FENIX_import ( "region_new" ); \
    _region_destroy             = FENIX_import ( "region_destroy" ); \
    _region_get                 = FENIX_import ( "region_get" ); \
    _draw_instance_at           = FENIX_import ( "draw_instance_at" ); \
    _draw_instance              = FENIX_import ( "draw_instance" ); \
    _instance_update_bbox       = FENIX_import ( "instance_update_bbox" ); \
    _instance_graph             = FENIX_import ( "instance_graph" ); \
    _scroll_region              = FENIX_import ( "scroll_region" ); \
    _gr_font_get                = FENIX_import ( "gr_font_get" ); \
    _gr_font_load               = FENIX_import ( "gr_font_load" ); \
    _gr_load_bdf                = FENIX_import ( "gr_load_bdf" ); \
    _gr_font_systemfont         = FENIX_import ( "gr_font_systemfont" ); \
    _gr_font_destroy            = FENIX_import ( "gr_font_destroy" ); \
    _gr_font_save               = FENIX_import ( "gr_font_save" ); \
    _gr_font_new                = FENIX_import ( "gr_font_new" ); \
    _gr_font_newfrombitmap      = FENIX_import ( "gr_font_newfrombitmap" ); \
    _gr_text_setcolor           = FENIX_import ( "gr_text_setcolor" ); \
	_gr_text_getcolor           = FENIX_import ( "gr_text_getcolor" ); \
    _gr_text_new                = FENIX_import ( "gr_text_new" ); \
    _gr_text_new_var            = FENIX_import ( "gr_text_new_var" ); \
    _gr_text_move               = FENIX_import ( "gr_text_move" ); \
    _gr_text_destroy            = FENIX_import ( "gr_text_destroy" ); \
    _gr_text_margintop          = FENIX_import ( "gr_text_margintop" ); \
    _gr_text_width              = FENIX_import ( "gr_text_width" ); \
    _gr_text_widthn             = FENIX_import ( "gr_text_widthn" ); \
    _gr_text_height             = FENIX_import ( "gr_text_height" ); \
    _gr_text_put                = FENIX_import ( "gr_text_put" ); \
    _gr_text_bitmap             = FENIX_import ( "gr_text_bitmap" ); \
    _gr_lock_screen             = FENIX_import ( "gr_lock_screen" ); \
    _gr_unlock_screen           = FENIX_import ( "gr_unlock_screen" ); \
    _gr_clear                   = FENIX_import ( "gr_clear" ); \
    _gr_clear_as                = FENIX_import ( "gr_clear_as" ); \
    _gr_put_pixel               = FENIX_import ( "gr_put_pixel" ); \
    _gr_get_pixel               = FENIX_import ( "gr_get_pixel" ); \
    _gr_setcolor                = FENIX_import ( "gr_setcolor" ); \
    _gr_vline                   = FENIX_import ( "gr_vline" ); \
    _gr_hline                   = FENIX_import ( "gr_hline" ); \
    _gr_line                    = FENIX_import ( "gr_line" ); \
    _gr_box                     = FENIX_import ( "gr_box" ); \
    _gr_rectangle               = FENIX_import ( "gr_rectangle" ); \
    _gr_circle                  = FENIX_import ( "gr_circle" ); \
    _gr_fcircle                 = FENIX_import ( "gr_fcircle" ); \
    _gr_blit                    = FENIX_import ( "gr_blit" ); \
    _gr_get_bbox                = FENIX_import ( "gr_get_bbox" ); \
    _gr_rotated_blit            = FENIX_import ( "gr_rotated_blit" ); \
    _gr_scroll_start            = FENIX_import ( "gr_scroll_start" ); \
    _gr_scroll_stop             = FENIX_import ( "gr_scroll_stop" ); \
    _gr_scroll_draw             = FENIX_import ( "gr_scroll_draw" ); \
    _gr_scroll_active           = FENIX_import ( "gr_scroll_active" ); \
    _gr_scroll_is_fullscreen    = FENIX_import ( "gr_scroll_is_fullscreen" ); \
    _gr_mode7_start             = FENIX_import ( "gr_mode7_start" ); \
    _gr_mode7_stop              = FENIX_import ( "gr_mode7_stop" ); \
    _gr_mode7_draw              = FENIX_import ( "gr_mode7_draw" ); \
    _gr_mode7_active            = FENIX_import ( "gr_mode7_active" ); \
    _gr_sys_color               = FENIX_import ( "gr_sys_color" ); \
    _gr_sys_puts                = FENIX_import ( "gr_sys_puts" ); \
    _gr_sys_putchar             = FENIX_import ( "gr_sys_putchar" ); \
    _gr_con_printf              = FENIX_import ( "gr_con_printf" ); \
    _gr_con_putline             = FENIX_import ( "gr_con_putline" ); \
    _gr_con_show                = FENIX_import ( "gr_con_show" ); \
    _gr_con_draw                = FENIX_import ( "gr_con_draw" ); \
    _gr_con_getkey              = FENIX_import ( "gr_con_getkey" ); \
    _gr_con_scroll              = FENIX_import ( "gr_con_scroll" ); \
    _gr_con_do                  = FENIX_import ( "gr_con_do" ); \
    _gr_convert16_565ToScreen   = FENIX_import ( "gr_convert16_565ToScreen" ); \
    _gr_convert16_ScreenTo565   = FENIX_import ( "gr_convert16_ScreenTo565" ); \
	_gr_fade16                  = FENIX_import ( "gr_fade16" ); \
    _local_strings              = FENIX_import ( "local_strings" ); \
    _localstr                   = FENIX_import ( "localstr" ); \
    _mainproc                   = FENIX_import ( "mainproc" ); \
    _procdef_get                = FENIX_import ( "procdef_get" ); \
    _sysproc_get                = FENIX_import ( "sysproc_get" ); \
    _sysproc_add                = FENIX_import ( "sysproc_add" ); \
    _sysproc_init               = FENIX_import ( "sysproc_init" ); \
    _globaldata                 = FENIX_import ( "globaldata" ); \
    _localdata                  = FENIX_import ( "localdata" ); \
    _local_size                 = FENIX_import ( "local_size" ); \
    _first_instance             = FENIX_import ( "first_instance" ); \
    _last_instance              = FENIX_import ( "last_instance" ); \
    _must_exit                  = FENIX_import ( "must_exit" ); \
    _instance_getid             = FENIX_import ( "instance_getid" ); \
    _instance_get               = FENIX_import ( "instance_get" ); \
    _instance_getfather         = FENIX_import ( "instance_getfather" ); \
    _instance_getson            = FENIX_import ( "instance_getson" ); \
    _instance_getbigbro         = FENIX_import ( "instance_getbigbro" ); \
    _instance_getsmallbro       = FENIX_import ( "instance_getsmallbro" ); \
    _instance_new               = FENIX_import ( "instance_new" ); \
    _instance_duplicate         = FENIX_import ( "instance_duplicate" ); \
    _instance_destroy           = FENIX_import ( "instance_destroy" ); \
    _instance_dump              = FENIX_import ( "instance_dump" ); \
    _instance_dump_all          = FENIX_import ( "instance_dump_all" ); \
    _instance_go                = FENIX_import ( "instance_go" ); \
    _instance_go_all            = FENIX_import ( "instance_go_all" ); \
    _instance_posupdate         = FENIX_import ( "instance_posupdate" ); \
    _instance_poschanged        = FENIX_import ( "instance_poschanged" ); \
    _sound_active               = FENIX_import ( "sound_active" ); \
    _sound_init                 = FENIX_import ( "sound_init" ); \
    _sound_close                = FENIX_import ( "sound_close" ); \
    _load_song                  = FENIX_import ( "load_song" ); \
    _play_song                  = FENIX_import ( "play_song" ); \
    _unload_song                = FENIX_import ( "unload_song" ); \
    _stop_song                  = FENIX_import ( "stop_song" ); \
    _pause_song                 = FENIX_import ( "pause_song" ); \
    _resume_song                = FENIX_import ( "resume_song" ); \
    _is_playing_song            = FENIX_import ( "is_playing_song" ); \
    _set_song_volume            = FENIX_import ( "set_song_volume" ); \
    _load_wav                   = FENIX_import ( "load_wav" ); \
    _play_wav                   = FENIX_import ( "play_wav" ); \
    _unload_wav                 = FENIX_import ( "unload_wav" ); \
    _stop_wav                   = FENIX_import ( "stop_wav" ); \
    _pause_wav                  = FENIX_import ( "pause_wav" ); \
    _resume_wav                 = FENIX_import ( "resume_wav" ); \
    _is_playing_wav             = FENIX_import ( "is_playing_wav" ); \
    _set_wav_volume             = FENIX_import ( "set_wav_volume" ); \
	_set_channel_volume         = FENIX_import ( "set_channel_volume" ); \
	_reserve_channels           = FENIX_import ( "reserve_channels" ); \
    _ini_musiccd                = FENIX_import ( "ini_musiccd" ); \
    _cd_play                    = FENIX_import ( "cd_play" ); \
    _cd_stop                    = FENIX_import ( "cd_stop" ); \
    _cd_playing                 = FENIX_import ( "cd_playing" ); \
    _fnc_export                 = FENIX_import ( "fnc_export" ); \
	_screen                     = FENIX_import ( "screen" ); \
	_gr_font_get                = FENIX_import ( "gr_font_get" ); \
	_gr_alpha8                  = FENIX_import ( "gr_alpha8" ); \
	_gr_alpha16                 = FENIX_import ( "gr_alpha16" ); \
	_syscolor16                 = FENIX_import ( "syscolor16" ); \
	_fntcolor16                 = FENIX_import ( "fntcolor16" ); \
	_syscolor8                  = FENIX_import ( "syscolor8" ); \
	_fntcolor8                  = FENIX_import ( "fntcolor8" ); \
	_frame_count                = FENIX_import ( "frame_count" ); \
	_current_time               = FENIX_import ( "current_time" ); \
    _drawing_stipple            = FENIX_import ( "drawing_stipple" ); \
    _gr_setalpha                = FENIX_import ( "gr_setalpha" ); \
	_gr_bezier                  = FENIX_import ( "gr_bezier" ); \
    _gr_drawing_new             = FENIX_import ( "gr_drawing_new" ); \
	_gr_drawing_destroy         = FENIX_import ( "gr_drawing_destroy" ); \
	_gr_drawing_move            = FENIX_import ( "gr_drawing_move" ); \
	_gr_fill_nearest_table      = FENIX_import ( "gr_fill_nearest_table" ); \
	_gr_mark_rect               = FENIX_import ( "gr_mark_rect" ); \
	_gr_mark_instance           = FENIX_import ( "gr_mark_instance" ); \
	_nearest_table              = FENIX_import ( "nearest_table" );

#ifdef __cplusplus
}
#endif

/* Keyboard codes */
#define KEY_0			 11
#define KEY_1			  2
#define KEY_2			  3
#define KEY_3			  4
#define KEY_4			  5
#define KEY_5			  6
#define KEY_6			  7
#define KEY_7			  8
#define KEY_8			  9
#define KEY_9			 10
#define KEY_A			 30
#define KEY_B			 48
#define KEY_BACKSLASH	 43
#define KEY_BACKSPACE	 14
#define KEY_C			 46
#define KEY_CAPSLOCK	 58
#define KEY_CARET		 40
#define KEY_COMMA		 51
#define KEY_D			 32
#define KEY_DELETE		 83
#define KEY_DOWN		 80
#define KEY_E			 18
#define KEY_END			 79
#define KEY_EQUALS		 90
#define KEY_ESCAPE		  1
#define KEY_F10			 68
#define KEY_F11			 87
#define KEY_F12			 88
#define KEY_F1			 59
#define KEY_F2			 60
#define KEY_F			 33
#define KEY_F3			 61
#define KEY_F4			 62
#define KEY_F5			 63
#define KEY_F6			 64
#define KEY_F7			 65
#define KEY_F8			 66
#define KEY_F9			 67
#define KEY_G			 34
#define KEY_GREATER		 91
#define KEY_H			 35
#define KEY_HOME		 71
#define KEY_I			 23
#define KEY_INSERT		 82
#define KEY_J			 36
#define KEY_K			 37
#define KEY_KP0			 11
#define KEY_KP1			  2
#define KEY_KP2			  3
#define KEY_KP3			  4
#define KEY_KP4			  5
#define KEY_KP5			  6
#define KEY_KP6			  7
#define KEY_KP7			  8
#define KEY_KP8			  9
#define KEY_KP9			 10
#define KEY_KP_DIVIDE	 53
#define KEY_KP_ENTER	 28
#define KEY_KP_MINUS	 74
#define KEY_KP_MULTIPLY	 55
#define KEY_KP_PERIOD	 52
#define KEY_KP_PLUS		 78
#define KEY_L			 38
#define KEY_LEFT		 75
#define KEY_LEFTBRACKET  26
#define KEY_LESS		 89
#define KEY_LMETA		 56
#define KEY_LSHIFT		 42
#define KEY_M			 50
#define KEY_MINUS		 12
#define KEY_N			 49
#define KEY_NUMLOCK		 69
#define KEY_O			 24
#define KEY_P			 25
#define KEY_PAGEDOWN	 81
#define KEY_PAGEUP		 73
#define KEY_PERIOD		 52
#define KEY_PLUS		 13
#define KEY_Q			 16
#define KEY_QUESTION	 92
#define KEY_QUOTE		 41
#define KEY_R			 19
#define KEY_RETURN		 28
#define KEY_RIGHT		 77
#define KEY_RIGHTBRACKET 27
#define KEY_RMETA		 56
#define KEY_RSHIFT		 54
#define KEY_S			 31
#define KEY_SCROLLOCK	 70
#define KEY_SEMICOLON	 39
#define KEY_SLASH		 53
#define KEY_SPACE		 57
#define KEY_T			 20
#define KEY_TAB			 15
#define KEY_U			 22
#define KEY_UP			 72
#define KEY_V			 47
#define KEY_W			 17
#define KEY_X			 45
#define KEY_Y			 21
#define KEY_Z			 44
#define KEY_RALT		 93
#define KEY_RCTRL		 94
#define KEY_LALT		 95
#define KEY_LCTRL		 96
#define KEY_MENU		 97

#endif
