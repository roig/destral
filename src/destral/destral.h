#pragma once
#include <entt/entity/fwd.hpp>
#include <SDL_events.h>
#include <functional>


namespace ds {

    
struct platform_event {
    SDL_Event* event;
    int accepted;
};


struct platform_app_desc {
    std::string window_name = "Destral Engine window";
    int window_width = 1280;
    int window_height = 720;

    // application callbacks
    std::function<void(entt::registry&) > init_cb;
    std::function<void(entt::registry&) > tick_cb;
    std::function<void(entt::registry&) > cleanup_cb;
    std::function<void(entt::registry&, platform_event& ) > on_event_cb;
};

int app_run(platform_app_desc& desc);

}

    

