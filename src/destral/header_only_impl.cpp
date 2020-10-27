/*
	This cpp is where we will compile all of our header only libraries
	and dependencies that have warnings
*/



#define AP_GLLOADER_IMPL
#include "ap/ap_gl33.h"

#define SOKOL_NO_DEPRECATED
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "ap/sokol_gfx.h"

#define AP_SDL_IMPL
#include "ap/ap_sdl.h"

#define AP_DEBUG_IMPL
#include "ap/ap_debug.h"


#pragma warning( push )
#pragma warning(disable : 4505) // 
#pragma warning(disable : 4996) // disable fopen warning in cute_png..
#pragma warning(disable : 4189) // 
#define CUTE_PNG_IMPLEMENTATION
#include "ap/cute_png.h"
#pragma warning( pop )


#define AP_UUID_IMPL
#include "ap/ap_uuid.h"

#define CUTE_C2_IMPLEMENTATION
#include "ap/cute_c2.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "ap/stb_truetype.h"


