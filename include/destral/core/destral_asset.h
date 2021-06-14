#pragma once

#include "destral/ecs/destral_ecs.h"

namespace ds {
	

	struct asset_registry;
	struct asset { 	entity id = entity_null;	};
	inline bool operator== (asset const& lhs, asset const& rhs) {return (lhs.id == lhs.id);	}


	asset_registry* asset_registry_create();
	void asset_registry_destroy(asset_registry* ar);

	void asset_register_asset_factory(asset_registry* ar, const std::string& asset_type_name,
		void* (*create_default_fn)(void) = nullptr, 
		void (*deleter_fn)(void *) = nullptr,
		void* (*create_from_file_fn)(const std::string& path) = nullptr,
		bool (*can_create_from_file_fn)(const std::string& path) = nullptr);

	template <typename T>void asset_register_asset_factory(asset_registry* ar, const std::string& asset_type_name, 
		void* (*create_from_file_fn)(const std::string& path) = nullptr, bool (*can_create_from_file_fn)(const std::string& path) = nullptr) {
		asset_register_asset_factory(ar, asset_type_name, []() { new T(); }, [](void* mem_cp) { ((T*)mem_cp)->~T(); }, create_from_file_fn, can_create_from_file_fn);
	}

	asset asset_create(asset_registry* ar, const std::string& asset_type_name);
	void asset_destroy(asset_registry* ar, asset a);
	asset asset_create_from_file(asset_registry* ar, const std::string& file);
	void* asset_get(asset_registry* ar, asset id, const std::string* asset_type);
	template <typename T> T* asset_get(asset_registry* ar, asset id, const std::string* asset_type) { return static_cast<T*>(asset_get(ar, id, asset_type)); }
	
//	/**
//* This function will create an asset from file and return the id of the newly created id.
//* Internally this function will search for an appropiate type factory to load that file.
//* returns type_id_null if the asset can't be created
//*/
//	asset asset_create_from_file(const std::string& file);
//
//	/**
//	* This function will create an asset using the default construction and will return the newly created id
//	* returns type_id_null if the asset can't be created
//	*/
//	asset asset_create_from_name(const std::string& asset_type);


		/**
	* This will return the asset id object requested checked with the type_id.
	* Returns nullptr if the id is not valid.
	* Will crash the application if the id is valid but not equal to the type_id supplied.
	*/
}