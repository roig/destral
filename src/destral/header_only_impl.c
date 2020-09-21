/*
	This cpp is where we will compile all of our header only libraries.
*/

#define AP_GLLOADER_IMPL
#include "ap_gl33.h"

#define SOKOL_NO_DEPRECATED
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"


#define AP_SDL_IMPL
#include "ap_sdl.h"

#define AP_DEBUG_IMPL
#include "ap_debug.h"
