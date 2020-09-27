#pragma once
#include <ap_debug.h>
#include <ap_sdl.h>
#include <destral/destral.h>
#include <destral/types.h>


entt::entity player;
void ak_tick(ds_world*r) {
	auto& tr = r->get<ds::cp::transform>(player);
	if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_RIGHT)) {
		ds::tr::set_position(*r, player, { tr.position.x + 10.0f, tr.position.y });
	} else if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_LEFT)){
		ds::tr::set_position(*r, player, { tr.position.x - 10.0f, tr.position.y });
	} else if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_UP)) {
		ds::tr::set_position(*r, player, { tr.position.x, tr.position.y +10.f});
	} else if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_DOWN)) {
		ds::tr::set_position(*r, player, { tr.position.x, tr.position.y - 10.f });
	}
}

sg_image link;
void ak_init(ds_world*r) {
	entt::entity e = ds::create_camera(*r, {});
	(void)e;

	ds::create_rectangle(*r, {}, glm::vec2{ 1270.f,200.f }, false, {0,0,1,1});
	ds::create_line(*r, {{0,0},{-100,100}, {-200,-200} });

	link = ds::rd::create_image("resources/link.png");
	player = ds::create_sprite(*r, link, { 300, 300 });

}

void ak_shutdown(ds_world* r) {
	

	
	
}

int main() {
	ds_app_desc app_desc = {0};

	app_desc.frame_cb = ak_tick;
	app_desc.init_cb = ak_init;
	app_desc.cleanup_cb = ak_shutdown;
	app_desc.window_name = "Arkanoid Game";
	app_desc.window_width = 1280;
	app_desc.window_height = 720;
	

	return ds_app_run(&app_desc);
}
