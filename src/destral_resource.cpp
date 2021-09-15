#include <destral/destral_resource.h>
#include <unordered_map>




namespace ds {



    class resource_cache {
        friend class resource;
    public:
        static constexpr const char* ctx_name = "ds_resource_cache";
        resource_cache(registry* _r) : r{ _r } {}
        ~resource_cache() {
            //free_loader();
            //impl::safeDelete(_fallback);
        }

        i32 last_change() const { return _lastChange; }
        i32 count() const { return (i32)_data.size(); }

        /**
         * @brief Reference count of given resource
         */
        i32 reference_count(const resource_key& key) const {
            auto it = _data.find(key.hashed_key);
            if (it == _data.end()) return 0;
            return it->second.referenceCount;
        }

        resource_state state(const resource_key& key) const {
            const auto it = _data.find(key.hashed_key);

            /* resource not loaded */
            if (it == _data.end() || (it->second.data == entity_null)) {
                /* Fallback found, add *Fallback to state */
                if (_fallbacks.contains(key.hashed_key)) {
                    if (it != _data.end() && it->second.state == resource_data_state::loading) {
                        return resource_state::loading_fallback;
                    } else if (it != _data.end() && it->second.state == resource_data_state::not_found) {
                        return resource_state::not_found_fallback;
                    } else {
                        return resource_state::not_loaded_fallback;
                    }
                }

                /* Fallback not found, loading didn't start yet */
                if (it == _data.end() || (it->second.state != resource_data_state::loading && it->second.state != resource_data_state::not_found)) {
                    return resource_state::not_loaded;
                }
            }

            /* loading / not_found without fallback, loaded_mutable / loaded_final */
            return static_cast<resource_state>(it->second.state);
        }


        resource get(const resource_key& key) {
            if (!_data.contains(key.hashed_key)) { // resource data not found
                load_resource(key);
            }

            return resource(this, key);
        }
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
        void set(const resource_key& key, entity data, const resource_data_state state, const resource_policy policy) {
            /* not_found / loading state shouldn't have any data */
            dscheckm((!r->entity_valid(data)) == (state == resource_data_state::not_found || state == resource_data_state::loading),
                "resource_cache::set(): data should be null if and only if state is NotFound or Loading");

            /* Cannot change resource with already final state */
            auto it = _data.find(key.hashed_key);
            dscheckm(it == _data.end() || it->second.state != resource_data_state::loaded_final,
                std::format("resource_cache::set(): cannot change already final resource key: {}", key.key));


            if (it == _data.end()) {
                /* Insert the resource, if not already there */
                it = _data.emplace(key.hashed_key, r).first;
            } else {
                /* Otherwise delete previous data */
                r->entity_destroy_delayed(it->second.data);
            }

            it->second.data = data;
            it->second.state = state;
            it->second.policy = policy;
            ++_lastChange;
        }


        /**
         * @brief Free all resources which are not referenced (refcount is zero)
         * Will not free the resources that are resident.
         */
        void free() {

        }

        /**
         * @brief Clear all resources of given type
         *
         * Unlike free() this function assumes that no resource is referenced.
         */
        void clear() { _data.clear(); }

        // Returns the fallback entity if any for an entity type. If no fallback found returns entity_null;
        entity fallback(i32 entity_type_id) { 
            auto it = _fallbacks.find(entity_type_id);
            if (it != _fallbacks.end()) {
                return it->second;
            }
            return entity_null;
        }

        void set_fallback(const char* entity_type, entity fallback_entity) {
            dsverify(_fallbacks.contains(fallback_entity.type_id) == false);
            _fallbacks[fallback_entity.type_id] = fallback_entity;
        }


        void set_loader(resource_loader loader) {
            _loaders.push_back(loader);
        }

    private:
        void load_resource(const resource_key& key) {
            for (i32 i = 0; i < _loaders.size(); i++) {
                if (_loaders[i].load_fn && _loaders[i].can_load_resource_fn && _loaders[i].can_load_resource_fn(r, key)) {
                    _loaders[i].load_fn(r, key);
                }
            }
        }


        struct resource_data {
            resource_data() {}
            resource_data(registry* r) {
                _r = r;
            }

            ~resource_data() {
                dscheckm(referenceCount == 0, "resource_cache: cleared/destroyed while data are still referenced");
                _r->entity_destroy_delayed(data);
            };

            resource_data& operator=(const resource_data&) = delete;
            resource_data& operator=(resource_data&&) = delete;

            registry* _r = nullptr;
            entity data = entity_null;
            resource_data_state state = resource_data_state::loaded_mutable;
            resource_policy policy = resource_policy::manual;
            i32 referenceCount = 0;
        };

        const resource_data& data(const resource_key& key) { return _data[key.hashed_key]; }

        void incrementReferenceCount(const resource_key& key) {
            _data[key.hashed_key].referenceCount++;
        }

        void decrementReferenceCount(const resource_key& key) {
            auto it = _data.find(key.hashed_key);
            dscheck(it != _data.end());

            /* Free the resource if it is reference counted */
            if (--it->second.referenceCount == 0 && it->second.policy == resource_policy::reference_counted) {
                _data.erase(it);
            }
        }
        //resource_key -> Data
        std::unordered_map<i32, resource_data> _data;


        std::unordered_map<i32, entity> _fallbacks; // resource type id -> fallback entity
      
        ds::darray<resource_loader> _loaders;

        i32 _lastChange = 0;
        registry* r = nullptr;
    };

    resource_cache* s_checked_get_resource_cache(registry* r) {
        resource_cache* c = r->ctx_get<resource_cache>(resource_cache::ctx_name);
        dscheck(c);
        return c;
    }

    void resource_init(registry* r) {
        dsverify(r->ctx_get<resource_cache>(resource_cache::ctx_name) == nullptr);
        r->ctx_set<resource_cache>(resource_cache::ctx_name, new resource_cache(r));
    }

    void resource_deinit(registry* r) {
        r->ctx_unset(resource_cache::ctx_name);
    }


    resource resource_get(registry* r, const resource_key& key) {
        auto rc = s_checked_get_resource_cache(r);
        return rc->get(key);
    }

    void resource_set(registry* r, const resource_key& key, entity data, const resource_data_state state, const resource_policy policy) {
        auto rc = s_checked_get_resource_cache(r);
        rc->set(key, data, state, policy);
    }

    void resource_set_fallback(registry* r, const char* entity_type, entity data) {
        auto rc = s_checked_get_resource_cache(r);
        rc->set_fallback(entity_type, data);
    }

    entity resource_get_fallback(registry* r, const char* entity_type_id) {
        auto rc = s_checked_get_resource_cache(r);
        const auto type_id = fnv1a_32bit(entity_type_id);
        return rc->fallback(type_id);
    }

    void resource_set_loader(registry* r, resource_loader loader) {
        auto rc = s_checked_get_resource_cache(r);
        rc->set_loader(loader);
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////


    /** @brief Copy constructor */
    resource::resource(const resource& other) :
        _manager{ other._manager }, _key{ other._key }, _lastCheck{ other._lastCheck }, _state{ other._state }, _data{ other._data } {
        if (_manager) _manager->incrementReferenceCount(_key);
    }

    resource::~resource() {
        if (_manager) _manager->decrementReferenceCount(_key);
    }

    resource::resource(resource_cache* manager, resource_key key) :
        _manager{ manager }, _key{ key }, _lastCheck{ 0 }, _state{ resource_state::not_loaded }, _data{ entity_null } {
        manager->incrementReferenceCount(key);
    }

    bool resource::operator==(const resource& other) const {
        return _manager == other._manager && _key == other._key;
    }
    bool resource::operator!=(const resource& other) const {
        return !operator==(other);
    }
    resource& resource::operator=(const resource& other) {
        if (_manager) _manager->decrementReferenceCount(_key);

        _manager = other._manager;
        _key = other._key;
        _lastCheck = other._lastCheck;
        _state = other._state;
        _data = other._data;

        if (_manager) _manager->incrementReferenceCount(_key);
        return *this;
    }

    void resource::acquire() {
        /* The data are already final, nothing to do */
        if (_state == resource_state::loaded_final) return;

        /* Nothing changed since last check */
        if (_manager->last_change() <= _lastCheck) return;

        /* Acquire new data and save last check time */
        const resource_cache::resource_data& d = _manager->data(_key);
        _lastCheck = _manager->last_change();

        /* Try to get the data */
        _data = d.data;
        _state = static_cast<resource_state>(d.state);

        /* Data are not available */
        if (_data == entity_null) {
            /* Fallback found, add *Fallback to state */
            _data = _manager->fallback(_key.hashed_key);
            if (_data != entity_null) {
                if (_state == resource_state::loading) {
                    _state = resource_state::loading_fallback;
                } else if (_state == resource_state::not_found) {
                    _state = resource_state::not_found_fallback;
                } else { 
                    _state = resource_state::not_loaded_fallback; 
                }

                /* Fallback not found and loading didn't start yet */
            } else if (_state != resource_state::loading && _state != resource_state::not_found) {
                _state = resource_state::not_loaded;
            }
        }
    }






















}









//
//namespace ds {
//
//	struct resource_reference {
//		static void serialize(registry* r, entity e, void* cp, bool reading) {
//			if (reading) {
//				auto rf = ((resource_reference*)cp);
//				rf->e = e;
//				rf->r = r;
//			}
//		}
//		entity e; // reference to the entity from this component
//		registry* r; // reference to the registry
//		std::string resource_key;
//		i32 resource_id;
//	};
//
//
//	struct resource_loader {
//		resource_load_from_file_fn* load_from_file_fn = nullptr;
//		std::string entity_type;
//		darray<std::string> extensions;
//	};
//
//
//	struct resource_data {
//		~resource_data() {
//			dsverify(ref_count == 0, "cleared/destroyed while data are still referenced");
//			r->entity_destroy_delayed(instance);
//		}
//		entity instance = entity_null; // resource instance
//		resource_data_state state = resource_data_state::loaded_mutable;
//		resource_policy policy = resource_policy::manual;
//		i32 ref_count = 0;
//		registry* r;
//	};
//
//	struct resource_cache {
//		static constexpr const char* ctx_name = "ds_resource_cache";
//
//		// type_id -> loader(for that type)
//		std::unordered_map<i32, resource_loader> loaders;
//
//		// key (file_path hashed) -> resource_data
//		std::unordered_map<i32, resource_data> entities;
//
//
//		void set(registry* r, i32 key, entity data, resource_data_state state, resource_policy policy) {
//			/* not_found / loading state shouldn't have any data */
//			if (state == resource_data_state::not_found || state == resource_data_state::loading) {
//				dsverifym((data == entity_null), "Data should be null if and only if state is not_found or loading");
//			}
//
//			auto it = entities.find(key);
//			/* Cannot change resource with already final state */
//			dsverifym(it == entities.end() || it->second.state != resource_data_state::loaded_final,
//				std::format("Cannot change already final resource key: {}", key));
//			/* Insert the resource, if not already there */
//			if (it == entities.end()) {
//				it = entities.emplace(key, resource_data()).first;
//			} else {  /* Otherwise delete previous data */
//				r->entity_destroy_delayed(it->second.instance);
//			}
//			it->second.instance = data;
//			it->second.state = state;
//			it->second.policy = policy;
//			it->second.r = r;
//			//TODO ++lastChange;
//		}
//
//		
//		//* Ask loader for the data, if they aren't there yet */
//		//    if(_loader && _data.find(key) == _data.end())
//		//        _loader->load(key);
//		//
//		//    return resource<T>(this, key);
//		//}
//
//		//entity get(i32 resource_id) {
//		//	if (entities.contains(resource_id)) {
//		//		// Entity is in the cache
//		//		return entities[cache_id];
//		//	} else {
//		//		return entity_null;
//		//	}
//		//}
//
//		resource_loader* get_loader(const char* file_path) {
//			/*std::string extension = fs::path(file_path).extension().string();
//			for (i32 i = 0; i < loaders.size(); i++) {
//				if (loaders[i].extensions.contains(extension)) {
//					return &loaders[i];
//				}
//			}*/
//			return nullptr;
//		}
//
//		resource get(registry* r, resource_loader* rl, i32 resource_id, const char* file_path) {
//			if (!rl) {
//				DS_TRACE(std::format("Can't find a loader for the resource id/path: {}", file_path));
//				resource res;
//				res._r = r;
//				res._resource_reference = r->entity_make(resource_ref)
//				return resource{._r = r, };
//			} else {
//				if (entities.contains(resource_id)) {
//					return resource{ r, key };
//				}
//				entity e = rl->load_from_file_fn(r, file_path);
//				if (e == entity_null) {
//					DS_WARNING(std::format("Error loading the resource id/path: {}", file_path));
//				} else {
//					entities[cache_id] = e;
//				}
//				return e;
//			}
//		}
//	};
//
//	resource_cache* s_checked_get_resource_cache(registry* r) {
//		resource_cache* c = r->ctx_get<resource_cache>(resource_cache::ctx_name);
//		dscheck(c);
//		return c;
//	}
//
//    void resource_ctx_init(registry* r) {
//		r->ctx_set<resource_cache>(resource_cache::ctx_name, new resource_cache());
//    }
//    
//    void resource_set(registry* r, i32 key, entity data, resource_data_state state, resource_policy policy) {
//		s_checked_get_resource_cache(r)->set(r, key, data, state, policy);
//
//    }
//
//    resource resource_get(i32 key) {
//		return {};
//
//    }
//
//	void resource_set_loader(registry* r, const char* entity_to_load, resource_load_from_file_fn* load_fn, const darray<std::string>& exts) {
//		resource_cache* c = s_checked_get_resource_cache(r);
//		const i32 entity_id = ds::fnv1a_32bit(entity_to_load);
//		dsverify(!c->loaders.contains(entity_id));
//		resource_loader rl;
//		rl.entity_type = entity_to_load;
//		rl.load_from_file_fn = load_fn;
//		rl.extensions = exts;
//		c->loaders.insert_or_assign(entity_id, rl);
//	}
//
//
//}