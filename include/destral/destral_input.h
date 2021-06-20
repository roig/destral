#pragma once

#include <destral/destral_common.h>
#include <destral/destral_math.h>

// These are generally copied from the SDL2 Scancode Keys,
// which are in turn based on the USB standards:
// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf

namespace ds {

enum class key: i32 {
	None = 0,

	// Keyboard keys starting from 4 to 256
	A = 4,	
	B = 5,
	C = 6,
	D = 7,
	E = 8,
	F = 9,
	G = 10,
	H = 11,
	I = 12,
	J = 13,
	K = 14,
	L = 15,
	M = 16,
	N = 17,
	O = 18,
	P = 19,
	Q = 20,
	R = 21,
	S = 22,
	T = 23,
	U = 24,
	V = 25,
	W = 26,
	X = 27,
	Y = 28,
	Z = 29,
	One = 30,
	Two = 31,
	Three = 32,
	Four = 33,
	Five = 34,
	Six = 35,
	Seven = 36,
	Eight = 37,
	Nine = 38,
	Ten = 39,
	Enter = 40,
	Escape = 41,
	Backspace = 42,
	Tab = 43,
	Space = 44,
	Minus = 45,
	Equals = 46,
	LeftBracket = 47,
	RightBracket = 48,
	Backslash = 49,
	Semicolon = 51,
	Apostrophe = 52,
	Tilde = 53,
	Comma = 54,
	Period = 55,
	Slash = 56,
	Capslock = 57,
	F1 = 58,
	F2 = 59,
	F3 = 60,
	F4 = 61,
	F5 = 62,
	F6 = 63,
	F7 = 64,
	F8 = 65,
	F9 = 66,
	F10 = 67,
	F11 = 68,
	F12 = 69,
	PrintScreen = 70,
	ScrollLock = 71,
	Pause = 72,
	Insert = 73,
	Home = 74,
	PageUp = 75,
	Delete = 76,
	End = 77,
	PageDown = 78,
	Right = 79,
	Left = 80,
	Down = 81,
	Up = 82,
	NumLock = 83,
	Keypad1 = 89,
	Keypad2 = 90,
	Keypad3 = 91,
	Keypad4 = 92,
	Keypad5 = 93,
	Keypad6 = 94,
	Keypad7 = 95,
	Keypad8 = 96,
	Keypad9 = 97,
	KeypadDivide = 84,
	KeypadMultiply = 85,
	KeypadMinus = 86,
	KeypadPlus = 87,
	KeypadEnter = 88,
	KeypadPeroid = 99,
	KeypadEquals = 103,
	KeypadComma = 133,
	LeftControl = 224,
	LeftShift = 225,
	LeftAlt = 226,
	LeftOS = 227,
	RightControl = 228,
	RightShift = 229,
	RightAlt = 230,
	RightOS = 231,

	// Mouse buttons starting from 300
	MouseLeftButton = 300,
	MouseRightButton = 301,
	MouseMiddleButton = 302,
	MouseX1Button = 303,
	MouseX2Button = 304,
	MouseX = 310,
	MouseY = 311,
	//MouseScrollUp = 312,
	//MouseScrollDown = 313,
	//MouseWheelAxis = 314,
 

	// Gamepad Keys starting from 400
	Gamepad_LeftX = 400,				// Left  Thumbstick X axis movement
	Gamepad_LeftY= 401,					// Left  Thumbstick Y axis movement
	Gamepad_RightX= 402,				// Right Thumbstick X axis movement
	Gamepad_RightY= 403,				// Right Thumbstick Y axis movement
	Gamepad_LeftTriggerAxis= 404,
	Gamepad_RightTriggerAxis= 405,
	Gamepad_LeftThumbstick= 406,		
	Gamepad_RightThumbstick= 407,
	Gamepad_Special_Left= 408,
	Gamepad_Special_Home= 409,
	Gamepad_Special_Right= 410,
	Gamepad_FaceButton_Bottom= 411,
	Gamepad_FaceButton_Right= 412,
	Gamepad_FaceButton_Left= 413,
	Gamepad_FaceButton_Top= 414,
	Gamepad_LeftShoulder= 415,
	Gamepad_RightShoulder= 416,
	Gamepad_LeftTrigger= 417,
	Gamepad_RightTrigger= 418,
	Gamepad_DPad_Up= 419,
	Gamepad_DPad_Down= 420,
	Gamepad_DPad_Right= 421,
	Gamepad_DPad_Left= 422,

	// Virtual key codes used for input axis button press/release emulation
	Gamepad_Left_Up= 423,
	Gamepad_Left_Down= 424,
	Gamepad_Left_Right= 425,
	Gamepad_Left_Left= 426,
	Gamepad_Right_Up= 427,
	Gamepad_Right_Down= 428,
	Gamepad_Right_Right= 429,
	Gamepad_Right_Left= 430,

};
	// maximum number of gamepads the input can handle
	constexpr i32 input_max_gamepads = 8;

	/** Check if the key is released in this frame (only returns 1 the frame that state switchs from pressed to released) Example: SDLK_PLUS*/
	bool key_is_triggered(key k, i32 gamepad_id = 0);

	/** Check if the key is pressed*/
	bool key_is_pressed(key k, i32 gamepad_id = 0);

	/** Check if the key is released*/
	bool key_is_released(key k, i32 gamepad_id = 0);

	/** Returns the axis value [-1,1] for the axis Key in the gamepad  or 0 if it's not an axis Key */
	float key_axis_value(key k, i32 gamepad_id = 0);

	/** Returns true if the Key k is an axis */
	bool key_is_axis(key k);

	/** Returns true if the Key k is a gamepad key*/
	bool key_is_gamepad(key k);


	// TODO callbacks when a gamepad connects/disconnects
}
