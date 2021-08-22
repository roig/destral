#pragma once
#include <destral/destral_image.h>

namespace ds {
	struct font {
		font(const char* file_path, i32 ptsize);
		~font();
		//sg_image build_text_texture(const char* utf8_text);
	private:
		void* _font = nullptr; // implementation detail
	};

}