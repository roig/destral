#include <destral.h>


using namespace ds;



int main() {
	app::config cfg;
    cfg.width = 800;
    cfg.height = 800;
    sg_image img = { 0 };

//    ecs::registry* r;
    cfg.on_init = [&]() {
        img = rd::load_texture("zelda_gba_tileset.png");



  //      r = ecs::registry_create();
    };
    
    cfg.on_shutdown = [&]() {
//        ecs::registry_destroy(r);
    };

    static float rot = 0.01f;
    static i32 mode = 0;

    static float x = 0;
    static float y = 0;


    cfg.on_tick = [&]() {
        rot += 0.001f;


        if (in::is_key_triggered(in::Key::D) || in::is_key_triggered(in::Key::MouseRightButton)) {
            mode = (mode + 1 ) % 2;
        }
        
        if (in::is_key_pressed(in::Key::Gamepad_Left_Left) ) {
            x -= 0.001f;
        } else if (in::is_key_pressed(in::Key::Gamepad_Left_Right)) {
            x += 0.001f;
        }

        if (in::is_key_pressed(in::Key::Gamepad_Right_Left)) {
            x -= 0.001f;
        } else if (in::is_key_pressed(in::Key::Gamepad_Right_Right)) {
            x += 0.001f;
        }

        if (in::is_key_pressed(in::Key::Gamepad_Left_Up)) {
            y += 0.001f;
        } else if (in::is_key_pressed(in::Key::Gamepad_Left_Down)) {
            y -= 0.001f;
        }

        if (in::is_key_pressed(in::Key::Gamepad_Right_Up)) {
            y += 0.001f;
        } else if (in::is_key_pressed(in::Key::Gamepad_Right_Down)) {
            y -= 0.001f;
        }


        switch (mode) {
        case 0:
            rd::draw_circle({ x,y }, 0.5f, vec4(1, 0, 0, 1), 5);
            rd::draw_rect(math::build_matrix({ 0.5, 0 }, rot), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
            rd::draw_fill_rect(math::build_matrix({ -0.5, 0 }, rot), { 0.5, 0.5 }, vec4(1, 0, 0, 1), 3);
            rd::draw_line({ { 0.5, 0.5 } , { 0.0, 0.5 }, {0, -0.5}, {-0.5, -0.5 } }, vec4(0, 0, 1, 1), 2);
            rd::draw_line({ { 0.3, 0.2 } , { 0.2, 0.7 }, {0, -0.5}, {0.5, 0.5 } }, vec4(0, 1, 1, 1), 1);
            rd::draw_texture(math::build_matrix({ 0, 0 }), img, { 1,1 }, rect::from_size({ 0,0 }, { 1, 1 }), vec4{ 1,1,1,1 }, 7);

            
            break;

        case 1:
            rd::draw_texture(math::build_matrix({ 0, 0 }), img, { 1,1 }, { { 0.5,0.5 }, { 1, 1 } }, vec4{ 1,1,1,1 }, 0);
            break;
        }
        

    };

    cfg.on_render = []() {
        //gfx::draw_quad({}, {});
    };

	app::run(cfg);
    
}



