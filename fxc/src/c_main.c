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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* BeOS includes */
#ifdef TARGET_BeOS
#include <dlfcn.h>
#endif

#include <fnx_loadlib.h>

#include "fxc.h"
#include "messages.c"


/* ---------------------------------------------------------------------- */
/* Módulo principal del compilador. Contiene código que inicializa los    */
/* identificadores conocidos, así como las funciones de compilado de      */
/* nivel más alto                                                         */
/* ---------------------------------------------------------------------- */

/* Tipos */
int identifier_dword ;
int identifier_word ;
int identifier_byte ;
int identifier_string ;
int identifier_float ;
int identifier_struct ;
int identifier_type ;

int identifier_program ;
int identifier_debug ;
int identifier_const ;
int identifier_begin ;
int identifier_end ;
int identifier_global ;
int identifier_local ;
int identifier_private ;
int identifier_const ;
int identifier_dup ;
int identifier_from ;
int identifier_step ;
int identifier_to ;
int identifier_if ;
int identifier_else ;
int identifier_elseif ;
int identifier_for ;
int identifier_while ;
int identifier_frame ;
int identifier_clone ;
int identifier_switch ;
int identifier_case ;
int identifier_default ;
int identifier_repeat ;
int identifier_until ;
int identifier_loop ;
int identifier_break ;
int identifier_continue ;
int identifier_return ;
int identifier_process ;
int identifier_offset ;
int identifier_sizeof ;
int identifier_pointer ;
int identifier_id ;

int identifier_and ;
int identifier_or  ;
int identifier_xor ;
int identifier_not ;

int identifier_plus ;
int identifier_minus ;
int identifier_plusplus ;
int identifier_minusminus ;
int identifier_equal ;
int identifier_multiply ;
int identifier_mod ;
int identifier_divide ;
int identifier_semicolon ;
int identifier_colon ;
int identifier_comma ;
int identifier_ror ;
int identifier_rol ;
int identifier_rightp ;
int identifier_leftp ;
int identifier_rightb ;
int identifier_leftb ;
int identifier_point ;
int identifier_twopoints ;

int identifier_eq ;
int identifier_ne ;
int identifier_gte ;
int identifier_lte ;
int identifier_lt ;
int identifier_gt ;
int identifier_question ;

int identifier_plusequal ;
int identifier_minusequal ;
int identifier_multequal ;
int identifier_divequal ;
int identifier_modequal ;
int identifier_orequal ;
int identifier_xorequal ;
int identifier_andequal ;
int identifier_rorequal ;
int identifier_rolequal ;
int identifier_mouse ;
int identifier_include ;

int identifier_import ;

int reserved_words ;

#ifdef TARGET_MAC
static int debug ;
#else
int debug;
#endif

void compile_init ()
{
	/* Initialize reserved words */

	identifier_dword 		= identifier_add("DWORD") ;
	identifier_word 		= identifier_add("WORD") ;
	identifier_byte 		= identifier_add("BYTE") ;
	identifier_float 		= identifier_add("FLOAT") ;
	identifier_string		= identifier_add("STRING") ;
	identifier_struct		= identifier_add("STRUCT") ;

	identifier_add_as	("INT",  identifier_dword) ;
	identifier_add_as	("SHORT",identifier_word) ;
	identifier_add_as	("CHAR", identifier_byte) ;

	identifier_include 		= identifier_add("INCLUDE") ;
	identifier_program 		= identifier_add("PROGRAM") ;
	identifier_debug   		= identifier_add("DEBUG") ;
	identifier_const   		= identifier_add("CONST") ;
	identifier_begin		= identifier_add("BEGIN") ;
	identifier_end			= identifier_add("END") ;
	identifier_process   	= identifier_add("PROCESS") ;
	identifier_global		= identifier_add("GLOBAL") ;
	identifier_local		= identifier_add("LOCAL") ;
	identifier_private		= identifier_add("PRIVATE") ;
	identifier_dup   		= identifier_add("DUP") ;
	identifier_from   		= identifier_add("FROM") ;
	identifier_to   		= identifier_add("TO") ;
	identifier_step   		= identifier_add("STEP") ;
	identifier_for   		= identifier_add("FOR") ;
	identifier_while   		= identifier_add("WHILE") ;
	identifier_repeat   	= identifier_add("REPEAT") ;
	identifier_until   		= identifier_add("UNTIL") ;
	identifier_switch   	= identifier_add("SWITCH") ;
	identifier_case			= identifier_add("CASE") ;
	identifier_default   	= identifier_add("DEFAULT") ;
	identifier_loop   		= identifier_add("LOOP") ;
	identifier_break   		= identifier_add("BREAK") ;
	identifier_continue   	= identifier_add("CONTINUE") ;
	identifier_return   	= identifier_add("RETURN") ;
	identifier_if   		= identifier_add("IF") ;
	identifier_else   		= identifier_add("ELSE") ;
	identifier_elseif  		= identifier_add("ELSEIF") ;
	identifier_frame   		= identifier_add("FRAME") ;
	identifier_clone   		= identifier_add("CLONE") ;

	identifier_and			= identifier_add("AND");
	identifier_or 			= identifier_add("OR");
	identifier_xor			= identifier_add("XOR");
	identifier_not			= identifier_add("NOT");

	identifier_sizeof		= identifier_add("SIZEOF");
	identifier_offset		= identifier_add("OFFSET");
	identifier_pointer		= identifier_add("POINTER");
	identifier_type			= identifier_add("TYPE");
	identifier_id			= identifier_add("ID");

	identifier_add_as	("&", identifier_offset) ;
	identifier_add_as	("!", identifier_not) ;
	identifier_add_as	("&&", identifier_and) ;
	identifier_add_as	("||", identifier_or) ;
	identifier_add_as	("|", identifier_or) ;
	identifier_add_as	("^", identifier_xor) ;

	identifier_plus			= identifier_add("+") ;
	identifier_minus		= identifier_add("-") ;
	identifier_plusplus		= identifier_add("++") ;
	identifier_minusminus	= identifier_add("--") ;
	identifier_multiply		= identifier_add("*") ;
	identifier_mod     		= identifier_add("%") ;
	identifier_divide		= identifier_add("/") ;
	identifier_equal		= identifier_add("=") ;
	identifier_semicolon	= identifier_add(";") ;
	identifier_comma		= identifier_add(",") ;
	identifier_ror      	= identifier_add(">>") ;
	identifier_rol      	= identifier_add("<<") ;
	identifier_rightp      	= identifier_add(")") ;
	identifier_leftp      	= identifier_add("(") ;
	identifier_rightb      	= identifier_add("]") ;
	identifier_leftb      	= identifier_add("[") ;
	identifier_point      	= identifier_add(".") ;
	identifier_twopoints	= identifier_add("..") ;
	identifier_question		= identifier_add("?") ;

	identifier_add_as	("MOD", identifier_mod) ;
	identifier_add_as   (":", identifier_semicolon) ;
	identifier_add_as   ("ELIF", identifier_elseif);
	identifier_add_as   ("ELSIF", identifier_elseif);
	identifier_colon        = identifier_add(":") ;

	identifier_eq		    = identifier_add("==") ;
	identifier_ne		    = identifier_add("!=") ;
	identifier_gt		    = identifier_add(">") ;
	identifier_lt		    = identifier_add("<") ;
	identifier_gte		    = identifier_add(">=") ;
	identifier_lte		    = identifier_add("<=") ;

	identifier_add_as	("<>", identifier_ne) ;
	identifier_add_as	("=>", identifier_gte) ;
	identifier_add_as	("=<", identifier_lte) ;

	identifier_plusequal	= identifier_add("+=") ;
	identifier_andequal		= identifier_add("&=") ;
	identifier_xorequal		= identifier_add("^=") ;
	identifier_orequal		= identifier_add("|=") ;
	identifier_divequal		= identifier_add("/=") ;
	identifier_modequal		= identifier_add("%=") ;
	identifier_multequal	= identifier_add("*=") ;
	identifier_minusequal	= identifier_add("-=") ;
	identifier_rorequal		= identifier_add(">>=") ;
	identifier_rolequal		= identifier_add("<<=") ;

	identifier_import	  = identifier_add("IMPORT") ;

	reserved_words = identifier_next_code() ;

	identifier_mouse      = identifier_add("MOUSE") ;

	varspace_init (&global) ;
	varspace_init (&local) ;
	globaldata = segment_new() ;
	localdata = segment_new() ;

}

extern void token_dump() ;

void compile_error (const char *fmt, ...)
{
	char text[4000] ;
	va_list ap;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	fprintf (stdout, MSG_COMPILE_ERROR, files[current_file], line_count, text) ;
	token_dump () ;
	exit (2) ;
}

int imports[1024] ;
int nimports = 0 ;

// Funcion nula
void * compile_export ( char * name )
{
    return ( NULL ) ;
}

void compile_import ()
{
	const char  * filename ;
	void        * library ;
	dlfunc      RegisterFunctions ;

#ifdef TARGET_BeOS
	char soname[1024];
	char * ptr;
#endif

	token_next() ;
	if (token.type != STRING)
		compile_error (MSG_STRING_EXP) ;
	imports[nimports++] = token.code ;

	filename = string_get(token.code) ;

#ifdef TARGET_BeOS
	snprintf (soname, 1024, "./%s.so", filename);

	/* Clean the name (strip .DLL, and use lowercase) */

	for (ptr = soname ; *ptr ; ptr++)
		*ptr = TOLOWER(*ptr);
	if (strlen(soname) > 7 && strcmp(ptr-7, ".dll.so") == 0)
		strcpy (ptr-7, ".so");

	library  = dlopen (soname, RTLD_NOW | RTLD_GLOBAL) ;
#else
	library  = dlopen (filename, RTLD_NOW | RTLD_GLOBAL) ;
#endif

	if (!library) compile_error (dlerror()) ;

	RegisterFunctions = dlsym (library, "RegisterFunctions") ;
	if (!RegisterFunctions) compile_error(MSG_NO_COMPATIBLE_DLL) ;

	(*RegisterFunctions)(compile_export, sysproc_add) ;
}

void compile_constants ()
{
	int code, value ;

	for (;;)
	{
		token_next() ;
		if (token.type == NOTOKEN)
			break ;
		if (token.type != IDENTIFIER)
			compile_error (MSG_CONSTANT_NAME_EXP) ;
		if (token.code == identifier_semicolon)
			continue ;
		if (token.code == identifier_begin ||
		    token.code == identifier_local ||
		    token.code == identifier_global)
		{
			token_back() ;
			return ;
		}
		if (token.code == identifier_end)
			return ;
		if (token.code < reserved_words)
		{
			compile_error (MSG_INVALID_IDENTIFIER);
		}
		code = token.code ;

		token_next() ;
		if (token.type != IDENTIFIER || token.code != identifier_equal)
			compile_error (MSG_EXPECTED, "=") ;

		value = compile_fixed_expresion() ;
		constants_add (code, fixed_expression_type, value) ;

		token_next() ;
		if (token.type == IDENTIFIER && token.code == identifier_semicolon)
			continue ;
		compile_error (MSG_EXPECTED, ";") ;
	}
}

void compile_process ()
{
	PROCDEF * proc ;
	VARIABLE  * var ;
	int code, tcode, params ;
	BASETYPE type ;
	TYPEDEF ctype ;

	tcode = token.code ;
	token_next () ;
	if (tcode == identifier_process &&
		(token.code == identifier_process
	      || token.code == identifier_dword
	      || token.code == identifier_word
	      || token.code == identifier_byte
	      || token.code == identifier_float
	      || token.code == identifier_string))
	{
		tcode = token.code ;
		token_next() ;
	}

	if (segment_by_name(token.code))
	{
		tcode = identifier_pointer ;
		token_next() ;
	}

	while (token.code == identifier_pointer)
	{
		tcode = token.code ;
		token_next() ;
	}

	if (token.type != IDENTIFIER || token.code < reserved_words)
		compile_error (MSG_PROCNAME_EXP) ;
	code = token.code ;

	token_next () ;
	if (token.type != IDENTIFIER || token.code != identifier_leftp)
		compile_error (MSG_EXPECTED, "(") ;

	proc = procdef_search (code) ;
	if (!proc) proc = procdef_new (procdef_getid(), code) ;
	proc->defined = 1 ;
	if (tcode == identifier_float)  proc->type = TYPE_FLOAT ;
	if (tcode == identifier_string) proc->type = TYPE_STRING ;
	if (tcode == identifier_word)   proc->type = TYPE_WORD ;
	if (tcode == identifier_byte)   proc->type = TYPE_BYTE ;
	if (tcode == identifier_pointer)proc->type = TYPE_POINTER ;
	token_next() ;

	/* Recoge los parámetros y su tipo */

	params = 0 ;
	type = TYPE_DWORD ;
	ctype = typedef_new(type) ;
	while (token.type != IDENTIFIER || token.code != identifier_rightp)
	{
		if (token.type == IDENTIFIER && token.code == identifier_dword)
		{
			type = TYPE_DWORD ;
			ctype = typedef_new(type) ;
			token_next() ;
		}
		else if (token.type == IDENTIFIER && token.code == identifier_word)
		{
			type = TYPE_WORD ;
			ctype = typedef_new(type) ;
			token_next() ;
		}
		else if (token.type == IDENTIFIER && token.code == identifier_byte)
		{
			type = TYPE_BYTE ;
			ctype = typedef_new(type) ;
			token_next() ;
		}
		else if (token.type == IDENTIFIER && token.code == identifier_string)
		{
			type = TYPE_STRING ;
			ctype = typedef_new(type) ;
			token_next() ;
		}
		else if (token.type == IDENTIFIER && token.code == identifier_float)
		{
			type = TYPE_FLOAT ;
			ctype = typedef_new(type) ;
			token_next() ;
		}
		else if (token.type == IDENTIFIER && segment_by_name(token.code))
		{
			ctype = *typedef_by_name(token.code) ;
			type = TYPE_STRUCT ;
			token_next() ;
		}

		while (token.type == IDENTIFIER && token.code == identifier_pointer)
		{
			type = TYPE_POINTER ;
			ctype = typedef_pointer(ctype) ;
			token_next() ;
		}

		if (type == TYPE_STRUCT)
		{
			type = TYPE_POINTER ;
			ctype = typedef_pointer(ctype) ;
		}

		if (token.type != IDENTIFIER || token.code < reserved_words)
			compile_error (MSG_INVALID_PARAM) ;

		var = varspace_search (&local, token.code) ;
		if (var) /* Un parámetro es en realidad un local */
		{
			if (typedef_base(var->type) != type)
				compile_error (MSG_INVALID_PARAMT) ;
			codeblock_add (&proc->code, MN_LOCAL, var->offset) ;
			codeblock_add (&proc->code, MN_PRIVATE, proc->pridata->current) ;
			codeblock_add (&proc->code, MN_PTR, 0) ;
			codeblock_add (&proc->code, MN_LET, 0) ;
			codeblock_add (&proc->code, MN_POP, 0) ;
			if (proc->privars->reserved == proc->privars->count)
				varspace_alloc (proc->privars, 16) ;
			proc->privars->vars[proc->privars->count].type   = typedef_new (TYPE_DWORD);
			proc->privars->vars[proc->privars->count].offset = proc->pridata->current ;
			proc->privars->vars[proc->privars->count].code   = -1 ;
			proc->privars->count++ ;
			segment_add_dword (proc->pridata, 0) ;
		}
		else /* Crear la variable privada */
		{
			if (proc->privars->reserved == proc->privars->count)
				varspace_alloc (proc->privars, 16) ;
			if (type == TYPE_STRING)
				varspace_varstring (proc->privars, proc->pridata->current) ;
			proc->privars->vars[proc->privars->count].type   = ctype;
			proc->privars->vars[proc->privars->count].offset = proc->pridata->current ;
			proc->privars->vars[proc->privars->count].code   = token.code ;
			proc->privars->count++ ;
			segment_add_dword (proc->pridata, 0) ;
		}

		if (proc->params != -1)
		{
			/* El proceso fue usado previamente */

			if (proc->paramtype[params] == TYPE_UNDEFINED)
				proc->paramtype[params] = type ;
			else if (proc->paramtype[params] == TYPE_DWORD &&
					(type == TYPE_BYTE || type == TYPE_WORD))
				proc->paramtype[params] = type ;
			else if (type == TYPE_DWORD && (proc->paramtype[params] == TYPE_BYTE || proc->paramtype[params] == TYPE_WORD))
				proc->paramtype[params] = type ;
			else if (proc->paramtype[params] != type)
				compile_error (MSG_INVALID_PARAMT) ;
		}
		else
		{
			proc->paramtype[params] = type;
		}
		params++ ;

		if (params == MAX_PARAMS)
			compile_error (MSG_TOO_MANY_PARAMS) ;

		token_next() ;
		if (token.type == IDENTIFIER && token.code == identifier_comma)
			token_next() ;
	}

	if (proc->params == -1)
		proc->params = params ;
	else if (proc->params != params)
		compile_error (MSG_INCORRECT_PARAMC) ;

	token_next () ;
	if (token.type == IDENTIFIER && token.code == identifier_semicolon)
		token_next () ;
	if (token.type == IDENTIFIER && token.code == identifier_private)
	{
		compile_varspace (proc->privars, proc->pridata, 1, 1, &local, &global) ;
		token_next () ;
	}

	/* Gestiona procesos cuyos parámetros son variables locales */

	if (token.type != IDENTIFIER || token.code != identifier_begin)
		compile_error (MSG_NO_BEGIN) ;
	compile_block (proc) ;
	if (token.type == IDENTIFIER && token.code == identifier_else)
		compile_error (MSG_ELSE_WOUT_IF) ;

	if (token.type != IDENTIFIER || token.code != identifier_end)
		compile_error (MSG_NO_END) ;

	codeblock_add (&proc->code, MN_END, 0) ;
#ifdef DEBUG
	if (debug)
	{
		printf ("\n\n---------- Process %d (%s)\n\n---- Private variables\n",
				proc->typeid, identifier_name(code)) ;
		varspace_dump (proc->privars, 0) ;
		//segment_dump  (proc->pridata) ;
		codeblock_dump (&proc->code) ;
	}
#endif
}

void compile_program ()
{
	int code ;

	/* Ahora lo del program es opcional :-P */

	token_next() ;
	if (token.type == IDENTIFIER && token.code == identifier_program)
	{
		token_next() ;
		if (token.type != IDENTIFIER || token.code < reserved_words)
			compile_error (MSG_PROGRAM_NAME_EXP) ;
		token_next() ;
		if (token.type != IDENTIFIER || token.code != identifier_semicolon)
			compile_error (MSG_EXPECTED, ";") ;
	}
	else token_back() ;

        mainproc = procdef_new(procdef_getid(), identifier_search_or_add("MAIN")) ;

	for (;;)
	{
		token_next () ;
		while (token.type == IDENTIFIER && token.code == identifier_semicolon)
			token_next() ;

		if (token.type == IDENTIFIER && token.code == identifier_import)
			compile_import() ;
		else if (token.type == IDENTIFIER && token.code == identifier_const)
			compile_constants() ;
		else if (token.type == IDENTIFIER && token.code == identifier_local)
			compile_varspace (&local, localdata, 1, 1, &global, NULL) ;
		else if (token.type == IDENTIFIER && token.code == identifier_global)
			compile_varspace (&global, globaldata, 1, 1, &local, NULL) ;
		else if (token.type == IDENTIFIER && token.code == identifier_private)
			compile_varspace (mainproc->privars, mainproc->pridata, 1, 1, &local, &global) ;
		else if (token.type == IDENTIFIER && token.code == identifier_begin)
		{
			if (mainproc->defined)
			{
				/* Hack para poder redefinir el proceso
				 * principal */
				mainproc->code.current -= 1 ;
			}
			mainproc->defined = 1 ;
			compile_block (mainproc) ;
			if (token.type == IDENTIFIER && token.code == identifier_else)
				compile_error (MSG_ELSE_WOUT_IF) ;
			if (token.type != IDENTIFIER || token.code != identifier_end)
				compile_error (MSG_NO_END) ;
			codeblock_add (&mainproc->code, MN_END, 0) ;
		}

		/* Tipo de dato definido por el usuario */

		else if (token.type == IDENTIFIER && token.code == identifier_type)
		{
			segment  * s = segment_new() ;
			VARSPACE * v = varspace_new() ;
			TYPEDEF    t = typedef_new(TYPE_STRUCT);

                        t.chunk[0].count = 1 ;

			token_next() ;
			if ((code = token.code) < reserved_words || token.type != IDENTIFIER)
				compile_error (MSG_INVALID_TYPE) ;

			t.varspace = v ;
			typedef_name (t, code) ;
			segment_name (s, code) ;

			compile_varspace (v, s, 0, 1, 0, 0) ;
			if (token.code != identifier_end)
				compile_error (MSG_NO_END) ;
		}

		/* Definición de proceso */

		else if (token.type == IDENTIFIER &&
				(token.code == identifier_process
			      || token.code == identifier_dword
			      || token.code == identifier_pointer
			      || token.code == identifier_word
			      || token.code == identifier_byte
			      || token.code == identifier_float
			      || token.code == identifier_string))

			compile_process() ;

	        else if (segment_by_name(token.code))

			compile_process() ;

		else	break ;
	}

#ifdef DEBUG
	if (global.count && debug)
	{
		printf ("\n---- Global variables\n\n") ;
		varspace_dump (&global, 0) ;
	}
	if (local.count && debug)
	{
		printf ("\n---- Local variables\n\n") ;
		varspace_dump (&local, 0) ;
		//segment_dump (localdata) ;
	}
#endif

	if (token.type != NOTOKEN)
		compile_error (MSG_UNEXPECTED_TOKEN) ;

#ifdef DEBUG
	if (debug)
	{
		printf ("\n----- Main procedure\n\n") ;
		codeblock_dump (&mainproc->code) ;
		printf ("\n") ;
	}
#endif
	program_postprocess() ;

	if (!mainproc->defined)
		compile_error (MSG_NO_MAIN) ;

#ifdef DEBUG_VERBOSE
	identifier_dump() ;
	string_dump() ;
#endif
}
