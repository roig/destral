#pragma once

/*
    destral_ecs.h

    FIX (dani) TODO proper instructions

    TODO: Update documentation..



    ////////// Registry information.

    * The registry holds the registered components, entities and systems that are always registered at the start.
    * After you register a component or an entity or a system, you can't unregister them. 
    

    ////////// Components

    Components are structs that you can register into the registry and assign them a type name to acces them later.
    
    The registry provides an special template to ease the registration process:   ecs::cp_register<T>

    This is an example of registration of two components:

    struct player {
        float x = 0;
        float y = 0;
        int health = 100;
        std::string name = "awesome player";
        void init(ecs::registry* r, ecs::entity e) { }
        void deinit(ecs::registry* r, ecs::entity e) { }
    };
    
    struct enemy {
        float x = 0;
        std::string enemy_name = "very bad enemy";
        void init(ecs::registry* r, ecs::entity e) { }
        void deinit(ecs::registry* r, ecs::entity e) { }
    };


    registry *g_rs = ecs::registry_create();
    DS_ECS_CP_REGISTER(g_r, player); // same ecs::cp_register<player>(g_r, "player");
    DS_ECS_CP_REGISTER(g_r, enemy);
    
    IMPORTANT: After this you can identify each component with the name "player" and "enemy".

    Things to remember, the functions init and deinit ARE MANDATORY they are always executed AFTER creating an
    entity that has those components. This allows initialization and deinitialization at entity level. The entity
    that owns this component is passed in the init and deinit second argument.


    ////////// Entities

    Entities are type identifiers that are composed by components.
    The registry allows the registration of entities with a number of components using the ecs::entity_register() function.
    

    

    1) register the components



    

    ////////// Construction of an entity:
    0 -> create the entity in the registry

    for each component (order specified on registration) {
        1 -> construct (placement new) the component 
        2 -> serialize/initialize component (default initialize) (This function should not rely on other components!)
    }

    
    --- now all the components are constructed and default initialized for that entity ---
    3 (TODO) -> call user initialize function (this should be a callback provided in entity_make)
        The purpose of this function is to allow to initialize some variables on a per instance or to be used in the step 4. 
        It´s like a custom constructor before the real entity constructor in step 4.
    
    4 (TODO) -> call initialize entity function. (begin_play) (this should be a callback set when registering a new entity type)
        this is like a constructor by parameter for that entity, because it can read the config variables set by step 3
        and configure all the components needed based on those configuration variables. It can be null of course.
        (This function can call other entity instantiation functions!) BUT! if you do it it will probably invalidate component
        pointers you saved, so retrieve them again after instantiation.

    
    Example:

    struct pos2d {
        int x = 1;
        int y = 2;
        static void placement_new() {placement_new()...}
        static void serialize() { read from key value pairs..  }
        // no initialize needed here
    };

    struct player {
        std::string name = "Unknown";
        int startingx = 0;
        static void placement_new() {placement_new()...}
        static void serialize() { read from key value pairs..  }

        // this function is not from the component... its for the entity..
        static void initialize() {
            get_cp(pos2d)->x = startingx;
        }

        
    };

    register_cp(pos2d, pos2d::serialize);
    register_cp(player, player::serialize);


    register_entity_type("Text2D", {{"pos2d", "text_rd"}}, player::initialize );

    ///////// Destruction of an entity:
    0 -> Call (TODO) (end_play) function registered for the entity. (
    for each component (inverse order specified on registration) {
        1 -> call on_cleanup function component if any. (This function MUST not rely on other components!, CAN'T)
        2 -> call destructor of the component if any.
        3 ->
    }
    3 -> Destroy the entity from the registry.
    

*/
#include <destral/core/destral_common.h>
#include <vector>



namespace ds::ecs {
//--------------------------------------------------------------------------------------------------
// Configuration

/* Entity opaque 64 bit identifier 
    16 bits      | 16 bits   | 32 bits
    [FFFF        | FFFF      | FFFF FFFF]
    [65535       | 65535     | 4294967295]
    Version       TypeIndex   Id
*/
using entity = std::uint64_t;
static constexpr std::uint64_t entity_max_id() { return 0xFFFFFFFF; }
static constexpr std::uint64_t entity_max_version() { return 0xFFFF; }
static constexpr std::uint64_t entity_max_type_idx() { return 0xFFFF; }
/* Returns the id part of the entity */
static constexpr uint32_t entity_id(entity e) { return e & 0xFFFFFFFF; }
/* Returns the version part of the entity */
static constexpr uint32_t entity_version(entity e) { return e >> 48; }
/* Returns the type index part of the entity */
static constexpr uint32_t entity_type_idx(entity e) { return (e << 16) >> 48; }
/* Makes a entity from an id, version and type_idx */
static constexpr entity entity_assemble(uint32_t id, uint32_t version, uint32_t type_idx) { return (entity(id) | (entity(type_idx) << 32)) | (entity(version) << 48); }
/* The entity_null is a entity that represents a null entity. */
static constexpr entity entity_null = entity_max_id();

//--------------------------------------------------------------------------------------------------
// Registry 
// Global context that holds each storage for each component types and the entities.
struct registry;
registry* registry_create(); /*  Allocates and initializes a registry context */
void registry_destroy(registry* r); /*  Deinitializes and frees a registry context */


//--------------------------------------------------------------------------------------------------
// Component Functions
#define DS_ECS_COMPONENT_REGISTER(r,T) component_register<T>(r, #T) 

typedef void (cp_serialize_fn)(registry* r, entity e, void* cp); // TODO not used yet
typedef void (placement_new_fn)(void* ptr);
typedef void (delete_fn)(void* ptr);
void component_register(registry* r, const std::string& name_id, size_t cp_sizeof, placement_new_fn* placement_new_fn = nullptr,
    delete_fn* del_fn = nullptr, cp_serialize_fn* optional_serialize_fn = nullptr);

template <typename T> void component_register(registry* r, const std::string& name_id) {
    auto constructor_fn = [](void* mem_cp) { new (mem_cp) T(); }; // placement new
    auto destructor_fn = [](void* mem_cp) { ((T*)mem_cp)->~T(); }; // destructor
    //TODO auto serialize_fn = [](registry* r, entity e, void* cp) { ((T*)mem_cp)->serialize(r, e); };
    component_register(r, name_id, sizeof(T), constructor_fn, destructor_fn, nullptr );
}

//--------------------------------------------------------------------------------------------------
// Entity Functions

typedef void (entity_init_fn)(registry* r, entity e);
typedef void (entity_deinit_fn)(registry* r, entity e);
// Registers an entity name with it's components names. 
void entity_register(registry* r, const std::string& entity_name, const std::vector<std::string>& cp_names, entity_init_fn* init_fn = nullptr, entity_deinit_fn* deinit_fn = nullptr);

// Instantiates an entity and calls the initialization function on it if it exists
// This will call internally entity_make_begin and then entity_make_end
// This will call the init function registered for the entity if any after the creation of all the components
// You can call this function during view iterations. The new created entity will not be iterated in the current view.
entity entity_make(registry* r, const std::string& entity_name);

// Instantiates an entity WITHOUT calling the initialization function of the entity
// This allows you to setup the parameters of the entity components before calling the init function. (like a constructor by params)
// IMPORTANT:
// Undefined Behaviour if you don't call entity_make_end on the entity returned by this function in order to have a fully constructed/initialized entity
entity entity_make_begin(registry* r, const std::string& entity_name);

// Finishes the instantiation of an entity created by entity_make_begin
// IMPORTANT: 
// Undefined behaviour if used on an entity not created using the entity_make_begin.
// Undefined behaviour if called multiple times on the same entity
void entity_make_end(registry* r, entity e);

// Returns the component cp for the entity e. If entity has not the cp, undefined behaviour. Use entity_try_get instead (faster)
void* entity_get(registry* r, entity e, const std::string& cp_name);
template <typename T> T* entity_get(registry* r, entity e, const std::string& cp_name) { return (T*)entity_get(r, e, cp_name); }

// Returns the component cp for the entity e if it exists or nullptr. (slower)
void* entity_try_get(registry* r, entity e, const std::string& cp_name);
template <typename T> T* entity_try_get(registry* r, entity e, const std::string& cp_name) { return (T*)entity_try_get(r, e, cp_name); }

// Returns true only if the entity is valid. Valid means that registry has created it. 
bool entity_valid(registry* r, entity e);

// Destroy the entity with the components associated with 
// IMPORTANT: Undefined Behaviour if you call this function while iterating views.
void entity_destroy(registry* r, entity e);

// Returns a copy of all the entities in the registry (WARNING: this is a slow operation)
// Remember that after operations this vector will not be update.
std::vector<entity> entity_all(registry* r);

// Marks this entity to be destroyed (using entity_destroy_flush_delayed)
// You can use this during view iterations
void entity_destroy_delayed(registry* r, entity e);

// Returns true if the entity is marked for delayed destruction using entity_destroy_delayed
// You can use this function during view iterations
bool entity_is_destroy_delayed(registry* r, entity e);

// Iterates all the entities marked for delayed destruction (using entity_destroy_delayed)
// IMPORTANT: Undefined behaviour if this function is called during a view iteration
void entity_destroy_flush_delayed(registry* r);




//--------------------------------------------------------------------------------------------------
// System pool functions

struct syspool;

syspool* syspool_create();
void syspool_destroy(syspool* s);

// Adds a system function to the syspool
typedef void (syspool_update_fn)(registry* r, float dt);
void syspool_add(syspool* sys, const std::string& sys_name, syspool_update_fn* sys_update_fn);

// This runs all the registered systems in the systempool with a registry and a delta
void syspool_run(syspool* sys, registry* r, float dt);


//--------------------------------------------------------------------------------------------------
// Context Variables Functions

//void* ctx_register(registry* r, const char* name_id, void* instance_ptr, delete_fn* del_fn);
//void* ctx_get(registry* r, std::uint64_t id);
//
//// Registers a context variable instance
//template <typename T> T* ctx_register(registry* r, const char* id, T* instance_ptr) {
//    return (T*)ctx_register(r, id, instance_ptr,[](void* ptr) {((T*)ptr)->~T();});
//}
//
//// Registers a context variable instance (the instance is created inside the function)
//template <typename T> T* ctx_register_instantiate(registry* r, const char* id) {
//    return (T*)ctx_register(r, id, new T(), [](void* ptr) {((T*)ptr)->~T(); });
//}
//
//template <typename T> T* ctx_get(registry* r, std::uint64_t ctx_id) {
//    return (T*)ctx_get(r, ctx_id);
//}


// Implementation details here...
namespace detail {

struct cp_storage {
    std::string name; /* component name */
    std::uint64_t cp_id = 0; /* component id for this storage */
    size_t cp_sizeof = 0; /* sizeof for each cp_data element */

    /*  packed component elements array. aligned with dense */
    std::vector<char> cp_data;

    /*  Dense entities array.
        - index is linked with the sparse value.
        - value is the full entity
    */
    std::vector<entity> dense;

    /*  sparse entity identifiers indices array.
        - index is the id of the entity. (without version and type_idx)
        - value is the index of the dense array (uint32_t)
        
        (Note) This can be refactored to an std::unordered_map<uint32_t, uint32_t> 
        to reduce memory footprint but at the cost of access time.
    */
    std::vector<uint32_t> sparse;

    cp_serialize_fn* serialize_fn = nullptr;
    placement_new_fn* placement_new_fn = nullptr;
    delete_fn* delete_fn = nullptr;

    inline bool contains(entity e) {
        dscheck(e != entity_null);
        const uint32_t eid = entity_id(e);
        return (eid < sparse.size()) && (sparse[eid] != entity_null);
    }

    void debug_arrays() {
        DS_LOG("Sparse:");
        for (auto i = 0; i < sparse.size(); i++) {
            DS_LOG(fmt::format("[{}] => {}", i, sparse[i]));
        }

        DS_LOG("Dense:");
        for (auto i = 0; i < dense.size(); i++) {
            DS_LOG(fmt::format("[{}] => {}", i, entity_stringify(dense[i])));
        }

        DS_LOG(fmt::format("Components vector: (cp_size: {}  bytesize: {}  cp_sizeof: {}", cp_data.size() / (float)cp_sizeof,  cp_data.size(), cp_sizeof));
    }


    std::string entity_stringify(entity e) {
        return fmt::format("Entity: {}  ( id: {}  version: {}   type: {})", e, entity_id(e), entity_version(e), entity_type_idx(e));
    }

    inline void* emplace(entity e) {
        dscheck(e != entity_null);
        // now allocate the data for the new component at the end of the array and memset to 0
        cp_data.resize(cp_data.size() + cp_sizeof, 0);

        // return the component data pointer (last position of the component sizes)
        void* cp_data_ptr = &cp_data[cp_data.size() - cp_sizeof];

        // Then add the entity to the sparse/dense arrays
        const uint32_t eid = entity_id(e);
        DS_LOG(fmt::format("Adding {}", entity_stringify(e)) );
        if (eid >= sparse.size()) { // check if we need to realloc
            sparse.resize(eid + 1, entity_null); // should be 0xFFFFFFFF (32 bits of 1)
        }
        
        sparse[eid] = (uint32_t)dense.size();
        dense.push_back(e);
       // debug_arrays();
        return cp_data_ptr;
    }

    inline void remove(entity e) {
        dscheck(contains(e));
        DS_LOG(fmt::format("Removing {}", entity_stringify(e)));

        // Remove from sparse/dense arrays
        const uint32_t pos_to_remove = sparse[entity_id(e)];
        const entity other = dense.back();
        sparse[entity_id(other)] = pos_to_remove;
        dense[pos_to_remove] = other;
        sparse[entity_id(e)] = entity_null;
        dense.pop_back();
        // swap to the last position (memmove because if cp_data_size 1 it will overlap dst and source.
        memmove(
            &(cp_data)[pos_to_remove * cp_sizeof],
            &(cp_data)[(cp_data.size() - cp_sizeof)],
            cp_sizeof);

        // and pop the last one
        dscheck(cp_data.size() >= cp_sizeof);
        cp_data.resize(cp_data.size() - cp_sizeof);
        
      //  debug_arrays();
    }

    inline void* get_by_index(size_t index) {
        dscheck((index * cp_sizeof) < cp_data.size());
        return &(cp_data)[index * cp_sizeof];
    }

    inline void* get(entity e) {
        dscheck(e != entity_null);
        dscheck(contains(e));
        return get_by_index(sparse[entity_id(e)]);
    }

    inline void* try_get(entity e) {
        dscheck(e != entity_null);
        return contains(e) ? get(e) : nullptr;
    }
};
}


//--------------------------------------------------------------------------------------------------
// Views
// 
// What is allowed and what is not allowed during views:
// Allowed:
// - You can create entities during iterations BUT it can invalidate component references. 
//  The view will not iterate the entities created during the view iteration.
//  The new entities will be iterated in subsequent view creations.
//  
//  Example of component dangling pointer when creating entities:
//  void example(registry* r) {
//      ecs::view v = ecs::view::create(r, { "bullet" });
//      while (v.valid()) {
//          bullet* p = v.data<bullet>(v.index("bullet"));
//          p->pos.x += 10;
// 
//          // HERE YOU CREATE ANOTHER ENTITY
//          ecs::entity_make(r, "BulletEntity");
//          // NOW COMPONENT REFERENCES ARE INVALIDATE
// 
//          p->pos.x += 10; // p POINTER IS A DANGLING POINTER!!!!
//          // YOU CAN FETCH AGAIN THE COMPONENT POINTER p by p = v.data<bullet>(v.index("bullet")); IF YOU NEED IT
//          v.next();
//      }
// 
//  }
// Not Allowed:
// - Destroying entities, you should delay the destruction of entities.



struct view {
    static view create(registry* r, const std::vector<std::string>& cp_names);

    // returns true if the current iterating entity is valid else false
    inline bool valid() {
        return _impl.cur_entity != entity_null;
    }

    // returns the current iterating entity
    inline entity entity() {
        return _impl.cur_entity;
    }

    // Returns the component index associated with a component id (use data function to retrieve the data)
    inline std::size_t index(const std::string& cp_name) {
        const auto cp_id_hashed = fnv1a_64bit(cp_name);
        for (std::size_t i = 0; i < _impl.cp_storages.size(); i++) {
            if (_impl.cp_storages[i]->cp_id == cp_id_hashed) { return i; }
        }
        // error, no component with that cp_id in this view!
        dscheckm(false, "component id not found in this view!");
        return 0;
    }

    // Returns the component data associated with the component index for this view (see index function)
    template <typename T>  inline T* data(size_t cp_idx) {
        dscheck(valid());
        dscheck(cp_idx < _impl.cp_storages.size());
        return (T*)_impl.cp_storages[cp_idx]->get(_impl.cur_entity);
    }



    // Advances the next entity that has all the components for the view
    inline void next() {
        dscheck(valid());
        // find the next contained entity that is inside all pools
        bool entity_contained = false;
        do {
            if (_impl.entity_index < _impl.entity_max_index - 1) {
                // select next entity from the iterating storage (smaller one..)
                ++_impl.entity_index;
                _impl.cur_entity = _impl.iterating_storage->dense[_impl.entity_index];
                // now check if the entity is contained in ALL other storages:
                entity_contained = _impl.is_entity_in_all_storages(_impl.cur_entity);
            } else {
                _impl.cur_entity = entity_null;
            }
        } while ((_impl.cur_entity != entity_null) && !entity_contained);
    }

    // implementation details
    struct view_impl {
        std::vector<detail::cp_storage*> cp_storages;
        detail::cp_storage* iterating_storage = nullptr;
        std::size_t entity_index = 0;
        std::size_t entity_max_index = 0; // it's like _impl.iterating_storage->dense.size() 
        ::ds::ecs::entity cur_entity = entity_null;
        inline bool is_entity_in_all_storages(::ds::ecs::entity e) {
            for (std::size_t st_id = 0; st_id < cp_storages.size(); ++st_id) {
                if (!cp_storages[st_id]->contains(e)) {
                    return false;
                }
            }
            return true;
        }
    };
    // implementation details
    view_impl _impl;
};





}
