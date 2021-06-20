#include <destral/destral_image.h>
#include <destral/destral_renderer.h>

namespace ds {

	image::~image() {
		destroy_texture(gpu_texid);
	}

}