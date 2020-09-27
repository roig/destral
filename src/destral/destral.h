#pragma once
#include <entt/entity/fwd.hpp>
#include <SDL_events.h>

struct ds_app_event {
    SDL_Event* event;
    int accepted;
};

/*
    
*/
struct ds_app_desc {
    const char* window_name;
    int window_width;
    int window_height;
    
    void (*init_cb)(entt::registry*);
    void (*frame_cb)(entt::registry*);
    void (*cleanup_cb)(entt::registry*);
    void (*event_cb)(entt::registry*, struct ds_app_event*);
};

int ds_app_run(struct ds_app_desc* desc);

    

