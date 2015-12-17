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


#include <stdio.h>
#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <string.h>

#include "fxi.h"
#include "dcb.h"

/* ---------------------------------------------------------------------- */
/* Este módulo contiene funciones que muestran el equivalente en texto de */
/* una instrucción o mnemónico, o de un bloque de código completo         */
/* ---------------------------------------------------------------------- */

struct
{
    char * name;
    int    code;
    int    params ;
}
mnemonics[] =
{
    { "ARRAY", MN_ARRAY, 1 },
    { "GET_PRIVATE", MN_GET_PRIV, 1 },
    { "GET_REMOTE", MN_GET_REMOTE, 1 },
    { "GET_LOCAL", MN_GET_LOCAL, 1 },
    { "GET_GLOBAL", MN_GET_GLOBAL, 1 },
    { "PRIVATE", MN_PRIVATE, 1 },
    { "REMOTE", MN_REMOTE, 1 },
    { "LOCAL", MN_LOCAL, 1 },
    { "GLOBAL", MN_GLOBAL, 1 },
    { "DUP", MN_DUP },
    { "PUSH", MN_PUSH, 1 },
    { "POP",  MN_POP },
    { "INDEX", MN_INDEX, 1 },
    { "NEG", MN_NEG },
    { "NOT", MN_NOT },
    { "CALL", MN_CALL, 1 },
    { "PROCESS", MN_PROC, 1 },
    { "SYSCALL", MN_SYSCALL, 1 },
    { "SYSPROC", MN_SYSPROC, 1 },
    { "CLONE", MN_CLONE, 1 },
    { "END", MN_END },
    { "RETURN", MN_RETURN },
    { "FRAME", MN_FRAME },
    { "TYPE", MN_TYPE, 1 },

    { "MUL", MN_MUL },
    { "DIV", MN_DIV },
    { "ADD", MN_ADD },
    { "SUB", MN_SUB },
    { "MOD", MN_MOD },
    { "ROR", MN_ROR },
    { "ROL", MN_ROL },
    { "AND", MN_AND },
    { "OR", MN_OR },
    { "XOR", MN_XOR },

    { "BNOT", MN_BNOT },
    { "BAND", MN_BAND },
    { "BOR", MN_BOR },
    { "BXOR", MN_BXOR },

    { "EQ", MN_EQ },
    { "NE", MN_NE },
    { "GT", MN_GT },
    { "LT", MN_LT },
    { "GTE", MN_GTE },
    { "LTE", MN_LTE },

    { "FLOAT2INT", MN_FLOAT2INT, 1 },
    { "INT2FLOAT", MN_INT2FLOAT, 1 },
    { "INT2WORD", MN_INT2WORD, 1 },
    { "INT2BYTE", MN_INT2BYTE, 1 },

    { "POSTINC", MN_POSTINC, 1 },
    { "POSTDEC", MN_POSTDEC, 1 },
    { "INC", MN_INC, 1 },
    { "DEC", MN_DEC, 1 },
    { "VARADD", MN_VARADD },
    { "VARSUB", MN_VARSUB },
    { "VARMUL", MN_VARMUL },
    { "VARDIV", MN_VARDIV },
    { "VARMOD", MN_VARMOD },
    { "VARXOR", MN_VARXOR },
    { "VARAND", MN_VARAND },
    { "VAROR", MN_VAROR },
    { "VARROR", MN_VARROR },
    { "VARROL", MN_VARROL },
    { "LET", MN_LET },
    { "LETNP", MN_LETNP },

    { "REPEAT", MN_REPEAT, 1 },
    { "BREAK", MN_BREAK, 1 },
    { "BRFALSE", MN_BRFALSE, 1 },
    { "RETRUE", MN_RETRUE, 1 },
    { "REFALSE", MN_REFALSE, 1 },
    { "JUMP", MN_JUMP, 1 },
    { "JFALSE", MN_JFALSE, 1 },
    { "JTRUE", MN_JTRUE, 1 },
    { "JTFALSE", MN_JTFALSE, 1 },
    { "JTTRUE", MN_JTTRUE, 1 },
    { "PTR", MN_PTR },

    { "SWITCH", MN_SWITCH },
    { "CASE", MN_CASE, 0 },
    { "CASE_R", MN_CASE_R, 0 },
    { "JNOCASE", MN_JNOCASE, 1 },

    { "CHRSTR", MN_CHRSTR },
    { "SUBSTR", MN_SUBSTR, 1 },

    { "STR2INT", MN_STR2INT, 1 },
    { "STR2CHR", MN_STR2CHR, 1 },
    { "INT2STR", MN_INT2STR, 1 },
    { "CHR2STR", MN_CHR2STR, 1 },
    { "STR2FLOAT", MN_STR2FLOAT, 1 },
    { "FLOAT2STR", MN_FLOAT2STR, 1 },
    { "POINTER2STR", MN_POINTER2STR, 1 },
    { "POINTER2BOL", MN_POINTER2BOL, 1 },

    { "A2STR", MN_A2STR, 0 },
    { "STR2A", MN_STR2A, 0 },
    { "STRACAT", MN_STRACAT, 0 },

    { "DEBUG", MN_DEBUG, 1 },

    { 0, -1 }
} ;

void mnemonic_dump (int i, int param)
{
    int n = 0 ;
    while (mnemonics[n].name)
    {
        if (mnemonics[n].code == (i & MN_MASK))
        {
            switch (MN_TYPEOF(i))
            {
                case MN_WORD:
                    printf ("WORD     ") ;
                    break ;
                case MN_BYTE:
                    printf ("BYTE     ") ;
                    break ;
                case MN_FLOAT:
                    printf ("FLOAT    ") ;
                    break ;
                case MN_STRING:
                    printf ("STRING   ") ;
                    break ;
                default:
                    printf ("         ") ;
                    break ;
            }
            printf ("%-12s", mnemonics[n].name) ;
            if (i == (MN_PUSH | MN_FLOAT))
                printf ("%-8f", *(float *)&param) ;
            else if (i & MN_1_PARAMS)
                printf ("%-8d", param) ;
/*            printf ("\n") ; */
        }
        n++ ;
    }
}

/* Muestra el árbol de instancias */

void instance_dump_all()
{
    INSTANCE * i ;

    for (i = first_instance ; i ; i = i->next)
    {
        if (!LOCDWORD(i,FATHER))
        {
            instance_dump (i, 1) ;

        }
    }

}

void instance_dump(INSTANCE * father, int indent)
{
    INSTANCE * i, * next ;
    PROCDEF * proc ;
    DCB_PROC * dcbproc ;
    char buffer[128] ;
    char line[128] ;
    int n, nid ;

    i = father ;
    if (!father) i = first_instance ;

    proc = i->proc ;
    dcbproc = &dcb.proc[proc->type] ;

    nid = LOCDWORD(i,PROCESS_ID) ;
    if (dcb.data.NID && dcbproc->data.ID) {
        sprintf (buffer, "%s", getid(dcbproc->data.ID)) ;
    } else {
        sprintf (buffer, "%s", proc->type == 0 ? "MAIN":"PROC") ;
    }

    line[0] = 0 ;
    for (n = 0 ; n < indent*3-3 ; n+=3)
        strcat (line, " \x03 ") ;
    sprintf (line+strlen(line), " \x01\x02 %-12s ", buffer) ;

    while (n++ < 8) strcat (line, " ") ;
    sprintf (line+strlen(line), "%7d", nid) ;

    if (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) strcat(line, "[W]");
    switch (LOCDWORD(i, STATUS) & ~STATUS_WAITING_MASK)
    {
        case STATUS_DEAD        :   strcat (line, "[D]") ; break ;
        case STATUS_KILLED      :   strcat (line, "[K]") ; break ;
        case STATUS_RUNNING     :   strcat (line, "   ") ; break ;
        case STATUS_SLEEPING    :   strcat (line, "[S]") ; break ;
        case STATUS_FROZEN      :   strcat (line, "[F]") ; break ;
/*        default:
            sprintf (line+strlen(line), "%-4d", LOCDWORD(i, STATUS)) ;
            break;
*/
    }

    gr_con_printf (line) ;

    if (!LOCDWORD(i,SON)) return ;

    next = instance_get (LOCDWORD(i,SON)) ;
    if (!next) gr_con_printf ("[FXI] \12**PANIC**\7 SON %d does not exist\n", LOCDWORD(i,SON)) ;
    i = next ;

    while (i)
    {
        proc = i->proc ;
        dcbproc = &dcb.proc[proc->type] ;

        if (LOCDWORD(i, SON)) {
            instance_dump (i, indent+1) ;
        } else {
            nid = LOCDWORD(i,PROCESS_ID) ;
            if (dcb.data.NID && dcbproc->data.ID) {
                sprintf (buffer, "%s", getid(dcbproc->data.ID)) ;
            } else {
                sprintf (buffer, "%s", proc->type == 0 ? "MAIN":"PROC") ;
            }

            line[0] = 0 ;
            for (n = 0 ; n < indent*3 ; n+=3)
                strcat (line, " \x03 ") ;

            sprintf (line+strlen(line), " \x01\x02 %-12s ", buffer) ;

            while (n++ < 8) strcat (line, " ") ;

            sprintf (line+strlen(line), "%7d", nid) ;

            if (LOCDWORD(i, STATUS) & STATUS_WAITING_MASK) strcat(line, "[W]");
            switch (LOCDWORD(i, STATUS) & ~STATUS_WAITING_MASK)
            {
                case STATUS_DEAD        :   strcat (line, "[D]") ; break ;
                case STATUS_KILLED      :   strcat (line, "[K]") ; break ;
                case STATUS_RUNNING     :   strcat (line, "   ") ; break ;
                case STATUS_SLEEPING    :   strcat (line, "[S]") ; break ;
                case STATUS_FROZEN      :   strcat (line, "[F]") ; break ;
/*                default:
                    sprintf (line+strlen(line), "%-4d", LOCDWORD(i, STATUS)) ;
                    break;
*/
            }
            gr_con_printf (line) ;

/*
            printf ("FAT: %-5d SON: %-5d BIG: %-5d SMA: %-5d\n",
                LOCDWORD(i, FATHER), LOCDWORD(i, SON),
                LOCDWORD(i, BIGBRO), LOCDWORD(i, SMALLBRO)) ;
*/
        }

        if (LOCDWORD(i, BIGBRO))
        {
            next = instance_get (LOCDWORD(i, BIGBRO)) ;
            if (!next)
                gr_con_printf ("[FXI] \12**PANIC**\7 BIGBRO %d does not exist\n", LOCDWORD(i, BIGBRO)) ;
            i = next ;
        }
        else
            break ;
    }
}
