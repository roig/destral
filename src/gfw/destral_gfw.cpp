// game framework
#include <destral/gfw/destral_gfw.h>
#include <destral/destral_app.h>
#include <destral/destral_renderer.h>

namespace ds {
	void cp::hierarchy::register_component(registry* r) {
        r->component_register<cp::hierarchy>(cp::hierarchy::name, 
            cp::hierarchy::serialize, cp::hierarchy::cleanup);
	}

    void cp::camera::register_component(registry* r) {
        r->component_register<cp::camera>(cp::camera::name);
    }

    void en::camera::register_entity(registry* r) {
        r->entity_register(en::camera::name, { cp::hierarchy::name, cp::camera::name});
    }

    void en::camera::render_cameras_system(registry* r) {
        view v = r->view_create({ cp::hierarchy::name, cp::camera::name });
        auto hr_idx = v.index(cp::hierarchy::name);
        auto cam_idx = v.index(cp::camera::name);
        while (v.valid()) {
            auto hr = v.data<cp::hierarchy>(hr_idx);
            auto cam = v.data<cp::camera>(cam_idx);
            render_add_camera(hr->ltw(), cam->viewport, cam->aspect, cam->ortho_width);
            v.next();
        }
    }
}