#pragma once


#include <ap_debug.h>
#include <ap_sdl.h>
#include <destral/destral.h>
#include <destral/types.h>
#include <destral/render.h>
#include <destral/ecs.h>
#include <destral/math_funs.h>
using namespace ds;

struct player {
	float move_vel = 700.0f;
};

struct ball {
	float move_vel = 800.f;
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
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_UP)) {
			delta.y++;
		} 
		if (ap_sdl_key_pressed(ap_sdl_app_input(), SDLK_DOWN)) {
			delta.y--;
		}
		delta = delta * pl.move_vel * dt;

		ds::co::move(r, e, delta, true);
	}
}

void update_ball(entt::registry& r) {
	auto balls_view = r.view<ball, cp::transform>();
	for (auto e : balls_view) {
		//auto& tr = player_view.get<cp::transform>(e);
		auto& b = balls_view.get<ball>(e);
		float dt =ap_sdl_app_dt();
		glm::vec2 delta = b.dir * b.move_vel * dt;
		auto hit = ds::co::move(r, e, delta, true);
		
		if (hit.blocking) {
			// Reflection vector
			b.dir = b.dir - 2.0f * glm::dot(b.dir, hit.normal) * hit.normal;
			b.dir = glm::normalize(b.dir);

			if (r.has<block>(hit.entity_hit)) {
				// If entity has BlockData means that we hit a block, destroy it!!
				ecs::destroy(r, hit.entity_hit);
			} else if (r.has<ball>(hit.entity_hit)) {
				// If it's another ball, change directions of the balls following the hit normal.
				auto& ball_cp = r.get<ball>(hit.entity_hit);
				ball_cp.dir = -hit.normal;
				b.dir = hit.normal;
			}
		}
	}
}


void ak_tick(entt::registry& r) {

	update_player(r);
	update_ball(r);
	
	
}

entt::entity create_wall(entt::registry& r, glm::vec2 position, glm::vec2 wall_size) {
	auto wall_e = ds::create_collider_rect(r, wall_size / 2.0f, position);
	auto rect_rd_e = ds::create_rectangle(r, {}, wall_size, true);
	tr::set_parent(r, rect_rd_e, wall_e);
	ecs::add_to_group(r, wall_e, rect_rd_e);
	return wall_e;
}

as::id tex_player_id = as::id_null;
as::id spr_player_id = as::id_null;
as::id spr_block_id = as::id_null;
as::id spr_ball_id = as::id_null;
entt::entity create_block(entt::registry& r, glm::vec2 position) {
	auto block_size_pixels = glm::vec2{ 385, 129 };
	auto block_ppu = 4.f;
	auto block_e = ds::create_collider_rect(r, block_size_pixels / block_ppu / 2.0f, position);
	r.emplace<block>(block_e);
	spr_block_id = as::create<sprite>();
	auto block_spr = as::get<sprite>(spr_block_id);
	block_spr->init_from_texture(tex_player_id);
	block_spr->src_rect = rect::from_size({ 0, 0 }, block_size_pixels); // select the sprite of block
	block_spr->ppu = block_ppu;
	auto block_spr_e = ds::create_sprite(r, spr_block_id);
	tr::set_parent(r, block_spr_e, block_e);
	ecs::add_to_group(r, block_e, block_spr_e);
	return block_e;
}

void create_ball(entt::registry& r, glm::vec2 position) {
	auto ball_radius = 66.f;
	auto ball_ppu = 2.5f;
	auto ball_e = ds::create_collider_circle(r, ball_radius / ball_ppu / 2.f, position);
	r.emplace<ball>(ball_e);
	spr_ball_id = as::create<sprite>();
	auto block_spr = as::get<sprite>(spr_ball_id);
	block_spr->init_from_texture(tex_player_id);
	block_spr->src_rect = rect::from_size({ 1403, 651 }, { ball_radius, ball_radius }); // select the sprite of block
	block_spr->ppu = ball_ppu;
	auto ball_spr_e = ds::create_sprite(r, spr_ball_id);
	tr::set_parent(r, ball_spr_e, ball_e);
	ecs::add_to_group(r, ball_e, ball_spr_e);
}

void ak_init(entt::registry& r) {
	// create a default camera
	ds::create_camera(r, {});

	// create the player entity that is in the root a collider
	{
		auto sprite_size = glm::vec2{ 349, 69 };
		auto player_ppu = 2.5f;
		auto pl_e = ds::create_collider_rect(r, sprite_size / player_ppu / 2.0f, { 0, -300 });
		r.emplace<player>(pl_e);
		// setup sprite player child
		tex_player_id = as::create_from_file("resources/BreakoutTiles.png");
		spr_player_id = as::create<sprite>();
		auto spr = as::get<sprite>(spr_player_id);
		spr->init_from_texture(tex_player_id);
		spr->src_rect = rect::from_size({ 0, 910 }, sprite_size); // select the sprite of the player
		spr->ppu = player_ppu;
		// set it as a child
		auto spr_pl_e = ds::create_sprite(r, spr_player_id);
		tr::set_parent(r, spr_pl_e, pl_e);
		ecs::add_to_group(r, pl_e, spr_pl_e);
	}


	// CREATE BLOCKS
	{
		for (int i = -5; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				create_block(r, { 96.25 * i, 250 - (32.25 * j) });
			}
		}
		/*create_block(r, { 96.25*5,250 });
		create_block(r, { 96.25,32.25+250 });*/
	}

	// CREATE THE BALL
	{
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
		create_ball(r, {});
	}

	{ // Create walls
		create_wall(r, { 0, 360 }, { 1280, 16 });
		create_wall(r, { -640, 0 }, { 16,720});
		create_wall(r, { 640, 0 }, { 16,720 });

		create_wall(r, { 0, -360 }, { 1280, 16 });
	}
}


void ak_shutdown(entt::registry& r) {
	
}

int main() {
	ds::platform_app_desc app_desc;
	app_desc.tick_cb = ak_tick;
	app_desc.init_cb = ak_init;
	app_desc.cleanup_cb = ak_shutdown;
	app_desc.window_name = "Arkanoid Game";
	app_desc.window_width = 1280;
	app_desc.window_height = 720;
	return ds::app_run(app_desc);
}

