#ifndef Magnum_AbstractResourceLoader_h
#define Magnum_AbstractResourceLoader_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class Magnum::resource_loader
 */

#include <string>

//#include "resource_cache.h"

namespace ds {

    /**
    @brief Base for resource loaders

    Provides (a)synchronous resource loading for resource_cache.

    @section resource_loader-usage Usage and subclassing

    Usage is done by subclassing. Subclass instances can be added to
    resource_cache using resource_cache::set_loader(). After adding the
    loader, each call to resource_cache::get() will call load()
    implementation unless the resource is already loaded (or loading is in
    progress). Note that resources requested before the loader was added are not
    affected by the loader.

    Subclassing is done by implementing at least do_load() function. The
    loading can be done synchronously or asynchronously (i.e., in another thread).
    The base implementation provides interface to resource_cache and manages
    loading progress (which is then available through functions requestedCount(),
    loadedCount() and not_found_count()).

    IMPORTANT: You shouldn't access the resource_cache directly when loading the data.

    In your do_load() implementation, after your resources are loaded, call
    set() to pass them to resource_cache or call set_not_found() to
    indicate that the resource was not found.


    Example implementation for synchronous mesh loader:

        class MeshResourceLoader: public AbstractResourceLoader<GL::Mesh> {
            void do_load(ResourceKey key) override {
                // Load the mesh...

                // Not found
                if(!found) {
                    setNotFound(key);
                    return;
                }

                // Found, pass it to resource manager
                set(key, mesh_ptr);
            }
        };

    You can then add it to resource manager instance like this. Note that the
    manager automatically deletes the all loaders on destruction before unloading
    all resources. It allows you to use resources in the loader itself without
    having to delete the loader explicitly to ensure proper resource unloading. In
    the following code, however, the loader destroys itself (and removes itself
    from the manager) before the manager is destroyed.

        resource_cache manager;
        mesh_resource_loader *loader = new mesh_resource_loader();

        manager.setLoader<GL::Mesh>(loader);

        // This will now automatically request the mesh from loader by calling load()
        resource<GL::Mesh> myMesh = manager.get<GL::Mesh>("my-mesh");

    */
    template<class T> class resource_loader {
    public:
        explicit resource_loader() : manager(nullptr), _requestedCount(0), _loadedCount(0), _notFoundCount(0) {}

        virtual ~resource_loader();

        /**
         * @brief Count of requested resources
         *
         * Count of resources requested by calling load().
         */
        std::size_t requested_count() const { return _requestedCount; }

        /**
         * @brief Count of not found resources
         *
         * Count of resources requested by calling load(), but not found
         * by the loader.
         */
        std::size_t not_found_count() const { return _notFoundCount; }

        /**
         * @brief Count of loaded resources
         *
         * Count of resources requested by calling load(), but not found
         * by the loader.
         */
        std::size_t loaded_count() const { return _loadedCount; }

        /**
         * @brief Request resource to be loaded
         *
         * If the resource isn't yet loaded or loading, state of the resource
         * is set to resource_state::loading and count of requested
         * features is incremented. Depending on implementation the resource
         * might be loaded synchronously or asynchronously.
         *
         * @see resource_cache::state(), requestedCount(),
         *      notFoundCount(), loadedCount()
         */
        void load(const resource_key& key);

    protected:
        /**
         * @brief Set loaded resource to resource manager
         *
         * If data is nullptr and state is
         * resource_data_state::not_found, increments count of not found
         * resources. Otherwise, if data is not nullptr @ce, increments
         * count of loaded resources. See resource_cache::set() for
         * more information.
         *
         * Note that resource's state is automatically set to
         * resource_data_state::loading when it is requested from
         * resource_cache and it's not loaded yet, so it's not needed to
         * call this function. For marking a resource as not found you can also
         * use the convenience set_not_found() variant.
         * @see loadedCount()
         */
        void set(const resource_key& key, T* data, resource_data_state state, resource_policy policy);

        /**
         * @brief Set loaded resource to resource manager
         *
         * Same as above function with state set to resource_data_state::loaded_final
         * and policy to resource_policy::resident.
         */
        void set(const resource_key& key, T* data) {
            set(key, data, resource_data_state::loaded_final, resource_policy::resident);
        }

        /**
         * @brief Mark resource as not found
         *
         * A convenience function calling set() with nullptr @ce
         * and resource_data_state::not_found.
         * @see notFoundCount()
         */
        void set_not_found(const resource_key& key) {
            /** @todo What policy for notfound resources? */
            set(key, nullptr, resource_data_state::not_found, resource_policy::resident);
        }


    private:
        /**
         * @brief Implementation for load()
         *
         * See class documentation for reimplementation guide.
         */
        virtual void do_load(const resource_key& key) = 0;

    private:
        friend resource_cache<T>;

        resource_cache<T>* manager;
        std::size_t _requestedCount,
            _loadedCount,
            _notFoundCount;
    };

    template<class T> resource_loader<T>::~resource_loader() {
        if (manager) manager->_loader = nullptr;
    }



    template<class T> void resource_loader<T>::load(const resource_key& key) {
        ++_requestedCount;
        /** @todo What policy for loading resources? */
        manager->set(key, nullptr, resource_data_state::loading, resource_policy::resident);

        do_load(key);
    }

    template<class T> void resource_loader<T>::set(const resource_key& key, T* data, resource_data_state state, resource_policy policy) {
        if (data) ++_loadedCount;
        if (!data && state == resource_data_state::not_found) ++_notFoundCount;
        manager->set(key, data, state, policy);
    }

}

#endif
