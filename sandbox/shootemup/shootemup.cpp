#include <destral.h>


using namespace ds;


static ecs::registry* g_r = nullptr;


struct player {
    float x = 0;
    float y = 0;
    int health = 100;
    std::string name = "awesome player";
    void init(ecs::registry* r, ecs::entity e) { DS_LOG("player init call!"); }
    void deinit(ecs::registry* r, ecs::entity e) { DS_LOG("enemy deinit call!"); }
};

struct enemy {
    float x = 0;
    std::string enemy_name = "very bad enemy";
    void init(ecs::registry* r, ecs::entity e) { DS_LOG("enemy init call!"); }
    void deinit(ecs::registry* r, ecs::entity e) { DS_LOG("enemy deinit call!"); }
};


void init() {
    // Register components
    g_r = ecs::registry_create();
    DS_ECS_CP_REGISTER(g_r, player); // same ecs::cp_register<player>(g_r, "player");
    DS_ECS_CP_REGISTER(g_r, enemy); // the identifier of the component is "enemy"_hs 

    // Register the entities
    ecs::entity_register(g_r, "PlayerEntity", { "player"_hs });
    ecs::entity_register(g_r, "BadEnemyEntity", { "enemy"_hs });

    // Register the systems
    ecs::system_add(g_r, "UpdatePlayerSystem", { "player"_hs }, [](ecs::registry* r, ecs::view* v) {
        DS_LOG("UpdatePlayerSystem called.");

        auto player_cpidx = v->index("player"_hs);
        while (v->valid()) {
            //rd::draw_rect({ 0,0 }, 0.5f);
            
            player* p = v->data<player>(player_cpidx);
            DS_LOG(p->name.c_str());
            v->next();
        }
    });

    ecs::system_add(g_r, "UpdateEnemySystem", { "enemy"_hs }, [](ecs::registry* r, ecs::view* v) {
        DS_LOG("UpdateEnemySystem called.");
    });


    // Create one player
    ecs::entity_make(g_r, "PlayerEntity"_hs);
    
    // Create three enemies
    ecs::entity_make(g_r, "BadEnemyEntity"_hs);
    ecs::entity_make(g_r, "BadEnemyEntity"_hs);
    ecs::entity_make(g_r, "BadEnemyEntity"_hs);

}

void tick() {
    ecs::run_systems(g_r);
}

void deinit() {
    ecs::registry_destroy(g_r);
}


int main() {
	app::config cfg;
    cfg.width = 800;
    cfg.height = 800;
    cfg.on_init = init;
    cfg.on_tick = tick;
    cfg.on_shutdown = deinit;
    app::run(cfg);
  

//    sg_image img = { 0 };
//
////    ecs::registry* r;
//    cfg.on_init = [&]() {
//        img = rd::load_texture("zelda_gba_tileset.png");
//
//
//
//  //      r = ecs::registry_create();
//    };
//    
//    cfg.on_shutdown = [&]() {
////        ecs::registry_destroy(r);
//    };
//
//    static float rot = 0.01f;
//    static i32 mode = 0;
//
//    static float x = 0;
//    static float y = 0;
//
//
//    cfg.on_tick = [&]() {
//        rot += 0.001f;
//
//
//        if (in::is_key_triggered(in::Key::D) || in::is_key_triggered(in::Key::MouseRightButton)) {
//            mode = (mode + 1 ) % 2;
//        }
//        
//        if (in::is_key_pressed(in::Key::Gamepad_Left_Left) ) {
//            x -= 0.001f;
//        } else if (in::is_key_pressed(in::Key::Gamepad_Left_Right)) {
//            x += 0.001f;
//        }
//
//        if (in::is_key_pressed(in::Key::Gamepad_Right_Left)) {
//            x -= 0.001f;
//        } else if (in::is_key_pressed(in::Key::Gamepad_Right_Right)) {
//            x += 0.001f;
//        }
//
//        if (in::is_key_pressed(in::Key::Gamepad_Left_Up)) {
//            y += 0.001f;
//        } else if (in::is_key_pressed(in::Key::Gamepad_Left_Down)) {
//            y -= 0.001f;
//        }
//
//        if (in::is_key_pressed(in::Key::Gamepad_Right_Up)) {
//            y += 0.001f;
//        } else if (in::is_key_pressed(in::Key::Gamepad_Right_Down)) {
//            y -= 0.001f;
//        }
//
//
//        switch (mode) {
//        case 0:
//            rd::draw_circle({ x,y }, 0.5f, vec4(1, 0, 0, 1), 5);
//            rd::draw_rect(math::build_matrix({ 0.5, 0 }, rot), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
//            rd::draw_fill_rect(math::build_matrix({ -0.5, 0 }, rot), { 0.5, 0.5 }, vec4(1, 0, 0, 1), 3);
//            rd::draw_line({ { 0.5, 0.5 } , { 0.0, 0.5 }, {0, -0.5}, {-0.5, -0.5 } }, vec4(0, 0, 1, 1), 2);
//            rd::draw_line({ { 0.3, 0.2 } , { 0.2, 0.7 }, {0, -0.5}, {0.5, 0.5 } }, vec4(0, 1, 1, 1), 1);
//            rd::draw_texture(math::build_matrix({ 0, 0 }), img, { 1,1 }, rect::from_size({ 0,0 }, { 1, 1 }), vec4{ 1,1,1,1 }, 7);
//
//            
//            break;
//
//        case 1:
//            rd::draw_texture(math::build_matrix({ 0, 0 }), img, { 1,1 }, { { 0.5,0.5 }, { 1, 1 } }, vec4{ 1,1,1,1 }, 0);
//            break;
//        }
//        
//
//    };
//
//    cfg.on_render = []() {
//        //gfx::draw_quad({}, {});
//    };


    
}



