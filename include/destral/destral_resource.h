#pragma once

#include <destral/destral_common.h>
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


    /** Key for accessing resource  */
    class resource_key {
    public:
        /**
         * @brief Default constructor
         *
         * Creates zero key. Note that it is not the same as calling other
         * constructors with empty string.
         */
        constexpr resource_key() {}

        /** @brief Construct resource key directly from hashed value */
        //explicit resource_key(u64 key): {}

        /** @brief Constructor */
        resource_key(const std::string& res_key) : key(res_key), hashed_key(fnv1a_64bit(res_key)) {}
        template<std::size_t size> constexpr resource_key(const char(&res_key)[size]) : key(res_key), hashed_key(fnv1a_64bit(res_key)) {}


        std::string key;
        u64 hashed_key = 0;
        
        /** @brief Equality comparison */
        bool operator==(const resource_key& other) const {
            return hashed_key == other.hashed_key;
        }
    };


    
    template<class> class resource_cache;
    

    /**
        resource reference
    */
    template<class T> class resource {
    public:
        /**
         * @brief Default constructor
         *
         * Creates empty resource. Resources are acquired from the manager by
         * calling @ref ResourceManager::get().
         */
        explicit resource() : _manager{ nullptr }, _lastCheck{ 0 }, _state{ resource_state::loaded_final }, _data{ nullptr } {}

        /** @brief Copy constructor */
        resource(const resource<T>& other) : _manager{ other._manager }, _key{ other._key }, _lastCheck{ other._lastCheck }, _state{ other._state }, _data{ other._data } {
            if (_manager) _manager->incrementReferenceCount(_key);
        }

        /** @brief Move constructor */
        resource(resource<T>&& other) noexcept;

        /** @brief Destructor */
        ~resource() {
            if (_manager) _manager->decrementReferenceCount(_key);
        }

        /** @brief Copy assignment */
        resource<T>& operator=(const resource<T>& other);

        /** @brief Move assignment */
        resource<T>& operator=(resource<T>&& other) noexcept;

        /** @brief Equality comparison */
        bool operator==(const resource<T>& other) const {
            return _manager == other._manager && _key == other._key;
        }

        /** @brief Equality comparison with other types is explicitly disallowed */
        template<class V> bool operator==(const resource<V>&) const = delete;

        /** @brief Non-equality comparison */
        bool operator!=(const resource<T>& other) const {
            return !operator==(other);
        }

        /** @brief Non-equality comparison with other types is explicitly disallowed */
        template<class V> bool operator!=(const resource<V>&) const = delete;

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
        operator bool() {
            acquire();
            return _data;
        }

        /**
         * @brief Access to resource data
         *
         * The resource must be loaded before accessing it. Use boolean
         * conversion operator or @ref state() for testing whether it is
         * loaded.
         */
        T* operator->() {
            acquire();
            dscheckm(_data, std::format("Resource: accessing not loaded data with key: {}  id:  {}", _key.key, _key.hashed_key));
            return static_cast<T*>(_data);
        }

    private:
        friend resource_cache<T>;
        resource(resource_cache<T>* manager, resource_key key) : _manager{ manager }, _key{ key }, _lastCheck{ 0 }, _state{ resource_state::not_loaded }, _data{ nullptr } {
            manager->incrementReferenceCount(key);
        }
        void acquire();
        resource_cache<T>* _manager;
        resource_key _key;
        std::size_t _lastCheck;
        resource_state _state;
        T* _data;
    };

    template<class T> resource<T>& resource<T>::operator=(const resource<T>& other) {
        if (_manager) _manager->decrementReferenceCount(_key);

        _manager = other._manager;
        _key = other._key;
        _lastCheck = other._lastCheck;
        _state = other._state;
        _data = other._data;

        if (_manager) _manager->incrementReferenceCount(_key);
        return *this;
    }

    template<class T> resource<T>::resource(resource<T>&& other) noexcept : _manager(other._manager), _key(other._key), _lastCheck(other._lastCheck), _state(other._state), _data(other._data) {
        other._manager = nullptr;
        other._key = {};
        other._lastCheck = 0;
        other._state = resource_state::loaded_final;
        other._data = nullptr;
    }

    template<class T> resource<T>& resource<T>::operator=(resource<T>&& other) noexcept {
        std::swap(_manager, other._manager);
        std::swap(_key, other._key);
        std::swap(_lastCheck, other._lastCheck);
        std::swap(_state, other._state);
        std::swap(_data, other._data);
        return *this;
    }

    template<class T> void resource<T>::acquire() {
        /* The data are already final, nothing to do */
        if (_state == resource_state::loaded_final) return;

        /* Nothing changed since last check */
        if (_manager->last_change() <= _lastCheck) return;

        /* Acquire new data and save last check time */
        const typename resource_cache<T>::Data& d = _manager->data(_key);
        _lastCheck = _manager->last_change();

        /* Try to get the data */
        _data = d.data;
        _state = static_cast<resource_state>(d.state);

        /* Data are not available */
        if (!_data) {
            /* Fallback found, add *Fallback to state */
            _data = _manager->fallback();
            if (_data) {
                if (_state == resource_state::loading)
                    _state = resource_state::loading_fallback;
                else if (_state == resource_state::not_found)
                    _state = resource_state::not_found_fallback;
                else _state = resource_state::not_loaded_fallback;

                /* Fallback not found and loading didn't start yet */
            } else if (_state != resource_state::loading && _state != resource_state::not_found)
                _state = resource_state::not_loaded;
        }
    }






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

template<class> class resource_loader;

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

template<class T> class resource_cache {
    template<class> friend class ds::resource;
    friend resource_loader<T>;

    public:
        resource_cache(): _fallback(nullptr), _loader(nullptr), _lastChange(0) {}
        resource_cache(const resource_cache<T>&) = delete;
        resource_cache(resource_cache<T>&&) = delete;
        virtual ~resource_cache();
        resource_cache<T>& operator=(const resource_cache<T>&) = delete;
        resource_cache<T>& operator=(resource_cache<T>&&) = delete;


        std::size_t last_change() const { return _lastChange; }

        std::size_t count() const { return _data.size(); }

        /**
         * @brief Reference count of given resource
         */
        std::size_t reference_count(const resource_key& key) const;

        resource_state state(const resource_key& key) const;

        resource<T> get(const resource_key& key);

        /**
         * @brief Set resource data
         * 
         * Resources with resource_policy::reference_counted are added with
         * zero reference count. It means that all reference counted resources
         * which were only loaded but not used will stay loaded and you need to
         * explicitly call free() to delete them.
         * 
         * ATTENTION: Subsequent updates are not possible if resource state is
         *      already @ref resource_state::loaded_final.
         */
        void set(const resource_key& key, T* data, resource_data_state state, resource_policy policy);

        /**
         * @brief Free all resources which are not referenced (refcount is zero)
         * Will not free the resources that are resident.
         */
        void free();

        /**
         * @brief Clear all resources of given type
         *
         * Unlike free() this function assumes that no resource is referenced.
         */
        void clear() { _data.clear(); }


        T* fallback() { return _fallback; }
        void set_fallback(T* data);



        resource_loader<T>* loader() { return _loader; }
        const resource_loader<T>* loader() const { return _loader; }

        void free_loader();

        /**
         * @brief Set loader for given type of resources
         * @param loader    Loader or @cpp nullptr @ce if unsetting previous
         *      loader.
         * @return Reference to self (for method chaining)
         *
         * See @ref abstract_resource_loader documentation for more information.
         * @attention The loader is deleted on destruction before unloading
         *      all resources.
         */
        void set_loader(resource_loader<T>* loader);


    private:
        struct Data;

        const Data& data(const resource_key& key) { return _data[key]; }

        void incrementReferenceCount(const resource_key& key) {
            ++_data[key].referenceCount;
        }

        void decrementReferenceCount(const resource_key& key);

        std::unordered_map<resource_key, Data> _data;
        T* _fallback;
        resource_loader<T>* _loader;
        std::size_t _lastChange;
};

namespace impl {
    template<class T> void safeDelete(T* data) {
        static_assert(sizeof(T) > 0, "Cannot delete pointer to incomplete type");
        delete data;
    }
}

template<class T> resource_cache<T>::~resource_cache() {
    free_loader();
    impl::safeDelete(_fallback);
}

template<class T> std::size_t resource_cache<T>::reference_count(const resource_key& key) const {
    auto it = _data.find(key);
    if(it == _data.end()) return 0;
    return it->second.referenceCount;
}

template<class T> resource_state resource_cache<T>::state(const resource_key& key) const {
    const auto it = _data.find(key);

    /* resource not loaded */
    if(it == _data.end() || !it->second.data) {
        /* Fallback found, add *Fallback to state */
        if(_fallback) {
            if(it != _data.end() && it->second.state == resource_data_state::loading)
                return resource_state::loading_fallback;
            else if(it != _data.end() && it->second.state == resource_data_state::not_found)
                return resource_state::not_found_fallback;
            else return resource_state::not_loaded_fallback;
        }

        /* Fallback not found, loading didn't start yet */
        if(it == _data.end() || (it->second.state != resource_data_state::loading && it->second.state != resource_data_state::not_found))
            return resource_state::not_loaded;
    }

    /* loading / not_found without fallback, loaded_mutable / loaded_final */
    return static_cast<resource_state>(it->second.state);
}

template<class T> resource<T> resource_cache<T>::get(const resource_key& key) {
    /* Ask loader for the data, if they aren't there yet */
    if(_loader && _data.find(key) == _data.end())
        _loader->load(key);

    return resource<T>(this, key);
}

template<class T> void resource_cache<T>::set(const resource_key& key, T* const data, const resource_data_state state, const resource_policy policy) {
    auto it = _data.find(key);

    /* not_found / loading state shouldn't have any data */
    dscheckm((data == nullptr) == (state == resource_data_state::not_found || state == resource_data_state::loading),
        "resource_cache::set(): data should be null if and only if state is NotFound or Loading");

    /* Cannot change resource with already final state */
    dscheckm(it == _data.end() || it->second.state != resource_data_state::loaded_final,
        std::format("resource_cache::set(): cannot change already final resource key: {}", key.key));

    /* Insert the resource, if not already there */
    if(it == _data.end())
        it = _data.emplace(key, Data()).first;

    /* Otherwise delete previous data */
    else impl::safeDelete(it->second.data);

    it->second.data = data;
    it->second.state = state;
    it->second.policy = policy;
    ++_lastChange;
}

template<class T> void resource_cache<T>::set_fallback(T* const data) {
    impl::safeDelete(_fallback);
    _fallback = data;
    /* Notify resources also in this case, as some of them could go from empty
       to a fallback (or from a fallback to empty) */
    ++_lastChange;
}

template<class T> void resource_cache<T>::free() {
    /* Delete all non-referenced non-resident resources */
    for(auto it = _data.begin(); it != _data.end(); ) {
        if(it->second.policy != resource_policy::resident && !it->second.referenceCount)
            it = _data.erase(it);
        else ++it;
    }
}

template<class T> void resource_cache<T>::set_loader(resource_loader<T>* const loader) {
    /* Delete previous loader */
    delete _loader;

    /* Add new loader */
    _loader = loader;
    if(_loader) _loader->manager = this;
}

template<class T> void resource_cache<T>::free_loader() {
    if(!_loader) return;

    _loader->manager = nullptr;
    delete _loader;
}

template<class T> void resource_cache<T>::decrementReferenceCount(const resource_key& key) {
    auto it = _data.find(key);
    dsverify(it != _data.end());

    /* Free the resource if it is reference counted */
    if(--it->second.referenceCount == 0 && it->second.policy == resource_policy::reference_counted)
        _data.erase(it);
}

template<class T> struct resource_cache<T>::Data {
    Data(): data(nullptr), state(resource_data_state::loaded_mutable), policy(resource_policy::manual), referenceCount(0) {}

    Data(const Data&) = delete;

    Data(Data&& other) noexcept: data{other.data}, state{other.state}, policy{other.policy}, referenceCount{other.referenceCount} {
        other.data = nullptr;
        other.referenceCount = 0;
    }

    ~Data() {
        dscheckm(referenceCount == 0, "resource_cache: cleared/destroyed while data are still referenced");
        impl::safeDelete(data);
    };

    Data& operator=(const Data&) = delete;
    Data& operator=(Data&&) = delete;

    T* data;
    resource_data_state state;
    resource_policy policy;
    std::size_t referenceCount;
};


}

namespace std {
    /** @brief `std::hash` specialization for @ref ds::resource_key */
    template<> struct hash<ds::resource_key> {
        size_t operator()(const ds::resource_key& key) const {
            return (size_t)key.hashed_key;
        }
    };
}

/* Make the definition complete */
#include "destral_resource_loader.h"


