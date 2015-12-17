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
 * FILE        : i_saveload.c
 * DESCRIPTION : Save/load functions based on varspace type info
 *
 * HISTORY:      0.83 - First version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <math.h>
#include <time.h>

#include "fxi.h"

static int savetype (file * fp, void * data, DCB_TYPEDEF * var);
static int loadtype (file * fp, void * data, DCB_TYPEDEF * var);

/*
 *  FUNCTION : loadvars
 *
 *  Load data from memory to a given file at the current file offset,
 *  using a varspace's type information
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		data			Pointer to the data
 *		var				Pointer to the type array
 *		nvars			Number of variables (length of var array)
 *
 *  RETURN VALUE :
 *      Number of bytes actually read
 *
 */

int loadvars (file * fp, void * data, DCB_VAR * var, int nvars)
{
	int result = 0;
	int partial;

	for (; nvars > 0; nvars--, var++)
	{
		partial = loadtype (fp, data, &var->Type);
		data = (Uint8*)data + partial;
		result += partial;
	}
	return result;
}


/*
 *  FUNCTION : loadtypes
 *
 *  Load data from memory to a given file at the current file offset,
 *  using type information stored in memory
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		data			Pointer to the data
 *		var				Pointer to the type array
 *		nvars			Number of variables (length of var array)
 *
 *  RETURN VALUE :
 *      Number of bytes actually read
 *
 */

int loadtypes (file * fp, void * data, DCB_TYPEDEF * var, int nvars)
{
	int result = 0;
	int partial;

	for (; nvars > 0; nvars--, var++)
	{
		partial = loadtype (fp, data, var);
		data = (Uint8*)data + partial;
		result += partial;
	}
	return result;
}

/*
 *  FUNCTION : savevars
 *
 *  Save data from memory to a given file at the current file offset,
 *  using a varspace's type information
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		data			Pointer to the data
 *		var				Pointer to the type array
 *		nvars			Number of variables (length of var array)
 *
 *  RETURN VALUE :
 *      Number of bytes actually written
 *
 */

int savevars (file * fp, void * data, DCB_VAR * var, int nvars)
{
	int result = 0;
	int partial;

	for (; nvars > 0; nvars--, var++)
	{
		partial = savetype (fp, data, &var->Type);
		data = (Uint8*) data + partial;
		result += partial;
	}
	return result;
}


/*
 *  FUNCTION : savetypes
 *
 *  Save data from memory to a given file at the current file offset,
 *  using type information stored in memory
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		data			Pointer to the data
 *		var				Pointer to the type array
 *		nvars			Number of variables (length of var array)
 *
 *  RETURN VALUE :
 *      Number of bytes actually written
 *
 */

int savetypes (file * fp, void * data, DCB_TYPEDEF * var, int nvars)
{
	int result = 0;
	int partial;

	for (; nvars > 0; nvars--, var++)
	{
		partial = savetype(fp, data, var);
		result += partial;
		data = (Uint8*)data + partial;
	}
	return result;
}

/*
 *  FUNCTION : savetype
 *
 *  Save one variable from memory to a given file at the current file offset,
 *  using the given type information. This is a convenience function
 *  used by both savevars and savetypes.
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		data			Pointer to the data
 *		var				Pointer to the variable type
 *
 *  RETURN VALUE :
 *      Number of bytes actually written
 *
 */

static int savetype (file * fp, void * data, DCB_TYPEDEF * var)
{
	int n = 0;
	int count = 1;
	int result = 0;
	const char * str;
	int len;
	int partial;

	for (;;)
	{
		switch (var->BaseType[n])
		{
			case TYPE_FLOAT:
			case TYPE_INT:
			case TYPE_DWORD:
				for (; count ; count--)
				{
					ARRANGE_DWORD (data);
					result += file_write (fp, data, 4);
					ARRANGE_DWORD (data);
					data = (Uint8*)data + 4;
				}
				break;
			case TYPE_SHORT:
			case TYPE_WORD:
				for (; count ; count--)
				{
					ARRANGE_WORD (data);
					result += file_write (fp, data, 2);
					ARRANGE_WORD (data);
					data = (Uint8*)data + 2;
				}
				break;
			case TYPE_BYTE:
			case TYPE_SBYTE:
			case TYPE_CHAR:
				result += file_write (fp, data, count);
				break;
			case TYPE_STRING:
				str = string_get(*(Uint32 *)data);
				len = strlen(str);
				ARRANGE_DWORD(&len);
				file_write (fp, &len, 4);
				file_write (fp, (void*)str, len);
				result += 4;
				break;
			case TYPE_ARRAY:
				count *= var->Count[n];
				n++;
				continue;
			case TYPE_STRUCT:
				for (; count ; count--)
				{
					partial = savevars(fp, data, dcb.varspace_vars[var->Members], dcb.varspace[var->Members].NVars);
					data = (Uint8*)data + partial;
					result += partial;
				}
				break;
			default:
				gr_error ("No es posible grabar esta estructura");
				break;
		}
		break;
	}
	return result;
}

/*
 *  FUNCTION : loadtype
 *
 *  Load one variable from a given file at the current file offset,
 *  using the given type information. This is a convenience function
 *  used by both loadvars and loadtypes.
 *
 *  PARAMS :
 *		fp				Pointer to the file object
 *		data			Pointer to the data
 *		var				Pointer to the variable type
 *
 *  RETURN VALUE :
 *      Number of bytes actually written
 *
 */

static int loadtype (file * fp, void * data, DCB_TYPEDEF * var)
{
	int n = 0;
	int count = 1;
	int result = 0;
	char * str;
	int len;
	int partial;

	for (;;)
	{
		switch (var->BaseType[n])
		{
			/* Not sure about float types */
			case TYPE_FLOAT:

			case TYPE_INT:
			case TYPE_DWORD:
				for (; count ; count--)
				{
					ARRANGE_DWORD (data);
					result += file_read (fp, data, 4);
					ARRANGE_DWORD (data);
					data = (Uint8*)data + 4;
				}
				break;
			case TYPE_SHORT:
			case TYPE_WORD:
				for (; count ; count--)
				{
					ARRANGE_WORD (data);
					result += file_read (fp, data, 2);
					ARRANGE_WORD (data);
					data = (Uint8*)data + 2;
				}
				break;
			case TYPE_SBYTE:
			case TYPE_BYTE:
			case TYPE_CHAR:
				result += file_read (fp, data, count);
				break;
			case TYPE_STRING:
				string_discard (*(Uint32*)data);
				file_read (fp, &len, 4);
				ARRANGE_DWORD(&len);
				str = malloc(len+1);
				if (str == 0)
				{
					gr_error ("loadtype: Sin memoria");
				}
				else
				{
					if (len > 0)
						file_read (fp, str, len);
					str[len] = 0;
					*(Uint32*)data = string_new(str);
					string_use(*(Uint32*)data);
					free(str);
				}
				result += 4;
				break;
			case TYPE_ARRAY:
				count *= var->Count[n];
				n++;
				continue;
			case TYPE_STRUCT:
				for (; count ; count--)
				{
					partial = loadvars(fp, data,
						dcb.varspace_vars[var->Members],
						dcb.varspace[var->Members].NVars);
					result += partial;
					data = (Uint8*)data + partial;
				}
				break;
			default:
				gr_error ("No es posible recuperar esta estructura");
				break;
		}
		break;
	}
	return result;
}

