//
// Created by kenny on 18/06/18.
//

#include "sdl_utilities.h"


bool mouse_inside (int mouse_x, int mouse_y, SDL_Rect rect){
	//Function checks if a mouse is inside a rectangle on the screen
    if (!(mouse_x >= rect.x && mouse_x <= rect.x+rect.w))
        return false;

    if (!(mouse_y >= rect.y && mouse_y <= rect.y + rect.h))
        return false;

    return true;
}


