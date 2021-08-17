#include <destral/destral_common.h>

#pragma warning( push )
#pragma warning(disable : 4996) // disable fopen warning
#define CUTE_ASEPRITE_IMPLEMENTATION
#include "../thirdparty/cute_aseprite.h"
#pragma warning( pop )

#define AP_GL33CORE_IMPL
#include <destral/thirdparty/ap_gl33core.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_LOG(msg) DS_LOG(msg)
#define SOKOL_ASSERT(c) dscheck(c)
#define SOKOL_EXTERNAL_GL_LOADER
#define SOKOL_NO_DEPRECATED
#include <destral/thirdparty/sokol_gfx.h>

#define STBI_ASSERT(x) dscheck(x)
#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"


#pragma warning( push )
#pragma warning(disable : 4244) // disable conversion possible loss of data
#pragma warning(disable : 4018) // disable signed/unsigned mismatch
#include "SDL_ttf.c"
#pragma warning( pop )


