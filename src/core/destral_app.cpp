#include <destral/core/destral_app.h>
#include <chrono>
#include <iostream>

#include "../backends/destral_platform_backend.h"
#include "../backends/destral_graphics_backend.h"


// Idea from:
// https://github.com/NoelFB/blah/blob/master/src/core/app.cpp

namespace ds::app {

	struct app {
		bool is_exiting = false;
		float fps = 0.0f; // frames per second
		float dt = 0.0f; // delta time in seconds
	};


	// global application instance
	static app g_app;

	bool run(const config& params) {
		platform_backend::init(params);
		graphics_backend::init();
		// input_backend::init();


		if (params.on_init) {
			params.on_init();
		}

		// Loop application
		const float MAX_FPS = 200.0f;
		const float MIN_FPS = 2.0f;  // 1 / 2 FPS = 500 milliseconds = > 0.5 seconds
		const float MIN_DELTA_SECONDS = 1.0f / MAX_FPS;
		const float MAX_DELTA_SECONDS = 1.0f / MIN_FPS;

		using namespace std::chrono;
		typedef high_resolution_clock hrclock;
		hrclock::time_point last = hrclock::now();
		
		while (!g_app.is_exiting) {

			// poll platform events
			platform_backend::frame();


			/////////////////////////////////////////////////////////// 
			/// Maybe we can make a loop that ticks with a fixed timestep
			/// 

			const hrclock::time_point now = hrclock::now();
			g_app.dt += std::chrono::duration<float>(now - last).count();
			last = now;

			// Avoid death spiral (TODO MIRAR BE COM FUNCIONA L'SPIRAL DEATH)
			if (g_app.dt > MAX_DELTA_SECONDS) {
				g_app.dt = MAX_DELTA_SECONDS;
				// We are going very slow! discard some delta
			}

			if (g_app.dt < MIN_DELTA_SECONDS) {
				// We are going too fast!! skip this frame
				continue;
			}

			g_app.fps = 1.0f / g_app.dt;

			///
			/////////////////////////////////////////////////////////// 


			// delta time tick (non-fixed...)
			{
				//input_backend::frame();
				graphics_backend::frame();

				if (params.on_frame) {
					params.on_frame();
				}
			}

			// render
			{
				graphics_backend::before_render();

				if (params.on_render) {
					params.on_render();
				}

				graphics_backend::after_render();
				platform_backend::present();
			}
			

			// reset elapsed accumulator
			g_app.dt = 0.0f;
		}


		// Cleanup process
		{
			if (params.on_shutdown) {
				params.on_shutdown();
			}

			//input_backend::shutdown();
			graphics_backend::shutdown();
			platform_backend::shutdown();
		}

		return 0;
	}

	void exit_request() {
		g_app.is_exiting = true;
	}
}




