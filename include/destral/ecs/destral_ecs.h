#pragma once

/*
    destral_ecs.h

    FIX (dani) TODO proper instructions

    TODO: Fer que el regisre vagi per steps, es a dir, primer components, despres entitats, despres sistemes (sino saltar errors)
    TODO: Construction and initialization of components.
    TODO: Construction and initialization of entities.
    TODO: Delayed construction of entities.
    TODO: Delayed destruction of entities.
    TODO: Something like entt::basic_handle: https://github.com/skypjack/entt/blob/master/src/entt/entity/handle.hpp


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
/* Makes a entity from an id and version */
static constexpr entity entity_assemble(uint32_t id, uint32_t version, uint32_t type_idx) { return (entity(id) | (entity(type_idx) << 32)) | (entity(version) << 48); }
/* The entity_null is a entity that represents a null entity. */
static constexpr entity entity_null = entity_max_id();

//--------------------------------------------------------------------------------------------------
// Registry: global context that holds each storage for each component types and the entities.

struct registry;
registry* registry_create(); /*  Allocates and initializes a registry context */
void registry_destroy(registry* r); /*  Deinitializes and frees a registry context */


//--------------------------------------------------------------------------------------------------
// Component Functions
#define DS_ECS_CP_REGISTER(r,T) cp_register<T>(r, #T) 

typedef void (cp_serialize_fn)(registry* r, entity e, void* cp);
typedef void (cp_cleanup_fn)(registry* r, entity e, void* cp);
typedef void (placement_new_fn)(void* ptr);
typedef void (delete_fn)(void* ptr);
void cp_register(registry* r, const char* name_id, size_t cp_sizeof, placement_new_fn* placement_new_fn = nullptr,
    delete_fn* del_fn = nullptr,  cp_serialize_fn* serialize_fn = nullptr, cp_cleanup_fn* cleanup_fn = nullptr);


//--------------------------------------------------------------------------------------------------
// Components View Functions
struct view;
view view_create(registry* r, const std::vector<std::uint64_t>& cp_ids);

//--------------------------------------------------------------------------------------------------
// System Functions
typedef void (sys_update_fn) (registry* r, view* view);
void system_add(registry* r, const char* name_id, const std::vector<std::uint64_t>& cp_ids, sys_update_fn* update_fn);
void run_systems(registry* r);

//--------------------------------------------------------------------------------------------------
// Entity Functions

// Registers an entity type with it's components
void entity_register(registry* r, const char* entity_name_id, std::vector<std::uint64_t> cps);
// Instantiates an entity
entity entity_make(registry* r, std::uint64_t id);
// Returns the component cp for the entity e. If entity has not the cp, undefined behaviour. Use entity_try_get instead
void* entity_get(registry* r, entity e, std::uint64_t cp_id);
// Returns the component cp for the entity e if it exists or nullptr.
void* entity_try_get(registry* r, entity e, std::uint64_t cp_id);
// Returns true only if the entity is valid. Valid means that registry has created it. 
bool entity_valid(registry* r, entity e);
// Deletes the entity with the components associated with (do not call this function while iterating views. Will invalidate pointers)
void entity_delete(registry* r, entity e);

//--------------------------------------------------------------------------------------------------
// Context Variables Functions

void* ctx_register(registry* r, const char* name_id, void* instance_ptr, delete_fn* del_fn);
void* ctx_get(registry* r, std::uint64_t id);


//--------------------------------------------------------------------------------------------------
// Template helpers

// This template function registers a component pulling constructors/destructor and functions from the type functions
// Uses init and deinit functions and default constructor/destructor
template <typename T>
void cp_register(registry* r, const char* name) {
    auto constructor_fn = [](void* mem_cp) { new (mem_cp) T(); }; // placement new
    auto destructor_fn = [](void* mem_cp) { ((T*)mem_cp)->~T(); }; // destructor
    auto init_fn = [](registry* r, entity e, void* mem_cp) { ((T*)mem_cp)->init(r, e); };
    auto deinit_fn = [](registry* r, entity e, void* mem_cp) { ((T*)mem_cp)->deinit(r, e); };
    cp_register(r, name, sizeof(T), constructor_fn, destructor_fn, init_fn, deinit_fn);
}
template <typename T> T* entity_get(registry* r, entity e, std::uint64_t cp_id) { return (T*)entity_get(r, e, cp_id); }
template <typename T> T* entity_try_get(registry* r, entity e, std::uint64_t cp_id) {  return (T*)entity_try_get(r, e, cp_id); }

// Registers a context variable instance
template <typename T> T* ctx_register(registry* r, const char* id, T* instance_ptr) {
    return (T*)ctx_register(r, id, instance_ptr,[](void* ptr) {((T*)ptr)->~T();});
}

// Registers a context variable instance (the instance is created inside the function)
template <typename T> T* ctx_register_instantiate(registry* r, const char* id) {
    return (T*)ctx_register(r, id, new T(), [](void* ptr) {((T*)ptr)->~T(); });
}

template <typename T> T* ctx_get(registry* r, std::uint64_t ctx_id) {
    return (T*)ctx_get(r, ctx_id);
}




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
    */
    std::vector<uint32_t> sparse;

    placement_new_fn* placement_new_fn = nullptr;
    cp_serialize_fn* serialize_fn = nullptr;
    cp_cleanup_fn* cleanup_fn = nullptr;
    delete_fn* delete_fn = nullptr;

    inline bool contains(entity e) {
        dscheck(e != entity_null);
        const uint32_t eid = entity_id(e);
        return (eid < sparse.size()) && (sparse[eid] != entity_null);
    }


    inline void* emplace(entity e) {
        dscheck(e != entity_null);
        // now allocate the data for the new component at the end of the array
        cp_data.resize(cp_data.size() + (cp_sizeof));

        // return the component data pointer (last position of the component sizes)
        void* cp_data_ptr = &cp_data[cp_data.size() - cp_sizeof];

        // Then add the entity to the sparse/dense arrays
        const uint32_t eid = entity_id(e);
        if (eid >= sparse.size()) { // check if we need to realloc
            sparse.resize(eid + 1, entity_null); // should be 0xFFFFFFFF (32 bits of 1)
        }
        sparse[eid] = (uint32_t)dense.size();
        dense.push_back(e);
        return cp_data_ptr;
    }

    inline void remove(entity e) {
        dscheck(contains(e));
        // Remove from sparse/dense arrays
        const uint32_t pos_to_remove = sparse[entity_id(e)];
        const entity other = dense.back();
        sparse[entity_id(other)] = pos_to_remove;
        dense[pos_to_remove] = other;
        sparse[pos_to_remove] = entity_null;
        dense.pop_back();
        // swap (memmove because if cp_data_size 1 it will overlap dst and source.
        memmove(
            &(cp_data)[pos_to_remove * cp_sizeof],
            &(cp_data)[(cp_data.size() - cp_sizeof)],
            cp_sizeof);
        // and pop
        cp_data.pop_back();
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
        return contains(e) ? get(e) : 0;
    }
};
}

struct view {
    // returns true if the current iterating entity is valid else false
    inline bool valid() {
        return _impl.cur_entity != entity_null;
    }

    // returns the current iterating entity
    inline entity entity() {
        return _impl.cur_entity;
    }

    // Returns the component index associated with a component id (use data function to retrieve the data)
    inline std::size_t index(std::uint64_t cp_id_hashed) {
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
            if (_impl.entity_index < _impl.iterating_storage->dense.size() - 1) {
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
