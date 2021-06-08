#include <destral.h>


using namespace ds;


static ecs::registry* g_r = nullptr;


struct bullet {
    vec2 pos;
    float velocity = 5.f;
    void init(ecs::registry* r, ecs::entity e) { DS_LOG("bullet init call!"); }
    void deinit(ecs::registry* r, ecs::entity e) { DS_LOG("bullet deinit call!"); }
    static void update(ecs::registry* r, ecs::view* v) {


        auto bullet_cpidx = v->index("bullet"_hs);
        while (v->valid()) {
            bullet* p = v->data<bullet>(bullet_cpidx);

            p->pos.y = p->pos.y + (p->velocity * app::dt());
            rd::draw_circle(p->pos, 0.1f, vec4(1, 0, 1, 1), 4);
            v->next();
        }
    }

};

struct player {
    vec2 pos = { 0,-0.7f };
    float velocity = 2;

    std::string name = "awesome player";
    void init(ecs::registry* r, ecs::entity e) { DS_LOG("player init call!"); }
    void deinit(ecs::registry* r, ecs::entity e) { DS_LOG("enemy deinit call!"); }


    static void update(ecs::registry* r, ecs::view* v) {
        auto player_cpidx = v->index("player"_hs);
        while (v->valid()) {
            player* p = v->data<player>(player_cpidx);
           
            if (in::is_key_triggered(in::Key::Space)) {
                // Here I want to create an entity and set the position of the bullet entity to 
                auto ebullet = ecs::entity_make(g_r, "BulletEntity"_hs);
                bullet* cp_bullet = (bullet*)ecs::entity_get(g_r, ebullet, "bullet"_hs);
                cp_bullet->pos = p->pos;
            }


            float dir = 0.f;
            if (in::is_key_pressed(in::Key::Left) || in::is_key_pressed(in::Key::Gamepad_Left_Left)) {
                dir = -1.f;
            } else if (in::is_key_pressed(in::Key::Right) || in::is_key_pressed(in::Key::Gamepad_Left_Right)) {
                dir = 1.f;
            }

            p->pos.x = p->pos.x + (p->velocity * app::dt() * dir);
            rd::draw_rect(math::build_matrix(p->pos), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
            v->next();
        }
    }
   
};



struct enemy {
    float x = 0;
    std::string enemy_name = "very bad enemy";
    void init(ecs::registry* r, ecs::entity e) { DS_LOG("enemy init call!"); }
    void deinit(ecs::registry* r, ecs::entity e) { DS_LOG("enemy deinit call!"); }
    static void update(ecs::registry* r, ecs::view* v) {

    }
};


void init() {
    // Register components
    g_r = ecs::registry_create();
    DS_ECS_CP_REGISTER(g_r, player); // same ecs::cp_register<player>(g_r, "player");
    DS_ECS_CP_REGISTER(g_r, enemy); // the identifier of the component is "enemy"_hs 
    DS_ECS_CP_REGISTER(g_r, bullet);

    // Register the entities
    ecs::entity_register(g_r, "PlayerEntity", { "player"_hs });
    ecs::entity_register(g_r, "BadEnemyEntity", { "enemy"_hs });
    ecs::entity_register(g_r, "BulletEntity", { "bullet"_hs });

    // Register the systems
    ecs::system_add(g_r, "UpdatePlayerSystem", { "player"_hs }, player::update);
    ecs::system_add(g_r, "UpdateEnemySystem", { "enemy"_hs }, enemy::update);
    ecs::system_add(g_r, "UpdateEnemySystem", { "bullet"_hs }, bullet::update);


    // Create one player
    ecs::entity_make(g_r, "PlayerEntity"_hs);
    

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



