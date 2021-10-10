#pragma once
#include <destral/destral_ecs.h>

namespace ds {

	namespace cp {
		struct resource {
			static constexpr const char* name = "ds_cp_resource";
            std::string key;
            i32 hashed_key = 0;
            // If true, means that this resource is loaded using a key (Not a Runtime resource)
            bool is_key_valid = false;
            static void register_component(registry* r);
		};
	}

    namespace cp {
        struct resource_loader {
            static constexpr const char* name = "ds_cp_resource_loader";    
            typedef entity (load_fn_t)(registry* r, const char* resource_key_filepath);
            typedef bool (can_load_fn_t)(registry* r, const char* resource_key_filepath);

            load_fn_t* load_fn = nullptr;
            can_load_fn_t* can_load_fn = nullptr;
            static void register_component(registry* r);
        };
    }

    // Retrieves or loads the resource in to the cache
    entity resource_get(registry* r, const char* resource_key_filepath);


    //// Sets the fallback resource for the entity_type_id
    //void resource_set_fallback(registry* r, const char* entity_type_id, entity data);

    //// Returs the fallback resource for the entity_type_id, returns entity_null if no fallback was set.
    //entity resource_get_fallback(registry* r, const char* entity_type_id);


   

}