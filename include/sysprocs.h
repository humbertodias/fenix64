/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.82
 *  Last stable release   :
 *  Project documentation : http://fenix.sourceforge.net
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
 * FILE        : sysprocs.h
 * DESCRIPTION : Defines FENIX language function SPECS and handlers
 *
 * HISTORY     : 0.73 - ADDED 0xEF MEMORY_TOTAL
 *               0.73 - ADDED 0xF7 MEMORY_FREE
 *               0.76 - REPLACED OLD SOUND LIB (0x130 - 0x13F)
 *               0.80 - ADDED 1 LAST PARAM TO MAP_BLOCK_COPY
 *               0.80 - CHANGED MOD FUNCS TO SONG FUNCS (0x130 - 0x137)
 *               0.82 - ADDED 0x77 SAVE_MAP
 *
 */

/*
 *  IN FXI: SYSMACRO returns a function pointer
 *  IN FXC: Already defined to return 0
 */

#ifndef SYSMACRO
#define SYSMACRO(a) a
#endif

#define MAX_SYSPROCS 2048

SYSPROC sysprocs[MAX_SYSPROCS] =
{
    /* Depurado */
    { 0x00, "SAY"                   , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_say) },
    /* Matemáticas */
    { 0x10, "RAND"                  , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_rand) },
    { 0x11, "RAND_SEED"             , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_rand_seed) },
    { 0x12, "ABS"                   , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_abs) },
    { 0x13, "POW"                   , "FF"    , TYPE_FLOAT  , 2 , SYSMACRO(fxi_pow) },
    { 0x14, "FGET_ANGLE"            , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_fget_angle) },
    { 0x15, "FGET_DIST"             , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_fget_dist) },
    { 0x16, "NEAR_ANGLE"            , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_near_angle) },
    { 0x17, "ADVANCE"               , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_advance) },
    { 0x18, "XADVANCE"              , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_xadvance) },
    { 0x19, "SQRT"                  , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_sqrt) },
    { 0x20, "COS"                   , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_cos) },
    { 0x21, "SIN"                   , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_sin) },
    { 0x22, "TAN"                   , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_tan) },
    { 0x23, "ACOS"                  , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_acos) },
    { 0x24, "ASIN"                  , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_asin) },
    { 0x25, "ATAN"                  , "F"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_atan) },
    /* Interacción entre procesos */
    { 0x30, "GET_ANGLE"             , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_get_angle) },
    { 0x31, "GET_DIST"              , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_get_dist) },
    { 0x32, "GET_DISTX"             , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_get_distx) },
    { 0x33, "GET_DISTY"             , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_get_disty) },
    { 0x34, "GET_ID"                , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_get_id) },
    { 0x35, "COLLISION"             , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_collision) },
    { 0x36, "SIGNAL"                , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_signal) },
    { 0x37, "LET_ME_ALONE"          , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_let_me_alone) },
    { 0x38, "EXIT"                  , "SI"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_exit) },
    { 0x39, "EXIT"                  , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_exit_1) },
    { 0x3A, "EXIT"                  , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_exit_0) },
    /* Entrada/Salida */
    { 0x50, "KEY"                   , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_key) },
    { 0x51, "GET_JOY_BUTTON"        , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_get_joy_button) },
    { 0x52, "GET_JOY_POSITION"      , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_get_joy_position) },
    { 0x53, "SELECT_JOY"            , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_select_joy) },
    /* Inicialización y carga */
    { 0x60, "SET_MODE"              , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_set_mode) },
    { 0x61, "SET_FPS"               , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_set_fps) },
    { 0x62, "LOAD_MAP"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_map) },
    { 0x63, "LOAD_FPG"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_fpg) },
    { 0x64, "UNLOAD_MAP"            , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_unload_map) },
    { 0x65, "UNLOAD_FPG"            , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_unload_fpg) },
    { 0x66, "GRAPHIC_SET"           , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_graphic_set) },
    { 0x67, "GRAPHIC_INFO"          , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_graphic_info) },
    /* Puntos de control */
    { 0x68, "GET_POINT"             , "IIIPP" , TYPE_DWORD  , 5 , SYSMACRO(fxi_get_point) },
    { 0x69, "GET_REAL_POINT"        , "IPP"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_get_real_point) },
    { 0x6A, "SET_POINT"             , "IIIII" , TYPE_DWORD  , 5 , SYSMACRO(fxi_set_point) },
    { 0x6B, "SET_CENTER"            , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_set_center) },
    /* Regiones */
    { 0x6D, "DEFINE_REGION"         , "IIIII" , TYPE_DWORD  , 5 , SYSMACRO(fxi_define_region) },
    { 0x6E, "OUT_REGION"            , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_out_region) },
    /* Fondo de pantalla */
    { 0x70, "PUT"                   , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_put) },
    { 0x71, "XPUT"                  , "IIIIIIII",TYPE_DWORD , 8 , SYSMACRO(fxi_xput) },
    { 0x72, "PUT_PIXEL"             , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_put_pixel) },
    { 0x73, "PUT_SCREEN"            , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_put_screen) },
    { 0x74, "CLEAR_SCREEN"          , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_clear_screen) },
    { 0x75, "GET_PIXEL"             , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_get_pixel) },
    /* Bitmaps */
    { 0x77, "SAVE_MAP"              , "IIS"  , TYPE_DWORD   , 3 , SYSMACRO(fxi_save_map) } ,
    { 0x78, "MAP_BLOCK_COPY"        , "IIIIIIIIII",TYPE_DWORD, 10 , SYSMACRO(fxi_map_block_copy) },
    { 0x79, "MAP_GET_PIXEL"         , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_map_get_pixel) },
    { 0x7A, "MAP_PUT"               , "IIIII" , TYPE_DWORD  , 5 , SYSMACRO(fxi_map_put) },
    { 0x7B, "MAP_XPUT"              , "IIIIIIII",TYPE_DWORD , 8 , SYSMACRO(fxi_map_xput) },
    { 0x7C, "MAP_PUT_PIXEL"         , "IIIII" , TYPE_DWORD  , 5 , SYSMACRO(fxi_map_put_pixel) },
    { 0x7D, "NEW_MAP"               , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_new_map) },
    { 0x7E, "MAP_CLEAR"             , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_map_clear) },
    { 0x7F, "MAP_CLONE"             , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_map_clone) },
    { 0x80, "MAP_NAME"              , "II"    , TYPE_STRING , 2 , SYSMACRO(fxi_map_name) },
    { 0x81, "MAP_SET_NAME"          , "IIS"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_map_set_name) },
    { 0x82, "MAP_EXISTS"            , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_map_exists) },
    { 0x83, "MAP_XPUTNP"            , "IIIIIIIIIII",TYPE_DWORD , 10 , SYSMACRO(fxi_map_xputnp) },
    { 0x84, "FPG_EXISTS"            , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_fpg_exists) },
    /* FPG */
    { 0x88, "FPG_ADD"               , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_fpg_add) },
    { 0x8A, "SAVE_FPG"              , "IS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_fpg_save) },
    { 0x8B, "NEW_FPG"               , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_fpg_new) },
    { 0x8B, "FPG_NEW"               , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_fpg_new) },
    /* Textos */
    { 0x90, "LOAD_FNT"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_fnt) },
    { 0x91, "UNLOAD_FNT"            , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_unload_fnt) },
    { 0x92, "WRITE"                 , "IIIIS" , TYPE_DWORD  , 5 , SYSMACRO(fxi_write) },
    { 0x93, "WRITE_INT"             , "IIIIP" , TYPE_DWORD  , 5 , SYSMACRO(fxi_write_int) },
    { 0x94, "MOVE_TEXT"             , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_move_text) },
    { 0x95, "DELETE_TEXT"           , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_delete_text) },
    { 0x96, "WRITE_IN_MAP"          , "ISI"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_write_in_map) },
    { 0x97, "TEXT_WIDTH"            , "IS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_text_width) },
    { 0x98, "TEXT_HEIGHT"           , "IS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_text_height) },
    { 0x99, "SAVE_FNT"              , "IS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_save_fnt) },
    { 0x9A, "LOAD_BDF"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_bdf) },
    /* Fecha */
    { 0x9E, "TIME"                  , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_time) },
    { 0x9F, "FTIME"                 , "SI"    , TYPE_STRING , 2 , SYSMACRO(fxi_ftime) },
    /* Ficheros */
    { 0xA0, "SAVE"                  , "SV++"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_save) },
    { 0xA1, "LOAD"                  , "SV++"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_load) },
    { 0xA2, "FOPEN"                 , "SI"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_fopen) },
    { 0xA3, "FCLOSE"                , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_fclose) },
    { 0xA4, "FREAD"                 , "IV++"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_fread) },
    { 0xA5, "FWRITE"                , "IV++"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_fwrite) },
    { 0xA6, "FSEEK"                 , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_fseek) },
    { 0xA7, "FTELL"                 , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_ftell) },
    { 0xA8, "FLENGTH"               , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_filelength) },
    { 0xA9, "FPUTS"                 , "IS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_fputs) },
    { 0xAA, "FGETS"                 , "I"     , TYPE_STRING , 1 , SYSMACRO(fxi_fgets) },
    { 0xAB, "FEOF"                  , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_feof) },
    { 0xAC, "FILE"                  , "S"     , TYPE_STRING , 1 , SYSMACRO(fxi_file) },
    /* Sonido de CD */
    { 0xB6, "PLAY_CD"               , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_play_cd) },
    { 0xB7, "STOP_CD"               , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_stop_cd) },
    { 0xB8, "IS_PLAYING_CD"         , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_is_playing_cd) },
    /* Modo 7 */
    { 0xC0, "START_MODE7"           , "IIIIII", TYPE_DWORD  , 6 , SYSMACRO(fxi_start_mode7) },
    { 0xC1, "STOP_MODE7"            , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_stop_mode7) },
    /* Scroll */
    { 0xC2, "START_SCROLL"          , "IIIIII", TYPE_DWORD  , 6 , SYSMACRO(fxi_start_scroll) },
    { 0xC3, "STOP_SCROLL"           , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_stop_scroll) },
    { 0xC4, "MOVE_SCROLL"           , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_move_scroll) },
    /* Animaciones FLI */
    { 0xC5, "START_FLI"             , "SII"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_start_fli) },
    { 0xC6, "END_FLI"               , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_end_fli) },
    { 0xC7, "FRAME_FLI"             , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_frame_fli) },
    { 0xC8, "RESET_FLI"             , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_reset_fli) },
    /* Tratamiento de cadenas */
    { 0xD0, "LEN"                   , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_strlen) },
    { 0xD1, "UCASE"                 , "S"     , TYPE_STRING , 1 , SYSMACRO(fxi_strupper) },
    { 0xD2, "LCASE"                 , "S"     , TYPE_STRING , 1 , SYSMACRO(fxi_strlower) },
    { 0xD3, "SUBSTR"                , "SII"   , TYPE_STRING , 3 , SYSMACRO(fxi_substr) },
    { 0xD4, "FIND"                  , "SS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_strfind) },
    { 0xD5, "ITOA"                  , "I"     , TYPE_STRING , 1 , SYSMACRO(fxi_itos) },
    { 0xD6, "FTOA"                  , "F"     , TYPE_STRING , 1 , SYSMACRO(fxi_ftos) },
    { 0xD7, "ATOI"                  , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_stoi) },
    { 0xD8, "ATOF"                  , "S"     , TYPE_FLOAT  , 1 , SYSMACRO(fxi_stof) },
    { 0xD9, "ASC"                   , "S"     , TYPE_BYTE   , 1 , SYSMACRO(fxi_asc) },
    { 0xDA, "CHR"                   , "I"     , TYPE_STRING , 1 , SYSMACRO(fxi_chr) },
    /* Extensiones adicionales */
    { 0xE0, "LOAD_PNG"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_png) },
    { 0xE1, "LOAD_PCX"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_pcx) },
    { 0xEB, "MAP_BUFFER"            , "II"    , TYPE_POINTER, 2 , SYSMACRO(fxi_map_buffer) },
    { 0xEC, "SET_COLORS"            , "IIP"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_set_colors) },
    { 0xED, "GET_COLORS"            , "IIP"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_get_colors) },
    { 0xF0, "ALLOC"                 , "I"     , TYPE_POINTER, 1 , SYSMACRO(fxi_alloc) },
    { 0xF1, "FREE"                  , "P"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_free) },
    { 0xF2, "REALLOC"               , "PI"    , TYPE_POINTER, 2 , SYSMACRO(fxi_realloc) },
    { 0xF3, "EXISTS"                , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_running) },
    { 0xF4, "MEMSET"                , "PBI"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_memset) },
    { 0xF5, "MEMSETW"               , "PWI"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_memsetw) },
    { 0xF6, "MEMCOPY"               , "PPI"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_memcopy) },
    { 0xEF, "MEMORY_TOTAL"          , ""      , TYPE_FLOAT  , 0 , SYSMACRO(fxi_memory_total) },
    { 0xF7, "MEMORY_FREE"           , ""      , TYPE_FLOAT  , 0 , SYSMACRO(fxi_memory_free) },
    /* Funciones de primitivas */
    { 0xF8, "DRAWING_MAP"           , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_drawing_map) },
    { 0xF9, "DRAWING_COLOR"         , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_drawing_color) },
    { 0xFA, "DRAW_LINE"             , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_line) },
    { 0xFB, "DRAW_RECT"             , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_rect) },
    { 0xFC, "DRAW_BOX"              , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_box) },
    { 0xFD, "DRAW_CIRCLE"           , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_circle) },
    { 0xFE, "DRAW_FCIRCLE"          , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_fcircle) },
    { 0xFF, "SET_TEXT_COLOR"        , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_set_text_color) },
    /* Búsqueda de caminos*/
    { 0x100,"PATH_FIND"             , "IIIIIII",TYPE_DWORD  , 7 , SYSMACRO(fxi_path_find)  },
    { 0x101,"PATH_GETXY"            , "PP"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_path_getxy)  },
    { 0x102,"PATH_WALL"             , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_path_wall)  },
    /* Blendops */
    { 0x110,"BLENDOP_NEW"           , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_create_blendop) },
    { 0x111,"BLENDOP_IDENTITY"      , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_blendop_identity) },
    { 0x112,"BLENDOP_TINT"          , "IFIII" , TYPE_DWORD  , 5 , SYSMACRO(fxi_blendop_tint) },
    { 0x113,"BLENDOP_TRANSLUCENCY"  , "IF"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_blendop_translucency) },
    { 0x114,"BLENDOP_INTENSITY"     , "IF"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_blendop_intensity) },
    { 0x115,"BLENDOP_SWAP"          , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_blendop_swap) },
    { 0x116,"BLENDOP_ASSIGN"        , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_blendop_assign) },
    { 0x117,"BLENDOP_APPLY"         , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_blendop_apply) },
    { 0x118,"BLENDOP_FREE"          , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_destroy_blendop) },
    { 0x119,"BLENDOP_GRAYSCALE"     , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_blendop_grayscale) },
    /* WM */
    { 0x120, "SET_TITLE"            , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_set_title) } ,
    { 0x121, "SET_ICON"             , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_set_icon) } ,
    /* Extended Write */
    { 0x122, "WRITE_VAR"            ,"IIIIV++", TYPE_DWORD  , 7 , SYSMACRO(fxi_write_var) },
    { 0x123, "WRITE_FLOAT"          , "IIIIP" , TYPE_DWORD  , 5 , SYSMACRO(fxi_write_float) },
    { 0x124, "WRITE_STRING"         , "IIIIP" , TYPE_DWORD  , 5 , SYSMACRO(fxi_write_string) },
    /* Nuevas funciones de sonido */
    { 0x130,"LOAD_SONG"             , "S"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_load_song) },
    { 0x131,"PLAY_SONG"             , "II"   , TYPE_DWORD   , 2 , SYSMACRO(fxi_play_song) },
    { 0x132,"UNLOAD_SONG"           , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_unload_song) },
    { 0x133,"STOP_SONG"             , ""     , TYPE_DWORD   , 0 , SYSMACRO(fxi_stop_song) },
    { 0x134,"PAUSE_SONG"            , ""     , TYPE_DWORD   , 0 , SYSMACRO(fxi_pause_song) },
    { 0x135,"RESUME_SONG"           , ""     , TYPE_DWORD   , 0 , SYSMACRO(fxi_resume_song) },
    { 0x136,"SET_SONG_VOLUME"       , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_set_song_volume) },
    { 0x137,"IS_PLAYING_SONG"       , ""     , TYPE_DWORD   , 0 , SYSMACRO(fxi_is_playing_song) },
    { 0x138,"LOAD_WAV"              , "S"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_load_wav) },
    { 0x139,"PLAY_WAV"              , "II"   , TYPE_DWORD   , 2 , SYSMACRO(fxi_play_wav) },
    { 0x13A,"UNLOAD_WAV"            , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_unload_wav) },
    { 0x13B,"STOP_WAV"              , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_stop_wav) },
    { 0x13C,"PAUSE_WAV"             , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_pause_wav) },
    { 0x13D,"RESUME_WAV"            , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_resume_wav) },
    { 0x13E,"IS_PLAYING_WAV"        , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_is_playing_wav) },
    { 0x13F,"SET_WAV_VOLUME"        , "II"   , TYPE_DWORD   , 2 , SYSMACRO(fxi_set_wav_volume) },
    { 0x140,"FADE_MUSIC_IN"         , "III"  , TYPE_DWORD   , 3 , SYSMACRO(fxi_fade_music_in) },
    { 0x141,"FADE_MUSIC_OFF"        , "I"    , TYPE_DWORD   , 1 , SYSMACRO(fxi_fade_music_off) },
    { 0x148,"SET_PANNING"           , "III"  , TYPE_DWORD   , 3 , SYSMACRO(fxi_set_panning) },
    { 0x149,"SET_POSITION"          , "III"  , TYPE_DWORD   , 3 , SYSMACRO(fxi_set_position) },
    { 0x14A,"SET_DISTANCE"          , "II"   , TYPE_DWORD   , 2 , SYSMACRO(fxi_set_distance) },
    { 0x14B,"REVERSE_STEREO"        , "II"   , TYPE_DWORD   , 2 , SYSMACRO(fxi_reverse_stereo) },
    /* funciones de MAQ */
    { 0x150, "GRAYSCALE"            , "IIB"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_grayscale) },
    { 0x151, "RGBSCALE"             , "IIFFF" , TYPE_DWORD  , 5 , SYSMACRO(fxi_rgbscale)},
    { 0x152, "BLUR"                 , "IIB"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_blur)},
    { 0x153, "FILTER"               , "IIP"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_filter)},
    { 0x154, "QUICKSORT"            , "PIIIBB", TYPE_DWORD  , 6 , SYSMACRO(fxi_quicksort)},
    /* Paleta de colores */
    { 0x160,"LOAD_PAL"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_load_pal) },
    { 0x161,"FADE"                  , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_fade) },
    { 0x162,"FADE_ON"               , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_fade_on) },
    { 0x163,"FADE_OFF"              , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_fade_off) },
    { 0x164,"ROLL_PALETTE"          , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_roll_palette) },
    { 0x165,"CONVERT_PALETTE"       , "IIP"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_convert_palette) },
    { 0x166,"FIND_COLOR"            , "BBB"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_find_color) },
    { 0x167,"RGB"                   , "BBB"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_rgb) },
    { 0x168,"GET_RGB"               , "IPPP"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_get_rgb) },
    /* Nuevas funciones en la 0.83 */
    { 0x16A,"GET_SCREEN"            , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_get_screen) },
    { 0x16B,"SET_MODE"              , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_set_mode_3) },
    { 0x16C,"SUBSTR"                , "SI"    , TYPE_STRING , 2 , SYSMACRO(fxi_substr2) },
    { 0x16D,"TRIM"                  , "S"     , TYPE_STRING , 1 , SYSMACRO(fxi_trim) },
    { 0x16E,"FORMAT"                , "I"     , TYPE_STRING , 1 , SYSMACRO(fxi_formatI) },
    { 0x16F,"FORMAT"                , "F"     , TYPE_STRING , 1 , SYSMACRO(fxi_formatF) },
    { 0x170,"FORMAT"                , "FI"    , TYPE_STRING , 2 , SYSMACRO(fxi_formatFI) },
    { 0x171,"STRREV"                , "S"     , TYPE_STRING , 1 , SYSMACRO(fxi_strrev) },
    { 0x172,"FIND"                  , "SSI"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_strfindSSI) },
    { 0x173,"LPAD"                  , "SI"    , TYPE_STRING , 2 , SYSMACRO(fxi_lpad) },
    { 0x174,"RPAD"                  , "SI"    , TYPE_STRING , 2 , SYSMACRO(fxi_rpad) },
    { 0x175,"STRCASECMP"            , "SS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_strcasecmp) },
    { 0x176,"SAVE_PNG"              , "IIS"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_save_png) },
    { 0x177,"REGEX"                 , "SS"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_regex) },
    { 0x178,"REGEX_REPLACE"         , "SSS"   , TYPE_STRING , 3 , SYSMACRO(fxi_regex_replace) },
    { 0x179,"SPLIT"                 , "SSPI"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_split) },
    { 0x17A,"JOIN"                  , "SPI"   , TYPE_STRING , 3 , SYSMACRO(fxi_join) },
    { 0x17B,"DRAW_CURVE"            , "IIIIIIIII", TYPE_DWORD,9 , SYSMACRO(fxi_bezier) },
    { 0x17C,"DRAWING_Z"             , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_drawing_at) },
    { 0x17D,"DELETE_DRAW"           , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_delete_drawing) },
    { 0x17E,"MOVE_DRAW"             , "III"   , TYPE_DWORD  , 3 , SYSMACRO(fxi_move_drawing) },
    { 0x17F,"DRAWING_ALPHA"         , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_drawing_alpha) },
	/* Nuevas funciones en la 0.84 */
    { 0x180,"CD"                    , ""      , TYPE_STRING , 0 , SYSMACRO(fxi_cd) } ,
    { 0x181,"CHDIR"                 , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_chdir) },
    { 0x182,"MKDIR"                 , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_mkdir) } ,
    { 0x183,"RMDIR"                 , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_rmdir) } ,
    { 0x184,"FILE_EXISTS"           , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_file_exists) } ,
    { 0x185,"GLOB"			        , "S"     , TYPE_STRING , 1 , SYSMACRO(fxi_glob) },
	{ 0x18F,"SET_MODE"              , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_set_mode_2) },
    { 0x190,"GET_TEXT_COLOR"        , ""      , TYPE_DWORD  , 0 , SYSMACRO(fxi_get_text_color) },
    { 0x199,"NEW_FNT"		        , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_fnt_new) },
    { 0x199,"FNT_NEW"		        , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_fnt_new) },
    { 0x19A,"GET_GLYPH"		        , "II"    , TYPE_DWORD  , 2 , SYSMACRO(fxi_get_glyph) },
    { 0x19B,"SET_GLYPH"		        , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_set_glyph) },
    { 0x19C,"SET_MODE"              , "IIII"  , TYPE_DWORD  , 4 , SYSMACRO(fxi_set_mode_4) }, 
    { 0x19D,"DRAWING_STIPPLE"	    , "I"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_drawing_stipple) },
    { 0x19E,"SAVE_PAL"              , "S"     , TYPE_DWORD  , 1 , SYSMACRO(fxi_save_pal) },
    { 0    ,0                       , ""      , 0           , 0 , 0  }
} ;
