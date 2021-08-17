#include <destral.h>


using namespace ds;



int main() {
    app_config cfg;
   // cfg.width = 800;
   // cfg.height = 800;

   // resource<image> img_resource;
   // resource<animation_collection> anim_resource;
   // sprite spr;
   // cfg.on_init = [&]() {

   //     caches_init();
   //     cache_images_add_info("zelda_tileset_gba", "zelda_gba_tileset.png");
   //     cache_animations_add_info("orco", "UnitAnimations.aseprite");

   //     anim_resource = cache_animations()->get("orco");


   //     spr.animations = cache_animations()->get("orco");
   //     spr.play("Clanrat.Walk");

   // };
   // 
   // //cfg.on_shutdown = [&]() {
   // //    
   // //    caches_deinit();
   // //};

   // static float rot = 0.01f;
   // static i32 mode = 0;

   // static float x = 0;
   // static float y = 0;


   ///* cfg.on_tick = [&](float dt) {
   //     rot += 0.001f;


   //     if (key_is_triggered(key::D) || key_is_triggered(key::MouseRightButton)) {
   //         mode = (mode + 1 ) % 2;
   //     }
   //     
   //     if (key_is_pressed(key::Gamepad_Left_Left) ) {
   //         x -= 0.001f;
   //     } else if (key_is_pressed(key::Gamepad_Left_Right)) {
   //         x += 0.001f;
   //     }

   //     if (key_is_pressed(key::Gamepad_Right_Left)) {
   //         x -= 0.001f;
   //     } else if (key_is_pressed(key::Gamepad_Right_Right)) {
   //         x += 0.001f;
   //     }

   //     if (key_is_pressed(key::Gamepad_Left_Up)) {
   //         y += 0.001f;
   //     } else if (key_is_pressed(key::Gamepad_Left_Down)) {
   //         y -= 0.001f;
   //     }

   //     if (key_is_pressed(key::Gamepad_Right_Up)) {
   //         y += 0.001f;
   //     } else if (key_is_pressed(key::Gamepad_Right_Down)) {
   //         y -= 0.001f;
   //     }


   //     switch (mode) {
   //     case 0:
   //         draw_circle({ x,y }, 0.5f, vec4(1, 0, 0, 1), 5);
   //         draw_rect(math::build_matrix({ 0.5, 0 }, rot), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
   //         draw_fill_rect(math::build_matrix({ -0.5, 0 }, rot), { 0.5, 0.5 }, vec4(1, 0, 0, 1), 3);
   //         draw_line({ { 0.5, 0.5 } , { 0.0, 0.5 }, {0, -0.5}, {-0.5, -0.5 } }, vec4(0, 0, 1, 1), 2);
   //         draw_line({ { 0.3, 0.2 } , { 0.2, 0.7 }, {0, -0.5}, {0.5, 0.5 } }, vec4(0, 1, 1, 1), 1);

   //         spr.update(dt);
   //         draw_texture(math::build_matrix({ 0, 0 }), spr.get_current_image(), { 0.6, 0.5 }, rect::from_size({ 0,0 }, { 1, 1 }), vec4{ 1,1,1,1 }, 7);

   //         
   //         break;

   //     case 1:
   //         draw_texture(math::build_matrix({ 0, 0 }), img_resource, { 1,1 }, { { 0.5,0.5 }, { 1, 1 } }, vec4{ 1,1,1,1 }, 0);
   //         break;
   //     }
   //     

   // };*/

   // //cfg.on_render = []() {
   // //    //gfx::draw_quad({}, {});
   // //};

	app_run(cfg);
    
}



