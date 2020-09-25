#pragma once
#include <destral/destral.h>
#include <destral/render.h>
#include <ap_sdl.h>
#include <ap_debug.h>
#include <ap_opengl.h>

GLuint g_tex;
void ak_tick(void *r) {
	//ds_rd_draw_all(g_tex);
}

void ak_init(void *r) {
	//g_tex = ap_gl_texture_load("resources/link.png");
}

void ak_shutdown(void* r) {
	

	
	
}

int main() {
	struct ds_app_desc app_desc = {0};

	app_desc.frame_cb = ak_tick;
	app_desc.init_cb = ak_init;
	app_desc.cleanup_cb = ak_shutdown;
	app_desc.window_name = "Arkanoid Game";
	app_desc.window_width = 1280;
	app_desc.window_height = 720;
	

	return ds_app_run(&app_desc);
}
