#pragma once
#include <ap_uuid.h>
#include <functional>
#include <any>
#include <string>

/**
Com funcionara l'asset Manager?

Basicament, llegirà tots els fitxer d'un directori i cercarà fitxers .dasset

Aquests fitxers seran un json amb la descripcio de l'asset que permetra carregar-lo.



Per començar tindrem un registre d'assets. Aquest registre de tipus d'assets contindrà els identificadors
i una descripcio de cada tipus d'asset que podrem carregar/descarregar.




*/
namespace ds {
	struct asset_type {
		std::int32_t type_id;
		std::function<std::any(const std::string& file_name)> load_from_file;
	};

	inline bool operator== (asset_type const& lhs, asset_type const& rhs) {
		return (lhs.type_id == lhs.type_id);
	}

	void register_asset_type(const asset_type& asset_t);
	ap::uuid create_asset(std::int32_t type_id);
	void destroy_asset(ap::uuid asset_id);
	std::any get_asset_data(ap::uuid asset_id);


	void asset_test();



}