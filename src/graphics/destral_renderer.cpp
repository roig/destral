#include <destral/graphics/destral_renderer.h>
#include "../backends/destral_platform_backend.h"
#include <destral/ecs/destral_ecs.h>

#define AP_GL33CORE_IMPL
#include <destral/thirdparty/ap_gl33core.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_LOG(msg) DS_LOG(msg)
#define SOKOL_ASSERT(c) dscheck(c)
#define SOKOL_EXTERNAL_GL_LOADER
#include <destral/thirdparty/sokol_gfx.h>

#define STBI_ASSERT(x) dscheck(x)
#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"

#include <unordered_map>
#include <map>

#include <filesystem>

namespace ds::rd {

	sg_image load_texture(const std::string& filename) {
		int x,y,n;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *pixels = stbi_load(filename.c_str(), &x, &y, &n, 0);
		if (pixels != nullptr) {

			// ... process data if not null ...
			// ... x = width, y = height, n = # 8-bit components per pixel ...
			// ... replace '0' with '1'..'4' to force that many components per pixel
			// ... but 'n' will always be the number that it would have been if you said 0
			sg_image_desc image_desc = { 0 };
			image_desc.width = x;
			image_desc.height = y;
			image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
			image_desc.min_filter = SG_FILTER_NEAREST;
			image_desc.mag_filter = SG_FILTER_NEAREST;
			image_desc.data.subimage[0][0] = {
				  .ptr = pixels,
				  .size = sizeof(pixels)
			};
			
			image_desc.label = "texture";
			auto img = sg_make_image(image_desc);
			stbi_image_free(pixels);
			return img;
		}
		DS_WARNING(fmt::format("Error loading the image: {}. Current path: {}", filename, std::filesystem::current_path().string()));

		
		return {0}; // TODO BAD
	}

	void destroy_texture(sg_image texture) {
		sg_destroy_image(texture);
	}

	struct mesh_data {
		sg_bindings bind = { 0 };
		int vertex_start = 0;
		int vertex_count = 0;
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
		// Per frame vertex buffer object
		static constexpr std::size_t MAX_VERTEX_DATA_BYTES_SIZE = 250000 * 4; // Maximum vertex data in the VBO
		sg_buffer vbo = { 0 };
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
		tri_textured_pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		tri_textured_pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
		tri_textured_pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		tri_textured_pipdesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
		g_rs.tris_textured_pip = sg_make_pipeline(tri_textured_pipdesc);
		

	}



	
	/// //////////////////////////////////////
	

	void init() {
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
	}

	void shutdown() {
		sg_shutdown();
	}

	void before_render() {

	}

	void tick() {

	}



	void after_render() {
		render_present();
	}

	void render_present() {

		// upload all the vertex data to the gpu
		sg_range vbo_range;
		vbo_range.ptr = g_rs.vertex_data.data();
		vbo_range.size = g_rs.vertex_data.size() * sizeof(float) <= g_rs.MAX_VERTEX_DATA_BYTES_SIZE ? g_rs.vertex_data.size() * sizeof(float) : g_rs.MAX_VERTEX_DATA_BYTES_SIZE;
		if (g_rs.vertex_data.size() * sizeof(float) > g_rs.MAX_VERTEX_DATA_BYTES_SIZE) {
			DS_WARNING("Vertex data exceds the maximum VBO size, probably some primitives will be not rendered.");
		}
		sg_update_buffer(g_rs.vbo, vbo_range);
		
		
		

		ivec2 draw_sz;
		platform_backend::get_drawable_size(&draw_sz.x, &draw_sz.y);
		/* default pass action (clear to grey) */
		sg_pass_action pass_action = { 0 };
		pass_action.colors[0] = { .action = SG_ACTION_CLEAR, .value = {0.5f, 0.5f, 0.5f, 1.0f } };
		
		sg_begin_default_pass(&pass_action, draw_sz.x, draw_sz.y);

		for (auto &z : g_rs.render_list) {
			//DS_LOG(fmt::format("Z : {}", z.first));
			for (auto& pip : z.second) {
				
				sg_apply_pipeline(sg_pipeline(pip.first));
				for (auto& md : pip.second) {
					sg_apply_bindings(md.bind);
					//	sg_apply_uniforms(..) // this is optinal to update uniform data when using custom shaders
					sg_draw(md.vertex_start, md.vertex_count, 1);
					//sg_draw(0, 12, 1);
				}
			}
		}
		sg_end_pass();
		sg_commit();

		g_rs.vertex_data.clear();
		g_rs.render_list.clear();
	
	}


	void rd::draw_line(const std::vector<vec2>& points, vec4 color, i32 depth) {
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

	void draw_circle(vec2 center, float radius, vec4 color, i32 depth) {
		const i32 segments = 24;
		const float increment = glm::two_pi<float>() / segments;

		std::vector<glm::vec2> per_pos; // perimeter positions
		for (float currAngle = 0.0f; currAngle <= glm::two_pi<float>(); currAngle += increment) {
			per_pos.push_back({center.x + radius * cos(currAngle), center.y + radius * sin(currAngle) });
		}

		// close the line
		per_pos.push_back(per_pos[0]);
		draw_line(per_pos, color, depth);
		return;
	}
	
	void draw_fill_circle(vec2 center, float radius , vec4 color, i32 depth) {
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
		for (auto i = 0; i < (per_pos.size() - 1); i++) 			{
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
	
	void draw_fill_rect(const mat3& tx, vec2 size, vec4 color, i32 depth) {
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

	void draw_rect(const mat3& tx, vec2 size, vec4 color, i32 depth) {
		const vec2 half_size = size / 2.0f;
		const std::vector<glm::vec2> points {
			tx * vec3{ -half_size.x, half_size.y, 1},
			tx * vec3{ +half_size.x, half_size.y , 1},
			tx * vec3{ +half_size.x, -half_size.y, 1},
			tx * vec3{ -half_size.x, -half_size.y, 1 },
			tx * vec3{ -half_size.x, half_size.y, 1 }
		};
		draw_line(points, color, depth);
	}

	// size world units (aqui tinc dubtes..)
	// uv_rect: normalized uv coordinates rectangle
	void draw_texture(const mat3& tx, sg_image texture, vec2 size, rect uv_rect, vec4 color, i32 depth) {
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
