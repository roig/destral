#include "destral_platform_backend.h"
#include "destral_input_backend.h"
#include <destral/core/destral_common.h>
#include <destral/input/destral_input.h>
#include <SDL.h>

// #include <SDL_syswm.h>

// Idea taken from: https://github.com/NoelFB/blah/blob/master/src/internal/platform_backend_sdl2.cpp

namespace ds {

	namespace impl {
		void sdl_log_redirect(void* userdata, int category, SDL_LogPriority priority, const char* message) {
			if (priority <= SDL_LOG_PRIORITY_INFO) {
				DS_LOG(message);
			} else if (priority <= SDL_LOG_PRIORITY_WARN) {
				DS_WARNING(message);

			} else {
				DS_FATAL(message);
			}
		}
	}


	struct sdl_state {
		SDL_Window* window = nullptr;

		// keyboard
		struct keyboard_key {
			bool pressed = false;
			bool repeat = false;
		};
		keyboard_key keys[SDL_NUM_SCANCODES] = { 0 };
		
		// mouse
		struct mouse_button {
			bool pressed = false;
			bool repeat = false;
		};
		mouse_button mouse_buttons[5] = {0};
		i32 mouse_x = 0;
		i32 mouse_y = 0;
		i32 mouse_relx = 0;
		i32 mouse_rely = 0;
	};

	static sdl_state g_sdl;




	namespace platform_backend {
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
			g_sdl.window = SDL_CreateWindow(config.name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.width, config.height, flags);
			if (g_sdl.window == nullptr) {
				DS_FATAL("Failed to create a Window");


			}

			// set window properties
			SDL_SetWindowResizable(g_sdl.window, SDL_TRUE);
			SDL_SetWindowMinimumSize(g_sdl.window, 256, 256);

		}

		// Called during shutdown
		void shutdown() {
			if (g_sdl.window != nullptr) {
				SDL_DestroyWindow(g_sdl.window);
			}
			g_sdl.window = nullptr;
			//displayed = false;

		/*	if (basePath != nullptr)
				SDL_free(basePath);

			if (userPath != nullptr)
				SDL_free(userPath);*/

			SDL_Quit();
		}

		// Called every frame
		void tick() {
			SDL_Event e;
			while (SDL_PollEvent(&e)) {
				switch (e.type) {
				case SDL_QUIT:
				{
					app::exit_request();
					/*auto config = app::config();
					if (config->on_exit_request != nullptr)
						config->on_exit_request();*/
				}break;
				case SDL_MOUSEWHEEL:
				{
				} break;

				case SDL_MOUSEMOTION:
				{
					input_backend::on_mouse_motion(ivec2{ e.motion.x, e.motion.y }, ivec2{ e.motion.xrel, e.motion.yrel });
				} break;

				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
				{
					input_backend::on_mouse_button_change(e.button.state == SDL_PRESSED, (ds::in::MouseButton)e.button.button);
				} break;

				case SDL_KEYUP:
				case SDL_KEYDOWN:
				{
					if (e.key.repeat == 0) {
						input_backend::on_key_change(e.key.state == SDL_PRESSED, (ds::in::Key)e.key.keysym.scancode);
					}
				} break;

				}
			}
		}

		// Called to present the window contents
		void present() {
			SDL_GL_SwapWindow(g_sdl.window);
		}

		// Sets the Window Fullscreen if enabled is not 0
		void set_fullscreen(bool enabled) {
			if (enabled)
				SDL_SetWindowFullscreen(g_sdl.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			else
				SDL_SetWindowFullscreen(g_sdl.window, 0);
		}

		void* gl_get_func(const char* name) {
			return SDL_GL_GetProcAddress(name);
		}

		void* gl_context_create() {
			void* pointer = SDL_GL_CreateContext(g_sdl.window);
			if (pointer == nullptr)
				DS_FATAL(fmt::format("SDL_GL_CreateContext failed: {}", SDL_GetError()));
			return pointer;
		}

		void gl_context_make_current(void* context) {
			SDL_GL_MakeCurrent(g_sdl.window, context);
		}

		void gl_context_destroy(void* context) {
			SDL_GL_DeleteContext(context);
		}

		void get_drawable_size(int* width, int* height) {
			dscheck(width);
			dscheck(height);
			// IMPORTANT This is only GL
			SDL_GL_GetDrawableSize(g_sdl.window, width, height);
			// If not GL use: SDL_GetWindowSize(window, width, height);

		}


	}

	namespace input_backend {
		void on_input_begin_frame() {
			g_sdl.mouse_relx = 0;
			g_sdl.mouse_rely = 0;

			// Update repeat for mouse button
			for (i32 i = 0; i < 5; i++) {
				if (g_sdl.mouse_buttons[i].pressed) {
					g_sdl.mouse_buttons[i].repeat = 1;
				}
			}


			for (i32 i = 0; i < SDL_NUM_SCANCODES; i++) {
				if (g_sdl.keys[i].pressed) {
					g_sdl.keys[i].repeat = 1;
				}
			}

		}

		void on_mouse_motion(ivec2 mouse_pos, ivec2 mouse_rel) {
			g_sdl.mouse_relx = mouse_rel.x;
			g_sdl.mouse_rely = mouse_rel.y;
			g_sdl.mouse_x = mouse_pos.x;
			g_sdl.mouse_y = mouse_pos.y;
		}

		void on_mouse_button_change(bool is_pressed, ds::in::MouseButton button) {
			g_sdl.mouse_buttons[(i32)button].pressed = is_pressed;
			g_sdl.mouse_buttons[(i32)button].repeat = 0;
		}
		
		void on_key_change(bool is_pressed, ds::in::Key key) {
			g_sdl.keys[(i32)key].repeat = 0;
			if (is_pressed) {
				g_sdl.keys[(i32)key].pressed = 1;
				//keyData.modifiers = fixedModifiers(e->key.keysym.mod);
			} else {
				g_sdl.keys[(i32)key].pressed = 0;
				//keyData.modifiers = fixedModifiers(e->key.keysym.mod);
			}
		}
	}

	// Input Backend functions
	namespace in {

		bool is_key_triggered(Key key) {
			if (key == Key::Unknown) return false;
			return g_sdl.keys[(i32)key].pressed && !g_sdl.keys[(i32)key].repeat;
		}
		bool is_key_pressed(Key key) {
			if (key == Key::Unknown) return false;
			return g_sdl.keys[(i32)key].pressed;
		}

		bool is_key_released(Key key) {
			if (key == Key::Unknown) return false;
			return !g_sdl.keys[(i32)key].pressed;
		}


		/*
			Query mouse input functions
		*/

		bool is_ms_triggered(MouseButton mouse_button) {
			if (mouse_button == MouseButton::None) return false;
			return g_sdl.mouse_buttons[(i32)mouse_button].pressed && !g_sdl.mouse_buttons[(i32)mouse_button].repeat;
		}
		
		bool is_ms_pressed(MouseButton mouse_button) {
			if (mouse_button == MouseButton::None) return false;
			return g_sdl.mouse_buttons[(i32)mouse_button].pressed;
		}
		
		bool is_ms_released(MouseButton mouse_button) {
			if (mouse_button == MouseButton::None) return false;
			return !g_sdl.mouse_buttons[(i32)mouse_button].pressed;
		}
		
		ivec2 get_ms_position() {
			return {g_sdl.mouse_x, g_sdl.mouse_y};
		}
		
		ivec2 get_ms_motion() {
			return { g_sdl.mouse_relx, g_sdl.mouse_rely };
		}


	} // in input namespace


} // ds namespace