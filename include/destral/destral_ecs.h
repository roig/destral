#pragma once

/*
    destral_ecs.h

    FIX (dani) TODO proper instructions

    TODO: Update documentation..



    ////////// Registry information.

    * The registry holds the registered components, entities and systems that are always registered at the start.
    * After you register a component or an entity or a system, you can't unregister them. 
    

    ////////// Components
    //TODO TODO
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


    ////////// Registering Entities

    Entities are type identifiers that are composed by components.
    The registry allows the registration of entities with a number of components using the ecs::entity_register() function.
    

    

    1) register the components



    

    ////////// Construction of an entity:
    There are two ways of constructing an entity:
        * using entity_make()
            Instantiates an entity and calls the initialization function on it if it exists.
            This will call internally entity_make_begin and then entity_make_end.
            This will call the entity init function registered for the entity type if any after the creation of all the components.
            IMPORTANT: You can call this function during view iterations. The new created entity will NOT be iterated in the current view.

        * using entity_make_begin() and entity_make_end();
            entity_make_begin() Will instantiate an entity with all the components WITHOUT calling the init function of the entity type.
            This allows you to setup the parameters of the entity components before calling the init function. (like a constructor by params)
            entity_make_end() Finishes the instantiation of an entity created by entity_make_begin
            IMPORTANT: UB if you don't call entity_make_end on the entity returned by this function in order to have a fully constructed/initialized entity
            IMPORTANT: UB if used on an entity not created using the entity_make_begin.
            IMPORTANT: UB if called multiple times on the same entity.

    
    The creation of an entity internally looks like this:

    Step 1: create the entity in the registry.
    Step 2: 
        for each component (order specified on registration) {
            1 -> construct (placement new) the component (calls the constructor of the component)
            2 -> call cp_serialize component function if any is set for that component 
            (You can retrieve inside this function ONLY components that were constructed/serialized before the current one)
        }
    --- now all the components are constructed and default initialized (cp_serialize function) for that entity ---
    Step 3: Call the init function for that entity of the entity type if any.



    
    ***** TODO NOT IMPLEMENTED AND EXPLAIN THIS CORRECTLY *****
    ***** (TODO) -> call user initialize function (this should be a callback provided in entity_make)
    *****     The purpose of this function is to allow to initialize some variables on a per instance or to be used in the step 4. 
    *****     It´s like a custom constructor before the real entity constructor in step 4.
    ***** Step 4: Call the initialize entity function. (begin_play) (this should be a callback set when registering a new entity type)
    *****     this is like a constructor by parameter for that entity, because it can read the config variables set by step 3
    *****     and configure all the components needed based on those configuration variables. It can be null of course.
    *****     (This function can call other entity instantiation functions!) BUT! if you do it it will probably invalidate component
    *****     pointers you saved, so retrieve them again after instantiation.
    ***** 

    ////////// Destruction of an entity:
    There is two ways of destroying an entity:
        * using entity_destroy (Non delayed): 
            This will destroy all the entity type components and the entity from the registry.
            IMPORTANT: Undefined Behaviour if you call this function during a view iteration. 
            IMPORTANT: Use the delayed mode if you need to destroy an entity during a view iteration.

        * using entity_destroy_delayed (Delayed):
            This marks entity to be destroyed.
            IMPORTANT: You MUST use this to destroy entities during iterations.
            
            - The function entity_destroy_flush_delayed will iterate and call entity_destroy on all
            entities that are marked to be destroyed.
            IMPORTANT: You MUST NOT call entity_destroy_flush_delayed during a view iteration.
            IMPORTANT: This function is called after every system update.
              
            - Use the function entity_is_destroy_delayed to retrieve if the entity is marked for destroy or not
     
    
    Entity destruction internal steps when calling entity_destroy :

    Step 1: First calls deinit function registered for that entity type if any is set.
    Step 2: 
        for each component (inverse order specified on registration) {
            1 -> call destructor of the component if any.
            2 -> remove the component memory from the storage.
        }
    Step 3: Destroy the entity from the registry.
*/
#include <destral/destral_common.h>
#include <destral/destral_containers.h>


namespace ds {
struct registry;

//--------------------------------------------------------------------------------------------------
// Entity:
// Entity recicles ids when arriving at max version number.
// INFO: https://docs.cryengine.com/display/SDKDOC4/EntityID+Explained
// id range (0 to max_INT32)  (this is because we can use an i32 to index all the entities in an array that retuns i32 indexes)
// version range (0 to max_UINT32)
// type_id (0 to max_INT32) (this allows to fit all the types in a i32 indexed array)
// 
struct entity {
    i32 id = max_id(); 
    u32 version = 0; // id will go from 0 to max_UINT32
    i32 type_id = max_type_id(); 

    // Returns true only if the entities are equal
    bool operator== (const entity& o) const { return (id == o.id) && (version == o.version) && (type_id == o.type_id); }

    // Returns true only if the entity is null
    bool is_null() { return *this == null;  }

    // The entity_null is a entity that represents a null entity.
    static const entity null;

    // Stringyfies an entity
    std::string to_string();
    static constexpr i32 max_id() { return std::numeric_limits<i32>::max(); }
    static constexpr u32 max_version() { return std::numeric_limits<u32>::max(); }
    static constexpr i32 max_type_id() { return std::numeric_limits<i32>::max(); }

};


//--------------------------------------------------------------------------------------------------
// Views
// 
// What is allowed and what is not allowed during views:
// ALLOWED:
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
//          r->ecs::entity_make("BulletEntity");
//          // NOW COMPONENT REFERENCES ARE INVALIDATED
// 
//          p->pos.x += 10; // p POINTER IS A DANGLING POINTER!!!!
//          // YOU CAN FETCH AGAIN THE COMPONENT POINTER p by p = v.data<bullet>(v.index("bullet")); IF YOU NEED IT
//          v.next();
//      }
// 
//  }
// NOT ALLOWED:
// - Destroying entities, you must delay the destruction of entities.
// Implementation details here...

struct view {
    // returns true if the current iterating entity is valid else false
    inline bool valid() { return !_impl.cur_entity.is_null();}

    // returns the current iterating entity
    inline ds::entity entity() { return _impl.cur_entity; }

    // Returns the component index associated with a component id (use data function to retrieve the data)
    i32 index(const char* cp_name);

    // Returns the raw component data pointer associated with the component index for this view (see index function)
    void* raw_data(i32 cp_idx);

    // Returns the component data associated with the component index for this view (see index function)
    template <typename T> inline T* data(i32 cp_idx) { return (T*)raw_data(cp_idx); }

    // Advances the next entity that has all the components for the view
    void next();
    
    // implementation details
    struct view_impl {
        struct cp_storage* iterating_storage = nullptr;
        ds::darray<struct cp_storage*> cp_storages;
        i32 entity_index = 0;
        i32 entity_max_index = 0; // it's like _impl.iterating_storage->dense.size() 
        ds::entity cur_entity = entity::null;
    };
    // implementation details
    view_impl _impl;
};

//--------------------------------------------------------------------------------------------------
// Registry 
// Global context that holds each storage for each component types and the entities.

struct cp_definition {
    std::string name;
    i32 cp_sizeof = 0;

    typedef void (placement_new_function)(void* ptr);
    placement_new_function* placement_new_fn = nullptr;

    typedef void (delete_function)(void* ptr);
    delete_function* delete_fn = nullptr;

    typedef void (serialize_function)(registry* r, entity e, void* cp); // TODO not used yet
    serialize_function* serialize_fn = nullptr;
};

struct entity_definition {
    std::string name;
    ds::darray<std::string> cp_names;

    typedef void (entity_init_fn)(registry* r, entity e);
    entity_init_fn* init_fn = nullptr;

    typedef void (entity_deinit_fn)(registry* r, entity e);
    entity_deinit_fn* deinit_fn = nullptr;
};


struct registry_impl;
struct registry {
    registry();
    ~registry();

    //--------------------------------------------------------------------------------------------------
    // Component functions
    #define DS_ECS_COMPONENT_REGISTER(r,T,cp_name) r->cp_register<T>(cp_name) 
    #define DS_ECS_COMPONENT_REGISTER_WITH_SERIALIZE(r,T) r->cp_register_serialize<T>(#T)
    void cp_register(const cp_definition& cp_def);

    template <typename T> void cp_register_serialize(const char* cp_name) {
        auto placement_new_fn = [](void* mem_cp) { new (mem_cp) T(); }; // placement new
        auto destructor_fn = [](void* mem_cp) { ((T*)mem_cp)->~T(); }; // destructor
        auto srlz_fn = [](registry* r, entity e, void* mem_cp) { ((T*)mem_cp)->cp_serialize(r, e); }; // serialize
        cp_definition cd{ .name = cp_name, .cp_sizeof = (i32)sizeof(T), .placement_new_fn = placement_new_fn,
        .delete_fn = destructor_fn, .serialize_fn = srlz_fn };
        cp_register(cd);
    }

    template <typename T> void cp_register(const char* cp_name) {
        auto placement_new_fn = [](void* mem_cp) { new (mem_cp) T(); }; // placement new
        auto destructor_fn = [](void* mem_cp) { ((T*)mem_cp)->~T(); }; // destructor
        cp_definition cd{ .name = cp_name, .cp_sizeof = (i32)sizeof(T), .placement_new_fn = placement_new_fn,
        .delete_fn = destructor_fn };
        cp_register(cd);
    }

    // Returns the component cp for the entity e. (faster version) If entity has not the cp, undefined behaviour use entity_try_get instead 
    void* cp_get(entity e, const char* cp_name);
    template <typename T> T* cp_get(entity e, const char* cp_name) { return (T*)cp_get(e, cp_name); }

    // Returns the component cp for the entity e if it exists or nullptr. (slower version)
    void* cp_try_get(entity e, const char* cp_name);
    template <typename T> T* cp_try_get(entity e, const char* cp_name) { return (T*)cp_try_get(e, cp_name); }

    //--------------------------------------------------------------------------------------------------
    // Entity functions

    // Registers an entity name with it's components names. 
    void entity_register(const entity_definition& e_def);

    // Instantiates an entity and calls the initialization function on it if it exists
    // This will call internally entity_make_begin and then entity_make_end
    // This will call the init function registered for the entity if any after the creation of all the components
    // You can call this function during view iterations. The new created entity will not be iterated in the current view.
    entity entity_make(const char* entity_name);

    // Instantiates an entity WITHOUT calling the initialization function of the entity
    // This allows you to setup the parameters of the entity components before calling the init function. (like a constructor by params)
    // IMPORTANT:
    // Undefined Behaviour if you don't call entity_make_end on the entity returned by this function in order to have a fully constructed/initialized entity
    entity entity_make_begin(const char* entity_name);

    // Finishes the instantiation of an entity created by entity_make_begin
    // IMPORTANT: 
    // Undefined behaviour if used on an entity not created using the entity_make_begin.
    // Undefined behaviour if called multiple times on the same entity
    void entity_make_end(entity e);

    // Returns true only if the entity is valid. Valid means that registry has created it. 
    bool entity_valid(entity e);

    // Returns a copy of all the entities in the registry (WARNING: this is a slow operation)
    // Remember that after operations this vector will not be update.
    ds::darray<entity> entity_all();

    // Destroy the entity with the components associated with 
    // IMPORTANT: Undefined Behaviour if you call this function while iterating views.
    void entity_destroy(entity e);

    // Marks this entity to be destroyed (using entity_destroy_flush_delayed)
    // You can use this during view iterations
    void entity_destroy_delayed(entity e);

    // Returns true if the entity is marked for delayed destruction using entity_destroy_delayed
    // You can use this function during view iterations
    bool entity_is_destroy_delayed(entity e);

    // Iterates all the entities marked for delayed destruction (using entity_destroy_delayed)
    // IMPORTANT: Undefined behaviour if this function is called during a view iteration
    void entity_destroy_flush_delayed();

    // Destroys ALL the entities with the components associated with
    // IMPORTANT: Undefined Behaviour if you call this function while iterating views.
    void entity_destroy_all();

    //--------------------------------------------------------------------------------------------------
    // Views 
    view view_create(const ds::darray<const char*>& cp_names);



    //--------------------------------------------------------------------------------------------------
    // Systems
    // Adds a system function to the list of systems in a queue
    #define DS_ECS_QUEUE_ADD_SYSTEM(r, queue_name, fun) r->system_queue_add(queue_name, #fun , fun )
    typedef void (system_update_fn)(registry* r);
    void system_queue_add(const char* queue_name, const char* sys_name, system_update_fn* sys_update_fn);


    struct sys_queue_run_stats {
        std::string queue_name;
        struct sys_run_stats {
            std::string sys_name;
            double miliseconds;
        };
        darray<sys_run_stats> sys_stats;
    };
    // This runs all the registered systems in the queue name and returns system statistics.
    sys_queue_run_stats system_queue_run(const char* queue_name);

    

    //--------------------------------------------------------------------------------------------------
    // Context Variables (Globals in the registry)
    // Context variables are like global instances tied to the registry. You can add and remove them at any time.
    // When the registry is deleted, the context variables will be deleted in reverse order of addition.
    void* ctx_set(const char* ctx_name_id, void* instance_ptr, void (*del_fn)(void* ptr));
    template <typename T> T* ctx_set(const char* ctx_name_id, T* instance_ptr) { return (T*)ctx_set(ctx_name_id, instance_ptr, [](void* ptr) {((T*)ptr)->~T(); }); }
    template <typename T> T* ctx_set_instantiate(const char* ctx_name_id) { return (T*)ctx_set(ctx_name_id, new T(), [](void* ptr) {((T*)ptr)->~T(); }); }

    void ctx_unset(const char* ctx_name_id);
    // Calls the deleter for each ctx variable set in inverse order of context variables registration. Then clears the maps/arrays of ctx vars
    void ctx_unset_all();

    // Returns the pointer to the context variable instance or nullptr
    void* ctx_get(const char* ctx_name_id);
    template <typename T> T* ctx_get(const char* ctx_name_id) { return (T*)ctx_get(ctx_name_id); }


    //--------------------------------------------------------------------------------------------------
    // Implementation details
    registry_impl* _r;
};


}
