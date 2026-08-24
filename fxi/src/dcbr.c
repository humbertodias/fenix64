/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.83
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
 * FILE        : dcbr.c
 * DESCRIPTION : DCB loading functions
 *
 * HISTORY:  0.83 - Patch to unify path processing
 *
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include "fxi.h"
#include "dcb.h"
#include "dirs.h"

void * globaldata = 0 ;
void * localdata  = 0 ;
int    local_size ;
int  * localstr   = 0 ;
int    local_strings ;

PROCDEF * procs ;
PROCDEF * mainproc ;
int procdef_count = 0 ;

PROCDEF * procdef_get (int n)
{
	if (n >= 0 && n < procdef_count)
		return &procs[n] ;

	return 0 ;
}

DCB_HEADER dcb ;

static int load_file (const char * filename, int n)
{
        char line[256] ;
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
                file_gets (fp, line, 256) ;
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
	unsigned int n ;
	char *path, *ptr ;

	file * fp = file_open (filename, "rb0") ;

	if (!fp) gr_error ("Error al abrir %s\n", filename) ;

	/* Cambia al directorio del DCB con chdir */
	path = dir_path_convert(filename) ; 
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

	file_read (fp, &dcb, DCB_HEADER_SIZE) ;

	if (memcmp (dcb.Header, "dcb\xD\x0A\x1F\x00\x00", 8) != 0 ||
	    (dcb.Version & 0xFF00) != (DCB_VERSION & 0xFF00))
	{
		gr_error ("%s: no es un DCB version %d o compatible",
				filename, DCB_VERSION >> 8) ;
		return 0 ;
	}

	globaldata = vm_malloc (dcb.SGlobal + 4) ;
	localdata  = vm_malloc (dcb.SLocal + 4) ;
	localstr   = (int *) malloc (4 * dcb.NLocStrings + 4) ;
	dcb.proc   = (DCB_PROC *) malloc (sizeof(DCB_PROC) * (1+dcb.NProcs)) ;
	procs      = (PROCDEF *) malloc (sizeof(PROCDEF) * (1+dcb.NProcs)) ;
	mainproc   = procs ;

	procdef_count = dcb.NProcs ;
	local_size    = dcb.SLocal ;
	local_strings = dcb.NLocStrings ;

	/* Recupera las zonas de datos globales */

	file_seek (fp, dcb.OGlobal, SEEK_SET) ;
	file_read (fp, globaldata, dcb.SGlobal) ;

	file_seek (fp, dcb.OLocal, SEEK_SET) ;
	file_read (fp, localdata, dcb.SLocal) ;

	if (dcb.NLocStrings)
	{
		file_seek (fp, dcb.OLocStrings, SEEK_SET) ;
		file_read (fp, localstr, dcb.NLocStrings * 4) ;
	}

	file_seek (fp, dcb.OProcsTab, SEEK_SET) ;
	for (n = 0 ; n < dcb.NProcs ; n++)
	{
		file_read (fp, &dcb.proc[n], DCB_PROC_SIZE) ;
	}

	/* Recupera los procesos */

	for (n = 0 ; n < dcb.NProcs ; n++)
	{
		procs[n].params       = dcb.proc[n].NParams ;
		procs[n].string_count = dcb.proc[n].NPriStrings ;
		procs[n].private_size = dcb.proc[n].SPrivate ;
		procs[n].code_size    = dcb.proc[n].SCode ;
		procs[n].id           = dcb.proc[n].ID ;
		procs[n].type         = n ;

		if (dcb.proc[n].SPrivate)
		{
			procs[n].pridata = (int *)malloc(dcb.proc[n].SPrivate) ;
			file_seek (fp, dcb.proc[n].OPrivate, SEEK_SET) ;
			file_read (fp, procs[n].pridata, dcb.proc[n].SPrivate) ;
		}

		if (dcb.proc[n].SCode)
		{
			procs[n].code = (int *) malloc(dcb.proc[n].SCode) ;
			file_seek (fp, dcb.proc[n].OCode, SEEK_SET) ;
			file_read (fp, procs[n].code, dcb.proc[n].SCode) ;
		}

		if (dcb.proc[n].NPriStrings)
		{
			procs[n].strings = (int *)malloc
				(dcb.proc[n].NPriStrings * 4) ;
			file_seek (fp, dcb.proc[n].OPriStrings, SEEK_SET) ;
			file_read (fp, procs[n].strings, 
					dcb.proc[n].NPriStrings * 4) ;
		}

		if (dcb.proc[n].NPriVars)
		{
			dcb.proc[n].privar = (DCB_VAR *)malloc
				(dcb.proc[n].NPriVars * DCB_VAR_SIZE) ;
			file_seek (fp, dcb.proc[n].OPriVars, SEEK_SET) ;
			file_read (fp, dcb.proc[n].privar, 
					dcb.proc[n].NPriVars * DCB_VAR_SIZE) ;
		}
	}

	/* Recupera las cadenas */

	string_load (fp) ;

	/* Recupera los ficheros incluídos */

	if (dcb.NFiles)
	{
		dcb.file = (DCB_FILE *) malloc(sizeof(DCB_FILE) * dcb.NFiles) ;
		file_seek (fp, dcb.OFilesTab, SEEK_SET) ;
		file_read (fp, dcb.file, sizeof(DCB_FILE) * dcb.NFiles) ;

		for (n = 0 ; n < dcb.NFiles ; n++)
		{
			file_add_xfile (fp, dcb.file[n].OFile,
				dcb.file[n].Name, dcb.file[n].SFile) ;
		}
	}

	/* Recupera los imports */

	if (dcb.NImports)
	{
		dcb.imports = (Uint32 *)malloc(4 * dcb.NImports) ;
		file_seek (fp, dcb.OImports, SEEK_SET) ;
		file_read (fp, dcb.imports, 4 * dcb.NImports) ;
	}
	
	/* Recupera los datos de depurado */
	
	if (dcb.NID)
	{
		dcb.id = (DCB_ID *) malloc(sizeof(DCB_ID) * dcb.NID) ;
		file_seek (fp, dcb.OID, SEEK_SET) ;
		file_read (fp, dcb.id, DCB_ID_SIZE * dcb.NID) ;
	}
	
	if (dcb.NGloVars)
	{
		dcb.glovar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.NGloVars) ;
		file_seek (fp, dcb.OGloVars, SEEK_SET) ;
		file_read (fp, dcb.glovar, DCB_VAR_SIZE * dcb.NGloVars) ;
	}
	
	if (dcb.NLocVars)
	{
		dcb.locvar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.NLocVars) ;
		file_seek (fp, dcb.OLocVars, SEEK_SET) ;
		file_read (fp, dcb.locvar, DCB_VAR_SIZE * dcb.NLocVars) ;
	}
	
	if (dcb.NVarSpaces)
	{
		dcb.varspace = (DCB_VARSPACE *) malloc(DCB_VARSPACE_SIZE * dcb.NVarSpaces) ;
		dcb.varspace_vars = (DCB_VAR **) malloc(sizeof(DCB_VAR *) * dcb.NVarSpaces) ;
		file_seek (fp, dcb.OVarSpaces, SEEK_SET) ;
		file_read (fp, dcb.varspace, DCB_VARSPACE_SIZE * dcb.NVarSpaces) ;
		
		for (n = 0 ; n < dcb.NVarSpaces ; n++)
		{
			dcb.varspace_vars[n] = 0 ;
			if (!dcb.varspace[n].NVars) continue ;
			dcb.varspace_vars[n] = (DCB_VAR *)
				malloc (DCB_VAR_SIZE * dcb.varspace[n].NVars) ;
			file_seek (fp, dcb.varspace[n].OVars, SEEK_SET) ;
			file_read (fp, dcb.varspace_vars[n],
				DCB_VAR_SIZE * dcb.varspace[n].NVars) ;
		}
	}
	
	if (dcb.NSourceFiles)
	{
		char filename[256] ;
		
		dcb.sourcecount = (int*)malloc (sizeof(int) * dcb.NSourceFiles) ;
		dcb.sourcelines = (char ***)malloc (sizeof(char **) * dcb.NSourceFiles) ;
		file_seek (fp, dcb.OSourceFiles, SEEK_SET) ;
		for (n = 0 ; n < dcb.NSourceFiles ; n++)
		{
			file_read (fp, filename, 256) ; /* FIX PARA LOS INCLUDES */
			if (!load_file (filename, n))
				gr_con_printf ("[FXI] File %s not found", filename) ;
		}
	}

	return 1 ;
}

char * getid(unsigned int code)
{
	unsigned int n ;
	for (n = 0 ; n < dcb.NID ; n++) {
		if (dcb.id[n].Code == code) 
			return dcb.id[n].Name ;
	}
	return "(?)" ;
}
