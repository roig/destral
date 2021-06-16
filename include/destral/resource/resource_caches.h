#pragma once

#include <destral/resource/resource.h>

// resources that destral handles by default
#include <destral/graphics/destral_image.h>

namespace ds {

	void caches_init();
	void caches_deinit();

	resource_cache<image>* cache_images();
	void cache_images_add_info(const resource_key& res_key, const std::string& filename);

}