#pragma once
#include <destral/destral_math.h>
#include <destral/thirdparty/sokol_gfx.h>
#include <vector>

namespace ds {
	struct texture {
		static constexpr const char* cp_name = "ds_texture_cp";
		static constexpr const char* e_name = "ds_texture";
		sg_image gpu_texid = { .id = 0 };
		~texture();
	};




	struct image {
		// the size in pixels of the image
		ivec2 size = { 0,0 };
		
		// Bytes per pixel
		i32 bpp = 0; 

		// Pixels data (bytes)
		std::vector<u8> pixels;

		// sg_image sokol image texture reference
		sg_image gpu_texid = { .id = 0 };
		~image();
	};
	



}


