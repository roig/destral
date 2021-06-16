#pragma once
#include <destral/core/destral_common.h>
#include <functional>

namespace ds::app {

	struct config {
        // window application name
		const char* name = "Destral Engine";

        // window width
        i32 width = 1280;

        // window height
        i32 height = 720;

        // constant framerate tick (normally you can use 60, 240, 480 etc..)
        // this will be used to call on_fixed_tick callback N times a frame
        i32 fixed_target_framerate = 480;

        // max tick iterations that we will allow before discarding the rest of the tick
        // (this is to prevent the spiral of death)
        i32 max_frame_iterations = 5;
       
        // the on_init callback is executed after the engine is fully initialized and before the main loop
		std::function<void(void)> on_init = nullptr;

        // the on_tick callback is executed once per each frame with the full dt of the frame
        // this is where your indepenent framerate logic should be (particles, cosmetics.. )
        // this happens before on_fixed_tick calls
        std::function<void(float dt)> on_tick = nullptr;

        // the on_fixed_tick callback is executed N times each frame tick with the fixed_target_framerate as dt of the fixed_tick
        // this is where your framerate dependent code should be (accelerations, physics, collisions..) 
        // this happens after on_tick call
        std::function<void(float dt)> on_fixed_tick = nullptr;

        // the on_render callback is executed at the end of the frame
        std::function<void(void)> on_render = nullptr;

        // the on_shutdown callback is executed once right before the application quits.
        std::function<void(void)> on_shutdown = nullptr;
	};
	

	bool run(const config& params);
    
    // request application exit, this is not immediate it will exit after the end of the loop
    void exit_request();

    // get the current fixed tick delta time
    float fixed_dt();

    // get the current non-fixed frame delta time
    float dt();

}