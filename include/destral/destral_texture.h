#pragma once
#include <destral/thirdparty/sokol_gfx.h>
#include <destral/destral_ecs.h>
namespace ds {
	/**
	* GPU texture entity and component.
	*/
	struct texture {
		static constexpr const char* cp_name = "ds_texture_cp";
		static constexpr const char* e_name = "ds_texture";
		sg_image gpu_texid = { .id = 0 };
		~texture();
		
		// Adds a texutre loader to the resource cache
        static void set_resource_loader(registry* r);
		// Registers the texture component to the registry
		static void register_component(registry* r);
		// Registers a texture type entity to the registry
		static void register_entity(registry* r);
	};
}