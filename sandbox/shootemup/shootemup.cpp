#include <destral.h>

using namespace ds;
void resources_test() {

    cache_images_add_info("Zelda", "content/zelda.png");
    cache_images_add_info("Zelda", "content/zelda.png");
    cache_images_add_info("Zelda", "content/zelda.png");
    cache_images_add_info("Zelda", "content/zelda.png");

    resource<image> t = cache_images()->get("Zelda");
    resource<int> it;

}




using namespace ds;


static registry* g_r = nullptr;
static syspool* g_syspool = nullptr;

struct bullet {
    vec2 pos;
    float velocity = 1.f;
    float timetolive = 1.0f;
};
void bullet_init(registry* r, entity e) { DS_LOG("bullet init call!"); }
void bullet_deinit(registry* r, entity e) { DS_LOG("bullet deinit call!"); }
void bullet_update(registry* r, float dt) {
   // DS_LOG("Start Bullet Update ---------");
    

   
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



    if (key_is_pressed(key::Delete)) {
        // Here I want to create an entity and set the position of the bullet entity to 

        auto all = g_r->entity_all();
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
    g_r = new registry();
    DS_ECS_COMPONENT_REGISTER(g_r, player);
    DS_ECS_COMPONENT_REGISTER(g_r, bullet);

    // Register the entities
    entity_definition player_def = { .name = "PlayerEntity" , .cp_names = {"player"} };
    g_r->entity_register(&player_def);
    entity_definition bullet_def = { .name = "BulletEntity" , .cp_names = {"bullet"} };
    g_r->entity_register(&bullet_def);

    // Register the systems
    g_syspool = syspool_create();
    syspool_add(g_syspool, "UpdatePlayerSystem", player_update);
    syspool_add(g_syspool, "UpdateBulletSystem", bullet_update);
    
    // Create one player
    g_r->entity_make("PlayerEntity");
}

void tick(float dt) {
    syspool_run(g_syspool, g_r, dt);
}

void fixed_tick(float dt) {

}


void deinit() {
    syspool_destroy(g_syspool);
    delete g_r;
}


int main() {
    app_config cfg;
    cfg.width = 800;
    cfg.height = 800;
    cfg.on_init = init;
    cfg.on_tick = tick;
    cfg.on_shutdown = deinit;
    app_run(cfg);
}



