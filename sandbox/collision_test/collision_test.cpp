#pragma once


#include <ap_debug.h>
#include <ap_sdl.h>
#include <destral/destral.h>
#include <destral/archtypes.h>
#include <destral/render.h>
#include <destral/ecs.h>
#include <destral/math_funs.h>
using namespace ds;




struct player {
	float move_vel = 500.0f;


};

struct ball {
	float move_vel = 600000.0f;
	glm::vec2 dir = { 0, -1 };

};

struct block {

};

void update_player(entt::registry& r) {
	auto player_view = r.view<player, cp::transform>();
	for (auto e : player_view) {
		//auto& tr = player_view.get<cp::transform>(e);
		auto& pl = player_view.get<player>(e);
		float dt = ap_sdl_app_dt();
		glm::vec2 delta = { 0,0 };

		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_RIGHT)) {
			delta.x++;
		}
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_LEFT)) {
			delta.x--;
		}
		/*if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_UP)) {
			delta.y++;
		}
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_DOWN)) {
			delta.y--;
		}*/
		delta = delta * pl.move_vel * dt;

		ds::co::move(r, e, delta, true);
	}
}

void update_ball(entt::registry& r) {
	auto balls_view = r.view<ball, cp::transform>();
	for (auto e : balls_view) {
		//auto& tr = player_view.get<cp::transform>(e);
		auto& b = balls_view.get<ball>(e);
		float dt = ap_sdl_app_dt();
		glm::vec2 delta = b.dir * b.move_vel * dt;
		auto hit = ds::co::move(r, e, delta, true);

		if (hit.blocking) {
			// Reflection vector
			b.dir = b.dir - 2.0f * glm::dot(b.dir, hit.normal) * hit.normal;
			b.dir = glm::normalize(b.dir);
			// If entity has BlockData means that we hit a block, destroy it!!
			if (r.has<block>(hit.entity_hit)) {
				ecs::destroy(r, hit.entity_hit);
			}
		}
	}
}


void ak_tick(entt::registry* r) {

	update_player(*r);
	update_ball(*r);


}

entt::entity create_wall(entt::registry& r, glm::vec2 position, glm::vec2 wall_size) {
	auto wall_e = ds::create_collider_rect(r, wall_size / 2.0f, position);
	auto rect_rd_e = ds::create_rectangle(r, {}, wall_size, true);
	tr::set_parent(r, rect_rd_e, wall_e);
	ecs::add_to_group(r, wall_e, rect_rd_e);
	return wall_e;
}



void ak_init(entt::registry* r) {
	// create a default camera
	ds::create_camera(*r, {});

	// create the player entity that is in the root a collider
	{
		auto sprite_size = glm::vec2{ 349, 69 };
		auto player_ppu = 2.5f;
		auto pl_e = ds::create_collider_rect(*r, sprite_size / player_ppu / 2.0f, { 0, -300 });
		r->emplace<player>(pl_e);
		auto rect_pl_e = ds::create_rectangle(*r, {}, sprite_size / player_ppu, true);
		tr::set_parent(*r, rect_pl_e, pl_e);
		ecs::add_to_group(*r, pl_e, rect_pl_e);
	}

	// CREATE THE BALL
	{
		auto ball_radius = 66.f;
		auto ball_ppu = 2.5f;
		auto ball_e = ds::create_collider_circle(*r, ball_radius / ball_ppu, { 0, 0 });
		r->emplace<ball>(ball_e);

		auto ball_circle_e = ds::create_circle(*r, {}, ball_radius / ball_ppu, true);

		tr::set_parent(*r, ball_circle_e, ball_e);
		ecs::add_to_group(*r, ball_e, ball_circle_e);
	}

	{ // Create walls
		create_wall(*r, { 0, 360 }, { 1280, 16 });
	}
}


void ak_shutdown(entt::registry* r) {




}

int main() {
	ds::app_desc = { 0 };

	app_desc.frame_cb = ak_tick;
	app_desc.init_cb = ak_init;
	app_desc.cleanup_cb = ak_shutdown;
	app_desc.window_name = "Arkanoid Game";
	app_desc.window_width = 1280;
	app_desc.window_height = 720;


	return ds_app_run(&app_desc);
}
