#include <destral.h>
#include <destral/destral_resource2.h>

using namespace ds;

entity g_link_texture;
entity g_link_sprite;
entity g_camera;
entity g_link_sprite_renderer;

entity make_sprite_from_texture(registry* r, entity e_texture) {
    dsverify(r->entity_is_name(e_texture, en::texture::name));
    cp::texture* cp_tex = r->component_get<cp::texture>(e_texture, cp::texture::name);
    ivec2 tex_size = cp_tex->get_size();

    auto e_sprite = r->entity_make(en::sprite::name);
    cp::sprite* cp_spr = r->component_get<cp::sprite>(e_sprite, cp::sprite::name);
    cp::sprite::animation anim;
    anim.name = "link_animation";
    cp::sprite::animation::frame anim_frame;
    anim_frame.texture = e_texture;
    anim_frame.time = 1;
    anim_frame.source_rect_px = ds::rect::from_size({ 0,0 }, {tex_size.x, tex_size.y});
    anim.frames.push_back(anim_frame);
    
    cp_spr->animations.set(fnv1a_32bit("link_animation"), anim);
    return e_sprite;
}


/**
 * @brief Create a texture uv rect normalized from a top left origin position rectangle, 
 * half the rectangle size and a texture size.
 * 
 * @param top_left_position_px top left position of the rect from a top left origin in pixels.
 * @param half_rect_size_px half rect size in pixels.
 * @param texture_size_px size of the texture in pixels.
 *
 * From:
 * (0,0)--------      
 * |
 * |   *(x,y)_
 * |   |      | 
 * |   |      | 
 * |   |______| 
 * |   
 * 
 * To:
 * |
 * |    ______ 
 * |   |      | 
 * |   |      | 
 * |   *(x,y)_|
 * |   
 * (0,0)--------      
 * 
 */
rect rect_texture_uv_from_top_left_origin_px(vec2 top_left_position_px, vec2 half_rect_size_px, vec2 texture_size_px) {
    const auto rect_center_px_from_top_left = top_left_position_px + half_rect_size_px;
    const vec2 rect_center_px_from_bottom_left{ rect_center_px_from_top_left.x, texture_size_px.y - rect_center_px_from_top_left.y };
    rect r = rect::from_center(rect_center_px_from_bottom_left, half_rect_size_px);
    r = math::map_range_clamped(rect::from_size({ 0,0 }, texture_size_px), rect::from_size({ 0,0 }, { 1,1 }), r);
    return r;
}

entity create_link_sprite_and_animations(registry *r, entity link_texture) {
    dsverify(r->entity_is_name(link_texture, en::texture::name));

    cp::texture* cp_tex = r->component_get<cp::texture>(link_texture, cp::texture::name);
    const ivec2 tex_size_px = cp_tex->get_size();

    auto e_sprite = r->entity_make(en::sprite::name);
    cp::sprite* cp_spr = r->component_get<cp::sprite>(e_sprite, cp::sprite::name);

    cp::sprite::animation anim;
    anim.name = "walk_down";
    const float sprite_frame_horizontal_spacing_px = 8;
    const vec2 sprite_size_px = { 24,32 };
    vec2 walk_rect_tl_origin_tl_pos_px = { 12,72 };
    for (i32 i = 0; i < 10; i++) {
        cp::sprite::animation::frame f;
        f.texture = link_texture;
        f.time = 0.05f;
        f.source_rect_px = rect_texture_uv_from_top_left_origin_px(walk_rect_tl_origin_tl_pos_px, sprite_size_px / 2.0f, tex_size_px);
        walk_rect_tl_origin_tl_pos_px.x += sprite_size_px.x + sprite_frame_horizontal_spacing_px;
        anim.frames.push_back(f);
    }
    cp_spr->animations.set(fnv1a_32bit(anim.name), anim);

    return e_sprite;
}


void test_resources_init(registry* r) {
    en::texture_loader::create_setup_entity(r);


    g_link_texture = resource_get(r, "content/LinkMinishCapSprites.png");
    // camera creation and setup:
    g_camera = r->entity_make(en::camera::name);
    auto cam_cp = r->component_get<cp::camera>(g_camera, cp::camera::name);
    cam_cp->ortho_width = 4;

    g_link_sprite = create_link_sprite_and_animations(r, g_link_texture);
    

    g_link_sprite_renderer = r->entity_make(en::sprite_renderer::name);
    auto sr_cp = r->component_get<cp::sprite_renderer>(g_link_sprite_renderer, cp::sprite_renderer::name);
    sr_cp->set_sprite(g_link_sprite);
    sr_cp->play("walk_down");
}

void test_resources_deinit(registry* r) {
    //link_texture = resource(); // remove reference to the resource
}

void test_render(registry* r) {
  //  cp::texture* t = r->component_try_get<cp::texture>(g_link_texture, cp::texture::name);
    
    //auto link_sample_src_uv = rect_texture_uv_from_top_left_origin_px({ 12,6 }, vec2{ 24,32 } / 2.0f, { 1208, 3736 });
  //  DS_LOG(std::format("{}", link_sample_src_uv));
   // render_texture(ds::math::build_matrix({ 0,0 }), t->gpu_texid, { 1, 1 }, link_sample_src_uv);
}

int main() {
    app_config cfg;
    cfg.width = 1280;
    cfg.height = 720;


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



