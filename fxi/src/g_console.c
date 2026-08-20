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

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "fxi.h"
#include "dcb.h"

/* ---------------------------------------------------------------------- */

#define MAXTEXT 5000

#define CHARWIDTH 6
#define CHARHEIGHT 8

#define CONSOLE_HISTORY 512
#define CONSOLE_LINES   25
#define CONSOLE_COLUMNS 80
#define COMMAND_HISTORY 128

#define HELPTXT \
            "\xAC" "02Process Info\xAC" "07\n"                                          \
            "\xAC" "04INSTANCES      \xAC" "07  List all running processes\n"           \
            "\xAC" "04GLOBALS        \xAC" "07  Show global vars with values\n"         \
            "\xAC" "04LOCALS proc    \xAC" "07  Show a process's local vars\n"          \
            "\xAC" "04PRIVATES proc  \xAC" "07  Show a process's private vars\n"        \
            "\xAC" "04PUBLICS proc   \xAC" "07  Show a process's public vars\n"         \
            "\n"                                                            \
            "\xAC" "02Debugging Commands\xAC" "07\n"                                    \
            "\xAC" "04TRACE          \xAC" "07  Execute one instruction and Debug\n"    \
            "\xAC" "04BREAK          \xAC" "07  List breakpoints\n"                     \
            "\xAC" "04BREAK proc     \xAC" "07  Add a breakpoint on process execution\n"\
            "\xAC" "04DELETE proc    \xAC" "07  Delete a breakpoint\n"                  \
            "\xAC" "04CONTINUE       \xAC" "07  Continue the execution\n"               \
            "\xAC" "04DOFRAME        \xAC" "07  Continue to next frame\n"               \
            "\n"                                                            \
            "\xAC" "02Misc\xAC" "07\n"                                                  \
            "\xAC" "04SHOW expression\xAC" "07  Evaluate and show some expression\n"    \
            "\xAC" "04STRINGS        \xAC" "07  Show all strings in memory\n"           \
            "\xAC" "04VARS           \xAC" "07  Show internals vars\n"                  \
            "\xAC" "04QUIT           \xAC" "07  Kill the program and exit\n\n"          \
            "\n"                                                            \
            "\xAC" "02Process Interaction\xAC" "07\n"                                   \
            "\xAC" "04RUN proc [args]\xAC" "07  Run a process\n"                        \
            "\xAC" "04KILL proc      \xAC" "07  Kill a process\n"                       \
            "\xAC" "04WAKEUP proc    \xAC" "07  Wakeup a process\n"                     \
            "\xAC" "04SLEEP proc     \xAC" "07  Sleep a process\n"                      \
            "\xAC" "04FREEZE proc    \xAC" "07  Freeze a process\n"                     \
            "\xAC" "04KILLALL proc   \xAC" "07  Kill all process with criteria\n"       \
            "\xAC" "04WAKEUPALL proc \xAC" "07  Wakeup all process with criteria\n"     \
            "\xAC" "04SLEEPALL proc  \xAC" "07  Sleep all process with criteria\n"      \
            "\xAC" "04FREEZEALL proc \xAC" "07  Freeze all process with criteria\n"     \
            "\n"                                                            \
            "You can evaluate free expressions in the console,\n"           \
            "and you can see/change local, public and private vars\n"       \
            "using the '.' operator (pe: 65535.X, MAIN.X, etc.)\n"

/* ---------------------------------------------------------------------- */

static void eval_immediate() ;
static void eval_value() ;
static void eval_factor() ;
static void eval_subexpression() ;
static char * eval_expression(const char * here, int interactive);

static int type_size (DCB_TYPEDEF orig) ;
static DCB_TYPEDEF reduce_type (DCB_TYPEDEF orig) ;
static void var2const() ;

/* Console contents */
static int console_lines = CONSOLE_LINES ;
static int console_columns = CONSOLE_COLUMNS ;

static char * console[CONSOLE_HISTORY] ;
static int console_initialized = 0 ;
static int console_head = 0 ;
static int console_tail = 0 ;

static char * command[COMMAND_HISTORY] ;
static int command_initialized = 0 ;
static int command_head = 0 ;
static int command_tail = 0 ;
static int command_count = 0 ;

static char * show_expression = 0;
static int  console_showcolor = 0xffffff;

/* Trace */
extern int      force_debug;
extern int      debug_next;
extern int      debug_on_frame;

extern int      trace_sentence;
extern INSTANCE * trace_instance;

/* ---------------------------------------------------------------------- */

/* Tipos de token */
#define IDENTIFIER 1
#define STRING     2
#define NUMBER     3
#define OPERATOR   4
#define NOTOKEN    5

struct
{
    int type ;
    DCB_VAR var ;
    double value ;
    void * data ;
    char name[256] ;
} result, lvalue ;

struct
{
    enum { T_ERROR, T_VARIABLE, T_NUMBER, T_CONSTANT, T_STRING } type ;
    char name[128] ;
    double  code ;
} token ;

static const char * token_ptr ;

/* ---------------------------------------------------------------------- */

#define N_CONSOLE_VARS (sizeof(console_vars)/sizeof(console_vars[0]))

#define CON_DWORD               0x0001
#define CON_DWORD_HEX           0x8001

struct
{
        char * name ;
        void * value ;
        int    type ;
}
console_vars[] =
{
        { "ENABLE_FILTERING",   &enable_filtering,      CON_DWORD },
#ifdef MMX_FUNCTIONS
        { "MMX",                &MMX_available,         CON_DWORD },
#endif
        { "REPORT_GRAPHICS",    &report_graphics,       CON_DWORD },
        { "REPORT_STRING",      &report_string,         CON_DWORD },

        { "DEBUG",              &debug,                 CON_DWORD },

        { "CONSOLE_LINES",      &console_lines,         CON_DWORD },
        { "CONSOLE_COLUMNS",    &console_columns,       CON_DWORD },

        { "SHOW_COLOR",         &console_showcolor,     CON_DWORD_HEX },

        { "FILES",              &opened_files,          CON_DWORD },

} ;

static char * describe_type (DCB_TYPEDEF type, int from) ;
static char * show_value (DCB_TYPEDEF type, void * data) ;
static void   show_struct (int num, char * title, int indent, void * data) ;
static void   show_var (DCB_VAR var, char * name, void * data, char * title, int indent) ;

/* ---------------------------------------------------------------------- */

void gr_con_printf (const char *fmt, ...)
{
    char text[MAXTEXT], * ptr, * iptr ;

    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    text[sizeof(text)-1] = 0;

    if (*text == '[')
    {
        memmove (text+3, text, strlen(text)+1) ;
        memmove (text, "\xAC" "08", 3) ;
        ptr = strchr(text, ']') ;
        if (ptr)
        {
            ptr++ ;
            memmove (ptr+3, ptr, strlen(ptr)+1) ;
            memmove (ptr, "\xAC" "07", 3) ;
        }
    }

    iptr = text ;
    ptr  = text ;

    while (*ptr)
    {
        if (*ptr == '\n')
        {
            *ptr = 0 ;
            gr_con_putline(iptr) ;
            iptr = ptr+1 ;
        }
        if (*ptr == '\xAC')
        {
            ptr++ ;
            if (isdigit(*ptr)) ptr++ ;
            if (isdigit(*ptr)) ptr++ ;
            continue ;
        }
        ptr++ ;
    }
    if (ptr > iptr) {
        gr_con_putline(iptr) ;
    }
}

void gr_con_putline (char * text)
{
    if (!console_initialized)
    {
        memset (console, 0, sizeof(console)) ;
        console_initialized = 1 ;
    }

    if (console[console_tail])
        free(console[console_tail]) ;
    console[console_tail] = strdup(text) ;

    console_tail++ ;
    if (console_tail == CONSOLE_HISTORY)
        console_tail = 0 ;
    if (console_tail == console_head)
    {
        console_head++ ;
        if (console_head == CONSOLE_HISTORY)
            console_head = 0 ;
    }
}

void gr_con_putcommand (const char * commandline)
{
    if (!command_initialized)
    {
        memset (command, 0, sizeof(command));
        command_initialized = 1;
    }

    if (command[command_tail])
        free(command[command_tail]);
    command[command_tail++] = strdup(commandline);
    if (command_tail == COMMAND_HISTORY)
        command_tail = 0;
    if (command_tail == command_head)
    {
        if (++command_head == COMMAND_HISTORY)
            command_head = 0;
    }
    else
        command_count++;
}

const char * gr_con_getcommand (int offset)
{
    if (offset >= 0 || offset < -command_count)
        return NULL;

    offset = command_tail + offset;
    while (offset < 0)
        offset = COMMAND_HISTORY + offset;
    return command[offset];
}

static int console_showing = 0 ;
static int console_scroll_pos = 0 ;
static int console_scroll_lateral_pos = 0 ;
static char console_input[128] ;

void gr_con_getkey(int key, int sym)
{
    static int history_offset = 0;
    char buffer[2] ;
    const char * command;

    if (key == 0 && sym == 273)
    {
        command = gr_con_getcommand(--history_offset);
        if (command == NULL)
            history_offset++;
        else
            strncpy (console_input, command, 127);
    }
    if (key == 0 && sym == 274)
    {
        if (history_offset == -1)
        {
            *console_input = 0;
            history_offset++;
        }
        else
        {
            command = gr_con_getcommand(++history_offset);
            if (command == NULL)
                history_offset--;
            else
                strncpy (console_input, command, 127);
        }
    }
    if (key == 8 && *console_input)
        console_input[strlen(console_input)-1] = 0 ;
    if (key == 27)
        *console_input = 0 ;
    if (key == 13 && *console_input)
    {
        console_scroll_pos = 0 ;
        gr_con_printf ("\xAC" "15> %s", console_input) ;
        gr_con_putcommand (console_input);
        gr_con_do (console_input) ;
        *console_input = 0 ;
        history_offset = 0;
    }
    if (key >= 32 && key <= 255)
    {
        buffer[0] = key ;
        buffer[1] = 0 ;
        strcat (console_input, buffer) ;
    }
}

void gr_con_show(int doit)
{
    /* Sólo se puede mostrar la consola si hay información de debug */
    if (dcb.data.NID > 0) {
        console_showing = doit ;
    }
}

extern int * stack_gptr;

void gr_con_draw()
{
    int x, y, line, count ;
    static int con_y = 0 ;
    static int vy = CHARHEIGHT ;

    if (!console_initialized)
        return ;

    if (console_columns > scrbitmap->width / CHARWIDTH) {
        console_columns = scrbitmap->width / CHARWIDTH ;
    }

    if (console_lines > ( scrbitmap->height - CHARHEIGHT * 2 ) / CHARHEIGHT) {
        console_lines = ( scrbitmap->height - CHARHEIGHT * 2 ) / CHARHEIGHT ;
    }

    if (console_showing)
    {
        if (con_y < console_lines*CHARHEIGHT)
        {
            con_y += vy ;
            if (vy > 4) vy-- ;
        }
        if (con_y > console_lines*CHARHEIGHT) con_y = console_lines*CHARHEIGHT, vy = CHARHEIGHT ;
    }
    else
    {
        if (con_y > 0)
        {
            con_y -= vy, vy++ ;
            background_dirty = 1;
        }
        if (con_y < 0) { con_y = 0 ; vy = CHARHEIGHT ; return ; }
    }

    x = (scrbitmap->width - console_columns*CHARWIDTH)/2 ;
    y = -console_lines*CHARHEIGHT + con_y ;

    if (show_expression != 0)
    {
        char * result = eval_expression (show_expression, 0);

        if (result == 0)
        {
            free (show_expression);
            show_expression = 0;
        }
        else
        {
            gr_sys_color (console_showcolor, 0) ;
            gr_sys_puts (scrbitmap,
                         (scrbitmap->width - strlen(result)*CHARWIDTH)/2,
                         (con_y <= 0) ? 2:con_y+CHARHEIGHT, result, strlen(result));
            gr_mark_rect (0, (con_y <= 0) ? 2:con_y+CHARHEIGHT, scr_width, CHARHEIGHT);
        }
    }

    if (con_y <= 0) return ;

    line = console_tail ;

    for (count = 0 ; count < console_lines+console_scroll_pos ; count++)
    {
        if (line == console_head)
            break ;
        line-- ;
        if (line < 0) line = CONSOLE_HISTORY-1 ;
    }
    console_scroll_pos = count-console_lines ;
    if (console_scroll_pos < 0) console_scroll_pos = 0 ;

    gr_mark_rect (x, y, console_columns * CHARWIDTH, console_lines * CHARHEIGHT + CHARHEIGHT*2);
    for (count = 0 ; count < console_lines ; count++)
    {
        gr_sys_color (0xC0C0C0, 0x402040) ;
        if (!console[line] || console_scroll_lateral_pos >= strlen(console[line]))
            gr_sys_puts (scrbitmap, x, y, "", console_columns) ;
        else {
            int pos = 0 ;
            int off = 0;

            do {
                if (console[line][pos] == '\xAC') {
                    off += 3;
                }
                pos++;
            } while (pos-off<console_scroll_lateral_pos) ;

            do {
                if (console[line][pos] == '\xAC') {
                    gr_sys_puts (scrbitmap, x, y, console[line]+pos, 3) ;
                    break;
                }
            } while (pos--) ;

            gr_sys_puts (scrbitmap, x, y, console[line]+console_scroll_lateral_pos+off, console_columns) ;
        }

        y += CHARHEIGHT ;
        line++ ;
        if (line == CONSOLE_HISTORY) line = 0 ;
    }

    if (console_showing && trace_sentence != -1) {
        if (trace_instance && instance_exists(trace_instance) && dcb.sourcecount[trace_sentence >> 24]) {
            gr_con_printf("[%s(%d):%d]\n%s",
                    (trace_instance->inproc ? trace_instance->inproc : trace_instance->proc)->name,
                    LOCDWORD(trace_instance, PROCESS_ID),
                    trace_sentence & 0xFFFFFF,
                    dcb.sourcelines [trace_sentence >> 24] [(trace_sentence & 0xFFFFFF)-1]) ;
        }
        debug_on_frame = 0;
        force_debug = 0;
        debug_next = 0;
        trace_sentence = -1;
    }

    gr_sys_color (0xFFFFFF, 0x404040) ;
    gr_sys_puts (scrbitmap, x, y, ">", 2) ;
    strcat(console_input, "_") ;
    gr_sys_puts (scrbitmap, x+CHARWIDTH*2, y, console_input, console_columns-2) ;
    console_input[strlen(console_input)-1] = 0 ;
}

void gr_con_scroll (int direction)
{
    if (direction > 0)
    {
        console_scroll_pos -= console_lines ;
        if (console_scroll_pos < 0)
            console_scroll_pos = 0 ;
    }
    else
    {
        console_scroll_pos += console_lines ;
        if (console_scroll_pos > CONSOLE_HISTORY)
            console_scroll_pos = CONSOLE_HISTORY ;
    }
}

void gr_con_lateral_scroll (int direction)
{
    if (direction > 0)
    {
        console_scroll_lateral_pos-- ;
        if (console_scroll_lateral_pos < 0)
            console_scroll_lateral_pos = 0 ;
    }
    else
    {
        console_scroll_lateral_pos++ ;
        if (console_scroll_lateral_pos > MAXTEXT)
            console_scroll_lateral_pos = MAXTEXT ;
    }
}

/* ---------------------------------------------------------------------- */

static char * describe_type (DCB_TYPEDEF type, int from)
{
    static char buffer[512] ;
    int i ;

    if (!from) buffer[0] = 0 ;

    switch (type.BaseType[from])
    {
        case TYPE_ARRAY:
            for (i = from ; type.BaseType[i] == TYPE_ARRAY; i++) ;
            describe_type (type, i) ;
            for (i = from ; type.BaseType[i] == TYPE_ARRAY; i++)
                _snprintf (buffer+strlen(buffer), 512-strlen(buffer), "[%d]", type.Count[i]-1) ;
            break ;
        case TYPE_STRUCT:
            strcat (buffer, "STRUCT") ;
            break ;
        case TYPE_DWORD:
            strcat (buffer, "DWORD") ;
            break ;
        case TYPE_INT:
            strcat (buffer, "INT") ;
            break ;
        case TYPE_SHORT:
            strcat (buffer, "SHORT") ;
            break ;
        case TYPE_WORD:
            strcat (buffer, "WORD") ;
            break ;
        case TYPE_BYTE:
            strcat (buffer, "BYTE") ;
            break ;
        case TYPE_SBYTE:
            strcat (buffer, "SIGNED BYTE") ;
            break ;
        case TYPE_CHAR:
            strcat (buffer, "CHAR") ;
            break ;
        case TYPE_STRING:
            strcat (buffer, "STRING") ;
            break ;
        case TYPE_FLOAT:
            strcat (buffer, "FLOAT") ;
            break ;
        case TYPE_POINTER:
            describe_type (type, from+1) ;
            strcat (buffer, " POINTER") ;
            break ;
    }

    return buffer ;
}

static char * show_value (DCB_TYPEDEF type, void * data)
{
    static char buffer[256] ;
    char * newbuffer ;
    int subsize;
    unsigned int n ;
    unsigned int count ;

    switch (type.BaseType[0])
    {
        case TYPE_ARRAY:
            count = type.Count[0];

            type = reduce_type(type) ;
            subsize = type_size(type) ;
            if (type.BaseType[0] == TYPE_STRUCT) {
                return "" ;
            }
            newbuffer = (char *) malloc(256) ;
            strcpy (newbuffer, "= (") ;
            for (n = 0 ; n < count ; n++)
            {
                if (n)
                    strcat (newbuffer, ", ") ;
                show_value (type, data) ;
                if (strlen(newbuffer) + strlen(buffer) > 30)
                {
                    strcat (newbuffer, "...") ;
                    break ;
                }
                strcat (newbuffer, buffer + 2) ;
                data = (Uint8*)data + subsize ;
            }
            strcat (newbuffer, ")") ;
            strcpy (buffer, newbuffer) ;
            free(newbuffer) ;
            return buffer ;
        case TYPE_STRUCT:
            return "" ;
        case TYPE_STRING:
            _snprintf (buffer, 512, "= \"%s\"", string_get(*(Uint32 *)data)) ;
            return buffer ;
        case TYPE_BYTE:
            _snprintf (buffer, 512, "= %d", *(Uint8 *)data) ;
            return buffer ;
        case TYPE_SBYTE:
            _snprintf (buffer, 512, "= %d", *(Sint8 *)data) ;
            return buffer ;
        case TYPE_CHAR:
            if (*(Uint8 *)data >= 32)
                _snprintf (buffer, 512, "= '%c'", *(Uint8 *)data) ;
            else
                _snprintf (buffer, 512, "= '\\x%02X'", *(Uint8 *)data) ;
            return buffer ;
        case TYPE_FLOAT:
            _snprintf (buffer, 512, "= %g", *(float *)data) ;
            return buffer ;
        case TYPE_WORD:
            _snprintf (buffer, 512, "= %d", *(Uint16 *)data) ;
            return buffer ;
        case TYPE_DWORD:
            _snprintf (buffer, 512, "= %ud", *(Uint32 *)data) ;
            return buffer ;
        case TYPE_SHORT:
            _snprintf (buffer, 512, "= %d", *(Sint16 *)data) ;
            return buffer ;
        case TYPE_INT:
            _snprintf (buffer, 512, "= %d", *(Sint32 *)data) ;
            return buffer ;
        case TYPE_POINTER:
            _snprintf (buffer, 512, "= 0x%08X", *(Uint32 *)data) ;
            return buffer ;
        default:
            return "?" ;
    }
}

static void show_struct (int num, char * title, int indent, void * data)
{
    int n, count ;
    DCB_VAR * vars ;

    vars = dcb.varspace_vars[num] ;
    count = dcb.varspace[num].NVars ;

    for (n = 0 ; n < count ; n++)
    {
        show_var (vars[n], 0, data ? (Uint8*)data + vars[n].Offset : 0, title, indent) ;
    }
}

static void show_var (DCB_VAR var, char * name, void * data, char * title, int indent)
{
    char spaces[256] ;

    strncpy (spaces, "                                ", indent) ;
    spaces[indent] = 0 ;

    if (!name)
    {
        unsigned int code ;

        name = "?" ;
        for (code = 0 ; code < dcb.data.NID ; code++)
        {
            if (dcb.id[code].Code == var.ID)
            {
                name = dcb.id[code].Name ;
                break ;
            }
        }
    }

    if (data) {
        gr_con_printf ("%s%s %s %s %s\n", title, spaces, describe_type(var.Type, 0), name, show_value(var.Type, data)) ;
    } else {
        gr_con_printf ("%s%s %s %s\n", title, spaces, describe_type(var.Type, 0), name) ;
    }

    if (var.Type.BaseType[0] == TYPE_STRUCT)
    {
        show_struct (var.Type.Members, title, indent+3, data) ;
        gr_con_printf ("%s%s END STRUCT", title, spaces) ;
    }
}


/* ---------------------------------------------------------------------- */

/* Very simple tokenizer */

static void get_token()
{
    char * ptr ;
    unsigned int n ;

    while (isspace(*token_ptr)) token_ptr++ ;

    if (!*token_ptr)
    {
        token.type = NOTOKEN ;
        return ;
    }

	/* Numbers */

	if (ISNUM(*token_ptr))
	{
		const char * ptr ;
		double num = 0, dec ;
		int base = 10 ;

		/* Hex/Bin/Octal numbers with the h/b/o sufix */

		ptr = token_ptr ;
		while (*ptr == '1' || *ptr == '0') ptr++ ;
		if ((*ptr == 'b' || *ptr == 'B') && !ISALNUM(ptr[1]))
			base = 2 ;
		else
		{
			ptr = token_ptr ;
			while (ISALNUM(*ptr)) ptr++ ;
			if (*ptr == 'h' || *ptr == 'H')
				base = 16 ;
			if (*ptr == 'o' || *ptr == 'O')
				base = 8 ;
		}

		/* Calculate the number value */

		while (ISNUM(*token_ptr) || (base > 10 && ISALNUM(*token_ptr)))
		{
			if (ISNUM(*token_ptr))
				num = num * base + (*token_ptr++ - '0') ;
			if (*token_ptr >= 'a' && *token_ptr <= 'f' && base > 10)
				num = num * base + (*token_ptr++ - 'a' + 10) ;
			if (*token_ptr >= 'A' && *token_ptr <= 'F' && base > 10)
				num = num * base + (*token_ptr++ - 'A' + 10) ;
		}
		token.type = NUMBER ;
	    token.code = num ;

		/* We have the integer part now - convert to int/float */

		if ((*token_ptr) == '.' && base == 10)
		{
			token_ptr++ ;
			if (!ISNUM(*token_ptr))
				token_ptr-- ;
			else
			{
				dec = 1.0 / (double)base ;
				while (ISNUM(*token_ptr) || (base > 100 && ISALNUM(*token_ptr)))
				{
					if (ISNUM(*token_ptr))
						num = num + dec * (*token_ptr++ - '0') ;
					if (*token_ptr >= 'a' && *token_ptr <= 'f' && base > 10)
						num = num + dec * (*token_ptr++ - 'a' + 10) ;
					if (*token_ptr >= 'A' && *token_ptr <= 'F' && base > 10)
						num = num + dec * (*token_ptr++ - 'A' + 10) ;
					dec /= (double)base ;
				}
				token.code = num ;
			}
		}

		/* Skip the base sufix */

		if (base == 16 && *token_ptr == 'h')
			token_ptr++;
		if (base == 8  && *token_ptr == 'o')
			token_ptr++;
		if (base == 2  && *token_ptr == 'b')
			token_ptr++;

        _snprintf (token.name, sizeof(token.name), "%g", token.code) ;
		return ;
	}

    if (*token_ptr == '"' || *token_ptr == '\'') /* Cadena */
    {
        char c = *token_ptr++ ;
        token.type = STRING ;
        ptr = token.name;
        while (*token_ptr && *token_ptr != c)
            *ptr++ = *token_ptr++ ;
        if (*token_ptr == c) token_ptr++ ;
        *ptr = 0 ;
        return ;
    }

    ptr = token.name ;
    *ptr++ = TOUPPER(*token_ptr) ;
    if (ISWORDCHAR(*token_ptr++))
    {
        while (ISWORDCHAR(*token_ptr))
            *ptr++ = TOUPPER(*token_ptr++) ;
    }
    *ptr = 0 ;

    for (n = 0 ; n < dcb.data.NID ; n++)
    {
        if (strcmp(dcb.id[n].Name, token.name) == 0)
        {
            token.type = IDENTIFIER ;
            token.code = dcb.id[n].Code ;
            strcpy (token.name, dcb.id[n].Name) ;
            return ;
        }
    }

    token.type = OPERATOR ;
}

static DCB_TYPEDEF reduce_type (DCB_TYPEDEF orig)
{
    int n ;
    for (n = 0 ; n < MAX_TYPECHUNKS-1 ; n++)
    {
        orig.BaseType[n] = orig.BaseType[n+1] ;
        orig.Count[n] = orig.Count[n+1] ;
    }
    return orig ;
}

static void var2const()
{
    while (result.type == T_VARIABLE && result.var.Type.BaseType[0] == TYPE_ARRAY)
        result.var.Type = reduce_type(result.var.Type) ;

    if (result.type == T_VARIABLE && result.var.Type.BaseType[0] == TYPE_STRING)
    {
        result.type = T_STRING ;
        strncpy (result.name, string_get(*(Sint32 *)(result.data)), 127) ;
        result.name[127] = 0 ;
    }
    if (result.type == T_VARIABLE && result.var.Type.BaseType[0] == TYPE_FLOAT)
    {
        result.type = T_CONSTANT ;
        result.value = *(float *)(result.data) ;
    }
    if (result.type == T_VARIABLE && (result.var.Type.BaseType[0] == TYPE_DWORD || result.var.Type.BaseType[0] == TYPE_INT))
    {
        result.type = T_CONSTANT ;
        result.value = *(Sint32 *)(result.data) ;
    }
    if (result.type == T_VARIABLE && (result.var.Type.BaseType[0] == TYPE_WORD || result.var.Type.BaseType[0] == TYPE_SHORT))
    {
        result.type = T_CONSTANT ;
        result.value = *(Sint16 *)(result.data) ;
    }
    if (result.type == T_VARIABLE && (result.var.Type.BaseType[0] == TYPE_BYTE || result.var.Type.BaseType[0] == TYPE_SBYTE))
    {
        result.type = T_CONSTANT ;
        result.value = *(Sint8 *)(result.data) ;
    }
    if (result.type == T_VARIABLE && result.var.Type.BaseType[0] == TYPE_CHAR)
    {
        result.type = T_STRING ;
        if (*(Uint8 *)result.data >= 32)
            _snprintf (result.name, sizeof(result.name), "%c", *(Uint8 *)result.data) ;
        else
            _snprintf (result.name, sizeof(result.name), "\\x%02X", *(Uint8 *)result.data) ;
    }
}

static int type_size (DCB_TYPEDEF orig)
{
    unsigned int n, total ;

    switch (orig.BaseType[0])
    {
        case TYPE_ARRAY:
            return orig.Count[0] * type_size(reduce_type(orig)) ;
        case TYPE_POINTER:
        case TYPE_STRING:
        case TYPE_DWORD:
        case TYPE_FLOAT:
        case TYPE_INT:
            return 4 ;
        case TYPE_WORD:
        case TYPE_SHORT:
            return 2 ;
        case TYPE_BYTE:
        case TYPE_SBYTE:
        case TYPE_CHAR:
            return 1 ;
        case TYPE_STRUCT:
            total = 0 ;
            for (n = 0 ; n < dcb.varspace[orig.Members].NVars ; n++)
                total += type_size (dcb.varspace_vars[orig.Members][n].Type) ;
            return total ;
        default:
            return 0 ;
    }
}

/* ---------------------------------------------------------------------- */

void eval_local (DCB_PROC * proc, INSTANCE * i)
{
    unsigned int n ;

    for (n = 0 ; n < dcb.data.NGloVars ; n++)
    {
        if (dcb.locvar[n].ID == token.code)
        {
            strcpy (result.name, token.name) ;
            result.type = T_VARIABLE ;
            result.var  = dcb.locvar[n] ;
            result.data = (Uint8*)i->locdata + dcb.locvar[n].Offset ;
            get_token() ;
            return ;
        }
    }

    for (n = 0 ; n < proc->data.NPriVars ; n++)
    {
        if (proc->privar[n].ID == token.code)
        {
            strcpy (result.name, token.name) ;
            result.type = T_VARIABLE ;
            result.var  = proc->privar[n] ;
            result.data = (Uint8*)i->pridata + proc->privar[n].Offset ;
            get_token() ;
            return ;
        }
    }

    for (n = 0 ; n < proc->data.NPubVars ; n++)
    {
        if (proc->pubvar[n].ID == token.code)
        {
            strcpy (result.name, token.name) ;
            result.type = T_VARIABLE ;
            result.var  = proc->pubvar[n] ;
            result.data = (Uint8*)i->pubdata + proc->pubvar[n].Offset ;
            get_token() ;
            return ;
        }
    }

    gr_con_printf ("Local or private or public variable not found") ;
    result.type = T_ERROR ;
}

void eval_immediate()
{
    unsigned int n ;

    if (token.type == NUMBER)
    {
        result.type = T_CONSTANT ;
        result.value = token.code ;
        get_token() ;
        return ;
    }

    if (token.type == STRING)
    {
        result.type = T_STRING ;
        _snprintf (result.name, sizeof(result.name), "%s", token.name) ;
        get_token() ;
        return ;
    }

    if (token.type != IDENTIFIER)
    {
        gr_con_printf ("Not a valid expression") ;
        result.type = T_ERROR ;
        return ;
    }

    if (token.name[0] == '(')
    {
        get_token() ;
        eval_subexpression() ;
        if (token.name[0] != ')')
        {
            gr_con_printf ("Unbalanced parens") ;
            result.type = T_ERROR ;
            return ;
        }
        get_token() ;
        return ;
    }

    if (token.name[0] == '-')
    {
        get_token() ;
        eval_immediate() ;
        var2const() ;
        if (result.type != T_CONSTANT)
        {
            result.type = T_ERROR ;
            gr_con_printf ("Operand is not a number\n") ;
            return ;
        }
        result.value = -result.value ;
        _snprintf (result.name, sizeof(result.name), "%g", result.value) ;
        return ;
    }

    for (n = 0 ; n < dcb.data.NGloVars ; n++)
    {
        if (dcb.glovar[n].ID == token.code)
        {
            strcpy (result.name, token.name) ;
            result.type = T_VARIABLE ;
            result.var  = dcb.glovar[n] ;
            result.data = (Uint8*)globaldata + dcb.glovar[n].Offset ;

            get_token() ;
            return ;
        }
    }

    if (strcmp(token.name, "MAIN") == 0)
        token.code = dcb.proc[0].data.ID ;

    for (n = 0 ; n < dcb.data.NProcs ; n++)
    {
        if (dcb.proc[n].data.ID == token.code)
        {
            INSTANCE * i = first_instance ;

            while (i) {
                if (i->proc->type == (int)n) break ;
                i = i->next ;
            }
            if (!i)
            {
                gr_con_printf ("No instance of process %s is active", token.name) ;
                result.type = T_ERROR ;
                return ;
            }

            get_token() ;
            if (token.name[0] != '.')
            {
                result.type = T_ERROR ;
                gr_con_printf ("Invalid use of a process name") ;
                return ;
            }
            get_token() ;
            eval_local(&dcb.proc[n], i) ;
            return ;
        }
    }

    gr_con_printf ("Variable does not exist (%s)", token.name) ;
    result.type = T_ERROR ;
    return ;
}

void eval_value()
{
    eval_immediate() ;
    if (result.type == T_ERROR) return ;

    for (;;)
    {
        if (token.name[0] == '.')
        {
            DCB_VARSPACE * v ;
            DCB_VAR * var ;
            unsigned int n ;

            var2const() ;
            if (result.type == T_CONSTANT)
            {
                INSTANCE * i ;
                i = instance_get((int)result.value) ;
                if (!i)
                {
                    result.type = T_ERROR ;
                    gr_con_printf ("No existe instancia %d\n", (int)result.value) ;
                    return ;
                }
                get_token() ;
                eval_local (&dcb.proc[i->proc->type], i) ;
                continue;
            }

            if (result.type != T_VARIABLE
                || result.var.Type.BaseType[0] != TYPE_STRUCT)
            {
                gr_con_printf ("%s is not an struct", result.name);
                result.type = T_ERROR ;
                return ;
            }
            get_token() ;
            if (token.type != IDENTIFIER)
            {
                gr_con_printf ("%s is not a member", token.name) ;
                result.type = T_ERROR ;
                return ;
            }

            v = &dcb.varspace[result.var.Type.Members] ;
            var = dcb.varspace_vars[result.var.Type.Members] ;
            for (n = 0 ; n < v->NVars ; n++)
            {
                if (var[n].ID == token.code)
                    break ;
            }
            if (n == v->NVars)
            {
                gr_con_printf ("%s is not a member", token.name) ;
                result.type = T_ERROR ;
                return ;
            }

            result.type = T_VARIABLE ;
            result.var = var[n] ;
            result.data = (Uint8 *)result.data + var[n].Offset ;
            strcat (result.name, ".") ;
            strcat (result.name, token.name) ;
            get_token() ;
            continue ;
        }

        if (token.name[0] == '[')
        {
            DCB_VAR i = result.var ;
            void * i_data = result.data ;
            char name[256] ;

            if (result.type != T_VARIABLE || result.var.Type.BaseType[0] != TYPE_ARRAY)
            {
                gr_con_printf ("%s is not an array", result.name) ;
                result.type = T_ERROR ;
                return ;
            }

            strcpy (name, result.name) ;
            get_token() ;
            eval_subexpression() ;
            if (result.type == T_ERROR) return ;
            if (token.name[0] == ']') get_token() ;
            var2const() ;

            if (result.type != T_CONSTANT)
            {
                gr_con_printf ("%s is not an integer", result.name) ;
                result.type = T_ERROR ;
                return ;
            }
            if (result.value < 0)
            {
                gr_con_printf ("Index (%d) less than zero", result.value) ;
                result.type = T_ERROR ;
                return ;
            }
            if (result.value >= i.Type.Count[0])
            {
                gr_con_printf ("Index (%d) out of bounds", result.value) ;
                result.type = T_ERROR ;
                return ;
            }

            result.type = T_VARIABLE ;
            result.var = i ;
            result.var.Type = reduce_type(i.Type) ;
            result.data = (Uint8 *)i_data + (int)result.value * type_size(result.var.Type) ;
            _snprintf (result.name, sizeof(result.name), "%s[%d]", name, (int)result.value) ;
            continue ;
        }

        break ;
    }
}

void eval_factor()
{
    double base = 1 ;
    int op = 0 ;

    for (;;)
    {
        eval_value() ;
        if (result.type == T_ERROR) return ;
        if (!strchr ("*/%", token.name[0]) && !op) return ;
        var2const() ;
        if (result.type != T_CONSTANT)
        {
            result.type = T_ERROR ;
            gr_con_printf ("Operand is not a number\n") ;
            return ;
        }
        if (!op) op = 1 ;
        if (op > 1 && !result.value)
        {
            result.type = T_ERROR ;
            gr_con_printf ("Divide by zero\n") ;
            return ;
        }
        if (op == 1) base *= result.value ;
        if (op == 2) base /= result.value ;
        if (op == 3) base = (int)base % (int)result.value ;
        if (!strchr ("*/%", token.name[0]))
        {
            result.type = T_CONSTANT ;
            result.value = base ;
            _snprintf (result.name, sizeof(result.name), "%g", base) ;
            return ;
        }
        op = (token.name[0] == '*') ? 1: (token.name[0] == '/') ? 2:3 ;
        get_token() ;
    }
}

static void eval_subexpression()
{
    double base = 0 ;
    int op = 0 ;

    for (;;)
    {
        eval_factor() ;
        if (result.type == T_ERROR) return ;
        if (token.name[0] != '+' && token.name[0] != '-' && !op) return ;
        var2const() ;
        if (result.type != T_CONSTANT)
        {
            result.type = T_ERROR ;
            gr_con_printf ("Operand is not a number\n") ;
            return ;
        }
        if (!op) op = 1 ;
        base += op * result.value ;
        if (token.name[0] != '+' && token.name[0] != '-')
        {
            result.type = T_CONSTANT ;
            result.value = base ;
            _snprintf (result.name, sizeof(result.name), "%g", base) ;
            return ;
        }
        op = (token.name[0] == '+') ? 1:-1 ;
        get_token() ;
    }
}

static char * eval_expression(const char * here, int interactive)
{
    static char buffer[1024];
    static char part[1024];

    while (*here == ' ') here++;

    token_ptr = here ;
    get_token() ;
    eval_subexpression() ;

    if (token.type != NOTOKEN && token.name[0] != ',' && token.name[0] != '=')
    {
        if (result.type != T_ERROR)
        {
            gr_con_printf ("Invalid expression");
            result.type = T_ERROR;
        }
        return 0;
    }

    memset (part, 0, sizeof(buffer));
    strncpy (part, here, token_ptr - here - ((token.type != NOTOKEN) ? 1:0));

    if (result.type == T_CONSTANT)
    {
        _snprintf (buffer, 1024, "%s = %g\n", part, result.value);
        if (interactive) gr_con_printf ("%s", buffer) ;
    }
    else if (result.type == T_STRING)
    {
        gr_con_printf ("%s = \"%s\"\n", part, result.name) ;
    }
    else if (result.type == T_VARIABLE)
    {
        lvalue = result ;

        if (token.name[0] == '=')
        {
            if (lvalue.type != T_VARIABLE)
            {
                strcpy (buffer, "Not an lvalue") ;
                if (interactive) gr_con_printf ("%s", buffer) ;
                return buffer ;
            }
            get_token() ;
            eval_subexpression() ;
            if (result.type == T_ERROR) return "" ;
            var2const() ;

            if ((lvalue.var.Type.BaseType[0] == TYPE_DWORD || lvalue.var.Type.BaseType[0] == TYPE_INT) && result.type == T_CONSTANT)
                *(Sint32 *)(lvalue.data) = (Sint32)result.value ;
            else if ((lvalue.var.Type.BaseType[0] == TYPE_WORD || lvalue.var.Type.BaseType[0] == TYPE_SHORT) && result.type == T_CONSTANT)
                *(Uint16 *)(lvalue.data) = (Uint16)result.value ;
            else if ((lvalue.var.Type.BaseType[0] == TYPE_BYTE || lvalue.var.Type.BaseType[0] == TYPE_SBYTE) && result.type == T_CONSTANT)
                *(Uint8 *)(lvalue.data) = (Uint8)result.value ;
            else if (lvalue.var.Type.BaseType[0] == TYPE_CHAR && result.type == T_STRING) {
                if (*result.name=='\\' && *(result.name+1)=='x')
                    *(Uint8 *)(lvalue.data) = (Uint8)strtol(result.name+2, NULL, 16);
                else
                    *(Uint8 *)(lvalue.data) = (Uint8)*(result.name);
            }
            else if (lvalue.var.Type.BaseType[0] == TYPE_FLOAT && result.type == T_CONSTANT)
                *(float *)(lvalue.data) = (float)result.value ;
            else if (lvalue.var.Type.BaseType[0] == TYPE_STRING && result.type == T_STRING)
            {
                string_discard (*(Uint32 *) lvalue.data) ;
                *(Uint32 *)(lvalue.data) = string_new (result.name) ;
                string_use (*(Uint32 *) lvalue.data) ;
            }
            else
            {
                strcpy (buffer, "Invalid assignation") ;
                if (interactive) gr_con_printf ("%s", buffer) ;
                return buffer ;
            }
        }

        if (interactive) {
            show_var (lvalue.var, lvalue.name, lvalue.data, "", 0) ;
        } else {
            strcpy (buffer, part) ;
            strcat (buffer, " ") ;
            strcat (buffer, show_value (lvalue.var.Type, lvalue.data));
        }
    }

    if (token.name[0] == ',')
    {
        char * temporary = strdup(buffer);
        int    size = strlen(temporary);

        if (eval_expression (token_ptr, interactive) == 0)
        {
            free (temporary);
            return 0;
        }
        if (strlen(buffer) + size < 1020 && !interactive)
        {
            memmove (buffer+size+2, buffer, strlen(buffer)+1);
            memcpy  (buffer, temporary, size);
            memcpy  (buffer+size, ", ", 2);
        }
        free (temporary);
        return buffer;
    }

    return buffer;
}

/* ---------------------------------------------------------------------- */
extern void freestrings(PROCDEF * proc, int * data);

INSTANCE * findproc(INSTANCE * last, char * action, char * ptr)
{
    INSTANCE * i = NULL;
    char * aptr;
    int procno;
    int n;

    if (*ptr)
    {
        if (*ptr >= '0' && *ptr <= '9')
        {
            if (last) return NULL;
            procno = atoi(ptr);
            for (i = first_instance ; i ; i = i->next)
                if (LOCDWORD(i, PROCESS_ID) == procno)
                    break;
            if (!i) {
                gr_con_printf ("Instance %d does not exist", procno);
                return NULL;
            }
        }
        else
        {
            aptr = action;
            while (ISWORDCHAR(*ptr)) {
                *aptr++ = TOUPPER(*ptr); ptr++;
            }
            *aptr = 0;

            for (n = 0 ; n < (int)dcb.data.NID ; n++)
                if (strcmp(dcb.id[n].Name, action) == 0)
                    break;
            for (procno = 0 ; procno < (int)dcb.data.NProcs ; procno++)
                if (dcb.proc[procno].data.ID == dcb.id[n].Code)
                    break;
            if (procno == (int)dcb.data.NProcs) {
                gr_con_printf ("Unknown process %s", action);
                return NULL;
            }
            for (i = last ? last->next : first_instance ; i ; i = i->next)
                if (i->proc->type == procno)
                    break;
            if (!i && !last) {
                gr_con_printf ("No instance of process %s created\n", action);
                return NULL;
            }
        }
    }

    return (i);
}

void gr_con_do (const char * command)
{
    const char * ptr ;
    char * aptr ;
    char action[256] ;
    unsigned int var ;
    int n ;
    int procno ;
    PROCDEF * p = NULL ;
    INSTANCE * i = NULL ;

    ptr = command ;
    while (*ptr && *ptr != ' ') ptr++ ;
    strncpy (action, command, ptr - command) ;
    action[ptr - command] = 0 ;
    while (*ptr == ' ') ptr++ ;
    aptr = action ;
    while (*aptr) {
        *aptr = TOUPPER(*aptr);
        aptr++;
    }

    /* Comandos */

    if (strcmp (action, "HELP") == 0)
    {
        gr_con_printf ( HELPTXT );
        return;
    }

    if (strcmp (action, "CONTINUE") == 0)
    {
        show_console = 0;
        force_debug = 0;
        debug_next = 0;
        return ;
    }

    if (strcmp (action, "DOFRAME") == 0)
    {
        show_console = 0;
        debug_on_frame = 1;
        force_debug = 0;
        debug_next = 0;
        return ;
    }

    if (strcmp (action, "TRACE") == 0)
    {
        show_console = 0;
        debug_on_frame = 0;
        force_debug = 0;
        debug_next = 1;
        return ;
    }

    if (strcmp (action, "BREAK") == 0)
    {
        if (*ptr) {
            if (*ptr >= '0' && *ptr <= '9')
            {
                procno = atoi(ptr);
                for (i = first_instance ; i ; i = i->next)
                    if (LOCDWORD(i, PROCESS_ID) == procno)
                        break;
                if (!i) {
                    gr_con_printf ("Instance %d does not exist", procno);
                } else {
                    i->breakpoint = 1;
                    gr_con_printf ("Ok");
                }
            }
            else
            {
                aptr = action;
                while (ISWORDCHAR(*ptr)) {
                    *aptr++ = TOUPPER(*ptr); ptr++;
                }
                *aptr = 0;

                if (*action) {
                    p = procdef_get_by_name(action);
                    if (!p) {
                        gr_con_printf ("Process type %d does not exist", procno);
                    } else {
                        p->breakpoint = 1;
                        gr_con_printf ("Ok");
                    }
                }
            }
        } else {
            int f=0;
            for (n = 0 ; n < procdef_count; n++) {
                if (procs[n].breakpoint) {
                    if (!f) {
                        gr_con_printf("\xAC" "02Process type breakpoints\xAC" "07\n");
                        f = 1;
                    }
                    gr_con_printf("%s\n", procs[n].name);
                }
            }
            if (f)
                gr_con_printf("\n");

            f = 0;
            for (i = first_instance ; i ; i = i->next) {
                if (i->breakpoint) {
                    if (!f) {
                            gr_con_printf("\xAC" "02Process breakpoints\xAC" "07\n");
                        f = 1;
                    }
                    gr_con_printf("%d\n", LOCDWORD(i, PROCESS_ID));
                }
            }
            if (f)
                gr_con_printf("\n");
        }
        return ;
    }

    if (strcmp (action, "DELETE") == 0)
    {
        if (*ptr) {
            if (*ptr >= '0' && *ptr <= '9')
            {
                procno = atoi(ptr);
                for (i = first_instance ; i ; i = i->next)
                    if (LOCDWORD(i, PROCESS_ID) == procno)
                        break;
                if (!i) {
                    gr_con_printf ("Instance %d does not exist", procno);
                } else {
                    i->breakpoint = 0;
                    gr_con_printf ("Ok");
                }
            }
            else
            {
                aptr = action;
                while (ISWORDCHAR(*ptr)) {
                    *aptr++ = TOUPPER(*ptr); ptr++;
                }
                *aptr = 0;

                p = procdef_get_by_name(action);
                if (!p) {
                    gr_con_printf ("Process type %d does not exist", procno);
                } else {
                    p->breakpoint = 0;
                    gr_con_printf ("Ok");
                }
            }
        }
        return ;
    }

    if (strcmp (action, "STRINGS") == 0)
    {
        string_dump() ;
        return ;
    }

    if (strcmp (action, "INSTANCES") == 0)
    {
        instance_dump_all() ;
        return ;
    }

    if (strcmp (action, "GLOBALS") == 0)
    {
        for (var = 0 ; var < dcb.data.NGloVars ; var++)
        {
            DCB_VAR * v = &dcb.glovar[var] ;
            show_var (*v, 0, (Uint8*)globaldata + v->Offset, "[GLOBAL]", 0) ;
        }
        return ;
    }

    if (strcmp (action, "LOCALS") == 0 ||
        strcmp (action, "PRIVATES") == 0 ||
        strcmp (action, "PUBLICS") == 0)
    {
        int show_locals = action[0] == 'L';
        int show_public = action[0] == 'P' && action[1] == 'U' ;

        i = findproc(NULL, action, ptr);

        if (show_locals) {
            for (var = 0 ; var < dcb.data.NLocVars ; var++)
            {
                DCB_VAR * v = &dcb.locvar[var] ;
                show_var (*v, 0, i ? (char*)i->locdata + v->Offset : 0, "[LOCAL]", 0) ;
            }
        } else if (show_public) {
            if (!i) {
                gr_con_printf ("Use: PUBLICS process");
                return;
            }
            for (var = 0 ; var < dcb.proc[i->proc->type].data.NPubVars ; var++)
            {
                DCB_VAR * v = &dcb.proc[i->proc->type].pubvar[var] ;

                /* Unnamed private vars are temporary params and loop
                   counters, and are ignored by this command */
                if ((int)v->ID >= 0) {
                    show_var (*v, 0, (char*)i->pubdata + v->Offset, "[PUBLIC]", 0) ;
                }
            }
        } else {
            if (!i) {
                gr_con_printf ("Use: PRIVATES process");
                return;
            }
            for (var = 0 ; var < dcb.proc[i->proc->type].data.NPriVars ; var++)
            {
                DCB_VAR * v = &dcb.proc[i->proc->type].privar[var] ;

                /* Unnamed private vars are temporary params and loop
                   counters, and are ignored by this command */
                if ((int)v->ID >= 0) {
                    show_var (*v, 0, (char*)i->pridata + v->Offset, "[PRIVATE]", 0) ;
                }
            }
        }
        return ;
    }

    if (strcmp (action, "SHOW") == 0)
    {
        if (show_expression) free(show_expression);
        show_expression = *ptr ? strdup(ptr) : 0;
        return;
    }

    if (strcmp (action, "VARS") == 0)
    {
        for (var = 0 ; var < N_CONSOLE_VARS ; var++)
        {
            switch (console_vars[var].type)
            {
                case CON_DWORD:
                    gr_con_printf ("[FXI] %s = %d\n", console_vars[var].name, *(int *)console_vars[var].value) ;
                    break;

                case CON_DWORD_HEX:
                    gr_con_printf ("[FXI] %s = %08Xh\n", console_vars[var].name, *(int *)console_vars[var].value) ;
                    break;

            }
        }
        return ;
    }

    if (strcmp (action, "RUN") == 0)
    {
        if (*ptr) {
            aptr = action;
            while (ISWORDCHAR(*ptr)) {
                *aptr++ = TOUPPER(*ptr); ptr++;
            }
            *aptr = 0;

            if (*action) {
                int i;
                INSTANCE * inst ;
                p = procdef_get_by_name(action);
                if (p) {

                    token_ptr = ptr ;
                    gr_con_printf ("%s\n", ptr );
                    inst = instance_new (p, NULL);

                    for ( i=0; i < p->params; i++ ){
                        int type = dcb.proc[p->type].privar[i].Type.BaseType[0];
                        get_token() ;
                        eval_subexpression() ;

                        if (result.type == T_VARIABLE)
                            var2const();

                        switch(result.type){
                            case T_CONSTANT:
                                switch(type){
                        			case    TYPE_FLOAT:
                                            PRIDWORD(inst, 4*i) = *(int *)&result.value ;
                        				    break;

                        			case    TYPE_INT:
                        			case    TYPE_DWORD:
                        			case    TYPE_POINTER:
                        			case    TYPE_SHORT:
                        			case    TYPE_WORD:
                        			case    TYPE_BYTE:
                        			case    TYPE_SBYTE:
                        		    case    TYPE_CHAR:
                                            PRIDWORD(inst, 4*i) = (int) result.value ;
                        				    break;

                        			case    TYPE_STRING:
                        			default:
                                            freestrings(p, inst->pridata);
                                            instance_destroy(inst);
                                            gr_con_printf("Invalid argument %d\n", i);
                                            return;
                                }
                                break;

                            case T_STRING:
                                PRIDWORD(inst, 4*i) = (int) string_new(result.name) ;
                                string_use(PRIDWORD(inst, 4*i));
                                break;

                            case T_VARIABLE:
                            default:
                                freestrings(p, inst->pridata);
                                instance_destroy(inst);
                                gr_con_printf("Invalid argument %d\n", i);
                                return;
                        }
                    }
                    gr_con_printf("Process %s is executed!\n", p->name );
                } else {
                    gr_con_printf("Process %s not found!\n", p->name );
                }
                return;
            }
        }
    }

    if (strcmp (action, "KILLALL") == 0 ||
        strcmp (action, "SLEEPALL") == 0 ||
        strcmp (action, "WAKEUPALL") == 0 ||
        strcmp (action, "FREEZEALL") == 0 )
    {
        char    act = *action;
        int     found = 0;
        char    * oaction = strdup(action);
        char    * optr = ptr;

        i = NULL;
        while ((i = findproc(i, action, ptr))) {
            found = 1;
            switch (act) {
                case 'K':
                    LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_KILLED ;
                    break;

                case 'W':
                    LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_RUNNING ;
                    break ;

                case 'S':
                    LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_SLEEPING ;
                    break ;

                case 'F':
                    LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_FROZEN ;
                    break;
            }
            strcpy(action, oaction);
            ptr = optr;
        }

        if(oaction) free(oaction);
        if(found) gr_con_printf ("Ok\n");
        return ;
    }

    if (strcmp (action, "KILL") == 0 ||
        strcmp (action, "SLEEP") == 0 ||
        strcmp (action, "WAKEUP") == 0 ||
        strcmp (action, "FREEZE") == 0 )
    {
        char act=*action;
        i = findproc(NULL, action, ptr);
        if (!i) return;

        switch (act) {
            case 'K':
                LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_KILLED ;
                break;

            case 'W':
                LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_RUNNING ;
                break ;

            case 'S':
                LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_SLEEPING ;
                break ;

            case 'F':
                LOCDWORD(i, STATUS) = (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) | STATUS_FROZEN ;
                break;
        }
        gr_con_printf ("Ok\n");
        return ;
    }

    if (strcmp (action, "QUIT") == 0)
    {
        do_exit(1) ;
        return ;
    }

    /* Variables FXI */

    for (var = 0 ; var < N_CONSOLE_VARS ; var++)
    {
        if (strcmp(console_vars[var].name, action) == 0)
        {
            switch (console_vars[var].type)
            {
                case CON_DWORD:
                    if (*ptr)
                    {
                        while (*ptr == '=' || *ptr == ' ') ptr++;
                        eval_expression (ptr, 0);
                        if (result.type != T_ERROR)
                            *(int *)console_vars[var].value = (int) result.value ;
                    }
                    gr_con_printf ("[FXI] %s = %d", console_vars[var].name, *(int *)console_vars[var].value) ;
                    return ;

                case CON_DWORD_HEX:
                    if (*ptr)
                    {
                        while (*ptr == '=' || *ptr == ' ') ptr++;
                        eval_expression (ptr, 0);
                        if (result.type != T_ERROR)
                            *(int *)console_vars[var].value = (int) result.value ;
                    }
                    gr_con_printf ("[FXI] %s = %08Xh\n", console_vars[var].name, *(int *)console_vars[var].value) ;
                    return ;
            }
        }
    }

    /* Expresiones */

    eval_expression (command, 1) ;
}

/* ---------------------------------------------------------------------- */
