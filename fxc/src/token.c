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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fxc.h"
#include "messages.c"

/* ---------------------------------------------------------------------- */
/* Tokenizador. La función token_next, ampliamente utilizada, recoge el   */
/* siguiente token (identificador, operador, etc) del código fuente, y    */
/* rellena la estructura global "token" con los datos del mismo.          */
/* ---------------------------------------------------------------------- */

int line_count = 0 ; // Se pone a 0, ya que lo incremente con cada \n, y hasta no obtener un \n no se procesa la linea (Splinter)
int current_file = 0 ;

static int disable_defines = 0;
static int identifiers_as_strings = 0;

struct _token token ;
struct _token token_prev ;
struct _token token_saved ;
static int use_saved = 0 ;

#define MAX_SOURCES 32
#define MAX_MACRO_PARAMS 16

typedef struct _define
{
	int		code;
	char *	text;
	int     param_count;
	int     param_id[MAX_MACRO_PARAMS];
}
DEFINE;

static const char * source_ptr ;
static char       * source_start ;
static const char * old_sources[MAX_SOURCES] ;
static char       * old_sources_start[MAX_SOURCES] ;
static int          old_line_counts[MAX_SOURCES] ;
static int          old_current_file[MAX_SOURCES] ;
static int          sources = 0 ;
static DEFINE  *    defines = NULL;
static int          defines_allocated = 0;
static int          defines_count = 0;
static int 			id_define;
static int 			id_ifdef;
static int 			id_ifndef;
static int 			id_endif;
static int 			id_else;
static int 			id_if;

/*
 *  FUNCTION : preprocessor_jumpto
 *
 *  Ignores all of the remaining source file, until
 *  a preprocessor directive with identifier id1
 *  (or id2 if non-zero), and moves the source_ptr
 *  source code pointer just after it.
 *
 *  This function is used by preprocessor()
 *  with a #ifdef, #ifndef or #if directive.
 *
 *  PARAMS :
 *      id1			Identifier of ending directive (i.e. id_endif)
 *		id2			Alternative ending directive (i.e. id_else) or 0 if none
 *
 *  RETURN VALUE :
 *      None
 */

void preprocessor_jumpto(int id, int id2)
{
	int depth = 1;

	use_saved = 0;

	while (depth > 0 && *source_ptr)
	{
		if (*source_ptr == '\n')
		{
			line_count++ ;
			source_ptr++;
			while (ISSPACE(*source_ptr)) {
			    if (*source_ptr == '\n') {
			        line_count++ ;
			    }
				source_ptr++;
			}

			if (*source_ptr == '#')
			{
				source_ptr++;
			    if (*source_ptr == '\n') {
			        line_count++ ;
			    }

				token_next();
				if (token.type == IDENTIFIER)
				{
					if (token.code == id || (id2 && token.code == id2))
						depth--;
					else if (token.code == id_ifdef || token.code == id_ifndef || token.code == id_if)
						depth++;
				}
				continue;
			}
		}
		source_ptr++;
	}
}

/*
 *  FUNCTION : preprocessor_expand
 *
 *  Expands a macro to a malloc'ed text area and moves the
 *  token processing to it using token_init(). This function
 *  is called just after the macro name is read (next token
 *  should be a '(' if any parameters needed)
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      None
 */

void preprocessor_expand (DEFINE * def)
{
	const char * param_left[MAX_MACRO_PARAMS];
	const char * param_right[MAX_MACRO_PARAMS];
	const char * begin;
	const char * old_source;
	char * text;
	int i, count, depth, allocated, size, part, actual_line_count;

	/* No params - easy case */

	if (def->param_count == -1)
	{
		token_init (def->text, current_file);
		return;
	}

	/* Find left parenthesis */

	token_next();
	if (token.type != IDENTIFIER || token.code != identifier_leftp)
		compile_error (MSG_EXPECTED, "(");

	/* Mark parameters' starting and ending positions */

	for (count = 0 ; count < def->param_count ; count++)
	{
		depth = 0;
		param_left[count] = source_ptr;

		while (*source_ptr && (*source_ptr != ')' || depth > 0) && *source_ptr != ',')
		{
			if (*source_ptr == '"' || *source_ptr == '\'')
			{
				begin = source_ptr++;
				while (*source_ptr && *source_ptr != *begin) source_ptr++;
				if (!*source_ptr) compile_error (MSG_EXPECTED, "\"");
				source_ptr++;
				continue;
			}
			if (*source_ptr == '(')
				depth++;
			source_ptr++;
		}
		param_right[count] = source_ptr;
		if (!*source_ptr)
			compile_error (MSG_EXPECTED, ")");
		if (*source_ptr == ')')
			break;
		source_ptr++;
	}

	if (count != def->param_count-1 || *source_ptr != ')') {
		compile_error(MSG_INCORRECT_PARAMC, string_get(def->code), def->param_count-1 );
	}
	source_ptr++;

	/* Expand the macro */

	allocated = 128;
	size = 0;
	text = (char *)malloc(allocated);
	old_source = source_ptr;
	source_ptr = def->text;
	actual_line_count = line_count;

	while (*source_ptr && *source_ptr != '\n')
	{
		begin = source_ptr;
		while (ISSPACE(*source_ptr) && *source_ptr != '\n') source_ptr++;

        if (*source_ptr ) {
    	    if ( *source_ptr != '\n') {
    			token_next();
    			if (token.type == IDENTIFIER)
    			{
    				/* Next token is an identifier. Search for parameter */

    				for (i = 0 ; i < def->param_count ; i++)
    				{
    					if (def->param_id[i] == token.code)
    						break;
    				}
    				if (i != def->param_count)
    				{
    					/* Parameter found - expand it */

    					part = param_right[i] - param_left[i];
    					if (size + part + 1 >= allocated)
    					{
    						allocated += ((part + 256) & ~ 127);
    						text = (char *)realloc(text, part);
    					}
    					text[size++] = ' ';
    					memcpy (text + size, param_left[i], part);
    					size += part;
    					continue;
    				}
    			}

    			/* No parameter found - copy the token */

    			part = source_ptr - begin;
    			if (size + part + 1 >= allocated)
    			{
    				allocated += ((part + 256) & ~127);
    				text = (char *)realloc(text, part);
    			}
    			memcpy (text + size, begin, part);
    			size += part;
    		} else {
                line_count++;
                source_ptr++;
            }
        }
	}
	text[size] = 0;
	source_ptr = old_source;
	line_count = actual_line_count;

	/* Now "include" the expanded text "file" */

	token_init (text, current_file);
	free(text);
}

/*
 *  FUNCTION : preprocessor
 *
 *  Evaluates a preprocessor directive. This function is called
 *  just after a '#' symbol is found after an end-of-line.
 *
 *  The function moves source_ptr to the next line.
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      None
 */

void preprocessor ()
{
	int i, ifdef;
	const char * ptr;
	int actual_line_count;

	static int initialized = 0;

	if (!initialized)
	{
		id_define = identifier_search_or_add("DEFINE");
		id_ifdef  = identifier_search_or_add("IFDEF");
		id_ifndef = identifier_search_or_add("IFNDEF");
		id_else   = identifier_search_or_add("ELSE");
		id_endif  = identifier_search_or_add("ENDIF");
		id_if     = identifier_search_or_add("IF");
		initialized = 1;
	}

	disable_defines = 1;
	token_next();

	if (token.type != IDENTIFIER)
		compile_error (MSG_UNKNOWN_PREP);

	/* #define TEXT value */

	if (token.code == id_define)
	{
		token_next();
		if (token.type != IDENTIFIER)
			compile_error (MSG_INVALID_IDENTIFIER);

		/* Allocate the macro */

		if (defines_allocated == defines_count)
		{
			defines_allocated += 8;
			defines = (DEFINE *) realloc(defines, sizeof(DEFINE) * defines_allocated);
		}

		defines[defines_count].code = token.code;

		/* Check for parameters: no space allowed between name and ( */

		if (*source_ptr == '(')
		{
			source_ptr++;
			for (defines[defines_count].param_count = i = 0 ; *source_ptr != ')' ; )
			{
				if (!*source_ptr)
					compile_error (MSG_EXPECTED, ")");

				if (i == MAX_MACRO_PARAMS)
					compile_error (MSG_TOO_MANY_PARAMS);

				token_next();

				if (token.type != IDENTIFIER || token.code < reserved_words)
					compile_error (MSG_INVALID_IDENTIFIER);

				defines[defines_count].param_id[i++] = token.code;
				defines[defines_count].param_count++;

				while (ISSPACE(*source_ptr)) source_ptr++;
				if (*source_ptr == ',') source_ptr++;
			}
			source_ptr++;
		}
		else
		{
			/* No parameters and no parenthesis */
			defines[defines_count].param_count = -1;
		}

		while (ISSPACE(*source_ptr) && *source_ptr != '\n')
			source_ptr++;

		ptr = source_ptr;

		while (*ptr && *ptr != '\n')
			ptr++;

		while (ptr > source_ptr && (!*ptr || ISSPACE(*ptr)))
			ptr--;

		defines[defines_count].text = (char *)malloc(ptr-source_ptr+2);
		strncpy (defines[defines_count].text, source_ptr, ptr-source_ptr+1);
		defines[defines_count].text[ptr-source_ptr+1] = 0;

		defines_count++;

		source_ptr = ptr+1;

		disable_defines = 0;

		return;
	}

	/* #ifdef CONST / #ifndef CONST*/

	if (token.code == id_ifdef || token.code == id_ifndef)
	{
		ifdef = token.code == id_ifdef;
		token_next();
		if (token.type != IDENTIFIER)
			compile_error (MSG_INVALID_IDENTIFIER);
		for (i = 0 ; i < defines_count ; i++)
		{
			if (defines[i].code == token.code)
			{
				if (ifdef)
				{
					disable_defines = 0;
					return;
				}
				break;
			}
		}
		if (!ifdef && i == defines_count)
		{
			disable_defines = 0;
			return;
		}

		preprocessor_jumpto(id_else, id_endif);
		disable_defines = 0;
		return;
	}

	/* #if */

	if (token.code == id_if)
	{
		expresion_result res;
		ptr = source_ptr;

		while (*ptr && *ptr != '\n') ptr++;
		if (*ptr) {
		    ptr++;
		    line_count++;
	    }

        actual_line_count=line_count;

		disable_defines = 0;
		identifiers_as_strings = 1;
	    res = compile_expresion(0,0,TYPE_DWORD);
		identifiers_as_strings = 0;
		source_ptr = ptr;

		line_count=actual_line_count;

		use_saved = 0;
		if (!res.constant)
			compile_error (MSG_CONSTANT_EXP);
		if (!res.value)
		{
			disable_defines = 1;
			preprocessor_jumpto(id_else, id_endif);
			disable_defines = 0;
		}
		return;
	}

	/* #else */

	if (token.code == id_else)
	{
		preprocessor_jumpto(id_endif, 0);
		disable_defines = 0;
		return;
	}

	/* #endif */

	if (token.code == id_endif)
	{
		disable_defines = 0;
		return;
	}

	/* Unknown preprocessor directive */
	compile_error (MSG_UNKNOWN_PREP);
}

void token_init (const char * source, int file)
{
	char * ptr;
	char * clean_source;

	if (sources == MAX_SOURCES)
		compile_error (MSG_TOO_MANY_INCLUDES) ;

	if (!source)
	{
		fprintf (stdout, "token_init: no source\n") ;
		exit (1) ;
	}

	/* Perform cleaning of the source file */

	clean_source = (char *) malloc(strlen(source) + 2);
	ptr = clean_source;
	*ptr++ = '\n';			/* Adds a blank line to detect first-line # directives */
	while (*source)
	{
		if (*source == '\\' && source[1] == '\n')
		{
			source += 2;
			*ptr++ = ' ';
		}
		else
		{
			*ptr++ = *source++;
		}
	}
	*ptr = 0;

	/* Store the old source pointer */

	old_line_counts   [sources] = line_count ;
	old_current_file  [sources] = current_file ;
	old_sources       [sources] = source_ptr ;
	old_sources_start [sources] = source_start ;
	sources++ ;

	/* Use the new source */

	line_count = 0;
	source_ptr = clean_source;
	source_start = clean_source;
	use_saved = 0;
	current_file = file ;
}

int token_endfile()
{
	if (source_start)
	{
		free (source_start);
		source_start = 0;
	}
	if (sources > 0)
	{
		sources-- ;
		source_ptr = old_sources[sources] ;
		source_start = old_sources_start[sources] ;
		line_count = old_line_counts[sources] ;
		current_file = old_current_file[sources] ;
	}
	return 0 ;
}

void token_dump ()
{
	fprintf (stdout, "(") ;
	if (token.type == NUMBER) fprintf (stdout, "%d", token.code) ;
	else if (token.type == FLOAT) fprintf (stdout, "%f", token.value) ;
	else if (token.type == STRING) fprintf (stdout, "\"%s\"", string_get(token.code)) ;
	else if (token.type == IDENTIFIER) fprintf (stdout, "\"%s\"", identifier_name(token.code)) ;
	else if (token.type == NOTOKEN) fprintf (stdout, "EOF") ;
	else fprintf (stdout, "unknown\n") ;
	fprintf (stdout, ")\n") ;
}

extern void load_file (const char * filename) ;


void token_next ()
{
	static char buffer[1024] ;
	static int  i, len ;
	char * buffer_ptr = buffer ;

	if (!source_ptr)
	{
		token.type = NOTOKEN ;
		return ;
	}

	if (use_saved)
	{
		token = token_saved ;
		use_saved = 0 ;
		return ;
	}
	token_prev = token ;

	while (1)
	{
		while (ISSPACE(*source_ptr))
		{
			if (*source_ptr == '\n')
			{
				line_count++ ;
				for (source_ptr++ ; ISSPACE(*source_ptr) ; source_ptr++)
				{
					if (*source_ptr == '\n') {
						line_count++ ;
			        }
				}
				if (*source_ptr != '#')
					break;

				/* Comandos de preprocesador */

				source_ptr++;
				preprocessor();
				continue;
			}
			source_ptr++ ;
		}

		if (!*source_ptr)
		{
			while (!*source_ptr)
			{
				if (sources == 0)
				{
					token.type = NOTOKEN ;
					return ;
				}
				token_endfile() ;
				if (!source_ptr)
				{
					token.type = NOTOKEN;
					return;
				}
			}
			continue;
		}

		/* Ignora comentarios */

		if (*source_ptr == '/' && *(source_ptr+1) == '*')
		{
			while (*source_ptr != '*' || source_ptr[1] != '/')
			{
				if (*source_ptr == '\n') {
				    line_count++ ;
    			}
				if (!*source_ptr)
				{
					token.type = NOTOKEN ;
					return ;
				}
				source_ptr++ ;
			}
			source_ptr += 2 ;
			continue ;
		}

		if (*source_ptr == '/' && *(source_ptr+1) == '/')
		{
			while (*source_ptr != '\n' && *source_ptr)
				source_ptr++ ;

			if (*source_ptr)
			{
				source_ptr++ ;
				line_count++ ;
				if (*source_ptr == '#')
				{
					source_ptr++;
					preprocessor();
				}
			}
			continue ;
		}

		/* Cadenas */

		if (*source_ptr == '"' || *source_ptr == '\'')
		{
			token.type = STRING ;
			token.code = string_compile(&source_ptr) ;
			return ;
		}

		/* Operadores de más de un caracter */

		len = 0 ;

		if (*source_ptr == '<')
		{
			if (source_ptr[1] == '<')
			{
				if (source_ptr[2] == '=') len = 3 ;
				else                      len = 2 ;
			}
			else if (source_ptr[1] == '>')	  len = 2 ;
			else if (source_ptr[1] == '=')	  len = 2 ;
			else                              len = 1 ;
		}
		else if (*source_ptr == '>')
		{
			if (source_ptr[1] == '>')
			{
				if (source_ptr[2] == '=') len = 3 ;
				else                      len = 2 ;
			}
			else if (source_ptr[1] == '=')	  len = 2 ;
			else if (source_ptr[1] == '>')	  len = 2 ;
			else                              len = 1 ;
		}
		else if (*source_ptr == '|')
		{
			if (source_ptr[1] == '|')
			{
				if (source_ptr[2] == '=') len = 3 ;
				else                      len = 2 ;
			}
			else if (source_ptr[1] == '=')    len = 2 ;
			else                              len = 1 ;
		}
		else if (*source_ptr == '=')
		{
			if (source_ptr[1] == '=')	  len = 2 ;
			else if (source_ptr[1] == '>')	  len = 2 ;
			else if (source_ptr[1] == '<')	  len = 2 ;
			else                              len = 1 ;
		}
		else if (*source_ptr == '.')
		{
			if (source_ptr[1] == '.')	  len = 2 ;
			else                              len = 1 ;
		}
		else if (strchr("!&^%*+-/", *source_ptr))
		{
			if (source_ptr[1] == '=')	  len = 2 ;
			else if (strchr("+-&^", *source_ptr) &&
			    source_ptr[1] == *source_ptr) len = 2 ;
			else                              len = 1 ;
		}

		if (len)
		{
			strncpy (buffer, source_ptr, len) ;
			buffer[len] = 0 ;
			source_ptr += len ;
			token.code = identifier_search_or_add(buffer) ;
			token.type = IDENTIFIER ;
			return ;
		}

		/* Numbers */

		if (ISNUM(*source_ptr))
		{
			const char * ptr ;
			double num = 0, dec ;
			int base = 10 ;

			/* Hex/Bin/Octal numbers with the h/b/o sufix */

			ptr = source_ptr ;
			while (*ptr == '1' || *ptr == '0') ptr++ ;
			if ((*ptr == 'b' || *ptr == 'B') && !ISALNUM(ptr[1]))
				base = 2 ;
			else
			{
				ptr = source_ptr ;
				while (ISALNUM(*ptr)) ptr++ ;
				if (*ptr == 'h' || *ptr == 'H')
					base = 16 ;
				if (*ptr == 'o' || *ptr == 'O')
					base = 8 ;
			}

			/* Calculate the number value */

			while (ISNUM(*source_ptr) || (base > 10 && ISALNUM(*source_ptr)))
			{
				if (ISNUM(*source_ptr))
					num = num * base + (*source_ptr++ - '0') ;
				if (*source_ptr >= 'a' && *source_ptr <= 'f' && base > 10)
					num = num * base + (*source_ptr++ - 'a' + 10) ;
				if (*source_ptr >= 'A' && *source_ptr <= 'F' && base > 10)
					num = num * base + (*source_ptr++ - 'A' + 10) ;
			}
			token.type = NUMBER ;
			if (base==16)
			    token.code = (unsigned int)num ;
			else
			    token.code = (int)num ;
			token.value = (float)num ;

			/* We have the integer part now - convert to int/float */

			if ((*source_ptr) == '.' && base == 10)
			{
				source_ptr++ ;
				if (!ISNUM(*source_ptr))
					source_ptr-- ;
				else
				{
					dec = 1.0 / (double)base ;
					while (ISNUM(*source_ptr) || (base > 100 && ISALNUM(*source_ptr)))
					{
						if (ISNUM(*source_ptr))
							num = num + dec * (*source_ptr++ - '0') ;
						if (*source_ptr >= 'a' && *source_ptr <= 'f' && base > 10)
							num = num + dec * (*source_ptr++ - 'a' + 10) ;
						if (*source_ptr >= 'A' && *source_ptr <= 'F' && base > 10)
							num = num + dec * (*source_ptr++ - 'A' + 10) ;
						dec /= (double)base ;
					}
					token.type  = FLOAT ;
					token.value = (float)num ;
				}
			}

			/* Skip the base sufix */

			if (base == 16 && *source_ptr == 'h')
				source_ptr++;
			if (base == 8  && *source_ptr == 'o')
				source_ptr++;
			if (base == 2  && *source_ptr == 'b')
				source_ptr++;

			return ;
		}

		/* Identificadores */

		if (ISWORDFIRST(*source_ptr))
		{
			while (ISWORDCHAR(*source_ptr))
			{
				if (buffer_ptr == buffer+1023)
					compile_error (MSG_IDENTIFIER_TOO_LONG) ;
				*buffer_ptr++ = TOUPPER(*source_ptr++) ;
			}
			*buffer_ptr++ = 0 ;
			token.code = identifier_search_or_add(buffer) ;
			token.type = IDENTIFIER ;

			/* Search for #define constant inclusion at this point */

			if (!disable_defines)
			{
				for (i = 0 ; i < defines_count ; i++)
				{
					if (defines[i].code == token.code)
					{
						preprocessor_expand(&defines[i]);
						token_next();
						return;
					}
				}
			}

			/* In a #if, all identifiers are strings */

			if (identifiers_as_strings)
			{
				token.type = STRING;
				token.code = string_new(buffer);
				return;
			}

			/* Include */

			if (token.code == identifier_include && !use_saved)
			{
				while (ISSPACE(*source_ptr)) source_ptr++ ;
				if (*source_ptr == '"')
				{
					source_ptr++ ;
					buffer_ptr = buffer ;
					while (*source_ptr && *source_ptr != '"')
					{
						if (buffer_ptr == buffer+1023)
							compile_error (MSG_IDENTIFIER_TOO_LONG) ;
						*buffer_ptr++ = *source_ptr++ ;
					}
					if (*source_ptr == '"') source_ptr++ ;
					*buffer_ptr = 0 ;
					load_file (buffer) ;
					token_next() ;
					return ;
				}
				compile_error (MSG_IDENTIFIER_EXP) ;
			}
			return ;
		}

		/* 1-char operator or invalid symbol */

		if (!*source_ptr) break ;

		if (*source_ptr > 0 && *source_ptr < 32)
		{
			compile_error (MSG_INVALID_CHAR) ;
			return ;
		}

		*buffer_ptr++ = *source_ptr++ ;
		*buffer_ptr = 0 ;
		token.code = identifier_search_or_add(buffer) ;
		token.type = IDENTIFIER ;

		return ;
	}

	token.type = NOTOKEN ;
	return ;		/* End-of-file */
}

void token_back ()
{
	if (use_saved)
	{
		fprintf (stdout, "Error: two token_back in a row\n") ;
		exit(1) ;
	}

	token_saved = token ;
	token = token_prev ;
	use_saved = 1 ;
}
