#pragma once


#include <ap_debug.h>
#include <ap_sdl.h>
#include <destral/destral.h>
#include <destral/types.h>
#include <destral/render.h>

using namespace ds;




struct player {
	float move_vel = 500.0f;


};

struct ball {
	float move_vel = 600.0f;
	glm::vec2 dir = { 0, -1 };

};

struct block {
	
};

void update_player(entt::registry& r) {
	auto player_view = r.view<player, cp::transform>();
	for (auto e: player_view) { 
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
			//auto blockd = ;
			if (r.has<block>(hit.entity_hit)) {
				r.destroy(hit.entity_hit);
			}
		}
	}
}


void ak_tick(entt::registry* r) {

	update_player(*r);
	update_ball(*r);
	float f = ap_sdl_app_fps();
	AP_INFO("%.10f", f);
}

as::id tex_player_id = as::id_null;
as::id spr_player_id = as::id_null;
as::id spr_block_id = as::id_null;
as::id spr_ball_id = as::id_null;
void ak_init(entt::registry* r) {
	
	// create a default camera
	ds::create_camera(*r, {});

	// create the player entity that is in the root a collider
	{
		auto sprite_size = glm::vec2{ 349, 69 };
		auto pl_e = ds::create_collider_rect(*r, sprite_size / 2.0f, { 0, -300 });
		r->emplace<player>(pl_e);
		// setup sprite player child
		tex_player_id = as::create_from_file("resources/BreakoutTiles.png");
		spr_player_id = as::create<sprite>();
		auto spr = as::get<sprite>(spr_player_id);
		spr->init_from_texture(tex_player_id);
		spr->src_rect = rect::from_size({ 0, 910 }, sprite_size); // select the sprite of the player
		// set it as a child
		tr::set_parent(*r, ds::create_sprite(*r, spr_player_id), pl_e);
	}


	// CREATE BLOCKS
	{
		auto block_size = glm::vec2{ 385, 129 };
		auto block_e = ds::create_collider_rect(*r, block_size / 2.0f, { 0, 300 });
		r->emplace<block>(block_e);
		spr_block_id = as::create<sprite>();
		auto block_spr = as::get<sprite>(spr_block_id);
		block_spr->init_from_texture(tex_player_id);
		block_spr->src_rect = rect::from_size({ 0, 0 }, block_size); // select the sprite of block
		tr::set_parent(*r, ds::create_sprite(*r, spr_block_id), block_e);
	}

	// CREATE THE BALL
	{
		auto ball_size = glm::vec2{ 66,66 };
		auto ball_e = ds::create_collider_circle(*r, 66/2.f, { 0, 0 });
		r->emplace<ball>(ball_e);
		spr_ball_id = as::create<sprite>();
		auto block_spr = as::get<sprite>(spr_ball_id);
		block_spr->init_from_texture(tex_player_id);
		block_spr->src_rect = rect::from_size({ 1403, 651 }, ball_size); // select the sprite of block
		tr::set_parent(*r, ds::create_sprite(*r, spr_ball_id), ball_e);
	}
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

