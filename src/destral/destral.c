#include "destral/destral.h"

// Those have to be the first
//#define AP_SDL_IMPL
#include "ap_sdl.h"
//#define AP_DEBUG_IMPL
#include "ap_debug.h"

///////////

//#include <entt/entity/registry.hpp>
//#include "destral/renderer.h"

// Main World
//entt::registry world;




void _ds_app_tick(void* d) {
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->init_cb) app->init_cb(NULL);
    //ds::render::drawCameras(world, APS_AppWindow());
}


void _ds_app_init(void* d) {
    ap_dbg_init("DestralApp");

    AP_INFO("Destral engine initialized");
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->init_cb) app->init_cb(NULL);
}

void _ds_app_shutdown(void* d) {
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->cleanup_cb) app->cleanup_cb(NULL);

    AP_INFO("Destral engine shutdown");
    ap_dbg_shutdown();
}

void _ds_app_on_event(SDL_Event* e, void* d) {
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->event_cb) {
        struct ds_app_event ev = {e, 0 };
        app->event_cb(NULL, &ev);
    }
}
   
int ds_app_run(struct ds_app_desc* desc) {
    AP_ASSERT(desc);

    // Configure window
    struct ap_sdl_window_desc wd = {0};
    wd.window_title = desc->window_name;
    wd.wsize_x = desc->window_width;
    wd.wsize_y = desc->window_height;
    wd.opengl_major = 3;
    wd.opengl_minor = 3;
    wd.is_gl_context_debug = 0;

    // Configure app
    struct ap_sdl_app_desc d = { 0 };
    d.frame_cb = &_ds_app_tick;
    d.init_cb = &_ds_app_init;
    d.cleanup_cb = &_ds_app_shutdown;
    d.event_cb = &_ds_app_on_event;

    return ap_sdl_app_run(&d, &wd, desc);
}
