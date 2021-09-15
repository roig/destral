#pragma once

#include <destral/destral_common.h>
#include <destral/destral_ecs.h>
#include <unordered_map>



namespace ds {

    /**
    @brief resource state

    @see @ref resource::state(), @ref ResourceManager::state()
    */
    enum class resource_state {
        /** The resource is not yet loaded (and no fallback is available). */
        not_loaded,

        /** The resource is not yet loaded and fallback resource is used instead. */
        not_loaded_fallback,

        /** The resource is currently loading (and no fallback is available). */
        loading,

        /** The resource is currently loading and fallback resource is used instead. */
        loading_fallback,

        /** The resource was not found (and no fallback is available). */
        not_found,

        /** The resource was not found and fallback resource is used instead. */
        not_found_fallback,

        /** The resource is loaded, but can be changed by the manager at any time. */
        loaded_mutable,

        /** The resource is loaded and won't be changed by the manager anymore. */
        loaded_final
    };

    /**
    @brief resource data state

    @see @ref ResourceManager::set(), @ref resource_state
    */
    enum class resource_data_state {
        /**
            * The resource is currently loading. Parameter @p data in
            * @ref ResourceManager::set() should be set to @cpp nullptr @ce.
            */
        loading = (i32)resource_state::loading,

        /**
        * The resource was not found. Parameter @p data in
        * @ref ResourceManager::set() should be set to @cpp nullptr @ce.
        */
        not_found = (i32)resource_state::not_found,

        /**
        * The resource can be changed by the manager in the future. This is
        * slower, as @ref resource needs to ask the manager for new version every
        * time the data are accessed, but allows changing the data for e.g.
        * debugging purposes.
        */
        loaded_mutable = (i32)resource_state::loaded_mutable,

        /**
        * The resource cannot be changed by the manager in the future. This is
        * faster, as @ref resource instances will ask for the data only one time,
        * thus suitable for production code.
        */
        loaded_final = (i32)resource_state::loaded_final
    };

    /**
    @brief resource policy

    @see @ref ResourceManager::set(), @ref ResourceManager::free()
     */
    enum class resource_policy {
        /** The resource will stay resident for whole lifetime of resource manager. */
        resident,

        /**
         * The resource will be unloaded when manually calling
         * @ref ResourceManager::free() if nothing references it.
         */
         manual,

         /** The resource will be unloaded when last reference to it is gone. */
         reference_counted
    };


    /** Key for accessing resource  */
    class resource_key {
    public:
        ///**
        // * @brief Default constructor
        // *
        // * Creates zero key. Note that it is not the same as calling other
        // * constructors with empty string.
        // */
        constexpr resource_key() {}

        /** @brief Construct resource key directly from hashed value */
        //explicit resource_key(u64 key): {}

        /** @brief Constructor */
        constexpr resource_key(const char* res_key) : key(res_key), hashed_key(fnv1a_32bit(res_key)) {}


        std::string key;
        i32 hashed_key = 0;
        
        bool operator==(const resource_key& other) const { return hashed_key == other.hashed_key; }
    };


    
    class resource_cache;
    

    /**
        resource reference
    */
    class resource {
    public:
        /**
         * @brief Default constructor
         *
         * Creates empty resource. Resources are acquired from the manager by
         * calling @ref ResourceManager::get().
         */
        explicit resource() {}
        resource(const resource& other);
        ~resource();

        /** @brief Move constructor */
        //resource(resource&& other) noexcept;

        /** @brief Copy assignment */
        resource& operator=(const resource& other);

        /** @brief Equality comparison */
        bool operator==(const resource& other) const;

        /** @brief Non-equality comparison */
        bool operator!=(const resource& other) const;

        /** @brief resource key */
        resource_key key() const { return _key; }

        /**
         * @brief resource state
         *
         * @see @ref operator bool(), @ref ResourceManager::state()
         */
        resource_state state() {
            acquire();
            return _state;
        }

        /**
         * @brief Whether the resource is available
         *
         * Returns @cpp false @ce when resource is not loaded and no fallback
         * is available (i.e. @ref state() is either
         * @ref resource_state::not_loaded, @ref resource_state::loading or
         * @ref resource_state::not_found), @cpp true @ce otherwise.
         */
        bool is_available() {
            acquire();
            return _data != entity_null;
        }

        /**
         * @brief Access to resource data
         *
         * The resource must be loaded before accessing it. Use boolean
         * conversion operator or @ref state() for testing whether it is
         * loaded.
         */
        entity get() {
            acquire();
            return _data;
        }

    private:
        void acquire();
        friend resource_cache;
        resource(resource_cache* manager, resource_key key);
        resource_cache* _manager = nullptr;
        resource_key _key = {};
        i32 _lastCheck = 0;
        resource_state _state = resource_state::loaded_final;
        entity _data = entity_null;
    };

    void resource_init(registry* r);
    void resource_deinit(registry* r);

    resource resource_get(registry* r, const resource_key& key); // TODO aixo hauria de ser un const char*?
    void resource_set(registry* r, const resource_key& key, entity data, const resource_data_state state, const resource_policy policy);
    void resource_set_fallback(registry* r, entity data);
    entity resource_get_fallback(registry* r, const char* entity_type_id); // TODO change to i32? type_id

    typedef void (resource_load_fn)(registry* r, const resource_key& key);
    typedef bool (resource_can_load_fn)(registry* r, const resource_key& key);
    struct resource_loader {
        resource_load_fn* load_fn = nullptr;
        resource_can_load_fn* can_load_resource_fn = nullptr;
    };
    void resource_set_loader(registry* r, resource_loader loader);



    

/**
@brief resouce_cache

Provides storage for a type.

Each resource is referenced from @ref resource class. For optimizing
performance, each resource can be set as mutable or final. loaded_mutable resources
can be modified by the manager and thus each @ref resource instance asks the
manager for modifications on each access. On the other hand, final resources
cannot be modified by the manager, so @ref resource instances don't have to ask
the manager every time, which is faster.

It's possible to provide fallback for resources which are not available using
set_fallback(). Accessing data of such resources will access the fallback
instead of failing on null pointer dereference. Availability and state of each
resource can be queried through function state() on the cache or
resource::state() on each resource.

The resources can be managed in three ways:
- resident resources, which stay in memory for whole lifetime of the manager.
- anually managed resources, which can be deleted by calling free() if nothing references them anymore.
- reference counted resources, which are deleted as soon as the last reference to them is removed.

resource state and policy is configured when setting the resource data in
set() and can be changed each time the data are updated, although already
loaded_final resources cannot obviously be set as mutable again.

Basic usage is:
    
    -   Filling the cache with resource data and acquiring the resources. Note
    that a resource can be acquired with @ref get() even before the cache
    contains the data for it, as long as the resource data are not accessed (or
    fallback is provided).

    resource_cache<texture> texture_cache;
    resource<texture> cube = texture_cache.get<texture>("cube_texture");

    // The cache doesn't have data for the cube yet, add them
    if(!cube) {
        texture* cube_texture_ptr = new texture();
        // ...
        manager.set(cube.key(), cube_texture_ptr);
    }

    -  Using the resource data.
    if (cube) {
        cube->....
    }
    

    -   Destroying resource references and deleting manager instance when nothing
    references the resources anymore.
*/



//template<class T> void resource_cache<T>::set_fallback(T* const data) {
//    impl::safeDelete(_fallback);
//    _fallback = data;
//    /* Notify resources also in this case, as some of them could go from empty
//       to a fallback (or from a fallback to empty) */
//    ++_lastChange;
//}
//
//template<class T> void resource_cache<T>::free() {
//    /* Delete all non-referenced non-resident resources */
//    for(auto it = _data.begin(); it != _data.end(); ) {
//        if(it->second.policy != resource_policy::resident && !it->second.referenceCount)
//            it = _data.erase(it);
//        else ++it;
//    }
//}
//template<class T> void resource_cache<T>::set_loader(resource_loader<T>* const loader) {
//    /* Delete previous loader */
//    delete _loader;
//
//    /* Add new loader */
//    _loader = loader;
//    if(_loader) _loader->manager = this;
//}
//
//template<class T> void resource_cache<T>::free_loader() {
//    if(!_loader) return;
//
//    _loader->manager = nullptr;
//    delete _loader;
//}






}

