#include <destral.h>

using namespace ds;


struct bullet {
    vec2 pos;
    float velocity = 1.f;
    float timetolive = 1.0f;
};
void bullet_init(registry* r, entity e) { DS_LOG("bullet init call!"); }
void bullet_deinit(registry* r, entity e) { DS_LOG("bullet deinit call!"); }
void bullet_update(registry* r, float dt) {
    view v = r->view_create({ "bullet" });
    while (v.valid()) {
        bullet* p = v.data<bullet>(v.index("bullet"));
        p->pos.y = p->pos.y + (p->velocity * dt);
        p->timetolive -= dt;
        draw_circle(p->pos, 0.1f, vec4(1, 0, 1, 1), 4);
        //DS_LOG(fmt::format("entity: {}   ttl: {}", v->entity(), p->timetolive));
        //if (p->timetolive < 0.0) {
        //   // ecs::entity_destroy_deferred(r, v->entity());
        //}
        //if (in::is_key_triggered(Space)) {
        //    // Here I want to create an entity and set the position of the bullet entity to 
        //    ecs::entity_make(g_r, "BulletEntity");
        //    //bullet* cp_bullet = (bullet*)ecs::entity_try_get(g_r, ebullet, "bullet");
        //    /*cp_bullet->pos = p->pos;*/
        //}
        v.next();
    }
    
  //  DS_LOG("End Bullet Update ----------");



    if (key_is_triggered(key::Delete)) {
        // Here I want to create an entity and set the position of the bullet entity to 

        auto all = app_registry()->entity_all();
        if (!all.empty()) {
            auto etodestroy = all[0];

            r->entity_destroy(etodestroy);
        }

        //bullet* cp_bullet = (bullet*)ecs::entity_try_get(g_r, ebullet, "bullet");
        /*cp_bullet->pos = p->pos;*/
    }

}



struct player {
    vec2 pos = { 0,-0.7f };
    float velocity = 2;
    std::string name = "awesome player";
};

void player_init(registry* r, entity e) { DS_LOG("player init call!"); }
void player_deinit(registry* r, entity e) { DS_LOG("player deinit call!"); }
void player_update(registry* r, float dt) {
    view vobj = r->view_create({ "player" });
    view* v = &vobj;
    auto player_cpidx = v->index("player");
    while (v->valid()) {
        player* p = v->data<player>(player_cpidx);
        if (key_is_triggered(key::Space)) {
            // Here I want to create an entity and set the position of the bullet entity to 
            auto ebullet = r->entity_make_begin("BulletEntity");
            bullet* cp_bullet = (bullet*)r->cp_try_get(ebullet, "bullet");
            cp_bullet->pos = p->pos;
            r->entity_make_end(ebullet);
        }

        float dir = 0.f;
        if (key_is_pressed(key::Left) || key_is_pressed(key::Gamepad_Left_Left)) {
            dir = -1.f;
        } else if (key_is_pressed(key::Right) || key_is_pressed(key::Gamepad_Left_Right)) {
            dir = 1.f;
        }

        p->pos.x = p->pos.x + (p->velocity * dt * dir);
        draw_rect(math::build_matrix(p->pos), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
        v->next();
    }
}

void init() {
    // Register components
    auto g_r = app_registry();
    DS_ECS_COMPONENT_REGISTER(g_r, player);
    DS_ECS_COMPONENT_REGISTER(g_r, bullet);

    // Register the entities
    g_r->entity_register({ .name = "PlayerEntity" , .cp_names = {"player"}, .init_fn = player_init, .deinit_fn = player_deinit });
    g_r->entity_register({ .name = "BulletEntity" , .cp_names = {"bullet"}, .init_fn = bullet_init, .deinit_fn = bullet_deinit });

    // Register the systems
    g_r->system_queue_add("FixedTickQueue", "UpdatePlayerSystem", player_update);
    g_r->system_queue_add("FixedTickQueue", "UpdateBulletSystem", bullet_update);
   
    // Create one player
    g_r->entity_make("PlayerEntity");
}

void tick(float dt) {
    app_registry()->system_queue_run("NormalTickQueue", dt);
}

void fixed_tick(float dt) {
    app_registry()->system_queue_run("FixedTickQueue", dt);
}


void deinit() {

}


int main() {
    app_config cfg;
    cfg.width = 800;
    cfg.height = 800;
    cfg.on_init = init;
    cfg.on_tick = tick;
    cfg.on_fixed_tick = fixed_tick;
    cfg.on_shutdown = deinit;
    app_run(cfg);
}



