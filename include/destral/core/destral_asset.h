//#pragma once
//
//#include <unordered_map>
//
//namespace ds {
//
//    enum class resource_data_state {
//        Loading,
//        NotFound,
//        Mutable,
//        Final
//    };
//
//
//    struct resource_key {
//        resource_key() = default;
//        resource_key(const std::string& res_key, const std::string& res_type_key) {
//            key = res_key;
//            type_key = res_type_key;
//            key_id = ds::fnv1a_64bit(res_key);
//            type_id = ds::fnv1a_64bit(res_type_key);
//        }
//        resource_key(const resource_key& res_key) : resource_key(res_key.key, res_key.type_key) {}
//        std::string key;
//        std::string type_key;
//        // hashed
//        ds::u64 key_id = 0;
//        ds::u64 type_id = 0;
//    };
//    inline bool operator== (resource_key const& lhs, resource_key const& rhs) { return (lhs.key_id == lhs.key_id) && (lhs.type_id == lhs.type_id); }
//
//    struct resource_desc {
//        resource_key key;
//        std::string file;
//    };
//
//    struct resource {
//        resource() = default;
//        resource(const resource_key& res_key) {
//            _impl.key = res_key;
//        }
//
//        template<typename T> T* get() {
//            acquire();
//            return static_cast<T>(data);
//        }
//
//        void* data() {
//            _impl.acquire();
//            return _impl.data;
//        }
//
//        resource_key key() {
//            _impl.key;
//        }
//
//        bool valid() {
//            return data() != nullptr;
//        }
//
//        struct impl {
//            // implementation details
//            resource_key key;
//            class resource_cache* cache = nullptr;
//            void* data = nullptr;
//            void acquire() {
//                /* The data are already final, nothing to do */
//                //if (_state == ResourceState::Final) return;
//
//                /* Nothing changed since last check */
//                //if (_manager->lastChange() <= _lastCheck) return;
//
//                /* Acquire new data and save last check time */
//                //const typename Implementation::ResourceManagerData<T>::Data& d = _manager->data(_key);
//                // _lastCheck = _manager->lastChange();
//
//                /* Try to get the data */
//                //_data = d.data;
//                //_state = static_cast<ResourceState>(d.state);
//
//                /* Data are not available */
//                //if (!data) {
//                //    /* Fallback found, add *Fallback to state */
//                //    if ((_data = _manager->fallback())) {
//                //        if (_state == ResourceState::Loading)
//                //            _state = ResourceState::LoadingFallback;
//                //        else if (_state == ResourceState::NotFound)
//                //            _state = ResourceState::NotFoundFallback;
//                //        else _state = ResourceState::NotLoadedFallback;
//
//                //        /* Fallback not found and loading didn't start yet */
//                //    } else if (_state != ResourceState::Loading && _state != ResourceState::NotFound)
//                //        _state = ResourceState::NotLoaded;
//                //}
//            }
//
//        };
//        impl _impl;
//
//
//
//
//    };
//
//    struct resource_loader {
//        std::string type_name;
//        void* (*create_default_fn)(void) = nullptr;
//        void (*deleter_fn)(void*) = nullptr;
//        void* (*create_from_file_fn)(const std::string& path) = nullptr;
//        bool (*can_create_from_file_fn)(const std::string& path) = nullptr;
//    };
//
//
//    struct resource_manager {
//        /// Asset descriptions
//        void add_description(const resource_desc& res_desc) {
//            dsverifym(!descriptions.contains(res_desc.key.key_id), std::format("Resource description: {} already exists", res_desc.key.key));
//            descriptions[res_desc.key.key_id] = res_desc;
//        }
//
//        // Returns the pointer to a description or nullptr if the key id and key type name doesn't exists/match
//        resource_desc* get_description(const resource_key& key) {
//            auto it = descriptions.find(key.key_id);
//            if (it != descriptions.end()) {
//                if (it->second.key == key) {
//                    return &it->second;
//                }
//            }
//            return nullptr;
//        }
//        
//        // res key id -> resource_desc
//        std::unordered_map<u64, resource_desc> descriptions;
//
//        ///////////////////////////////////
//
//        // LOADERS
//        void add_loader(const resource_loader& loader) {
//            dsverifym(!loaders.contains(ds::fnv1a_64bit(loader.type_name)), std::format("Resource Loader: {} already exists", loader.type_name));
//            loaders[ds::fnv1a_64bit(loader.type_name)] = loader;
//        }
//
//        resource_loader* get_loader(const resource_key& key) {
//            auto it = loaders.find(key.type_id);
//            if (it != loaders.end()) {
//                return &it->second;
//            }
//            return nullptr;
//        }
//
//        void* load_default(const resource_key& k) {
//            auto loader = get_loader(k);
//            if (loader && loader->create_default_fn) {
//                return loader->create_default_fn();
//            }
//            return nullptr;
//        }
//
//        void* load_from_file(const resource_key& k, const std::string& file) {
//            auto loader = get_loader(k);
//            if (loader) {
//                if (loader->can_create_from_file_fn) {
//                    if (loader->can_create_from_file_fn(file)) {
//                        return loader->create_from_file_fn(file);
//                    }
//                }
//            }
//            return nullptr;
//        }
//
//        void loader_delete_resource(const resource_key& k) {
//            auto loader = get_loader(k);
//            if (loader) {
//                if (loader->can_create_from_file_fn) {
//                    if (loader->can_create_from_file_fn(file)) {
//                        return loader->create_from_file_fn(file);
//                    }
//                }
//            }
//            return nullptr;
//        }
//
//
//
//        std::unordered_map<u64, resource_loader> loaders;
//        ///////////////////////////////////
//
//        // 
//        resource get(const resource_key& k) {
//
//        }
//
//        struct cache {
//            // key_id to instance
//            std::unordered_map<u64, void*> instances;
//        };
//
//        void* get_from_cache(u64 key_id) {
//
//        }
//
//        void remove_from_cache(u64 key_id) {
//
//        }
//
//
//        std::unordered_map<u64, cache> types_caches;
//
//
//
//    };
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//#include "destral/ecs/destral_ecs.h"
//
//namespace ds {
//	
//
//	struct asset_registry;
//	struct asset { 	entity id = entity_null;	};
//	inline bool operator== (asset const& lhs, asset const& rhs) {return (lhs.id == lhs.id);	}
//
//
//	asset_registry* asset_registry_create();
//	void asset_registry_destroy(asset_registry* ar);
//
//	void asset_register_asset_factory(asset_registry* ar, const std::string& asset_type_name,
//		void* (*create_default_fn)(void) = nullptr, 
//		void (*deleter_fn)(void *) = nullptr,
//		void* (*create_from_file_fn)(const std::string& path) = nullptr,
//		bool (*can_create_from_file_fn)(const std::string& path) = nullptr);
//
//	template <typename T>void asset_register_asset_factory(asset_registry* ar, const std::string& asset_type_name, 
//		void* (*create_from_file_fn)(const std::string& path) = nullptr, bool (*can_create_from_file_fn)(const std::string& path) = nullptr) {
//		asset_register_asset_factory(ar, asset_type_name, []() { new T(); }, [](void* mem_cp) { ((T*)mem_cp)->~T(); }, create_from_file_fn, can_create_from_file_fn);
//	}
//
//	asset asset_create(asset_registry* ar, const std::string& asset_type_name);
//	void asset_destroy(asset_registry* ar, asset a);
//	asset asset_create_from_file(asset_registry* ar, const std::string& file);
//	void* asset_get(asset_registry* ar, asset id, const std::string* asset_type);
//	template <typename T> T* asset_get(asset_registry* ar, asset id, const std::string* asset_type) { return static_cast<T*>(asset_get(ar, id, asset_type)); }
//	
////	/**
////* This function will create an asset from file and return the id of the newly created id.
////* Internally this function will search for an appropiate type factory to load that file.
////* returns type_id_null if the asset can't be created
////*/
////	asset asset_create_from_file(const std::string& file);
////
////	/**
////	* This function will create an asset using the default construction and will return the newly created id
////	* returns type_id_null if the asset can't be created
////	*/
////	asset asset_create_from_name(const std::string& asset_type);
//
//
//		/**
//	* This will return the asset id object requested checked with the type_id.
//	* Returns nullptr if the id is not valid.
//	* Will crash the application if the id is valid but not equal to the type_id supplied.
//	*/
//}