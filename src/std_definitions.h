#ifndef SDL_INPUT_STD_DEFINITIONS_H
#define SDL_INPUT_STD_DEFINITIONS_H

/*
    If you wish to use non-standard allocation methods, you can define them
    with the input_* prefix, and they will be used instead.

    For example you can use the SDL allocation methods like so:

    #define input_malloc SDL_malloc
*/

#ifndef input_malloc

#include <stdlib.h>
#include <string.h>

#define input_malloc malloc
#define input_calloc calloc
#define input_realloc realloc
#define input_free free
#define input_memmove memmove

#endif

#endif