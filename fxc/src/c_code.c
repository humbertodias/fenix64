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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fxc.h"
#include "messages.c"

extern int compile_fixed_expresion ();

/* Static utility function for compile_bestproc */
static void strdelchars (char * str, char * chars);

int reduce_arrays = 1;

/* ---------------------------------------------------------------------- */
/* Compilador de expresiones y sentencias. En este módulo están todas las */
/* funciones de compilado que generan código efectivo.                    */
/* ---------------------------------------------------------------------- */

PROCDEF * proc ;
CODEBLOCK * code ;

/* Comprueba que los parámetros de una expresión binaria sean datos
 * numéricos. Devuelve el tipo de operación (MN_FLOAT o MN_DWORD) */

static int check_integer_type (expresion_result *exp)
{
    if (typedef_is_pointer(exp->type))
    {
//        codeblock_add (code, MN_POINTER2BOL, 0) ;
        exp->type = typedef_new(TYPE_DWORD) ;
    }

    if (typedef_is_integer(exp->type))
    {
        if (typedef_base(exp->type) == TYPE_BYTE)
            return MN_BYTE ;
        if (typedef_base(exp->type) == TYPE_WORD)
            return MN_WORD ;
        return MN_DWORD ;
    }

    compile_error (MSG_INTEGER_REQUIRED) ;
    return 0;
}

static int check_integer_types (expresion_result *left, expresion_result *right)
{
    if (typedef_is_pointer(left->type))
    {
//        codeblock_add (code, MN_POINTER2BOL, 1) ;
        left->type = typedef_new(TYPE_DWORD) ;
    }

    if (typedef_is_pointer(right->type))
    {
//        codeblock_add (code, MN_POINTER2BOL, 0) ;
        right->type = typedef_new(TYPE_DWORD) ;
    }

    if (typedef_is_integer(left->type))
    {
        if (typedef_is_integer(right->type))
        {
            if (typedef_base(left->type) == typedef_base(right->type))
            {
                if (typedef_base(left->type) == TYPE_BYTE)
                    return MN_BYTE ;
                if (typedef_base(left->type) == TYPE_WORD)
                    return MN_WORD ;
            }
            return MN_DWORD ;
        }
    }

    compile_error (MSG_INTEGER_REQUIRED) ;
    return 0;
}

static int check_numeric_types (expresion_result *left, expresion_result *right)
{
    if (typedef_base(left->type) == TYPE_FLOAT)
    {
        if (typedef_base(right->type) == TYPE_FLOAT)
            return MN_FLOAT ;

        if (typedef_is_integer(right->type))
        {
            codeblock_add (code, MN_INT2FLOAT | mntype(right->type, 0), 0) ;
            right->fvalue = (float)right->value ;
            return MN_FLOAT ;
        }
    }

    if (typedef_is_integer(left->type))
    {
        if (typedef_is_integer(right->type))
        {
            if (typedef_base(left->type) == typedef_base(right->type))
                return mntype(left->type, 0);
            if (typedef_base(left->type) < typedef_base(right->type))
                return mntype(left->type, 0) ;
            return mntype(right->type, 0) ;
        }

        if (typedef_base(right->type) == TYPE_FLOAT)
        {
            codeblock_add (code, MN_INT2FLOAT, 1) ;
            left->fvalue = (float)left->value ;
            return MN_FLOAT ;
        }
    }

    if (left->type.chunk[0].type == TYPE_CHAR || right->type.chunk[0].type == TYPE_CHAR)
    {
        if (typedef_base(right->type) == TYPE_STRING)
        {
            codeblock_add (code, MN_STR2CHR, 0);
            if (right->constant == 1)
                right->value = (unsigned char)*(string_get(right->value));
            right->type = typedef_new(TYPE_CHAR);
            return MN_BYTE;
        }
        if (typedef_base(left->type) == TYPE_STRING)
        {
            codeblock_add (code, MN_STR2CHR, 1);
            if (left->constant == 1)
                left->value = (unsigned char)*(string_get(left->value));
            left->type = typedef_new(TYPE_CHAR);
            return MN_BYTE;
        }
        if (typedef_is_integer(right->type) || typedef_is_integer(right->type))
            return MN_BYTE;
        if (left->type.chunk[0].type == TYPE_CHAR && right->type.chunk[0].type == TYPE_CHAR)
            return MN_BYTE;
    }

    compile_error (MSG_INCOMP_TYPES) ;
    return 0 ;
}

/* Comprueba que los parámetros de una expresión binaria sean cadenas
 * o datos numéricos. Devuelve MN_STRING o el tipo de dato numérico */

static int check_numeric_or_string_types (expresion_result * left,
        expresion_result * right)
{
    if (typedef_is_array(left->type) && left->type.chunk[1].type == TYPE_CHAR && typedef_is_string(right->type))
    {
        left->type = typedef_new(TYPE_STRING);
        left->lvalue = 0;
        codeblock_add (code, MN_A2STR, 1) ;
        return MN_STRING;
    }
    if (typedef_is_array(right->type) && right->type.chunk[1].type == TYPE_CHAR && typedef_is_string(left->type))
    {
        right->type = typedef_new(TYPE_STRING);
        right->lvalue = 0;
        codeblock_add (code, MN_A2STR, 0) ;
        return MN_STRING;
    }
    if (typedef_is_array(right->type) && right->type.chunk[1].type == TYPE_CHAR &&
        typedef_is_array(left->type)  &&  left->type.chunk[1].type == TYPE_CHAR)
    {
        left->type = typedef_new(TYPE_STRING);
        right->type = typedef_new(TYPE_STRING);
        left->lvalue = 0;
        right->lvalue = 0;
        codeblock_add (code, MN_A2STR, 0) ;
        codeblock_add (code, MN_A2STR, 1) ;
        return MN_STRING;
    }
    if (typedef_is_string(left->type) && typedef_is_string(right->type))
        return MN_STRING ;
    if (typedef_is_string(left->type) || typedef_is_string(right->type))
        compile_error (MSG_INCOMP_TYPES) ;

    return check_numeric_types (left, right) ;
}

/* Devuelve el código que hay que adjuntar a un mnemónico para producir
 * una variante del mismo, adecuada al tipo de dato concreto */

int mntype(TYPEDEF type, int accept_structs)
{
    BASETYPE t ;

    while (typedef_is_array(type))
        type = typedef_reduce(type) ;
    t = typedef_base(type) ;

    if (t == TYPE_DWORD)   return MN_DWORD | MN_UNSIGNED;
    if (t == TYPE_INT)     return MN_DWORD;
    if (t == TYPE_WORD)    return MN_WORD | MN_UNSIGNED;
    if (t == TYPE_SHORT)   return MN_WORD ;
    if (t == TYPE_BYTE)    return MN_BYTE | MN_UNSIGNED;
    if (t == TYPE_SBYTE)   return MN_BYTE ;
    if (t == TYPE_CHAR)    return MN_BYTE ;
    if (t == TYPE_FLOAT)   return MN_FLOAT ;
    if (t == TYPE_STRING)  return MN_STRING;
    if (t == TYPE_POINTER) return MN_DWORD ;

    if (t == TYPE_STRUCT && accept_structs)
        return 0;

    compile_error (MSG_INCOMP_TYPE) ;
    return 0;
}

/* Compila el tamaño de una VARIABLE o estructura local, global o privada
Se agrega local al proceso, (Splinter)
*/

int compile_sizeof (VARSPACE * here)
{
    VARIABLE * var  = NULL ;
    expresion_result ind ;
    int offset, sub = 0, base = 0 ;
    TYPEDEF type, * usertype ;

    token_next() ;

    if (token.type == IDENTIFIER && (token.code == identifier_unsigned || token.code == identifier_signed)) { /* "UNSIGNED" or "SIGNED" */
        token_next();
    }

    if (token.type != IDENTIFIER) {
        compile_error (MSG_INCOMP_TYPE) ;
    }

    /* Base datatypes */

         if (token.code == identifier_pointer || token.code == identifier_multiply) base = 4 ;
    else if (token.code == identifier_dword)   base = 4 ;
    else if (token.code == identifier_int)     base = 4 ;
    else if (token.code == identifier_string)  base = 4 ;
    else if (token.code == identifier_float)   base = 4 ;
    else if (token.code == identifier_short)   base = 2 ;
    else if (token.code == identifier_word)    base = 2 ;
    else if (token.code == identifier_char)    base = 1 ;
    else if (token.code == identifier_byte)    base = 1 ;
    else {
        usertype = typedef_by_name(token.code) ;
        if (usertype) base = typedef_size(*usertype) ;
    }

    if (base) {
        for (;;) {
            token_next() ;
            if (token.type == IDENTIFIER && (token.code == identifier_pointer || token.code == identifier_multiply)) /* "POINTER" */
            {
                base = 4 ;
                continue ;
            }
            token_back() ;
            break ;
        }
        return base ;
    }

    if (!here) {
        // Splinter, se agrega localidad...
        here = proc->privars ;
        var = varspace_search (here, token.code) ;
        if (!var) {
            here = proc->pubvars ;
            var = varspace_search (here, token.code) ;
        }
        if (!var) {
            here = &local ;
            var = varspace_search (here, token.code) ;
        }
        if (!var) {
            here = &global ;
            var = varspace_search (here, token.code) ;
        }
    } else {
        var = varspace_search (here, token.code) ;
    }

    if (!var) compile_error (MSG_UNKNOWN_IDENTIFIER) ;

    token_next() ;

    /* Indexado de punteros ptr[0] */

    if (token.type == IDENTIFIER && token.code == identifier_leftb && typedef_is_pointer(var->type)) { /* "[" */
        CODEBLOCK_POS p = codeblock_pos(code);
        offset = code->current ;
        ind = compile_subexpresion() ;
        if (!typedef_is_integer(ind.type)) {
            compile_error (MSG_INTEGER_REQUIRED) ;
        }

        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_rightb) { /* "]" */
            compile_error (MSG_EXPECTED, "]") ;
        }
        codeblock_setpos(code, p);
        return typedef_size(typedef_reduce(var->type)) ;
    }

    /* Indexado de cadenas */

    if (token.type == IDENTIFIER && token.code == identifier_leftb && typedef_is_string(var->type)) { /* "[" */
        CODEBLOCK_POS p = codeblock_pos(code);
        ind = compile_subexpresion() ;
        if (!typedef_is_integer(ind.type)) {
            compile_error (MSG_INTEGER_REQUIRED) ;
        }

        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_rightb) { /* "]" */
            compile_error (MSG_EXPECTED, "]") ;
        }
        codeblock_setpos(code, p);
        return 1 ;
    }

    /* Indexado de arrays */

    type = var->type ;

    while (token.type == IDENTIFIER && token.code == identifier_leftb) /* "[" */
    {
        if (typedef_is_struct(type) && typedef_count(type) == 1) {
            compile_error (MSG_NOT_AN_ARRAY) ;
        }

        if (!typedef_is_struct(type) && !typedef_is_array (type)) {
            compile_error (MSG_NOT_AN_ARRAY) ;
        }

        if (code) {
            CODEBLOCK_POS p = codeblock_pos(code);
            ind = compile_expresion (0,0,TYPE_DWORD) ;
            codeblock_setpos(code, p);
        } else {
            ind = compile_expresion (0,0,TYPE_DWORD) ;
        }

        if (ind.constant && (ind.value < 0 || ind.value >= typedef_count(type))) {
            compile_error (MSG_BOUND) ;
        }

        type = typedef_reduce(type) ;

        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_rightb) { /* "]" */
            compile_error (MSG_EXPECTED, "]") ;
        }

        sub = 1 ;
        token_next() ;
    }

    if (token.type == IDENTIFIER && token.code == identifier_point) { /* "." */
        if (typedef_is_struct(type) || typedef_base(type) == TYPE_DWORD || typedef_base(type) == TYPE_INT) {
            return compile_sizeof (typedef_members(type)) ;
        }
        return compile_sizeof (&local) ;
    }

    token_back() ;
    return typedef_size (type) ;
}

/* Compila el acceso a una VARIABLE global, local, privada o publica */

expresion_result compile_sublvalue (VARSPACE * from, int base_offset, VARSPACE * remote)
{
    VARIABLE           * var = NULL ;
    VARSPACE           * here = from ;
    VARSPACE           * privars = (proc ? proc->privars : NULL) ;
    VARSPACE           * pubvars = (proc ? proc->pubvars : NULL) ;
    expresion_result    res, ind ;

    if (here) token_next() ;

    if (token.type != IDENTIFIER) {
        compile_error (MSG_IDENTIFIER_EXP) ;
    }

    if (!here && !remote) {
        // Splinter, se agrega localidad...
        if (proc) {
            here = privars ;
            var = varspace_search (here, token.code) ;
            if (!var) {
                here = pubvars ;
                var = varspace_search (here, token.code) ;
           }
        }
        if (!var) {
            here = &local ;
            var = varspace_search (here, token.code) ;
        }
        if (!var) {
            here = &global ;
            var = varspace_search (here, token.code) ;
        }
    }
    else
    {
        if (remote) {
            here = remote ;
            var = varspace_search (here, token.code) ;
        }

        if (!var) {
            here = from;
            var = varspace_search (here, token.code) ;
        }
    }

    if (!var) compile_error (MSG_UNKNOWN_IDENTIFIER) ;

    if (var->offset - base_offset != 0 ||
             here == &global ||
             here == &local  ||
             (here == privars && privars) ||
             (here == pubvars && pubvars) ||
             (remote && here == remote)
       )
    {
        codeblock_add (code, (
                              (remote && here == remote)        ? MN_REMOTE_PUBLIC  :
                               here == &global                  ? MN_GLOBAL         :
                              (here == &local && from == here)  ? MN_REMOTE         :
                               here == &local                   ? MN_LOCAL          :
                              (here == privars && privars)      ? MN_PRIVATE        :
                              (here == pubvars && pubvars)      ? MN_PUBLIC         :
                                                                  MN_INDEX
                             ) | mntype(var->type, 1), var->offset - base_offset ) ;

        if ((here == pubvars && pubvars)||(remote && here == remote)){ /* Tambien las locales remotas ? */
            proc->flags |= PROC_USES_PUBLICS;
        }

        if (here == &local) {
            proc->flags |= PROC_USES_LOCALS;
        }
    }

    token_next() ;

    res.type       = var->type ;
    res.lvalue     = 1 ;
    res.asignation = 0 ;
    res.constant   = 0 ;
    res.call       = 0 ;
    res.value      = 0 ;

    /* Indexado vía [...] */

    while (token.type == IDENTIFIER && token.code == identifier_leftb) /* "[" */
    {
        /* De estructuras o arrays */

        if (typedef_is_struct(res.type)  && typedef_count(res.type) == 1)
            compile_error (MSG_NOT_AN_ARRAY) ;

        /* Cadenas y punteros se indexan en otro nivel */

        if (typedef_is_pointer(res.type) || typedef_is_string(res.type)) {
            break ;
        }

        if (!typedef_is_struct(res.type) && !typedef_is_array (res.type)) {
            compile_error (MSG_NOT_AN_ARRAY) ;
        }

        ind = compile_expresion (0,0,TYPE_DWORD) ;
        if (ind.lvalue) {
            codeblock_add (code, MN_PTR | mntype(ind.type, 0), 0) ;
        }

        if (ind.constant && (ind.value < 0 || ind.value >= typedef_count(res.type))) {
            compile_error (MSG_BOUND) ;
        }

        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_rightb) { /* "]" */
            compile_error (MSG_EXPECTED, "]") ;
        }

        if (typedef_is_array(res.type))
        {
            res.type = typedef_reduce(res.type) ;
            codeblock_add (code, MN_ARRAY, typedef_size(res.type)) ;
        }
        else /* estructura */
        {
            codeblock_add (code, MN_ARRAY, typedef_size(res.type) / typedef_count(res.type)) ;
        }

        token_next() ;
    }

    /* Un acceso a un array es un acceso a su primer elemento */
    res.count=1;
    if (typedef_is_array(res.type) && reduce_arrays == 1)
    {
        if (res.type.chunk[1].type != TYPE_CHAR) {
            while (typedef_is_array(res.type)) {
                res.count *= (typedef_count(res.type) ? typedef_count(res.type) : 1);
                res.type = typedef_reduce(res.type) ;
            }
        }
    }

    token_back() ;

    return res ;
}

/*
 *  FUNCTION : compile_bestproc
 *
 *  Compile a system call, given a list of system functions
 *  with the same name
 *
 *  PARAMS:
 *      procs           List of system functions
 *
 *  RETURN VALUE:
 *      Identifier code allocated for the function
 */

SYSPROC * compile_bestproc (SYSPROC ** procs)
{
    int n, proc_count = 0 ;
    expresion_result res ;
    int count = 0 ;
    char validtypes[32] ;
    char type = -1 ;
    int min_params = 0 ;
    const char * proc_name = procs[0]->name ;

    while (procs[proc_count]) proc_count++ ;

    /* Get the minimum number of parameters */

    for (n = 0 ; n < proc_count ; n++)
        if (procs[n]->params > min_params)
            min_params = procs[n]->params ;

    for (;;)
    {
        token_next() ;
        if (token.type == IDENTIFIER && token.code == identifier_rightp) /* ")" */
        {
            token_back() ;
            break ;
        }
        token_back() ;

        count++ ;

        /* Eliminate any process that has not as many parameters */

        for (n = 0 ; n < proc_count ; n++)
        {
            if (procs[n]->params < count)
            {
                memmove (&procs[n], &procs[n+1], sizeof(SYSPROC*) * (proc_count-n)) ;
                proc_count-- ;
                n-- ;
            }
        }

        if (proc_count == 0) {
            compile_error (MSG_INCORRECT_PARAMC, proc_name, min_params) ;
        }

        /* Find all the available types */

        validtypes[0] = 0 ;
        for (n = 0 ; n < proc_count ; n++)
        {
            if (!strchr(validtypes, procs[n]->paramtypes[count-1]))
            {
                validtypes[strlen(validtypes)+1] = 0 ;
                validtypes[strlen(validtypes)] = procs[n]->paramtypes[count-1];
            }
        }

        if (strlen(validtypes) == 1)
        {
            /* Same type for any function variant */

            if (validtypes[0] == 'V')
            {
                /* Function will receive a varspace struct */
                reduce_arrays = 0;
                res = compile_expresion (0, 1, 0);
                reduce_arrays = 1;

                while (typedef_is_pointer(res.type))
                {
                    codeblock_add (code, MN_PTR, 0);
                    res.type = typedef_reduce(res.type);
                }
                if (typedef_is_struct(res.type))
                {
                    int size = res.type.varspace->count * sizeof(DCB_TYPEDEF);
                    int nvar;

                    segment_alloc (globaldata, size);
                    codeblock_add (code, MN_GLOBAL, globaldata->current) ;
                    for (nvar = 0 ; nvar < res.type.varspace->count ; nvar++)
                    {
                        DCB_TYPEDEF type;
                        dcb_settype (&type, &res.type.varspace->vars[nvar].type);
                        memcpy ((Uint8*)globaldata->bytes + globaldata->current, &type, sizeof(DCB_TYPEDEF));
                        globaldata->current += sizeof(DCB_TYPEDEF);
                    }
                    codeblock_add (code, MN_PUSH | MN_DWORD, res.type.varspace->count);
                    count += 2;
                }
                else
                {
                    DCB_TYPEDEF type;
                    dcb_settype (&type, &res.type);
                    segment_alloc (globaldata, sizeof(TYPEDEF));
                    codeblock_add (code, MN_GLOBAL, globaldata->current) ;
                    memcpy ((Uint8*)globaldata->bytes + globaldata->current, &type, sizeof(DCB_TYPEDEF));
                    globaldata->current += sizeof(DCB_TYPEDEF);
                    codeblock_add (code, MN_PUSH | MN_DWORD, 1);
                    count += 2;
                }
            }
            else
            {
                switch (validtypes[0])
                {
                    case 'I': type = TYPE_DWORD   ; break ;
                    case 'B': type = TYPE_BYTE    ; break ;
                    case 'W': type = TYPE_WORD    ; break ;
                    case 'S': type = TYPE_STRING  ; break ;
                    case 'P': type = TYPE_POINTER ; break ;
                    case 'F': type = TYPE_FLOAT   ; break ;
                    default:  compile_error (MSG_INVALID_PARAMT) ;
                }

                res = compile_expresion(0,0,type) ;
                if (res.lvalue) codeblock_add (code, mntype(res.type, 0) | MN_PTR, 0) ;
            }
        }
        else
        {
            /* Different types availables */

            res = compile_expresion(0,0,TYPE_UNDEFINED) ;
            if (res.lvalue) codeblock_add (code, mntype(res.type, 0) | MN_PTR, 0) ;

            /* Eliminate any incompatible data type, but allow some
             * conversions if no exact match is available */

            switch (typedef_base(res.type))
            {
                case TYPE_DWORD:
                case TYPE_SHORT:
                case TYPE_BYTE:
                case TYPE_SBYTE:
                case TYPE_WORD:
                case TYPE_INT:
                    strdelchars (validtypes, "SFP") ;
                    break ;
                case TYPE_FLOAT:
                    if (strchr (validtypes, 'F'))
                        strdelchars (validtypes, "SPIWB") ;
                    else
                        strdelchars (validtypes, "SP") ;
                    break ;
                case TYPE_STRING:
                    if (strchr (validtypes, 'S'))
                        strdelchars (validtypes, "FPIWB") ;
                    else
                        strdelchars (validtypes, "P") ;
                    break ;
                case TYPE_POINTER:
                    strdelchars (validtypes, "SFIWB") ;
                    break ;
                default:
                    break ;
            }

            if (strlen(validtypes) != 1) compile_error (MSG_INVALID_PARAMT) ;

            /* Eliminate all functions that are not selected */

            for (n = 0 ; n < proc_count ; n++)
            {
                if (procs[n]->paramtypes[count-1] != validtypes[0])
                {
                    memmove (&procs[n], &procs[n+1], sizeof(SYSPROC*) * (proc_count-n)) ;
                    proc_count-- ;
                    n-- ;
                }
            }

            /* Convert the result to the appropiate type, if needed */

            switch (validtypes[0])
            {
                case 'I': type = TYPE_DWORD   ; break ;
                case 'B': type = TYPE_BYTE    ; break ;
                case 'W': type = TYPE_WORD    ; break ;
                case 'S': type = TYPE_STRING  ; break ;
                case 'P': type = TYPE_POINTER ; break ;
                case 'F': type = TYPE_FLOAT   ; break ;
                default:  compile_error (MSG_INVALID_PARAMT) ;
            }
            res = convert_result_type (res, type) ;
        }

        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_comma) /* "," */
        {
            token_back() ;
            break ;
        }
    }

    /* Eliminate any process that has too many parameters */

    for (n = 0 ; n < proc_count ; n++)
    {
        if (procs[n]->params != count)
        {
            memmove (&procs[n], &procs[n+1], sizeof(SYSPROC*) * (proc_count-n)) ;
            proc_count-- ;
            n-- ;
        }
    }

    if (proc_count > 1) compile_error (MSG_MULTIPLE_PROCS_FOUND, proc_name);
    if (proc_count == 0) compile_error (MSG_INCORRECT_PARAMC, proc_name, min_params) ;
    codeblock_add (code, MN_SYSCALL, procs[0]->code) ;

    return procs[0] ;
}

static void strdelchars (char * str, char * chars)
{
    while (*str)
    {
        if (strchr(chars, *str))
            strcpy (str, str+1) ;
        else
            str++ ;
    }
}

/* Compila una lista de parámetros */

int compile_paramlist (BASETYPE * types, const char * paramtypes)
{
    expresion_result res ;
    int count = 0, type ;

    for (;;)
    {
        type = types ? *types : TYPE_UNDEFINED ;
        if (paramtypes)
        {
            switch (*paramtypes++)
            {
                case 'I': type = TYPE_DWORD   ; break ;
                case 'B': type = TYPE_BYTE    ; break ;
                case 'W': type = TYPE_WORD    ; break ;
                case 'S': type = TYPE_STRING  ; break ;
                case 'P': type = TYPE_POINTER ; break ;
                case 'F': type = TYPE_FLOAT   ; break ;
                default:  compile_error (MSG_INVALID_PARAMT) ;
            }
        }

        res = compile_expresion(0,0,type) ;

        if (types)
        {
            if (*types == TYPE_UNDEFINED)
                *types = typedef_base (res.type) ;
            types++ ;
        }
        if (res.lvalue) codeblock_add (code, mntype(res.type, 0) | MN_PTR, 0) ;
        count++ ;

        token_next() ;
        if (token.type == IDENTIFIER && token.code == identifier_comma) /* "," */
            continue ;
        token_back() ;
        break ;
    }
    return count ;
}

/*
 *  FUNCTION : compile_cast
 *
 *  Compiles a cast operator (c-like type conversion: ([type])value)
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      Returns the expression result
 *
 */

expresion_result compile_cast ()
{
    TYPEDEF  type;
    BASETYPE basetype = TYPE_INT;
    int      tokens   = 0;

    int      signed_prefix = 0;
    int      unsigned_prefix = 0;

    expresion_result res;

    token_next();

    // Check for signed/unsigned prefix

    if (token.type == IDENTIFIER)
    {
        if (token.code == identifier_signed) {
            signed_prefix = 1;
            tokens++;
            token_next();
        } else if (token.code == identifier_unsigned) {
            unsigned_prefix = 1;
            tokens++;
            token_next();
        }
    }

    // Create the data type definition

    if (token.type == IDENTIFIER)
    {
        if (token.code == identifier_dword)
        {
            basetype = signed_prefix ? TYPE_INT : TYPE_DWORD;
            signed_prefix = unsigned_prefix = 0;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_word)
        {
            basetype = signed_prefix ? TYPE_SHORT : TYPE_WORD;
            signed_prefix = unsigned_prefix = 0;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_byte)
        {
            basetype = signed_prefix ? TYPE_SBYTE : TYPE_BYTE;
            signed_prefix = unsigned_prefix = 0;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_int)
        {
            basetype = unsigned_prefix ? TYPE_DWORD : TYPE_INT;
            signed_prefix = unsigned_prefix = 0;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_short)
        {
            basetype = unsigned_prefix ? TYPE_WORD : TYPE_SHORT;
            signed_prefix = unsigned_prefix = 0;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_char)
        {
            basetype = TYPE_CHAR;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_float)
        {
            basetype = TYPE_FLOAT ;
            tokens++;
            token_next() ;
        }
        else if (token.code == identifier_string)
        {
            basetype = TYPE_STRING ;
            tokens++;
            token_next() ;
        }
    }

    // Don't allow a signed/unsigned prefix in non-integer types

    if (signed_prefix || unsigned_prefix)
    {
        compile_error (MSG_INVALID_TYPE);
    }

    // If type is not a basic one: check for user-defined types

    if (tokens == 0)
    {
        TYPEDEF * typep = typedef_by_name(token.code);
        if (typep == NULL)
        {
            type = typedef_new(TYPE_INT);
            compile_error(MSG_INVALID_TYPE);
        }
        else
            type = *typep;
    }
    else
        type = typedef_new(basetype);

    // Check for pointers to defined types

    while (token.type == IDENTIFIER && (token.code == identifier_pointer || token.code == identifier_multiply)) /* "POINTER" */
    {
        type = typedef_pointer(type);
        tokens++;
        token_next() ;
    }

    // Check for the right parent

    if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
        compile_error(MSG_EXPECTED, ")");

    // Do the cast

    res = compile_value();

    if (typedef_is_equal(res.type, type))
        return res;

    if (typedef_is_pointer(type))
    {
        // Conversion of pointer to pointer of another type

        if (typedef_is_pointer(res.type))
        {
            res.type = type;
            return res;
        }
        compile_error(MSG_PTR_CONVERSION);
    }
    else if (typedef_is_float(type))
    {
        // Conversion of integer to float

        if (typedef_is_integer(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_INT2FLOAT, 0);
            res.type = type;
        }
        else
            compile_error(MSG_CONVERSION);
    }
    else if (type.chunk[0].type == TYPE_CHAR)
    {
        if (typedef_is_string(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR | MN_STRING, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_STR2INT, 0);
            res.type = typedef_new(TYPE_INT);
        }
        else
        {
            compile_error(MSG_CONVERSION);
        }
    }
    else if (typedef_is_integer(type))
    {
        // Conversion of float, string or integer to integer

        if (typedef_is_float(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR | MN_FLOAT, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_FLOAT2INT, 0);
            res.type = typedef_new(TYPE_INT);
        }
        else if (typedef_is_string(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR | MN_STRING, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_STR2INT, 0);
            res.type = typedef_new(TYPE_INT);
        }
        else if (typedef_is_array(res.type) && res.type.chunk[1].type == TYPE_CHAR)
        {
            codeblock_add (code, MN_A2STR, 0);
            codeblock_add (code, MN_STR2INT, 0);
            res.type = typedef_new(TYPE_INT);
        }
        else if (typedef_is_integer(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR, 0);
                res.lvalue = 0;
            }
        }
        else
        {
            compile_error(MSG_CONVERSION);
        }

        if (type.chunk[0].type == TYPE_BYTE && typedef_is_integer(res.type))
        {
            codeblock_add (code, MN_INT2BYTE, 0);
            res.type = type;
        }
        else if (type.chunk[0].type == TYPE_SBYTE && typedef_is_integer(res.type))
        {
            codeblock_add (code, MN_INT2BYTE, 0);
            res.type = type;
        }
        else if (type.chunk[0].type == TYPE_WORD && typedef_is_integer(res.type))
        {
            codeblock_add (code, MN_INT2WORD, 0);
            res.type = type;
        }
        else if (type.chunk[0].type == TYPE_SHORT && typedef_is_integer(res.type))
        {
            codeblock_add (code, MN_INT2WORD, 0);
            res.type = type;
        }
    }
    else if (typedef_is_string(type))
    {
        // Conversión de puntero, float, entero o cadena de ancho fijo a cadena

        if (typedef_is_array(res.type) && res.type.chunk[1].type == TYPE_CHAR)
        {
            codeblock_add (code, MN_A2STR, 0);
            res.type = typedef_new(TYPE_STRING);
        }
        else if (typedef_is_pointer(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_A2STR, 0);
            res.type = typedef_new(TYPE_STRING);
        }
        else if (res.type.chunk[0].type == TYPE_CHAR)
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_CHR2STR, 0);
            res.type = typedef_new(TYPE_STRING);
        }
        else if (typedef_is_integer(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_INT2STR | mntype(res.type, 0), 0);
            res.type = typedef_new(TYPE_STRING);
        }
        else if (typedef_is_float(res.type))
        {
            if (res.lvalue)
            {
                codeblock_add (code, MN_PTR, 0);
                res.lvalue = 0;
            }
            codeblock_add (code, MN_FLOAT2STR, 0);
            res.type = typedef_new(TYPE_STRING);
        }
        else
            compile_error(MSG_CONVERSION);
    }
    else
        compile_error(MSG_CONVERSION);

    return res;
}

/* Compila un valor (elemento más pequeño del lenguaje) */

expresion_result compile_value ()
{
    CONSTANT * c ;
    SYSPROC * sysproc ;
    PROCDEF * cproc ;
    int param_count, id ;

    expresion_result res ;

    token_next() ;

    /* ( ... ) */

    // Minima Optimizacion (Splinter)
    if (token.type == IDENTIFIER ) {

        if (token.code == identifier_leftp) /* "(" */
        {
            /* Check for cast-type expressions */

            token_next();
            if (token.type == IDENTIFIER && identifier_is_type(token.code))
            {
                token_back();
                return compile_cast();
            }
            token_back();

            res = compile_subexpresion() ;
            token_next() ;
            if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                compile_error (MSG_EXPECTED, ")") ;
            return res ;
        }

        /* TYPE */

        if (token.code == identifier_type) /* "TYPE" */
        {
            token_next( );
            /* "TYPE mouse" */
            if (token.type == IDENTIFIER && token.code == identifier_mouse) /* "MOUSE" */
            {
                codeblock_add (code, MN_PUSH, -1) ;
                res.value      = -1 ;
                res.lvalue     = 0 ;
                res.constant   = 1 ;
                res.asignation = 0 ;
                res.call       = 0 ;
                res.type       = typedef_new(TYPE_INT) ;
                return res ;
            }
            if (token.type != IDENTIFIER || token.code < reserved_words)
                compile_error (MSG_PROCESS_NAME_EXP) ;

            codeblock_add (code, MN_TYPE, token.code) ;
            res.value      = 0 ;
            res.lvalue     = 0 ;
            res.constant   = 0 ;
            res.asignation = 0 ;
            res.call       = 0 ;
            res.type       = typedef_new(TYPE_INT) ;
            return res ;
        }

        /* OFFSET */

        if (token.code == identifier_offset || token.code == identifier_bandoffset) /* "OFFSET" or "&" */
        {
            res = compile_factor() ; /* Para permitir &a.b */
            if (!res.lvalue)
                compile_error (MSG_NOT_AN_LVALUE) ;

            res.lvalue = 0 ;
            res.type   = typedef_pointer(res.type) ;
            return res ;
        }

        /* POINTER */

        if (token.code == identifier_leftb) /* "[" */
        {
            res = compile_subexpresion() ;
            if (!typedef_is_pointer(res.type))
                compile_error (MSG_NOT_A_POINTER) ;

            if (res.lvalue) codeblock_add (code, MN_PTR, 0) ;

            res.type = typedef_reduce(res.type) ;
            token_next() ;

            if (token.type != IDENTIFIER || token.code != identifier_rightb) /* "]" */
                compile_error (MSG_EXPECTED, "]") ;

            res.lvalue = 1 ;
            return res ;
        }

        if (token.code == identifier_pointer || token.code == identifier_multiply) /* "POINTER" or "*" */
        {
            res = compile_factor() ; /* Para aceptar *ptr++ */
            if (!typedef_is_pointer(res.type))
                compile_error (MSG_NOT_A_POINTER) ;

            if (res.lvalue) codeblock_add (code,  MN_PTR, 0) ;

            res.type = typedef_reduce(res.type) ;
            res.lvalue = 1 ;
            return res ;
        }

        /* SIZEOF */

        if (token.code == identifier_sizeof) /* "SIZEOF" */
        {
            token_next() ;
            if (token.type != IDENTIFIER || token.code != identifier_leftp) /* "(" */
                compile_error (MSG_EXPECTED, "(") ;

            res.value      = compile_sizeof(0) ;
            res.lvalue     = 0 ;
            res.constant   = 1 ;
            res.asignation = 0 ;
            res.call       = 0 ;
            res.type   = typedef_new(TYPE_DWORD) ;
            codeblock_add (code, MN_PUSH, res.value) ;
            token_next() ;

            if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                compile_error (MSG_EXPECTED, ")") ;

            return res ;
        }
    }

    /* Numbers */

    if (token.type == NUMBER)
    {
        codeblock_add (code, MN_PUSH, token.code) ;
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.constant   = 1 ;
        res.call       = 0 ;
        res.value      = token.code ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }

    if (token.type == FLOAT)
    {
        codeblock_add (code, MN_PUSH | MN_FLOAT, *(int *)&token.value) ;
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.constant   = 1 ;
        res.call       = 0 ;
        res.fvalue     = token.value ;
        res.type       = typedef_new(TYPE_FLOAT) ;
        return res ;
    }

    /* Strings */

    if (token.type == STRING)
    {
        codeblock_add (code, MN_STRING | MN_PUSH, token.code) ;
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.constant   = 1 ;
        res.call       = 0 ;
        res.value      = token.code ;
        res.type       = typedef_new(TYPE_STRING) ;
        return res ;
    }

    /* Constants */

    if (token.type != IDENTIFIER)
        compile_error (MSG_UNKNOWN_IDENTIFIER) ;

    c = constants_search (token.code) ;

    if (c)
    {
        if (typedef_is_string(c->type))
            codeblock_add (code, MN_PUSH | MN_STRING, c->value) ;
        else
            codeblock_add (code, MN_PUSH, c->value) ;
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.constant   = 1 ;
        res.call       = 0 ;
        res.value      = c->value ;
        res.fvalue     = *(float *)&c->value;
        res.type       = c->type ;
        return res ;
    }

    /* Llamada a un procedimiento o función del sistema */

    id = token.code ;

    token_next() ;

    // Minima Optimizacion (Splinter)
    if (token.type == IDENTIFIER ) {
        if (token.code == identifier_leftp) /* "(" */
        {
            SYSPROC ** sysproc_list = sysproc_getall(id) ;

            if (sysproc_list)
            {
                sysproc = compile_bestproc (sysproc_list);
                free (sysproc_list);

                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                    compile_error (MSG_EXPECTED, ")") ;

                res.lvalue     = 0 ;
                res.asignation = 0 ;
                res.constant   = 0 ;
                res.call       = 1 ;
                res.value      = 0 ;
                res.type       = typedef_new(sysproc->type) ;
                return res ;
            }

            /* Llama a un procedimiento del usuario */

            cproc = procdef_search (id) ;
            if (!cproc) cproc = procdef_new (procdef_getid(), id) ;

            token_next() ;

            if (token.type != IDENTIFIER || token.code != identifier_rightp) { /* ")" */
                token_back() ;
                param_count = compile_paramlist (cproc->paramtype, 0) ;
                token_next() ;
            } else {
                param_count = 0 ;
            }

            if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                compile_error (MSG_EXPECTED, ")") ;

            if (cproc->params == -1) {
                cproc->params = param_count ;
            } else if (cproc->params != param_count) {
                compile_error (MSG_INCORRECT_PARAMC, identifier_name(cproc->identifier), cproc->params ) ;
            }

            codeblock_add (code, MN_CALL, id) ;
            res.lvalue     = 0 ;
            res.asignation = 0 ;
            res.constant   = 0 ;
            res.call       = 1 ;
            res.value      = 0 ;
            res.type       = typedef_new(cproc->type) ;
            return res ;
        }
    }

    token_back() ;

    /* Valor asignable */

    return compile_sublvalue (0, 0, NULL) ;

}

expresion_result compile_factor ()
{
    expresion_result res, part ;

    token_next() ;

    res.lvalue     = 0 ;
    res.call       = 0 ;
    res.constant   = 0 ;
    res.asignation = 0 ;

    /* "+2" */
    if (token.type == IDENTIFIER && token.code == identifier_plus) /* "+" */
    {
        token_next() ;
    }

    /* "-2" */
    if (token.type == IDENTIFIER && token.code == identifier_minus) /* "-" */
    {
        part = compile_factor() ;
        if (part.lvalue) codeblock_add (code, mntype(part.type, 0) | MN_PTR, 0) ;
        codeblock_add (code, (mntype(part.type, 0) == MN_FLOAT ? MN_FLOAT : 0) | MN_NEG, 0) ;
        res.type = part.type ;
        if (typedef_is_integer(part.type) ||
            typedef_base(part.type) == TYPE_FLOAT)
        {
            res.constant = part.constant ;
            res.value    = -part.value ;
            res.fvalue   = -part.fvalue ;
            return res ;
        }
        compile_error (MSG_NUMBER_REQUIRED) ;
    }
    else if (token.type == IDENTIFIER && token.code == identifier_not) /* "NOT" or "!" */
    {
        part = compile_factor() ;
        if (part.lvalue) codeblock_add (code, (mntype(part.type, 0) == MN_FLOAT ? MN_FLOAT : 0) | MN_PTR, 0) ;
        if (typedef_is_pointer(part.type))
        {
//            codeblock_add (code, mntype(part.type, 0) | MN_POINTER2BOL, 0) ;
            part.type = typedef_new(TYPE_DWORD) ;
        }
        codeblock_add (code, (mntype(part.type, 0) == MN_FLOAT ? MN_FLOAT : 0) | MN_NOT, 0) ;
        if (typedef_is_integer(part.type) ||
            typedef_is_pointer(part.type) ||
            typedef_base(part.type) == TYPE_FLOAT)
        {
            res.constant = part.constant ;
            res.value    = !part.value ;
            res.fvalue   = (float)!part.fvalue ;
            res.type     = part.type ;
            return res ;
        }
        compile_error (MSG_NUMBER_REQUIRED) ;
        return res ;
    }
    else if (token.type == IDENTIFIER && token.code == identifier_bnot) /* "BNOT" or "~" */
    {
        part = compile_factor() ;
        if (part.lvalue) codeblock_add (code, (mntype(part.type, 0) == MN_FLOAT ? MN_FLOAT : 0) | MN_PTR, 0) ;
        if (typedef_is_pointer(part.type))
        {
//            codeblock_add (code, mntype(part.type, 0) | MN_POINTER2BOL, 0) ;
            part.type = typedef_new(TYPE_DWORD) ;
        }
        codeblock_add (code, (mntype(part.type, 0) == MN_FLOAT ? MN_FLOAT : 0) | MN_BNOT, 0) ;
        if (typedef_is_integer(part.type))
        {
            res.constant = part.constant ;
            res.value    = ~part.value ;
            res.type     = typedef_new(TYPE_INT) ;
            return res ;
        }
        compile_error (MSG_NUMBER_REQUIRED) ;
        return res ;
    }
    else if (token.type == IDENTIFIER && token.code == identifier_plusplus) /* "++" */
    {
        part = compile_factor() ;
        if (!part.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
        if (typedef_is_string(part.type)) {
            compile_error (MSG_INCOMP_TYPE) ;
        }
        if (typedef_is_pointer(part.type))
             codeblock_add (code,  MN_INC, typedef_size(typedef_reduce(part.type))) ;
        else codeblock_add (code, mntype(part.type, 0) | MN_INC, 1) ;
        res.asignation = 1 ;
        res.lvalue = 1 ;
        res.type = part.type ;
        return res ;
    }
    else if (token.type == IDENTIFIER && token.code == identifier_minusminus) /* "--" */
    {
        part = compile_factor() ;
        if (!part.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
        if (typedef_is_string(part.type)) {
            compile_error (MSG_INCOMP_TYPE) ;
        }
        if (typedef_is_pointer(part.type))
             codeblock_add (code,  MN_DEC, typedef_size(typedef_reduce(part.type))) ;
        else codeblock_add (code, mntype(part.type, 0) | MN_DEC, 1) ;
        res.asignation = 1 ;
        res.lvalue = 1 ;
        res.type = part.type ;
        return res ;
    }
    token_back() ;

    part = compile_value() ;

    /* Sufijos (operadores ., [], etc) */

    for (;;)
    {
        token_next() ;

        /* Operador "." */

        if (token.type == IDENTIFIER && token.code == identifier_point) /* "." */
        {
            if (typedef_is_pointer(part.type))
            {
                part.type = typedef_reduce(part.type) ;
                if (!typedef_is_struct(part.type))
                    compile_error (MSG_STRUCT_REQUIRED) ;
                codeblock_add (code, MN_PTR, 0) ;
            }

            if (typedef_is_struct(part.type))
            {
                VARSPACE * v = typedef_members(part.type) ;
                if (!v->vars) compile_error (MSG_STRUCT_REQUIRED) ;
                part = compile_sublvalue (v, v->vars[0].offset, NULL) ;
            } else {
                VARSPACE * v = typedef_members(part.type) ;
                if (typedef_base(part.type) != TYPE_DWORD && typedef_base(part.type) != TYPE_INT)
                    compile_error (MSG_INTEGER_REQUIRED) ;
                if (part.lvalue) codeblock_add (code, MN_PTR, 0) ;

                part = compile_sublvalue (&local, 0, v) ;  /* referenciada REMOTA por proceso (Splinter) */
            }
            continue ;
        }
        else if (token.type == IDENTIFIER && token.code == identifier_plusplus) /* Operador ++ posterior */
        {
            if (!part.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
            if (typedef_is_string(part.type)) {
                compile_error (MSG_INCOMP_TYPE) ;
            }
            if (typedef_is_pointer(part.type))
                 codeblock_add (code,  MN_POSTINC, typedef_size(typedef_reduce(part.type))) ;
            else codeblock_add (code, mntype(part.type, 0) | MN_POSTINC, 1) ;
            part.asignation = 1 ;
            part.lvalue = 0 ;
            continue ;
        }
        else if (token.type == IDENTIFIER && token.code == identifier_minusminus) /* Operador -- posterior */
        {
            if (!part.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
            if (typedef_is_string(part.type)) {
                compile_error (MSG_INCOMP_TYPE) ;
            }
            if (typedef_is_pointer(part.type))
                 codeblock_add (code,  MN_POSTDEC, typedef_size(typedef_reduce(part.type))) ;
            else codeblock_add (code, mntype(part.type, 0) | MN_POSTDEC, 1) ;
            part.asignation = 1 ;
            part.lvalue = 0 ;
            continue ;
        }

        /* Indexado vía [...] */

        if (token.type == IDENTIFIER && token.code == identifier_leftb) /* "[" */
        {
            /* De punteros */

            if (typedef_is_pointer(part.type))
            {
                if (part.lvalue) codeblock_add (code, MN_PTR, 0) ;
                part.type   = typedef_reduce(part.type) ;
                res = compile_subexpresion() ;
                if (!typedef_is_integer(res.type))
                    compile_error (MSG_INTEGER_REQUIRED) ;
                if (res.lvalue) codeblock_add (code, mntype(res.type, 0) | MN_PTR, 0) ;
                codeblock_add (code, MN_ARRAY, typedef_size(part.type)) ;
                part.lvalue = 1 ;
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_rightb) /* "]" */
                    compile_error (MSG_EXPECTED, "]") ;
                continue ;
            }

            /* De cadenas */

            if (typedef_is_string(part.type))
            {
                if (part.lvalue) codeblock_add (code, MN_STRING | MN_PTR, 0) ;
                res = compile_subexpresion() ;
                if (!typedef_is_integer(res.type))
                    compile_error (MSG_INTEGER_REQUIRED) ;
                if (res.lvalue) codeblock_add (code, mntype(res.type, 0) | MN_PTR, 0) ;
                codeblock_add (code, MN_CHRSTR, 0) ;
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_rightb) /* "]" */
                    compile_error (MSG_EXPECTED, "]") ;
                part.type   = typedef_new(TYPE_STRING) ;
                part.lvalue = 0 ;
            }
            continue ;
        }

        break ;
    }

    token_back() ;
    return part ;
}

expresion_result compile_operand ()
{
    expresion_result left, right, res ;
    int op ;
    BASETYPE t ;

    left = compile_factor() ;

    for (;;)
    {
        token_next() ;
        if (token.type == IDENTIFIER && token.code == identifier_multiply) /* "*" */
        {
            if (left.lvalue) codeblock_add (code, mntype(left.type, 0) | MN_PTR, 0) ;
            right = compile_factor() ;
            if (right.lvalue) codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;

            t = check_numeric_types (&left, &right) ;
            codeblock_add (code, MN_MUL | t, 0) ;

            res.constant   = (right.constant && left.constant) ;

            if (t == MN_FLOAT)
            {
                res.fvalue = left.fvalue * right.fvalue ;
                res.type   = typedef_new (TYPE_FLOAT) ;
            }
            else
            {
                res.type   = typedef_new (TYPE_INT) ;
                res.value  = left.value * right.value ;
            }

            res.lvalue     = 0 ;
            res.asignation = 0 ;
            res.call       = 0 ;
            left = res ;
            continue ;
        }
        else if (token.type == IDENTIFIER && (token.code == identifier_divide || token.code == identifier_mod)) /* "/" or "%" */
        {
            op = token.code == identifier_mod ? MN_MOD : MN_DIV ;
            if (op == MN_MOD && typedef_base(left.type) == TYPE_FLOAT) {
                compile_error (MSG_INTEGER_REQUIRED) ;
            }
            if (left.lvalue) codeblock_add (code, mntype(left.type, 0) | MN_PTR, 0) ;
            right = compile_factor() ;
            if (op == MN_MOD && typedef_base(right.type) == TYPE_FLOAT)
                compile_error (MSG_INTEGER_REQUIRED) ;
            if (right.lvalue) codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;
            t = check_numeric_types (&left, &right) ;
            codeblock_add (code, op | t, 0) ;
            res.constant   = (right.constant && left.constant) ;
            res.lvalue     = 0 ;
            res.asignation = 0 ;
            res.call       = 0 ;
            res.type       = typedef_new(t == MN_FLOAT ? TYPE_FLOAT:TYPE_INT) ;

            if (res.constant)
            {
                if (t == MN_FLOAT)
                {
                    if (right.fvalue == 0.0)
                        compile_error (MSG_DIVIDE_BY_ZERO) ;
                    if (op == MN_MOD)
                        compile_error (MSG_NUMBER_REQUIRED) ;
                    res.fvalue = left.fvalue / right.fvalue ;
                    res.type = typedef_new(TYPE_FLOAT) ;
                }
                else
                {
                    if (right.value == 0)
                        compile_error (MSG_DIVIDE_BY_ZERO) ;
                    res.value = op == MN_MOD ? left.value % right.value : left.value / right.value ;
                    res.type = typedef_new(TYPE_INT) ;
                }
            }
            left = res ;
            continue ;
        }
        token_back() ;
        break ;
    }
    return left ;
}

expresion_result compile_operation ()
{
    expresion_result left, right, res ;
    int op ;
    BASETYPE t ;

    left = compile_operand() ;

    for (;;)
    {
        token_next() ;

        /* Suma (o resta) de un entero a un puntero */

        if (typedef_is_pointer(left.type) && token.type == IDENTIFIER &&
            (token.code == identifier_plus || token.code == identifier_minus)) /* "++" o "--" */
        {
            TYPEDEF ptr_t = typedef_reduce(left.type) ;

            op = token.code == identifier_plus ? MN_ADD:MN_SUB ;
            if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
            right = compile_operand() ;
            if (right.lvalue)
                codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;
            if (!typedef_is_integer(right.type))
                compile_error (MSG_INCOMP_TYPES) ;
            if (typedef_size(ptr_t) > 1)
            {
                codeblock_add (code, MN_ARRAY,
                    (op == MN_ADD ? 1:-1) * typedef_size(ptr_t)) ;
            }
            else
                codeblock_add (code, op, 0) ;
            res.constant   = 0 ;
            res.lvalue     = 0 ;
            res.asignation = 0 ;
            res.call       = 0 ;
            res.type       = left.type ;
            left = res ;
            continue ;
        }

        /* Suma de cadenas */

        if (typedef_is_array(left.type) && left.lvalue &&
            token.type == IDENTIFIER && token.code == identifier_plus && left.type.chunk[1].type == TYPE_CHAR) /* "+" */
        {
            codeblock_add (code, MN_A2STR, 0) ;
            left.lvalue = 0 ;
            left.type = typedef_new(TYPE_STRING) ;
        }

        /* Suma/resta de valores numéricos */

        if (token.type == IDENTIFIER && (token.code == identifier_plus || token.code == identifier_minus)) /* "+" or "-" */
        {
            op = token.code == identifier_plus ? MN_ADD : MN_SUB ;
            if (left.lvalue) codeblock_add (code, mntype(left.type, 0) | MN_PTR, 0) ;
            right = compile_operand() ;

            /* Concatenación de cadenas */

            if ((typedef_is_string(left.type) || typedef_is_string(right.type)) && op == MN_ADD)
            {
                if (typedef_is_array(right.type) && right.lvalue &&
                    right.type.chunk[1].type == TYPE_CHAR)
                {
                    codeblock_add (code, MN_A2STR, 0) ;
                    right.type = typedef_new(TYPE_STRING) ;
                    right.lvalue = 0 ;
                }
                if (right.lvalue)
                    codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;

                if (typedef_is_integer(right.type))
                    codeblock_add (code, MN_INT2STR | mntype(right.type, 0), 0) ;
                else if (typedef_is_float(right.type))
                    codeblock_add (code, MN_FLOAT2STR, 0) ;
                else if (typedef_is_pointer(right.type))
                    codeblock_add (code, MN_POINTER2STR, 0) ;
                else if (typedef_base(right.type) == TYPE_CHAR)
                    codeblock_add (code, MN_CHR2STR, 0);
                else if (!typedef_is_string(right.type))
                    compile_error (MSG_INCOMP_TYPES) ;

                if (typedef_is_integer(left.type))
                    codeblock_add (code, MN_INT2STR | mntype(left.type, 0), 1) ;
                else if (typedef_is_float(left.type))
                    codeblock_add (code, MN_FLOAT2STR, 1) ;
                else if (typedef_is_pointer(left.type))
                    codeblock_add (code, MN_POINTER2STR, 1) ;
                else if (!typedef_is_string(left.type))
                    compile_error (MSG_INCOMP_TYPES) ;

                codeblock_add (code, MN_STRING | MN_ADD, 0) ;
                res.constant   = 0 ;
                res.lvalue     = 0 ;
                res.asignation = 0 ;
                res.call       = 0 ;
                res.type       = typedef_new(TYPE_STRING) ;
                left = res ;
                continue ;
            }

            if (right.lvalue) codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;

            t = check_numeric_types (&left, &right) ;
            if (t != MN_FLOAT) t = MN_DWORD ;
            codeblock_add (code, op | t, 0) ;
            res.constant   = (right.constant && left.constant) ;
            res.lvalue     = 0 ;
            res.asignation = 0 ;
            res.call       = 0 ;
            if (t == MN_FLOAT)
            {
                res.type   = typedef_new (TYPE_FLOAT) ;
                res.fvalue = op == MN_ADD ? left.fvalue + right.fvalue
                                          : left.fvalue - right.fvalue ;
            }
            else
            {
                res.type   = typedef_new (TYPE_DWORD) ;
                res.value  = op == MN_ADD ? left.value + right.value
                                          : left.value - right.value ;
            }
            left = res ;
            continue ;
        }
        token_back() ;
        break ;
    }
    return left ;
}

expresion_result compile_rotation ()
{
    expresion_result left, right, res ;
    int op ;
    BASETYPE t ;

    left = compile_operation() ;

    token_next() ;
    if (token.type == IDENTIFIER && (token.code == identifier_ror || token.code == identifier_rol)) /* ">>" or "<<" */
    {
        op = token.code == identifier_ror ? MN_ROR : MN_ROL ;
        if (left.lvalue) codeblock_add (code, mntype(left.type, 0) | MN_PTR, 0) ;
        right = compile_operand() ;
        if (right.lvalue) codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;
        t = check_numeric_types (&left, &right) ;
        codeblock_add (code, op | mntype(left.type, 0), 0) ;
        res.constant   = (right.constant && left.constant) ;
        if (t == MN_FLOAT)
        {
            compile_error (MSG_INTEGER_REQUIRED) ;
        }
        else
        {
            res.type   = typedef_new (TYPE_DWORD) ;
            res.value  = (op == MN_ROR ? (left.value >> right.value) : (left.value << right.value)) ;
        }
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        return res ;
    }
    token_back() ;
    return left ;
}

expresion_result compile_comparison ()
{
    expresion_result left = compile_rotation (), right, res ;
    int op ;
    BASETYPE t ;

    token_next() ;
    if (token.type == IDENTIFIER && (
        token.code == identifier_eq ||      /* "==" */
        token.code == identifier_gt ||      /* ">" */
        token.code == identifier_lt ||      /* "<" */
        token.code == identifier_gte ||     /* ">=" or "=>" */
        token.code == identifier_lte ||     /* "<=" or "=<" */
        token.code == identifier_ne ))      /* "!=" or "<>" */
    {
        int is_unsigned = 0;

        op = token.code  ;
        if (left.lvalue && (left.type.chunk[0].type != TYPE_ARRAY || left.type.chunk[1].type != TYPE_CHAR))
            codeblock_add (code, mntype(left.type, 0) | MN_PTR, 0) ;
        right = compile_comparison() ;
        if (right.lvalue) codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;
        t = check_numeric_or_string_types (&left, &right) ;
        if (t != MN_FLOAT && t != MN_STRING) t = MN_DWORD ;

        if (typedef_is_unsigned(left.type) && typedef_is_unsigned(right.type))
            is_unsigned = MN_UNSIGNED;


        if (op == identifier_eq) {       /* "==" */
            codeblock_add (code, t | MN_EQ, 0) ;
            if (left.constant && right.constant)
            {
                if (t == MN_DWORD)
                    res.value = left.value == right.value;
                else if (t == MN_FLOAT)
                    res.fvalue = (float)(left.fvalue == right.fvalue);
                else
                    res.value = strcmp(string_get(left.value), string_get(right.value)) == 0;
            }
        } else if (op == identifier_gt) {   /* ">" */
            codeblock_add (code, t | MN_GT | is_unsigned, 0) ;
            if (left.constant && right.constant)
            {
                if (t == MN_DWORD)
                    res.value = left.value > right.value;
                else if (t == MN_FLOAT)
                    res.fvalue = (float)(left.fvalue > right.fvalue);
                else
                    res.value = strcmp(string_get(left.value), string_get(right.value)) > 0;
            }
        } else if (op == identifier_lt) { /* "<" */
            codeblock_add (code, t | MN_LT | is_unsigned, 0) ;
            if (left.constant && right.constant)
            {
                if (t == MN_DWORD)
                    res.value = left.value < right.value;
                else if (t == MN_FLOAT)
                    res.fvalue = (float)(left.fvalue < right.fvalue);
                else
                    res.value = strcmp(string_get(left.value), string_get(right.value)) < 0;
            }
        } else if (op == identifier_gte) {  /* ">=" or "=>" */
            codeblock_add (code, t | MN_GTE | is_unsigned, 0) ;
            if (left.constant && right.constant)
            {
                if (t == MN_DWORD)
                    res.value = left.value >= right.value;
                else if (t == MN_FLOAT)
                    res.fvalue = (float)(left.fvalue >= right.fvalue);
                else
                    res.value = strcmp(string_get(left.value), string_get(right.value)) >= 0;
            }
        } else if (op == identifier_lte) {  /* "<=" or "=<" */
            codeblock_add (code, t | MN_LTE | is_unsigned, 0) ;
            if (left.constant && right.constant)
            {
                if (t == MN_DWORD)
                    res.value = left.value <= right.value;
                else if (t == MN_FLOAT)
                    res.fvalue = (float)(left.fvalue <= right.fvalue);
                else
                    res.value = strcmp(string_get(left.value), string_get(right.value)) <= 0;
            }
        } else if (op == identifier_ne) {   /* "!=" or "<>" */
            codeblock_add (code, t | MN_NE, 0) ;
            if (left.constant && right.constant)
            {
                if (t == MN_DWORD)
                    res.value = left.value != right.value;
                else if (t == MN_FLOAT)
                    res.fvalue = (float)(left.fvalue != right.fvalue);
                else
                    res.value = strcmp(string_get(left.value), string_get(right.value)) != 0;
            }
        }
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    token_back() ;
    return left ;
}

expresion_result compile_compseq ()
{
    expresion_result left = compile_comparison (), right, res ;
    int et1;
    int et2;

    token_next() ;
    if (token.type == IDENTIFIER && token.code == identifier_and) /* "AND" or "&&" */
    {
        et1 = codeblock_label_add(code);

        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_type (&left) ;
        codeblock_add (code, MN_JTFALSE, et1) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_type (&right) ;

        codeblock_add (code, MN_AND, 0) ;

        codeblock_label_set (code, et1, code->current) ;
/*
        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_types (&left, &right) ;
        codeblock_add (code, MN_AND, 0) ;
*/
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.value      = (left.value && right.value) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    if (token.type == IDENTIFIER && token.code == identifier_or) /* "OR" or "||" */
    {
        et1 = codeblock_label_add(code);
        et2 = codeblock_label_add(code);

        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_type (&left) ;
        codeblock_add (code, MN_JTFALSE, et1) ;

        codeblock_add (code, MN_POP, 0);
        codeblock_add (code, MN_PUSH, 1);
        codeblock_add (code, MN_JUMP, et2);

        codeblock_label_set (code, et1, code->current) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_type (&right) ;

        codeblock_add (code, MN_OR, 0) ;

        codeblock_label_set (code, et2, code->current) ;
/*
        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_types (&left, &right) ;
        codeblock_add (code, MN_OR, 0) ;
*/
        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.value      = (left.value || right.value) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    if (token.type == IDENTIFIER && token.code == identifier_xor) /* "XOR" or "^^" */
    {
        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_types (&left, &right) ;

        codeblock_add (code, MN_XOR, 0) ;

        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.value      = ((left.value!=0) ^ (right.value!=0)) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    if (token.type == IDENTIFIER && (token.code == identifier_band || token.code == identifier_bandoffset)) /* "BAND" or "&" */
    {
        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_types (&left, &right) ;

        codeblock_add (code, MN_BAND, 0) ;

        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.value      = (left.value & right.value) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    if (token.type == IDENTIFIER && token.code == identifier_bor) /* "BOR" or "|" */
    {
        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_types (&left, &right) ;

        codeblock_add (code, MN_BOR, 0) ;

        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.value      = (left.value | right.value) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    if (token.type == IDENTIFIER && token.code == identifier_bxor) /* "BXOR" or "^" */
    {
        if (left.lvalue) codeblock_add (code, MN_PTR, 0) ;
        right = compile_compseq() ;
        if (right.lvalue) codeblock_add (code, MN_PTR, 0) ;
        check_integer_types (&left, &right) ;

        codeblock_add (code, MN_BXOR, 0) ;

        res.lvalue     = 0 ;
        res.asignation = 0 ;
        res.call       = 0 ;
        res.constant   = (right.constant && left.constant) ;
        res.value      = (left.value ^ right.value) ;
        res.type       = typedef_new(TYPE_INT) ;
        return res ;
    }
    token_back() ;
    return left ;
}

expresion_result compile_subexpresion ()
{
    expresion_result base = compile_compseq(), right, res ;
    int op, et1, et2 ;
    BASETYPE type ;

    token_next() ;

    if (token.type == IDENTIFIER)
    {
        /* Operador EXPR ? TRUE : FALSE */
        if (token.code == identifier_question)  /* "?" */
        {
            base = convert_result_type (base, TYPE_DWORD);
            et1 = codeblock_label_add(code);
            et2 = codeblock_label_add(code);
            codeblock_add (code, MN_JFALSE, et1);
            right = compile_expresion (0, 0, 0);
            codeblock_add (code, MN_JUMP, et2);
            codeblock_label_set (code, et1, code->current);
            token_next();

            if (token.type != IDENTIFIER || token.code != identifier_colon) {  /* ":" */
                compile_error (MSG_EXPECTED, ":");
            }

            res = compile_expresion (0, 0, right.type.chunk[0].type);
            codeblock_label_set (code, et2, code->current);

            if (base.constant && res.constant && right.constant) {
                if (typedef_is_integer(base.type)) {
                    return base.value ? right : res;
                } else if (typedef_is_float(base.type)) {
                    return base.fvalue ? right : res;
                }
            }
            res.constant = 0;
            res.lvalue = (right.lvalue && res.lvalue);

            return res;
        }

        /* Asignaciones a cadenas de ancho fijo */

        if (typedef_is_array(base.type) && base.lvalue &&
                    base.type.chunk[1].type == TYPE_CHAR &&
                    token.code == identifier_equal) /* "=" */
        {
            right = compile_expresion(0,0,TYPE_UNDEFINED) ;
            if (typedef_is_integer(right.type)) {
                codeblock_add (code, MN_INT2STR | mntype(right.type, 0), 0) ;
            } else if (typedef_is_float(right.type)) {
                codeblock_add (code, MN_FLOAT2STR, 0) ;
            } else if (!typedef_is_string(right.type)) {
                compile_error (MSG_INCOMP_TYPE) ;
            }
            codeblock_add (code, MN_STR2A, base.type.chunk[0].count-1) ;
            right.asignation = 1 ;

            return right ;
        }

        /* Asignaciones a punteros */

        if (typedef_is_pointer(base.type) && token.code == identifier_equal) /* "=" */
        {
            TYPEDEF pointer_type ;

            pointer_type = typedef_reduce (base.type) ;
            if (!base.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;

            right = compile_expresion(0,0,TYPE_UNDEFINED) ;

            if ((typedef_base(right.type) == TYPE_DWORD || typedef_base(right.type) == TYPE_INT) && right.constant && right.value == 0) {
                right.type = base.type ;
            }

            if (!typedef_is_pointer(right.type)) {
                compile_error (MSG_NOT_A_POINTER) ;
            }
            /* Un puntero "void" puede asignarse a otro cualquiera */
            if (typedef_base(typedef_reduce(right.type)) == TYPE_UNDEFINED) {
                right.type = typedef_pointer (pointer_type) ;
            }
            if (typedef_base(typedef_reduce(right.type)) != typedef_base(pointer_type)) {
                compile_error (MSG_TYPES_NOT_THE_SAME) ;
            }
            codeblock_add (code, MN_DWORD | MN_LET, 0) ;

            res.lvalue     = 1 ;
            res.asignation = 1 ;
            res.call       = 0 ;
            res.constant   = 0 ;
            res.value      = 0 ;
            res.type       = base.type ;

            return res ;
        }

        /* Asignaciones a cadenas */

        if (typedef_is_string(base.type) && token.code == identifier_equal) /* "=" */
        {
            if (!base.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;

            right = compile_expresion(0,0,TYPE_UNDEFINED) ;

            if (typedef_is_integer(right.type)) {
                codeblock_add (code, MN_INT2STR | mntype(right.type, 0), 0) ;
            } else if (typedef_is_float(right.type)) {
                codeblock_add (code, MN_FLOAT2STR, 0) ;
            } else if (!typedef_is_string(right.type)) {
                compile_error (MSG_INCOMP_TYPE) ;
            }
            codeblock_add (code, MN_STRING | MN_LET, 0) ;

            res.lvalue     = 1 ;
            res.asignation = 1 ;
            res.call       = 0 ;
            res.constant   = 0 ;
            res.value      = 0 ;
            res.type       = typedef_new(TYPE_STRING) ;

            return res ;
        }

        /* Puntero += entero */

        if (typedef_is_pointer(base.type) && (token.code == identifier_plusequal || token.code == identifier_minusequal)) /* "+=" or "-=" */
        {
            TYPEDEF pointer_type ;

            op = (token.code == identifier_plusequal ? MN_VARADD:MN_VARSUB) ;
            pointer_type = typedef_reduce (base.type) ;
            if (!base.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;

            right = compile_expresion(0,0,TYPE_UNDEFINED) ;

            if (!typedef_is_integer(right.type)) {
                compile_error (MSG_INCOMP_TYPES) ;
            }

            if (typedef_size(pointer_type) > 1) {
                codeblock_add (code, MN_ARRAY,
                    (op == MN_VARADD ? 1:-1) * typedef_size(pointer_type)) ;
            } else {
                codeblock_add (code, op, 0) ;
            }

            res.lvalue     = 1 ;
            res.asignation = 1 ;
            res.call       = 0 ;
            res.constant   = 0 ;
            res.value      = 0 ;
            res.type       = typedef_new(TYPE_STRING) ;

            return res ;
        }

        /* Cadena += cadena */

        if (typedef_is_array(base.type) && base.lvalue &&
                    base.type.chunk[1].type == TYPE_CHAR &&
                    token.code == identifier_plusequal) /* "+=" */
        {
            right = compile_expresion(0,0,TYPE_UNDEFINED) ;
            if (typedef_is_integer(right.type)) {
                codeblock_add (code, MN_INT2STR | mntype(right.type, 0), 0) ;
            } else if (typedef_is_float(right.type)) {
                codeblock_add (code, MN_FLOAT2STR, 0) ;
            } else if (!typedef_is_string(right.type)) {
                compile_error (MSG_INCOMP_TYPE) ;
            }
            codeblock_add (code, MN_STRACAT, base.type.chunk[0].count-1) ;
            right.asignation = 1 ;

            return right ;
        }

        if (typedef_is_string(base.type) && token.code == identifier_plusequal) /* "+=" */
        {
            if (!base.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
            right = compile_expresion(0,0,TYPE_UNDEFINED) ;
            if (typedef_is_integer(right.type)) {
                codeblock_add (code, MN_INT2STR | mntype(right.type, 0), 0) ;
            } else if (typedef_is_float(right.type)) {
                codeblock_add (code, MN_FLOAT2STR, 0) ;
            } else if (!typedef_is_string(right.type)) {
                compile_error (MSG_INCOMP_TYPE) ;
            }
            codeblock_add (code, MN_STRING | MN_VARADD, 0) ;

            res.lvalue     = 1 ;
            res.asignation = 1 ;
            res.call       = 0 ;
            res.constant   = 0 ;
            res.value      = 0 ;
            res.type       = typedef_new(TYPE_STRING) ;

            return res ;
        }

        /* Otra posible combinación */

        if (     token.code == identifier_plusequal     /* "+=" */
             ||  token.code == identifier_minusequal    /* "-=" */
             ||  token.code == identifier_multequal     /* "*=" */
             ||  token.code == identifier_divequal      /* "/=" */
             ||  token.code == identifier_modequal      /* "%=" */
             ||  token.code == identifier_orequal       /* "|=" */
             ||  token.code == identifier_andequal      /* "&=" */
             ||  token.code == identifier_xorequal      /* "^=" */
             ||  token.code == identifier_rorequal      /* ">>=" */
             ||  token.code == identifier_rolequal      /* "<<=" */
             ||  token.code == identifier_equal)        /* "=" */
        {
            op = token.code ;
            if (typedef_is_array(base.type)) {
                compile_error (MSG_EXPECTED, "[") ;
            }

            if (typedef_is_struct(base.type)) {
                // Assignation to struct: struct copy

                if (token.code != identifier_equal) { /* "=" */
                    compile_error (MSG_EXPECTED, "=") ;
                }

                right = compile_expresion(0, 0, 0);
                while (typedef_is_array(right.type)) {
                    right.type = typedef_reduce(right.type);
                }

                if (typedef_base(right.type) != TYPE_POINTER) {
                    compile_error (MSG_STRUCT_REQUIRED);
                }

                if (!typedef_is_struct(typedef_reduce(right.type))) {
                    compile_error (MSG_STRUCT_REQUIRED);
                } else if (right.type.varspace != base.type.varspace) {
                    compile_error (MSG_TYPES_NOT_THE_SAME);
                } else {
                    /*
                     *  Struct copy operator
                     */

                    SYSPROC * proc_copy = sysproc_get (identifier_search_or_add("#COPY#"));
                    SYSPROC * proc_memcopy = sysproc_get (identifier_search_or_add("MEMCOPY"));
                    int size, nvar;

                    while (typedef_is_pointer(base.type))
                    {
                        codeblock_add (code, MN_PTR, 0);
                        res.type = typedef_reduce(base.type);
                    }

                    if (typedef_base(base.type) != TYPE_STRUCT)
                    {
                        compile_error (MSG_STRUCT_REQUIRED);
                    }
                    else
                    {
                        size = right.type.varspace->count * sizeof(DCB_TYPEDEF);

                        if (right.type.varspace->stringvar_count > 0)
                        {
                            // True struct copy version

                            segment_alloc (globaldata, size);
                            codeblock_add (code, MN_GLOBAL, globaldata->current) ;
                            for (nvar = 0 ; nvar < right.type.varspace->count ; nvar++)
                            {
                                DCB_TYPEDEF type;
                                dcb_settype (&type, &right.type.varspace->vars[nvar].type);
                                memcpy ((Uint8*)globaldata->bytes + globaldata->current, &type, sizeof(DCB_TYPEDEF));
                                globaldata->current += sizeof(DCB_TYPEDEF);
                            }
                            codeblock_add (code, MN_PUSH | MN_DWORD, right.type.varspace->count);
                            codeblock_add (code, MN_PUSH | MN_DWORD, right.count?right.count:1);
                            codeblock_add (code, MN_SYSCALL, proc_copy->code);
                        }
                        else
                        {
                            // Optimized fast memcopy version
                            codeblock_add (code, MN_PUSH | MN_DWORD, right.type.varspace->size*(right.count?right.count:1));
                            codeblock_add (code, MN_SYSCALL, proc_memcopy->code);
                        }
                    }
                    base.type = right.type;
                    base.constant = 0;
                    base.lvalue = 0;
                    base.call = 1;
                }
                return base;
            }

            if (!base.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
            right = compile_expresion(0,0,typedef_base(base.type)) ;
            if (right.lvalue) codeblock_add (code, mntype(right.type, 0) | MN_PTR, 0) ;

            type = check_numeric_types (&base, &right) ;

            if (op == identifier_plusequal)                 /* "+=" */
                codeblock_add (code, type | MN_VARADD, 0) ;
            else if (op == identifier_minusequal)           /* "-=" */
                codeblock_add (code, type | MN_VARSUB, 0) ;
            else if (op == identifier_multequal)            /* "*=" */
                codeblock_add (code, type | MN_VARMUL, 0) ;
            else if (op == identifier_divequal)             /* "/=" */
                codeblock_add (code, type | MN_VARDIV, 0) ;
            else if (op == identifier_modequal)             /* "%=" */
                codeblock_add (code, type | MN_VARMOD, 0) ;
            else if (op == identifier_orequal)              /* "|=" */
                codeblock_add (code, type | MN_VAROR, 0) ;
            else if (op == identifier_andequal)             /* "&=" */
                codeblock_add (code, type | MN_VARAND, 0) ;
            else if (op == identifier_xorequal)             /* "^=" */
                codeblock_add (code, type | MN_VARXOR, 0) ;
            else if (op == identifier_rorequal)             /* ">>=" */
                codeblock_add (code, type | MN_VARROR, 0) ;
            else if (op == identifier_rolequal)             /* "<<=" */
                codeblock_add (code, type | MN_VARROL, 0) ;
            else if (op == identifier_equal)                /* "=" */
                codeblock_add (code, type | MN_LET, 0) ;

            res.lvalue     = 1 ;
            res.asignation = 1 ;
            res.call       = 0 ;
            res.constant   = 0 ;
            res.value      = 0 ;
            res.type       = right.type ;

            return res ;
        } else {
            token_back() ;
        }
    } else {
        token_back() ;
    }

    return base ;
}

expresion_result compile_expresion (int need_constant, int need_lvalue, BASETYPE t)
{
    expresion_result res ;

    CODEBLOCK_POS pos ;

    if (code) pos = codeblock_pos(code);

    res = compile_subexpresion() ;

    /* Interpreta una estructura tal cual como un puntero a la misma */

    if (res.lvalue && typedef_base(res.type) == TYPE_STRUCT && !need_lvalue)
    {
        res.type = typedef_pointer(res.type) ;
        res.lvalue = 0 ;
        res.constant = 0 ;
    }

    /* Interpretar arrays de byte como cadenas */

    if (typedef_base(res.type) == TYPE_ARRAY && res.type.chunk[1].type == TYPE_CHAR && res.lvalue && !need_lvalue)
    {
        codeblock_add (code, MN_A2STR, 0) ;
        res.type = typedef_new(TYPE_STRING) ;   /* Array 2 String */
        res.lvalue = 0 ;
    }

    /* Quita los lvalue */

    if (!need_lvalue && res.lvalue)
    {
        res.lvalue = 0 ;
        codeblock_add (code, mntype(res.type, 0) | MN_PTR, 0) ;
    }

    /* Conversiones de tipo */

    if (t != TYPE_UNDEFINED)
        res = convert_result_type (res, t) ;

    /* Optimización de datos constantes */

    if (res.constant)
    {
        if (code) codeblock_setpos(code, pos);
        if (typedef_base(res.type) == TYPE_FLOAT)
            codeblock_add (code, MN_PUSH | MN_FLOAT, *(int *)&res.fvalue) ;
        else if (typedef_base(res.type) == TYPE_STRING)
            codeblock_add (code, MN_PUSH | MN_STRING, res.value) ;
        else
            codeblock_add (code, MN_PUSH, res.value) ;
    }

    if (need_lvalue && !res.lvalue)
        compile_error (MSG_VARIABLE_REQUIRED) ;
    if (need_constant && !res.constant)
        compile_error (MSG_CONSTANT_EXP) ;

    return res ;
}

/*
 *  FUNCTION : convert_result_type
 *
 *  Given an expresion result in the current context, convert it
 *  if possible to the basic type given (and emit the necessary code)
 *
 *  PARAMS:
 *      res             Result of expression at current context
 *      t               Basic type required
 *
 *  RETURN VALUE:
 *      The converted type result
 */

expresion_result convert_result_type (expresion_result res, BASETYPE t)
{
    /* Conversiones de tipo */

    if (t < 9 && typedef_is_integer(res.type))
    {
        res.type = typedef_new(t);
    }
    if (typedef_base(res.type) == TYPE_POINTER && t == TYPE_STRING)
    {
        codeblock_add (code, MN_POINTER2STR, 0) ;
        res.type = typedef_new(t) ; /* Pointer 2 String */
    }
    if (typedef_base(res.type) == TYPE_DWORD && res.constant
            && res.value == 0 && t == TYPE_POINTER)
    {
        res.type = typedef_new(t) ; /* Null pointer */
    }
    if (typedef_base(res.type) == TYPE_POINTER && t < 8)
    {
/*        codeblock_add (code, MN_POINTER2BOL, 0) ; */
        res.type = typedef_new(t) ; /* Pointer 2 Int */
    }
    if (typedef_base(res.type) == TYPE_FLOAT && t < 8)
    {
        if (t < 4)
            codeblock_add (code, MN_FLOAT2INT, 0) ;
        else
            codeblock_add (code, MN_FLOAT2INT, 0) ;

        res.type = typedef_new(t) ;
        res.value = (int)res.fvalue ;
    }
    if (t == TYPE_FLOAT && typedef_is_integer(res.type))
    {
        codeblock_add (code, MN_INT2FLOAT, 0) ;
        res.type = typedef_new(TYPE_FLOAT) ;
        res.fvalue = (float)res.value ;
    }
    if ((t == TYPE_BYTE || t == TYPE_WORD || t == TYPE_DWORD) &&
            typedef_is_integer(res.type))
    {
        res.type = typedef_new(t) ;
    }
    if (t == TYPE_STRING && typedef_is_integer(res.type))
    {
        codeblock_add (code, MN_INT2STR | mntype(res.type, 0), 0) ;
        if (res.constant)
        {
            char buffer[32] ;
            switch (res.type.chunk[0].type)
            {
                case TYPE_INT:
                    sprintf (buffer, "%d", res.value) ;
                    break;
                case TYPE_WORD:
                    sprintf (buffer, "%d", (Uint16)res.value) ;
                    break;
                case TYPE_BYTE:
                    sprintf (buffer, "%d", (Uint8)res.value) ;
                    break;
                case TYPE_SHORT:
                    sprintf (buffer, "%d", (Sint16)res.value) ;
                    break;
                case TYPE_SBYTE:
                    sprintf (buffer, "%d", (Sint8)res.value) ;
                    break;
                case TYPE_DWORD:
                    sprintf (buffer, "%u", (Uint32)res.value) ;
                    break;
            }
            res.value = string_new(buffer) ;
        }
        res.type = typedef_new(t) ;
    }
    if (t == TYPE_STRING && typedef_base(res.type) == TYPE_CHAR)
    {
        codeblock_add (code, MN_CHR2STR, 0) ;
        if (res.constant)
        {
            char buffer[2] ;
            buffer[0] = res.value;
            buffer[1] = 0;
            res.value = string_new(buffer) ;
        }
        res.type = typedef_new(t) ;
    }

    if (t != TYPE_UNDEFINED && typedef_base(res.type) != t)
    {
        switch (t)
        {
            case TYPE_CHAR:

                /* Allow string-to-char conversions */

                if (typedef_is_string(res.type))
                {
                    codeblock_add (code, MN_STR2CHR, 0) ;
                    if (res.constant == 1)
                        res.value = (unsigned char)*(string_get(res.value));
                }
                else
                    compile_error (MSG_INTEGER_REQUIRED) ;
                break ;

            case TYPE_DWORD:
            case TYPE_INT:
            case TYPE_WORD:
            case TYPE_SHORT:
            case TYPE_BYTE:
            case TYPE_SBYTE:
                if (typedef_is_array(res.type) && res.lvalue &&
                    res.type.chunk[1].type == TYPE_CHAR)
                {
                    codeblock_add (code, MN_A2STR, 0) ;
                    codeblock_add (code, MN_STR2INT, 0) ;
                    res.lvalue = 0 ;
                    res.constant = 0;
                }
                else if (typedef_is_string(res.type))
                {
                    codeblock_add (code, MN_STR2INT, 0) ;
                    if (res.constant == 1)
                        res.value = atoi(string_get(res.value));
                }
                else if (typedef_base(res.type) == TYPE_CHAR)
                {
                    ;
                }
                else
                    compile_error (MSG_INTEGER_REQUIRED) ;
                break ;
            case TYPE_FLOAT:
                if (typedef_is_string(res.type))
                {
                    codeblock_add (code, MN_STR2FLOAT, 0) ;
                    if (res.constant == 1)
                        res.fvalue = (float)atof(string_get(res.value));
                }
                else
                    compile_error (MSG_NUMBER_REQUIRED) ;
                break ;
            case TYPE_STRING:
                if (typedef_is_array(res.type) && res.lvalue &&
                    res.type.chunk[1].type == TYPE_CHAR)
                {
                    codeblock_add (code, MN_A2STR, 0) ;
                    res.lvalue = 0 ;
                }
                else if (typedef_is_integer(res.type))
                {
                    codeblock_add (code, MN_INT2STR | mntype(res.type, 0), 0) ;
                    if (res.constant)
                    {
                        char buffer[32] ;
                        sprintf (buffer, "%d", res.value) ;
                        res.value = string_new(buffer) ;
                    }
                }
                else if (typedef_is_float(res.type))
                {
                    codeblock_add (code, MN_FLOAT2STR, 0) ;
                    if (res.constant)
                    {
                        char buffer[32] ;
                        sprintf (buffer, "%g", res.fvalue) ;
                        res.value = string_new(buffer) ;
                    }
                }
                else
                    compile_error (MSG_STRING_EXP) ;
                break ;
            default:
                compile_error (MSG_INCOMP_TYPE) ;
        }
        res.type = typedef_new(t) ;
    }

    return res ;
}

int compile_sentence_end ()
{
    token_next() ;

    if (token.type == NOTOKEN)
        return 1;

    if (token.type == IDENTIFIER && token.code == identifier_semicolon) /* ";" */
        return 0 ;

    /*
    if (token.type == IDENTIFIER && token.code == identifier_end)
    {
        token_back() ;
        return 1 ;
    }
    */

    compile_error (MSG_EXPECTED, ";") ;
    return 0;
}

extern int dcb_options ;

void basetype_describe (char * buffer, BASETYPE t)
{
	switch (t)
	{
		case TYPE_INT:
			sprintf (buffer, "INT") ;
			return ;

		case TYPE_DWORD:
			sprintf (buffer, "DWORD") ;
			return ;

		case TYPE_SHORT:
			sprintf (buffer, "SHORT") ;
			return ;

		case TYPE_WORD:
			sprintf (buffer, "WORD") ;
			return ;

		case TYPE_BYTE:
			sprintf (buffer, "BYTE") ;
			return ;

		case TYPE_CHAR:
			sprintf (buffer, "CHAR") ;
			return ;

		case TYPE_SBYTE:
			sprintf (buffer, "SIGNED BYTE") ;
			return ;

		case TYPE_STRING:
			sprintf (buffer, "STRING") ;
			return ;

		case TYPE_FLOAT:
			sprintf (buffer, "FLOAT") ;
			return ;

		case TYPE_STRUCT:
		    sprintf (buffer, "STRUCT") ;

		case TYPE_ARRAY:
			sprintf (buffer, "ARRAY") ;
			return ;

		case TYPE_POINTER:
			sprintf (buffer, "POINTER") ;
			return ;

		case TYPE_UNDEFINED:
		default:
			sprintf (buffer, "<UNDEFINED>") ;
			return ;
	}
}

void compile_block (PROCDEF * p)
{
    int loop, last_loop, et1, et2 ;
    expresion_result res, from, to ;

    proc = p ;
    code = &p->code ;

    for (;;)
    {
        token_next() ;
        if (token.type == NOTOKEN)
            break ;

        if (token.type == IDENTIFIER)
        {
            if (token.code == identifier_end    ||  /* "END" */
                token.code == identifier_until  ||  /* "UNTIL" */
                token.code == identifier_else   ||  /* "ELSE" */
                token.code == identifier_elseif)    /* "ELSEIF" */
                break ;

            if (token.code == identifier_semicolon) /* ";" */
                continue ;

            if (token.code == identifier_colon)     /* ":" */
                continue ;

            /* CONTINUE */

            if (token.code == identifier_continue)  /* "CONTINUE" */
            {
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                if (!code->loop_active)
                    compile_error (MSG_NO_LOOP) ;
                codeblock_add (code, MN_REPEAT, code->loop_active) ;
                compile_sentence_end() ;
                continue ;
            }

            /* BREAK */

            if (token.code == identifier_break) /* "BREAK" */
            {
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                if (!code->loop_active)
                    compile_error (MSG_NO_LOOP) ;
                codeblock_add (code, MN_BREAK, code->loop_active) ;
                compile_sentence_end() ;
                continue ;
            }

            /* FRAME */

            if (token.code == identifier_frame) /* "FRAME" */
            {
                if (proc->type != TYPE_INT && proc->type != TYPE_DWORD)
                {
                    if (!(proc->flags & PROC_FUNCTION))
                        compile_error(MSG_FRAME_REQUIRES_INT);
                }
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                {
                    token_back() ;
                    compile_expresion (0,0,TYPE_DWORD) ;
                    codeblock_add (code, MN_FRAME, 0) ;
                    compile_sentence_end() ;
                }
                else
                {
                    codeblock_add (code, MN_PUSH, 100) ;
                    codeblock_add (code, MN_FRAME, 0) ;
                }
                proc->flags |= PROC_USES_FRAME;
                continue ;
            }

            /* DEBUG */

            if (token.code == identifier_debug) /* "DEBUG" */
            {
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                codeblock_add (code, MN_DEBUG, 0) ;
                compile_sentence_end() ;
                continue ;
            }

            /* RETURN */

            if (token.code == identifier_return)    /* "RETURN" */
            {
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                {
                    token_back() ;
                    compile_expresion (0,0,p->type) ;
                    codeblock_add (code, MN_RETURN, 0) ;
                    compile_sentence_end() ;
                }
                else
                {
                    codeblock_add (code, MN_END, 0) ;
                }
                continue ;
            }

            /* ONEXIT */

            if (token.code == identifier_onexit)    /* "ONEXIT" */
            {
                /* Finalizo el bloque actual y todo el codigo a continuacion es onexit */
                codeblock_add (code, MN_END, 0) ;
                p->exitcode = code->current;
                continue ;
            }

            /* CLONE */

            if (token.code == identifier_clone) /* "CLONE" */
            {
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                et1 = codeblock_label_add (code) ;
                codeblock_add (code, MN_CLONE, et1) ;
                compile_block(p) ;
                codeblock_label_set (code, et1, code->current) ;
                proc->flags |= PROC_USES_FRAME;
                continue ;
            }

            /* IF */

            if (token.code == identifier_if)    /* "IF" */
            {
                /* Label at the end of a IF/ELSEIF/ELSEIF/ELSE chain */
                int end_of_chain = -1;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                for (;;)
                {
                    token_next() ;
                    if (token.type != IDENTIFIER || token.code != identifier_leftp) /* "(" */
                    {
                        token_back() ;
                        compile_expresion (0, 0, TYPE_DWORD);
                        token_next() ;
                        if (token.type != IDENTIFIER || (token.code != identifier_semicolon && token.code != identifier_colon)) /* ";" or ":" */
                            compile_error (MSG_EXPECTED, "(") ;
                    }
                    else
                    {
                        compile_expresion (0, 0, TYPE_DWORD);
                        token_next() ;
                        if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                            compile_error (MSG_EXPECTED, ")") ;
                    }

                    et1 = codeblock_label_add (code) ;
                    codeblock_add (code, MN_JFALSE, et1) ;
                    compile_block(p) ;
                    if (token.type == IDENTIFIER && token.code == identifier_else)  /* "ELSE" */
                    {
                        et2 = codeblock_label_add (code) ;
                        codeblock_add (code, MN_JUMP, et2) ;
                        codeblock_label_set (code, et1, code->current) ;
                        compile_block(p) ;
                        codeblock_label_set (code, et2, code->current) ;
                        break;
                    }
                    else if (token.type == IDENTIFIER && token.code == identifier_elseif)   /* "ELSEIF" */
                    {
                        if (end_of_chain == -1)
                            end_of_chain = codeblock_label_add (code) ;
                        codeblock_add (code, MN_JUMP, end_of_chain);
                        codeblock_label_set (code, et1, code->current) ;
                        continue;
                    }
                    else
                    {
                        codeblock_label_set (code, et1, code->current) ;
                        break;
                    }
                }
                if (end_of_chain != -1)
                    codeblock_label_set (code, end_of_chain, code->current) ;
                continue ;
            }

            /* FOR */

            if (token.code == identifier_for)   /* "FOR" */
            {
                int forline = line_count + (current_file << 24) ;

                loop = codeblock_loop_add (code) ;
                et1 = codeblock_label_add (code) ;
                et2 = codeblock_label_add (code) ;

                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_leftp) /* "(" */
                    compile_error (MSG_EXPECTED, "(") ;

                /* Inicializadores */
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                {
                    token_back() ;
                    do
                    {
                        compile_expresion(0,0,TYPE_DWORD) ;
                        codeblock_add (code, MN_POP, 0) ;
                        token_next() ;
                    }
                    while (token.type == IDENTIFIER && token.code == identifier_comma) ; /* "," */
                }
                if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                    compile_error (MSG_EXPECTED, ";") ;

                codeblock_label_set (code, et1, code->current) ;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, forline) ;
                }

                /* Condiciones */
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                {
                    token_back() ;
                    do
                    {
                        compile_expresion(0,0,TYPE_DWORD) ;
                        codeblock_add (code, MN_BRFALSE, loop) ;
                        token_next() ;
                    }
                    while (token.type == IDENTIFIER && token.code == identifier_comma) ; /* "," */
                }
                if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                    compile_error (MSG_EXPECTED, ";") ;
                codeblock_add (code, MN_JUMP, et2) ;

                /* Incrementos */
                codeblock_loop_start (code, loop, code->current) ;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, forline) ;
                }

                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_rightp)    /* ")" */
                {
                    token_back() ;
                    do
                    {
                        compile_expresion(0,0,TYPE_DWORD) ;
                        codeblock_add (code, MN_POP, 0) ;
                        token_next() ;
                    }
                    while (token.type == IDENTIFIER && token.code == identifier_comma) ;    /* "," */
                }
                if (token.type != IDENTIFIER || token.code != identifier_rightp)    /* ")" */
                    compile_error (MSG_EXPECTED, ")") ;

                codeblock_add (code, MN_JUMP, et1) ;

                /* Bloque */
                codeblock_label_set (code, et2, code->current) ;

                last_loop = code->loop_active ;
                code->loop_active = loop ;
                compile_block(p) ;
                code->loop_active = last_loop ;

                codeblock_add (code, MN_REPEAT, loop) ;
                codeblock_loop_end (code, loop, code->current) ;
                continue ;
            }

            /* SWITCH */

            if (token.code == identifier_switch)    /* "SWITCH" */
            {
                int switch_type = 0;
                expresion_result switch_exp ;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_leftp) /* "(" */
                    compile_error (MSG_EXPECTED, "(") ;
                switch_exp = compile_expresion(0, 0, 0) ;
                switch_type = typedef_base(switch_exp.type);
                if (switch_type == TYPE_ARRAY && switch_exp.type.chunk[0].type == TYPE_CHAR)
                {
                    codeblock_add (code, MN_A2STR, 0) ;
                    switch_type = TYPE_STRING;
                }
                else if (switch_type != TYPE_STRING)
                {
                    switch_exp = convert_result_type (switch_exp, TYPE_INT);
                    switch_type = TYPE_INT;
                }
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_rightp)    /* ")" */
                    compile_error (MSG_EXPECTED, ")") ;
                token_next() ;
                if (token.type != IDENTIFIER || (token.code != identifier_semicolon && token.code != identifier_colon)) /* ";" or ":" */
                    token_back() ;

                if (switch_type == TYPE_STRING)
                    codeblock_add (code, MN_SWITCH | MN_STRING, 0) ;
                else
                    codeblock_add (code, MN_SWITCH, 0) ;

                et1 = codeblock_label_add (code) ;
                for (;;)
                {
                    if (dcb_options & DCB_DEBUG) {
                        codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                    }
                    token_next() ;
                    if (token.type == IDENTIFIER && token.code == identifier_case)  /* "CASE" */
                    {
                        for (;;)
                        {
                            token_next() ;
                            if (token.type == IDENTIFIER && token.code == identifier_colon) /* ":" */
                                break ;
                            if (token.type == IDENTIFIER && token.code == identifier_comma) /* "," */
                                continue ;
                            token_back() ;

                            compile_expresion(0,0,switch_type) ;
                            token_next() ;
                            if (token.type == IDENTIFIER && token.code == identifier_twopoints) /* ".." */
                            {
                                compile_expresion(0,0,switch_type) ;
                                if (switch_type != TYPE_STRING)
                                    codeblock_add (code, MN_CASE_R, 0) ;
                                else
                                    codeblock_add (code, MN_CASE_R | MN_STRING, 0) ;
                                token_next() ;
                            }
                            else
                            {
                                if (switch_type == TYPE_STRING)
                                    codeblock_add (code, MN_CASE | MN_STRING, 0) ;
                                else
                                    codeblock_add (code, MN_CASE, 0) ;
                            }
                            if (token.type == IDENTIFIER && token.code == identifier_colon) /* ":" */
                                break ;
                            if (token.type != IDENTIFIER || token.code != identifier_comma) /* "," */
                                compile_error (MSG_EXPECTED, ";") ;
                        }
                        et2 = codeblock_label_add (code) ;
                        codeblock_add (code, MN_JNOCASE, et2) ;
                        compile_block (p) ;
                        codeblock_add (code, MN_JUMP, et1) ;
                        codeblock_label_set (code, et2, code->current) ;
                    }
                    else if (token.type == IDENTIFIER && token.code == identifier_default) /* "DEFAULT" */
                    {
                        if (dcb_options & DCB_DEBUG) {
                            codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                        }
                        token_next() ;
                        if (token.type != IDENTIFIER || token.code != identifier_colon) /* ":" */
                            compile_error (MSG_EXPECTED, ";") ;
                        compile_block(p) ;
                    }
                    else if (token.type == IDENTIFIER && token.code == identifier_semicolon)    /* ";" */
                        continue ;
                    else if (token.type == IDENTIFIER && token.code == identifier_end)  /* "END" */
                        break ;
                    else
                        compile_error (MSG_EXPECTED, "CASE");
                }
                codeblock_label_set (code, et1, code->current) ;
                continue ;
            }

            /* LOOP */

            if (token.code == identifier_loop)  /* "LOOP" */
            {
                loop = codeblock_loop_add (code) ;
                codeblock_loop_start (code, loop, code->current) ;
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }

                last_loop = code->loop_active ;
                code->loop_active = loop ;
                compile_block(p) ;
                code->loop_active = last_loop ;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                codeblock_add (code, MN_REPEAT, loop) ;
                codeblock_loop_end (code, loop, code->current) ;
                continue ;
            }

            /* FROM ... TO */

            if (token.code == identifier_from)  /* "FROM" */
            {
                int inc = 1 ;
                CODEBLOCK_POS var_pos;
                CODEBLOCK_POS var_end;

                et1 = codeblock_label_add (code) ;

                // Compile the variable access

                loop = codeblock_loop_add(code) ;
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                var_pos = codeblock_pos(code);
                res = compile_value () ;
                var_end = codeblock_pos(code);
                if (!res.lvalue) compile_error (MSG_VARIABLE_REQUIRED) ;
                if (!typedef_is_integer(res.type)) compile_error (MSG_INTEGER_REQUIRED);

                // Compile the assignation of first value

                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_equal) /* "=" */
                    compile_error (MSG_EXPECTED, "=") ;
                from = compile_expresion(0,0,typedef_base(res.type));
                codeblock_add (code, MN_LETNP | mntype(res.type, 0), 0);

                // Compile the loop termination check

                codeblock_label_set (code, et1, code->current) ;

                codeblock_add_block (code, var_pos, var_end);
                codeblock_add (code, MN_PTR | mntype(res.type, 0), 0);
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_to)    /* "TO" */
                    compile_error (MSG_EXPECTED, "TO") ;
                to = compile_expresion(0,0,TYPE_DWORD) ;

                token_next() ;
                if (token.type == IDENTIFIER && token.code == identifier_step)  /* "STEP" */
                {
                    CODEBLOCK_POS p = codeblock_pos(code);
                    expresion_result r = compile_expresion(1, 0, typedef_base(res.type)) ;
                    if (!r.constant) compile_error (MSG_CONSTANT_EXP);
                    if (!typedef_is_integer(r.type)) compile_error (MSG_INTEGER_REQUIRED);
                    inc = r.value;

                    codeblock_setpos(code, p);
                    if (inc > 0)
                        codeblock_add (code, MN_LTE, 0) ;
                    else
                    {
                        if (inc == 0) compile_error (MSG_INVALID_STEP) ;
                        codeblock_add (code, MN_GTE, 0) ;
                    }
                }
                else
                {
                    if (from.constant && to.constant)
                    {
                        if (from.value > to.value) inc = -1 ;
                        codeblock_add (code, (from.value > to.value ? MN_GTE : MN_LTE), 0) ;
                    }
                    else
                        codeblock_add (code, MN_LTE, 0) ;
                    token_back() ;
                }
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                codeblock_add (code, MN_BRFALSE, loop) ;

                // Compile the loop block contents

                last_loop = code->loop_active ;
                code->loop_active = loop ;
                compile_block(p) ;
                code->loop_active = last_loop ;

                // Compile the increment and looping code

                codeblock_loop_start (code, loop, code->current) ;

                codeblock_add_block (code, var_pos, var_end);
                if (inc == 1)
                    codeblock_add (code, MN_INC | mntype(res.type, 0), 1) ;
                else if (inc == -1)
                    codeblock_add (code, MN_DEC | mntype(res.type, 0), 1) ;
                else
                {
                    codeblock_add (code, MN_PUSH, inc) ;
                    codeblock_add (code, MN_VARADD | mntype(res.type, 0), 0) ;
                }
                codeblock_add (code, MN_POP, 0) ;
                codeblock_add (code, MN_JUMP, et1) ;

                codeblock_loop_end (code, loop, code->current) ;
                continue ;
            }

            /* REPEAT ... UNTIL */

            if (token.code == identifier_repeat) /* "REPEAT" */
            {
                et1 = codeblock_label_add (code) ;
                et2 = codeblock_label_add (code) ;

                loop = codeblock_loop_add (code) ;

                codeblock_add (code, MN_JUMP, et1) ;

                codeblock_loop_start (code, loop, code->current) ;

                codeblock_add (code, MN_JUMP, et2) ;
                codeblock_label_set (code, et1, code->current) ;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }

                last_loop = code->loop_active ;
                code->loop_active = loop ;
                compile_block(p) ;
                code->loop_active = last_loop ;

                codeblock_label_set (code, et2, code->current) ;

                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                if (token.type != IDENTIFIER || token.code != identifier_until) /* "UNTIL" */
                    compile_error (MSG_EXPECTED, "UNTIL") ;
                token_next() ;
                if (token.type == IDENTIFIER && token.code == identifier_leftp) /* "(" */
                {
                    compile_expresion (0, 0, TYPE_DWORD) ;
                    token_next() ;
                    if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                        compile_error (MSG_EXPECTED, ")") ;
                }
                else
                {
                    token_back() ;
                    compile_expresion (0, 0, TYPE_DWORD) ;
                    token_next() ;
                    if (token.type != IDENTIFIER || token.code != identifier_semicolon) /* ";" */
                        compile_error (MSG_EXPECTED, ";") ;
                }
                codeblock_add (code, MN_JFALSE, et1) ;
                codeblock_loop_end (code, loop, code->current) ;
                continue ;
            }

            /* WHILE ... END */

            if (token.code == identifier_while) /* "WHILE" */
            {
                token_next() ;
                if (token.type != IDENTIFIER || token.code != identifier_leftp) /* "(" */
                {
                    token_back() ;
                    loop = codeblock_loop_add (code) ;
                    codeblock_loop_start (code, loop, code->current) ;
                    if (dcb_options & DCB_DEBUG) {
                        codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                    }

                    compile_expresion (0, 0, TYPE_DWORD) ;
                    token_next() ;
                    if (token.type != IDENTIFIER || (token.code != identifier_semicolon && token.code != identifier_colon ) ) /* ";" or ":" */
                        compile_error (MSG_EXPECTED, ";") ;
                }
                else
                {
                    loop = codeblock_loop_add (code) ;
                    codeblock_loop_start (code, loop, code->current) ;
                    if (dcb_options & DCB_DEBUG) {
                        codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                    }
                    compile_expresion (0, 0, TYPE_DWORD) ;
                    token_next() ;
                    if (token.type != IDENTIFIER || token.code != identifier_rightp) /* ")" */
                        compile_error (MSG_EXPECTED, ")") ;
                }
                if (dcb_options & DCB_DEBUG) {
                    codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
                }
                codeblock_add (code, MN_BRFALSE, loop) ;

                last_loop = code->loop_active ;
                code->loop_active = loop ;
                compile_block(p) ;
                code->loop_active = last_loop ;

                codeblock_add (code, MN_REPEAT, loop) ;
                codeblock_loop_end (code, loop, code->current) ;
                continue ;
            }
        }

        if (token.type != IDENTIFIER) // || token.code < reserved_words)
            compile_error (MSG_INVALID_SENTENCE) ;

        token_back() ;

        if (dcb_options & DCB_DEBUG) {
            codeblock_add (code, MN_SENTENCE, line_count + (current_file << 24)) ;
        }

        /* Asignation */

        res = compile_subexpresion() ;
        if (!res.asignation && !res.call)
            compile_error (MSG_INVALID_SENTENCE) ;
        if (typedef_is_string(res.type) && !res.lvalue)
            codeblock_add (code, MN_STRING | MN_POP, 0) ;
        else
            codeblock_add (code, MN_POP, 0) ;

        if (compile_sentence_end()) break ;
    }
}

