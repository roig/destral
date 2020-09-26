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
#pragma warning( disable : 4456 )
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#pragma warning( pop )


#pragma warning( push )
#pragma warning( disable : 4221 )
#pragma warning( disable : 4706 )
#pragma warning( disable : 4702 )
#include "flecs.c"
#pragma warning( pop )