#pragma once
#include <destral/core/destral_common.h>

namespace ds::app {
    //    The on_init callback is executed after the engine is fully initialized and before the main loop
    //    The on_frame callback is executed on each tick loop
    //    The on_render callback is executed on each render tick loop
    //    The on_shutdown callback is executed once right before the application quits.
	struct config {
		const char* name = "Destral Engine";
        i32 width = 1280;
        i32 height = 720;
		void (*on_init)() = nullptr;
        void (*on_frame)() = nullptr;
        void (*on_render)() = nullptr;
        void (*on_shutdown)() = nullptr;
	};
	

	bool run(const config& params);
    
    void exit_request();
}