#pragma once
#include <destral/thirdparty/sokol_gfx.h>
#include <destral/destral_ecs.h>
#include <destral/destral_math.h>	

namespace ds {

	// Components
	namespace cp {
		struct texture {
			static constexpr const char* name = "ds_texture_cp";
			sg_image gpu_texid = { .id = 0 };
			~texture();
			// Registers the texture component to the registry
			static void register_component(registry* r);

			// Returns the size of the texture in pixels
			ivec2 get_size();
		};
	}

	// Entities
	namespace en {
		// The texture entity contains the cp::texture and cp::resource components
		// It handles a GPU texture image.
		namespace texture {
			constexpr const char* name = "ds_texture_en";
			void register_entity(registry* r);
		}

		// texture loader entity is the entity that loads an en::texture resource
		namespace texture_loader {
			constexpr const char* name = "ds_texture_loader_en";
			void register_entity(registry* r);
			entity create_setup_entity(registry* r);
		}
	}


}