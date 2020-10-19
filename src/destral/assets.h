#pragma once
#include <functional>
#include <string>
#include "nlohmann/json_fwd.hpp"
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>



// WARNING This module is under development/testing
// WARNING This module is under development/testing
// WARNING This module is under development/testing

/**

Com funcionara l'asset Manager?

Basicament, llegirà tots els fitxer d'un directori i cercarà fitxers .dasset

Aquests fitxers seran un json amb la descripcio de l'asset que permetra carregar-lo.



Per començar tindrem un registre d'assets. Aquest registre de tipus d'assets contindrà els identificadors
i una descripcio de cada tipus d'asset que podrem carregar/descarregar.




*/
namespace ds::as {

	using id = entt::entity;
	constexpr id id_null = entt::null;
	using type_id = std::uint32_t;
	constexpr type_id type_id_null = 0;

	struct factory_type {
		// factory_type asset unique identifier
		type_id type_id = type_id_null;

		// Just a helper name for that type (not used to identify anything)
		std::string type_name = "unknown";

		// function to check if a file can be imported with this factory
		std::function <bool(const std::string& file)> can_import_from_file = nullptr;

		// function to create an std::any from file
		std::function<std::unique_ptr<void, std::function<void(void*)> > (const std::string& file)> create_from_file = nullptr;

		// function to create an std::any default
		std::function<std::unique_ptr<void, std::function<void(void*)> > ()> create_default = nullptr;

		std::function<void (nlohmann::json& j)> serialize = nullptr;

		std::function< std::unique_ptr<void, std::function<void(void*)> > (const nlohmann::json& j)> deserialize = nullptr;
	};

	inline bool operator== (factory_type const& lhs, factory_type const& rhs) {
		return (lhs.type_id == lhs.type_id);
	}

	void register_factory_type(const factory_type& asset_t);

	/**
	* This doesn't do anything right now (TODO)
	*/
	void init();

	/**
	* This will free all the asset objects.
	*/
	void shutdown();

	/**
	* This function will create an asset from file and return the id of the newly created id.
	* Internally this function will search for an appropiate type factory to load that file.
	* returns type_id_null if the asset can't be created
	*/
	id create_from_file(const std::string& file);

	/**
	* This function will create an asset using the default construction and will return the newly created id
	* returns type_id_null if the asset can't be created
	*/
	id create_from_type_id(type_id type_id);

	/**
	* This will return the asset id object requested checked with the type_id.
	* Returns nullptr if the id is not valid.
	* Will crash the application if the id is valid but not equal to the type_id supplied.
	*/
	void* get_ptr(id id, type_id type_id);


	// get an asset object pointer by id and casted to the correct type checking it's type_id using the templated type
	template<typename T>
	T* get(id id) {
		void* instance = get_ptr(id, T::type_id);
		if (instance) {
			return static_cast<T*>(instance);
		} else {
			return nullptr;
		}
	}
	
	// Create an asset by type id using templated type_id
	template<typename T>
	id create() {
		return create_from_type_id(T::type_id);
	}
}