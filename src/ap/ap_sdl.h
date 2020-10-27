#pragma once
/*
	ap_sdl.h -- Astral Pixel SDL2 OpenGL wrapper

    Do this :
        #define AP_SDL_IMPL
    before including this file in one C/C++ file to create the implementation.

	OVERVIEW
	========
	The ds sdl header currently implements:
	- Tiny abstraction around SDL input (keyboard, mouse)
	- Window opengl creation



	USAGE:
	========
	------- APP  -------
	Easy mode:

	ap_sdl_app_run(...)

	This creates initializes a window and a renderer and loops the application.
	You can set up callbacks to get called in each step of the application


	------- VIDEO -------
	
	In order to open an opengl window use the function:
	- ap_sdl_create_window


	------- INPUT -------
	Example of a main loop to update the input:


	while(true) {
		ap_sdl_begin_frame();

		// here you can
	}
*/

#include <SDL_video.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <SDL_render.h>


#ifdef __cplusplus
extern "C" {
#endif


/****************
	WINDOW
*****************/

/** Creates a default SDL2 accelerated renderer window. 
	Non owning pointers of out_window and out_renderer, user must explicitly destroy them 
*/
int ap_sdl_create_window(const char* window_title, int width, int height, SDL_Window** out_window, SDL_Renderer** out_renderer);

/** Creates a default opengl 3.3 window. 
	Non owning pointers of out_window and out_gl_ctx, user must explicitly destroy them
*/
int ap_sdl_create_window_gl(const char* window_name, int width, int height, SDL_Window** out_window, SDL_GLContext* out_gl_ctx);

/****************
	RENDERING
*****************/

/** 
	Loads a SDL_Texture from a file. (this is only valid when using the SDL2 accelerated renderer.
	It only supports .png files.
	return SDL_Texture* of the created texture. You must call SDL_DestroyTexture in order to destroy that texture.
*/
SDL_Texture* ap_sdl_texture_load(const char* file_name, SDL_Renderer* renderer);




struct ap_sdl_ms_butt_state {
	int8_t pressed;
	int8_t repeat;
};

struct ap_sdl_ms_state {
	struct ap_sdl_ms_butt_state buttons[5];

	//X,Y relative to window
	Sint32 x;
	Sint32 y;

	// frame motion
	Sint32 xrel;
	Sint32 yrel;
};


/*
	ap_sdl_KeyboardButtonState

	ap_sdl_KeyboardButtonState defines the state of a keyboard button.
	- pressed means that the button is pressed in that frame
	- repeat means that the button is pressed after the first frame that is pressed.
	When button is released both properties are set to 0.
*/
struct ap_sdl_KeyboardButtonState {
	int8_t pressed;
	int8_t repeat;

	// mask of modifiers
	//int modifiers;
};

struct ap_sdl_KeyboardState {
	struct ap_sdl_KeyboardButtonState keys[SDL_NUM_SCANCODES];
};

/****************
	INPUT
*****************/

/*
	ap_sdl_input

	Holds the current state of the input (keyboard/mouse)
*/
struct ap_sdl_input {
	struct ap_sdl_KeyboardState kb;
	struct ap_sdl_ms_state ms;
};

/*
	Query keyboard input functions
*/

/** Check if the key is released in this frame (only returns 1 the frame that state switchs from pressed to released) Example: SDLK_PLUS*/
int ap_sdl_key_triggered(struct ap_sdl_input* state, SDL_Keycode key);
/** Check if the key is pressed*/
int ap_sdl_key_pressed(struct ap_sdl_input* state, SDL_Keycode key);
/** Check if the key is released*/
int ap_sdl_key_released(struct ap_sdl_input* state, SDL_Keycode key);


/*
	Query mouse input functions
*/

/** Check if the key is released in this frame (only returns 1 the frame that state switchs from pressed to released) Example: SDL_BUTTON_LEFT*/
int ap_sdl_ms_triggered(struct ap_sdl_input* state, Uint8 mouse_button);
/** Check if the key is pressed Example: SDL_BUTTON_LEFT*/
int ap_sdl_ms_pressed(struct ap_sdl_input* state, Uint8 mouse_button);
/** Check if the key is released Example: SDL_BUTTON_LEFT*/
int ap_sdl_ms_released(struct ap_sdl_input* state, Uint8 mouse_button);
/** Returns the position of the mouse: {0,0} is at the top-left of the window. units are in pixels (x+ right) (y+ down)*/
void ap_sdl_ms_position(struct ap_sdl_input* state, Sint32* x, Sint32* y);
/** Returns the motion done in this frame, units are in pixels (x+ right) (y+ down)*/
void ap_sdl_ms_motion(struct ap_sdl_input* state, Sint32* x, Sint32* y);


/*
	Update input state
*/
/** Updates the state of the inputs at the start of the frame, before polling events*/
void ap_sdl_input_begin_frame(struct ap_sdl_input* state);

/** Updates the data of the inputs based on the SDL input events*/
void ap_sdl_input_handle_event(struct ap_sdl_input* state, SDL_Event* e);


/*
	MAIN LOOP APPLICATION
*/

struct ap_sdl_app_desc {
	uint32_t _start_canary;
	void (*init_cb)(void* user_data);
	void (*frame_cb)(void* user_data);
	void (*cleanup_cb)(void* user_data);
	void (*event_cb)(SDL_Event*, void* user_data);
	int renderer_mode; // mode 0 -> SDL2 renderer mode = 1 GL
	const char* window_title;
	int wwidth; // window width
	int wheight; // window height
	uint32_t _end_canary;
};

/**
	Runs an application loop that handles input and events and calls callback functions for the user
	The callbacks will be called with the user data passed by pointer in this function.
*/
int ap_sdl_app_run(struct ap_sdl_app_desc* desc, void* user_data);

/** Returns the current delta time in seconds for this frame (must be called on frame callback)*/
float ap_sdl_app_dt();

/** Returns the current fps value for the application (must be called on frame callback)*/
float ap_sdl_app_fps();

/** Returns the window pointer of the application */
SDL_Window* ap_sdl_app_window();

/** Returns the SDL2 renderer pointer of the application */
SDL_Renderer* ap_sdl_app_renderer();

/** Returns the input pointer structure of the application (window)*/
struct ap_sdl_input* ap_sdl_app_input();



#ifdef __cplusplus
}
#endif
/*--- IMPLEMENTATION ---------------------------------------------------------*/
//#define AP_SDL_IMPL
#ifdef AP_SDL_IMPL
#include <assert.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_log.h>
#include <SDL_keyboard.h>


/** Creates a default SDL2 renderer window **/
int ap_sdl_create_window(const char* window_title, int window_width, int window_height, SDL_Window** out_window, SDL_Renderer** out_renderer) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	//Create window
	*out_window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!out_window) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	*out_renderer = SDL_CreateRenderer(*out_window, -1, SDL_RENDERER_ACCELERATED);
	if (!out_renderer) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create  renderer: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/** Creates a default opengl 3.3 window. */
int ap_sdl_create_window_gl(const char* window_title, int window_width, int window_height, SDL_Window** out_window, SDL_GLContext* out_gl_ctx) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return EXIT_FAILURE;
	}


	// Debug context flag
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, params->is_gl_context_debug ? SDL_GL_CONTEXT_DEBUG_FLAG : 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//Create window
	*out_window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!out_window) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	*out_gl_ctx = SDL_GL_CreateContext(*out_window);
	if (!out_gl_ctx) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create gl context: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/** Application globals */
SDL_Window* g_window;
SDL_Renderer* g_renderer;
SDL_GLContext g_glctx;

struct ap_sdl_input g_app_input = { 0 };
float g_tm_dtacc_s;
float g_tm_fps;


SDL_Window* ap_sdl_app_window() {
	return g_window;
}

SDL_Renderer* ap_sdl_app_renderer() {
	return g_renderer;
}


struct ap_sdl_input* ap_sdl_app_input() {
	return &g_app_input;
}

float ap_sdl_app_dt() {
	return g_tm_dtacc_s;
}

float ap_sdl_app_fps() {
	return g_tm_fps;
}

#define AP_GL33COMPATLOADER_IMPL
#include "ap_gl33compat.h"

int ap_sdl_app_run(struct ap_sdl_app_desc* desc, void* user_data) {
	assert(desc);
	assert((desc->_start_canary == 0) && (desc->_end_canary == 0));

	if (desc->renderer_mode == 0) {
		if (ap_sdl_create_window(desc->window_title, desc->wwidth, desc->wheight, &g_window, &g_renderer) == EXIT_FAILURE) {
			return EXIT_FAILURE;
		}
	} else {
		if (ap_sdl_create_window_gl(desc->window_title, desc->wwidth, desc->wheight, &g_window, &g_glctx) == EXIT_FAILURE) {
			return EXIT_FAILURE;
		}
		gladLoadGL();
	}

	if (desc->init_cb) desc->init_cb(user_data);
	int app_ended = 0;

	// Time related initialization
	Uint64 tm_ts_last = SDL_GetPerformanceCounter();
	g_tm_dtacc_s = 0.0f;
	g_tm_fps = 0.0f;
	const float MAX_FPS = 200.0f;
	const float MIN_FPS = 2.0f;  // 1 / 2 FPS = 500 milliseconds = > 0.5 seconds
	const float MIN_DELTA = 1.0f / MAX_FPS;
	const float MAX_DELTA = 1.0f / MIN_FPS;

	while (!app_ended) {
		// seconds = units / persecond      OR     units = seconds * persecond
		// add current delta in seconds (now - last) / frequency
		const Uint64 tm_ts_now = SDL_GetPerformanceCounter();
		g_tm_dtacc_s += (tm_ts_now - tm_ts_last) / (float) SDL_GetPerformanceFrequency();
		tm_ts_last = tm_ts_now;
		
		// Avoid death spiral (TODO MIRAR BE COM FUNCIONA L'SPIRAL DEATH)
		if (g_tm_dtacc_s > MAX_DELTA) {
			g_tm_dtacc_s = MAX_DELTA;
			// We are going very slow! discard some delta
		}

		if (g_tm_dtacc_s < MIN_DELTA) {
			// We are going too fast!! skip this frame
			continue;
		}
		g_tm_fps = 1.0f / g_tm_dtacc_s;
		// 1 start frame (handle events)
		ap_sdl_input_begin_frame(&g_app_input);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			// Handle input events
			ap_sdl_input_handle_event(&g_app_input, &e);

			// Close window event
			if (e.type == SDL_WINDOWEVENT && 
				e.window.event == SDL_WINDOWEVENT_CLOSE &&
				e.window.windowID == SDL_GetWindowID(g_window)) {
				app_ended = 1;
				continue;
			}

			// handle other events in user land
			if (desc->event_cb) desc->event_cb(&e, user_data);
		}

		//printf("DT: %f   FPS: %f\n", g_tm_dtacc_s, 1.0f/ g_tm_dtacc_s);
		// 2 Handle the frame update
		if (desc->frame_cb) desc->frame_cb(user_data);

		// Blit the screen
		if (desc->renderer_mode == 0) {
			SDL_RenderPresent(g_renderer);
		} else {
			SDL_GL_SwapWindow(g_window);
		}

		// reset elapsed accumulator
		g_tm_dtacc_s = 0;
	}

	// user cleanup callback
	if (desc->cleanup_cb) desc->cleanup_cb(user_data);

	// sdl cleanup
	if (desc->renderer_mode == 0) {
		SDL_DestroyRenderer(g_renderer);
	} else {
		SDL_GL_DeleteContext(g_glctx);
	}
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	return EXIT_SUCCESS;
}





int ap_sdl_key_triggered(struct ap_sdl_input* state, SDL_Keycode iKeyCode) {
	struct ap_sdl_KeyboardButtonState* key = &state->kb.keys[(SDL_GetScancodeFromKey(iKeyCode))];
	return key->pressed && !key->repeat;
}

int ap_sdl_key_pressed(struct ap_sdl_input* state, SDL_Keycode iKeyCode) {
	struct ap_sdl_KeyboardButtonState* key = &state->kb.keys[(SDL_GetScancodeFromKey(iKeyCode))];
	return key->pressed;
}

int ap_sdl_key_released(struct ap_sdl_input* state, SDL_Keycode iKeyCode) {
	struct ap_sdl_KeyboardButtonState* key = &state->kb.keys[(SDL_GetScancodeFromKey(iKeyCode))];
	return !key->pressed;
}


struct ap_sdl_ms_butt_state* APSI_GetMouseButtonState(struct ap_sdl_input* state, Uint8 mouse_button) {
	assert((mouse_button <= SDL_BUTTON_X2) || (mouse_button >= SDL_BUTTON_LEFT));
	return &state->ms.buttons[mouse_button - 1];
}

int ap_sdl_ms_triggered(struct ap_sdl_input* state, Uint8 mouse_button) {
	struct ap_sdl_ms_butt_state* but = APSI_GetMouseButtonState(state, mouse_button);
	return but->pressed && !but->repeat;
}

int ap_sdl_ms_pressed(struct ap_sdl_input* state, Uint8 mouse_button) {
	struct ap_sdl_ms_butt_state* but = APSI_GetMouseButtonState(state, mouse_button);
	return but->pressed;
}

int ap_sdl_ms_released(struct ap_sdl_input* state, Uint8 mouse_button) {
	struct ap_sdl_ms_butt_state* but = APSI_GetMouseButtonState(state, mouse_button);
	return !but->pressed;
}

void ap_sdl_ms_position(struct ap_sdl_input* state, Sint32* x, Sint32* y) {
	*x = state->ms.x;
	*y = state->ms.y;
}

void ap_sdl_ms_motion(struct ap_sdl_input* state, Sint32* relx, Sint32* rely) {
	*relx = state->ms.xrel;
	*rely = state->ms.yrel;
}

void ap_sdl_input_begin_frame(struct ap_sdl_input* state) {
	// Reset mouse relative motion each frame
	state->ms.xrel = 0;
	state->ms.yrel = 0;

	// Update repeat for mouse button
	int s = sizeof(state->ms.buttons) / sizeof(struct ap_sdl_ms_butt_state);
	for (Uint8 i = 0; i < s; i++) {
		struct ap_sdl_ms_butt_state* ms_but_data = APSI_GetMouseButtonState(state, i);
		if (ms_but_data->pressed) {
			ms_but_data->repeat = 1;
		}
	}
}



void ap_sdl_input_handle_event(struct ap_sdl_input* state, SDL_Event* e) {
	switch (e->type) {

	case SDL_MOUSEWHEEL:
	{
	} break;

	case SDL_MOUSEMOTION:
	{
		state->ms.x = e->motion.x;
		state->ms.y = e->motion.y;
		state->ms.xrel = e->motion.xrel;
		state->ms.yrel = e->motion.yrel;

	} break;

	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
	{
		//SDL_BUTTON_LEFT - 1  because SDL_BUTTON_LEFT is 1
		struct ap_sdl_ms_butt_state* ms_but_data = &state->ms.buttons[e->button.button - 1];

		if (e->button.state == SDL_PRESSED) {
			ms_but_data->pressed = 1;
		} else {
			ms_but_data->pressed = 0;
		}
		ms_but_data->repeat = 0;
	} break;

	case SDL_KEYUP:
	case SDL_KEYDOWN:
	{
		struct ap_sdl_KeyboardButtonState* kb = &state->kb.keys[e->key.keysym.scancode];
		if (e->key.state == SDL_PRESSED) {
			kb->repeat = e->key.repeat;
			kb->pressed = 1;
			//keyData.modifiers = fixedModifiers(e->key.keysym.mod);
		} else {
			kb->repeat = 0;
			kb->pressed = 0;
			//keyData.modifiers = fixedModifiers(e->key.keysym.mod);
		}
	} break;

	}
}


//// CUTE TEXTURE LOADING CODE

#pragma warning( push )
#pragma warning(disable : 4996) // disable fopen warning in cute_png..
#define CUTE_PNG_IMPLEMENTATION
#include "cute_png.h"
#pragma warning( pop )

SDL_Texture* ap_sdl_texture_load(const char* file_name, SDL_Renderer* renderer) {
	cp_image_t img = cp_load_png(file_name);
	if (!img.pix) {
		return NULL;
	}
	//cp_flip_image_horizontal(&img);

	Uint32 rmask, gmask, bmask, amask;
	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(img.pix, img.w, img.h, 32,img.w * 4, rmask, gmask, bmask, amask);
	if (surface == NULL) {
		/*fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		exit(1);*/
		return NULL;
	}


	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == NULL) {
		/*fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
		exit(1);*/
		SDL_FreeSurface(surface);
		return NULL;
	}

	SDL_FreeSurface(surface);
	return texture;
}




#endif //AP_SDL_IMPL




