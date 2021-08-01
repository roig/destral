#pragma once

#include <destral/destral_common.h>
#include <destral/destral_math.h>
#include <destral/thirdparty/sokol_gfx.h>
#include <destral/destral_resource.h>
#include <destral/destral_image.h>
#include <vector>

namespace ds {
	// Loads a texture
	sg_image load_texture_file(const std::string& filename);

	// Loads a texture to the gpu
	sg_image load_texture_memory(const u8* pixels_data, i32 width, i32 height);

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
	void draw_rect(const mat3& model, vec2 size, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw filled rectangle primitive
	void draw_fill_rect(const mat3& model, vec2 size, vec4 color = { 1,1,1,1 }, i32 depth = 0);

	// Submit a draw texture rectangle primitive
	void draw_texture(const mat3& model, sg_image texture = { 0 }, vec2 size = { 1,1 }, rect source_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);
	void draw_texture(const mat3& model, resource<image> img, vec2 size = { 1,1 }, rect source_rect = rect::from_size({ 0,0 }, { 1, 1 }), vec4 color = { 1,1,1,1 }, i32 depth = 0);







	//	/** The viewport is the rectangle into which the contents of the
//	* camera will be displayed, expressed as a factor (between 0 and 1)
//	* of the size of the screen window to which the camera is applied.
//	* By default, a view has a viewport which covers the entire screen:
//	* vec4(0.0, 0.0, 1.0, 1.0)
//	*
//	* |      *(1,1)
//	* |
//	* |______
//	* (0,0)
//	*/
//glm::vec4 viewport = glm::vec4(0.f, 0.f, 1.f, 1.f);

///** @brief color used when the camera clears their viewport */
//glm::vec4 clear_color = glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f };

//// half vertical size in world units
//float half_vsize = 360.0f;

//// aspect ratio of the camera, used to calculate the width, based on half_vsize
//float aspect = 16.0f / 9.0f;

	void draw_camera(ivec2 vp_size = ivec2(1280, 720), vec4 /* un rect potser? */camera_vp = vec4{ 0.5f, 0.5f, 0.5f, 1.0f }, const mat3& camera_ltw = mat3(1), float aspect = 16/9.0f, float half_vsize = 360.0, vec4 clear_color = vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
}
