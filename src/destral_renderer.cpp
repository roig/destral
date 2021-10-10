#include <destral/destral_renderer.h>
#include "backends/destral_platform_backend.h"
#include <destral/destral_ecs.h>
#include <destral/thirdparty/ap_gl33core.h>
#include <destral/destral_texture.h>
#include <unordered_map>
#include <map>

#include "thirdparty/SDL_ttf.h"


namespace ds::ttf {



	void ttf_init() {
		dsverifym(TTF_Init() == 0, std::format("{}", TTF_GetError())    );
	}

	void ttf_deinit() {
		TTF_Quit();
	}

	////ptsize: Point size(based on 72DPI) to load font as.This basically translates to pixel height.
	//void ttf_load_fond(const char* file_path, i32 ptsize) {
	//	TTF_Font* f = TTF_OpenFont(file_path, ptsize);
	//	dsverifym(f != nullptr, std::format("{}", TTF_GetError()));
	//}

	//void ttf_render_text_fast(const char* utf8_text, vec2 position, TTF_Font* font, SDL_Color fg) {

	//}
}


// Current limitations
// * There can be only one camera, check render_set_camera, 
//		Why? because sg_update_buffer can't be updated inside the same frame.
//		Whe should use sg_append_buffer but it's not implemented now.

namespace ds {
	








	struct mesh_data {
		sg_bindings bind = { 0 };
		int vertex_start = 0;
		int vertex_count = 0;
	};

	struct camera_data {
		camera_data(const vec4& camera_vp, const mat3& camera_ltw, float aspect, float ortho_width) {
			dsverify(aspect >= 0);
			const float half_vsize = ortho_width / aspect;
			ivec2 vp_size;
			platform_backend::get_drawable_size(&vp_size.x, &vp_size.y);
			projection_matrix = glm::ortho(-aspect * half_vsize, aspect * half_vsize, -half_vsize, half_vsize);
			view_matrix = glm::inverse(camera_ltw);
			scis = { (i32)(camera_vp.x * vp_size.x),
				(i32)(camera_vp.y * vp_size.y),
				(i32)((camera_vp.z * vp_size.x) - (camera_vp.x * vp_size.x)),
				(i32)((camera_vp.w * vp_size.y) - (camera_vp.y * vp_size.y)) };
			vp = { (i32)(camera_vp.x * vp_size.x),
				(i32)(camera_vp.y * vp_size.y),
				(i32)((camera_vp.z * vp_size.x) - (camera_vp.x * vp_size.x)),
				(i32)((camera_vp.w * vp_size.y) - (camera_vp.y * vp_size.y)) };
		}
		ivec4 vp;
		ivec4 scis;
		mat3 projection_matrix;
		mat3 view_matrix;
	};

	struct renderer_state {
		sg_shader non_textured_sh = { 0 };
		sg_shader textured_sh = { 0 };
		
		sg_pipeline tris_non_textured_pip = { 0 };
		sg_pipeline tris_textured_pip = { 0 };
		sg_pipeline lines_pip = { 0 };

		// -z is inside, z+ is front
		std::map<i32, std::unordered_map<std::uint32_t /* sg_pipeline */, std::vector<mesh_data> > > render_list;

		std::vector<float> vertex_data;

		// Holds the vertex data vertices transformed
		std::vector<float> vertex_data_transformed;

		// Per frame vertex buffer object
		static constexpr std::size_t MAX_VERTEX_DATA_BYTES_SIZE = 250000 * 4; // Maximum vertex data in the VBO
		sg_buffer vbo = { 0 };
		
		std::vector< camera_data > cameras;
	};

	static renderer_state g_rs; // renderer state


	void create_shaders_and_pipelines() {
		sg_shader_desc sh_textured_desc = { 0 };
		sh_textured_desc.fs.images[0] = { .name = "tex", .image_type = SG_IMAGETYPE_2D },
		sh_textured_desc.vs.source =
			"#version 330\n"
			"layout(location=0) in vec2 position;\n"
			"layout(location=1) in vec4 color0;\n"
			"layout(location=2) in vec2 texcoord0;\n"
			"out vec4 color;\n"
			"out vec2 uv;"
			"void main() {\n"
			"  gl_Position = vec4(position,0,1);\n"
			"  color = color0;\n"
			"  uv = texcoord0;\n"
			"}\n";
		sh_textured_desc.fs.source =
			"#version 330\n"
			"uniform sampler2D tex;"
			"in vec4 color;\n"
			"in vec2 uv;\n"
			"out vec4 frag_color;\n"
			"void main() {\n"
			"  frag_color = texture(tex, uv) * color;\n"
			"}\n";
		///https://github.com/floooh/sokol-samples/blob/master/glfw/texcube-glfw.c
		g_rs.textured_sh = sg_make_shader(sh_textured_desc);


		/* no textured shader */
		sg_shader_desc sh_non_textured_desc = { 0 };
		sh_non_textured_desc.vs.source =
			"#version 330\n"
			"layout(location=0) in vec2 position;\n"
			"layout(location=1) in vec4 color0;\n"
			"layout(location=2) in vec2 texcoord;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"  gl_Position = vec4(position,0,1);\n"
			"  color = color0;\n"
			"}\n";
		sh_non_textured_desc.fs.source =
			"#version 330\n"
			"in vec4 color;\n"
			"out vec4 frag_color;\n"
			"void main() {\n"
			"  frag_color = color;\n"
			"}\n";
		g_rs.non_textured_sh = sg_make_shader(sh_non_textured_desc);

		/* create a pipeline object (default render states are fine for triangle) */
		sg_pipeline_desc spipdesc = { 0 };
		spipdesc.shader = g_rs.non_textured_sh;
		spipdesc.label = "tris-non-textured-pipeline";
		spipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		spipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
		spipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		spipdesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
		g_rs.tris_non_textured_pip = sg_make_pipeline(spipdesc);


		sg_pipeline_desc linespipdesc = { 0 };
		linespipdesc.shader = g_rs.non_textured_sh;
		linespipdesc.label = "lines-pipeline";
		linespipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		linespipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
		linespipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		linespipdesc.primitive_type = SG_PRIMITIVETYPE_LINE_STRIP;
		g_rs.lines_pip = sg_make_pipeline(linespipdesc);

		sg_pipeline_desc tri_textured_pipdesc = { 0 };
		tri_textured_pipdesc.shader = g_rs.textured_sh;
		tri_textured_pipdesc.label = "tris-textured-pipeline";
		
		// Default blending
		tri_textured_pipdesc.colors[0].blend.enabled = true;
		tri_textured_pipdesc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		tri_textured_pipdesc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		tri_textured_pipdesc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
		tri_textured_pipdesc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA;
		tri_textured_pipdesc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		tri_textured_pipdesc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;

		tri_textured_pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		tri_textured_pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
		tri_textured_pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		tri_textured_pipdesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
		g_rs.tris_textured_pip = sg_make_pipeline(tri_textured_pipdesc);
		

	}



	
	/// //////////////////////////////////////
	

	void render_init() {
		// Create GL context
		platform_backend::gl_context_create();

		// Load GL functions
		gladLoadGL();

		// Setup Sokol
		sg_desc desc = { 0 };
		desc.buffer_pool_size = 64;
		desc.image_pool_size = 64;
		desc.pipeline_pool_size = 64;
		sg_setup(desc);

		// reserve the memory for this vector
		g_rs.vertex_data.reserve(g_rs.MAX_VERTEX_DATA_BYTES_SIZE);

		// create the main Vertex Buffer Object that is update each frame
		sg_buffer_desc buffer_desc = { .size = g_rs.MAX_VERTEX_DATA_BYTES_SIZE, .usage = SG_USAGE_STREAM, .label = "vertex-vbo" };
		g_rs.vbo = sg_make_buffer(buffer_desc);

		// setup default pipelines
		create_shaders_and_pipelines();

		// initialized ttf
		ttf::ttf_init();
	}

	void render_deinit() {
		ttf::ttf_deinit();
		sg_shutdown();
	}

	// Returns the vertex data transformed by a projection and a view matrix
	std::vector<float> s_transform_vertex_data(const mat3& projection, const mat3& view) {
		const mat3 vp_mat = projection * view;
		std::vector<float> transformed_vertex_data = g_rs.vertex_data;
		for (size_t i = 0; i < transformed_vertex_data.size(); i = i + 8 /* vertex stride */) {
			const vec3 vpos = vp_mat * vec3(transformed_vertex_data[i], transformed_vertex_data[i + 1], 1);
			transformed_vertex_data[i] = vpos.x;
			transformed_vertex_data[i + 1] = vpos.y;
		}
		return transformed_vertex_data;
	}

	void s_gpu_upload_vertex_data(const std::vector<float>& vdata) {
		// upload all the vertex data to the gpu
		sg_range vbo_range;
		vbo_range.ptr = vdata.data();
		vbo_range.size = vdata.size() * sizeof(float) <= g_rs.MAX_VERTEX_DATA_BYTES_SIZE ? vdata.size() * sizeof(float) : g_rs.MAX_VERTEX_DATA_BYTES_SIZE;
		if (vdata.size() * sizeof(float) > g_rs.MAX_VERTEX_DATA_BYTES_SIZE) {
			DS_WARNING("Vertex data exceds the maximum VBO size, probably some primitives will be not rendered.");
		}

		// only update the buffer if it has data
		if (vdata.size() != 0) {
			//sg_append_buffer(g_rs.vbo, vbo_range);
			sg_update_buffer(g_rs.vbo, vbo_range);
		}
	}

	void s_render_all_primitives(const mat3& projection, const mat3& view) {
		auto vdata = s_transform_vertex_data(projection, view);
		s_gpu_upload_vertex_data(vdata);
		
		for (auto& z : g_rs.render_list) {
			//DS_LOG(std::format("Z : {}", z.first));
			for (auto& pip : z.second) {
				sg_apply_pipeline(sg_pipeline(pip.first));
				for (auto& md : pip.second) {
					sg_apply_bindings(md.bind);
					//	sg_apply_uniforms(..) // this is optinal to update uniform data when using custom shaders
					sg_draw(md.vertex_start, md.vertex_count, 1);
				}
			}
		}
	}

	// Clears the entire screen
	void s_render_clear_screen() {

		ivec2 vp_size;
		platform_backend::get_drawable_size(&vp_size.x, &vp_size.y);
		sg_pass_action pass_action = { 0 };
		pass_action.colors[0] = { .action = SG_ACTION_CLEAR, .value = {0.3f, 0.3f, 0.3f, 1.0f } };
		sg_begin_default_pass(&pass_action, vp_size.x, vp_size.y);
		//sg_apply_viewport(0, 0, vp_size.x, vp_size.y, false);
		//sg_apply_scissor_rect(0, 0, vp_size.x, vp_size.y, false);
		sg_end_pass();

		
	}

	void s_render_camera(const camera_data& cam) {
		// This sets a default pass (don't care = don't clear the screen)
		ivec2 vp_size;
		platform_backend::get_drawable_size(&vp_size.x, &vp_size.y);
		sg_pass_action pass_action = {0};
		pass_action.colors[0] = { .action = SG_ACTION_DONTCARE };
		sg_begin_default_pass(&pass_action, vp_size.x, vp_size.y);

		// Setup viewport and scissors for this camera
		sg_apply_viewport(cam.vp.x, cam.vp.y, cam.vp.z, cam.vp.w, false);
		sg_apply_scissor_rect(cam.scis.x, cam.scis.y, cam.scis.z, cam.scis.w, false);

		// Draw all the primitives with this projection and view matrices
		s_render_all_primitives(cam.projection_matrix, cam.view_matrix);
		sg_end_pass();
	}



	void s_render_all_cameras() {
		if (g_rs.cameras.size() == 0) {
			// If no camera added found, use a default camera with a ortho_width of 1 world unit
			// and center it at 0,0
			ivec2 vp_size;
			platform_backend::get_drawable_size(&vp_size.x, &vp_size.y);
			const float aspect = vp_size.x / (float)vp_size.y;
			const float ortho_width = 1.0f;
			camera_data default_cam({0.f, 0.f, 1.f, 1.f }, mat3(1.0f), aspect, 1.0f);
			s_render_camera(default_cam);
		} else {
			for (size_t i = 0; i < g_rs.cameras.size(); i++) {
				s_render_camera(g_rs.cameras[i]);
			}
		}
	}

	void render_add_camera(const mat3& camera_ltw, vec4 camera_vp, float aspect, float ortho_width)	{
		// TODO: we only support one camera for now
		camera_data cd(camera_vp, camera_ltw, aspect, ortho_width);
		if (g_rs.cameras.empty()) {
			g_rs.cameras.push_back(cd);
		} else {
			// TODO: here we always use ONE camera..
			g_rs.cameras[0] = cd;
		}
	}

	// This flushes all the primitives and cameras
	void s_render_clear() {
		g_rs.vertex_data.clear();
		g_rs.render_list.clear();
		g_rs.cameras.clear();
	}

	// This draws all the cameras 
	void render_present() {
		// start the frame by clearing the full screen
		s_render_clear_screen();
		// render all the primitives in all the cameras
		s_render_all_cameras();
		// ends the frame
		sg_commit();
		s_render_clear();
	}

	void render_line(const std::vector<vec2>& points, vec4 color, i32 depth) {
		mesh_data md;
		md.bind.vertex_buffers[0] = g_rs.vbo;
		md.vertex_start = (int)g_rs.vertex_data.size() / 8;
		md.vertex_count = (int)points.size();

		// fill vertex data
		for (auto& p : points) {
			g_rs.vertex_data.push_back(p.x);
			g_rs.vertex_data.push_back(p.y);
			g_rs.vertex_data.push_back(color.r);
			g_rs.vertex_data.push_back(color.g);
			g_rs.vertex_data.push_back(color.b);
			g_rs.vertex_data.push_back(color.a);
			g_rs.vertex_data.push_back(0);
			g_rs.vertex_data.push_back(0);
		}

		// Add the primitive to the render list
		g_rs.render_list[depth][g_rs.lines_pip.id].push_back(md);
	}

	void render_circle(vec2 center, float radius, vec4 color, i32 depth) {
		const i32 segments = 24;
		const float increment = glm::two_pi<float>() / segments;

		std::vector<glm::vec2> per_pos; // perimeter positions
		for (float currAngle = 0.0f; currAngle <= glm::two_pi<float>(); currAngle += increment) {
			per_pos.push_back({center.x + radius * cos(currAngle), center.y + radius * sin(currAngle) });
		}

		// close the line
		per_pos.push_back(per_pos[0]);
		render_line(per_pos, color, depth);
		return;
	}
	
	void render_fill_circle(vec2 center, float radius , vec4 color, i32 depth) {
		const i32 segments = 24;
		const float increment = glm::two_pi<float>() / segments;

		std::vector<glm::vec2> per_pos; // perimeter positions
		for (float currAngle = 0.0f; currAngle <= glm::two_pi<float>(); currAngle += increment) {
			per_pos.push_back({ center.x + radius * cos(currAngle), center.y + radius * sin(currAngle) });
		}

		mesh_data md;
		md.bind.vertex_buffers[0] = g_rs.vbo;
		md.vertex_start = (int)g_rs.vertex_data.size() / 8;
		md.vertex_count = 0;


		// fill vertex data, all circle triangles minus one
		for (size_t i = 0; i < (per_pos.size() - 1); i++) 			{
			const float vertex[] = {
				center.x, center.y,				color.r, color.g, color.b, color.a, 0,0,	//0
				per_pos[i].x, per_pos[i].y,     color.r, color.g, color.b, color.a, 0,0,	//1
				per_pos[i + 1].x, per_pos[i + 1].y,     color.r, color.g, color.b, color.a, 0,0		//2
			};
			md.vertex_count = md.vertex_count + 3;
			g_rs.vertex_data.insert(g_rs.vertex_data.end(), vertex, vertex + (3 * 8));
		}

		// last triangle
		const float vertex[] = {
			// positions            colors
			center.x, center.y,													color.r, color.g, color.b, color.a, 0,0,	//0
			per_pos[per_pos.size() - 1].x, per_pos[per_pos.size() - 1].y,		color.r, color.g, color.b, color.a, 0,0,	//1
			per_pos[0].x, per_pos[0].y,											color.r, color.g, color.b, color.a, 0,0		//2

		};
		md.vertex_count = md.vertex_count + 3;
		g_rs.vertex_data.insert(g_rs.vertex_data.end(), vertex, vertex + (3 * 8));

		// Add the primitive to the render list
		g_rs.render_list[depth][g_rs.tris_non_textured_pip.id].push_back(md);
		
	}
	
	void render_fill_rect(const mat3& tx, vec2 size, vec4 color, i32 depth) {
		const vec2 half_size = size / 2.0f;
		
		const glm::vec2 positions[]{
			tx * vec3{ -half_size.x, half_size.y, 1 },
			tx * vec3{ +half_size.x, half_size.y, 1 },
			tx * vec3{ +half_size.x, -half_size.y, 1 },
			tx * vec3{ -half_size.x, -half_size.y, 1 }
		};

		const float vertex[] = {
			// positions            colors
			positions[0].x, positions[0].y,     color.r, color.g, color.b, color.a, 0,0,	//0
			positions[1].x, positions[1].y,     color.r, color.g, color.b, color.a, 0,0,	//1
			positions[2].x, positions[2].y,     color.r, color.g, color.b, color.a, 0,0,	//2

			positions[0].x, positions[0].y,     color.r, color.g, color.b, color.a, 0,0,	//0
			positions[2].x, positions[2].y,     color.r, color.g, color.b, color.a, 0,0,	//2
			positions[3].x, positions[3].y,     color.r, color.g, color.b, color.a, 0,0		//3
		};

		mesh_data md;
		md.bind.vertex_buffers[0] = g_rs.vbo;
		md.vertex_start = (int)g_rs.vertex_data.size() / 8;
		md.vertex_count = 6;

		// Add the primitive to the render list
		g_rs.render_list[depth][g_rs.tris_non_textured_pip.id].push_back(md);

		// Insert the vertex data to the vertex array
		g_rs.vertex_data.insert(g_rs.vertex_data.end(), vertex, vertex + (6 * 8));
	}

	void render_rect(const mat3& tx, vec2 size, vec4 color, i32 depth) {
		const vec2 half_size = size / 2.0f;
		const std::vector<glm::vec2> points {
			tx * vec3{ -half_size.x, half_size.y, 1},
			tx * vec3{ +half_size.x, half_size.y , 1},
			tx * vec3{ +half_size.x, -half_size.y, 1},
			tx * vec3{ -half_size.x, -half_size.y, 1 },
			tx * vec3{ -half_size.x, half_size.y, 1 }
		};
		render_line(points, color, depth);
	}

	// size world units (aqui tinc dubtes..)
	// uv_rect: normalized uv coordinates rectangle
	//void render_texture(const mat3& tx, resource<image> image, vec2 size, rect uv_rect, vec4 color, i32 depth) {
	//	render_texture(tx, image->gpu_texid, size, uv_rect, color, depth);
	//}

	//void render_texture(registry *r, const mat3& model, resource texture_res, vec2 size, rect uv_rect, vec4 color, i32 depth) {
	//	if (texture_res.is_available()) {
	//		entity texture_e = texture_res.get();
	//		dsverify(r->entity_valid(texture_e));
	//		texture* tex = r->component_try_get<texture>(texture_e, texture::cp_name);
	//		dsverifym(tex, "Resource is not a texture!");
	//		render_texture(model, tex->gpu_texid, size, uv_rect, color, depth);
	//	} else {
	//		// TODO render fallback texture?
	//	}
	//}

	void render_texture(const mat3& tx, sg_image texture, vec2 size, rect uv_rect, vec4 color, i32 depth) {
		// TEST IMAGE
		//static sg_image test_img = { .id = 0 };

		//if (test_img.id == 0) {
		//	uint32_t pixels[4 * 4] = {
		//		0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
		//		0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
		//		0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
		//		0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
		//	};
		//	sg_image_desc test_img_desc = { 0 };
		//	test_img_desc.width = 4;
		//	test_img_desc.height = 4;
		//	test_img_desc.data.subimage[0][0] = SG_RANGE(pixels);
		//	test_img_desc.label = "cube-texture";
		//	test_img = sg_make_image(test_img_desc);
		//}


		const vec2 half_size = size / 2.0f;

		const glm::vec2 positions[]{
			tx * vec3{ -half_size.x, half_size.y, 1 },
			tx * vec3{ +half_size.x, half_size.y, 1 },
			tx * vec3{ +half_size.x, -half_size.y, 1 },
			tx * vec3{ -half_size.x, -half_size.y, 1 }
		};

		const float vertex[] = {
			// positions            colors
			positions[0].x, positions[0].y,     color.r, color.g, color.b, color.a, uv_rect.top_left().x, uv_rect.top_left().y,			//0
			positions[1].x, positions[1].y,     color.r, color.g, color.b, color.a, uv_rect.top_right().x, uv_rect.top_right().y,		//1
			positions[2].x, positions[2].y,     color.r, color.g, color.b, color.a, uv_rect.bottom_right().x, uv_rect.bottom_right().y,	//2

			positions[0].x, positions[0].y,     color.r, color.g, color.b, color.a, uv_rect.top_left().x, uv_rect.top_left().y,			//0
			positions[2].x, positions[2].y,     color.r, color.g, color.b, color.a, uv_rect.bottom_right().x, uv_rect.bottom_right().y,	//2
			positions[3].x, positions[3].y,     color.r, color.g, color.b, color.a, uv_rect.bottom_left().x, uv_rect.bottom_left().y,	//3
		};

		mesh_data md;
		md.bind.vertex_buffers[0] = g_rs.vbo;
		md.bind.fs_images[0] = texture;
		md.vertex_start = (int)g_rs.vertex_data.size() / 8;
		md.vertex_count = 6;


		// Add the primitive to the render list
		g_rs.render_list[depth][g_rs.tris_textured_pip.id].push_back(md);

		// Insert the vertex data to the vertex array
		g_rs.vertex_data.insert(g_rs.vertex_data.end(), vertex, vertex + (6 * 8));
	}



}
