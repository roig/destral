#pragma once
#include <destral/destral_app.h>

// Idea taken from:
// https://github.com/NoelFB/blah/blob/master/src/internal/platform_backend.h

namespace ds::platform_backend {

	// Called to initialize the platform backend
	void init(const app_config& config);

	// Called during shutdown
	void shutdown();

	// Called every frame, will poll system events
	void tick();

	// Called to present the window contents
	void present();

	// Sets the Window Fullscreen if enabled is not 0
	void set_fullscreen(bool enabled);

	// OpenGL Methods
	void* gl_get_func(const char* name);
	void* gl_context_create();
	void gl_context_make_current(void* context);
	void gl_context_destroy(void* context);

	void get_drawable_size(i32* width, i32* height);
	

}