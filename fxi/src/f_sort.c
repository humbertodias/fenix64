/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.85
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
 * FILE        : f_sort.c
 * DESCRIPTION : FXI functions for array sorting
 */

#include <stdlib.h>
#include <string.h>

#include "fxi.h"

int keyoffset = 0;

/*
 *	Comparison functions used by sort_variables
 */

int compare_int		(const Sint32 * a, const Sint32 * b)	{ return *(Sint32 *)((Uint8 *)a + keyoffset) - *(Sint32 *)((Uint8 *)b + keyoffset); }
int compare_dword	(const Uint32 * a, const Uint32 * b)	{ return *(Uint32 *)((Uint8 *)a + keyoffset) - *(Uint32 *)((Uint8 *)b + keyoffset); }
int compare_word	(const Uint16 * a, const Uint16 * b)	{ return *(Uint16 *)((Uint8 *)a + keyoffset) - *(Uint16 *)((Uint8 *)b + keyoffset); }
int compare_short   (const Sint16 * a, const Sint16 * b)	{ return *(Sint16 *)((Uint8 *)a + keyoffset) - *(Sint16 *)((Uint8 *)b + keyoffset); }
int compare_byte    (const Uint8  * a, const Uint8  * b)	{ return *(Uint8  *)((Uint8 *)a + keyoffset) - *(Uint8  *)((Uint8 *)b + keyoffset); }
int compare_sbyte   (const Sint8  * a, const Sint8  * b)	{ return *(Sint8  *)((Uint8 *)a + keyoffset) - *(Sint8  *)((Uint8 *)b + keyoffset); }
int compare_float   (const float  * a, const float  * b)	{ return *(float  *)((Uint8 *)a + keyoffset) - *(float  *)((Uint8 *)b + keyoffset); }
int compare_string  (const int    * a, const int    * b)	{ return strcmp(string_get(*(int *)((Uint8 *)a + keyoffset)), string_get(*(int *)((Uint8 *)b + keyoffset))); }

/*
 *  FUNCTION : dcb_typedef_reduce
 *
 *  Utility function to convert a DCB TYPEDEF (reduced type definition
 *  used in the DCB file instead of the full TYPEDEF of the compiler)
 *  to the definition of a pointer to itself.
 *
 *	Beware: it could cause a type to grow too much. In this case this
 *  function will fail with a gr_con_printf and with no operation performed.
 *
 *  PARAMS:
 *      type			Pointer to the type to be converted
 *
 *  RETURN VALUE:
 *		None
 *
 */

static void dcb_typedef_enlarge(DCB_TYPEDEF * type)
{
	int i;

	for (i = 0 ; i < MAX_TYPECHUNKS ; i++)
	{
		if (i != TYPE_POINTER && i != TYPE_ARRAY)
			break;
	}
	if (i >= MAX_TYPECHUNKS-1)
	{
		gr_con_printf ("Tipo de dato demasiado complejo");
		return;
	}
	for (i = MAX_TYPECHUNKS-1 ; i > 0 ; i--)
	{
		type->BaseType[i] = type->BaseType[i-1];
		type->Count[i]    = type->Count[i-1];
	}
	type->BaseType[0] = TYPE_POINTER;
}

/*
 *  FUNCTION : dcb_typedef_reduce
 *
 *  Utility function to convert a DCB TYPEDEF (reduced type definition
 *  used in the DCB file instead of the full TYPEDEF of the compiler)
 *  to its pointed type.
 *
 *  PARAMS:
 *      type			Pointer to the type to be converted
 *
 *  RETURN VALUE:
 *		None
 *
 */

static void dcb_typedef_reduce(DCB_TYPEDEF * type)
{
	int i;

	for (i = 0 ; i < MAX_TYPECHUNKS-1 ; i++)
	{
		type->BaseType[i] = type->BaseType[i+1];
		type->Count[i]    = type->Count[i+1];
	}
}

/*
 *  FUNCTION : dcb_typedef_size
 *
 *  Utility function to calculate the size of a variable using its
 *  DCB TYPEDEF (reduced type definition used in the DCB file)
 *
 *  PARAMS:
 *      type			Pointer to the type
 *
 *  RETURN VALUE:
 *		Size in bytes of the variable
 *
 */

static int dcb_typedef_size (const DCB_TYPEDEF * type)
{
	int i = 0;
	int count = 1;

	while (type->BaseType[i] == TYPE_ARRAY)
	{
		count *= type->Count[i];
		i++;
	}
	switch (type->BaseType[i])
	{
		case TYPE_DWORD:
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		case TYPE_POINTER:
			return 4 * count;

		case TYPE_SHORT:
		case TYPE_WORD:
			return 2 * count;

		case TYPE_CHAR:
		case TYPE_SBYTE:
		case TYPE_BYTE:
			return count;

		case TYPE_STRUCT:
		{
			unsigned int maxoffset = 0;
			unsigned int n;
			DCB_TYPEDEF * maxvar = NULL;

			for (n = 0 ; n < dcb.varspace[type->Members].NVars ; n++)
			{
				if (dcb.varspace_vars[type->Members][n].Offset > maxoffset)
				{
					maxoffset = dcb.varspace_vars[type->Members][n].Offset;
					maxvar    = &dcb.varspace_vars[type->Members][n].Type;
				}
			}
			if (maxvar == NULL)
			{
				gr_con_printf ("Estructura vacía");
				return 0;
			}

			return count * maxoffset + dcb_typedef_size(maxvar);
		}

		default:
			gr_con_printf ("Tipo desconocido");
			return 0;
	}
}

/*
 *  FUNCTION : sort_variables
 *
 *  Sorting function used by all FXI variants
 *
 *  PARAMS:
 *      data			Pointer to the start of the array
 *		key_offset		Offset of the key variable inside each element
 *		key_type		Basic type (like TYPE_INT) of the key variable
 *		element_size	Size of a single element
 *		elements		Number of elements to be sorted
 *
 *  RETURN VALUE:
 *		1 if succesful, 0 if error
 *
 */

static int sort_variables (void * data, int key_offset, int key_type, int element_size, int elements)
{
	int (*compare)(const void *, const void *) = NULL;

    keyoffset = key_offset;

	switch (key_type)
	{
		case TYPE_INT:
			compare = compare_int;
			break;
		case TYPE_WORD:
			compare = compare_word;
			break;
		case TYPE_DWORD:
			compare = compare_dword;
			break;
		case TYPE_SHORT:
			compare = compare_short;
			break;
		case TYPE_BYTE:
			compare = compare_byte;
			break;
		case TYPE_SBYTE:
			compare = compare_sbyte;
			break;
		case TYPE_CHAR:
			compare = compare_byte;
			break;
		case TYPE_STRING:
			compare = compare_string;
			break;
		case TYPE_FLOAT:
			compare = compare_float;
			break;
		default:
			gr_con_printf ("Tipo de dato usado como clave de ordenación inválido");
			return 0;
	}

	qsort (data, elements, element_size, compare);
	return 1;
}

/**
 *	SORT (variable)
 *	Sorts an array of structs or simple values. If a struct, uses the first
 *	variable as a key for sorting order.
 **/

int fxi_sort (INSTANCE * my, int * params)
{
	// Get the description of the data to be sorted

	void *			data = (void *)params[0];
	DCB_TYPEDEF *	type = (DCB_TYPEDEF *)params[1];
	DCB_TYPEDEF     copy = *type;
	int				vars = params[2];
	int				element_size;

	// Is it valid?

	if (type->BaseType[0] != TYPE_ARRAY)
	{
		gr_con_printf ("Sólo se permite ordenar un array de estructuras o valores");
		return 0;
	}
	if (vars > 1)
	{
		gr_con_printf ("Intento de ordenar una estructura con un sólo elemento");
		return 0;
	}
	if (type->Count[0] < 2)
	{
		gr_con_printf ("Intento de ordenar un array con un sólo elemento");
		return 0;
	}

	// Get the key type

	dcb_typedef_reduce(&copy);
	element_size = dcb_typedef_size(&copy);

	// If an struct or array, get its first element

	for (;;)
	{
		while (copy.BaseType[0] == TYPE_ARRAY)
			dcb_typedef_reduce(&copy);
		if (copy.BaseType[0] == TYPE_STRUCT)
			copy = dcb.varspace_vars[copy.Members][0].Type;
		else
			break;
	}

	// Do the sorting

	return sort_variables (data, 0, copy.BaseType[0], element_size, type->Count[0]);
}

/**
 *	KSORT (variable, key)
 *	Sorts an array of structs, using the given variable as a key
 **/

int fxi_ksort (INSTANCE * my, int * params)
{
	// Get the description of the data to be sorted

	void *			data = (void *)params[0];
	DCB_TYPEDEF *	type = (DCB_TYPEDEF *)params[1];
	DCB_TYPEDEF     copy = *type;
	int				vars = params[2];
	int				element_size;

	void *			key_data = (void *)params[3];
	DCB_TYPEDEF *	key_type = (DCB_TYPEDEF *)params[4];
	int				key_vars = params[5];

	// Is it valid?

	if (type->BaseType[0] != TYPE_ARRAY)
	{
		gr_con_printf ("Sólo se permite ordenar un array de estructuras o valores");
		return 0;
	}
	if (vars > 1)
	{
		gr_con_printf ("Intento de ordenar una estructura con un sólo elemento");
		return 0;
	}
	if (type->Count[0] < 2)
	{
		gr_con_printf ("Intento de ordenar un array con un sólo elemento");
		return 0;
	}

	// Check the key given

	dcb_typedef_reduce(&copy);
	element_size = dcb_typedef_size(&copy);

	if ((Uint8 *)key_data > (Uint8*)data + element_size || key_data < data)
	{
		gr_con_printf ("Intento de ordenar usando una clave externa al primer elemento");
		return 0;
	}

	copy = *key_type;

	// If an struct or array, get its first element

	for (;;)
	{
		while (copy.BaseType[0] == TYPE_ARRAY)
			dcb_typedef_reduce(&copy);
		if (copy.BaseType[0] == TYPE_STRUCT)
			copy = dcb.varspace_vars[copy.Members][0].Type;
		else
			break;
	}

	// Do the sorting

	return sort_variables (data, (Uint8*)key_data - (Uint8*)data, copy.BaseType[0], element_size, type->Count[0]);
}

/**
 *	SORT (variable, count)
 *	Sorts an array of structs or simple values. If a struct, uses the first
 *	variable as a key for sorting order. It accepts arrays of one element
 *  or a pointer to an array, unlike the simple SORT version.
 **/

int fxi_sort_n (INSTANCE * my, int * params)
{
	// Get the description of the data to be sorted

	void *			data = (void *)params[0];
	DCB_TYPEDEF *	type = (DCB_TYPEDEF *)params[1];
	DCB_TYPEDEF     copy = *type;
	int				vars = params[2];
	int				element_size;

	// Is it valid?

	if (type->BaseType[0] == TYPE_POINTER)
	{
		dcb_typedef_reduce(&copy);
		data = *(void **)data;
	}
	if (copy.BaseType[0] != TYPE_ARRAY)
	{
		dcb_typedef_enlarge(&copy);
		copy.BaseType[0] = TYPE_ARRAY;
		copy.Count[0]    = 1;
	}
	if (vars > 1)
	{
		gr_con_printf ("Intento de ordenar una estructura con un sólo elemento");
		return 0;
	}

	// Get the key type

	dcb_typedef_reduce(&copy);
	element_size = dcb_typedef_size(&copy);

	// If an struct or array, get its first element

	for (;;)
	{
		while (copy.BaseType[0] == TYPE_ARRAY)
			dcb_typedef_reduce(&copy);
		if (copy.BaseType[0] == TYPE_STRUCT)
			copy = dcb.varspace_vars[copy.Members][0].Type;
		else
			break;
	}

	// Do the sorting

	return sort_variables (data, 0, copy.BaseType[0], element_size, params[3]);
}

/**
 *	KSORT (variable, key, elements)
 *	Sorts an array of structs, using the given variable as a key and
 *  limiting the sorting to the first n elements. Accepts pointers or
 *  single elements, unlike the previous version of KSORT above.
 **/

int fxi_ksort_n (INSTANCE * my, int * params)
{
	// Get the description of the data to be sorted

	void *			data = (void *)params[0];
	DCB_TYPEDEF *	type = (DCB_TYPEDEF *)params[1];
	DCB_TYPEDEF     copy = *type;
	int				vars = params[2];
	int				element_size;

	void *			key_data = (void *)params[3];
	DCB_TYPEDEF *	key_type = (DCB_TYPEDEF *)params[4];
	int				key_vars = params[5];

	// Is it valid?

	if (type->BaseType[0] == TYPE_POINTER)
	{
		dcb_typedef_reduce(&copy);
		data = *(void **)data;
	}
	if (copy.BaseType[0] != TYPE_ARRAY)
	{
		// If the type is not an array or a pointer, it is a
		// pointer value already reduced by the bytecode

		dcb_typedef_enlarge(&copy);
		copy.BaseType[0] = TYPE_ARRAY;
		copy.Count[0]    = 1;
	}
	if (vars > 1)
	{
		gr_con_printf ("Intento de ordenar una estructura con un sólo elemento");
		return 0;
	}

	// Check the key given

	dcb_typedef_reduce(&copy);
	element_size = dcb_typedef_size(&copy);

	if ((Uint8 *)key_data > (Uint8*)data + element_size || key_data < data)
	{
		gr_con_printf ("Intento de ordenar usando una clave externa al primer elemento");
		return 0;
	}

	copy = *key_type;

	// If an struct or array, get its first element

	for (;;)
	{
		while (copy.BaseType[0] == TYPE_ARRAY)
			dcb_typedef_reduce(&copy);
		if (copy.BaseType[0] == TYPE_STRUCT)
			copy = dcb.varspace_vars[copy.Members][0].Type;
		else
			break;
	}

	// Do the sorting

	return sort_variables (data, (Uint8*)key_data - (Uint8*)data, copy.BaseType[0], element_size, params[6]);
}
