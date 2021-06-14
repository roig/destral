#include <destral.h>


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
    

   
    view v = view_create(r, { "bullet" });
    while (v.valid()) {
        bullet* p = v.data<bullet>(v.index("bullet"));
        p->pos.y = p->pos.y + (p->velocity * dt);
        p->timetolive -= dt;
        rd::draw_circle(p->pos, 0.1f, vec4(1, 0, 1, 1), 4);
        //DS_LOG(fmt::format("entity: {}   ttl: {}", v->entity(), p->timetolive));
        //if (p->timetolive < 0.0) {
        //   // ecs::entity_destroy_deferred(r, v->entity());
        //}
        //if (in::is_key_triggered(in::Key::Space)) {
        //    // Here I want to create an entity and set the position of the bullet entity to 
        //    ecs::entity_make(g_r, "BulletEntity");
        //    //bullet* cp_bullet = (bullet*)ecs::entity_try_get(g_r, ebullet, "bullet");
        //    /*cp_bullet->pos = p->pos;*/
        //}
        v.next();
    }
    
  //  DS_LOG("End Bullet Update ----------");



    if (in::is_key_triggered(in::Key::Delete)) {
        // Here I want to create an entity and set the position of the bullet entity to 

        auto all = entity_all(r);
        if (!all.empty()) {
            auto etodestroy = all.at(0);

            entity_destroy(g_r, etodestroy);
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
    
    view vobj = view_create(r, { "player" });
    view* v = &vobj;
    auto player_cpidx = v->index("player");
    while (v->valid()) {
        player* p = v->data<player>(player_cpidx);

        if (in::is_key_triggered(in::Key::Space)) {
            // Here I want to create an entity and set the position of the bullet entity to 
            auto ebullet = entity_make_begin(g_r, "BulletEntity");
            bullet* cp_bullet = (bullet*)entity_try_get(g_r, ebullet, "bullet");
            cp_bullet->pos = p->pos;
            entity_make_end(r, ebullet);
        }

        float dir = 0.f;
        if (in::is_key_pressed(in::Key::Left) || in::is_key_pressed(in::Key::Gamepad_Left_Left)) {
            dir = -1.f;
        } else if (in::is_key_pressed(in::Key::Right) || in::is_key_pressed(in::Key::Gamepad_Left_Right)) {
            dir = 1.f;
        }

        p->pos.x = p->pos.x + (p->velocity * dt * dir);
        rd::draw_rect(math::build_matrix(p->pos), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
        v->next();
    }
}

void init() {
    // Register components
    g_r = registry_create();
    DS_ECS_COMPONENT_REGISTER(g_r, player);
    DS_ECS_COMPONENT_REGISTER(g_r, bullet);

    // Register the entities
    entity_register(g_r, "PlayerEntity", { "player" });
    entity_register(g_r, "BulletEntity", { "bullet" });

    // Register the systems
    g_syspool = syspool_create();
    syspool_add(g_syspool, "UpdatePlayerSystem", player_update);
    syspool_add(g_syspool, "UpdateBulletSystem", bullet_update);
    
    // Create one player
    entity_make(g_r, "PlayerEntity");
}

void tick(float dt) {
    syspool_run(g_syspool, g_r, dt);
}

void fixed_tick(float dt) {

}


void deinit() {
    syspool_destroy(g_syspool);
    registry_destroy(g_r);
}


int main() {
	app::config cfg;
    cfg.width = 800;
    cfg.height = 800;
    cfg.on_init = init;
    cfg.on_tick = tick;
    cfg.on_shutdown = deinit;
    app::run(cfg);
}



