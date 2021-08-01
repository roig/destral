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

#include <destral/destral_ecs.h>
#include <destral/destral_base64.h>
#include <unordered_map>

namespace ds {
    /* Returns the type index part of the entity */
    static constexpr u32 entity_type_idx(entity e) { return e.type; }
    /* Makes a entity from an id, version and type_idx */
    static constexpr entity entity_assemble(u32 id, u32 version, u32 type_idx) { return entity{ .id = id, .version = version, .type = type_idx }; }

    std::string entity_to_string(entity e) {
        return std::format("Entity: ( id: {}  version: {}   type: {})", e.id, e.version, e.type);
    }

    ///* Pack the id and version to an entity handle */
    //static constexpr u64 entity_handle_pack(u32 id, u32 version) { return ((u64)id | (((u64)version) << 32)); }
    ///* Unpacks an entity handle to the id and version */
    //static constexpr void entity_handle_unpack(u64 handle, u32* id, u32* version) { 
    //    *id = handle & entity_max_id();
    //    *version = handle >> 32;
    //}



    struct ctx_variable_info {
        std::string name;
        u64 hashed_name = 0;
        void* instance_ptr = nullptr;
        delete_fn* deleter_fn = nullptr;
    };

    struct entity_type {
        // Entity name id
        std::string name;
        
        // Holds the component ids hashed using
        std::vector<u64> cp_ids;

        // Holds the component names ids (non hashed)
        std::vector<std::string> cp_names;

        // Entity init and deinit callbacks
        entity_init_fn* init_fn = nullptr;
        entity_deinit_fn* deinit_fn = nullptr;
    };

    struct registry {
        /* contains all the created entities */
        std::vector<entity> entities;

        /* first index in the list to recycle */
        u32 available_id = detail::entity_max_id();

        /* Hold the component storages */
        std::unordered_map<std::uint64_t, detail::cp_storage> cp_storages;

        /* Hold the registered entity types the key is the hashed string using fnv1a_64bit */
        std::unordered_map<std::uint64_t, entity_type> types;

        /* Holds the hashed key for the types map in a vector.
           Used to retrieve the hashed key from an the type index part of the entity */
        std::vector<std::uint64_t> entity_hashed_types;

        // hold the entities to be destroyed (delayed)
        std::vector<entity> entities_to_destroy;
        

        // Context variables maps/arrays (both have the same pointer) only vector makes calls the delete
        std::unordered_map<u64, ctx_variable_info*> ctx_vars;
        std::vector<ctx_variable_info*> ctx_vars_ordered;
    };

    /* Calls the deleter for each ctx variable set in inverse order of context variables registration. Then clears the maps/arrays of ctx vars */
    static void ctx_unset_all(registry* r) {
        dscheck(r);
        // call the deleter function first
        for (size_t i = 0; i < r->ctx_vars_ordered.size(); ++i) {
            auto ctx_var_info_ptr = r->ctx_vars_ordered[r->ctx_vars_ordered.size() - 1 - i];
            if (ctx_var_info_ptr->deleter_fn) {
                // delete the context variable instance!
                ctx_var_info_ptr->deleter_fn(ctx_var_info_ptr->instance_ptr);
            }
            // deletes the pointer to the context var information
            delete ctx_var_info_ptr;
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
        auto all = entity_all(r);
        for (auto i = 0; i < all.size(); i++) {
            entity_destroy(r, all[i]);
        }

        // delete all the context variables
        ctx_unset_all(r);
        delete r;
        r = nullptr;
    }

    // Performs the release of an entity in the registry by adding it to the recycle list
    static inline void s_release_entity(registry* r, entity e) {
        const u32 e_id = e.id;
        u32 new_version = e.version;
        ++new_version;

        // assemble an entity to be used for recycling
        r->entities[e_id] = entity_assemble(r->available_id, new_version, detail::entity_max_type_idx());
        r->available_id = e_id;
    }

    // Performs the creation process of a new entity of type_idx either by
    // recycling an entity id or by creating a new one if no available for recycling.
    static inline entity s_create_entity(registry* r, u32 type_idx) {
        dscheck(r);
        if (r->available_id == detail::entity_max_id()) {
            // Generate a new entity
            // check if we can't create more
            dsverifym(r->entities.size() < detail::entity_max_id(), std::format("Can't create more entities!"));
            const entity e = entity_assemble((u32)r->entities.size(), 0, type_idx);
            r->entities.push_back(e);
            return e;
        } else {
            // Recycle an entity
            dscheck(r->available_id != detail::entity_max_id());
            // get the first available entity id
            const u32 curr_id = r->available_id;
            const u32 curr_ver = r->entities[curr_id].version;
            // point the available_id to the "next" id
            r->available_id = r->entities[curr_id].id;
            // now join the id and version and type idx to create the new entity
            const entity recycled_e = entity_assemble(curr_id, curr_ver, type_idx);
            // assign it to the entities array
            r->entities[curr_id] = recycled_e;
            return recycled_e;
        }
    }

    /* Returns the storage pointer for the given cp id or nullptr if not exists */
    static detail::cp_storage* s_get_storage(registry* r, u64 cp_id) {
        dscheck(r);
        if (!r->cp_storages.contains(cp_id)) {
            return nullptr;
        }
        return &r->cp_storages[cp_id];
    }

    // Adds an entity type to the registry
    static void s_add_entity_type(registry* r, const entity_type& et) {
        dscheck(!et.name.empty());
        const auto entity_type_id = ds::fnv1a_64bit(et.name);
        dscheck(r->entity_hashed_types.size() < detail::entity_max_type_idx()); // no more types can be indexed..
        dscheck(!r->types.contains(entity_type_id)); // you are trying to register an existing entity type
        dscheckCode( // Check if all component ids exist
            for (auto& c : et.cp_names) { dscheckm(s_get_storage(r, ds::fnv1a_64bit(c)), std::format("Component: {} not found/registered!", c)); }
        );

        
        r->types.insert({ entity_type_id, et });
        r->entity_hashed_types.push_back(entity_type_id);
    }

    // Returns the type index (the one that goes in the entity) from a type_id (hashed from entity_name).
    static u32 s_get_entity_type_idx(registry* r, u64 type_id) {
        // Find the type index for that entity type id
        for (u32 i = 0; r->types.size(); i++) {
            if (r->entity_hashed_types[i] == type_id) {
                return i;
                break;
            }
        }
        DS_FATAL(std::format("Type id: {} Not found!", type_id));
    }

    // Returns the entity_type pointer from a valid entity
    static entity_type* s_get_entity_type(registry* r, entity e) {
        dscheck(r);
        dsverify(entity_valid(r, e));
        // first get the entity type index from entity e
        const auto etype_idx = entity_type_idx(e);
        dsverify(etype_idx < r->entity_hashed_types.size());
        const auto etype_hashed_id = r->entity_hashed_types[etype_idx];
        dsverifym(r->types.contains(etype_hashed_id), std::format("Entity type id: {} not found.", etype_hashed_id));
        entity_type* type = &r->types[etype_hashed_id];
        return type;
    }

    void entity_register(registry* r, const std::string& entity_name, const std::vector<std::string>& cp_names,
        entity_init_fn* init_fn, entity_deinit_fn* deinit_fn) {
        dscheck(r);

        entity_type et;
        et.name = entity_name;
        et.cp_names = cp_names;
        et.init_fn = init_fn;
        et.deinit_fn = deinit_fn;
        for (auto& c : cp_names) { et.cp_ids.push_back(fnv1a_64bit(c)); }

        s_add_entity_type(r, et);
    }

    entity entity_make_begin(registry* r, const std::string& entity_name) {
        dscheck(r);
        dscheck(!entity_name.empty());
        const u64 entity_type_id = ds::fnv1a_64bit(entity_name);
        dscheckm(r->types.contains(entity_type_id), std::format("Entity name: {} is not a registered one!", entity_name));


        // Find the type index for that entity name
        u32 type_idx = s_get_entity_type_idx(r, entity_type_id);
        
        // Create the entity
        entity e = s_create_entity(r, type_idx);

        // Emplace all the components
        entity_type* type = &r->types[entity_type_id];
        for (std::size_t i = 0; i < type->cp_ids.size(); ++i) {
            const u64 cp_id = type->cp_ids[i];
            auto st = s_get_storage(r, cp_id);
            dscheck(st);

            // 0 -> Emplace the component to the storage (only reserves memory) like a malloc
            void* cp_data = st->emplace(e);

            // 1 -> Placement new (defalut construct) on that memory
            // this will call default constructors for all the variables in the struct component
            if (st->placement_new_fn) {
                st->placement_new_fn(cp_data);
            }
        }
        return e;
    }

    void entity_make_end(registry* r, entity e) {
        entity_type* et = s_get_entity_type(r, e);
        // Call init function for the entity if available
        if (et->init_fn) {
            et->init_fn(r, e);
        }
    }


    // Process the full creation of an entity type id.
    // This includes the creation of all the components and their initialization in order
    entity entity_make(registry* r, const std::string& entity_name) {
        entity e = entity_make_begin(r, entity_name);
        entity_make_end(r, e);
        return e;
    }

    // Process the full destruction of an entity.
    // This includes the cleanup of all the components and their destruction in reverse order
    void entity_destroy(registry* r, entity e) {
        dscheck(r);
        // retrieve the entity type from entity
        const std::uint32_t type_idx = entity_type_idx(e);
        dscheck(type_idx < r->entity_hashed_types.size());
        const std::uint64_t hashed_type_id = r->entity_hashed_types[type_idx];
        dscheck(r->types.contains(hashed_type_id));
        entity_type* type = &r->types[hashed_type_id];

        // call deinit function for the entity before removing components
        if (type->deinit_fn) {
            type->deinit_fn(r, e);
        }

        // cleanup the cps in reverse order
        std::vector<std::uint64_t>& cps = type->cp_ids;
        for (size_t i = 0; i < cps.size(); ++i) {
            const size_t cp_id = cps[cps.size() - 1 - i];
            auto st = s_get_storage(r, cp_id);
            dscheck(st);

            void* cp_data = st->get(e);

            // 1 -> call destructor cp function
            if (st->delete_fn) {
                st->delete_fn(cp_data);
            }

            // 2 -> remove the cp from the cp storage
            st->remove(e);
        }
        // 3 -> release_entity with a desired new version
        s_release_entity(r, e);
    }

    void* entity_get(registry* r, entity e, const std::string& cp_name) {
        dscheck(r);
        dscheck(entity_valid(r, e));
        const auto cp_id = ds::fnv1a_64bit(cp_name);
        dscheck(s_get_storage(r, cp_id));
        return s_get_storage(r, cp_id)->get(e);
    }

    void* entity_try_get(registry* r, entity e, const std::string& cp_name) {
        dscheck(r);
        dscheck(entity_valid(r, e));
        const auto cp_id = ds::fnv1a_64bit(cp_name);
        dscheck(s_get_storage(r, cp_id));
        return s_get_storage(r, cp_id)->try_get(e);
    }

    bool entity_valid(registry* r, entity e) {
        dscheck(r);
        const u32 id = e.id;
        entity e1, e2;
        if (e1 == e2) {

        }

        return (id < r->entities.size()) && (r->entities[id] == e);
    }

    std::vector<entity> entity_all(registry* r) {
        dscheck(r);
        // If no entities are available to recycle, means that the full vector is valid
        if (r->available_id == detail::entity_max_id()) {
            return r->entities;
        } else {
            std::vector<entity> alive;
            for (u32 i = 0; i < r->entities.size(); ++i) {
                entity e = r->entities[i];
                
                // if the entity id is the same as the index, means its not a recycled one
                if (e.id == i) {
                    alive.push_back(e);
                }
            }
            return alive;
        }
    }

    void component_register(registry* r, const std::string& name_id, size_t cp_sizeof, placement_new_fn* placement_new_fn,
        delete_fn* del_fn, cp_serialize_fn* optional_serialize_fn) {
        dscheck(r);
        dscheck(!name_id.empty());
        const auto cp_id = ds::fnv1a_64bit(name_id);
        dscheckm(!r->cp_storages.contains(cp_id), "Trying to register a new component with a registered name."); 
        detail::cp_storage cp_storage;
        cp_storage.name = name_id;
        cp_storage.cp_id = cp_id;
        cp_storage.cp_sizeof = cp_sizeof;
        cp_storage.serialize_fn = optional_serialize_fn;
        cp_storage.placement_new_fn = placement_new_fn;
        cp_storage.delete_fn = del_fn;
        r->cp_storages.insert({ cp_id, cp_storage });
    }

    void entity_destroy_delayed(registry* r, entity e) {
        dscheck(r);
        r->entities_to_destroy.push_back(e);
    }

    bool entity_is_destroy_delayed(registry* r, entity e) {
        dscheck(r);
        for (auto i = 0; i < r->entities_to_destroy.size(); i++) {
            if (r->entities_to_destroy[i] == e) {
                return true;
            }
        }
        return false;
    }

    void entity_destroy_flush_delayed(registry* r) {
        dscheck(r);
        for (auto entity_to_destroy : r->entities_to_destroy) {
            if (entity_valid(r, entity_to_destroy)) 
            {
                entity_destroy(r, entity_to_destroy);
            }
        }
        r->entities_to_destroy.clear();
    }

    struct system {
        std::string name;
        syspool_update_fn* update_fn = nullptr;
    };

    struct syspool {
        std::vector<system> systems;
    };

    syspool* syspool_create() {
        return new syspool();
    }

    void syspool_destroy(syspool* s) {
        dscheck(s);
        delete s;
    }

    
    void syspool_add(syspool* s, const std::string& sys_name, syspool_update_fn* sys_update_fn) {
        dscheck(s);
        system sys;
        sys.name = sys_name;
        sys.update_fn = sys_update_fn;
        s->systems.push_back(sys);
    }

    // This runs all the registered systems with a registry and a delta
    void syspool_run(syspool* s, registry* r, float dt) {
        dscheck(s);
        dscheck(r);
        for (auto i = 0; i < s->systems.size(); i++) {
            if (s->systems[i].update_fn) {
                s->systems[i].update_fn(r, dt);
                entity_destroy_flush_delayed(r);
            }
        }
    }

    void* ctx_set(registry* r, const std::string& ctx_name_id, void* instance_ptr, delete_fn* del_fn) {
        dscheck(r);
        dscheck(!ctx_name_id.empty());
        dscheck(instance_ptr);
        const auto id = ds::fnv1a_64bit(ctx_name_id);
        dscheck(!r->ctx_vars.contains(id));

        ctx_variable_info* ctx_var = new ctx_variable_info();
        ctx_var->name = ctx_name_id;
        ctx_var->deleter_fn = del_fn;
        ctx_var->instance_ptr = instance_ptr;
        r->ctx_vars[id] = ctx_var;
        r->ctx_vars_ordered.push_back(ctx_var);
        return instance_ptr;
    }

    void ctx_unset(registry* r, const std::string& ctx_name_id) {
        dscheck(r);
        dscheck(!ctx_name_id.empty());
        const auto id = ds::fnv1a_64bit(ctx_name_id);
        
        // Find the element in the ordered variables vector
        bool found = false;
        std::size_t i;
        for (i = 0; i < r->ctx_vars_ordered.size(); i++) {
            if (r->ctx_vars_ordered[i]->hashed_name = id) {
                found = true;
            }
        }

        if (found) {
            // delete the ctx var instance
            auto var_info_ptr = r->ctx_vars_ordered[i];
            if (var_info_ptr->deleter_fn) {
                var_info_ptr->deleter_fn(var_info_ptr->instance_ptr);
            }
            // delete the info pointer
            delete var_info_ptr;
            // erase from the map
            r->ctx_vars.erase(id);
            
            // erase the element from the vector (this will mantain the order)
            r->ctx_vars_ordered.erase(r->ctx_vars_ordered.begin() + i);
        } else {
            DS_WARNING(std::format("Context variable name id: {} not found! When trying to delete it.", ctx_name_id));
        }
    }

    void* ctx_get(registry* r, const std::string& ctx_name_id) {
        dscheck(r);
        dscheck(!ctx_name_id.empty());
        auto id = ds::fnv1a_64bit(ctx_name_id);
        if (r->ctx_vars.contains(id)) {
            return r->ctx_vars[id]->instance_ptr;
        } else {
            return nullptr;
        }
    }

    view view_create(registry* r, const std::vector<std::string>& cp_ids) {
        view view;
        // Retrieve all the system storages for component ids for this system
        // and find the shorter storage (the one with less entities to iterate)
        view._impl.cp_storages.reserve(cp_ids.size());
        for (size_t cp_id_idx = 0; cp_id_idx < cp_ids.size(); ++cp_id_idx) {
            const auto cp_id = cp_ids[cp_id_idx];
            auto* cp_storage = s_get_storage(r, ds::fnv1a_64bit(cp_id));
            dscheckm(cp_storage, std::format("Component '{}' id not registered!", cp_id));

            // find the shorter storage to iterate it
            if (view._impl.iterating_storage == nullptr) {
                view._impl.iterating_storage = cp_storage;
            } else if (cp_storage->dense.size() < view._impl.iterating_storage->dense.size()) {
                view._impl.iterating_storage = cp_storage;
            }
            view._impl.cp_storages.push_back(cp_storage);
        }
        dscheck(view._impl.iterating_storage);

        // Set view to initial state
        view._impl.entity_index = 0;
        view._impl.entity_max_index = view._impl.iterating_storage->dense.size();
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
}




