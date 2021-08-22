#include <destral/destral_font.h>
#include <destral/destral_renderer.h>

#include "thirdparty/SDL_ttf.h"
namespace ds {
	font::font(const char* file_path, i32 ptsize) {
		TTF_Font* f = TTF_OpenFont(file_path, ptsize);
		dsverifym(f != nullptr, std::format("{}", TTF_GetError()));
		_font = (void*)f;
	}

	font::~font() {
		if (_font) {
			TTF_CloseFont((TTF_Font*)_font);
		}
	}

	//sg_image font::build_text_texture(const char* utf8_text) {
	//	// TODO FONTS
	//	//SDL_Color color = { 1,0,0 };
	//	//auto surface = TTF_RenderUTF8_Solid((TTF_Font*)_font, utf8_text, color);
	//	//dsverify(surface != nullptr);
	//	//	
	//	//auto img = load_texture_memory((u8*)surface->pixels, surface->w, surface->h);
	//	//draw_texture(mat3(1.0), )
	//	//SDL_FreeSurface(surface);
	//}
	//sg_image font_build_text(font* font, const char* ut8_text) {

	//}
}