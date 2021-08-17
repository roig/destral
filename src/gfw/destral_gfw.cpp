// game framework
#include <destral/gfw/destral_gfw.h>
#include <destral/destral_app.h>
#include <destral/destral_renderer.h>

namespace ds {



	void game_framework_register_entities(registry* r) {
        // Register the components
        r->cp_register_serialize<hierarchy>(hierarchy::cp_name);
        r->cp_register_serialize<camera>(camera::cp_name);

        // Register the entities
        r->entity_register({ .name = camera::e_name, .cp_names = {hierarchy::cp_name, camera::cp_name}});
	}

    void camera::render_cameras_system(registry* r) {
        view v = r->view_create({ hierarchy::cp_name, camera::cp_name });
        auto hr_idx = v.index(hierarchy::cp_name);
        auto cam_idx = v.index(camera::cp_name);
        while (v.valid()) {
            auto hr = v.data<hierarchy>(hr_idx);
            auto cam = v.data<camera>(cam_idx);
            render_add_camera(hr->ltw(), cam->viewport, cam->aspect, cam->ortho_width);
            v.next();
        }
    }

	void game_framework_deinit() {

	}
}