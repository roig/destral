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


#pragma warning( push )
#pragma warning(disable : 4505) // 
#pragma warning(disable : 4996) // disable fopen warning in cute_png..
#pragma warning(disable : 4189) // 
#define CUTE_PNG_IMPLEMENTATION
#include "cute_png.h"
#pragma warning( pop )


#define AP_UUID_IMPL
#include "ap_uuid.h"

