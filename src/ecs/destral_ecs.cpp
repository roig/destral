/**************** Implementation ****************/
/// IMPLEMENTATION DETAILS:
/*
    de_storage
    Here is the storage for each component type.

    ENTITIES:

    The main idea comes from ENTT C++ library:
    https://github.com/skypjack/entt
    https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#views
    (Credits to skypjack) for the awesome library.

    We have an sparse array that maps entity identifiers to the dense array indices that contains the full entity.

    sparse array:
    sparse => contains the index in the dense array of an entity identifier (without version)
    that means that the index of this array is the entity identifier (without version) and
    the content is the index of the dense array.

    dense array:
    dense => contains all the entities (entity).
    the index is just that, has no meaning here, it's referenced in the sparse.
    the content is the entity.

    this allows fast iteration on each entity using the dense array or
    lookup for an entity position in the dense using the sparse array.

    ---------- Example:
    Adding:
     entity = 3 => (e3)
     entity = 1 => (e1)

    In order to check the entities first in the sparse, we have to retrieve the uint32_t part of the entity.
    The uint32_t part will be used to index the sparse array.
    The full entity will be the value in the dense array.

                           0    1     2    3
    sparse idx:         eid0 eid1  eid2  eid3    this is the array index based on uint32_t (NO VERSION)
    sparse content:   [ null,   1, null,   0 ]   this is the array content. (index in the dense array)

    dense         idx:    0    1
    dense     content: [ e3,  e2]

    COMPONENT DATA:
    How the component data is stored?
    This is the easy part, the component data array is aligned with the dense array.
    This means that the entity from the index 0 of the dense array has the component data
    of the index 0 of the cp_data array.

    The packed component elements data is aligned always with the dense array from the sparse set.

    adding/removing an entity to the storage will:
        - add/remove from the sparse
        - use the sparse_set dense array position to move the components data aligned.

    Example:

                  idx:    0    1    2
    dense     content: [ e3,  e2,  e1]
    cp_data   content: [e3c, e2c, e1c] contains component data for the entity in the corresponding index

    If now we remove from the storage the entity e2:

                  idx:    0    1    2
    dense     content: [ e3,  e1]
    cp_data   content: [e3c, e1c] contains component data for the entity in the corresponding index

    note that the alignment to the index in the dense and in the cp_data is always preserved.

    This allows fast iteration for each component and having the entities accessible aswell.
    for (i = 0; i < dense_size; i++) {  // mental example, wrong syntax
        entity e = dense[i];
        void*   ecp = cp_data[i];
    }


*/

#include <destral/ecs/destral_ecs.h>
#include <destral/core/destral_base64.h>
#include <unordered_map>

namespace ds::ecs {

    struct context_variable {
        std::string name;
        void* instance_ptr = nullptr;
        delete_fn* deleter_fn = nullptr;
    };

    struct entity_type {
        std::string name;
        // Holds the component ids hashed using std::hash<std::string>
        std::vector<std::uint64_t> cp_ids;
    };

    struct system {
        std::string name;
        // Holds the component ids hashed using std::hash<std::string>
        std::vector<std::uint64_t> cp_ids;
        // Function that will be executed for that system when running systems.
        sys_update_fn* update_fn = nullptr;
    };

    struct registry {
        /* contains all the created entities */
        std::vector<entity> entities;

        /* first index in the list to recycle */
        uint32_t available_id = { entity_null };

        /* Hold the component storages */
        std::unordered_map<std::uint64_t, detail::cp_storage> cp_storages;

        /* Hold the registered systems (in order) */
        std::vector<system> systems;

        /* Hold the registered entity types the key is the hashed string using fnv1a_64bit */
        std::unordered_map<std::uint64_t, entity_type> types;
        /* Holds the hashed key for the types map in a vector.
           Used to retrieve the hashed key from an the type index part of the entity */
        std::vector<std::uint64_t> entity_hashed_types;

        // Context variables maps/arrays (both have the same pointer) only vector makes calls the delete
        std::unordered_map<std::uint64_t, context_variable*> ctx_vars;
        std::vector<context_variable*> ctx_vars_ordered;
    };

    /* Calls the deleter for each ctx variable set in inverse order of context variables registration. Then clears the maps/arrays of ctx vars */
    static void ctx_unset_all(registry* r) {
        dscheck(r);
        // call the deleter function first
        for (size_t i = 0; i < r->ctx_vars_ordered.size(); ++i) {
            auto ctx_var = r->ctx_vars_ordered[r->ctx_vars_ordered.size() - 1 - i];
            if (ctx_var->deleter_fn) {
                ctx_var->deleter_fn(ctx_var->instance_ptr);
            }
        }
        r->ctx_vars.clear();
        r->ctx_vars_ordered.clear();
    }

    registry* registry_create() {
        registry* r = new registry();
        return r;
    }

    void registry_destroy(registry* r) {
        dscheck(r);
        // delete all the entities...
        // TODO (FIX) DANI important!!

        // delete all the context variables
        ctx_unset_all(r);
        delete r;
    }

    static inline void s_release_entity(registry* r, entity e, uint32_t desired_version) {
        const uint32_t e_id = entity_id(e);
        r->entities[e_id] = entity_assemble(r->available_id, desired_version, 0);
        r->available_id = e_id;
    }

    static inline entity s_create_enitty(registry* r, uint32_t type_idx) {
        dscheck(r);
        if (r->available_id == entity_null) {
            // Generate a new entity
            dscheck(r->entities.size() < entity_max_id()); // can't create more entity identifiers
            const entity e = entity_assemble((uint32_t)r->entities.size(), 0, type_idx);
            r->entities.push_back(e);
            return e;
        } else {
            // Recycle an entity
            dscheck(r->available_id != entity_null);
            // get the first available entity id
            const uint32_t curr_id = r->available_id;
            const uint32_t curr_ver = entity_version(r->entities[curr_id]);
            // point the available_id to the "next" id
            r->available_id = entity_id(r->entities[curr_id]);
            // now join the id and version and type idx to create the new entity
            const entity recycled_e = entity_assemble(curr_id, curr_ver, type_idx);
            // assign it to the entities array
            r->entities[curr_id] = recycled_e;
            return recycled_e;
        }
    }

    /* Returns the storage pointer for the given cp id or nullptr if not exists */
    static detail::cp_storage* s_get_storage(registry* r, std::uint64_t cp_id) {
        dscheck(r);
        if (!r->cp_storages.contains(cp_id)) {
            return nullptr;
        }
        return &r->cp_storages[cp_id];
    }

    void entity_register(registry* r, const char* entity_name_id, std::vector<std::uint64_t> cps) {
        const auto type_id = ds::detail::fnv1a_64bit(entity_name_id);
        dscheck(r);
        dscheck(entity_name_id);
        dscheck(r->entity_hashed_types.size() < entity_max_type_idx()); // no more types can be indexed..
        dscheck(!r->types.contains(type_id)); // you are trying to register an existing entity type
        dscheckCode( // Check if all component ids exist
            for (auto c : cps) {  dscheckm(s_get_storage(r, c), "Component id not found/registered!");  }
        );

        entity_type et;
        et.name = entity_name_id;
        et.cp_ids = cps;
        r->types.insert({ type_id, et });
        r->entity_hashed_types.push_back(type_id);
    }

    entity entity_make(registry* r, std::uint64_t e_id) {
        dscheck(r);
        dscheck(r->types.contains(e_id));
        uint32_t type_idx = 0xFFFFFFFF;
        for (uint32_t i = 0; r->types.size(); i++) {
            if (r->entity_hashed_types[i] == e_id) {
                type_idx = i;
                break;
            }
        }
        // create the entity
        entity e = s_create_enitty(r, type_idx);
        // and emplace all the components
        entity_type* type = &r->types[e_id];
        for (size_t i = 0; i < type->cp_ids.size(); ++i) {
            const auto cp_id = type->cp_ids[i];
            auto st = s_get_storage(r, cp_id);
            dscheck(st);

            // 0 -> Emplace the component to the storage (only reserves memory) like a malloc
            void* cp_data = st->emplace(e);

            // 1 -> Placement new (defalut construct) on that memory
            // this will call default constructors for all the variables in the struct
            if (st->placement_new_fn) {
                st->placement_new_fn(cp_data);
            }

            // 2 -> Initialize
            // This is the user initialize function for the component
            if (st->serialize_fn) {
                st->serialize_fn(r, e, cp_data);
            }
        }
        // FIX (dani) call entity begin play
        return e;
    }

    void entity_delete(registry* r, entity e) {
        dscheck(r);
        // retrieve the entity type from entity
        const std::uint32_t type_idx = entity_type_idx(e);
        dscheck(type_idx < r->entity_hashed_types.size());
        const std::uint64_t hashed_type_id = r->entity_hashed_types[type_idx];
        dscheck(r->types.contains(hashed_type_id));
        // cleanup the cps in inverse order
        std::vector<std::uint64_t>& cps = r->types[hashed_type_id].cp_ids;
        for (size_t i = 0; i < cps.size(); ++i) {
            const size_t cp_id = cps[cps.size() - 1 - i];
            auto st = s_get_storage(r, cp_id);
            dscheck(st);

            void* cp_data = st->get(e);

            // 1 -> call cleanup cp function
            if (st->cleanup_fn) {
                st->cleanup_fn(r, e, cp_data);
            }

            // 2 -> call destructor cp function
            if (st->delete_fn) {
                st->delete_fn(cp_data);
            }

            // 3 -> remove it from the cp storage
            st->remove(e);
        }
        // 4 -> release_entity with a desired new version
        uint32_t new_version = entity_version(e);
        ++new_version;
        s_release_entity(r, e, new_version);
    }

    void* entity_get(registry* r, entity e, const char* cp) {
        dscheck(r);
        dscheck(entity_valid(r, e));
        dscheck(s_get_storage(r, ds::detail::fnv1a_64bit(cp)));
        return s_get_storage(r, ds::detail::fnv1a_64bit(cp))->get(e);
    }

    void* entity_try_get(registry* r, entity e, const char* cp) {
        dscheck(r);
        dscheck(entity_valid(r, e));
        dscheck(s_get_storage(r, ds::detail::fnv1a_64bit(cp)));
        return s_get_storage(r, ds::detail::fnv1a_64bit(cp))->try_get(e);
    }

    bool entity_valid(registry* r, entity e) {
        dscheck(r);
        const uint32_t id = entity_id(e);
        return (id < r->entities.size()) && (r->entities[id] == e);
    }

    void cp_register(registry* r, const char* name_id, size_t cp_sizeof,
        placement_new_fn* placement_new_fn, delete_fn* del_fn,
        cp_serialize_fn* serialize_fn, cp_cleanup_fn* cleanup_fn) {
        dscheck(r);
        dscheck(name_id);
        const auto cp_id = ds::detail::fnv1a_64bit(name_id);
        dscheck(!r->cp_storages.contains(cp_id)); // you are trying to register a component with the same name/id
        detail::cp_storage cp_storage;
        //cp_storage.name = name;
        cp_storage.cp_id = cp_id;
        cp_storage.cp_sizeof = cp_sizeof;
        cp_storage.serialize_fn = serialize_fn;
        cp_storage.cleanup_fn = cleanup_fn;
        cp_storage.placement_new_fn = placement_new_fn;
        cp_storage.delete_fn = del_fn;
        r->cp_storages.insert({ cp_id, cp_storage });
    }

    void system_add(registry* r, const char* name_id, const std::vector<std::uint64_t>& cp_ids, sys_update_fn* update_fn = nullptr) {
        dscheck(name_id);
        // check if all the component ids are valid
        for (size_t cp_id_idx = 0; cp_id_idx < cp_ids.size(); ++cp_id_idx) {
            dscheck(s_get_storage(r, cp_ids[cp_id_idx])); // no component storage for this cp_id!!!
        }
        system s;
        s.name = name_id;
        s.update_fn = update_fn;
        s.cp_ids = cp_ids;
        r->systems.push_back(s);
    }

    view view_create(registry* r, const std::vector<std::uint64_t>& cp_ids) {
        view view;
        // Retrieve all the system storages for component ids for this system
        // and find the shorter storage (the one with less entities to iterate)
        view._impl.cp_storages.reserve(cp_ids.size());
        for (size_t cp_id_idx = 0; cp_id_idx < cp_ids.size(); ++cp_id_idx) {
            const auto cp_id = cp_ids[cp_id_idx];
            auto* cp_storage = s_get_storage(r, cp_id);
            dscheck(cp_storage); // no component storage for this cp_id!!!

            // find the shorter storage to iterate it
            if (view._impl.iterating_storage == nullptr) {
                view._impl.iterating_storage = cp_storage;
            } else if (cp_storage->dense.size() < view._impl.iterating_storage->dense.size()) {
                view._impl.iterating_storage = cp_storage;
            }
            view._impl.cp_storages.push_back(cp_storage);
        }
        dscheck(view._impl.iterating_storage);
        // Set view to default invalid state
        view._impl.entity_index = 0;
        view._impl.cur_entity = entity_null;
        // Now find the first valid entity in the iterating dense array that is contained in all the storages
        for (size_t i = 0; i < view._impl.iterating_storage->dense.size(); ++i) {
            view._impl.entity_index = i;
            const auto e = view._impl.iterating_storage->dense[i];
            if (e != entity_null) {
                if (view._impl.is_entity_in_all_storages(e)) {
                    view._impl.cur_entity = view._impl.iterating_storage->dense[i];
                    break;
                }
            }
        }

        return view;
    }

    void run_systems(registry* r) {
        dscheck(r);
        for (size_t i = 0; i < r->systems.size(); ++i) {
            system* sys = &r->systems[i];
            // if no component ids for this system or no update function set skip it
            if ((sys->cp_ids.size() == 0) || (sys->update_fn == nullptr)) {
                continue;
            }
            auto view = view_create(r, sys->cp_ids);
            sys->update_fn(r, &view);
        }
    }

    void* ctx_register(registry* r, const char* name, void* ptr, delete_fn* del_fn) {
        dscheck(r);
        dscheck(ptr);
        const auto id = ds::detail::fnv1a_64bit(name);
        dscheck(!r->ctx_vars.contains(id));

        context_variable* ctx_var = new context_variable();
        ctx_var->name = name;
        ctx_var->deleter_fn = del_fn;
        ctx_var->instance_ptr = ptr;
        r->ctx_vars[id] = ctx_var;
        r->ctx_vars_ordered.push_back(ctx_var);

        return ptr;
    }

    void* ctx_get(registry* r, std::uint64_t id) {
        dscheck(r);
        if (r->ctx_vars.contains(id)) {
            return r->ctx_vars[id]->instance_ptr;
        } else {
            return nullptr;
        }
    }
}




