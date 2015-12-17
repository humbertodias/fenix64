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

#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <stdio.h>
#include <string.h>

#include "fxc.h"
#include "dcb.h"

/* Datos externos accedidos vilmente */

extern char * string_mem ;
extern int * string_offset ;
extern int string_count, string_used ;
extern int procdef_count ;
extern int identifier_count ;
extern PROCDEF ** procs ;

/* Gestión de la lista de ficheros a incluir en el DCB */

DCB_FILE * dcb_files = 0 ;
int        dcb_ef_alloc = 0 ;
int        dcb_filecount = 0 ;
char **    dcb_fullname = 0 ;

void dcb_add_file (const char * filename)
{
	file * fp ;
	long   size ;
	const char * ptr ;
    int i ;
    char    buffer[1024];

    if (filename[0] == '@')
    {
        fp = file_open (filename+1, "rb") ;
        if (!fp) return ;
        while (!file_eof(fp))
        {
            file_gets (fp, buffer, sizeof(buffer)) ;
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
		dcb_files = (DCB_FILE *)realloc (dcb_files, sizeof(DCB_FILE) * (dcb_ef_alloc += 16)) ;
		dcb_fullname   = (char **)realloc (dcb_fullname, sizeof(char *) * dcb_ef_alloc) ;
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

VARSPACE ** dcb_orig_varspace = 0 ;
int dcb_varspaces = 0 ;

static int dcb_varspace (VARSPACE * v)
{
	int n, result ;

	for (n = 0 ; n < dcb_varspaces ; n++)
	{
		if (dcb_orig_varspace[n] == v)
			return n ;
	}

	dcb.varspace = (DCB_VARSPACE *) realloc (dcb.varspace, sizeof(DCB_VARSPACE) * (dcb_varspaces+1)) ;
	dcb_orig_varspace = (VARSPACE **) realloc (dcb_orig_varspace, sizeof(VARSPACE *) * (dcb_varspaces+1)) ;

	dcb_orig_varspace[dcb_varspaces] = v ;

	dcb.varspace[dcb_varspaces].NVars = v->count ;
	result = dcb_varspaces++ ;

	for (n = 0 ; n < v->count ; n++)
	{
		if (v->vars[n].type.varspace) {
			dcb_varspace (v->vars[n].type.varspace) ;
		}
	}

	return result ;
}

/* Conversiones de TYPEDEF */

void dcb_settype (DCB_TYPEDEF * d, TYPEDEF * t)
{
	int n ;

	for (n = 0 ; n < MAX_TYPECHUNKS ; n++)
	{
		d->BaseType[n] = t->chunk[n].type ;
		d->Count   [n] = t->chunk[n].count ;
	}

	if (t->varspace) {
		d->Members = dcb_varspace (t->varspace) ;
	} else {
		d->Members = NO_MEMBERS ;
	}
}

/* Función principal que crea el fichero DCB
 * (ver dcb.h con la estructura del mismo) */

DCB_HEADER dcb ;

int dcb_save (const char * filename, int options, const char * stubname)
{
	file * fp ;
	Uint32 n, i ;
	long offset ;
    long offset_dcbfiles ;
	identifier * id ;
	long stubsize = 0;
	void * stubdata;

	int NPriVars = 0 ;
	int NPubVars = 0 ;

	int SCode    = 0 ;
	int SPrivate = 0 ;
	int SPublic  = 0 ;

	fp = file_open (filename, "wb0") ;

	if (!fp)
	{
		fprintf (stdout, "Error: imposible abrir %s\n", filename) ;
		return 0 ;
	}

	/* Write the stub file */
	if (stubname != NULL)
	{
		file * stub = file_open (stubname, "rb0");

		if (!stub)
		{
			fprintf (stdout, "Error: imposible abrir %s\n", stubname);
			return 0;
		}

		stubsize = file_size(stub);
		stubdata = malloc(stubsize);
		if (stubdata == NULL)
		{
			fprintf (stdout, "Error: Stub %s demasiado grande\n", stubname);
			return 0;
		}
		if (file_read(stub, stubdata, stubsize) != stubsize)
		{
			fprintf (stdout, "Error leyendo stub %s\n", stubname);
			return 0;
		}
		file_write (fp, stubdata, stubsize);
		if (stubsize & 15)
		{
			file_write (fp, stubdata, 16 - (stubsize & 15));
			stubsize += 16 - (stubsize & 15);
		}
		free(stubdata);
		file_close(stub);
	}

	/* Asumimos que dcb.varspace ha sido rellenado anteriormente */

	/* 1. Rellenar los datos de la cabecera */

	memcpy (dcb.data.Header, "dcb\xD\xA\x1F\00\00", 8) ;
	dcb.data.Version      = DCB_VERSION ;

	dcb.data.NProcs       = procdef_count ;
	dcb.data.NFiles       = dcb_filecount ;
	dcb.data.NID          = identifier_count ;
	dcb.data.NStrings     = string_count ;
	dcb.data.NLocVars     = local.count ;
	dcb.data.NLocStrings  = local.stringvar_count ;
	dcb.data.NGloVars     = global.count ;

	dcb.data.SGlobal      = globaldata->current ;
	dcb.data.SLocal       = localdata->current ;
	dcb.data.SText        = string_used ;

	dcb.data.NImports     = nimports ;
	dcb.data.NSourceFiles = n_files ;

    /* Splinter, Correccion para guardar info necesaria */

    if (!(options & DCB_DEBUG))
	{
	    dcb.data.NID      = 0 ;
/*	    dcb.data.NLocVars = 0 ;
	    dcb.data.NGloVars = 0 ;
*/	}

	/* 2. Crear tabla de procesos */

	dcb.proc = (DCB_PROC *) malloc(sizeof(DCB_PROC) * dcb.data.NProcs) ;

	for (n = 0 ; n < dcb.data.NProcs ; n++)
	{
		dcb.proc[n].data.ID          = procs[n]->identifier ;
		dcb.proc[n].data.Flags       = procs[n]->flags ;
		dcb.proc[n].data.NParams     = procs[n]->params ;

		dcb.proc[n].data.NPriVars    = procs[n]->privars->count ;
		dcb.proc[n].data.NPriStrings = procs[n]->privars->stringvar_count ;

		dcb.proc[n].data.NPubVars    = procs[n]->pubvars->count ;
		dcb.proc[n].data.NPubStrings = procs[n]->pubvars->stringvar_count ;

		dcb.proc[n].data.NSentences  = procs[n]->sentence_count ;

		dcb.proc[n].data.SPrivate    = procs[n]->pridata->current ;
		dcb.proc[n].data.SPublic     = procs[n]->pubdata->current ;

		dcb.proc[n].data.SCode       = procs[n]->code.current * 4 ;

		dcb.proc[n].data.OExitCode   = procs[n]->exitcode ;

		SCode    += dcb.proc[n].data.SCode ;

		NPriVars += dcb.proc[n].data.NPriVars ;
		NPubVars += dcb.proc[n].data.NPubVars ;

		SPrivate += dcb.proc[n].data.SPrivate ;
		SPublic  += dcb.proc[n].data.SPublic ;

		dcb.proc[n].sentence = (DCB_SENTENCE *) malloc(sizeof(DCB_SENTENCE) * dcb.proc[n].data.NSentences) ;

		for (i = 0 ; i < dcb.proc[n].data.NSentences ; i++)
		{
			dcb.proc[n].sentence[i].NFile = 0 ;
			dcb.proc[n].sentence[i].NLine = procs[n]->sentences[i].line ;
			dcb.proc[n].sentence[i].NCol  = procs[n]->sentences[i].col ;
            dcb.proc[n].sentence[i].OCode = procs[n]->sentences[i].offset ;
        }

        /* Splinter, tipos de parametros */

        dcb.proc[n].privar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.proc[n].data.NPriVars) ;
        for (i = 0 ; i < dcb.proc[n].data.NPriVars ; i++)
        {
                dcb_settype (&dcb.proc[n].privar[i].Type, &procs[n]->privars->vars[i].type) ;

                dcb.proc[n].privar[i].ID     = procs[n]->privars->vars[i].code ;
                dcb.proc[n].privar[i].Offset = procs[n]->privars->vars[i].offset ;
        }

        dcb.proc[n].pubvar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.proc[n].data.NPubVars) ;
        for (i = 0 ; i < dcb.proc[n].data.NPubVars ; i++)
        {
                dcb_settype (&dcb.proc[n].pubvar[i].Type, &procs[n]->pubvars->vars[i].type) ;

                dcb.proc[n].pubvar[i].ID     = procs[n]->pubvars->vars[i].code ;
                dcb.proc[n].pubvar[i].Offset = procs[n]->pubvars->vars[i].offset ;
        }
    }

	/* 3. Crear tablas globales */

	dcb.id = (DCB_ID *) malloc(sizeof(DCB_ID) * dcb.data.NID) ;

	id = identifier_first() ;

	for (n = 0 ; n < dcb.data.NID ; n++)
	{
		assert (id != 0) ;
		strncpy (dcb.id[n].Name, id->name, sizeof(dcb.id[n].Name)) ;
		dcb.id[n].Code = id->code ;

		id = identifier_next(id) ;
	}

	dcb.glovar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.data.NGloVars) ;
	dcb.locvar = (DCB_VAR *) malloc(sizeof(DCB_VAR) * dcb.data.NLocVars) ;

	for (n = 0 ; n < dcb.data.NGloVars ; n++)
	{
		dcb_settype (&dcb.glovar[n].Type, &global.vars[n].type) ;

		dcb.glovar[n].ID     = global.vars[n].code ;
		dcb.glovar[n].Offset = global.vars[n].offset ;
	}

	for (n = 0 ; n < dcb.data.NLocVars ; n++)
	{
		dcb_settype (&dcb.locvar[n].Type, &local.vars[n].type) ;

		dcb.locvar[n].ID     = local.vars[n].code ;
		dcb.locvar[n].Offset = local.vars[n].offset ;
	}

	dcb.data.NVarSpaces = dcb_varspaces ;

	dcb.file = dcb_files ;

	/* 4. Cálculo de offsets */

	offset = sizeof(DCB_HEADER_DATA) ;

	dcb.data.OProcsTab    = offset ; offset += sizeof(DCB_PROC_DATA) * dcb.data.NProcs ;
	dcb.data.OStrings     = offset ; offset += 4 * dcb.data.NStrings ;
	dcb.data.OGloVars     = offset ; offset += sizeof(DCB_VAR) * dcb.data.NGloVars ;
	dcb.data.OLocVars     = offset ; offset += sizeof(DCB_VAR) * dcb.data.NLocVars ;
	dcb.data.OLocStrings  = offset ; offset += 4 * dcb.data.NLocStrings ;
	dcb.data.OID          = offset ; offset += sizeof(DCB_ID) * dcb.data.NID ;
	dcb.data.OVarSpaces   = offset ; offset += sizeof(DCB_VARSPACE) * dcb.data.NVarSpaces ;
	dcb.data.OText        = offset ; offset += dcb.data.SText ;
	dcb.data.OImports     = offset ; offset += 4 * dcb.data.NImports ;
	dcb.data.OGlobal      = offset ; offset += dcb.data.SGlobal ;
	dcb.data.OLocal       = offset ; offset += dcb.data.SLocal ;
	dcb.data.OSourceFiles = offset ; offset += 256 * dcb.data.NSourceFiles ;

	for (n = 0 ; n < dcb.data.NVarSpaces ; n++)
	{
		dcb.varspace[n].OVars       = offset ; offset += sizeof(DCB_VAR) * dcb.varspace[n].NVars ;
	}

	for (n = 0 ; n < dcb.data.NProcs ; n++)
    {
        dcb.proc[n].data.OSentences  = offset ; offset += sizeof(DCB_SENTENCE) * dcb.proc[n].data.NSentences ;
        /* Privadas */
        dcb.proc[n].data.OPriVars    = offset ; offset += sizeof(DCB_VAR) * dcb.proc[n].data.NPriVars ;
		dcb.proc[n].data.OPriStrings = offset ; offset += 4 * dcb.proc[n].data.NPriStrings ;
		dcb.proc[n].data.OPrivate    = offset ; offset += dcb.proc[n].data.SPrivate ;
		/* Publicas */
		dcb.proc[n].data.OPubVars    = offset ; offset += sizeof(DCB_VAR) * dcb.proc[n].data.NPubVars ;
		dcb.proc[n].data.OPubStrings = offset ; offset += 4 * dcb.proc[n].data.NPubStrings ;
		dcb.proc[n].data.OPublic     = offset ; offset += dcb.proc[n].data.SPublic ;
		/* Code */
		dcb.proc[n].data.OCode       = offset ; offset += dcb.proc[n].data.SCode ;
	}

	dcb.data.OFilesTab   = offset ; offset += sizeof(DCB_FILE) * dcb.data.NFiles ;

	for (n = 0 ; n < dcb.data.NFiles ; n++)
	{
		dcb.file[n].OFile = offset ;
		offset += dcb.file[n].SFile ;
	}

	/* 5. Guardar todo en disco ordenadamente */

	file_write (fp, &dcb, sizeof(DCB_HEADER_DATA)) ;

	for (n = 0 ; n < dcb.data.NProcs ; n++)
		file_write (fp, &dcb.proc[n], sizeof(DCB_PROC_DATA)) ;

	file_write (fp, string_offset, 4 * dcb.data.NStrings) ;
	file_write (fp, dcb.glovar, sizeof(DCB_VAR) * dcb.data.NGloVars) ;
	file_write (fp, dcb.locvar, sizeof(DCB_VAR) * dcb.data.NLocVars) ;
	file_write (fp, local.stringvars, 4 * dcb.data.NLocStrings) ;
	file_write (fp, dcb.id, sizeof(DCB_ID) * dcb.data.NID) ;
	file_write (fp, dcb.varspace, sizeof(DCB_VARSPACE) * dcb.data.NVarSpaces) ;
	file_write (fp, string_mem, dcb.data.SText) ;
	file_write (fp, &imports, 4 * dcb.data.NImports) ;
	file_write (fp, globaldata->bytes, dcb.data.SGlobal) ;
	file_write (fp, localdata->bytes, dcb.data.SLocal) ;
    file_write (fp, files, 256 * dcb.data.NSourceFiles) ;

	for (n = 0 ; n < dcb.data.NVarSpaces ; n++)
	{
		VARIABLE * var ;
		DCB_VAR v ;

		var = &dcb_orig_varspace[n]->vars[0] ;

		for (i = 0 ; i < dcb.varspace[n].NVars ; i++, var++)
		{
			dcb_settype (&v.Type, &var->type) ;
			v.ID     = var->code ;
			v.Offset = var->offset ;

			file_write (fp, &v, sizeof(DCB_VAR)) ;
		}
	}

	for (n = 0 ; n < dcb.data.NProcs ; n++)
    {
        file_write (fp, dcb.proc[n].sentence, sizeof(DCB_SENTENCE) * dcb.proc[n].data.NSentences) ;
        /* Privadas */
        file_write (fp, dcb.proc[n].privar, sizeof(DCB_VAR) * dcb.proc[n].data.NPriVars) ;
		file_write (fp, procs[n]->privars->stringvars, 4 * dcb.proc[n].data.NPriStrings) ;
		file_write (fp, procs[n]->pridata->bytes, dcb.proc[n].data.SPrivate) ;
        /* Publicas */
		file_write (fp, dcb.proc[n].pubvar, sizeof(DCB_VAR) * dcb.proc[n].data.NPubVars) ;
		file_write (fp, procs[n]->pubvars->stringvars, 4 * dcb.proc[n].data.NPubStrings) ;
		file_write (fp, procs[n]->pubdata->bytes, dcb.proc[n].data.SPublic) ;
		/* Code */
		file_write (fp, procs[n]->code.data, dcb.proc[n].data.SCode) ;
	}

    offset_dcbfiles = file_pos(fp) ;
    file_seek  (fp, offset_dcbfiles, SEEK_SET) ;
	file_write (fp, dcb.file, sizeof(DCB_FILE) * dcb.data.NFiles) ;

	for (n = 0 ; n < dcb.data.NFiles ; n++)
	{
		char buffer[8192] ;
		file * fp_r = file_open (dcb_fullname[n], "rb") ;
		int chunk_size ;

		assert (fp_r) ;
		dcb.file[n].OFile = file_pos(fp) ;
		while (!file_eof(fp_r))
		{
			chunk_size = file_read  (fp_r, buffer, 8192) ;
			file_write (fp, buffer, chunk_size) ;
            if (chunk_size < 8192) {
                break ;
            }
		}
		dcb.file[n].SFile = file_pos(fp) - dcb.file[n].OFile ;

        printf ("File added: %32s (%10d bytes)\n", dcb.file[n].Name, dcb.file[n].SFile) ;

		file_close (fp_r) ;
	}

    offset = file_pos(fp) ;
	file_seek  (fp, offset_dcbfiles, SEEK_SET) ;
	file_write (fp, dcb.file, sizeof(DCB_FILE) * dcb.data.NFiles) ;

	/* Write the stub signature */

	if (stubname != NULL)
	{
		struct
		{
			char	magic[12];
			int		dcb_offset;
		}
		dcb_signature;

	    /* Voy al final del archivo */
	    file_seek  (fp, offset, SEEK_SET) ;

		strcpy (dcb_signature.magic, "DCB Stub\x1A\x0D\x0A");
		dcb_signature.dcb_offset = (int)stubsize;

		ARRANGE_DWORD(&dcb_signature.dcb_offset);

		file_write (fp, &dcb_signature, sizeof(dcb_signature));
	}

	file_close (fp) ;

	/* 6. Mostrar estadísticas */

	printf ("\nFichero %s grabado (%ld bytes):\n\n", filename, offset) ;
	printf ("          Procesos:   %8d procesos\n", dcb.data.NProcs) ;
	printf ("    Datos globales:   %8d bytes \n", dcb.data.SGlobal) ;
	printf ("     Datos locales:   %8d bytes\n", dcb.data.SLocal) ;
	printf ("    Datos privados:   %8d bytes\n", SPrivate) ;
	printf ("    Datos publicos:   %8d bytes\n", SPublic) ;
	printf ("            Codigo:   %8d bytes\n", SCode) ;

    printf ("Variables globales:   %8d variables \n", dcb.data.NGloVars) ;
    printf (" Variables locales:   %8d variables \n", dcb.data.NLocVars) ;
    printf ("Variables privadas:   %8d variables\n", NPriVars) ;
    printf ("Variables publicas:   %8d variables\n", NPubVars) ;
    printf ("   Identificadores:   %8d identificadores\n", dcb.data.NID) ;
    printf ("       Estructuras:   %8d estructuras\n", dcb.data.NVarSpaces) ;
    printf ("           Cadenas:   %8d cadenas  (%d bytes)\n", dcb.data.NStrings, dcb.data.SText) ;

	if (dcb.data.NFiles) {
    	printf ("          Ficheros:   %8d ficheros (%ld bytes)\n", dcb.data.NFiles, offset - dcb.data.OFilesTab) ;
	}

	printf ("\n") ;

	return 1 ;
}
