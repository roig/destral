#pragma once

#include <destral/destral_common.h>
#include <destral/destral_math.h>
#include <destral/thirdparty/sokol_gfx.h>
#include <destral/destral_resource2.h>
#include <destral/destral_image.h>
#include <vector>

namespace ds {
	// Called to initialize the graphics renderer
	void render_init();

	// Shuts down the graphics renderer
	void render_deinit();

	// This draws all the primitives in all the cameras added. 
	// Clears all the primitives and cameras at the end.
	void render_present();

	// This sets a custom camera to render all the primitives
	//	The viewport is the rectangle into which the contents of the
	//	camera will be displayed, expressed as a factor (between 0 and 1)
	//	of the size of the screen window to which the camera is applied.
	//	By default, a view has a viewport which covers the entire screen:
	//	vec4(0.0, 0.0, 1.0, 1.0)
	//	
	//	|      *(1,1)
	//	|
	//	|______
	//	(0,0)
	//
	// TODO: we only support one camera per frame now
	void render_add_camera(const mat3& cam_ltw, vec4 cam_vp, float aspect, float ortho_width);

	// DRAW PRIMITIVES

	// Submit a draw line primitive
	void render_line(const std::vector<vec2>& points, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw circle primitive
	void render_circle(vec2 center, float radius = 25.f, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw filled circle primitive
	void render_fill_circle(vec2 center, float radius = 25.f, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw rectangle primitive
	void render_rect(const mat3& model, vec2 size, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw filled rectangle primitive
	void render_fill_rect(const mat3& model, vec2 size, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw texture rectangle primitive
	void render_texture(const mat3& model, sg_image texture = { 0 }, vec2 size = { 1,1 }, rect source_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);

	//void render_texture(registry* r, const mat3& model, resource texture, vec2 size = { 1,1 }, rect source_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);
	/*void draw_texture(const mat3& model, resource<image> img, vec2 size = { 1,1 }, rect source_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);*/
}
