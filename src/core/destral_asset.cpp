//#include <destral/core/destral_asset.h>
//#include <filesystem>
//#include <unordered_map>
//namespace ds {
//	struct asset_factory {
//		// Asset name identifier
//		std::string asset_type_name = "";
//
//		// Function to create an asset instance (using default constructor)
//		void* (*create_default_fn)(void) = nullptr;
//
//		// Function to delete an asset instance pointer
//		void (*deleter_fn)(void*) = nullptr;
//
//		// Function to create an asset instance from a file
//		void* (*create_from_file_fn)(const std::string& path) = nullptr;
//
//		// Function to check if we can load the asset from file
//		bool (*can_create_from_file_fn)(const std::string& path) = nullptr;
//
//		// TODO serialize/deserialize functions
//		//std::function <bool(const std::string& file)> can_import_from_file = nullptr;
//	};
//
//	struct asset_instance {
//		// Asset type name (for identifying the real asset type)
//		std::string asset_type_name;
//		
//		// This is the instance pointer for that asset instance
//		void* data = nullptr;
//
//		// This contains the file where we imported this asset. 
//		// (can be empty if it's an asset created by default) without from file)
//		std::string file_path;
//	};
//	//inline bool operator== (asset_instance const& lhs, asset_instance const& rhs) {	return (lhs.id == lhs.id);	}
//
//
//	struct asset_registry {
//		asset_registry() {
//			ecs_registry = registry_create();
//		}
//		~asset_registry() {
//			registry_destroy(ecs_registry);
//		}
//		registry* ecs_registry = nullptr;
//		std::unordered_map<u64, asset_factory> asset_factories;
//
//		asset_factory* get_asset_factory(const std::string& asset_type_name) {
//			auto res = asset_factories.find(fnv1a_64bit(asset_type_name));
//			return res == asset_factories.end() ? nullptr: &res->second;
//		}
//
//		asset_factory* get_asset_factory_that_can_import_file(const std::string& file) {
//			for (auto& factory : asset_factories) {
//				if (factory.second.can_create_from_file_fn) {
//					if (factory.second.can_create_from_file_fn(file)) {
//						return &factory.second;
//					}
//				}
//			}
//			return nullptr;
//		}
//
//		asset new_asset(asset_factory* factory, void* instance_data, const std::string& file) {
//			asset new_asset;
//			new_asset.id = entity_make(ecs_registry, "ds_asset_instance");
//			auto instance = entity_get<asset_instance>(ecs_registry, new_asset.id, "asset_instance");
//			instance->asset_type_name = factory->asset_type_name;
//			instance->file_path = file;
//			instance->data = instance_data;
//			return new_asset;
//		}
//
//		void destroy_asset(asset a) {
//			dsverify(entity_valid(ecs_registry, a.id));
//			auto instance = entity_get<asset_instance>(ecs_registry, a.id, "asset_instance");
//			dsverify(instance);
//			auto factory = get_asset_factory(instance->asset_type_name);
//			dsverify(factory);
//			if (factory->deleter_fn) {
//				factory->deleter_fn(instance->data);
//			}
//			entity_destroy(ecs_registry, a.id);
//		}
//
//	};
//
//	asset_registry* asset_registry_create() {
//		auto ar = new asset_registry();
//		DS_ECS_COMPONENT_REGISTER(ar->ecs_registry, asset_instance);
//
//		entity_register(ar->ecs_registry, "ds_asset_instance", { "asset_instance" });
//
//		return ar;
//	}
//
//	void asset_registry_destroy(asset_registry* ar) {
//		dscheck(ar);
//
//		// TODO unload all assets first
//		// unload all asset_factories
//		delete ar;
//	}
//
//
//	void asset_register_asset_factory(asset_registry* ar, const std::string& asset_type_name,
//		void* (*create_default_fn)(void), void (*deleter_fn)(void*), 
//		void* (*create_from_file_fn)(const std::string& path), bool (*can_create_from_file_fn)(const std::string& path)) {
//		dscheck(ar);
//		dsverifym(!asset_type_name.empty(), "Asset type name must not be empty!");
//		const auto asset_type_hashed = fnv1a_64bit(asset_type_name);
//		dsverifym(!ar->asset_factories.contains(asset_type_hashed), "Asset type id is already registered.");
//		asset_factory af;
//		af.asset_type_name = asset_type_name;
//		af.create_default_fn = create_default_fn;
//		af.deleter_fn = deleter_fn;
//		af.create_from_file_fn = create_from_file_fn;
//		af.can_create_from_file_fn = can_create_from_file_fn;
//		ar->asset_factories[asset_type_hashed] = af;
//	}
//
//	asset asset_create(asset_registry* ar, const std::string& asset_type_name) {
//		dscheck(ar);
//		auto factory = ar->get_asset_factory(asset_type_name);
//		if (!factory) {
//			DS_WARNING(std::format("Could not create asset with asset type name: {}. Asset type name factory not registered.", asset_type_name));
//			return {};
//		}
//
//		if (factory->create_default_fn) {
//			return ar->new_asset(factory, factory->create_default_fn(), "");
//		} else {
//			DS_WARNING(std::format("Could not create asset with asset type name: {}. Not default create function registered.", asset_type_name));
//			return {};
//		}
//	}
//
//	asset asset_create_from_file(asset_registry* ar, const std::string& file) {
//		dscheck(ar);
//		namespace fs = std::filesystem;
//		fs::path file_path(file);
//
//		// Check if this is a valid file with an extension
//		if (fs::is_regular_file(file_path) && file_path.has_extension() && fs::exists(file_path)) {
//			// Find an asset factory that can import this asset from file
//			auto factory = ar->get_asset_factory_that_can_import_file(file);
//			if (!factory) {
//				DS_WARNING(std::format("Could not create from file: {}. No asset factory registered found to load this type of file.", file));
//				return {};
//			}
//
//			if (factory->create_from_file_fn) {
//				auto asset_instance_data = factory->create_from_file_fn(file);
//				if (asset_instance_data) {
//					return ar->new_asset(factory, asset_instance_data, file);
//				} else {
//					DS_WARNING(std::format("Error creating asset from file: {}. Factory asset type name: {}", file, factory->asset_type_name));
//				}
//			} else {
//				// Factory type to import create this asset doesn't exists
//				DS_WARNING(std::format("Error creating asset from file: {}. Factory asset type name: {} doesn't have create_from_file function.", file, factory->asset_type_name));
//			}
//		} else {
//			DS_WARNING(std::format("Error creating asset from file: {}. File has no extension / or file doesn't exists.", file));
//		}
//		return {};
//	}
//
//	void asset_destroy(asset_registry* ar, asset a) {
//		dscheck(ar);
//		ar->destroy_asset(a);
//	}
//	
//
//
//
//
//}