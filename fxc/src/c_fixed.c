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

#include <stdio.h>
#include <string.h>

#include "fxc.h"
#include "messages.c"

/* ---------------------------------------------------------------------- */
/* Este módulo contiene funciones para compilar valores constantes, como  */
/* los usados en la inicialización de variables. No permite variables.    */
/* ---------------------------------------------------------------------- */

extern int compile_fixed_expresion() ;

TYPEDEF fixed_expression_type;

int compile_fixed_value ()
{
	int value ;

	token_next() ;

	/* (...) */

	if (token.type == IDENTIFIER && token.code == identifier_leftp)
	{
		value = compile_fixed_expresion() ;
		token_next() ;
		if (token.type != IDENTIFIER || token.code != identifier_rightp)
			compile_error (MSG_EXPECTED, ")") ;
		return value ;
	}

	/* Numbers */

	if (token.type == NUMBER) 
	{
		fixed_expression_type = typedef_new(TYPE_DWORD);
		return token.code ;
	}

	if (token.type == FLOAT)  
	{
		fixed_expression_type = typedef_new(TYPE_FLOAT);
		return *(int *)&token.value ;
	}

	/* Strings */

	if (token.type == STRING) 
	{
		fixed_expression_type = typedef_new(TYPE_STRING);
		return token.code ;
	}

	/* Constants */

	if (token.type == IDENTIFIER)
	{
		CONSTANT * c = constants_search (token.code) ;
		if (c) 
		{
			fixed_expression_type = c->type;
			return c->value ;
		}
	}

	compile_error (MSG_CONSTANT_EXP) ;
	return 0 ;
}

int compile_fixed_factor ()
{
	token_next() ;

	if (token.type == IDENTIFIER && token.code == identifier_minus)
		return -compile_fixed_factor() ;
	if (token.type == IDENTIFIER && token.code == identifier_not)
		return !compile_fixed_factor() ;
	if (token.type == IDENTIFIER && token.code == identifier_plusplus)
		compile_error (MSG_CONSTANT_EXP) ;
	if (token.type == IDENTIFIER && token.code == identifier_minusminus)
		compile_error (MSG_CONSTANT_EXP) ;

	token_back() ;
	return compile_fixed_value() ;
}

int compile_fixed_operand ()
{
	int left = compile_fixed_factor(), right ;

	for (;;)
	{
		token_next() ;
		if (token.type == IDENTIFIER && token.code == identifier_multiply)
		{
			right = compile_fixed_operand() ;
			left *= right ;
			continue ;
		}
		if (token.type == IDENTIFIER && token.code == identifier_divide)
		{
			right = compile_fixed_operand() ;
			if (right == 0) compile_error (MSG_DIVIDE_BY_ZERO) ;
			left /= right ;
			continue ;
		}
		if (token.type == IDENTIFIER && token.code == identifier_mod)
		{
			right = compile_fixed_operand() ;
			if (right == 0) compile_error (MSG_DIVIDE_BY_ZERO) ;
			left %= right ;
			continue ;
		}
		token_back() ;
		break ;
	}
	return left ;
}

int compile_fixed_operation ()
{
	int left = compile_fixed_operand(), right ;

	for (;;)
	{
		token_next() ;
		if (token.type == IDENTIFIER && token.code == identifier_plus)
		{
			right = compile_fixed_operand() ;
			left += right ;
			continue ;
		}
		if (token.type == IDENTIFIER && token.code == identifier_minus)
		{
			right = compile_fixed_operand() ;
			left -= right ;
			continue ;
		}
		token_back() ;
		break ;
	}
	return left ;
}

int compile_fixed_rotation ()
{
	int left = compile_fixed_operation(), right ;

	token_next() ;
	if (token.type == IDENTIFIER && token.code == identifier_rol)
	{
		right = compile_fixed_rotation() ;
		return left << right ;
	}
	if (token.type == IDENTIFIER && token.code == identifier_ror)
	{
		right = compile_fixed_rotation() ;
		return left >> right ;
	}
	token_back() ;
	return left ;
}

int compile_fixed_clausule ()
{
	int left = compile_fixed_rotation (), right ;

	token_next() ;
	if (token.type == IDENTIFIER && token.code == identifier_and)
	{
		right = compile_fixed_clausule() ;
		return left & right ;
	}
	if (token.type == IDENTIFIER && token.code == identifier_or)
	{
		right = compile_fixed_clausule() ;
		return left | right ;
	}
	if (token.type == IDENTIFIER && token.code == identifier_xor)
	{
		right = compile_fixed_clausule() ;
		return left ^ right ;
	}
	token_back() ;
	return left ;
}

int compile_fixed_comparison ()
{
	int left = compile_fixed_clausule (), right ;

	token_next() ;
	if (token.type == IDENTIFIER)
	{
		if (token.code == identifier_eq)
		{
			right = compile_fixed_comparison() ;
			return left == right ;
		}
		if (token.code == identifier_gt)
		{
			right = compile_fixed_comparison() ;
			return left > right ;
		}
		if (token.code == identifier_lt)
		{
			right = compile_fixed_comparison() ;
			return left < right ;
		}
		if (token.code == identifier_gte)
		{
			right = compile_fixed_comparison() ;
			return left >= right ;
		}
		if (token.code == identifier_lte)
		{
			right = compile_fixed_comparison() ;
			return left <= right ;
		}
		if (token.code == identifier_ne)
		{
			right = compile_fixed_comparison() ;
			return left != right ;
		}
	}
	token_back() ;
	return left ;
}

int compile_fixed_expresion ()
{
	int lvalue = compile_fixed_comparison() ;

	token_next() ;
	if (token.type == IDENTIFIER)
	{
		if ( token.code == identifier_plusequal  ||
		     token.code == identifier_minusequal ||
		     token.code == identifier_multequal  ||
		     token.code == identifier_divequal   ||
		     token.code == identifier_orequal    ||
		     token.code == identifier_andequal   ||
		     token.code == identifier_xorequal      )
			compile_error (MSG_CONSTANT_EXP);
	}
	token_back() ;
	return lvalue ;
}

