#include "assets.h"
#include <unordered_map>
#include "ap_debug.h"
#include <any>
#include <fstream>
#include <filesystem>

#include "nlohmann/json.hpp"

/*
    Estructures:

    asset_types_registry: registre que control·la quins tipus d'asset podem carregar/manegar etc..
    referencia un asset_type_id amb la informacio per crear assets d'aquell tipus.
    Exemple:
    2 -> texture_desc
    456 -> sprite


    
    asset_registry: registre principal que control·la quins fitxers .dasset hi ha a la carpeta "/assets". 
    referencia un asset_id amb el fitxer en el directori. Conte les dades guardades que serveixen per generar un asset.
    exemple:
    1231234 -> 1231234.json



    asset_cache: Cache que conte els objectes assets actualment creats.
    Quan volguem crear un asset en runtime, el crearem aqui. a partir de la informacio del asset_types_registry.
    Si un asset id no existeix al asset_registry voldra dir que es un asset en runtime (no esta guardat al registre).
    El podem guardar fent un save.




    Quin seria el flow d'un asset?

    1 - Creacio per defecte.
        Crea un uuid i l'afegeix a la cache runtime.

    2 - Creacio importat desde fitxer, creacio importat desde xurro binari.
        Crea un uuid unic. Crea el fitxer en la cache runtime. Carrega desde file/binary.






        Header d'un fitxer .dasset :

        {
            "version": 0, // version of the asset file
            "uuid": "f81d4fae-7dec-11d0-a765-00a0c91e6bf6", // unique id of this asset
            "type_id": 123123, // type id of this asset
            "data": {} // custom data of the asset 
        }
*/


using json = nlohmann::json;
namespace fs = std::filesystem;

struct asset {
    ap::uuid id; // asset key
    std::int32_t type_id; // asset type key
    std::any instance;
};

inline bool operator== (asset const& lhs, asset const& rhs) {
    return (lhs.id == lhs.id);
}

namespace std {
    template<>
    struct hash< ds::asset_type > {
    public:
        size_t operator()(const ds::asset_type& asset_d) const {
            return asset_d.type_id;
        }
    };
  
    template<>
    struct hash< asset > {
    public:
        size_t operator()(const asset& asset_d) const {
            return std::hash<ap::uuid>()(asset_d.id);
        }
    };
}







namespace ds {
// Maintains the information to generate the asset from binary/filesystem
std::unordered_map<ap::uuid, asset> g_asset_registry;

// Mantains the registered asset type
std::unordered_map<std::int32_t, asset_type> g_asset_types;

void register_asset_type(const asset_type& asset_t) {
   auto result = g_asset_types.try_emplace(asset_t.type_id, asset_t);
   AP_ASSERTM(result.second, "Asset type_id exists");

}


void create_asset_by_uuid(ap::uuid asset_id, std::int32_t asset_type_id) {
    ap::uuid new_asset_id = asset_id;
    auto result = g_asset_registry.find(new_asset_id);
    if (result != g_asset_registry.end()) {
        // add it to the registry  
        g_asset_registry.emplace(new_asset_id, asset{ new_asset_id, asset_type_id });
    } else {
        AP_FATAL("Trying to create an asset with an uuid that already exists! : %s", 
            ap::uuid_to_string(new_asset_id).c_str())
    }
}

ap::uuid create_asset(std::int32_t asset_type_id) {
    // get a unique uuid that is not in the registry
    ap::uuid new_asset_id = ap::uuid_v4();
    auto result = g_asset_registry.find(new_asset_id);
    while (result != g_asset_registry.end()) {
        new_asset_id = ap::uuid_v4();
        result = g_asset_registry.find(new_asset_id);
    } 

    // add it to the registry  
    g_asset_registry.emplace(new_asset_id, asset{ new_asset_id, asset_type_id });
    return new_asset_id;
}


void destroy_asset(ap::uuid asset_id) {
    auto result = g_asset_registry.find(asset_id);
    if (result != g_asset_registry.end()) {
        // destroy procedure
        //if (result->second.data_stream.is_open()) {
        //    result->second.data_stream.close();
        //    // TODO here remove the file
        //}
    }
    g_asset_registry.erase(asset_id);
}


std::any get_asset_data(ap::uuid asset_id) {
    auto result = g_asset_registry.find(asset_id);
    if (result != g_asset_registry.end()) {
              //TODO  
    }
    return {};
}

void read_assets(const fs::path& path) {
    AP_INFO("Start loading assets from path: %s", path.string().c_str());
    if (fs::exists(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            AP_TRACE("%s", entry.path().string().c_str());

            if (!fs::is_regular_file(entry)) {
                continue;
            }


            std::ifstream file;
            file.open(entry.path());

            json asset_json;
            asset_json << file;
            file.close();

            auto version_it = asset_json.find("version");
            auto uuid_it = asset_json.find("uuid");
            auto type_id_it = asset_json.find("type_id");
            auto data_it = asset_json.find("data");
            if (version_it != asset_json.end() &&
                uuid_it != asset_json.end() &&
                type_id_it != asset_json.end() &&
                data_it != asset_json.end()) {
                // then the asset is " valid"
                std::int32_t a_type_id = type_id_it.value().get<std::int32_t>();
                ap::uuid uuid = ap::uuid_from(uuid_it.value().get<std::string>());
                create_asset_by_uuid(uuid, a_type_id);
            } else {
                AP_WARNING("Error loading asset file: %s . Invalid JSON format. Skipping this asset",
                    entry.path().string().c_str());
            }

        }
    }
    AP_INFO("Finished loading assets");
}

bool save_asset(ap::uuid asset_id) {
    auto asset_it = g_asset_registry.find(asset_id);
    if (asset_it == g_asset_registry.end()) {
        AP_WARNING("Error saving asset %s , this id doesn't exists!", ap::uuid_to_string(asset_id).c_str() );
        return false;
    }

    auto asset_type_it = g_asset_types.find(asset_it->second.type_id);
    if (asset_type_it == g_asset_types.end()) {
        AP_WARNING("Error saving asset %s , type_id %d doesn't exists!", ap::uuid_to_string(asset_id).c_str(), 
           asset_it->second.type_id);
        return false;
    }

    std::ofstream file;
    fs::path file_path = "assets";
    file_path /= ap::uuid_to_string(asset_id) + ".dasset";
    file.open(file_path, std::ofstream::out | std::ofstream::trunc);
    if (!file.is_open()) {
        AP_WARNING("Error saving asset %s to file: %s ", ap::uuid_to_string(asset_id).c_str(), file_path.string().c_str());
        return false;
    }


    // build the dasset json file structure
    json json_asset =
    {
        {"version", 0}, // version of the dasset file
        {"uuid", ap::uuid_to_string(asset_id) }, // unique id of this asset
        {"type_id", asset_it->second.type_id }, // type id of this asset
        {"data" , {}} // custom data of the asset 
    };

    // call the serializer for that concrete type
    if (asset_type_it->second.serialize) {
        asset_type_it->second.serialize(json_asset["data"]);
    }


    AP_TRACE("Asset data to save: %s", json_asset.dump(4).c_str());
    
    file << json_asset;

    AP_INFO("Asset %s saved to %s succesfully.", ap::uuid_to_string(asset_id).c_str(), file_path.string().c_str());
   
    file.close();

    return true;
}

void asset_test() {
    read_assets(fs::current_path() / "assets");

    //TODO fer servir nlohman json
    asset_type tex_asset_type;
    tex_asset_type.type_id = 1;
    tex_asset_type.serialize = [](auto& j) {
        j["testing"] = "textureta";
        j["tes"] = 34;
    };
    register_asset_type(tex_asset_type);

    auto asset_id = create_asset(1);
    save_asset(asset_id);    
}




}


