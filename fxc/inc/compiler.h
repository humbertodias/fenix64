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

#ifndef __COMPILER_H
#define __COMPILER_H

#ifdef TARGET_MAC
#include <SDL/SDL_types.h>
#else
#include <SDL_types.h>
#endif

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

#ifndef __PROCDEF_H
#include "procdef.h"
#endif

#ifndef __VARSPACE_H
#include "varspace.h"
#endif

/* Compilador */

typedef struct _expresion_result
{
    int asignation ;
    int call ;
    int lvalue ;
    int constant ;
    TYPEDEF type ;

    int count; // Usados para array genericos

    Sint32 value ;
    float  fvalue ;
}
expresion_result ;

extern void compile_error (const char * error, ...) ;

/* Compilado de secciones superiores */
extern void compile_init ();
extern void compile_program ();
extern void compile_sentence (PROCDEF * p) ;
extern void compile_block(PROCDEF *) ;

/* Compilado de secciones especiales (definición de datos, etc) */
extern int  compile_array_data (VARSPACE * n, segment * data, int size, int subsize, BASETYPE *t) ;
extern int  compile_varspace (VARSPACE * n, segment * data, int additive, int copies, int padding, VARSPACE ** c, int alignment) ;
extern void compile_constants () ;

#define DEFAULT_ALIGNMENT 4

/* Compilador de expresiones */
extern expresion_result compile_value () ;
extern expresion_result compile_factor () ;
extern expresion_result compile_operand () ;
extern expresion_result compile_operation () ;
extern expresion_result compile_rotation () ;
extern expresion_result compile_clausule () ;
extern expresion_result compile_comparison () ;
extern expresion_result compile_subexpresion () ;
extern expresion_result compile_expresion (int need_constant, int need_lvalue, BASETYPE t) ;
extern expresion_result convert_result_type (expresion_result res, BASETYPE t);

/* Códigos de identificadores y palabras reservadas */

extern int

    identifier_program,     identifier_debug,       identifier_const,
    identifier_begin,       identifier_end,         identifier_struct,
    identifier_global,      identifier_local,       identifier_public,      identifier_private,
    identifier_const,       identifier_dup,         identifier_while,
    identifier_repeat,      identifier_until,       identifier_switch,
    identifier_case,        identifier_default,     identifier_loop,
    identifier_for,         identifier_from,        identifier_step,
    identifier_to,          identifier_if,          identifier_else,
    identifier_break,       identifier_continue,    identifier_return,
    identifier_frame,       identifier_clone,       identifier_onexit,
    identifier_sizeof,      identifier_bandoffset,
    identifier_offset,      identifier_pointer,     identifier_type,
    identifier_and,         identifier_or ,         identifier_xor,         identifier_not,
    identifier_band,        identifier_bor,         identifier_bxor,        identifier_bnot,
    identifier_plus,        identifier_minus,       identifier_plusplus,    identifier_minusminus,
    identifier_equal,       identifier_multiply,    identifier_mod,
    identifier_divide,      identifier_semicolon,   identifier_colon,
    identifier_comma,       identifier_ror,         identifier_rol,
    identifier_rightp,      identifier_leftp,       identifier_rightb,
    identifier_leftb,       identifier_point,       identifier_twopoints,
    identifier_eq,          identifier_ne,          identifier_gte,
    identifier_lte,         identifier_lt,          identifier_gt,
    identifier_plusequal,   identifier_minusequal,  identifier_multequal,
    identifier_divequal,    identifier_modequal,    identifier_orequal,
    identifier_xorequal,    identifier_andequal,    identifier_rorequal,
    identifier_rolequal,    identifier_mouse,       identifier_dword,
    identifier_word,        identifier_byte,        identifier_string,
    identifier_float,       identifier_include,     identifier_type,
    identifier_import,      identifier_elseif,      identifier_question,
    identifier_function,    identifier_int,         identifier_short,
    identifier_char,        identifier_unsigned,    identifier_signed ;

extern int reserved_words ;  /* Número de palabras reservadas */

extern int identifier_is_type(int id);
extern int identifier_is_basic_type(int id);

#endif
