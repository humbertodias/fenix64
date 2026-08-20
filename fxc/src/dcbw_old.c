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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fxc.h"
#include "dcb.h"

/* Datos externos accedidos vilmente */

extern char * string_mem ;
extern int string_offset[] ;
extern int string_count, string_used ;
extern int procdef_count ;
extern int identifier_count ;
extern PROCDEF * * procs ;

/* Gestión de la lista de ficheros a incluir en el DCB */

DCB_FILE * dcb_files = 0 ;
int        dcb_ef_alloc = 0 ;
int        dcb_filecount = 0 ;
char * *   dcb_fullname = 0 ;

void dcb_add_file (const char * filename)
{
	file * fp ;
	long   size ;
	const char * ptr ;
        int i ;

        if (filename[0] == '@')
        {
                char buffer[256] ;

                fp = file_open (filename+1, "rb") ;
                if (!fp) return ;
                while (!file_eof(fp))
                {
                        file_gets (fp, buffer, 256) ;
                        if (strchr(buffer, '\n'))
                                *strchr(buffer,'\n') = 0 ;
                        if (buffer[0] == '#' || !buffer[0])
                                continue ;
                        dcb_add_file(buffer) ;
                }
                file_close(fp) ;
                return ;
        }

        fp = file_open (filename, "rb0") ;
	if (!fp) return ;
	size = file_size (fp) ;
	file_close (fp) ;

	if (dcb_ef_alloc == dcb_filecount)
	{
		dcb_files = (DCB_FILE *)realloc (dcb_files,
				sizeof(DCB_FILE) * (dcb_ef_alloc += 16)) ;
		dcb_fullname   = (char * *)realloc (dcb_fullname, 
				sizeof(char *) * dcb_ef_alloc) ;
		if (!dcb_files || !dcb_fullname)
			compile_error ("dcb_add_file: out of memory");
	}

	ptr = filename ;
	while (strchr(ptr, '\\')) ptr = strchr(ptr,'\\')+1 ;

	if (strlen(ptr) > 55)
	{
		file_close (fp) ;
		return ;
	}

        /* Comprueba que el fichero no haya sido ya añadido */

        for (i = 0 ; i < dcb_filecount ; i++)
                if (strcmp(ptr, dcb_files[i].Name) == 0)
                        return ;

	strcpy (dcb_files[dcb_filecount].Name, ptr) ;
	dcb_files[dcb_filecount].SFile = size ;
	dcb_fullname[dcb_filecount] = strdup(filename) ;
	dcb_filecount++ ;
}

/* Hack para poder asignar ID's a los varspaces */

VARSPACE * * dcb_orig_varspace = 0 ;
int dcb_varspaces = 0 ;

static int dcb_varspace (VARSPACE * v)
{
	int n ;

	for (n = 0 ; n < dcb_varspaces ; n++)
	{
		if (dcb_orig_varspace[n] == v) 
			return n ;
	}

	dcb.varspace = (DCB_VARSPACE *) realloc (dcb.varspace,
			sizeof(DCB_VARSPACE) * (dcb_varspaces+1)) ;
	dcb_orig_varspace = (VARSPACE * *) realloc (dcb_orig_varspace,
			sizeof(VARSPACE *) * (dcb_varspaces+1)) ;

	dcb_orig_varspace[dcb_varspaces] = v ;

	dcb.varspace[dcb_varspaces].NVars = v->count ;
	dcb_varspaces++ ;

	for (n = 0 ; n < v->count ; n++)
	{
		if (v->vars[n].type.varspace)
			dcb_varspace (v->vars[n].type.varspace) ;
	}

	return dcb_varspaces-1 ;
}

/* Conversiones de TYPEDEF */

static void dcb_settype (DCB_TYPEDEF * d, TYPEDEF * t)
{
	int n ;

	for (n = 0 ; n < MAX_TYPECHUNKS ; n++)
	{
		d->BaseType[n] = t->chunk[n].type ;
		d->Count   [n] = t->chunk[n].count ;
	}

	if (t->varspace)
		d->Members = dcb_varspace (t->varspace) ;
	else
		d->Members = NO_MEMBERS ;
}

/* Función principal que crea el fichero DCB 
 * (ver dcb.h con la estructura del mismo) */

DCB_HEADER dcb ;

int dcb_save (const char * filename, int options)
{
	file * fp ;
	Uint32 n, i ;
	long offset ;
        long offset_dcbfiles ;
	identifier * id ;

	int NPriVars = 0 ;
	int SCode    = 0 ;
	int SPrivate = 0 ;
	
	fp = file_open (filename, "wb0") ;

	if (!fp)
	{
		fprintf (stdout, "Error: imposible abrir %s\n", filename) ;
		return 0 ;
	}

	/* Los varspace se van rellenando automáticamente */

	dcb.varspace = 0 ;

	/* 1. Rellenar los datos de la cabecera */

	memcpy (dcb.Header, "dcb\xD\xA\x1F\00\00", 8) ;
	dcb.Version = DCB_VERSION ;

	dcb.NProcs       = procdef_count ;
	dcb.NFiles       = dcb_filecount ;
	dcb.NID          = identifier_count ;
	dcb.NStrings     = string_count ;
	dcb.NLocVars     = local.count ;
	dcb.NLocStrings  = local.stringvar_count ;
	dcb.NGloVars     = global.count ;

	dcb.SGlobal      = globaldata->current ;
	dcb.SLocal       = localdata->current ;
	dcb.SText        = string_used ;

	dcb.NImports     = nimports ;
	dcb.NSourceFiles = n_files ;

	if (!(options & DCB_DEBUG))
	{
		dcb.NID      = 0 ;
		dcb.NLocVars = 0 ;
		dcb.NGloVars = 0 ;
	}

	/* 2. Crear tabla de procesos */

	dcb.proc = (DCB_PROC *) malloc(sizeof(DCB_PROC) * dcb.NProcs) ;

	for (n = 0 ; n < dcb.NProcs ; n++)
	{
		dcb.proc[n].ID          = procs[n]->identifier ;
		dcb.proc[n].NParams     = procs[n]->params ;
		dcb.proc[n].NPriVars    = procs[n]->privars->count ;
		dcb.proc[n].NPriStrings = procs[n]->privars->stringvar_count ;
		dcb.proc[n].NSentences  = procs[n]->sentence_count ;
		dcb.proc[n].SPrivate    = procs[n]->pridata->current ;
		dcb.proc[n].SCode       = procs[n]->code.current * 4 ;

		SCode    += dcb.proc[n].SCode ;
		NPriVars += dcb.proc[n].NPriVars ;
		SPrivate += dcb.proc[n].SPrivate ;

		if (!(options & DCB_DEBUG))
		{
			dcb.proc[n].NPriVars   = 0 ;
			dcb.proc[n].NSentences = 0 ;
		}

		dcb.proc[n].sentence = (DCB_SENTENCE *) malloc
			(sizeof(DCB_SENTENCE) * dcb.proc[n].NSentences) ;

		for (i = 0 ; i < dcb.proc[n].NSentences ; i++)
		{
			dcb.proc[n].sentence[i].NFile = 0 ;
			dcb.proc[n].sentence[i].NLine = procs[n]->sentences[i].line ;
			dcb.proc[n].sentence[i].NCol  = procs[n]->sentences[i].col ;
			dcb.proc[n].sentence[i].OCode = procs[n]->sentences[i].offset ;
		}

		dcb.proc[n].privar = (DCB_VAR *) malloc 
			(sizeof(DCB_VAR) * dcb.proc[n].NPriVars) ;

		for (i = 0 ; i < dcb.proc[n].NPriVars ; i++)
		{
			dcb_settype (&dcb.proc[n].privar[i].Type, 
				     &procs[n]->privars->vars[i].type) ;

			dcb.proc[n].privar[i].ID     = procs[n]->privars->vars[i].code ;
			dcb.proc[n].privar[i].Offset = procs[n]->privars->vars[i].offset ;
		}
	}

	/* 3. Crear tablas globales */

	dcb.id = (DCB_ID *) malloc(sizeof(DCB_ID) * dcb.NID) ;

	id = identifier_first() ;

	for (n = 0 ; n < dcb.NID ; n++)
	{
		assert (id != 0) ;
		memcpy (dcb.id[n].Name, id->name, sizeof(dcb.id[n].Name)) ;
		dcb.id[n].Code = id->code ;

		id = identifier_next(id) ;
	}

	dcb.glovar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.NGloVars) ;
	dcb.locvar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.NLocVars) ;

	for (n = 0 ; n < dcb.NGloVars ; n++)
	{
		dcb_settype (&dcb.glovar[n].Type, &global.vars[n].type) ;

		dcb.glovar[n].ID     = global.vars[n].code ;
		dcb.glovar[n].Offset = global.vars[n].offset ;
	}

	for (n = 0 ; n < dcb.NLocVars ; n++)
	{
		dcb_settype (&dcb.locvar[n].Type, &local.vars[n].type) ;

		dcb.locvar[n].ID     = local.vars[n].code ;
		dcb.locvar[n].Offset = local.vars[n].offset ;
	}

	dcb.NVarSpaces = dcb_varspaces ;

	dcb.file = dcb_files ;

	/* 4. Cálculo de offsets */

	offset = DCB_HEADER_SIZE ;

	dcb.OProcsTab    = offset ; offset += DCB_PROC_SIZE * dcb.NProcs ;
	dcb.OStrings     = offset ; offset += 4 * dcb.NStrings ;
	dcb.OGloVars     = offset ; offset += DCB_VAR_SIZE * dcb.NGloVars ;
	dcb.OLocVars     = offset ; offset += DCB_VAR_SIZE * dcb.NLocVars ;
	dcb.OLocStrings  = offset ; offset += 4 * dcb.NLocStrings ;
	dcb.OID          = offset ; offset += DCB_ID_SIZE * dcb.NID ;
	dcb.OVarSpaces   = offset ; offset += DCB_VARSPACE_SIZE * dcb.NVarSpaces ;
	dcb.OText        = offset ; offset += dcb.SText ;
	dcb.OImports     = offset ; offset += 4 * dcb.NImports ;
	dcb.OGlobal      = offset ; offset += dcb.SGlobal ;
	dcb.OLocal       = offset ; offset += dcb.SLocal ;
	dcb.OSourceFiles = offset ; offset += 256 * dcb.NSourceFiles ;

	for (n = 0 ; n < dcb.NVarSpaces ; n++)
	{
		dcb.varspace[n].OVars       = offset ; offset += DCB_VAR_SIZE * dcb.varspace[n].NVars ;
	}

	for (n = 0 ; n < dcb.NProcs ; n++)
	{
		dcb.proc[n].OSentences  = offset ; offset += DCB_SENTENCE_SIZE * dcb.proc[n].NSentences ;
		dcb.proc[n].OPriVars    = offset ; offset += DCB_VAR_SIZE * dcb.proc[n].NPriVars ;
		dcb.proc[n].OPriStrings = offset ; offset += 4 * dcb.proc[n].NPriStrings ;
		dcb.proc[n].OPrivate    = offset ; offset += dcb.proc[n].SPrivate ;
		dcb.proc[n].OCode       = offset ; offset += dcb.proc[n].SCode ;
	}

	dcb.OFilesTab   = offset ; offset += DCB_FILE_SIZE * dcb.NFiles ;

	for (n = 0 ; n < dcb.NFiles ; n++)
	{
		dcb.file[n].OFile = offset ;
		offset += dcb.file[n].SFile ;
	}

	/* 5. Guardar todo en disco ordenadamente */

	file_write (fp, &dcb, DCB_HEADER_SIZE) ;
	
	for (n = 0 ; n < dcb.NProcs ; n++)
		file_write (fp, &dcb.proc[n], DCB_PROC_SIZE) ;

	file_write (fp, &string_offset, 4 * dcb.NStrings) ;
	file_write (fp, dcb.glovar, DCB_VAR_SIZE * dcb.NGloVars) ;
	file_write (fp, dcb.locvar, DCB_VAR_SIZE * dcb.NLocVars) ;
	file_write (fp, local.stringvars, 4 * dcb.NLocStrings) ;
	file_write (fp, dcb.id, DCB_ID_SIZE * dcb.NID) ;
	file_write (fp, dcb.varspace, DCB_VARSPACE_SIZE * dcb.NVarSpaces) ;
	file_write (fp, string_mem, dcb.SText) ;
	file_write (fp, &imports, 4 * dcb.NImports) ;
	file_write (fp, globaldata->bytes, dcb.SGlobal) ;
	file_write (fp, localdata->bytes, dcb.SLocal) ;
        file_write (fp, files, 256 * dcb.NSourceFiles) ;

	for (n = 0 ; n < dcb.NVarSpaces ; n++)
	{
		VARIABLE * var ;
		DCB_VAR v ;

		var = &dcb_orig_varspace[n]->vars[0] ;

		for (i = 0 ; i < dcb.varspace[n].NVars ; i++, var++)
		{
			dcb_settype (&v.Type, &var->type) ;
			v.ID     = var->code ;
			v.Offset = var->offset ;

			file_write (fp, &v, DCB_VAR_SIZE) ;
		}
	}

	for (n = 0 ; n < dcb.NProcs ; n++)
	{
		file_write (fp, dcb.proc[n].sentence, DCB_SENTENCE_SIZE * dcb.proc[n].NSentences) ;
		file_write (fp, dcb.proc[n].privar, DCB_VAR_SIZE * dcb.proc[n].NPriVars) ;
		file_write (fp, procs[n]->privars->stringvars, 4 * dcb.proc[n].NPriStrings) ;
		file_write (fp, procs[n]->pridata->bytes, dcb.proc[n].SPrivate) ;
		file_write (fp, procs[n]->code.data, dcb.proc[n].SCode) ;
	}

        offset_dcbfiles = file_pos(fp) ;
        file_seek  (fp, offset_dcbfiles, SEEK_SET) ;
	file_write (fp, dcb.file, DCB_FILE_SIZE * dcb.NFiles) ;

	for (n = 0 ; n < dcb.NFiles ; n++)
	{
		char buffer[8192] ;
		file * fp_r = file_open (dcb_fullname[n], "rb") ;
		int chunk_size ;

		assert (fp_r) ;
		dcb.file[n].OFile = file_pos(fp) ;
		while (!file_eof(fp_r))
		{
			chunk_size = file_read  (fp_r, buffer, 8192) ;
			file_write (fp,   buffer, chunk_size) ;
                        if (chunk_size < 8192)
                                break ;
		}
		dcb.file[n].SFile = file_pos(fp) - dcb.file[n].OFile ;
                printf ("File added: %32s (%10d bytes)\n", 
                        dcb.file[n].Name, dcb.file[n].SFile) ;

		file_close (fp_r) ;
	}

        offset = file_pos(fp) ;

        file_seek  (fp, offset_dcbfiles, SEEK_SET) ;
	file_write (fp, dcb.file, DCB_FILE_SIZE * dcb.NFiles) ;

	file_close (fp) ;

	/* 6. Mostrar estadísticas */

	printf ("\nFichero %s grabado (%ld bytes):\n\n", filename, offset) ;
	printf ("          Procesos:   %8d procesos\n", dcb.NProcs) ;
	printf ("    Datos globales:   %8d bytes \n", dcb.SGlobal) ;
	printf ("     Datos locales:   %8d bytes\n", dcb.SLocal) ;
	printf ("    Datos privados:   %8d bytes\n", SPrivate) ;
	printf ("            Código:   %8d bytes\n", SCode) ;

	if (options & DCB_DEBUG)
	{
	printf ("Variables globales:   %8d variables \n", dcb.NGloVars) ;
	printf (" Variables locales:   %8d variables \n", dcb.NLocVars) ;
	printf ("Variables privadas:   %8d variables\n", NPriVars) ;
	printf ("   Identificadores:   %8d identificadores\n", dcb.NID) ;
	printf ("       Estructuras:   %8d estructuras\n", dcb.NVarSpaces) ;
	}

	printf ("           Cadenas:   %8d cadenas  (%d bytes)\n", dcb.NStrings, dcb.SText) ;

	if (dcb.NFiles)
	{
	printf ("          Ficheros:   %8d ficheros (%ld bytes)\n", dcb.NFiles, offset - dcb.OFilesTab) ;
	}

	printf ("\n") ;
	return 1 ;
}
