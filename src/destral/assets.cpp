#include "assets.h"
#include <unordered_map>
#include "ap_debug.h"
#include <any>
#include <fstream>
#include <filesystem>
#include "entt/entity/registry.hpp"
#include "nlohmann/json.hpp"

/*
    Structure:
    Basically we have two registry:
    
    - g_asset_types : Mantains the registered asset types.
    - g_asset_registry: Maintains the assets that are created from runtime and from filesystem

    


    
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
        Crea un id i l'afegeix a la cache runtime.

    2 - Creacio importat desde fitxer, creacio importat desde xurro binari.
        Crea un id unic. Crea el fitxer en la cache runtime. Carrega desde file/binary.






        Header d'un fitxer .dasset :

        {
            "version": 0, // version of the asset file
            "id": "f81d4fae-7dec-11d0-a765-00a0c91e6bf6", // unique id of this asset
            "type_id": 123123, // type id of this asset
            "data": {} // custom data of the asset 
        }
*/


using json = nlohmann::json;
namespace fs = std::filesystem;

namespace ds {

struct asset {
    asset_id id = asset_id_null; // asset key
    asset_type_id type_id = asset_type_id_null; // asset type key

    // This is the cache for the created object instance for this asset
    std::unique_ptr<void, std::function<void(void*) >> instance;

    // This contains the file where we imported this asset. 
    // (can be empty if it's an asset created by default) without from file)
    std::string file_path; 
};

inline bool operator== (asset const& lhs, asset const& rhs) {
    return (lhs.id == lhs.id);
}

}
namespace std {
    template<>
    struct hash< ds::asset_factory_type > {
    public:
        size_t operator()(const ds::asset_factory_type& asset_d) const {
            return asset_d.type_id;
        }
    };
  
    template<>
    struct hash< ds::asset > {
    public:
        size_t operator()(const ds::asset& asset_d) const {
            return entt::to_integral(asset_d.id);
        }
    };
}







namespace ds {




// Maintains the information to generate the asset from binary/filesystem
entt::registry g_asset_registry;

// Mantains the registered asset factory type
std::unordered_map<asset_type_id, asset_factory_type> g_asset_types;

void register_asset_factory_type(const asset_factory_type& asset_t) {
   AP_ASSERTM(asset_t.type_id != 0, "Invalid asset factory type id 0.");
   auto result = g_asset_types.try_emplace(asset_t.type_id, asset_t);
   AP_ASSERTM(result.second, "Asset factory type_id exists");
   AP_TRACE("Registered asset factory type: %d : %s ", asset_t.type_id, asset_t.type_name.c_str());
}

asset_factory_type* get_asset_factory(asset_type_id type_id) {
    auto res = g_asset_types.find(type_id);
    if (res == g_asset_types.end()) {
        return nullptr;
    } else {
        return &res->second;
    }
}

asset_id create_asset_from_file(const std::string& file) {
    fs::path file_path(file);
    asset_id new_asset_id = asset_id_null;
    // Check if this is a valid file with an extension
    if (fs::is_regular_file(file_path) && file_path.has_extension() && fs::exists(file_path)) {
        // Find an asset factory that can import this asset from file
        asset_factory_type* factory_ptr = nullptr;
        for (auto& factory : g_asset_types) {
            if (factory.second.can_import_from_file) {
                factory.second.can_import_from_file(file);
                factory_ptr = &factory.second;
                break;
            }
        }

        if (factory_ptr && factory_ptr->create_from_file) {
            new_asset_id = g_asset_registry.create();
            g_asset_registry.emplace<asset>(new_asset_id, new_asset_id, factory_ptr->type_id, std::move(factory_ptr->create_from_file(file)), file);
        } else {
            // Factory type to import create this asset doesn't exists
            AP_WARNING("Error creating asset from file: %s. "
                "Can't find a valid registered asset_type_factory to create from file this asset.", file.c_str());
        }

    } else {
        AP_WARNING("Error creating asset from file: %s."
            "Is not a regular file / has not extension / file doesn't exists.", file.c_str());
    }
    return new_asset_id;
}

asset_id create_asset_from_type_id(asset_type_id type_id) {
    asset_factory_type* fact = get_asset_factory(type_id);
    asset_id new_asset_id = asset_id_null;
    if (fact && fact->create_default) {
        new_asset_id = g_asset_registry.create();
        g_asset_registry.emplace<asset>(new_asset_id, new_asset_id, fact->type_id, std::move(fact->create_default()), "");
    } else {
        AP_WARNING("Error creating asset from type_id: %d. This type_id is not registered or it doesn't have a create_default function.", type_id);
    }
    return new_asset_id;
}

void* get_asset_raw(asset_id id, asset_type_id type_id) {
    void* instance = nullptr;
    if (g_asset_registry.valid(id)) {
        auto a = g_asset_registry.try_get<asset>(id);
        if (a) {
            if (a->type_id == type_id) {
                if (a->instance) {
                    instance = a->instance.get();
                } else {
                    asset_factory_type *factory = get_asset_factory(a->type_id);
                    if (factory && factory->create_from_file) {
                        a->instance = std::move(factory->create_from_file(a->file_path));
                        instance = a->instance.get();
                    }
                }
            } else {
                AP_FATAL("Asset id: %d doesn't match with the type_id: %d supplied.", id, type_id);
            }
        }
    }
    return instance;
}



void asset_test() {
 //   read_assets(fs::current_path() / "assets");



    //tex_asset_type.serialize = [](auto& j) {
    //    j["testing"] = "textureta";
    //    j["tes"] = 34;
    //};
   // auto asset_id = create_asset(1);
   // save_asset(asset_id);    
}

//void create_asset_by_id(std::uint32_t asset_id, std::uint32_t asset_type_id) {
//    if (!g_asset_registry.valid(entt::entity(asset_id))) {
//        // add it to the registry
//        auto new_e = g_asset_registry.create(entt::entity(asset_id));
//        AP_ASSERT(entt::entity(asset_id) == new_e);
//        g_asset_registry.emplace<asset>(new_e, asset_id, asset_type_id);
//
//    } else {
//        AP_WARNING("Trying to create an asset with an id that already exists! : %d", asset_id);
//    }
//}
//
//std::uint32_t create_asset(std::uint32_t asset_type_id) {
//    // generate a new entity id
//    auto new_e = g_asset_registry.create();
//    const auto asset_id = entt::to_integral(new_e);
//    g_asset_registry.emplace<asset>(new_e, asset_id, asset_type_id);
//    return asset_id;
//}
//
//
//void destroy_asset(std::uint32_t asset_id) {
//    if (g_asset_registry.valid(entt::entity(asset_id))) {
//        // destroy procedure
//        //if (result->second.data_stream.is_open()) {
//        //    result->second.data_stream.close();
//        //    // TODO here remove the file
//        //}
//        g_asset_registry.destroy(entt::entity(asset_id));
//    }
//}
//
//
//std::any get_asset_data(std::uint32_t asset_id) {
//    if (g_asset_registry.valid(entt::entity(asset_id))) {
//        //TODO
//    }
//    return {};
//}
//
//void read_assets(const fs::path& path) {
//    if (!fs::exists(path)) {
//        AP_WARNING("Error reading assets from directory: %s (directory doesn't exists). Trying to create it.", path.string().c_str());
//        if (fs::create_directory(path)) {
//            AP_INFO("Assets directory: %s created succesfully", path.string().c_str());
//        } else {
//            AP_WARNING("Can't create assets directory: %s (no loading/saving of assets will be possible)", path.string().c_str());
//            return;
//        }
//    }
//    
//    AP_INFO("Start loading assets from path: %s", path.string().c_str());
//    for (const auto& entry : fs::directory_iterator(path)) {
//        AP_TRACE("%s", entry.path().string().c_str());
//
//        if (!fs::is_regular_file(entry)) {
//            continue;
//        }
//
//        std::ifstream file;
//        file.open(entry.path());
//
//        json asset_json;
//        file >> asset_json;
//        file.close();
//
//        auto version_it = asset_json.find("version");
//        auto id_it = asset_json.find("id");
//        auto type_id_it = asset_json.find("type_id");
//        auto data_it = asset_json.find("data");
//        if (version_it != asset_json.end() &&
//            id_it != asset_json.end() &&
//            type_id_it != asset_json.end() &&
//            data_it != asset_json.end()) {
//            // then the asset is " valid"
//            std::uint32_t a_type_id = type_id_it.value().get<std::uint32_t>();
//            std::uint32_t id = id_it.value().get<std::uint32_t>();
//            create_asset_by_id(id, a_type_id);
//        } else {
//            AP_WARNING("Error loading asset file: %s . Invalid JSON format. Skipping this asset",
//                entry.path().string().c_str());
//        }
//
//    }
//    AP_INFO("Finished loading assets");
//}
//
//bool save_asset(std::uint32_t asset_id) {
//    
//    asset* asset_data = g_asset_registry.try_get<asset>(entt::entity(asset_id));
//    if (!asset_data) {
//        AP_WARNING("Error saving asset %d , this id doesn't exists!", asset_id );
//        return false;
//    }
//
//    
//
//    auto asset_type_it = g_asset_types.find(asset_data->type_id);
//    if (asset_type_it == g_asset_types.end()) {
//        AP_WARNING("Error saving asset %d , type_id %d doesn't exists!", asset_id, asset_data->type_id);
//        return false;
//    }
//
//    std::ofstream file;
//    fs::path file_path = "assets";
//    file_path /= std::to_string(asset_id) + ".dasset";
//    file.open(file_path, std::ofstream::out | std::ofstream::trunc);
//    if (!file.is_open()) {
//        AP_WARNING("Error saving asset %d to file: %s ", asset_id, file_path.string().c_str());
//        return false;
//    }
//
//
//    // build the dasset json file structure
//    json json_asset =
//    {
//        {"version", 0}, // version of the dasset file
//        {"id", asset_id }, // unique id of this asset
//        {"type_id", asset_data->type_id }, // type id of this asset
//        {"data" , {}} // custom data of the asset 
//    };
//
//    // call the serializer for that concrete type
//    if (asset_type_it->second.serialize) {
//        asset_type_it->second.serialize(json_asset["data"]);
//    }
//
//
//    AP_TRACE("Asset data to save: %s", json_asset.dump(4).c_str());
//    
//    file << json_asset;
//
//    AP_INFO("Asset id: %d saved to %s succesfully.", asset_id, file_path.string().c_str());
//   
//    file.close();
//
//    return true;
//}
//

//



}



