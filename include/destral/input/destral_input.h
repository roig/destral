#pragma once

#include <destral/core/destral_common.h>
#include <destral/math/destral_math.h>


// These are generally copied from the SDL2 Scancode Keys,
// which are in turn based on the USB standards:
// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf


enum class Key: ds::i32 {

};


#define DESTRAL_KEY_DEFINITIONS \
DESTRAL_DEFINE_KEY(None, 0) \
DESTRAL_DEFINE_KEY(AnyKey, 1) \
DESTRAL_DEFINE_KEY(Unknown, 0) \
DESTRAL_DEFINE_KEY(A, 4) \
DESTRAL_DEFINE_KEY(B, 5) \
DESTRAL_DEFINE_KEY(C, 6) \
DESTRAL_DEFINE_KEY(D, 7) \
DESTRAL_DEFINE_KEY(E, 8) \
DESTRAL_DEFINE_KEY(F, 9) \
DESTRAL_DEFINE_KEY(G, 10) \
DESTRAL_DEFINE_KEY(H, 11) \
DESTRAL_DEFINE_KEY(I, 12) \
DESTRAL_DEFINE_KEY(J, 13) \
DESTRAL_DEFINE_KEY(K, 14) \
DESTRAL_DEFINE_KEY(L, 15) \
DESTRAL_DEFINE_KEY(M, 16) \
DESTRAL_DEFINE_KEY(N, 17) \
DESTRAL_DEFINE_KEY(O, 18) \
DESTRAL_DEFINE_KEY(P, 19) \
DESTRAL_DEFINE_KEY(Q, 20) \
DESTRAL_DEFINE_KEY(R, 21) \
DESTRAL_DEFINE_KEY(S, 22) \
DESTRAL_DEFINE_KEY(T, 23) \
DESTRAL_DEFINE_KEY(U, 24) \
DESTRAL_DEFINE_KEY(V, 25) \
DESTRAL_DEFINE_KEY(W, 26) \
DESTRAL_DEFINE_KEY(X, 27) \
DESTRAL_DEFINE_KEY(Y, 28) \
DESTRAL_DEFINE_KEY(Z, 29) \
DESTRAL_DEFINE_KEY(D1, 30) \
DESTRAL_DEFINE_KEY(D2, 31) \
DESTRAL_DEFINE_KEY(D3, 32) \
DESTRAL_DEFINE_KEY(D4, 33) \
DESTRAL_DEFINE_KEY(D5, 34) \
DESTRAL_DEFINE_KEY(D6, 35) \
DESTRAL_DEFINE_KEY(D7, 36) \
DESTRAL_DEFINE_KEY(D8, 37) \
DESTRAL_DEFINE_KEY(D9, 38) \
DESTRAL_DEFINE_KEY(D0, 39) \
DESTRAL_DEFINE_KEY(Enter, 40) \
DESTRAL_DEFINE_KEY(Escape, 41) \
DESTRAL_DEFINE_KEY(Backspace, 42) \
DESTRAL_DEFINE_KEY(Tab, 43) \
DESTRAL_DEFINE_KEY(Space, 44) \
DESTRAL_DEFINE_KEY(Minus, 45) \
DESTRAL_DEFINE_KEY(Equals, 46) \
DESTRAL_DEFINE_KEY(LeftBracket, 47) \
DESTRAL_DEFINE_KEY(RightBracket, 48) \
DESTRAL_DEFINE_KEY(Backslash, 49) \
DESTRAL_DEFINE_KEY(Semicolon, 51) \
DESTRAL_DEFINE_KEY(Apostrophe, 52) \
DESTRAL_DEFINE_KEY(Tilde, 53) \
DESTRAL_DEFINE_KEY(Comma, 54) \
DESTRAL_DEFINE_KEY(Period, 55) \
DESTRAL_DEFINE_KEY(Slash, 56) \
DESTRAL_DEFINE_KEY(Capslock, 57) \
DESTRAL_DEFINE_KEY(F1, 58) \
DESTRAL_DEFINE_KEY(F2, 59) \
DESTRAL_DEFINE_KEY(F3, 60) \
DESTRAL_DEFINE_KEY(F4, 61) \
DESTRAL_DEFINE_KEY(F5, 62) \
DESTRAL_DEFINE_KEY(F6, 63) \
DESTRAL_DEFINE_KEY(F7, 64) \
DESTRAL_DEFINE_KEY(F8, 65) \
DESTRAL_DEFINE_KEY(F9, 66) \
DESTRAL_DEFINE_KEY(F10, 67) \
DESTRAL_DEFINE_KEY(F11, 68) \
DESTRAL_DEFINE_KEY(F12, 69) \
DESTRAL_DEFINE_KEY(F13, 104) \
DESTRAL_DEFINE_KEY(F14, 105) \
DESTRAL_DEFINE_KEY(F15, 106) \
DESTRAL_DEFINE_KEY(F16, 107) \
DESTRAL_DEFINE_KEY(F17, 108) \
DESTRAL_DEFINE_KEY(F18, 109) \
DESTRAL_DEFINE_KEY(F19, 110) \
DESTRAL_DEFINE_KEY(F20, 111) \
DESTRAL_DEFINE_KEY(F21, 112) \
DESTRAL_DEFINE_KEY(F22, 113) \
DESTRAL_DEFINE_KEY(F23, 114) \
DESTRAL_DEFINE_KEY(F24, 115) \
DESTRAL_DEFINE_KEY(PrintScreen, 70) \
DESTRAL_DEFINE_KEY(ScrollLock, 71) \
DESTRAL_DEFINE_KEY(Pause, 72) \
DESTRAL_DEFINE_KEY(Insert, 73) \
DESTRAL_DEFINE_KEY(Home, 74) \
DESTRAL_DEFINE_KEY(PageUp, 75) \
DESTRAL_DEFINE_KEY(Delete, 76) \
DESTRAL_DEFINE_KEY(End, 77) \
DESTRAL_DEFINE_KEY(PageDown, 78) \
DESTRAL_DEFINE_KEY(Right, 79) \
DESTRAL_DEFINE_KEY(Left, 80) \
DESTRAL_DEFINE_KEY(Down, 81) \
DESTRAL_DEFINE_KEY(Up, 82) \
DESTRAL_DEFINE_KEY(Numlock, 83) \
DESTRAL_DEFINE_KEY(Application, 101) \
DESTRAL_DEFINE_KEY(Execute, 116) \
DESTRAL_DEFINE_KEY(Help, 117) \
DESTRAL_DEFINE_KEY(Menu, 118) \
DESTRAL_DEFINE_KEY(Select, 119) \
DESTRAL_DEFINE_KEY(Stop, 120) \
DESTRAL_DEFINE_KEY(Redo, 121) \
DESTRAL_DEFINE_KEY(Undo, 122) \
DESTRAL_DEFINE_KEY(Cut, 123) \
DESTRAL_DEFINE_KEY(Copy, 124) \
DESTRAL_DEFINE_KEY(Paste, 125) \
DESTRAL_DEFINE_KEY(Find, 126) \
DESTRAL_DEFINE_KEY(Mute, 127) \
DESTRAL_DEFINE_KEY(VolumeUp, 128) \
DESTRAL_DEFINE_KEY(VolumeDown, 129) \
DESTRAL_DEFINE_KEY(AltErase, 153) \
DESTRAL_DEFINE_KEY(SysReq, 154) \
DESTRAL_DEFINE_KEY(Cancel, 155) \
DESTRAL_DEFINE_KEY(Clear, 156) \
DESTRAL_DEFINE_KEY(Prior, 157) \
DESTRAL_DEFINE_KEY(Enter2, 158) \
DESTRAL_DEFINE_KEY(Separator, 159) \
DESTRAL_DEFINE_KEY(Out, 160) \
DESTRAL_DEFINE_KEY(Oper, 161) \
DESTRAL_DEFINE_KEY(ClearAgain, 162) \
DESTRAL_DEFINE_KEY(KeypadA, 188) \
DESTRAL_DEFINE_KEY(KeypadB, 189) \
DESTRAL_DEFINE_KEY(KeypadC, 190) \
DESTRAL_DEFINE_KEY(KeypadD, 191) \
DESTRAL_DEFINE_KEY(KeypadE, 192) \
DESTRAL_DEFINE_KEY(KeypadF, 193) \
DESTRAL_DEFINE_KEY(Keypad0, 98) \
DESTRAL_DEFINE_KEY(Keypad00, 176) \
DESTRAL_DEFINE_KEY(Keypad000, 177) \
DESTRAL_DEFINE_KEY(Keypad1, 89) \
DESTRAL_DEFINE_KEY(Keypad2, 90) \
DESTRAL_DEFINE_KEY(Keypad3, 91) \
DESTRAL_DEFINE_KEY(Keypad4, 92) \
DESTRAL_DEFINE_KEY(Keypad5, 93) \
DESTRAL_DEFINE_KEY(Keypad6, 94) \
DESTRAL_DEFINE_KEY(Keypad7, 95) \
DESTRAL_DEFINE_KEY(Keypad8, 96) \
DESTRAL_DEFINE_KEY(Keypad9, 97) \
DESTRAL_DEFINE_KEY(KeypadDivide, 84) \
DESTRAL_DEFINE_KEY(KeypadMultiply, 85) \
DESTRAL_DEFINE_KEY(KeypadMinus, 86) \
DESTRAL_DEFINE_KEY(KeypadPlus, 87) \
DESTRAL_DEFINE_KEY(KeypadEnter, 88) \
DESTRAL_DEFINE_KEY(KeypadPeroid, 99) \
DESTRAL_DEFINE_KEY(KeypadEquals, 103) \
DESTRAL_DEFINE_KEY(KeypadComma, 133) \
DESTRAL_DEFINE_KEY(KeypadLeftParen, 182) \
DESTRAL_DEFINE_KEY(KeypadRightParen, 183) \
DESTRAL_DEFINE_KEY(KeypadLeftBrace, 184) \
DESTRAL_DEFINE_KEY(KeypadRightBrace, 185) \
DESTRAL_DEFINE_KEY(KeypadTab, 186) \
DESTRAL_DEFINE_KEY(KeypadBackspace, 187) \
DESTRAL_DEFINE_KEY(KeypadXor, 194) \
DESTRAL_DEFINE_KEY(KeypadPower, 195) \
DESTRAL_DEFINE_KEY(KeypadPercent, 196) \
DESTRAL_DEFINE_KEY(KeypadLess, 197) \
DESTRAL_DEFINE_KEY(KeypadGreater, 198) \
DESTRAL_DEFINE_KEY(KeypadAmpersand, 199) \
DESTRAL_DEFINE_KEY(KeypadColon, 203) \
DESTRAL_DEFINE_KEY(KeypadHash, 204) \
DESTRAL_DEFINE_KEY(KeypadSpace, 205) \
DESTRAL_DEFINE_KEY(KeypadClear, 216) \
DESTRAL_DEFINE_KEY(LeftControl, 224) \
DESTRAL_DEFINE_KEY(LeftShift, 225) \
DESTRAL_DEFINE_KEY(LeftAlt, 226) \
DESTRAL_DEFINE_KEY(LeftOS, 227) \
DESTRAL_DEFINE_KEY(RightControl, 228) \
DESTRAL_DEFINE_KEY(RightShift, 229) \
DESTRAL_DEFINE_KEY(RightAlt, 230) \
DESTRAL_DEFINE_KEY(RightOS, 231)

//#define DESTRAL_BUTTON_DEFINITIONS \
//	DEFINE_BTN(None, -1) \
//	DEFINE_BTN(A, 0) \
//	DEFINE_BTN(B, 1) \
//	DEFINE_BTN(X, 2) \
//	DEFINE_BTN(Y, 3) \
//	DEFINE_BTN(Back, 4) \
//	DEFINE_BTN(Select, 5) \
//	DEFINE_BTN(Start, 6) \
//	DEFINE_BTN(LeftStick, 7) \
//	DEFINE_BTN(RightStick, 8) \
//	DEFINE_BTN(LeftShoulder, 9) \
//	DEFINE_BTN(RightShoulder, 10) \
//	DEFINE_BTN(Up, 11) \
//	DEFINE_BTN(Down, 12) \
//	DEFINE_BTN(Left, 13) \
//	DEFINE_BTN(Right, 14)


namespace ds::in {

	// Keyboard Keys
	enum class Key 	{
#define DESTRAL_DEFINE_KEY(name, value) name = value,
		DESTRAL_KEY_DEFINITIONS
#undef DESTRAL_DEFINE_KEY
	};

	// Mouse Buttons
	enum class MouseButton {
		None = -1,
		Left = 0,
		Middle = 1,
		Right = 2,
		X1 = 3,
		X2 = 4
	};

	/*
		Keyboard input functions
	*/

	/** Check if the key is released in this frame (only returns 1 the frame that state switchs from pressed to released) Example: SDLK_PLUS*/
	bool is_key_triggered(Key key);
	/** Check if the key is pressed*/
	bool is_key_pressed(Key key);
	/** Check if the key is released*/
	bool is_key_released(Key key);


	/*
		Query mouse input functions
	*/

	/** Check if the key is released in this frame (only returns 1 the frame that state switchs from pressed to released) Example: SDL_BUTTON_LEFT*/
	bool is_ms_triggered(MouseButton mouse_button);
	/** Check if the key is pressed Example: MouseButton::Left */
	bool is_ms_pressed(MouseButton mouse_button);
	/** Check if the key is released Example: MouseButton::Left */
	bool is_ms_released(MouseButton mouse_button);
	/** Returns the position of the mouse: {0,0} is at the top-left of the window. units are in pixels (x+ right) (y+ down)*/
	ivec2 get_ms_position();
	/** Returns the motion done in this frame, units are in pixels (x+ right) (y+ down)*/
	ivec2 get_ms_motion();


// GAMEPAD TODO
//	// Game Controller Buttons
//	enum class Button {
//#define DEFINE_BTN(name, value) name = value,
//		BLAH_BUTTON_DEFINITIONS
//#undef DEFINE_BTN
//	};
//
//	// Game Controller Axes
//	enum class Axis {
//		None = -1,
//		LeftX = 0,
//		LeftY = 1,
//		RightX = 2,
//		RightY = 3,
//		LeftTrigger = 4,
//		RightTrigger = 5
//	};
}
