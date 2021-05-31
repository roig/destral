#pragma once
#include <destral/core/destral_common.h>
#include <destral/math/destral_math.h>
#include <destral/input/destral_input.h>

// Idea taken from:
// https://github.com/NoelFB/blah/blob/14a53c0f3a5b68278cc63d7c2769d2e3c750a190/src/internal/graphics_backend.h
// 

namespace ds::input_backend {

	void on_input_begin_frame();
	void on_key_change(bool is_pressed, ds::in::Key key, i32 controller_id);
	void on_mouse_motion(ds::ivec2 pos, ds::ivec2 relative);
	void on_gamepad_axis_change(float value, in::Key axis, i32 controller_idx);

}