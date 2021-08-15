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


    

    struct cp_storage {
        cp_definition cd;
        u64 cp_id = 0; /* component id for this storage */

        /*  packed component elements array. aligned with dense */
        ds::darray<u8> cp_data;
        /*  Dense entities array.
            - index is linked with the sparse value.
            - value is the full entity
        */
        ds::darray<entity> dense;

        /*  sparse entity identifiers indices array.
            - index is the id of the entity. (without version and type_idx)
            - value is the index of the dense array (uint32_t)

            (Note) This can be refactored to an std::unordered_map<uint32_t, uint32_t>
            to reduce memory footprint but at the cost of access time.
        */
        ds::darray<i32> sparse;

        inline bool contains(entity e) {
            dscheck(!e.is_null());
            const i32 eid = e.id;
            return (eid < sparse.size()) && (sparse[eid] != entity::max_id());
        }

        void debug_arrays() {
            DS_LOG("Sparse:");
            for (auto i = 0; i < sparse.size(); i++) {
                DS_LOG(std::format("[{}] => {}", i, sparse[i]));
            }

            DS_LOG("Dense:");
            for (auto i = 0; i < dense.size(); i++) {
                DS_LOG(std::format("[{}] => {}", i, dense[i].to_string()));
            }

            DS_LOG(std::format("Components vector: (cp_size: {}  bytesize: {}  cp_sizeof: {}",
                cp_data.size() / (float)cd.cp_sizeof, cp_data.size(), cd.cp_sizeof));
        }

        inline void* emplace(entity e) {
            dscheck(!e.is_null());
            // now allocate the data for the new component at the end of the array and memset to 0
            cp_data.resize(cp_data.size() + cd.cp_sizeof, 0);

            // return the component data pointer (last position of the component sizes)
            void* cp_data_ptr = &cp_data[cp_data.size() - cd.cp_sizeof];

            // Then add the entity to the sparse/dense arrays
            const i32 eid = e.id;
            DS_LOG(std::format("Adding {}", e.to_string()));
            if (eid >= sparse.size()) { // check if we need to realloc
                sparse.resize(eid + (i32)1, entity::max_id()); // default to entity_maxid means that is not valid.
            }

            sparse[eid] = dense.size();
            dense.push_back(e);
            // debug_arrays();
            return cp_data_ptr;
        }

        inline void remove(entity e) {
            dscheck(contains(e));
            DS_LOG(std::format("Removing {}", e.to_string()));

            // Remove from sparse/dense arrays
            const i32 pos_to_remove = sparse[e.id];
            entity other = dense.back();
            sparse[other.id] = pos_to_remove;
            dense[pos_to_remove] = other;
            sparse[e.id] = entity::max_id();
            dense.pop_back();
            // swap to the last position (memmove because if cp_data_size 1 it will overlap dst and source.
            memmove(
                &(cp_data)[pos_to_remove * cd.cp_sizeof],
                &(cp_data)[(cp_data.size() - cd.cp_sizeof)],
                cd.cp_sizeof);

            // and pop the last one
            dscheck(cp_data.size() >= cd.cp_sizeof);
            cp_data.resize(cp_data.size() - cd.cp_sizeof, 0);

            //  debug_arrays();
        }

        inline void* get_by_index(i32 index) {
            dscheck((index * cd.cp_sizeof) < cp_data.size());
            return &(cp_data)[index * cd.cp_sizeof];
        }

        inline void* get(entity e) {
            dscheck(!e.is_null());
            dscheck(contains(e));
            return get_by_index(sparse[e.id]);
        }

        inline void* try_get(entity e) {
            dscheck(!e.is_null());
            return contains(e) ? get(e) : nullptr;
        }
    };
    
    const entity entity::null = entity();

    std::string entity::to_string() {
        return std::format("Entity: ( id: {}  version: {}   type: {})", id, version, type_id);
    }

    struct ctx_variable_info {
        std::string name;
        u64 hashed_name = 0;
        void* instance_ptr = nullptr;
        void (*deleter_fn)(void*) = nullptr;
    };

    struct entity_type {
        // Entity name id
        std::string name;
        
        // Holds the component ids hashed using
        ds::darray<u64> cp_ids;

        // Holds the component names ids (non hashed)
        ds::darray<std::string> cp_names;

        // Entity init and deinit callbacks
        
        entity_definition::entity_init_fn* init_fn = nullptr;
        entity_definition::entity_deinit_fn* deinit_fn = nullptr;
    };

    struct system_type {
        std::string name;
        registry::system_update_fn* update_fn = nullptr;
    };

    struct registry_impl {
        /* contains all the created entities */
        ds::darray<entity> entities;

        /* first index in the list to recycle */
        i32 available_id = entity::max_id();

        /* Hold the component storages */
        std::unordered_map<std::uint64_t, cp_storage> cp_storages;

        /* Hold the registered entity types the key is the hashed string using fnv1a_64bit */
        std::unordered_map<std::uint64_t, entity_type> types;

        /* Holds the hashed key for the types map in a vector.
           Used to retrieve the hashed key from an the type index part of the entity */
        ds::darray<std::uint64_t> entity_hashed_types;

        // hold the entities to be destroyed (delayed)
        ds::darray<entity> entities_to_destroy;
        

        // Context variables maps/arrays (both have the same pointer) only vector makes calls the delete
        std::unordered_map<u64, ctx_variable_info*> ctx_vars;
        ds::darray<ctx_variable_info*> ctx_vars_ordered;


        // Systems
        std::unordered_map<std::string, ds::darray<system_type>> system_queues;
    };

    /* Calls the deleter for each ctx variable set in inverse order of context variables registration. Then clears the maps/arrays of ctx vars */
    static void ctx_unset_all(registry* r) {
        dscheck(r);
        // call the deleter function first
        for (i32 i = 0; i < r->_r->ctx_vars_ordered.size(); ++i) {
            auto ctx_var_info_ptr = r->_r->ctx_vars_ordered[r->_r->ctx_vars_ordered.size() - 1 - i];
            if (ctx_var_info_ptr->deleter_fn) {
                // delete the context variable instance!
                ctx_var_info_ptr->deleter_fn(ctx_var_info_ptr->instance_ptr);
            }
            // deletes the pointer to the context var information
            delete ctx_var_info_ptr;
        }
        r->_r->ctx_vars.clear();
        r->_r->ctx_vars_ordered.clear();
    }

    registry::registry() { _r = new registry_impl(); }
    registry::~registry() { 
       
        // delete all the entities...
        auto all = entity_all();
        for (auto i = 0; i < all.size(); i++) {
            entity_destroy(all[i]);
        }

        // delete all the context variables
        ctx_unset_all(this);
        delete _r;
        _r = nullptr;
    }

    // Performs the release of an entity in the registry by adding it to the recycle list
    static inline void s_release_entity(registry_impl* r, entity e) {
        i32 new_version = e.version;

        // Wrap around the version
        if (new_version == e.max_version()) {
            new_version = 0;
        } else {
            ++new_version;
        }

        // assemble an entity to be used for recycling
        const i32 e_id = e.id;
        r->entities[e_id] = { .id = r->available_id, .version = new_version, .type_id = entity::max_type_id() };
        r->available_id = e_id;
    }

    // Performs the creation process of a new entity of type_idx either by
    // recycling an entity id or by creating a new one if no available for recycling.
    static inline entity s_create_entity(registry* r, i32 type_idx) {
        dscheck(r);
        if (r->_r->available_id == entity::max_id()) {
            // Generate a new entity
            // check if we can't create more
            dsverifym(r->_r->entities.size() < entity::max_id(), std::format("Can't create more entities!"));
            const entity e = { .id = r->_r->entities.size(), .version = 0, .type_id = type_idx };
            r->_r->entities.push_back(e);
            return e;
        } else {
            // Recycle an entity
            dscheck(r->_r->available_id != entity::max_id());
            // get the first available entity id
            const i32 curr_id = r->_r->available_id;
            const i32 curr_ver = r->_r->entities[curr_id].version;
            // point the available_id to the "next" id
            r->_r->available_id = r->_r->entities[curr_id].id;
            // now join the id and version and type idx to create the new entity
            const entity recycled_e = { .id = curr_id, .version = curr_ver, .type_id = type_idx };
            // assign it to the entities array
            r->_r->entities[curr_id] = recycled_e;
            return recycled_e;
        }
    }

    /* Returns the storage pointer for the given cp id or nullptr if not exists */
    static cp_storage* s_get_storage(registry* r, u64 cp_id) {
        dscheck(r);
        if (!r->_r->cp_storages.contains(cp_id)) {
            return nullptr;
        }
        return &r->_r->cp_storages[cp_id];
    }

    // Adds an entity type to the registry
    static void s_add_entity_type(registry* r, const entity_type& et) {
        dsverify(!et.name.empty());
        const auto entity_type_id = ds::fnv1a_64bit(et.name);
        dsverify(r->_r->entity_hashed_types.size() < entity::max_type_id()); // no more types can be indexed..
        dsverify(!r->_r->types.contains(entity_type_id)); // you are trying to register an existing entity type
        dscheckCode( // Check if all component ids exist
            for (i32 i = 0; i < et.cp_names.size(); i++) {
                dscheckm(s_get_storage(r, ds::fnv1a_64bit(et.cp_names[i])), std::format("Component: {} not found/registered!", et.cp_names[i])); 
            }
        );

        
        r->_r->types.insert({ entity_type_id, et });
        r->_r->entity_hashed_types.push_back(entity_type_id);
    }

    // Returns the type index (the one that goes in the entity) from a type_id (hashed from entity_name).
    static i32 s_get_entity_type_idx(registry* r, u64 type_id) {
        // Find the type index for that entity type id
        for (i32 i = 0; r->_r->types.size(); i++) {
            if (r->_r->entity_hashed_types[i] == type_id) {
                return i;
                break;
            }
        }
        DS_FATAL(std::format("Type id: {} Not found!", type_id));
    }

    // Returns the entity_type pointer from a valid entity
    static entity_type* s_get_entity_type(registry* r, entity e) {
        dscheck(r);
        dsverify(r->entity_valid(e));
        // first get the entity type index from entity e
        const auto etype_idx = e.type_id;
        dsverify(etype_idx < r->_r->entity_hashed_types.size());
        const auto etype_hashed_id = r->_r->entity_hashed_types[etype_idx];
        dsverifym(r->_r->types.contains(etype_hashed_id), std::format("Entity type id: {} not found.", etype_hashed_id));
        entity_type* type = &r->_r->types[etype_hashed_id];
        return type;
    }

    void registry::entity_register(const entity_definition& edef) {

        entity_type et;

        et.name = edef.name;
        et.cp_names = edef.cp_names;
        et.init_fn = edef.init_fn;
        et.deinit_fn = edef.deinit_fn;
        for (i32 i = 0; i < et.cp_names.size(); i++) {
             et.cp_ids.push_back(fnv1a_64bit(et.cp_names[i])); 
        }

        s_add_entity_type(this, et);
    }

    entity registry::entity_make_begin(const char* entity_name) {
        dscheck(entity_name != nullptr);
        const u64 entity_type_id = ds::fnv1a_64bit(entity_name);
        dscheckm(_r->types.contains(entity_type_id), std::format("Entity name: {} is not a registered one!", entity_name));


        // Find the type index for that entity name
        i32 type_idx = s_get_entity_type_idx(this, entity_type_id);
        
        // Create the entity
        entity e = s_create_entity(this, type_idx);

        // Emplace all the components
        entity_type* type = &_r->types[entity_type_id];
        for (i32 i = 0; i < type->cp_ids.size(); ++i) {
            const u64 cp_id = type->cp_ids[i];
            auto st = s_get_storage(this, cp_id);
            dscheck(st);

            // 0 -> Emplace the component to the storage (only reserves memory) like a malloc
            void* cp_data = st->emplace(e);

            // 1 -> Placement new (defalut construct) on that memory
            // this will call default constructors for all the variables in the struct component
            if (st->cd.placement_new_fn) {
                st->cd.placement_new_fn(cp_data);
            }
        }
        return e;
    }

    void registry::entity_make_end(entity e) {
        entity_type* et = s_get_entity_type(this, e);
        // Call init function for the entity if available
        if (et->init_fn) {
            et->init_fn(this, e);
        }
    }


    // Process the full creation of an entity type id.
    // This includes the creation of all the components and their initialization in order
    entity registry::entity_make(const char* entity_name) {
        entity e = entity_make_begin(entity_name);
        entity_make_end(e);
        return e;
    }

    // Process the full destruction of an entity.
    // This includes the cleanup of all the components and their destruction in reverse order
    void registry::entity_destroy(entity e) {
        // retrieve the entity type from entity
        const i32 type_idx = e.type_id;
        dscheck(type_idx < _r->entity_hashed_types.size());
        const i64 hashed_type_id = _r->entity_hashed_types[type_idx];
        dscheck(_r->types.contains(hashed_type_id));
        entity_type* type = &_r->types[hashed_type_id];

        // call deinit function for the entity before removing components
        if (type->deinit_fn) {
            type->deinit_fn(this, e);
        }

        // cleanup the cps in reverse order
        ds::darray<u64>& cps = type->cp_ids;
        for (i32 i = 0; i < cps.size(); ++i) {
            const u64 cp_id = cps[cps.size() - 1 - i];
            auto st = s_get_storage(this, cp_id);
            dscheck(st);

            void* cp_data = st->get(e);

            // 1 -> call destructor cp function
            if (st->cd.delete_fn) {
                st->cd.delete_fn(cp_data);
            }

            // 2 -> remove the cp from the cp storage
            st->remove(e);
        }
        // 3 -> release_entity with a desired new version
        s_release_entity(_r, e);
    }

    void* registry::cp_get(entity e, const char* cp_name) {
        
        dscheck(entity_valid(e));
        const auto cp_id = ds::fnv1a_64bit(cp_name);
        dscheck(s_get_storage(this, cp_id));
        return s_get_storage(this, cp_id)->get(e);
    }

    void* registry::cp_try_get( entity e, const char* cp_name) {
        dscheck(entity_valid(e));
        const auto cp_id = ds::fnv1a_64bit(cp_name);
        dscheck(s_get_storage(this, cp_id));
        return s_get_storage(this, cp_id)->try_get(e);
    }

    bool registry::entity_valid(entity e) {
        return (e.id < _r->entities.size() ) && ( _r->entities[e.id].equal(e) );
    }

    ds::darray<entity> registry::entity_all() {
        // If no entities are available to recycle, means that the full vector is valid
        if (_r->available_id == entity::max_id()) {
            return _r->entities;
        } else {
            ds::darray<entity> alive;
            for (i32 i = 0; i < _r->entities.size(); ++i) {
                entity e = _r->entities[i];
                
                // if the entity id is the same as the index, means its not a recycled one
                if (e.id == i) {
                    alive.push_back(e);
                }
            }
            return alive;
        }
    }

    void registry::cp_register(const cp_definition& cd) {
        dscheck(!cd.name.empty());
        const auto cp_id = ds::fnv1a_64bit(cd.name);
        dsverifym(!_r->cp_storages.contains(cp_id), "Trying to register a new component with a registered name."); 
        cp_storage cp_st;
        cp_st.cd = cd;
        cp_st.cp_id = cp_id;
        _r->cp_storages.insert({ cp_id, cp_st });
    }

    void registry::entity_destroy_delayed(entity e) {
        _r->entities_to_destroy.push_back(e);
    }

    bool registry::entity_is_destroy_delayed(entity e) {
        for (auto i = 0; i < _r->entities_to_destroy.size(); i++) {
            if (_r->entities_to_destroy[i].equal(e)) {
                return true;
            }
        }
        return false;
    }

    void registry::entity_destroy_flush_delayed() {
        for (i32 i = 0; i < _r->entities_to_destroy.size(); i++) {
            if (entity_valid(_r->entities_to_destroy[i])) {
                entity_destroy(_r->entities_to_destroy[i]);
            }
        }
        _r->entities_to_destroy.clear();
    }

    void registry::system_queue_add(const char* queue_name, const char* sys_name, system_update_fn* sys_update_fn) {
        dscheck(queue_name);
        dscheck(sys_name);
        _r->system_queues[queue_name].push_back(system_type(sys_name, sys_update_fn));
    }

    
    void registry::system_queue_run(const char* queue_name, float dt) {
        dscheck(queue_name);
        if (_r->system_queues.contains(queue_name)) {
            auto& sys_list = _r->system_queues[queue_name];
            for (i32 i = 0; i < sys_list.size(); i++) {
                if (sys_list[i].update_fn) {
                    sys_list[i].update_fn(this, dt);
                    entity_destroy_flush_delayed();
                }
            }
        }
    }

    void* registry::ctx_set(const char* ctx_name_id, void* instance_ptr, void (*del_fn)(void* ptr)) {
        dscheck(ctx_name_id != nullptr);
        dscheck(instance_ptr);
        const auto id = ds::fnv1a_64bit(ctx_name_id);
        dscheck(!_r->ctx_vars.contains(id));

        ctx_variable_info* ctx_var = new ctx_variable_info();
        ctx_var->name = ctx_name_id;
        ctx_var->deleter_fn = del_fn;
        ctx_var->instance_ptr = instance_ptr;
        _r->ctx_vars[id] = ctx_var;
        _r->ctx_vars_ordered.push_back(ctx_var);
        return instance_ptr;
    }

    void registry::ctx_unset(const char* ctx_name_id) {
        dscheck(ctx_name_id != nullptr);
        const auto id = ds::fnv1a_64bit(ctx_name_id);
        
        // Find the element in the ordered variables vector
        bool found = false;
        i32 i;
        for (i = 0; i < _r->ctx_vars_ordered.size(); i++) {
            if (_r->ctx_vars_ordered[i]->hashed_name = id) {
                found = true;
            }
        }

        if (found) {
            // delete the ctx var instance
            auto var_info_ptr = _r->ctx_vars_ordered[i];
            if (var_info_ptr->deleter_fn) {
                var_info_ptr->deleter_fn(var_info_ptr->instance_ptr);
            }
            // delete the info pointer
            delete var_info_ptr;
            // erase from the map
            _r->ctx_vars.erase(id);
            
            // erase the element from the vector (this will mantain the order)
            _r->ctx_vars_ordered.remove_at(i);
        } else {
            DS_WARNING(std::format("Context variable name id: {} not found! When trying to delete it.", ctx_name_id));
        }
    }

    void* registry::ctx_get(const char* ctx_name_id) {
        dscheck(ctx_name_id != nullptr);
        auto id = ds::fnv1a_64bit(ctx_name_id);
        if (_r->ctx_vars.contains(id)) {
            return _r->ctx_vars[id]->instance_ptr;
        } else {
            return nullptr;
        }
    }




    //--------------------------------------------------------------------------------------------------
    // Views 

    // Returns true if the entity e is in all storages
    static bool s_view_is_entity_in_all_storages(view* v, ds::entity e) {
        for (i32 st_id = 0; st_id < v->_impl.cp_storages.size(); ++st_id) {
            if (!v->_impl.cp_storages[st_id]->contains(e)) {
                return false;
            }
        }
        return true;
    }

    view registry::view_create(const ds::darray<const char*>& cp_ids) {
        view view;
        // Retrieve all the system storages for component ids for this system
        // and find the shorter storage (the one with less entities to iterate)
        for (i32 cp_id_idx = 0; cp_id_idx < cp_ids.size(); ++cp_id_idx) {
            auto cp_id = cp_ids[cp_id_idx];
            auto* cp_storage = s_get_storage(this, ds::fnv1a_64bit(cp_id));
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
        view._impl.cur_entity = entity::null;

        // Now find the first valid entity in the iterating dense array that is contained in all the storages
        for (i32 i = 0; i < view._impl.iterating_storage->dense.size(); ++i) {
            view._impl.entity_index = i;
            auto e = view._impl.iterating_storage->dense[i];
            if (!e.is_null()) {
                if (s_view_is_entity_in_all_storages(&view, e)) {
                    view._impl.cur_entity = view._impl.iterating_storage->dense[i];
                    break;
                }
            }
        }

        return view;
    }

    // Returns the component index associated with a component id (use data function to retrieve the data)
    i32 view::index(const char* cp_name) {
        const u64 cp_id_hashed = fnv1a_64bit(cp_name);
        for (i32 i = 0; i < _impl.cp_storages.size(); i++) {
            if (_impl.cp_storages[i]->cp_id == cp_id_hashed) { return i; }
        }
        // error, no component with that cp_id in this view!
        dscheckm(false, "component id not found in this view!");
        return 0;
    }

    // Returns the raw component data pointer associated with the component index for this view (see index function)
    void* view::raw_data(i32 cp_idx) {
        dscheck(valid());
        dscheck(cp_idx >= 0);
        dscheck(cp_idx < _impl.cp_storages.size());
        return _impl.cp_storages[cp_idx]->get(_impl.cur_entity);
    }

    // Advances the next entity that has all the components for the view
    void view::next() {
        dscheck(valid());
        // find the next contained entity that is inside all pools
        bool entity_contained = false;
        do {
            if (_impl.entity_index < _impl.entity_max_index - 1) {
                // select next entity from the iterating storage (smaller one..)
                ++_impl.entity_index;
                _impl.cur_entity = _impl.iterating_storage->dense[_impl.entity_index];
                // now check if the entity is contained in ALL other storages:
                entity_contained = s_view_is_entity_in_all_storages(this, _impl.cur_entity);
            } else {
                _impl.cur_entity = entity::null;
            }
        } while ((!_impl.cur_entity.is_null()) && !entity_contained);
    }

   


}




