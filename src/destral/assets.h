#pragma once
//#include <ap_uuid.h>
#include <functional>
#include <any>
#include <string>
#include "nlohmann/json_fwd.hpp"
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>

/**
Com funcionara l'asset Manager?

Basicament, llegirà tots els fitxer d'un directori i cercarà fitxers .dasset

Aquests fitxers seran un json amb la descripcio de l'asset que permetra carregar-lo.



Per començar tindrem un registre d'assets. Aquest registre de tipus d'assets contindrà els identificadors
i una descripcio de cada tipus d'asset que podrem carregar/descarregar.




*/
namespace ds {

	using asset_id = entt::entity;
	constexpr asset_id asset_id_null = entt::null;
	using asset_type_id = std::uint32_t;
	constexpr asset_type_id asset_type_id_null = 0;

	struct asset_factory_type {
		// factory_type asset unique identifier
		asset_type_id type_id = asset_type_id_null;

		// Just a helper name for that type (not used to identify anything)
		std::string type_name = "unknown";

		// function to check if a file can be imported with this factory
		std::function <bool(const std::string& file)> can_import_from_file = nullptr;

		// function to create an std::any from file
		std::function<std::unique_ptr<void, std::function<void(void*)> > (const std::string& file)> create_from_file = nullptr;

		// function to create an std::any default
		std::function<std::unique_ptr<void, std::function<void(void*)> > ()> create_default = nullptr;

		//std::function<void (nlohmann::json& j)> serialize = nullptr;

		//std::function<std::any (const nlohmann::json& j)> deserialize = nullptr;
	};

	inline bool operator== (asset_factory_type const& lhs, asset_factory_type const& rhs) {
		return (lhs.type_id == lhs.type_id);
	}

	void register_asset_factory_type(const asset_factory_type& asset_t);

	/**
	* This function will create an asset from file and return the asset_id of the newly created asset_id.
	* Internally this function will search for an appropiate type factory to load that file.
	* returns asset_type_id_null if the asset can't be created
	*/
	asset_id create_asset_from_file(const std::string& file);

	/**
	* This function will create an asset using the default construction and will return the newly created asset_id
	* returns asset_type_id_null if the asset can't be created
	*/
	asset_id create_asset_from_type_id(asset_type_id type_id);

	/**
	* This will return the asset id object requested checked with the type_id.
	* Returns nullptr if the asset_id is not valid.
	* Will crash the application if the id is valid but not equal to the type_id supplied.
	*/
	void* get_asset_raw(asset_id id, asset_type_id type_id);


	// Helper function to get an asset by id and type_id casted to the correct type
	template<typename T>
	T* get_asset(asset_id id, asset_type_id type_id) {
		void* instance = get_asset_raw(id, type_id);
		if (instance) {
			return static_cast<T*>(instance);
		} else {
			return nullptr;
		}
	}

	void asset_test();



}