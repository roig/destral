#pragma once
#include <destral/destral_common.h>
#include <destral/destral_ecs.h>

namespace ds {
    // Destral internal system queues
    namespace queue {
        // Ordered queues by execution
        static constexpr const char* engine_init = "ds_engine_init"; // Engine subsystems init are here
        static constexpr const char* game_init = "ds_game_init"; // This is where your init systems should be, they wil be executed on game init

        /////////// Start frame:
        static constexpr const char* pre_update = "ds_pre_update"; // Input update polling
        static constexpr const char* update = "ds_update"; // This is where your indepenent framerate logic should be(particles, cosmetics..)

            // While available time for a fixed update
                // Called 0 to N times per frame this is where your framerate dependent code should be (accelerations, physics, collisions..) 
                static constexpr const char* fixed_update = "ds_fixed_update"; 
            // End fixed while

        static constexpr const char* pre_render= "ds_pre_render";
        static constexpr const char* render = "ds_render"; // Here is where you should put your rendering systems
        static constexpr const char* post_render = "ds_post_render"; // Engine specific post render systems are executed here
        /////////// End frame

        static constexpr const char* game_deinit = "ds_game_deinit"; // called when application quits
        static constexpr const char* engine_deinit = "ds_engine_deinit"; // called when application quits after game_deinit queue
    }


	struct app_config {
        // window application name
		const char* name = "Destral Engine";

        // window width
        i32 width = 1280;

        // window height
        i32 height = 720;

        // constant framerate tick (normally you can use 60, 240, 480 etc..)
        // this will be used to call on_fixed_tick callback N times a frame
        i32 fixed_target_framerate = 480;

        // max tick iterations that we will allow before discarding the rest of the tick
        // (this is to prevent the spiral of death)
        i32 max_frame_iterations = 5;
        
        // the on_ecs_config callback is executed after the engine is fully initialized and before the main loop
        // Here is where you will configure your components, entities and systems queues.
        void (*on_ecs_config)(registry* r) = nullptr;
	};

	bool app_run(const app_config& params);
    
    // request application exit, this is not immediate it will exit after the end of the loop
    void app_exit_request();

    // Return the current dt based on the current update step we are on.
    // If we are in the fixed dt update, it will return the fixed_dt else it will return the app_non_fixed_dt()
    float app_dt();

    // returns the application registry global
    registry* app_registry();

    // get the current fixed tick delta time.
    // IMPORTANT: use app_dt when fetching the dt from a ecs system
    float app_fixed_dt();

    // get the current non-fixed frame delta time
    // IMPORTANT: use app_dt when fetching the dt from a ecs system
    float app_non_fixed_dt();

    // Returns the configuration passed in app_run
    app_config app_get_config();


}