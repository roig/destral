#pragma once
#include "cglm/affine2d.h"

#include <SDL_events.h>
struct ds_app_event {
    SDL_Event* event;
    int accepted;
};

/*
    
*/
typedef void ds_world; // TODO Change this
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

    

