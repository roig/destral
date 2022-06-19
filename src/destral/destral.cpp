#include "ap/ap_sdl.h"
#include "ap/ap_debug.h"
#include "destral.h"
#include "transform.h"
#include "render.h"
#include "assets.h"
#include "entt/entity/registry.hpp"
#include "ecs.h"



entt::registry g_world;

namespace ds {
    


void app_init(void* data) {
    ap_dbg_init("logs/DestralApp");
    ds::ecs::init(g_world);
    ds::tr::set_callbacks(g_world);
    ds::rd::init();
    ds::as::init();


    platform_app_desc* app = (platform_app_desc*)data;
    if (app->init_cb)
        app->init_cb(g_world);
}

void app_tick(void* data) {
    //AP_TRACE("New Frame Starts.");
    /* Run systems */
    platform_app_desc* app = (platform_app_desc*)data;
    if (app->tick_cb)
        app->tick_cb(g_world);

    ds::rd::draw_all(g_world);
}



void app_cleanup(void* data) {
    platform_app_desc* app = (platform_app_desc*)data;
    if (app->cleanup_cb) app->cleanup_cb(g_world);


    ds::as::shutdown();
    ds::rd::shutdown();
    ds::ecs::shutdown();
    AP_INFO("Destral engine shutdown");
    ap_dbg_shutdown();
}

void app_on_event(SDL_Event* event, void* data) {
    platform_app_desc* app = (platform_app_desc*)data;
    if (app->on_event_cb) {
        platform_event ev = {event, 0 };
        app->on_event_cb(g_world, ev);
    }
}
   
int app_run(platform_app_desc& desc) {
    // Configure app
    ap_sdl_app_desc d = { 0 };
    d.renderer_mode = 1;
    d.window_title = desc.window_name.c_str();
    d.wwidth = desc.window_width;
    d.wheight = desc.window_height;
    d.frame_cb = &app_tick;
    d.init_cb = &app_init;
    d.cleanup_cb = &app_cleanup;
    d.event_cb = &app_on_event;
    d.renderer_mode = 1;

    return ap_sdl_app_run(&d, &desc);
}
}










//int game_run() {
//	// create_window(); // opengl
//
//	// time initialization
//
//	// main loop till game not ended.
//	{
//		// get performance timer
//		// 
//
//	}
//	
//}
//
//
//int app_bucle(struct ap_sdl_app_desc* desc, void* user_data) {
//	assert(desc);
//
//	if (desc->renderer_mode == 0) {
//		if (ap_sdl_create_window(desc->window_title, desc->wwidth, desc->wheight, &g_window, &g_renderer) == EXIT_FAILURE) {
//			return EXIT_FAILURE;
//		}
//	}
//	else {
//		if (ap_sdl_create_window_gl(desc->window_title, desc->wwidth, desc->wheight, &g_window, &g_glctx) == EXIT_FAILURE) {
//			return EXIT_FAILURE;
//		}
//		gladLoadGL();
//	}
//
//	if (desc->init_cb) desc->init_cb(user_data);
//	int app_ended = 0;
//
//	// Time related initialization
//	Uint64 tm_ts_last = SDL_GetPerformanceCounter();
//	g_tm_dtacc_s = 0.0f;
//	g_tm_fps = 0.0f;
//	const float MAX_FPS = 200.0f;
//	const float MIN_FPS = 2.0f;  // 1 / 2 FPS = 500 milliseconds = > 0.5 seconds
//	const float MIN_DELTA = 1.0f / MAX_FPS;
//	const float MAX_DELTA = 1.0f / MIN_FPS;
//
//	while (!app_ended) {
//		// seconds = units / persecond      OR     units = seconds * persecond
//		// add current delta in seconds (now - last) / frequency
//		const uint64_t now = SDL_GetPerformanceCounter();
//		g_tm_dtacc_s += (tm_ts_now - tm_ts_last) / (float)SDL_GetPerformanceFrequency();
//		tm_ts_last = tm_ts_now;
//
//		// Avoid death spiral (TODO MIRAR BE COM FUNCIONA L'SPIRAL DEATH)
//		if (g_tm_dtacc_s > MAX_DELTA) {
//			g_tm_dtacc_s = MAX_DELTA;
//			// We are going very slow! discard some delta
//		}
//
//		if (g_tm_dtacc_s < MIN_DELTA) {
//			// We are going too fast!! skip this frame
//			continue;
//		}
//		g_tm_fps = 1.0f / g_tm_dtacc_s;
//		// 1 start frame (handle events)
//		ap_sdl_input_begin_frame(&g_app_input);
//		SDL_Event e;
//		while (SDL_PollEvent(&e)) {
//			// Handle input events
//			ap_sdl_input_handle_event(&g_app_input, &e);
//
//			// Close window event
//			if (e.type == SDL_WINDOWEVENT &&
//				e.window.event == SDL_WINDOWEVENT_CLOSE &&
//				e.window.windowID == SDL_GetWindowID(g_window)) {
//				app_ended = 1;
//				continue;
//			}
//
//			// handle other events in user land
//			if (desc->event_cb) desc->event_cb(&e, user_data);
//		}
//
//		//printf("DT: %f   FPS: %f\n", g_tm_dtacc_s, 1.0f/ g_tm_dtacc_s);
//		// 2 Handle the frame update
//		if (desc->frame_cb) desc->frame_cb(user_data);
//
//		// Blit the screen
//		if (desc->renderer_mode == 0) {
//			SDL_RenderPresent(g_renderer);
//		}
//		else {
//			SDL_GL_SwapWindow(g_window);
//		}
//
//		// reset elapsed accumulator
//		g_tm_dtacc_s = 0;
//	}
//
//	// user cleanup callback
//	if (desc->cleanup_cb) desc->cleanup_cb(user_data);
//
//	// sdl cleanup
//	if (desc->renderer_mode == 0) {
//		SDL_DestroyRenderer(g_renderer);
//	}
//	else {
//		SDL_GL_DeleteContext(g_glctx);
//	}
//	SDL_DestroyWindow(g_window);
//	SDL_Quit();
//	return EXIT_SUCCESS;
//}
