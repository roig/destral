#include "destral_platform_backend.h"
#include "destral_input_backend.h"
#include <destral/destral_common.h>
#include <destral/destral_input.h>
#include <SDL.h>
#include <array>
#include <unordered_map>

// #include <SDL_syswm.h>

// Idea taken from: https://github.com/NoelFB/blah/blob/master/src/internal/platform_backend_sdl2.cpp

// SDL state and structures
namespace sdl {
	using namespace ds;

	void log_redirect(void* userdata, int category, SDL_LogPriority priority, const char* message) {
		if (priority <= SDL_LOG_PRIORITY_INFO) {
			DS_LOG(message);
		} else if (priority <= SDL_LOG_PRIORITY_WARN) {
			DS_WARNING(message);

		} else {
			DS_FATAL(message);
		}
	}
	

	// key_details: information about a key/button/etc... name if is mouse or axis etc..
	// key_state: information about the state of a key/button/etc.. 

	// map<key, key_details> keys_config; // map with the key to key_details information.
	// map<key, key_state> kb_ms_state; // map with the keyboard and mouse keys state information (no gamepads)

	// controller: contains the SDL_Joystick id and the gamepad_state;
	// map<key, key_state> gamepad_state; // map with the gamepad keys state information (no keyboard/mouse)


	struct key_state {
		bool pressed = false;
		bool repeat = false;
		float value = 0.0;
	};

	struct gamepad_state {
		// joy_id -1 is an invalid (not connected gamepad)
		ds::i32 joy_id = -1;
		std::unordered_map<ds::key, key_state> buttons_state;

		void clear() {
			joy_id = -1;
			buttons_state.clear();
		}
	};

	//FKeyDetails unreal engine
	struct key_details {
		enum key_flags {
			NoFlags = 0,
			GamepadKey = 1 << 0,
			MouseButton = 1 << 1,
			FloatAxis = 1 << 2,
		};

		ds::u32 flags = static_cast<ds::u32>(key_flags::NoFlags);
		bool is_gamepad() const { return flags & key_flags::GamepadKey; }
		bool is_mouse() const { return flags & key_flags::MouseButton; }
		bool is_axis_float() const { return flags & key_flags::FloatAxis; }

		// TODO key text.. etc..
	};

	struct sdl_state {
		sdl_state() {
			gamepads.fill({});
		}

		SDL_Window* window = nullptr;

		// details
		std::unordered_map<ds::key, key_details> keys;

		// keyboard and mouse keys state
		std::unordered_map<ds::key, key_state> kb_ms_state;

		// This defines a relation with the gamepad index and the value (SDL_JoystickID)
		std::array<gamepad_state, ds::input_max_gamepads> gamepads;

		void add_key(ds::key k, const key_details& d) {
			keys[k] = d;
			if (d.is_gamepad()) {
				// add the default state in all the gamepads
				for (auto& g : gamepads) {
					g.buttons_state[k] = {};
				}
			} else {
				// add the default state in the kb_ms state
				kb_ms_state[k] = {};
			}
		}

		// Finds a free array index and returns it. If no free gamepad index found will return -1
		ds::i32 find_free_gamepad_index() {
			ds::i32 free_index = -1;
			for (auto i = 0; i < gamepads.size(); i++) {
				if (gamepads[i].joy_id == -1) {
					free_index = i;
					break;
				}
			}
			return free_index;
		}

		ds::i32 find_gamepad_index_by_joy_id(ds::i32 joy_id) {
			ds::i32 index = -1;
			for (auto i = 0; i < gamepads.size(); i++) {
				if (gamepads[i].joy_id == joy_id) {
					index = i;
					break;
				}
			}
			return index;
		}
	};

	static sdl_state g_sdl;

	void init_keys() {

		g_sdl.add_key(key::A, {});
		g_sdl.add_key(key::B, {});
		g_sdl.add_key(key::C, {});
		g_sdl.add_key(key::D, {});
		g_sdl.add_key(key::E, {});
		g_sdl.add_key(key::F, {});
		g_sdl.add_key(key::G, {});
		g_sdl.add_key(key::H, {});
		g_sdl.add_key(key::I, {});
		g_sdl.add_key(key::J, {});
		g_sdl.add_key(key::K, {});
		g_sdl.add_key(key::L, {});
		g_sdl.add_key(key::M, {});
		g_sdl.add_key(key::N, {});
		g_sdl.add_key(key::O, {});
		g_sdl.add_key(key::P, {});
		g_sdl.add_key(key::Q, {});
		g_sdl.add_key(key::R, {});
		g_sdl.add_key(key::S, {});
		g_sdl.add_key(key::T, {});
		g_sdl.add_key(key::U, {});
		g_sdl.add_key(key::V, {});
		g_sdl.add_key(key::W, {});
		g_sdl.add_key(key::X, {});
		g_sdl.add_key(key::Y, {});
		g_sdl.add_key(key::Z, {});
		g_sdl.add_key(key::One, {});
		g_sdl.add_key(key::Two, {});
		g_sdl.add_key(key::Three, {});
		g_sdl.add_key(key::Four, {});
		g_sdl.add_key(key::Five, {});
		g_sdl.add_key(key::Six, {});
		g_sdl.add_key(key::Seven, {});
		g_sdl.add_key(key::Eight, {});
		g_sdl.add_key(key::Nine, {});
		g_sdl.add_key(key::Ten, {});
		g_sdl.add_key(key::Enter, {});
		g_sdl.add_key(key::Escape, {});
		g_sdl.add_key(key::Backspace, {});
		g_sdl.add_key(key::Tab, {});
		g_sdl.add_key(key::Space, {});
		g_sdl.add_key(key::Minus, {});
		g_sdl.add_key(key::Equals, {});
		g_sdl.add_key(key::LeftBracket, {});
		g_sdl.add_key(key::RightBracket, {});
		g_sdl.add_key(key::Backslash, {});
		g_sdl.add_key(key::Semicolon, {});
		g_sdl.add_key(key::Apostrophe, {});
		g_sdl.add_key(key::Tilde, {});
		g_sdl.add_key(key::Comma, {});
		g_sdl.add_key(key::Period, {});
		g_sdl.add_key(key::Slash, {});
		g_sdl.add_key(key::Capslock, {});
		g_sdl.add_key(key::F1, {});
		g_sdl.add_key(key::F2, {});
		g_sdl.add_key(key::F3, {});
		g_sdl.add_key(key::F4, {});
		g_sdl.add_key(key::F5, {});
		g_sdl.add_key(key::F6, {});
		g_sdl.add_key(key::F7, {});
		g_sdl.add_key(key::F8, {});
		g_sdl.add_key(key::F9, {});
		g_sdl.add_key(key::F10, {});
		g_sdl.add_key(key::F11, {});
		g_sdl.add_key(key::F12, {});
		g_sdl.add_key(key::PrintScreen, {});
		g_sdl.add_key(key::ScrollLock, {});
		g_sdl.add_key(key::Pause, {});
		g_sdl.add_key(key::Insert, {});
		g_sdl.add_key(key::Home, {});
		g_sdl.add_key(key::PageUp, {});
		g_sdl.add_key(key::Delete, {});
		g_sdl.add_key(key::End, {});
		g_sdl.add_key(key::PageDown, {});
		g_sdl.add_key(key::Right, {});
		g_sdl.add_key(key::Left, {});
		g_sdl.add_key(key::Down, {});
		g_sdl.add_key(key::Up, {});
		g_sdl.add_key(key::NumLock, {});
		g_sdl.add_key(key::Keypad1, {});
		g_sdl.add_key(key::Keypad2, {});
		g_sdl.add_key(key::Keypad3, {});
		g_sdl.add_key(key::Keypad4, {});
		g_sdl.add_key(key::Keypad5, {});
		g_sdl.add_key(key::Keypad6, {});
		g_sdl.add_key(key::Keypad7, {});
		g_sdl.add_key(key::Keypad8, {});
		g_sdl.add_key(key::Keypad9, {});
		g_sdl.add_key(key::KeypadDivide, {});
		g_sdl.add_key(key::KeypadMultiply, {});
		g_sdl.add_key(key::KeypadMinus, {});
		g_sdl.add_key(key::KeypadPlus, {});
		g_sdl.add_key(key::KeypadEnter, {});
		g_sdl.add_key(key::KeypadPeroid, {});
		g_sdl.add_key(key::KeypadEquals, {});
		g_sdl.add_key(key::KeypadComma, {});
		g_sdl.add_key(key::LeftControl, {});
		g_sdl.add_key(key::LeftShift, {});
		g_sdl.add_key(key::LeftAlt, {});
		g_sdl.add_key(key::LeftOS, {});
		g_sdl.add_key(key::RightControl, {});
		g_sdl.add_key(key::RightShift, {});
		g_sdl.add_key(key::RightAlt, {});
		g_sdl.add_key(key::RightOS, {});
		g_sdl.add_key(key::MouseLeftButton, { .flags = key_details::MouseButton });
		g_sdl.add_key(key::MouseRightButton, { .flags = key_details::MouseButton });
		g_sdl.add_key(key::MouseMiddleButton, { .flags = key_details::MouseButton });
		g_sdl.add_key(key::MouseX1Button, { .flags = key_details::MouseButton });
		g_sdl.add_key(key::MouseX2Button, { .flags = key_details::MouseButton });
		g_sdl.add_key(key::MouseX, { .flags = key_details::MouseButton | key_details::FloatAxis });
		g_sdl.add_key(key::MouseY, { .flags = key_details::MouseButton | key_details::FloatAxis });
		//g_sdl.add_key( // MouseScrollUp			,{.flags = key_details::MouseButton | key_details::FloatAxis});
		//g_sdl.add_key( // MouseScrollDown		,{.flags = key_details::MouseButton | key_details::FloatAxis});
		//g_sdl.add_key( // MouseWheelAxis			,{.flags = key_details::MouseButton | key_details::FloatAxis});
		g_sdl.add_key(key::Gamepad_LeftX, { .flags = key_details::GamepadKey | key_details::FloatAxis });
		g_sdl.add_key(key::Gamepad_LeftY, { .flags = key_details::GamepadKey | key_details::FloatAxis });
		g_sdl.add_key(key::Gamepad_RightX, { .flags = key_details::GamepadKey | key_details::FloatAxis });
		g_sdl.add_key(key::Gamepad_RightY, { .flags = key_details::GamepadKey | key_details::FloatAxis });
		g_sdl.add_key(key::Gamepad_LeftTriggerAxis, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_RightTriggerAxis, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_LeftThumbstick, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_RightThumbstick, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_Special_Left, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_Special_Home, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_Special_Right, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_FaceButton_Bottom, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_FaceButton_Right, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_FaceButton_Left, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_FaceButton_Top, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_LeftShoulder, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_RightShoulder, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_LeftTrigger, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_RightTrigger, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_DPad_Up, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_DPad_Down, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_DPad_Right, { .flags = key_details::GamepadKey });
		g_sdl.add_key(key::Gamepad_DPad_Left, { .flags = key_details::GamepadKey });
		
		// virtual key codes used for input axis button press/release emulation
		g_sdl.add_key( key::Gamepad_Left_Up				,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Left_Down			,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Left_Right			,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Left_Left			,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Right_Up			,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Right_Down			,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Right_Right			,{.flags = key_details::GamepadKey });
		g_sdl.add_key( key::Gamepad_Right_Left			,{.flags = key_details::GamepadKey });


	}

}

// destral_input.h public functions implementation based on SDL state and structures
namespace ds {
	static bool s_is_valid_gamepad_id(i32 gamepad_id) {
		if (!(gamepad_id >= 0 && gamepad_id < input_max_gamepads)) {
			DS_WARNING(std::format("Gamepad id {} out of bounds!", gamepad_id));
			return false;
		}
		return true;
	}

	bool key_is_axis(key k) {
		auto it_key = sdl::g_sdl.keys.find(k);
		if (it_key != sdl::g_sdl.keys.end()) {
			return it_key->second.is_axis_float();
		}
		return false;
	}

	bool key_is_gamepad(key k) {
		auto it_key = sdl::g_sdl.keys.find(k);
		if (it_key != sdl::g_sdl.keys.end()) {
			return it_key->second.is_gamepad();
		}
		return false;
	}

	float key_axis_value(key key, i32 gamepad_id) {
		if (!s_is_valid_gamepad_id(gamepad_id)) {
			return 0;
		}

		if (key == key::None) return 0;

		auto it_key = sdl::g_sdl.keys.find(key);
		if (it_key != sdl::g_sdl.keys.end()) {
			if (it_key->second.is_axis_float() && it_key->second.is_gamepad()) {
				return sdl::g_sdl.gamepads[gamepad_id].buttons_state[key].value;
			}
		}
		return 0;
	}

	bool key_is_triggered(key key, i32 gamepad_id) {
		if (!s_is_valid_gamepad_id(gamepad_id)) {
			return false;
		}

		if (key == key::None) return false;

		auto it_key = sdl::g_sdl.keys.find(key);
		if (it_key != sdl::g_sdl.keys.end()) {
			if (it_key->second.is_gamepad()) {
				return sdl::g_sdl.gamepads[gamepad_id].buttons_state[key].pressed
					&& !sdl::g_sdl.gamepads[gamepad_id].buttons_state[key].repeat;
			} else {
				return sdl::g_sdl.kb_ms_state[key].pressed && !sdl::g_sdl.kb_ms_state[key].repeat;
			}
				
				
		}
		return false;
	}
	bool key_is_pressed(key key, i32 gamepad_id) {
			
		if (!s_is_valid_gamepad_id(gamepad_id)) {
			return false;
		}

		if (key == key::None) return false;
		auto it_key = sdl::g_sdl.keys.find(key);
		if (it_key != sdl::g_sdl.keys.end()) {
			if (it_key->second.is_gamepad()) {
				return sdl::g_sdl.gamepads[gamepad_id].buttons_state[key].pressed;
			} else {
				return sdl::g_sdl.kb_ms_state[key].pressed;
			}
		}
		return false;
	}

	bool key_is_released(key key, i32 gamepad_id) {
		if (!s_is_valid_gamepad_id(gamepad_id)) {
			return false;
		}

		if (key == key::None) return false;
		auto it_key = sdl::g_sdl.keys.find(key);
		if (it_key != sdl::g_sdl.keys.end()) {
			if (it_key->second.is_gamepad()) {
				return !sdl::g_sdl.gamepads[gamepad_id].buttons_state[key].pressed;
			} else {
				return !sdl::g_sdl.kb_ms_state[key].pressed;
			}
		}
		return false;
	}
} // ds::in namespace

// platform_backend implementation for SDL
namespace ds::platform_backend {
	// Called to initialize the platform backend
	void init() {

		// TODO: Redirect the sdl logs to our own function
		// control this via some kind of config flag
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
		SDL_LogSetOutputFunction(sdl::log_redirect, nullptr);

		// TODO: Print SDL version to the log
		SDL_version version;
		SDL_GetVersion(&version);
		DS_LOG(std::format("SDL v.{}.{}.{}", version.major, version.minor, version.patch));


		// initialize SDL
		dsverifym(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) == 0, "Failed to initialize SDL2");

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
		sdl::g_sdl.window = SDL_CreateWindow(app_get_config().name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			app_get_config().width, app_get_config().height, flags);
		dsverifym(sdl::g_sdl.window, "Failed to create a Window");

		// set window properties
		SDL_SetWindowResizable(sdl::g_sdl.window, SDL_TRUE);
		SDL_SetWindowMinimumSize(sdl::g_sdl.window, 256, 256);

		sdl::init_keys();
	}

	// Called during shutdown
	void deinit() {
		if (sdl::g_sdl.window != nullptr) {
			SDL_DestroyWindow(sdl::g_sdl.window);
		}
		sdl::g_sdl.window = nullptr;
		//displayed = false;

	/*	if (basePath != nullptr)
			SDL_free(basePath);

		if (userPath != nullptr)
			SDL_free(userPath);*/

		SDL_Quit();
	}

	// Called every frame
	void poll_events() {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
			{
				app_exit_request();
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
				key mouse_key = key::None;
				switch (e.button.button) {
				case SDL_BUTTON_LEFT: mouse_key = key::MouseLeftButton; break;
				case SDL_BUTTON_MIDDLE: mouse_key = key::MouseMiddleButton; break;
				case SDL_BUTTON_RIGHT: mouse_key = key::MouseRightButton; break;
				case SDL_BUTTON_X1: mouse_key = key::MouseX1Button; break;
				case SDL_BUTTON_X2: mouse_key = key::MouseX2Button; break;
				}
				input_backend::on_key_change(e.button.state == SDL_PRESSED, mouse_key, 0);
			} break;

			case SDL_KEYUP:
			case SDL_KEYDOWN:
			{
				if (e.key.repeat == 0) {
					// The key enum identifier in ds::Key is the same of SDL scancode, so we can cast transparently here
					input_backend::on_key_change(e.key.state == SDL_PRESSED, (key)e.key.keysym.scancode, 0);
				}
			} break;

			// Gamepad Controller
			case SDL_CONTROLLERDEVICEADDED:
			{
				// This number should not be saved, it's like an index for unopened joysticks, not the joystick id.
				auto device_id = e.cdevice.which;
				auto controller = SDL_GameControllerOpen(device_id);
				auto name = SDL_GameControllerName(controller);
				// this is the joystick id for that game controller
				auto joy_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
				//auto vendor = SDL_GameControllerGetVendor(ptr);
				//auto product = SDL_GameControllerGetProduct(ptr);
				//auto version = SDL_GameControllerGetProductVersion(ptr);



				// Free index found, add the controller joy id to that index
				i32 free_index = sdl::g_sdl.find_free_gamepad_index();
				if (free_index >= 0) {
					sdl::g_sdl.gamepads[free_index].joy_id = joy_id;
					DS_LOG(std::format("controller added: {} [id = {}, internal_joy_id = {}]", name, free_index, joy_id));
				}


			}break;

			case SDL_CONTROLLERDEVICEREMOVED:
			{
				// This is the joystick index
				const i32 joy_id = e.cdevice.which;

				// Find the joystick index
				i32 gamepad_index = sdl::g_sdl.find_gamepad_index_by_joy_id(joy_id);
				
				// gamepad_index found, remove the controller joy id to that index
				if (gamepad_index >= 0) {
					auto controller = SDL_GameControllerFromInstanceID(sdl::g_sdl.gamepads[gamepad_index].joy_id);
					auto name = SDL_GameControllerName(controller);
					DS_LOG(std::format("controller removed: {} [id = {}, internal_joy_id = {}]", name, gamepad_index, joy_id));
					sdl::g_sdl.gamepads[gamepad_index].clear();

				}
			}break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				const i32 gamepad_index = sdl::g_sdl.find_gamepad_index_by_joy_id(e.cbutton.which);

				if (gamepad_index == -1) {
					break;
				}

				key key = key::None;
				switch (e.cbutton.button) {
				case SDL_CONTROLLER_BUTTON_A:	key = key::Gamepad_FaceButton_Bottom; break;
				case SDL_CONTROLLER_BUTTON_B:	key = key::Gamepad_FaceButton_Right; break;
				case SDL_CONTROLLER_BUTTON_X:	key = key::Gamepad_FaceButton_Left; break;
				case SDL_CONTROLLER_BUTTON_Y:	key = key::Gamepad_FaceButton_Top; break;
				case SDL_CONTROLLER_BUTTON_BACK:	key = key::Gamepad_Special_Left; break;
				case SDL_CONTROLLER_BUTTON_GUIDE:	key = key::Gamepad_Special_Home; break;
				case SDL_CONTROLLER_BUTTON_START:	key = key::Gamepad_Special_Right; break;
				case SDL_CONTROLLER_BUTTON_LEFTSTICK:	key = key::Gamepad_LeftThumbstick; break;
				case SDL_CONTROLLER_BUTTON_RIGHTSTICK:	key = key::Gamepad_RightThumbstick; break;
				case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:	key = key::Gamepad_LeftShoulder; break;
				case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:	key = key::Gamepad_RightShoulder; break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:	key = key::Gamepad_DPad_Up; break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:	key = key::Gamepad_DPad_Down; break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:	key = key::Gamepad_DPad_Left; break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:	key = key::Gamepad_DPad_Right; break;
					/*case SDL_CONTROLLER_BUTTON_MISC1		:	key = Gamepad_FaceButton_Bottom; break;
					case SDL_CONTROLLER_BUTTON_PADDLE1		:	key = Gamepad_FaceButton_Bottom; break;
					case SDL_CONTROLLER_BUTTON_PADDLE2		:	key = Gamepad_FaceButton_Bottom; break;
					case SDL_CONTROLLER_BUTTON_PADDLE3		:	key = Gamepad_FaceButton_Bottom; break;
					case SDL_CONTROLLER_BUTTON_PADDLE4		:	key = Gamepad_FaceButton_Bottom; break;
					case SDL_CONTROLLER_BUTTON_TOUCHPAD		:	key = Gamepad_FaceButton_Bottom; break;*/
				}


				if (key != key::None) {
					input_backend::on_key_change(e.button.state == SDL_PRESSED, key, gamepad_index);
				}

			}break;

			case SDL_CONTROLLERAXISMOTION:
			{
				// Find the joystick index
				const i32 gamepad_index = sdl::g_sdl.find_gamepad_index_by_joy_id(e.cbutton.which);

				if (gamepad_index == -1) {
					break;
				}

				key key = key::None;
				switch (e.caxis.axis) {
				case SDL_CONTROLLER_AXIS_LEFTX: key = key::Gamepad_LeftX; break;
				case SDL_CONTROLLER_AXIS_LEFTY: key = key::Gamepad_LeftY; break;
				case SDL_CONTROLLER_AXIS_RIGHTX: key = key::Gamepad_RightX; break;
				case SDL_CONTROLLER_AXIS_RIGHTY: key = key::Gamepad_RightY; break;
				case SDL_CONTROLLER_AXIS_TRIGGERLEFT: key = key::Gamepad_LeftTriggerAxis; break;
				case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: key = key::Gamepad_RightTriggerAxis; break;
				default: break;
				}

				if (key == key::None) {
					break;
				}

				const float value = e.caxis.value >= 0 ? e.caxis.value / 32767.0f : e.caxis.value / 32768.0f;
				//DS_LOG(std::format("axis {} value: {}", SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)e.caxis.axis), value));
				input_backend::on_gamepad_axis_change(value, key, gamepad_index);
			}break;





			}
		}
	}

	// Called to present the window contents
	void swap_buffers() {
		SDL_GL_SwapWindow(sdl::g_sdl.window);
	}

	// Sets the Window Fullscreen if enabled is not 0
	void set_fullscreen(bool enabled) {
		if (enabled)
			SDL_SetWindowFullscreen(sdl::g_sdl.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		else
			SDL_SetWindowFullscreen(sdl::g_sdl.window, 0);
	}

	void* gl_get_func(const char* name) {
		return SDL_GL_GetProcAddress(name);
	}

	void* gl_context_create() {
		void* ctx = nullptr;
		ctx = SDL_GL_CreateContext(sdl::g_sdl.window);
		dsverifym(ctx, std::format("SDL_GL_CreateContext failed: {}", SDL_GetError()));
		return ctx;
	}

	void gl_context_make_current(void* context) {
		SDL_GL_MakeCurrent(sdl::g_sdl.window, context);
	}

	void gl_context_destroy(void* context) {
		SDL_GL_DeleteContext(context);
	}

	void get_drawable_size(i32* width, i32* height) {
		dscheck(width);
		dscheck(height);
		// IMPORTANT This is only GL
		int x, y;
		SDL_GL_GetDrawableSize(sdl::g_sdl.window, &x, &y);
		*width = x; *height = y;
		// If not GL use: SDL_GetWindowSize(window, width, height);
	}


}

// input_backend implementation for SDL
namespace ds::input_backend {
	void on_input_begin_frame() {
		// update repeat state of keyboard and mouse keys
		for (auto& k : sdl::g_sdl.kb_ms_state) {
			if (k.second.pressed) {
				k.second.repeat = true;
			}
		}

		// update repeat state of connected gamepad buttons
		for (auto& b : sdl::g_sdl.gamepads) {
			if (b.joy_id != -1) {
				for (auto& button_state : b.buttons_state) {
					if (button_state.second.pressed) {
						button_state.second.repeat = true;
					}
				}
			}
		}
	}

	void on_mouse_motion(ivec2 mouse_pos, ivec2 mouse_rel) {
		sdl::g_sdl.kb_ms_state[key::MouseX].value = static_cast<float>(mouse_pos.x);
		sdl::g_sdl.kb_ms_state[key::MouseY].value = static_cast<float>(mouse_pos.y);
	}

	void on_key_change(bool is_pressed, key key, i32 controller_idx) {
		dscheck(controller_idx >= 0 && controller_idx < input_max_gamepads);
		auto it_key = sdl::g_sdl.keys.find(key);
		if (it_key != sdl::g_sdl.keys.end()) {
			if (it_key->second.is_gamepad()) {
				if (sdl::g_sdl.gamepads[controller_idx].buttons_state[key].pressed != is_pressed) {
					sdl::g_sdl.gamepads[controller_idx].buttons_state[key].pressed = is_pressed;
					sdl::g_sdl.gamepads[controller_idx].buttons_state[key].repeat = false;
				}
			} else {
				if (sdl::g_sdl.kb_ms_state[key].pressed != is_pressed) {
					sdl::g_sdl.kb_ms_state[key].pressed = is_pressed;
					sdl::g_sdl.kb_ms_state[key].repeat = false;
				}
			}
		}
	}

	void on_gamepad_axis_change(float value, key axis, i32 controller_idx) {
		dscheck(controller_idx >= 0 && controller_idx < input_max_gamepads);
		auto it_key = sdl::g_sdl.keys.find(axis);
		if (it_key != sdl::g_sdl.keys.end()) {
			if (it_key->second.is_axis_float() && it_key->second.is_gamepad()) {
				sdl::g_sdl.gamepads[controller_idx].buttons_state[axis].value = value;

				// threshold for axis value to fire the virtual keys
				// Maybe this should be configurable!
				const float virtual_key_threshold = 0.3f;
				// Handle virtual keys based on axis
				switch (axis) {
				case key::Gamepad_LeftX:
					if (value >= virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Left_Right, controller_idx);
					} else if (value <= -virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Left_Left, controller_idx);
					} else {
						on_key_change(false, key::Gamepad_Left_Right, controller_idx);
						on_key_change(false, key::Gamepad_Left_Left, controller_idx);
					}
					break;
				case key::Gamepad_LeftY:
					if (value >= virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Left_Down, controller_idx);
					} else if (value <= -virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Left_Up, controller_idx);
					} else {
						on_key_change(false, key::Gamepad_Left_Down, controller_idx);
						on_key_change(false, key::Gamepad_Left_Up, controller_idx);
					}
					break;
				case key::Gamepad_RightX:
					if (value >= virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Right_Right, controller_idx);
					} else if (value <= -virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Right_Left, controller_idx);
					} else {
						on_key_change(false, key::Gamepad_Right_Right, controller_idx);
						on_key_change(false, key::Gamepad_Right_Left, controller_idx);
					}
					break; 			
				case key::Gamepad_RightY:
					if (value >= virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Right_Down, controller_idx);
					} else if (value <= -virtual_key_threshold) {
						on_key_change(true, key::Gamepad_Right_Up, controller_idx);
					} else {
						on_key_change(false, key::Gamepad_Right_Down, controller_idx);
						on_key_change(false, key::Gamepad_Right_Up, controller_idx);
					}
					break;
				default: break;
				}

			}
		}



	}
}