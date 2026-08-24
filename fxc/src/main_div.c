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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fxc.h"

/* ----------------------------------------------------------------------- */
/* Este módulo contiene las definiciones de constantes, locales y globales */
/* predefinidos, así como el código que los carga y define cada una de las */
/* funciones del sistema (es llamado antes de empezar a compilar).         */
/* ----------------------------------------------------------------------- */

struct
{
	char * name ;
	int    code ;
} 
keys[] =
{
	{ "_ESC",			1 },
	{ "_1",				2 },
	{ "_2",				3 },
	{ "_3",				4 },
	{ "_4",				5 },
	{ "_5",				6 },
	{ "_6",				7 },
	{ "_7",				8 },
	{ "_8",				9 },
	{ "_9",				10 },
	{ "_0",				11 },
	{ "_MINUS",			12 },
	{ "_PLUS",			13 },
	{ "_BACKSPACE",		14 },
	{ "_TAB",			15 },
	{ "_Q",				16 },
	{ "_W",				17 },
	{ "_E",				18 },
	{ "_R",				19 },
	{ "_T",				20 },
	{ "_Y",				21 },
	{ "_U",				22 },
	{ "_I",				23 },
	{ "_O",				24 },
	{ "_P",				25 },
	{ "_L_BRACHET",		26 },
	{ "_R_BRACHET",		27 },
	{ "_ENTER",			28 },
	{ "_C_ENTER",		28 },
	{ "_CONTROL",		29 },
	{ "_A",				30 },
	{ "_S",				31 },
	{ "_D",				32 },
	{ "_F",				33 },
	{ "_G",				34 },
	{ "_H",				35 },
	{ "_J",				36 },
	{ "_K",				37 },
	{ "_L",				38 },
	{ "_SEMICOLON",		39 },
	{ "_APOSTROPHE",	40 },
	{ "_WAVE",			41 },
	{ "_L_SHIFT",		42 },
	{ "_BACKSLASH",		43 },
	{ "_Z",				44 },
	{ "_X",				45 },
	{ "_C",				46 },
	{ "_V",				47 },
	{ "_B",				48 },
	{ "_N",				49 },
	{ "_M",				50 },
	{ "_COMMA",			51 },
	{ "_POINT",			52 },
	{ "_SLASH",			53 },
	{ "_C_BACKSLASH",	53 },
	{ "_R_SHIFT",		54 },
	{ "_C_ASTERISK",    55 },
	{ "_PRN_SCR",		55 },
	{ "_ALT",			56 },
	{ "_SPACE",			57 },
	{ "_CAPS_LOCK",		58 },
	{ "_F1",			59 },
	{ "_F2",			60 },
	{ "_F3",			61 },
	{ "_F4",			62 },
	{ "_F5",			63 },
	{ "_F6",			64 },
	{ "_F7",			65 },
	{ "_F8",			66 },
	{ "_F9",			67 },
	{ "_F10",			68 },
	{ "_NUM_LOCK",		69 },
	{ "_SCROLL_LOCK",	70 },
	{ "_HOME",			71 },
	{ "_C_HOME",		71 },
	{ "_UP",			72 },
	{ "_C_UP",			72 },
	{ "_PGUP",			73 },
	{ "_C_PGUP",		73 },
	{ "_C_MINUS",		74 },
	{ "_LEFT",			75 },
	{ "_C_LEFT",		75 },
	{ "_C_CENTER",		76 },
	{ "_RIGHT",			77 },
	{ "_C_RIGHT",		77 },
	{ "_C_PLUS",		78 },
	{ "_END",			79 },
	{ "_C_END",			79 },
	{ "_DOWN",			80 },
	{ "_C_DOWN",		80 },
	{ "_PGDN",			81 },
	{ "_C_PGDN",		81 },
	{ "_INS",			82 },
	{ "_C_INS",			82 },
	{ "_DEL",			83 },
	{ "_C_DEL",			83 },
	{ "_F11",			87 },
	{ "_F12",			88 },
	{ "_LESS",			89 },
	{ "_EQUALS",		90 },
	{ "_GREATER",		91 },
	{ "_ASTERISK",		92 },
	{ "_R_ALT",			93 },
	{ "_R_CONTROL",		94 },
	{ "_L_ALT",			95 },
	{ "_L_CONTROL",		96 },
	{ "_MENU",			97 },
	{ 0, 0 }
} ;

char * locals_def = 
"	STRUCT reserved \n"
"		process_id ; \n"
"		id_scan ; \n"
"		process_type ; \n"
"		type_scan ; \n"
"		status = 2 ; \n"
"		changed ; \n"
"		unused2 ; \n"
"		unused3 ; \n"
"		prev_z ; \n"
"		distance1 ; \n"
"		distance2 ; \n"
"		frame_percent ; \n"
"		box_x0, box_y0 ; \n"
"		box_x1, box_y1 ; \n"
"	END \n"
" \n"
"	father ; \n"
"	son ; \n"
"	smallbro ; \n"
"	bigbro ; \n"
"	priority ; \n"
"	ctype ; \n"
"	cnumber ; \n"
"	x ; \n"
"	y ; \n"
"	z ; \n"
"	graph ; \n"
"	flags ; \n"
"	size = 100 ; \n"
"	angle ; \n"
"	region ; \n"
"	file ; \n"
"	pointer xgraph ; \n"
"	height ; \n"
"	resolution ; \n"
"   alpha = 255 ; \n"
"   size_x = 100 ; \n"
"   size_y = 100 ; \n"
"   blendop ; \n"
"   STRUCT _saved_ \n"
"	    x ; \n"
"	    y ; \n"
"	    graph ; \n"
"	    angle ; \n"
"	    alpha ; \n"
"	    blendop ; \n"
"	    size ; \n"
"	    size_x ; \n"
"	    size_y ; \n"
"	    flags ; \n"
"   END \n"
 ;

char * globals_def = 
"	STRUCT mouse \n"
"		x, y ; \n"
"		graph ; \n"
"		file ; \n"
"		z = -512 ; \n"
"		angle ; \n"
"		size = 100 ; \n"
"		flags ; \n"
"		region ; \n"
"		left, middle, right ; \n"
"		wheelup, wheeldown ; \n"
"	END \n"
" \n"
"	STRUCT scroll[9] \n"
"		x0, y0 ; \n"
"		x1, y1 ; \n"
"		z = 512 ; \n"
"		camera ; \n"
"		ratio = 200; \n"
"		speed ; \n"
"		region1 = -1 ; \n"
"		region2 = -1 ; \n"
"		flags1 ; \n"
"		flags2 ; \n"
"               follow = -1 ; \n"
"		reserved[6] ;		// Tamaño: 20 dwords  \n"
"	END \n"
" \n"
"	STRUCT joy \n"
"		left ; \n"
"		right ; \n"
"		up ; \n"
"		down ; \n"
"		button1 ; \n"
"		button2 ; \n"
"		button3 ; \n"
"		button4 ; \n"
"	END \n"
" \n"
"	STRUCT setup \n"
"		card ; \n"
"		port ; \n"
"		irq ; \n"
"		dma ; \n"
"		dma2 ; \n"
"		master ; \n"
"		sound_fx ; \n"
"		cd_audio ; \n"
"	END \n"
" \n"
"	timer[9] ; \n"
" \n"
"	text_z = -256; \n"
"	fading ; \n"
"	shift_status ; \n"
"	ascii ; \n"
"	scan_code ; \n"
"	joy_filter ; \n"
"	joy_status ; \n"
"	restore_type ; \n"
"	dump_type ; \n"
"	max_process_time ; \n"
"	 \n"
"	// Modo 7 \n"
"	 \n"
"	STRUCT m7[9] \n"
"		camera ; \n"
" \n"
"		height   = 32 ; \n"
"		distance = 64 ; \n"
"		horizon  = 0 ; \n"
"		focus    = 256 ; \n"
"		z        = 256 ; \n"
"		color    = 0 ; \n"
"		flags    = 0 ; \n"
"		reserved[1] ; \n"
"	END \n"
" \n"
"	// No documentados \n"
" \n"
"	fps ; \n"
" \n"
"	// Propios de Fenix \n"
" \n"
"	text_flags ; \n"
"	pansep = 64 ; \n"
"	reverb = 0 ; \n"
"	volume = 128 ; \n"
"	sound_freq = 22050; \n"
"	sound_mode = 1 ; \n" 
"   graph_mode = 0; \n"				/* Undocumented - Obsolete */
" \n"
"   argc ; \n"
"   string argv[32] ; \n"
" \n"
"	speed_gauge = 0 ; \n"
"	FLOAT frame_time = 0 ; \n"
"   STRING regex_reg[15] ; \n"
"   alpha_steps = 16 ; \n"
"  \n"
"	STRUCT fileinfo \n"
"       STRING path; \n"
"       STRING name; \n"
"       directory; \n"
"       hidden; \n"
"       readonly; \n"
"       size; \n"
"       STRING created; \n"
"       STRING modified; \n"
"   END \n"
"   full_screen = 0 ; \n"				/* Undocumented - Obsolete */
" \n";

struct
{
	char * name ;
	int    code ;
} 
constants_def[] =
{
	{ "M320X200",			3200200 },
	{ "M320X240",			3200240 },
	{ "M320X400",			3200400 },
	{ "M360X240",			3600240 },
	{ "M376X282",			3760282 },
	{ "M400X300",			4000300 },
	{ "M512X384",			5120384 },
	{ "M640X400",			6400400 },
	{ "M640X480",			6400480 },
	{ "M800X600",			8000600 },
	{ "M1024X768",			10240768 },
	{ "M1280X1024",			12801024 },
	{ "MODE_8BITS",			8 },
	{ "MODE_16BITS",		16 },
	{ "MODE_8BPP",			8 },
	{ "MODE_16BPP",			16 },
	{ "MODE_WINDOW",		0x0000 },
	{ "MODE_2XSCALE",		0x0100 },
	{ "MODE_FULLSCREEN",	0x0200 },
	{ "MODE_DOUBLEBUFFER",	0x0400 },
	{ "MODE_HARDWARE",		0x0800 },
	{ "DOUBLE_BUFFER",      0x0400 },		/* Obsolete */
	{ "HW_SURFACE",		    0x0800 },		/* Obsolete */
	{ "TRUE",				1 },
	{ "FALSE",				0 },
	{ "S_KILL",				0 },
	{ "S_WAKEUP",			1 },
	{ "S_SLEEP",			2 },
	{ "S_FREEZE",			3 },
	{ "S_KILL_TREE",		100 },
	{ "S_WAKEUP_TREE",		101 },
	{ "S_SLEEP_TREE",		102 },
	{ "S_FREEZE_TREE",		103 },
	{ "ALL_TEXT",			0 },
	{ "ALL_SOUND",			-1 },
	{ "G_WIDE",				0 },			/* Obsolete */
	{ "G_WIDTH",			0 },
	{ "G_HEIGHT",			1 },
	{ "G_CENTER_X",			2 },
	{ "G_CENTER_Y",			3 },
	{ "G_X_CENTER",			2 },
	{ "G_Y_CENTER",			3 },
	{ "G_PITCH",			4 },
	{ "G_DEPTH",			5 },
	{ "G_FRAMES",			6 },
	{ "G_ANIMATION_STEPS",	7 },
	{ "G_ANIMATION_STEP",	8 },
	{ "G_ANIMATION_SPEED",	9 },
	{ "PF_NODIAG",			1 },
	{ "PF_REVERSE",			2 },
	{ "C_SCREEN",			0 },
	{ "C_SCROLL",			1 },
	{ "C_M7",				2 },
	{ "PARTIAL_DUMP",		0 },
	{ "COMPLETE_DUMP",		1 },
	{ "NO_RESTORE",			-1 },
	{ "PARTIAL_RESTORE",	0 },
	{ "COMPLETE_RESTORE",	1 },
	{ "C_0",				1 },
	{ "C_1",				2 },
	{ "C_2",				4 },
	{ "C_3",				8 },
	{ "C_4",				16 },
	{ "C_5",				32 },
	{ "C_6",				64 },
	{ "C_7",				128 },
	{ "C_8",				256 },
	{ "C_9",				512 },
	{ "MIN_INT",			-32767 },
	{ "MAX_INT",		 	32767 },
	{ "PI",					180000 },
	{ "BACKGROUND",         0 },
	{ "SCREEN",             -1 },
	{ "O_READ",             0 },
	{ "O_READWRITE",        1 },
	{ "O_RDWR",             1 },
	{ "O_WRITE",            2 },
	{ "O_ZREAD",            3 },
	{ "O_ZWRITE",           4 },
	{ 0, 0 }
} ;

void div_init ()
{
	int i = 0, code ;
	while (keys[i].name)
	{
		code = identifier_search_or_add (keys[i].name) ;
		constants_add (code, typedef_new(TYPE_DWORD), keys[i].code) ;
		i++ ;
	}
	i = 0 ;
	while (constants_def[i].name)
	{
		code = identifier_search_or_add (constants_def[i].name) ;
		constants_add (code, typedef_new(TYPE_DWORD), constants_def[i].code) ;
		i++ ;
	}

	token_init (globals_def, 0) ;
	compile_varspace (&global, globaldata, 1, 1, NULL, NULL) ;
	token_init (locals_def, 0) ;
	compile_varspace (&local, localdata, 1, 1, NULL, NULL) ;
}
