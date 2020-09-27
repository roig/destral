#pragma once
#include <ap_debug.h>
#include <ap_sdl.h>
#include <destral/destral.h>
#include <destral/types.h>

using namespace ds;

struct player {
	float move_vel = 200.0f;


};

struct ball {
	float move_vel = 300.0f;


};





void update_player(entt::registry& r) {
	auto player_view = r.view<player, cp::transform>();
	for (auto e: player_view) { 
		auto& tr = player_view.get<cp::transform>(e);
		auto& pl = player_view.get<player>(e);
		float dt = ap_sdl_app_dt();
		glm::vec2 delta = { 0,0 };

		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_RIGHT)) {
			delta.x++;
		} 
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_LEFT)) {
			delta.x--;
		} 
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_UP)) {
			delta.y++;
		} 
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_DOWN)) {
			delta.y--;
		}
		delta = delta * pl.move_vel * dt;

		ds::tr::set_position(r, e, tr.position + delta);
	}
}

void update_ball(entt::registry& r) {

}


void ak_tick(entt::registry* r) {

	update_player(*r);
	update_ball(*r);
}

sg_image link;
void ak_init(entt::registry* r) {
	// create a default camera
	ds::create_camera(*r, {});

	// create the player entity with an attached rectangle
	auto pl_e = r->create();
	r->emplace<player>(pl_e);
	r->emplace<cp::transform>(pl_e);
	auto pl_spr = ds::create_rectangle(*r, {}, glm::vec2{ 300.f,50.f }, true, {0,1,0,1});
	tr::set_parent(*r, pl_spr, pl_e);

	

	//ds::create_line(*r, {{0,0},{-100,100}, {-200,-200} });

	//link = ds::rd::create_image("resources/link.png");
	//player = ds::create_sprite(*r, link, { 300, 300 });

}

void ak_shutdown(entt::registry* r) {
	

	
	
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
