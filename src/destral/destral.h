#pragma once
#include <entt/entity/fwd.hpp>
#include <SDL_events.h>

struct ds_app_event {
    SDL_Event* event;
    int accepted;
};

/*
    
*/
using ds_world = entt::registry;
struct ds_app_desc {
    const char* window_name;
    int window_width;
    int window_height;
    
    void (*init_cb)(ds_world*);
    void (*frame_cb)(ds_world *);
    void (*cleanup_cb)(ds_world*);
    void (*event_cb)(ds_world*, struct ds_app_event*);
};

int ds_app_run(struct ds_app_desc* desc);

    

