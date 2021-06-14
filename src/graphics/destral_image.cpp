#include <destral/core/destral_common.h>

#define STBI_ASSERT(x) dscheck(x)
//#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"

#include <destral/graphics/destral_image.h>
#include <filesystem>

namespace ds {

	image* load_image(const std::string& path) {
		image* img = new image();
		int w, h, bpp;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &bpp, 0);
		if (pixels == nullptr) {
			DS_WARNING(std::format("Error loading the image: {}. Current path: {}", path, std::filesystem::current_path().string()));

			return nullptr;
		}
		img->size.x = w;
		img->size.y = h;
		img->bpp = bpp;
		img->pixels.assign(pixels, pixels + (img->size.x * img->size.y * img->bpp));
		stbi_image_free(pixels);
		return img;
	}

}