#include <destral/math/destral_math.h>

namespace ds {
	struct image {

		// the size in pixels of the image
		ivec2 size = { 0,0 };
		
		// Bytes per pixel
		i32 bpp = 0; 

		// Pixels data
		std::vector<u8> pixels;
	};


	// loads a image file
	image* load_image(const std::string& path);

}


