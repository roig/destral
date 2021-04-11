#include "destral_platform_backend.h"
#include <destral/core/destral_common.h>
#include <SDL.h>

//#include <SDL_syswm.h>

// Idea taken from: https://github.com/NoelFB/blah/blob/master/src/internal/platform_backend_sdl2.cpp

namespace ds::platform_backend {
	namespace impl {
		void sdl_log_redirect(void* userdata, int category, SDL_LogPriority priority, const char* message) 	{
			if (priority <= SDL_LOG_PRIORITY_INFO)
				DS_LOG(message);
			else if (priority <= SDL_LOG_PRIORITY_WARN)
				DS_WARNING(message);
			else
				DS_FATAL(message);
		}
	}
	static SDL_Window* g_window = nullptr;

	// Called to initialize the platform backend
	void init(const app::config& config) {

		// TODO: Redirect the sdl logs to our own function
		// control this via some kind of config flag
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
		SDL_LogSetOutputFunction(impl::sdl_log_redirect, nullptr);


		// TODO: Print SDL version to the log
		SDL_version version;
		SDL_GetVersion(&version);
		DS_LOG(fmt::format("SDL v.{}.{}.{}", version.major, version.minor, version.patch));


		// initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
			DS_FATAL("Failed to initialize SDL2");
		}

		int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;


		// For now only OpenGL33:
		flags |= SDL_WINDOW_OPENGL;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

		//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


		// create the window
		g_window = SDL_CreateWindow(config.name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.width, config.height, flags);
		if (g_window == nullptr) {
			DS_FATAL("Failed to create a Window");


		}

		// set window properties
		SDL_SetWindowResizable(g_window, SDL_TRUE);
		SDL_SetWindowMinimumSize(g_window, 256, 256);

	}

	// Called during shutdown
	void shutdown() {
		if (g_window != nullptr) {
			SDL_DestroyWindow(g_window);
		}
		g_window = nullptr;
		//displayed = false;

	/*	if (basePath != nullptr)
			SDL_free(basePath);

		if (userPath != nullptr)
			SDL_free(userPath);*/

		SDL_Quit();
	}

	// Called every frame
	void frame() {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				app::exit_request();
				/*auto config = app::config();
				if (config->on_exit_request != nullptr)
					config->on_exit_request();*/
			}
		}
	}

	// Called to present the window contents
	void present() {
		SDL_GL_SwapWindow(g_window);
	}

	// Sets the Window Fullscreen if enabled is not 0
	void set_fullscreen(bool enabled) {
		if (enabled)
			SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		else
			SDL_SetWindowFullscreen(g_window, 0);
	}

	void* gl_get_func(const char* name) {
		return SDL_GL_GetProcAddress(name);
	}

	void* gl_context_create() {
		void* pointer = SDL_GL_CreateContext(g_window);
		if (pointer == nullptr)
			DS_FATAL(fmt::format("SDL_GL_CreateContext failed: {}", SDL_GetError()));
		return pointer;
	}

	void gl_context_make_current(void* context) {
		SDL_GL_MakeCurrent(g_window, context);
	}

	void gl_context_destroy(void* context) {
		SDL_GL_DeleteContext(context);
	}

}