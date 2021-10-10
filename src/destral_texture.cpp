#include <destral/destral_texture.h>
#include <destral/destral_resource2.h>
#include <destral/destral_renderer.h>
#include <destral/destral_filesystem.h>
#include "thirdparty/stb_image.h"
namespace ds {

	static sg_image create_sg_image_from_memory(const u8* pixels_data, i32 width, i32 height) {
		sg_image_desc image_desc = { 0 };
		image_desc.width = width;
		image_desc.height = height;
		image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
		image_desc.min_filter = SG_FILTER_NEAREST;
		image_desc.mag_filter = SG_FILTER_NEAREST;
		image_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
		image_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
		image_desc.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
		image_desc.data.subimage[0][0] = {
			  .ptr = pixels_data,
			  .size = sizeof(pixels_data)
		};

		//image_desc.label = "texture";
		return sg_make_image(image_desc);
	}

	static sg_image create_sg_iamge_from_file(const std::string& filename) {
		int x, y, n;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* pixels = stbi_load(filename.c_str(), &x, &y, &n, 0);
		if (pixels != nullptr) {
			// ... process data if not null ...
			// ... x = width, y = height, n = # 8-bit components per pixel ...
			// ... replace '0' with '1'..'4' to force that many components per pixel
			// ... but 'n' will always be the number that it would have been if you said 0
			sg_image img = create_sg_image_from_memory(pixels, x, y);
			stbi_image_free(pixels);
			return img;
		}
		DS_WARNING(std::format("Error loading the image: {}. Current path: {}", filename));
		return { 0 };
	}

	///////////////////////

	ivec2 cp::texture::get_size() {
		sg_image_info info = sg_query_image_info(gpu_texid);
		return { info.width, info.height };
	}

	cp::texture::~texture() {
		sg_destroy_image(gpu_texid);
	}

	void cp::texture::register_component(registry* r) {
		r->component_register<cp::texture>(cp::texture::name);
	}
	
	void en::texture::register_entity(registry* r) {
		r->entity_register(en::texture::name, { cp::texture::name, cp::resource::name });
	}

    void en::texture_loader::register_entity(registry* r) {
		r->entity_register(en::texture_loader::name, { cp::resource_loader::name });
    }

	entity en::texture_loader::create_setup_entity(registry* r) {
		entity e_tl = r->entity_make(en::texture_loader::name);
		cp::resource_loader *rl = r->component_get<cp::resource_loader>(e_tl, cp::resource_loader::name);
		rl->can_load_fn = [](registry* r, const char* res_key_filepath) {
			return fs_has_extension(res_key_filepath, ".png");
		};
		rl->load_fn = [](registry* r, const char* res_key_filepath) {
			auto e = r->entity_make(en::texture::name);
			cp::texture* t = r->component_get<cp::texture>(e, cp::texture::name);
			sg_image gpu_img = create_sg_iamge_from_file(res_key_filepath);
			if (gpu_img.id == 0) {
				return entity_null;
			} else {
				t->gpu_texid = gpu_img;
				return e;
			}
		};
		return e_tl;
	}
}