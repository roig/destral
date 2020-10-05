#include "assets.h"
#include <unordered_map>
#include "ap_debug.h"
#include <any>
#include <fstream>
#include <filesystem>

#include "rapidjson/document.h"

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
std::unordered_map<ap::uuid, asset_type> g_asset_types;

void register_asset_type(const asset_type& asset_t) {
   // auto result = g_asset_types.insert(asset_t);
   // AP_ASSERTM(result.second, "Asset type id exists");
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
    
    for (const auto& entry : fs::directory_iterator(path)) {
        AP_TRACE("%s", entry.path().string().c_str());
    }
}

bool save_asset(ap::uuid asset_id) {
    auto res = g_asset_registry.find(asset_id);
    if (res == g_asset_registry.end()) {
        AP_WARNING("Error saving asset %s , this id doesn't exists!", ap::uuid_to_string(asset_id).c_str() );
        return false;
    }


    
    std::ofstream file;
    std::string filename = ap::uuid_to_string(asset_id);
    file.open(filename, std::ofstream::out | std::ofstream::trunc);
    if (!file.is_open()) {
        AP_WARNING("Error saving asset %s to file: %s ", ap::uuid_to_string(asset_id).c_str(), filename.c_str());
        return false;
    }

    // asset header

   
    file.close();

    return true;
}

void asset_test() {
    read_assets(fs::current_path());

    //TODO fer servir nlohman json

    rapidjson::Document d; // Null
    d.SetObject();
    Value contact(kObject);
    contact.AddMember("name", "Milo", document.GetAllocator());
    contact.AddMember("married", true, document.GetAllocator());

    rapidjson::Value v;    // Null
    v.SetInt(10);
    v = 10;     // Shortcut, same as above

}




}


