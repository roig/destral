#include "destral_graphics_backend.h"
#include "destral_platform_backend.h"

// Idea from
// https://github.com/NoelFB/blah/blob/14a53c0f3a5b68278cc63d7c2769d2e3c750a190/src/internal/graphics_backend_gl.cpp

#define AP_GL33CORE_IMPL
#include <destral/thirdparty/ap_gl33core.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_LOG(msg) DS_LOG(msg)
#define SOKOL_ASSERT(c) dscheck(c)
#define SOKOL_EXTERNAL_GL_LOADER
#include <destral/thirdparty/sokol_gfx.h>


namespace ds::graphics_backend {
	namespace impl {}



	void init() {

		platform_backend::gl_context_create();

		// Load GL functions
		gladLoadGL();

		sg_desc desc = { 0 };
		desc.buffer_pool_size = 64;
		desc.image_pool_size = 64;
		desc.pipeline_pool_size = 64;
		sg_setup(desc);
	}

	void shutdown() {
		sg_shutdown();
	}

	void frame() {
		
	}

	void before_render() {

	}

	void after_render() {
		sg_commit();
	}


	
}

