#include <destral.h>


using namespace ds;


resource link_texture;
void test_resources_init(registry* r) {
    link_texture = resource_get(r, "content/NES-ZeldaLink.png");

    entity texture = link_texture.get();
}

void test_resources_deinit(registry* r) {
    link_texture = resource(); // remove reference to the resource
}

void test_render(registry* r) {
    render_texture(r, ds::math::build_matrix({ 0,0 }), link_texture);
}

int main() {
    app_config cfg;
    cfg.width = 256*4;
    cfg.height = 240*4;


    //sprite walk_link;
    ////resource<animation_collection> link_animations;
    cfg.on_ecs_config = [](registry* r) {
        DS_REGISTRY_QUEUE_ADD_SYSTEM(r,queue::game_init, test_resources_init);
        DS_REGISTRY_QUEUE_ADD_SYSTEM(r,queue::update, test_render);
        DS_REGISTRY_QUEUE_ADD_SYSTEM(r,queue::game_deinit, test_resources_deinit);
       

    //    
    //    caches_init();
    //    cache_images_add_info("ZeldaLink", "content/NES-ZeldaLink.png");

    //    animation_collection link_animations;
    //    animation walk_down;
    //    walk_down.name = "link_walk_down";
    //    animation_frame f1;

    //    //auto img = new image();
    //    //f1.source_rect_px = rect::from_top_left_size()



    //    //walk_down.frames

    //    //link_animations.animations["link_walk_down"] = animation();
    //    ////16x16 frames  1,11 frame top left one
    };
    
    //cfg.on_shutdown = [&]() {
    //    caches_deinit();
    //};

    //cfg.on_tick = [&](float dt) {

    //};

    //cfg.on_render = []() {
    //    draw_texture(math::build_matrix({ 0,0 }), cache_images()->get("ZeldaLink"));
    //};

	app_run(cfg);
    
}



