#include <destral/destral_app.h>
#include <chrono>
#include <iostream>

#include "backends/destral_platform_backend.h"
#include "backends/destral_input_backend.h"
#include <destral/destral_renderer.h>

#include <thread>

// Idea from:
// https://github.com/NoelFB/blah/blob/master/src/core/app.cpp

namespace ds {
	struct time {
		std::chrono::duration<float> dt_fixed_acc = std::chrono::seconds(0);
		std::chrono::duration<float> current_frame_dt = std::chrono::seconds(0);
		std::chrono::duration<float> current_fixed_dt = std::chrono::seconds(0);
	};

	struct app {
		bool is_exiting = false;
		
	};

	// global instances
	static app g_app;
	static app_config g_cfg;
	static time g_time;
	static registry* g_registry = nullptr;

	bool app_run(const app_config& cfg_) {
		g_cfg = cfg_;
		


		platform_backend::init(g_cfg);
		render_init();
		// input_backend::init();


		g_registry = registry_create();
		if (g_cfg.on_init) {
			g_cfg.on_init();
		}

		typedef std::chrono::high_resolution_clock hrclock;
		hrclock::time_point last = hrclock::now();
		
		// Update loop
		while (!g_app.is_exiting) {

			// 1) compute delta
			{ 
				// target fixed tick seconds
				g_time.current_fixed_dt = std::chrono::duration<float>(1.0f / g_cfg.fixed_target_framerate);

				// First set new full frame delta and increment dt_fixed_acc
				hrclock::time_point now = hrclock::now();
				g_time.current_frame_dt = (now - last);
				g_time.dt_fixed_acc += (now - last);
				last = now;

				// check if we are going too fast and sleep if necessary
				// this checks if no we can't make a fixed_tick currently and will wait till we can make at least one
				while (g_time.dt_fixed_acc < g_time.current_fixed_dt) {
					// wait
					std::this_thread::sleep_for(g_time.current_fixed_dt - g_time.dt_fixed_acc);

					// then add the new delta to dt and dt_fixed_accum
					now = hrclock::now();
					g_time.current_frame_dt += (now - last);
					g_time.dt_fixed_acc += (now - last);
					last = now;
				}

				// check if we are doing too many updates (avoid spiral of death)
				// this checks if we are doing too many fixed_ticks
				const auto max_time_allowed = g_time.current_fixed_dt * g_cfg.max_frame_iterations;
				if (g_time.dt_fixed_acc > max_time_allowed) {
					//DS_WARNING("Too many updates, set max time allowed as dt_acc_time to avoid possible spiral of death");
					g_time.current_frame_dt = max_time_allowed - (g_time.current_frame_dt - g_time.dt_fixed_acc);
					g_time.dt_fixed_acc = max_time_allowed;
				}
			}

			// 1) Begin Frame
			{
				input_backend::on_input_begin_frame();
			}

			// 2) Full tick perform
			{
				platform_backend::tick();
				render_tick();


				// 2) Tick for the full frame dt (independent timestep)
				if (g_cfg.on_tick) {
					g_cfg.on_tick(app_dt());
				}
			}


			// 3) Do as many fixed updates as we can (fixed timestep)
			{
				while (g_time.dt_fixed_acc >= g_time.current_fixed_dt) {
					if (g_cfg.on_fixed_tick) {
						g_cfg.on_fixed_tick(app_fixed_dt());
					}
					g_time.dt_fixed_acc -= g_time.current_fixed_dt;
				}
			}

			// 4) render loop
			{
				render_before_render();

				if (g_cfg.on_render) {
					g_cfg.on_render();
				}
				
				render_after_render();
				platform_backend::present();
			}
		}


		// Cleanup process
		{
			if (g_cfg.on_shutdown) {
				g_cfg.on_shutdown();
			}


			registry_destroy(g_registry);
			//input_backend::shutdown();
			render_shutdown();
			platform_backend::shutdown();


		}

		return 0;
	}

	void app_exit_request() {
		g_app.is_exiting = true;
	}

	float app_fixed_dt() {
		return g_time.current_fixed_dt.count();
	}

	float app_dt() {
		return g_time.current_frame_dt.count();
	}

	// returns the application registry global
	registry* app_registry() {
		return g_registry;
	}

}



