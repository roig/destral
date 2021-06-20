#pragma once

#include <destral/destral_resource.h>

// resources that destral handles by default
#include <destral/destral_image.h>
#include <destral/destral_sprite.h>

namespace ds {

	void caches_init();
	void caches_deinit();

	resource_cache<image>* cache_images();
	void cache_images_add_info(const resource_key& res_key, const std::string& filename);

	resource_cache<animation_collection>* cache_animations();
	void cache_animations_add_info(const resource_key& res_key, const std::string& filename);

}