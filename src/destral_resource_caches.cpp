#include <destral/destral_resource_caches.h>
#include <filesystem>

#include "thirdparty/stb_image.h"
#include "thirdparty/cute_aseprite.h"

#include <destral/destral_renderer.h>




namespace fs = std::filesystem;
namespace ds {
	void flip_image_horizontal(image& img) {
		struct pixel {
			u8 r, g, b, a;
		};
		pixel* pix = (pixel*)img.pixels.data();
		int w = img.size.x;
		int h = img.size.y;
		int flips = h / 2;
		for (int i = 0; i < flips; ++i) {
			pixel* a = (pixel*)pix + w * i;
			pixel* b = (pixel*)pix + w * (h - i - 1);
			for (int j = 0; j < w; ++j) {
				pixel t = *a;
				*a = *b;
				*b = t;
				++a;
				++b;
			}
		}
	}

	// loads an image from data
	image* s_load_image(i32 w, i32 h, i32 bpp, u8* pixels, bool flip_horizontal) {
		image* img = new image();
		img->size.x = w;
		img->size.y = h;
		img->bpp = bpp;
		img->pixels.assign(pixels, pixels + (img->size.x * img->size.y * img->bpp));
		if (flip_horizontal) {
			flip_image_horizontal(*img);
		}
		img->gpu_texid = load_texture_memory(img->pixels.data(), w, h);
		
		return img;
	}

	// loads a image from file
	image* s_load_image(const std::string& path) {
		int w, h, bpp;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &bpp, 0);
		if (pixels == nullptr) {
			DS_WARNING(std::format("Error loading the image: {}. Current path: {}", path, std::filesystem::current_path().string()));
			return nullptr;
		}
		image* img = s_load_image(w, h, bpp, pixels, false);
		stbi_image_free(pixels);
		return img;
	}




	// loads an aseprite file
	animation_collection* s_load_animation_aseprite(const std::string& path) {

		auto ase = cute_aseprite_load_from_file(path.c_str(), nullptr);
		if (!ase) {
			return nullptr;
		}

		animation_collection* collection = new animation_collection();


		if (ase->tag_count) {

			// add animations by tag first (using the full frame)
			for (i32 tag_idx = 0; tag_idx < ase->tag_count; tag_idx++) {
				ase_tag_t* curr_tag = &ase->tags[tag_idx];
				animation anim;
				anim.name = curr_tag->name;
				for (i32 frame_idx = curr_tag->from_frame; frame_idx <= curr_tag->to_frame; frame_idx++) {
					// load an insert to the cache a runtime resource
					resource_key frame_image_key = anim.name + "_frame_" + std::to_string(frame_idx);
					image* frame_image = s_load_image(ase->w, ase->h, 4, (u8*)ase->frames[frame_idx].pixels, true);
					cache_images()->set(frame_image_key, frame_image, resource_data_state::loaded_final, resource_policy::reference_counted);
					
					animation_frame anim_frame;
					anim_frame.image = cache_images()->get(frame_image_key);
					anim_frame.time = ase->frames[frame_idx].duration_milliseconds / 1000.0f;
					anim.frames.push_back(anim_frame);
				}
				collection->animations[anim.name] = anim;
			}

			// add animations by tag and per layer (cell)
			// the name of this animations will be tag then layer: for example Idle.Orc (Idle is the tag and Orc is the layer)
			for (i32 layer_idx = 0; layer_idx < ase->layer_count; layer_idx++) {
				ase_layer_t* curr_layer = &ase->layers[layer_idx];
				if (curr_layer->type == ASE_LAYER_TYPE_GROUP) {
					continue;
				}


				for (i32 tag_idx = 0; tag_idx < ase->tag_count; tag_idx++) {
					ase_tag_t* curr_tag = &ase->tags[tag_idx];

					animation new_anim;
					new_anim.name = curr_layer->name + std::string(".") + curr_tag->name;  // Layer.Tag  (example Orc.Idle )
					// add animation frames
					for (i32 frame_idx = curr_tag->from_frame; frame_idx <= curr_tag->to_frame; frame_idx++) {
						ase_frame_t* curr_frame = &ase->frames[frame_idx];
						ase_cel_t* curr_cell = &curr_frame->cels[layer_idx];
						
						if (!curr_cell->pixels) {
							continue;
						}
						resource_key frame_cell_res_key = new_anim.name + "_frame_" + std::to_string(frame_idx);
						image* cell_image = s_load_image(curr_cell->w, curr_cell->h, 4, (u8*)curr_cell->pixels, true);
						cache_images()->set(frame_cell_res_key, cell_image, resource_data_state::loaded_final, resource_policy::reference_counted);

						animation_frame anim_frame;
						anim_frame.image = cache_images()->get(frame_cell_res_key);
						anim_frame.time = ase->frames[frame_idx].duration_milliseconds / 1000.0f;
						new_anim.frames.push_back(anim_frame);
					}

					// If we have frames in this animation add it to the collection
					if (!new_anim.frames.empty()) {
						collection->animations[new_anim.name] = new_anim;
					}
				}
			}

			
		} else {
			//animation anim;

			//for (i32 frame_idx = 0; frame_idx < ase->frame_count; frame_idx++) {
			//	// load an insert to the cache a runtime resource
			//	resource_key frame_image_key = path + std::to_string(frame_idx);
			//	image* frame_image = s_load_image(ase->w, ase->h, 4, (u8*)ase->frames[frame_idx].pixels, true);
			//	cache_images()->set(frame_image_key, frame_image, resource_data_state::loaded_final, resource_policy::reference_counted);

			//	animation_frame anim_frame;
			//	anim_frame.image = cache_images()->get(frame_image_key);
			//	anim_frame.time = ase->frames[frame_idx].duration_milliseconds / 1000.0f;
			//	anim.frames.push_back(anim_frame);
			//}
			//spr->animations["default"] = anim;
		}
		cute_aseprite_free(ase);
		return collection;
	}


	/////////////////////////////////// LOADERS
	struct resource_loader_info {
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


	class resource_loader_image : public resource_loader<image> {
	public:
		void do_load(const resource_key& key) override {
			auto info = resources_info.get_info(key);
			if (!info) {
				set_not_found(key);
				DS_WARNING(std::format("Error loading resource key: {}  resource info not found.", key.key));
				return;
			}

			fs::path file_path = *info;

			if (fs::is_regular_file(file_path) && file_path.has_extension() && fs::exists(file_path)) {
				image* img_ptr = nullptr;
				if (file_path.extension() == ".aseprite") {
					//img_ptr = s_load_image_aseprite(file_path.string());
				} else {
					img_ptr = s_load_image(file_path.string());
				}

				if (!img_ptr) {
					set_not_found(key);
					DS_WARNING(std::format("Error loading image resource key: {}  file:  {}", key.key, file_path.string()));
					return;
				}
				// resource created !
				set(key, img_ptr, resource_data_state::loaded_mutable, resource_policy::manual);
			} else {
				set_not_found(key);
				DS_WARNING(std::format("Error loading image resource key: {}  file:  {}  invalid file or not exists. Current path: {}",
					key.key, file_path.string(), fs::current_path().string()));
			}
			
		}
		resource_loader_info resources_info;

	};

	class resource_loader_animation : public resource_loader<animation_collection> {
	public:
		void do_load(const resource_key& key) override {
			auto info = resources_info.get_info(key);
			if (!info) {
				set_not_found(key);
				DS_WARNING(std::format("Error loading animation resource key: {}  resource info not found.", key.key));
				return;
			}

			fs::path file_path = *info;

			if (fs::is_regular_file(file_path) && file_path.has_extension() && fs::exists(file_path)) {
				animation_collection* collection = nullptr;
				if (file_path.extension() == ".aseprite") {
					collection = s_load_animation_aseprite(file_path.string());
				}

				if (!collection) {
					set_not_found(key);
					DS_WARNING(std::format("Error loading animation resource key: {}  file:  {}", key.key, file_path.string()));
					return;
				}
				// resource created !
				set(key, collection, resource_data_state::loaded_mutable, resource_policy::manual);
			} else {
				set_not_found(key);
				DS_WARNING(std::format("Error loading animation resource key: {}  file:  {}  invalid file or not exists. Current path: {}",
					key.key, file_path.string(), fs::current_path().string()));
			}

		}
		resource_loader_info resources_info;

	};



	//////////////////////////////////////
	struct internal_caches {
		resource_cache<image>* images = nullptr;
		resource_cache<animation_collection>* animations = nullptr;
	} g_caches;


	void caches_init() {
		dsverifym(!g_caches.images, "You can't call caches_init multiple times!");
		g_caches.images = new resource_cache<image>();
		g_caches.images->set_loader(new resource_loader_image());

		g_caches.animations = new resource_cache<animation_collection>();
		g_caches.animations->set_loader(new resource_loader_animation());

	}

	void caches_deinit() {
		if (g_caches.images) delete g_caches.images;
		if (g_caches.animations) delete g_caches.animations;
	}

	resource_cache<image>* cache_images() {
		return g_caches.images;
	}

	void cache_images_add_info(const resource_key& res_key, const std::string& filename) {
		dsverify(g_caches.images);
		resource_loader_image* loader = (resource_loader_image *)g_caches.images->loader();
		loader->resources_info.add_info(res_key, filename);
	}


	resource_cache<animation_collection>* cache_animations() {
		return g_caches.animations;
	}
	void cache_animations_add_info(const resource_key& res_key, const std::string& filename) {
		dsverify(g_caches.animations);
		resource_loader_animation* loader = (resource_loader_animation*)g_caches.animations->loader();
		loader->resources_info.add_info(res_key, filename);
	}


}