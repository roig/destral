#include "ap_sdl.h"
#include "ap_debug.h"
#include "destral.h"
#include "transform.h"
#include "render.h"
#include "assets.h"
#include <entt/entity/registry.hpp>
#include "ecs.h"

entt::registry g_world;

void _ds_app_init(void* d) {
    ap_dbg_init("logs/DestralApp");
    ds::ecs::init(g_world);
    ds::tr::set_callbacks(g_world);
    ds::rd::init();
    ds::as::init();

    ds_app_desc* app = (ds_app_desc*)d;
    if (app->init_cb)
        app->init_cb(&g_world);
}

void _ds_app_tick(void* d) {
    /* Run systems */

    //ds_rd_draw_line((float[]) { -100, 0 }, (float[]) { 100, 0 });
    ds::rd::draw_all(g_world);
    // ds_rd_draw_line(0, -100, 0, 100);
    ds_app_desc* app = (ds_app_desc*) d;
    if (app->frame_cb)
        app->frame_cb(&g_world);

}



void _ds_app_shutdown(void* d) {
    ds_app_desc* app = (ds_app_desc*) d;
    if (app->cleanup_cb) app->cleanup_cb(&g_world);


    ds::as::shutdown(); // this should be the first one
    ds::rd::shutdown();
    ds::ecs::shutdown();
    AP_INFO("Destral engine shutdown");
    ap_dbg_shutdown();
}

void _ds_app_on_event(SDL_Event* e, void* d) {
    ds_app_desc* app = (ds_app_desc*) d;
    if (app->event_cb) {
        ds_app_event ev = {e, 0 };
        app->event_cb(&g_world, &ev);
    }
}
   
int ds_app_run(struct ds_app_desc* desc) {
    AP_ASSERT(desc);

    // Configure app
    ap_sdl_app_desc d = { 0 };
    d.renderer_mode = 1;
    d.window_title = desc->window_name;
    d.wwidth = desc->window_width;
    d.wheight = desc->window_height;
    d.frame_cb = &_ds_app_tick;
    d.init_cb = &_ds_app_init;
    d.cleanup_cb = &_ds_app_shutdown;
    d.event_cb = &_ds_app_on_event;
    d.renderer_mode = 1;

    return ap_sdl_app_run(&d, desc);
}
