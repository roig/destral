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
	------- VIDEO -------
	
	In order to open an opengl window use the function:
	- ap_sdl_CreateWindow


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

/*
	WINDOW CREATION
*/
struct ap_sdl_window_desc {
	uint32_t _start_canary;
	const char* window_title;
	uint32_t wsize_x; /* window width */
	uint32_t wsize_y; /* window height */
	uint32_t opengl_major;
	uint32_t opengl_minor;
	uint32_t is_gl_context_debug;
	uint32_t _end_canary;
};


/** This creates an opengl window*/
int ap_sdl_create_window(struct ap_sdl_window_desc* params, SDL_Window** out_window, SDL_GLContext* out_gl_ctx);

/** Creates a default opengl 3.3 window. */
int ap_sdl_create_window_default(const char* window_name, int width, int height, SDL_Window** out_window, SDL_GLContext* out_gl_ctx);

/** cleanup of the window and the opengl context */
void ap_sdl_destroy_window(SDL_Window* window, SDL_GLContext* gl_ctx);

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
	uint32_t _end_canary;
};

/**
	Runs an application loop that handles input and events and calls callback functions for the user
	The callbacks will be called with the user data passed by pointer in this function.
*/
int ap_sdl_app_run(struct ap_sdl_app_desc* desc, struct ap_sdl_window_desc* window_desc, void* user_data);

/** Returns the current delta time in seconds for this frame (must be called on frame callback)*/
float ap_sdl_app_dt();

/** Returns the current fps value for the application (must be called on frame callback)*/
float ap_sdl_app_fps();

/** Returns the window pointer of the application */
SDL_Window* ap_sdl_app_window();

/** Returns the input pointer structure of the application (window)*/
struct ap_sdl_input* ap_sdl_app_input();

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef AP_SDL_IMPL
#include <assert.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_log.h>


int ap_sdl_create_window(struct ap_sdl_window_desc* params, SDL_Window** out_window, SDL_GLContext* out_gl_ctx) {
	assert(params);
	assert((params->_start_canary == 0) && (params->_end_canary == 0));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 0;
    }


    // Debug context flag
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, params->is_gl_context_debug ? SDL_GL_CONTEXT_DEBUG_FLAG: 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params->opengl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params->opengl_minor);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    //Create window
    *out_window = SDL_CreateWindow(params->window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, params->wsize_x, params->wsize_y, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!out_window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 0;
    }

    *out_gl_ctx = SDL_GL_CreateContext(*out_window);
    if (!out_gl_ctx) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create gl context: %s", SDL_GetError());
        return 0;
    }

    return 1;
}

int ap_sdl_create_window_default(const char* window_name, int width, int height, SDL_Window** out_window, SDL_GLContext* out_gl_ctx) {
	struct ap_sdl_window_desc desc = { 0 };
	desc.window_title = window_name;
	desc.wsize_x = width;
	desc.wsize_y = height;
	desc.opengl_major = 3;
	desc.opengl_minor = 3;
	desc.is_gl_context_debug = 1;
	return ap_sdl_create_window(&desc, out_window, out_gl_ctx);
}


void ap_sdl_destroy_window(SDL_Window* window, SDL_GLContext* gl_ctx) {
	SDL_GL_DeleteContext(*gl_ctx);
	SDL_DestroyWindow(window);
}

/** Application globals */
SDL_Window* g_window;
SDL_GLContext g_glctx;
struct ap_sdl_input g_app_input = { 0 };
float g_tm_dtacc_s = 0;
float g_tm_fps = 0;

SDL_Window* ap_sdl_app_window() {
	return g_window;
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


int ap_sdl_app_run(struct ap_sdl_app_desc* desc, struct ap_sdl_window_desc* window_desc, void* user_data) {
	assert(desc);
	assert((desc->_start_canary == 0) && (desc->_end_canary == 0));
	assert(window_desc);

	if (!ap_sdl_create_window(window_desc, &g_window, &g_glctx)) {
		return EXIT_FAILURE;
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

		// 1 start frame (handle events)
		ap_sdl_input_begin_frame(&g_app_input);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			// handle input events
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

		SDL_GL_SwapWindow(g_window);
		// reset elapsed accumulator
		g_tm_dtacc_s = 0;
	}

	if (desc->cleanup_cb) desc->cleanup_cb(user_data);
	ap_sdl_destroy_window(g_window, &g_glctx);
	SDL_Quit();
	return EXIT_SUCCESS;
}



#include <SDL_keyboard.h>

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

//Magnum::Int fixedModifiers(Uint16 mod) {
//	Uint16 modifiers = mod;
//	if (modifiers & (Uint16)KeyboardModifier::Shift) modifiers |= (Uint16)KeyboardModifier::Shift;
//	if (modifiers & (Uint16)KeyboardModifier::Ctrl) modifiers |= (Uint16)KeyboardModifier::Ctrl;
//	if (modifiers & (Uint16)KeyboardModifier::Alt) modifiers |= (Uint16)KeyboardModifier::Alt;
//	if (modifiers & (Uint16)KeyboardModifier::Super) modifiers |= (Uint16)KeyboardModifier::Alt;
//	return (Magnum::Int)modifiers;
//}


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

#endif //AP_SDL_IMPL





//enum ap_sdl_keycode {
//	AP_SDL_Unknown = SDLK_UNKNOWN,     /**< Unknown key */
//	AP_SDL_LeftShift = SDLK_LSHIFT,
//	AP_SDL_RightShift = SDLK_RSHIFT,
//	AP_SDL_LeftCtrl = SDLK_LCTRL,
//	AP_SDL_RightCtrl = SDLK_RCTRL,
//	AP_SDL_LeftAlt = SDLK_LALT,
//	AP_SDL_RightAlt = SDLK_RALT,
//	AP_SDL_LeftSuper = SDLK_LGUI,		/* Left Super Key Windows */
//	AP_SDL_RightSuper = SDLK_RGUI,
//	AP_SDL_AltGr = SDLK_MODE,
//	AP_SDL_Enter = SDLK_RETURN,        /**< Enter */
//	AP_SDL_Esc = SDLK_ESCAPE,          /**< Escape */
//	AP_SDL_Up = SDLK_UP,               /**< Up arrow */
//	AP_SDL_Down = SDLK_DOWN,           /**< Down arrow */
//	AP_SDL_Left = SDLK_LEFT,           /**< Left arrow */
//	AP_SDL_Right = SDLK_RIGHT,         /**< Right arrow */
//	AP_SDL_Home = SDLK_HOME,           /**< Home */
//	AP_SDL_End = SDLK_END,             /**< End */
//	AP_SDL_PageUp = SDLK_PAGEUP,       /**< Page up */
//	AP_SDL_PageDown = SDLK_PAGEDOWN,   /**< Page down */
//	AP_SDL_Backspace = SDLK_BACKSPACE, /**< Backspace */
//	AP_SDL_Insert = SDLK_INSERT,       /**< Insert */
//	AP_SDL_Delete = SDLK_DELETE,       /**< Delete */
//	AP_SDL_F1 = SDLK_F1,               /**< F1 */
//	AP_SDL_F2 = SDLK_F2,               /**< F2 */
//	AP_SDL_F3 = SDLK_F3,               /**< F3 */
//	AP_SDL_F4 = SDLK_F4,               /**< F4 */
//	AP_SDL_F5 = SDLK_F5,               /**< F5 */
//	AP_SDL_F6 = SDLK_F6,               /**< F6 */
//	AP_SDL_F7 = SDLK_F7,               /**< F7 */
//	AP_SDL_F8 = SDLK_F8,               /**< F8 */
//	AP_SDL_F9 = SDLK_F9,               /**< F9 */
//	AP_SDL_F10 = SDLK_F10,             /**< F10 */
//	AP_SDL_F11 = SDLK_F11,             /**< F11 */
//	AP_SDL_F12 = SDLK_F12,             /**< F12 */
//	AP_SDL_Space = SDLK_SPACE,         /**< Space */
//	AP_SDL_Tab = SDLK_TAB,             /**< Tab */
//	AP_SDL_Quote = SDLK_QUOTE,
//	AP_SDL_Comma = SDLK_COMMA,         /**< Comma */
//	AP_SDL_Period = SDLK_PERIOD,       /**< Period */
//	AP_SDL_Minus = SDLK_MINUS,         /**< Minus */
//	AP_SDL_Plus = SDLK_PLUS,           /**< Plus */
//	AP_SDL_Slash = SDLK_SLASH,         /**< Slash */
//	AP_SDL_Percent = SDLK_PERCENT,     /**< Percent */
//	AP_SDL_Semicolon = SDLK_SEMICOLON, /**< Semicolon ; */
//	AP_SDL_Equal = SDLK_EQUALS,        /**< Equal */
//	AP_SDL_LeftBracket = SDLK_LEFTBRACKET,
//	AP_SDL_RightBracket = SDLK_RIGHTBRACKET,
//	AP_SDL_Backslash = SDLK_BACKSLASH,
//	AP_SDL_Backquote = SDLK_BACKQUOTE,
//	AP_SDL_Zero = SDLK_0,              /**< Zero */
//	AP_SDL_One = SDLK_1,               /**< One */
//	AP_SDL_Two = SDLK_2,               /**< Two */
//	AP_SDL_Three = SDLK_3,             /**< Three */
//	AP_SDL_Four = SDLK_4,              /**< Four */
//	AP_SDL_Five = SDLK_5,              /**< Five */
//	AP_SDL_Six = SDLK_6,               /**< Six */
//	AP_SDL_Seven = SDLK_7,             /**< Seven */
//	AP_SDL_Eight = SDLK_8,             /**< Eight */
//	AP_SDL_Nine = SDLK_9,              /**< Nine */
//	AP_SDL_A = SDLK_a,                 /**< Letter A */
//	AP_SDL_B = SDLK_b,                 /**< Letter B */
//	AP_SDL_C = SDLK_c,                 /**< Letter C */
//	AP_SDL_D = SDLK_d,                 /**< Letter D */
//	AP_SDL_E = SDLK_e,                 /**< Letter E */
//	AP_SDL_F = SDLK_f,                 /**< Letter F */
//	AP_SDL_G = SDLK_g,                 /**< Letter G */
//	AP_SDL_H = SDLK_h,                 /**< Letter H */
//	AP_SDL_I = SDLK_i,                 /**< Letter I */
//	AP_SDL_J = SDLK_j,                 /**< Letter J */
//	AP_SDL_K = SDLK_k,                 /**< Letter K */
//	AP_SDL_L = SDLK_l,                 /**< Letter L */
//	AP_SDL_M = SDLK_m,                 /**< Letter M */
//	AP_SDL_N = SDLK_n,                 /**< Letter N */
//	AP_SDL_O = SDLK_o,                 /**< Letter O */
//	AP_SDL_P = SDLK_p,                 /**< Letter P */
//	AP_SDL_Q = SDLK_q,                 /**< Letter Q */
//	AP_SDL_R = SDLK_r,                 /**< Letter R */
//	AP_SDL_S = SDLK_s,                 /**< Letter S */
//	AP_SDL_T = SDLK_t,                 /**< Letter T */
//	AP_SDL_U = SDLK_u,                 /**< Letter U */
//	AP_SDL_V = SDLK_v,                 /**< Letter V */
//	AP_SDL_W = SDLK_w,                 /**< Letter W */
//	AP_SDL_X = SDLK_x,                 /**< Letter X */
//	AP_SDL_Y = SDLK_y,                 /**< Letter Y */
//	AP_SDL_Z = SDLK_z,                 /**< Letter Z */
//	AP_SDL_NumZero = SDLK_KP_0,            /**< Numpad zero */
//	AP_SDL_NumOne = SDLK_KP_1,             /**< Numpad one */
//	AP_SDL_NumTwo = SDLK_KP_2,             /**< Numpad two */
//	AP_SDL_NumThree = SDLK_KP_3,           /**< Numpad three */
//	AP_SDL_NumFour = SDLK_KP_4,            /**< Numpad four */
//	AP_SDL_NumFive = SDLK_KP_5,            /**< Numpad five */
//	AP_SDL_NumSix = SDLK_KP_6,             /**< Numpad six */
//	AP_SDL_NumSeven = SDLK_KP_7,           /**< Numpad seven */
//	AP_SDL_NumEight = SDLK_KP_8,           /**< Numpad eight */
//	AP_SDL_NumNine = SDLK_KP_9,            /**< Numpad nine */
//	AP_SDL_NumDecimal = SDLK_KP_DECIMAL,   /**< Numpad decimal */
//	AP_SDL_NumDivide = SDLK_KP_DIVIDE,     /**< Numpad divide */
//	AP_SDL_NumMultiply = SDLK_KP_MULTIPLY, /**< Numpad multiply */
//	AP_SDL_NumSubtract = SDLK_KP_MINUS,    /**< Numpad subtract */
//	AP_SDL_NumAdd = SDLK_KP_PLUS,          /**< Numpad add */
//	AP_SDL_NumEnter = SDLK_KP_ENTER,       /**< Numpad enter */
//	AP_SDL_NumEqual = SDLK_KP_EQUALS       /**< Numpad equal */
//};
