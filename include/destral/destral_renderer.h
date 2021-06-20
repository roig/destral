#pragma once

#include <destral/destral_common.h>
#include <destral/destral_math.h>
#include <destral/thirdparty/sokol_gfx.h>
#include <destral/destral_resource.h>
#include <destral/destral_image.h>
#include <vector>

namespace ds {
	// Loads a texture
	sg_image load_texture(const std::string& filename);

	// Loads a texture to the gpu
	sg_image load_texture(const u8* pixels_data, i32 width, i32 height);

	// Destroys a texture
	void destroy_texture(sg_image texture);



	// Called to initialize the graphics renderer
	void render_init();

	// Shuts down the graphics renderer
	void render_shutdown();

	// Called once per frame (does nothing for now)
	void render_tick();

	// Called once per frame (does nothing for now)
	void render_before_render();

	// Called once per frame, calls render_present internally to draw all submitted primitives.
	void render_after_render();

	// This draws all the render primitives previously submitted
	void render_present();



	// DRAW PRIMITIVES

	// Submit a draw line primitive
	void draw_line(const std::vector<vec2>& points, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw circle primitive
	void draw_circle(vec2 center, float radius = 25.f, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw filled circle primitive
	void draw_fill_circle(vec2 center, float radius = 25.f, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw rectangle primitive
	void draw_rect(const mat3& tx, vec2 size, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw filled rectangle primitive
	void draw_fill_rect(const mat3& tx, vec2 size, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw texture rectangle primitive
	void draw_texture(const mat3& tx, sg_image texture = { 0 }, vec2 size = { 1,1 }, rect source_texture_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);
	void draw_texture(const mat3& tx, resource<image> img, vec2 size = { 1,1 }, rect source_texture_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);

}
