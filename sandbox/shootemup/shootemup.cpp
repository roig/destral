#include <destral.h>

using namespace ds;

struct handler_registry {
    i64 create_or_recycle_handler() {
        if (available_handler_id == 0) {
            // Verify if we can create more handlers
            dsverifym(handlers.size() < (size_t)handler_max_id(), "Can't create more handlers!");

            // Generate a new handler
            const u64 version = 1;
            const i32 id = (i32)handlers.size() + 1;
            const i64 handler = handler_assemble(id, version);
            handlers.push_back(handler);
            DS_LOG(std::format("Entity created: {} (new id)", stringify_handler(handler)));
            return handler;
        } else {
            // Recycle an entity
            // get the first available entity id and the version
            const i64 h = s_get_handler_by_id(available_handler_id);
            const i32 id = available_handler_id;
            const u64 ver = get_handler_version(h);
            // point the available_handler_id to the "next" id
            available_handler_id = get_handler_id(h);

            // create and set the new handler
            const i64 handler = handler_assemble(id, ver);
            s_set_handler(handler);
            DS_LOG(std::format("Entity created: {} (recicled id)", stringify_handler(handler)));
            return handler;
        }
    }

    void release_handler(i64 h) {
        i32 id = get_handler_id(h);
        u64 version = get_handler_version(h);

        if (version == handler_max_version()) {
            // This entity can't be used anymore
            // set the version to 0 for this handler
            DS_TRACE(std::format("Entity released: {} (id invalidated by max version)", stringify_handler(h)));
            s_set_handler(handler_assemble(id, 0));
        } else {
            // Increment the version of that entity and add the entity to the recycle list
            DS_TRACE(std::format("Entity released: {}", stringify_handler(h)));
            ++version;
            const i64 handler_recycle = handler_assemble(available_handler_id, version);
            handlers[id - 1] = handler_recycle;
            available_handler_id = id;
        }
    }

    bool is_valid_entity_id(i64 h) {
        const u64 version = get_handler_version(h);
        const i32 id = get_handler_id(h);
        if (version == 0 || id == 0) { return false; }
        if ((size_t)id <= handlers.size() && (s_get_handler_by_id(id) == h)) { return true; }
        return false;
    }

    std::string stringify_handler(i64 h) {
        return std::format("( id: {}  version: {} )", get_handler_id(h), get_handler_version(h));
    }

    inline i32 get_handler_id(i64 h) { return (i32)(h & 0x7FFFFFFF); }


    i32 handler_max_id() { return INT32_MAX; 
    };
    u64 handler_max_version() { 
       return UINT64_MAX >> 31; 
    }
    u64 get_handler_version(i64 h) { return ((u64)h >> 31); }
    i64 handler_assemble(i32 id, u64 v) {
        i64 hid = ((u64)(id) & (u64)0x7FFFFFFF);
        i64 hv = (v << 31);
        i64 h = 0;//  h = (i64)(((u64)(id) & (u64)0xEFFFFFFF)) | ((u64)(v) << (u64)(31));
        h = hid | hv;

        return h;
    }

    std::vector<i64> handlers;
    /* first index in the list to recycle */
    i32 available_handler_id = 0; // 0 means no handler available for index


private:
    // Unsafe access
    inline i64 s_get_handler_by_id(i32 id) {
        dscheck(id >= 0);
        return handlers[id - 1]; 
    }
    // Unsafe access
    inline void s_set_handler(i64 h) { 
        const i32 id = get_handler_id(h);
        dscheck(id != 0); 
        handlers[id - 1] = h; 
    }
};

struct handle_registry_tester {
    static constexpr const char* cp_name = "HRTesterCP";
    static constexpr const char* e_name = "HRTester";
    handler_registry hr;
    darray<i64> entities;
    static void update_manual_insert_and_release(registry* r) {
        view v = r->view_create({ cp_name });
        while (v.valid()) {
            handle_registry_tester* p = v.data<handle_registry_tester>(v.index(cp_name));

            if (key_is_triggered(key::Insert)) {
                auto e = p->hr.create_or_recycle_handler();
                p->entities.push_back(e);
            }

            if (key_is_triggered(key::Delete)) {
                if (!p->entities.empty()) {
                    auto e = p->entities.back();
                    p->hr.release_handler(e);
                    p->entities.pop_back();
                }
            }

            v.next();
        }
    }
};

void handle_registry_add_test_system(registry* r) {
    r->component_register<handle_registry_tester>(handle_registry_tester::cp_name);

    // Register game entities
    r->entity_register(handle_registry_tester::e_name, {handle_registry_tester::cp_name});

    // Register game systems
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::update, handle_registry_tester::update_manual_insert_and_release);
}





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
                bullet* cp_bullet = r->component_get<bullet>(ebullet, bullet::cp_name);
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

void testInputUpdate(registry* r) {
    bool isconnected = ds::gamepad_is_connected(0);
    DS_LOG(std::format("Gamepad connected state: {}",isconnected));
}

void testMapContainer() {
    ds::dmap<std::string> map;
    map.set(0,"0");
    map.set(1,"1");
    map.set(2,"2");
    map.set(3,"3");
    map.set(4,"4");

    DS_LOG(std::format("Key 0 should be 0 -> {}",map[0]));
    DS_LOG(std::format("Key 1 should be 1 -> {}",map[1]));
    DS_LOG(std::format("Key 2 should be 2 -> {}",map[2]));
    DS_LOG(std::format("Key 3 should be 3 -> {}",map[3]));
    map.set(2,"22");
    DS_LOG(std::format("Key 2 should be 22 -> {}",map[2]));

    map.remove("3");
    DS_LOG(std::format("Key 3 should be false -> {}",map.is_valid_key(3)));

    map.remove(4);
    DS_LOG(std::format("Key 4 should be false -> {}",map.is_valid_key(3)));
}

struct ecs_test_spawned {
    static constexpr const char* cp_name = "ECSTesterEntityCP";
    static constexpr const char* e_name = "ECSTesterEntity";
};

struct ecs_tester {
    static constexpr const char* cp_name = "ECSTesterCP";
    static constexpr const char* e_name = "ECSTester";
    static void test_add_remove_iterate(registry* r) {

    }
    darray<entity> entities;
    static void update_manual_insert_and_release(registry* r) {
        view v = r->view_create({ cp_name });
        while (v.valid()) {
            auto* p = v.data<ecs_tester>(v.index(cp_name));

            if (key_is_triggered(key::Insert)) {
                p->entities.push_back(r->entity_make(ecs_test_spawned::e_name));
            }

            if (key_is_triggered(key::Delete)) {
                if (!p->entities.empty()) {
                    auto e = p->entities.back();
                    r->entity_destroy_delayed(e);
                    p->entities.pop_back();
                }
            }

            v.next();
        }
    }
};

void ecs_test_register(registry* r) {
    r->component_register<ecs_tester>(ecs_tester::cp_name);
    r->component_register<ecs_test_spawned>(ecs_test_spawned::cp_name);

    r->entity_register(ecs_tester::e_name, { ecs_tester::cp_name });
    r->entity_register(ecs_test_spawned::e_name, { ecs_test_spawned::cp_name });

    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::update, ecs_tester::update_manual_insert_and_release);
    r->entity_make(ecs_tester::e_name);
}

void ecs_config(registry* r) {
    //ecs_test_register(r);

    game_framework_register_entities(r); // this registers camera and hierarchy components

    // Now register game ecs:
    // Register game components
    r->component_register<player>(player::cp_name);
    r->component_register<bullet>(bullet::cp_name);
    r->component_register<enemy>(enemy::cp_name);
    r->component_register<enemy_spawner>(enemy_spawner::cp_name);

    // Register game entities
    r->entity_register(player::e_name, {player::cp_name});
    r->entity_register(bullet::e_name, {bullet::cp_name});
    r->entity_register(enemy::e_name, {enemy::cp_name});
    r->entity_register(enemy_spawner::e_name, {enemy_spawner::cp_name});

    // Register game systems
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::update, player::update);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::update, enemy_spawner::update);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::fixed_update, player::fixed_update);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::fixed_update, bullet::fixed_update);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::fixed_update, enemy::fixed_update);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r,queue::update,testInputUpdate);

    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::render, player::render);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::render, bullet::render);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::render, enemy::render);
    DS_REGISTRY_QUEUE_ADD_SYSTEM(r, queue::render, camera::render_cameras_system);

    //handle_registry_add_test_system(r);
    //r->entity_make(handle_registry_tester::e_name);
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

    testMapContainer();
    app_run(cfg);
}



