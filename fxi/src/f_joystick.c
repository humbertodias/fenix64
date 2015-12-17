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
 * FILE        : f_joystick.c
 * DESCRIPTION : FXI functions for joystick handling
 */

#include "fxi.h"

SDL_Joystick * _joysticks[MAX_JOYS];
SDL_Joystick * selected_joystick=NULL;


/* Internals by Splinter 2007 */

static int _open_joy(int njoy) {
    if (njoy >= 0 && njoy < MAX_JOYS) {
        if (!_joysticks[njoy]) {
            // Open new
    		_joysticks[njoy] = SDL_JoystickOpen(njoy) ;
    		if (_joysticks[njoy]) {
    		    selected_joystick = _joysticks[njoy];
    			SDL_JoystickUpdate() ;
    			return njoy ;
    		}
        } else {
            return njoy;
        }
    }
    return -1;
}

/**
 *	NUM_JOY (int CD)
 *	Returns the number of joysticks present in the system
 **/

int fxi_joy_num (INSTANCE * my, int * params)
{
	return SDL_NumJoysticks() ;
}

/**
 *	JOY_NAME (int JOY)
 *	Returns the name for a given joystick present in the system
 **/

int fxi_joy_name (INSTANCE * my, int * params)
{
	int result;
	result = string_new(SDL_JoystickName(params[0]));
	string_use(result);
	return result;
}


/**
 *	JOY_SELECT (int JOY)
 *	Returns the selected joystick number
 **/

int fxi_joy_select (INSTANCE * my, int * params)
{
    return ( _open_joy(params[0]) );
}


/* Funciones que pueden usarse especificando joystick en particular */


/**
 *	JOY_BUTTONS (int JOY)
 *	Returns the selected joystick total buttons
 **/

int fxi_joy_buttons (INSTANCE * my, int * params)
{
	if ( selected_joystick ){
		return SDL_JoystickNumButtons(selected_joystick) ;
	}
    return -1 ;
}

int fxi_joy_buttons_specific (INSTANCE * my, int * params)
{
	if ( _open_joy(params[0]) != -1){
		return SDL_JoystickNumButtons(_joysticks[params[0]]) ;
	}
    return -1 ;
}

/**
 *	JOY_AXIS (int JOY)
 *	Returns the selected joystick total axes
 **/

int fxi_joy_axes (INSTANCE * my, int * params)
{
	if ( selected_joystick ){
		return SDL_JoystickNumAxes(selected_joystick) ;
	}
    return -1 ;
}

int fxi_joy_axes_specific (INSTANCE * my, int * params)
{
	if ( _open_joy(params[0]) != -1){
		return SDL_JoystickNumAxes(_joysticks[params[0]]) ;
	}
    return -1 ;
}


/**
 *	JOY_GET_BUTTON (int JOY)
 *	Returns the selected joystick state for the given button
 **/

int fxi_joy_get_button (INSTANCE * my, int * params)
{
	if ( selected_joystick && params[0]>=0 && params[0]<=SDL_JoystickNumButtons(selected_joystick) ){
		return SDL_JoystickGetButton(selected_joystick,params[0]) ;
	}
    return -1 ;

}

int fxi_joy_get_button_specific (INSTANCE * my, int * params)
{
	if ( _open_joy(params[0]) != -1){
	    if ( params[1]>=0 && params[1]<=SDL_JoystickNumButtons(_joysticks[params[0]])){
    		return SDL_JoystickGetButton(_joysticks[params[0]],params[1]) ;
    	}
    }
    return -1 ;
}

/**
 *	JOY_GET_POSITION (int JOY)
 *	Returns the selected joystick state for the given axis
 **/

int fxi_joy_get_position (INSTANCE * my, int * params)
{
	if ( selected_joystick && params[0]>=0 && params[0]<=SDL_JoystickNumAxes(selected_joystick)){
		return SDL_JoystickGetAxis(selected_joystick,params[0]) ;
	}
    return -1 ;

}

int fxi_joy_get_position_specific (INSTANCE * my, int * params)
{
	if ( _open_joy(params[0]) != -1){
	    if ( params[1]>=0 && params[1]<=SDL_JoystickNumAxes(_joysticks[params[0]])){
    		return SDL_JoystickGetAxis(_joysticks[params[0]],params[1]) ;
    	}
    }
    return -1 ;
}
