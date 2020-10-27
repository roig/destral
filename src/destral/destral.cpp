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


    ds::as::shutdown(); // this should be the first one
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