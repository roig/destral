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
		bool doing_fixed_update = false;
	};

	// global instances
	static app g_app;
	static app_config g_cfg;
	static time g_time;
	static registry* g_registry = nullptr;

	app_config app_get_config() {
		return g_cfg;
	}

	void init_engine_ecs(registry* r) {
		r->system_queue_add(queue::engine_init, "platform_init", [](registry* r) {platform_backend::init();});
		r->system_queue_add(queue::engine_init, "render_init", [](registry* r) { render_init(); });

		r->system_queue_add(queue::pre_update, "input_begin_frame", [](registry* r) {input_backend::on_input_begin_frame(); });
		r->system_queue_add(queue::pre_update, "platform_poll_events", [](registry* r) { platform_backend::poll_events(); });

		r->system_queue_add(queue::post_render, "render_present", [](registry* r) { render_present(); });
		r->system_queue_add(queue::post_render, "render_swap_buffers", [](registry* r) { platform_backend::swap_buffers(); });

		r->system_queue_add(queue::engine_deinit, "render_deinit", [](registry* r) { render_deinit(); });
		r->system_queue_add(queue::engine_deinit, "platform_deinit", [](registry* r) { platform_backend::deinit(); });
	}

	bool app_run(const app_config& cfg_) {
		g_cfg = cfg_;
		g_registry = new registry();

		// configure ECS for engine and game
		init_engine_ecs(g_registry);
		if (g_cfg.on_ecs_config) { g_cfg.on_ecs_config(g_registry); }

		// run init systems
		g_registry->system_queue_run(queue::engine_init);
		g_registry->system_queue_run(queue::game_init);

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
				g_registry->system_queue_run(queue::pre_update);
			}

			// 2) Full tick perform
			{
				// 2) Tick for the full frame dt (independent timestep)
				g_registry->system_queue_run(queue::update);
			}


			// 3) Do as many fixed updates as we can (fixed timestep)
			{
				g_app.doing_fixed_update = true;
				while (g_time.dt_fixed_acc >= g_time.current_fixed_dt) {
					g_registry->system_queue_run(queue::fixed_update);
					g_time.dt_fixed_acc -= g_time.current_fixed_dt;
				}
				g_app.doing_fixed_update = false;
			}

			// 4) render
			{
				g_registry->system_queue_run(queue::pre_render);
				g_registry->system_queue_run(queue::render);
				g_registry->system_queue_run(queue::post_render);
			}
		}


		// Cleanup process
		{

			g_registry->system_queue_run(queue::game_deinit);
			g_registry->system_queue_run(queue::engine_deinit);
			
			delete g_registry;
		}

		return 0;
	}

	void app_exit_request() {
		g_app.is_exiting = true;
	}

	float app_dt() {
		if (g_app.doing_fixed_update) return app_fixed_dt(); else return app_non_fixed_dt();
	}

	float app_fixed_dt() {
		return g_time.current_fixed_dt.count();
	}

	float app_non_fixed_dt() {
		return g_time.current_frame_dt.count();
	}

	// returns the application registry global
	registry* app_registry() {
		return g_registry;
	}

}


