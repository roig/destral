// game framework
#include <destral/gfw/destral_gfw.h>

namespace ds {



	void game_framework_init(registry* r) {
        // Register the components
        DS_ECS_COMPONENT_REGISTER_WITH_SERIALIZE(r, hierarchy);
        DS_ECS_COMPONENT_REGISTER_WITH_SERIALIZE(r, camera);

        // Register the entities
        r->entity_register({ .name = "ds_camera_entity" , .cp_names = {"hierarchy", "camera"}});

        // Register the systems
        r->system_queue_add("ds_render_queue", "update_camera_system", &camera::update_camera_system);
	}

    void camera::update_camera_system(registry* r) {
        
    }

	void game_framework_deinit() {

	}
}