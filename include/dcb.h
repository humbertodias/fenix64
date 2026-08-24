/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.82
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

/*
 * FILE        : dcb.h
 * DESCRIPTION : Data compiled block information
 *
 * HISTORY: Corrected DCB_PROC_SIZE TO 48
 */

#ifndef __DCB_H
#define __DCB_H

#ifdef TARGET_MAC
#include <SDL/SDL_types.h>
#else
#include <SDL_types.h>
#endif

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

#define DCB_DEBUG 1

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

/* Opción del GNU C para que la estructura ocupe el mínimo de memoria */

#ifdef __GNUC__
#define __PACKED __attribute__ ((packed))
#else
#define __PACKED
#endif

/* Estructura del fichero .dcb */

/* Please update the version's high-number between Fenix versions */
#define DCB_VERSION 0x0100

#define DCB_ID_SIZE 64

typedef struct
{
    Uint8   Name[60] ;
    Uint32  Code ;
}
__PACKED
DCB_ID ;

#define DCB_FILE_SIZE 64

#define DCB_FILE_COMPRESSED 1

typedef struct
{
    Uint8   Name[55] ;
    Uint8   Flags ;
    Uint32  SFile ;
    Uint32  OFile ;
}
__PACKED
DCB_FILE ;

#define DCB_SENTENCE_SIZE sizeof(DCB_SENTENCE)

typedef struct
{
    Uint32  NFile ;
    Uint32  NLine ;
    Uint32  NCol ;
    Uint32  OCode ;
}
__PACKED
DCB_SENTENCE ;

#define DCB_TYPEDEF_SIZE sizeof(DCB_TYPEDEF)
#define NO_MEMBERS       0xFFFFFFFF

typedef struct
{
    Uint8   BaseType [MAX_TYPECHUNKS] ;
    Uint32  Count    [MAX_TYPECHUNKS] ;
    Uint32  Members ;
}
__PACKED
DCB_TYPEDEF ;

#define DCB_VAR_SIZE sizeof(DCB_VAR)

typedef struct
{
    DCB_TYPEDEF Type ;      /* 40 bytes */
    Uint32      ID ;
    Uint32      Offset ;
    Uint32      Varspace ;
}
__PACKED
DCB_VAR ;

#define DCB_VARSPACE_SIZE sizeof(DCB_VARSPACE)

typedef struct
{
    Uint32  NVars ;
    Uint32  OVars ;
}
__PACKED
DCB_VARSPACE ;

#define DCB_PROC_SIZE 48

typedef struct          /* Cabecera de cada proceso     */
{
    Uint32  ID ;

    Uint32  NParams ;
    Uint32  NPriVars ;
    Uint32  NPriStrings ;
    Uint32  NSentences ;

    Uint32  SPrivate ;
    Uint32  SCode ;

    Uint32  OPrivate ;
    Uint32  OPriVars ;
    Uint32  OPriStrings ;
    Uint32  OCode ;
    Uint32  OSentences ;

    /* - Aquí acaba la parte que se carga desde el fichero - */

    DCB_SENTENCE    * sentence ;
    DCB_VAR         * privar ;
}
__PACKED
DCB_PROC ;

#define DCB_HEADER_SIZE 280

typedef struct          /* Cabecera general del fichero */
{
    Uint8           Header[8] ; /* "DCB"            */
    Uint32          Version ;   /* 0x0100 para versión 1.0  */

    Uint32          NProcs ;
    Uint32          NFiles ;
    Uint32          NID ;
    Uint32          NStrings ;
    Uint32          NLocStrings ;
    Uint32          NLocVars ;
    Uint32          NGloVars ;
    Uint32          NVarSpaces ;

    Uint32          SGlobal ;
    Uint32          SLocal ;
    Uint32          SText ;

    Uint32          NImports ;
    Uint32          NSourceFiles ;
    Uint32          __reserved1[3] ;

    Uint32          OProcsTab ;
    Uint32          OID ;
    Uint32          OStrings ;
    Uint32          OText ;
    Uint32          OGlobal ;
    Uint32          OGloVars ;
    Uint32          OLocal ;
    Uint32          OLocVars ;
    Uint32          OLocStrings ;
    Uint32          OVarSpaces ;
    Uint32          OFilesTab ;
    Uint32          OImports ;
    Uint32          OSourceFiles ;
    Uint32          __reserved2[3] ;

    /* - Aquí acaba la parte que se carga del fichero */

    DCB_ID          * id ;
    DCB_VAR         * glovar ;
    DCB_VAR         * locvar ;
    DCB_PROC        * proc ;
    DCB_FILE        * file ;
    DCB_VARSPACE    * varspace ;
    DCB_VAR        ** varspace_vars ;
    Uint32          * imports ;
    char          *** sourcelines ;
    int             * sourcecount ;
}
__PACKED
DCB_HEADER ;

extern DCB_HEADER dcb ;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
