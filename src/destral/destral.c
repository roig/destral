#include "destral/destral.h"
#include "destral/ecs.h"
#include "destral/render.h"
#include "flecs.h"



// This always to the bottom..
#include "ap_sdl.h"
#include "ap_debug.h"



/* Component type */
typedef struct Message {
    const char* text;
} Message;

/* Must have the same name as the ECS_SYSTEM definition */
void PrintMessage(ecs_iter_t* it) {
    /* Get a pointer to the array of the first column in the system. The order
     * of columns is the same as the one provided in the system signature. */
    ECS_COLUMN(it, Message, msg, 1);

    /* Iterate all the messages */
    for (int i = 0; i < it->count; i++) {
        printf("%s\n", msg[i].text);
    }
}


ecs_world_t* world;

void _ds_app_init(void* d) {
    ap_dbg_init("DestralApp");
    world = ecs_init();
    ds_ecs_register_cp(world);
    ds_rd_init();

    /* Create the world, pass arguments for overriding the number of threads,fps
    * or for starting the admin dashboard (see flecs.h for details). */
    world = ecs_init();

    /* Define component */
    ECS_COMPONENT(world, Message);

    /* Define a system called PrintMessage that is executed every frame, and
     * subscribes for the 'Message' component */
    ECS_SYSTEM(world, PrintMessage, EcsOnUpdate, Message);

    /* Create new entity, add the component to the entity */
    ecs_entity_t e = ecs_new(world, Message);
    ecs_set(world, e, Message, { .text = "Hello Flecs!" });

    /* Set target FPS for main loop to 1 frame per second */
    //ecs_set_target_fps(world, 1);


    AP_INFO("Destral engine initialized");
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->init_cb) app->init_cb(NULL);



}

void _ds_app_tick(void* d) {
    /* Run systems */
    //ecs_progress(world, ap_sdl_app_dt());
    //ds_rd_draw_line((float[]) { -100, 0 }, (float[]) { 100, 0 });
    ds_rd_draw_all();
   // ds_rd_draw_line(0, -100, 0, 100);
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->init_cb) app->frame_cb(NULL);
 
    //ds::render::drawCameras(world, APS_AppWindow());
}



void _ds_app_shutdown(void* d) {
    struct ds_app_desc* app = (struct ds_app_desc*) d;
    if (app->cleanup_cb) app->cleanup_cb(NULL);


    
    ecs_fini(world);
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

    // Configure app
    struct ap_sdl_app_desc d = { 0 };
    d.renderer_mode = 0; // SDL2 renderer
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
