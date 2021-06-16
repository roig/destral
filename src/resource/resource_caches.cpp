#include <destral/resource/resource_caches.h>
#include <filesystem>

#define STBI_ASSERT(x) dscheck(x)
//#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"


namespace fs = std::filesystem;
namespace ds {
	// loads a image file
	image* s_load_image(const std::string& path) {
		int w, h, bpp;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &bpp, 0);
		if (pixels == nullptr) {
			DS_WARNING(std::format("Error loading the image: {}. Current path: {}", path, std::filesystem::current_path().string()));
			return nullptr;
		}

		image* img = new image();
		img->size.x = w;
		img->size.y = h;
		img->bpp = bpp;
		img->pixels.assign(pixels, pixels + (img->size.x * img->size.y * img->bpp));
		stbi_image_free(pixels);
		return img;
	}

	/////////////////////////////////// LOADERS
	class resource_loader_image : public resource_loader<image> {
	public:
		void do_load(const resource_key& key) override {
			auto info = get_info(key);
			if (!info) {
				set_not_found(key);
				DS_WARNING(std::format("Error loading resource key: {}  resource info not found.", key.key));
				return;
			}

			fs::path file_path = *info;

			if (fs::is_regular_file(file_path) && file_path.has_extension() && fs::exists(file_path)) {
				image* img_ptr = s_load_image(file_path.string());
				if (!img_ptr) {
					set_not_found(key);
					DS_WARNING(std::format("Error loading resource key: {}  file:  {}", key.key, file_path.string()));
					return;
				}
				// resource created !
				set(key, img_ptr, resource_data_state::loaded_mutable, resource_policy::reference_counted);
			} else {
				set_not_found(key);
				DS_WARNING(std::format("Error loading resource key: {}  file:  {}  invalid file or not exists. Current path: {}",
					key.key, file_path.string(), fs::current_path().string()));
			}
			
		}

		void add_info(const resource_key& res_key, const std::string& res_file) {
			auto it = resources_info.find(res_key);
			dsverifym(it == resources_info.end(), std::format("Duplicated key while adding resource info key: {}   id:  {}", res_key.key, res_key.hashed_key));
			resources_info[res_key] = res_file;
		}

		std::string* get_info(const resource_key& res_key) {
			auto it = resources_info.find(res_key);
			if (it == resources_info.end()) return nullptr;
			return &it->second;
		}
		
		std::unordered_map<resource_key, std::string> resources_info;
	};


	//////////////////////////////////////
	struct internal_caches {
		resource_cache<image>* images = nullptr;
	} g_caches;


	void caches_init() {
		dsverifym(!g_caches.images, "You can't call caches_init multiple times!");
		g_caches.images = new resource_cache<image>();
		g_caches.images->set_loader(new resource_loader_image());
	}

	void caches_deinit() {
		if (g_caches.images) delete g_caches.images;
	}

	resource_cache<image>* cache_images() {
		return g_caches.images;
	}

	void cache_images_add_info(const resource_key& res_key, const std::string& filename) {
		dsverify(g_caches.images);
		resource_loader_image* loader = (resource_loader_image *)g_caches.images->loader();
		loader->add_info(res_key, filename);
	}
}