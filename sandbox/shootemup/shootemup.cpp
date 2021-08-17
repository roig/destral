#include <destral.h>

using namespace ds;


struct bullet {
    static constexpr const char* cp_name = "BulletComponent";
    static constexpr const char* e_name = "BulletEntity";
    vec2 pos;
    float velocity = 1.f;
    float timetolive = 1.0f;
    static void fixed_update(registry* r) {
        view v = r->view_create({ cp_name });
        while (v.valid()) {
            bullet* p = v.data<bullet>(v.index(cp_name));
            p->pos.y = p->pos.y + (p->velocity * app_dt());
            p->timetolive -= app_dt();
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

    static void render(registry* r) {
        view v = r->view_create({ cp_name });
        while (v.valid()) {
            bullet* p = v.data<bullet>(v.index(cp_name));
            draw_circle(p->pos, 0.1f, vec4(1, 0, 1, 1), 4);
            v.next();
        }
    }

};





struct player {
    static constexpr const char* cp_name = "PlayerComponent";
    static constexpr const char* e_name = "PlayerEntity";
    vec2 pos = { 0,-0.7f };
    float velocity = 2;
    std::string name = "awesome player";

    static void update(registry* r) {
        view v = r->view_create({ cp_name });
        auto player_cpidx = v.index(cp_name);
        while (v.valid()) {
            player* p = v.data<player>(player_cpidx);
            if (key_is_triggered(key::Space)) {
                // Here I want to create an entity and set the position of the bullet entity to 
                auto ebullet = r->entity_make_begin(bullet::e_name);
                bullet* cp_bullet = r->cp_try_get<bullet>(ebullet, bullet::cp_name);
                cp_bullet->pos = p->pos;
                r->entity_make_end(ebullet);
            }
            v.next();
        }
    }

    static void fixed_update(registry* r) {
        view v = r->view_create({ cp_name });

        auto player_cpidx = v.index(cp_name);
        while (v.valid()) {
            player* p = v.data<player>(player_cpidx);
            float dir = 0.f;
            if (key_is_pressed(key::Left) || key_is_pressed(key::Gamepad_Left_Left)) {
                dir = -1.f;
            } else if (key_is_pressed(key::Right) || key_is_pressed(key::Gamepad_Left_Right)) {
                dir = 1.f;
            }

            p->pos.x = p->pos.x + (p->velocity * app_dt() * dir);
            draw_rect(math::build_matrix(p->pos), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
            v.next();
        }
    }

    static void render(registry* r) {
        view v = r->view_create({ cp_name });
        auto player_cpidx = v.index(cp_name);
        while (v.valid()) {
            player* p = v.data<player>(player_cpidx);
            draw_rect(math::build_matrix(p->pos), { 0.5, 0.5 }, vec4(0, 1, 0, 1), 4);
            v.next();
        }
    }
};




struct enemy {
    static constexpr const char* cp_name = "EnemyComponent";
    static constexpr const char* e_name = "EnemyEntity";
    vec2 pos = { 0,0 };
    float velocity = 0.5f;

    static void fixed_update(registry* r) {
        view v = r->view_create({ cp_name });
        while (v.valid()) {
            auto* en = v.data<enemy>(v.index(cp_name));
            en->pos.y = en->pos.y - (en->velocity * app_dt());
            v.next();
        }
    }

    static void render(registry* r) {
        view v = r->view_create({ cp_name });
        while (v.valid()) {
            auto* en = v.data<enemy>(v.index(cp_name));
            draw_circle(en->pos, 0.05f, vec4(1, 0, 0, 1), 4);
            v.next();
        }
    }

};



struct enemy_spawner {
    static constexpr const char* cp_name = "EnemySpawnerComponent";
    static constexpr const char* e_name = "EnemySpawnerEntity";
    float spawn_delay = 2.0;
    float time_acc = 0;
    static void update(registry* r) {
        view v = r->view_create({ cp_name });

        while (v.valid()) {
            auto* es = v.data<enemy_spawner>(v.index(cp_name));
            es->time_acc += app_dt();
            while (es->time_acc >= es->spawn_delay) {
                es->time_acc -= es->spawn_delay;
                es->time_acc = glm::max(es->time_acc, 0.0f);
                r->entity_make(enemy::e_name);
            }
            v.next();
        }
    }
};

void ecs_config(registry* r) {
    game_framework_register_entities(r); // this registers camera and hierarchy components

    // Now register game ecs:
    // Register game components
    r->cp_register<player>(player::cp_name);
    r->cp_register<bullet>(bullet::cp_name);
    r->cp_register<enemy>(enemy::cp_name);
    r->cp_register<enemy_spawner>(enemy_spawner::cp_name);

    // Register game entities
    r->entity_register({ .name = player::e_name, .cp_names = {player::cp_name} });
    r->entity_register({ .name = bullet::e_name, .cp_names = {bullet::cp_name} });
    r->entity_register({ .name = enemy::e_name , .cp_names = {enemy::cp_name} });
    r->entity_register({ .name = enemy_spawner::e_name, .cp_names = {enemy_spawner::cp_name} });

    // Register game systems
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::update, player::update);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::update, enemy_spawner::update);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::fixed_update, player::fixed_update);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::fixed_update, bullet::fixed_update);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::fixed_update, enemy::fixed_update);

    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::render, player::render);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::render, bullet::render);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::render, enemy::render);
    DS_ECS_QUEUE_ADD_SYSTEM(r, queue::render, camera::render_cameras_system);

    // Spawn game entities
    r->entity_make(player::e_name);
    r->entity_make(enemy_spawner::e_name);
    r->entity_make(camera::e_name);
}

int main() {
    app_config cfg;
    cfg.width = 1280;
    cfg.height = 720;
    cfg.on_ecs_config = ecs_config;
    app_run(cfg);
}



