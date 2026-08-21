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

/*
 * FILE        : dcbr.c
 * DESCRIPTION : DCB loading functions
 *
 * HISTORY:  0.83 - Patch to unify path processing
 *
 */

#include <stdio.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifndef TARGET_Win32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include "fxi.h"
#include "dcb.h"
#include "dirs.h"
#include "files.h"
#include "pslang.h"

void * globaldata = 0 ;
void * localdata  = 0 ;
int    local_size ;
int  * localstr   = 0 ;
int    local_strings ;

PROCDEF * procs = NULL ;
PROCDEF * mainproc = NULL ;
int procdef_count = 0 ;

PROCDEF * procdef_get (int n)
{
	if (n >= 0 && n < procdef_count)
		return &procs[n] ;

	return NULL ;
}

PROCDEF * procdef_get_by_name (char * name)
{
    int n;
    for (n = 0 ; n < procdef_count; n++)
        if (strcmp(procs[n].name, name) == 0)
            return &procs[n];

	return NULL ;
}

int dcb_is_v1 (void)
{
	return (dcb.data.Version & 0xFF00) == (DCB_VERSION_084 & 0xFF00);
}

DCB_HEADER dcb ;

static char * trim(char * ptr)
{
    char * ostr = ptr;
    char * bptr = ptr;

    while (*ptr == ' ' || *ptr == '\n' || *ptr == '\r' || *ptr == '\t') ptr++;

    while (*ptr) *bptr++ = *ptr++;

    while (bptr > ostr && (bptr[-1] == ' ' || bptr[-1] == '\n' || bptr[-1] == '\r' || bptr[-1] == '\t')) bptr--;

    *bptr = 0;

    return (ostr);
}

static int load_file (const char * filename, int n)
{
    char line[2048] ;
    int allocated = 16 ;
    int count = 0 ;
    char ** lines ;
    file * fp ;

    fp = file_open (filename, "r0") ;
    if (!fp)
    {
        dcb.sourcelines[n] = 0 ;
        dcb.sourcecount[n] = 0 ;
        return 0 ;
    }

    lines = (char **) malloc(sizeof(char**) * 16) ;

    while (!file_eof(fp))
    {
        file_qgets (fp, line, 2048) ;
        trim(line);
        if (allocated == count)
        {
            allocated += 16 ;
            lines = realloc (lines, sizeof(char**) * allocated) ;
        }
        lines[count++] = strdup(line) ;
    }
    file_close (fp) ;
    dcb.sourcelines[n] = lines ;
    dcb.sourcecount[n] = count ;
    return 1 ;
}

int dcb_load (const char * filename)
{
	file * fp ;

	/* check for existence of the DCB FILE */
	if (!file_exists(filename)) return 0 ;

	fp = file_open (filename, "rb0") ;
	if (!fp)
		gr_error ("Error al abrir %s\n", filename) ;

	return dcb_load_from (fp, 0);
}

/* 0.84 bytecode used opcodes 0x1E-0x45; 0.93 inserted JTFALSE/JTTRUE. */
static void dcb_translate_code_v1 (int * code, int nbytes)
{
	int nwords, i;

	if (!code || nbytes < 4) return ;
	nwords = nbytes / 4 ;
	i = 0 ;
	while (i < nwords)
	{
		int word = code[i] ;
		int op = word & MN_MASK ;
		int base = op & 0x7F ;

		if (base >= 0x1E && base <= 0x45)
			code[i] = (word & ~MN_MASK) | ((base + 2) | (op & 0x80)) ;

		i++ ;
		if (MN_PARAMS (op) && i < nwords)
			i++ ;
	}
}

int dcb_load_from (file * fp, int offset)
{
	unsigned int n ;
	char *path, *ptr ;

	/* Cambia al directorio del DCB con chdir */
	path = dir_path_convert(fp->name) ;
	for (ptr = path+strlen(path) ; ptr >= path ; ptr--)
		if (*ptr == '/' || *ptr == '\\') break ;
	ptr[1] = 0 ;
	chdir (path) ;
	base_dir = path ;

	/* En WIN32, cambia a la unidad del DCB */

#ifdef WIN32
	if (path[0] && path[1] == ':')
	{
		if (path[0] >= 'A' && path[0] <= 'Z')
			base_drive = path[0]-'A'+1 ;

		if (path[0] >= 'a' && path[0] <= 'z')
			base_drive = path[0]-'a'+1 ;

		_chdrive (base_drive);
	}
#endif

	/* Lee el contenido del fichero */

	file_seek (fp, offset, SEEK_SET);
	file_read (fp, &dcb, sizeof(DCB_HEADER_DATA)) ;

	if (memcmp (dcb.data.Header, "dcb\xD\x0A\x1F\x00\x00", 8) != 0)
		return 0 ;

	{
		unsigned ver = dcb.data.Version & 0xFF00 ;
		if (ver != (DCB_VERSION & 0xFF00) && ver != (DCB_VERSION_084 & 0xFF00))
			return 0 ;
	}

	{
		int global_extra = 0 ;
		int local_extra = 0 ;
		int is_v1 = ((dcb.data.Version & 0xFF00) == (DCB_VERSION_084 & 0xFF00)) ;

		if (is_v1)
		{
			global_extra = 4 * (1 + 12 + 400 + 2) ;
			local_extra = 4 * 8 ;
		}

		vm_arena_init () ;
		globaldata = vm_malloc (dcb.data.SGlobal + global_extra + 4) ;
		localdata  = vm_malloc (dcb.data.SLocal + local_extra + 4) ;
		memset (globaldata, 0, dcb.data.SGlobal + global_extra + 4) ;
		memset (localdata, 0, dcb.data.SLocal + local_extra + 4) ;
		offsets_init (dcb.data.Version, dcb.data.SGlobal, dcb.data.SLocal) ;
		local_size = dcb.data.SLocal + local_extra ;
	}
	localstr   = (int *) malloc (4 * dcb.data.NLocStrings + 4) ;
	dcb.proc   = (DCB_PROC *) malloc (sizeof(DCB_PROC) * (1+dcb.data.NProcs)) ;
	procs      = (PROCDEF *) malloc (sizeof(PROCDEF) * (1+dcb.data.NProcs)) ;
	mainproc   = procs ;

	procdef_count = dcb.data.NProcs ;
	local_strings = dcb.data.NLocStrings ;
	memset (dcb.proc, 0, sizeof(DCB_PROC) * (1+dcb.data.NProcs)) ;
	memset (procs, 0, sizeof(PROCDEF) * (1+dcb.data.NProcs)) ;

	/* Recupera las zonas de datos globales */

	file_seek (fp, offset + dcb.data.OGlobal, SEEK_SET) ;
	file_read (fp, globaldata, dcb.data.SGlobal) ;

	file_seek (fp, offset + dcb.data.OLocal, SEEK_SET) ;
	file_read (fp, localdata, dcb.data.SLocal) ;

	if (dcb.data.NLocStrings)
	{
		file_seek (fp, offset + dcb.data.OLocStrings, SEEK_SET) ;
		file_read (fp, localstr, dcb.data.NLocStrings * 4) ;
	}

	file_seek (fp, offset + dcb.data.OProcsTab, SEEK_SET) ;
	if ((dcb.data.Version & 0xFF00) == (DCB_VERSION_084 & 0xFF00))
	{
		for (n = 0 ; n < dcb.data.NProcs ; n++)
		{
			DCB_PROC_DATA_V1 old ;

			file_read (fp, &old, sizeof(DCB_PROC_DATA_V1)) ;
			dcb.proc[n].data.ID          = old.ID ;
			dcb.proc[n].data.NParams     = old.NParams ;
			dcb.proc[n].data.NPriVars    = old.NPriVars ;
			dcb.proc[n].data.NPriStrings = old.NPriStrings ;
			dcb.proc[n].data.NSentences  = old.NSentences ;
			dcb.proc[n].data.SPrivate    = old.SPrivate ;
			dcb.proc[n].data.SCode       = old.SCode ;
			dcb.proc[n].data.OPrivate    = old.OPrivate ;
			dcb.proc[n].data.OPriVars    = old.OPriVars ;
			dcb.proc[n].data.OPriStrings = old.OPriStrings ;
			dcb.proc[n].data.OCode       = old.OCode ;
			dcb.proc[n].data.OSentences  = old.OSentences ;
		}
	}
	else
	{
		for (n = 0 ; n < dcb.data.NProcs ; n++)
			file_read (fp, &dcb.proc[n], sizeof(DCB_PROC_DATA)) ;
	}

	/* Recupera las cadenas */

	dcb.data.OStrings += offset;
	dcb.data.OText += offset;
	string_load (fp) ;

	/* Recupera los ficheros incluídos */

	if (dcb.data.NFiles)
	{
		dcb.file = (DCB_FILE *) malloc(sizeof(DCB_FILE) * dcb.data.NFiles) ;
		file_seek (fp, offset + dcb.data.OFilesTab, SEEK_SET) ;
		file_read (fp, dcb.file, sizeof(DCB_FILE) * dcb.data.NFiles) ;

		for (n = 0 ; n < dcb.data.NFiles ; n++)
		{
			file_add_xfile (fp, dcb.file[n].OFile, dcb.file[n].Name, dcb.file[n].SFile) ;
		}
	}

	/* Recupera los imports */

	if (dcb.data.NImports)
	{
		dcb.imports = (Uint32 *)malloc(4 * dcb.data.NImports) ;
		file_seek (fp, offset + dcb.data.OImports, SEEK_SET) ;
		file_read (fp, dcb.imports, 4 * dcb.data.NImports) ;
	}

	/* Recupera los datos de depurado */

	if (dcb.data.NID)
	{
		dcb.id = (DCB_ID *) malloc(sizeof(DCB_ID) * dcb.data.NID) ;
		file_seek (fp, offset + dcb.data.OID, SEEK_SET) ;
		file_read (fp, dcb.id, sizeof(DCB_ID) * dcb.data.NID) ;
	}

	if (dcb.data.NGloVars)
	{
		dcb.glovar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.data.NGloVars) ;
		file_seek (fp, offset + dcb.data.OGloVars, SEEK_SET) ;
		file_read (fp, dcb.glovar, sizeof(DCB_VAR) * dcb.data.NGloVars) ;
	}

	if (dcb.data.NLocVars)
	{
		dcb.locvar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.data.NLocVars) ;
		file_seek (fp, offset + dcb.data.OLocVars, SEEK_SET) ;
		file_read (fp, dcb.locvar, sizeof(DCB_VAR) * dcb.data.NLocVars) ;
	}

	if (dcb.data.NVarSpaces)
	{
		dcb.varspace = (DCB_VARSPACE *) malloc(sizeof(DCB_VARSPACE) * dcb.data.NVarSpaces) ;
		dcb.varspace_vars = (DCB_VAR **) malloc(sizeof(DCB_VAR *) * dcb.data.NVarSpaces) ;
		file_seek (fp, offset + dcb.data.OVarSpaces, SEEK_SET) ;
		file_read (fp, dcb.varspace, sizeof(DCB_VARSPACE) * dcb.data.NVarSpaces) ;

		for (n = 0 ; n < dcb.data.NVarSpaces ; n++)
		{
			dcb.varspace_vars[n] = 0 ;
			if (!dcb.varspace[n].NVars) continue ;
			dcb.varspace_vars[n] = (DCB_VAR *)
				malloc (sizeof(DCB_VAR) * dcb.varspace[n].NVars) ;
			file_seek (fp, offset + dcb.varspace[n].OVars, SEEK_SET) ;
			file_read (fp, dcb.varspace_vars[n],
				sizeof(DCB_VAR) * dcb.varspace[n].NVars) ;
		}
	}

	if (dcb.data.NSourceFiles)
	{
		char filename[256] ;

		dcb.sourcecount = (int*)malloc (sizeof(int) * dcb.data.NSourceFiles) ;
		dcb.sourcelines = (char ***)malloc (sizeof(char **) * dcb.data.NSourceFiles) ;
		file_seek (fp, offset + dcb.data.OSourceFiles, SEEK_SET) ;
		for (n = 0 ; n < dcb.data.NSourceFiles ; n++)
		{
			file_read (fp, filename, 256) ; /* FIX PARA LOS INCLUDES */
			if (!load_file (filename, n))
				gr_con_printf ("[FXI] File %s not found", filename) ;
		}
	}

	/* Recupera los procesos */

	for (n = 0 ; n < dcb.data.NProcs ; n++)
	{
		procs[n].params             = dcb.proc[n].data.NParams ;
		if (procs[n].params > 256)
			procs[n].params = 0 ;
		procs[n].string_count       = dcb.proc[n].data.NPriStrings ;
		procs[n].pubstring_count    = dcb.proc[n].data.NPubStrings ;
		procs[n].private_size       = dcb.proc[n].data.SPrivate ;
		procs[n].public_size        = dcb.proc[n].data.SPublic ;
		procs[n].code_size          = dcb.proc[n].data.SCode ;
		procs[n].id                 = dcb.proc[n].data.ID ;
		procs[n].flags              = dcb.proc[n].data.Flags ;
		procs[n].type               = n ;
		procs[n].name               = getid(procs[n].id) ;
        procs[n].breakpoint         = 0;

		if (dcb.proc[n].data.SPrivate)
		{
			procs[n].pridata = (int *)malloc(dcb.proc[n].data.SPrivate) ;
			file_seek (fp, offset + dcb.proc[n].data.OPrivate, SEEK_SET) ;
			file_read (fp, procs[n].pridata, dcb.proc[n].data.SPrivate) ;
		}

		if (dcb.proc[n].data.SPublic)
		{
			procs[n].pubdata = (int *)malloc(dcb.proc[n].data.SPublic) ;
			file_seek (fp, offset + dcb.proc[n].data.OPublic, SEEK_SET) ;
			file_read (fp, procs[n].pubdata, dcb.proc[n].data.SPublic) ;
		}

		if (dcb.proc[n].data.SCode)
		{
			procs[n].code = (int *) malloc(dcb.proc[n].data.SCode) ;
			file_seek (fp, offset + dcb.proc[n].data.OCode, SEEK_SET) ;
			file_read (fp, procs[n].code, dcb.proc[n].data.SCode) ;

			if ((dcb.data.Version & 0xFF00) == (DCB_VERSION_084 & 0xFF00))
				dcb_translate_code_v1 (procs[n].code, dcb.proc[n].data.SCode) ;

	        if (dcb.proc[n].data.OExitCode)
	            procs[n].exitcode = procs[n].code + dcb.proc[n].data.OExitCode ;
	        else
	            procs[n].exitcode = NULL ;
		}

		if (dcb.proc[n].data.NPriStrings)
		{
			procs[n].strings = (int *)malloc(dcb.proc[n].data.NPriStrings * 4) ;
			file_seek (fp, offset + dcb.proc[n].data.OPriStrings, SEEK_SET) ;
			file_read (fp, procs[n].strings, dcb.proc[n].data.NPriStrings * 4) ;
		}

		if (dcb.proc[n].data.NPubStrings)
		{
			procs[n].pubstrings = (int *)malloc(dcb.proc[n].data.NPubStrings * 4) ;
			file_seek (fp, offset + dcb.proc[n].data.OPubStrings, SEEK_SET) ;
			file_read (fp, procs[n].pubstrings, dcb.proc[n].data.NPubStrings * 4) ;
		}

		if (dcb.proc[n].data.NPriVars)
		{
			dcb.proc[n].privar = (DCB_VAR *)malloc(dcb.proc[n].data.NPriVars * sizeof(DCB_VAR)) ;
			file_seek (fp, offset + dcb.proc[n].data.OPriVars, SEEK_SET) ;
			file_read (fp, dcb.proc[n].privar, dcb.proc[n].data.NPriVars * sizeof(DCB_VAR)) ;
		}

		if (dcb.proc[n].data.NPubVars)
		{
			dcb.proc[n].pubvar = (DCB_VAR *)malloc(dcb.proc[n].data.NPubVars * sizeof(DCB_VAR)) ;
			file_seek (fp, offset + dcb.proc[n].data.OPubVars, SEEK_SET) ;
			file_read (fp, dcb.proc[n].pubvar, dcb.proc[n].data.NPubVars * sizeof(DCB_VAR)) ;
		}
	}

	return 1 ;
}

char * getid(unsigned int code)
{
	unsigned int n ;
	for (n = 0 ; n < dcb.data.NID ; n++) {
		if (dcb.id[n].Code == code)
			return dcb.id[n].Name ;
	}
	return "(?)" ;
}
