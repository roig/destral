#include "render.h"

#include <ap_debug.h>
#include <ap_sdl.h>
#pragma warning( push )
#pragma warning(disable : 4505) // 
#pragma warning(disable : 4996) // disable fopen warning in cute_png..
#pragma warning(disable : 4189) // 
#include "cute_png.h"
#pragma warning( pop )


#include "transform.h"

#include <ap_gl33compat.h>


#include <entt/entity/registry.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include <filesystem>


//#include <glm/gtx/matrix_transform_2d.hpp>


using namespace ds;

void sprite::init_from_texture(as::id tex_id) {
    texture_id = tex_id;
    auto t = as::get<ds::texture>(tex_id);
    if (!t) {
        return;
    }
    auto img_info = sg_query_image_info(t->image);
    src_rect = rect::from_size({ 0,0 }, { img_info.width, img_info.height });
}





sg_pipeline g_base_lines_pip = { 0 };
sg_pipeline g_base_triangle_strip_pip = { 0 };
sg_pipeline g_base_textured_pip = { 0 };


sg_shader g_shd_base = { 0 };
sg_shader g_shd_tex = { 0 };



// global vertices buffer
#define BUFFER_SIZE 16384
 
std::array<float, BUFFER_SIZE> vert_buf;
std::array<std::uint32_t, BUFFER_SIZE> index_buf;

//static float    tex_buf[BUFFER_SIZE * 8];
//static float    vert_buf[BUFFER_SIZE * 8];
//static uint8_t  color_buf[BUFFER_SIZE * 16];
//static uint32_t index_buf[BUFFER_SIZE * 6];

sg_buffer g_vbo = { 0 };
sg_buffer g_ibo = { 0 };

void register_render_asset_types() {
    using namespace as;
    factory_type sprite_factory = {0};
    sprite_factory.type_id = sprite::type_id;
    sprite_factory.type_name = "Sprite";
    sprite_factory.create_default = []() { return std::unique_ptr<void, std::function<void(void*)>>(new ds::sprite{}, [](void* ptr) {delete static_cast<texture*>(ptr); }); };
    sprite_factory.create_from_file = nullptr;
    sprite_factory.can_import_from_file = nullptr;
    register_factory_type(sprite_factory);

    factory_type texture_factory = { 0 };
    texture_factory.type_id = texture::type_id;
    texture_factory.type_name = "Texture";
    texture_factory.create_default = nullptr;
    texture_factory.create_from_file =
        [](const std::string& file) { 
        std::unique_ptr< void, std::function<void(void*)>> asset_ptr{ nullptr,
            [](void* ptr) {
                texture* t = static_cast<texture*>(ptr);
                sg_destroy_image(t->image);
                delete t;
            }
        };

        sg_image img = rd::create_image(file.c_str());
        if (img.id == SG_INVALID_ID) {
            return asset_ptr;
        } else {
            asset_ptr.reset(new texture{ img });
        }
        return asset_ptr;
    };
    texture_factory.can_import_from_file = [](const std::string& file) {
        if (std::filesystem::path(file).has_extension()) {
            return std::filesystem::path(file).extension().string() == ".png";
        }
        return false;
    };
    register_factory_type(texture_factory);

}

void rd::init() {
    sg_desc d = { 0 };
    sg_setup(&d);

    register_render_asset_types();

    // First create a dynamic streaming buffer
  /*  sg_buffer_desc buff_d = { 0 };
    buff_d.type = SG_BUFFERTYPE_VERTEXBUFFER;
    buff_d.usage = SG_USAGE_STREAM;
    buff_d.size = vert_buf.size() * sizeof(float);
    g_vbo = sg_make_buffer(buff_d);
    
    sg_buffer_desc buff_ibo_d = { 0 };
    buff_ibo_d.type = SG_BUFFERTYPE_INDEXBUFFER;
    buff_ibo_d.usage = SG_USAGE_STREAM;
    buff_ibo_d.size = index_buf.size() * sizeof(std::uint32_t);
    g_ibo = sg_make_buffer(buff_ibo_d);*/


    sg_shader_desc sh1 = { 0 };
    sh1.fs.images[0] = { .name = "tex", .type = SG_IMAGETYPE_2D },
    sh1.vs.source =
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
    sh1.fs.source = "#version 330\n"
        "uniform sampler2D tex;"
        "in vec4 color;\n"
        "in vec2 uv;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  frag_color = texture(tex, uv) * color;\n"
        "}\n";
    ///https://github.com/floooh/sokol-samples/blob/master/glfw/texcube-glfw.c
    g_shd_tex = sg_make_shader(sh1);

   
    /* a shader */
    sg_shader_desc sh2 = { 0 };
    sh2.vs.source =
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
    sh2.fs.source = "#version 330\n"
        "in vec4 color;\n"
        "in vec2 uv;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  frag_color = color;\n"
        "}\n";
    g_shd_base = sg_make_shader(sh2);




    /* a pipeline state object for line strips */
    {
        sg_pipeline_desc pipdesc = { 0 };
        pipdesc.shader = g_shd_base;
        pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
        pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
        pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
        pipdesc.primitive_type = SG_PRIMITIVETYPE_LINE_STRIP;
        g_base_lines_pip = sg_make_pipeline(pipdesc);
    }

    {
        sg_pipeline_desc pipdesc = { 0 };
        pipdesc.shader = g_shd_base;
        pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
        pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
        pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
        pipdesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
        g_base_triangle_strip_pip = sg_make_pipeline(pipdesc);
    }

    /* create pipeline for texture shader object */
    {
    
    sg_pipeline_desc pipdesc = { 0 };
    pipdesc.shader = g_shd_tex;
    pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
    pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    pipdesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipdesc.blend.enabled = true;
    pipdesc.blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pipdesc.blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pipdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    g_base_textured_pip = sg_make_pipeline(pipdesc);
    
    
    }
}


struct mesh_data {
    sg_pipeline pip = { 0 };
    sg_bindings bindings = { 0 };
    int num_elements = 0;
};

void draw_mesh(const mesh_data& md) {
    sg_apply_pipeline(md.pip);
    sg_apply_bindings(md.bindings);
    sg_draw(0, md.num_elements, 1);
}

void draw_sprite(entt::registry& r, const glm::mat3& mvp_mat, as::id sprite_asset_id, const glm::vec4& color) {
    sprite* spr = as::get<sprite>(sprite_asset_id);
    if (!spr) {
        return;
    }
    texture* tex = as::get<texture>(spr->texture_id);
    if (!tex) {
        return;
    }

    // Vertices
    //*1|       |3
    //* |       |
    //* |_______|
    //*0        2
    auto tex_info = sg_query_image_info(tex->image);

    // retrieve the size in world units for that sprite
    const auto rect_size = spr->src_rect.size() / spr->ppu;

    // calculate the positions for that sprite
    const auto half_rect_size = rect_size / 2.0f;
    float pos[]{
        -half_rect_size.x , -half_rect_size.y,
        -half_rect_size.x , half_rect_size.y,
        half_rect_size.x , -half_rect_size.y,
        half_rect_size.x , half_rect_size.y
    };

    // Transform positions
    for (size_t i = 0; i < 4; i++) {
        const glm::vec3 tx_pos = mvp_mat * glm::vec3(pos[i * 2], pos[(i * 2) + 1], 1.f);
        pos[i * 2] = tx_pos.x;
        pos[(i * 2) + 1] = tx_pos.y;
    }

    auto uvs_rect = map_range_clamped(rect::from_size({0,0}, {tex_info.width, tex_info.height}), rect::from_size({ 0,0 }, { 1,1 }), spr->src_rect);
    // Vertices
    //*1|       |3
    //* |       |
    //* |_______|
    //*0        2
    float vertices[] = {
        pos[0], pos[1], color.r, color.g, color.b, color.a, uvs_rect.bottom_left().x,uvs_rect.bottom_left().y,
        pos[2], pos[3], color.r, color.g, color.b, color.a, uvs_rect.top_left().x,uvs_rect.top_left().y,
        pos[4], pos[5], color.r, color.g, color.b, color.a, uvs_rect.bottom_right().x,uvs_rect.bottom_right().y,
        pos[6], pos[7], color.r, color.g, color.b, color.a, uvs_rect.top_right().x,uvs_rect.top_right().y,
    };

    sg_buffer_desc buff_desc = { 0 };
    buff_desc.size = sizeof(vertices);
    buff_desc.content = vertices;
    sg_buffer vbuf = sg_make_buffer(&buff_desc);

    mesh_data m;
    m.pip = g_base_textured_pip;
    m.num_elements = 4;
    m.bindings.vertex_buffers[0] = vbuf;
    m.bindings.fs_images[0] = tex->image;

    draw_mesh(m);
    sg_destroy_buffer(vbuf);
}

void draw_line(entt::registry& r, const glm::mat3& mvp_mat, const std::vector<glm::vec2>& points, const glm::vec4& color) {
    std::vector<float> vertices;
    
    for (const glm::vec2& p : points) {
        glm::vec3 tx_point = mvp_mat * glm::vec3(p.x,p.y,1);
        vertices.push_back(tx_point.x);
        vertices.push_back(tx_point.y);
        vertices.push_back(color.x);
        vertices.push_back(color.y);
        vertices.push_back(color.z);
        vertices.push_back(color.w);
        vertices.push_back(0); //uvs unused
        vertices.push_back(0); //uvs unused
    }
    sg_buffer_desc buff_desc = { 0 };
    buff_desc.size = sizeof(float) * (int) vertices.size();
    buff_desc.content = vertices.data();
    sg_buffer vbuf = sg_make_buffer(&buff_desc);
   

    mesh_data m;
    m.pip = g_base_lines_pip;
    m.num_elements = (int)points.size();
    m.bindings.vertex_buffers[0] = vbuf;
    draw_mesh(m);
    sg_destroy_buffer(vbuf);
}

void draw_rect(entt::registry& r, const glm::mat3& mvp_mat, const glm::vec2& size, const glm::vec4& color, bool filled) {
    if (!filled) {
        // Vertices
        //  1_______2
        //* |       |
        //* |       |
        //* |_______|
        //*0,4       3
        std::vector<glm::vec2> pos = {
            {-size.x / 2.0f, -size.y / 2.0f },
            {-size.x / 2.0f, size.y / 2.0f  },
            {size.x / 2.0f, size.y / 2.0f   },
            {size.x / 2.0f, -size.y / 2.0f  },
            {-size.x / 2.0f, -size.y / 2.0f }
        };
        draw_line(r, mvp_mat, pos, color);
        return;
    }

    // Vertices
    //*1|       |3
    //* |       |
    //* |_______|
    //*0        2
    float pos[] = {
        -size.x / 2.0f, -size.y / 2.0f,
        -size.x / 2.0f, size.y / 2.0f,
        size.x / 2.0f, -size.y / 2.0f,
        size.x / 2.0f, size.y / 2.0f
    };

    // Transform positions
    for (size_t i = 0; i < 4; i++) {
        const glm::vec3 tx_pos = mvp_mat * glm::vec3(pos[i*2], pos[(i * 2) + 1], 1.f);
        pos[i * 2] = tx_pos.x;
        pos[(i * 2) + 1] = tx_pos.y;
    }

    float vertices[] = {
        pos[0], pos[1], color.r, color.g, color.b, color.a, 0,0,
        pos[2], pos[3], color.r, color.g, color.b, color.a, 0,0,
        pos[4], pos[5], color.r, color.g, color.b, color.a, 0,0,
        pos[6], pos[7], color.r, color.g, color.b, color.a, 0,0
    };
    sg_buffer_desc buff_desc = { 0 };
    buff_desc.size = sizeof(vertices);
    buff_desc.content = vertices;
    sg_buffer vbuf = sg_make_buffer(&buff_desc);

    mesh_data m;
    m.pip = g_base_triangle_strip_pip;
    m.num_elements = 4;
    m.bindings.vertex_buffers[0] = vbuf;
  
    draw_mesh(m);
    sg_destroy_buffer(vbuf);
}


void draw_entities(entt::registry& r, const glm::mat3& vp_mat) {

    {
        auto lines = r.view<cp::transform, cp::line_rd>();
        for (auto entity : lines) {
            auto& ltr = lines.get<cp::transform>(entity);
            auto& lr = lines.get<cp::line_rd>(entity);
            draw_line(r, vp_mat * ltr.ltw, lr.points, lr.color);
        }
    }

    {
        auto rects = r.view<cp::transform, cp::rect_rd>();
        for (auto entity : rects) {
            auto& ltr = rects.get<cp::transform>(entity);
            auto& rr = rects.get<cp::rect_rd>(entity);
            draw_rect(r, vp_mat * ltr.ltw, rr.size, rr.color, rr.filled);
        }
    }

    {
        auto sprs = r.view<cp::transform, cp::sprite_rd>();
        for (auto entity : sprs) {
            auto& ltr = sprs.get<cp::transform>(entity);
            auto& spr = sprs.get<cp::sprite_rd>(entity);
            draw_sprite(r, vp_mat * ltr.ltw, spr.sprite_id, spr.color);
        }
    }
}




void draw_camera(entt::registry& r, glm::ivec2 vp_size, const cp::camera& cam, const cp::transform& tr_cam) {
    sg_apply_viewport((int)(cam.viewport.x * vp_size.x), (int)(cam.viewport.y * vp_size.y),
        (int)((cam.viewport.z * vp_size.x)-(cam.viewport.x * vp_size.x)), (int)((cam.viewport.w * vp_size.y)- (cam.viewport.y * vp_size.y)), false);
    sg_apply_scissor_rect((int)(cam.viewport.x * vp_size.x), (int)(cam.viewport.y * vp_size.y),
        (int)((cam.viewport.z * vp_size.x) - (cam.viewport.x * vp_size.x)), (int)((cam.viewport.w * vp_size.y) - (cam.viewport.y * vp_size.y)), false);

    glClearColor(cam.clear_color.r, cam.clear_color.g, cam.clear_color.b, cam.clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up projection
    glm::mat3 projection_mat(glm::ortho(-cam.aspect * cam.half_vsize, cam.aspect * cam.half_vsize, -cam.half_vsize, cam.half_vsize ));

    // Setup view matrix from inverse camera
    glm::mat3 view_mat = glm::inverse(tr_cam.ltw);

    draw_entities(r, projection_mat * view_mat);
}



void rd::draw_all(entt::registry& r) {
    glm::ivec2 ws; 
    SDL_GetWindowSize(ap_sdl_app_window(), &ws.x, &ws.y);
    /* default pass action (clear to grey) */
    sg_pass_action pass_action = { 0 };
    sg_begin_default_pass(&pass_action, ws.x, ws.y);

    auto view_cameras = r.view<cp::camera, cp::transform>();
    for (auto entity : view_cameras) {
        auto& tr = view_cameras.get<cp::transform>(entity);
        auto& cam = view_cameras.get<cp::camera>(entity);
        draw_camera(r, ws, cam, tr);
    }

    sg_end_pass();
    sg_commit();
}


void rd::shutdown() {
    /* cleanup */
    sg_shutdown();
}

sg_image rd::create_image(const char* file_name) {
    assert(file_name);
    cp_image_t img = cp_load_png(file_name);
    if (!img.pix) {
        AP_WARNING("Error loading texture file : %s : %s", file_name, cp_error_reason);
        return { SG_INVALID_ID };
    }

    cp_flip_image_horizontal(&img);

    sg_image_desc d = { 
        .width = img.w,
        .height = img.h,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST
    };
    d.content.subimage[0][0].ptr = img.pix;
    d.content.subimage[0][0].size = sizeof(cp_pixel_t) * img.w * img.h;

    sg_image tex = sg_make_image(d);
    cp_free_png(&img);
    return tex;
}

/*Magnum::Matrix3 projectionMatrixToSDLSpace(SDL_Window* window, const Vector2& cameraSize) {
	const Vector2 winSizeHalf = Vector2(framebufferSize(window)) / 2.f;
	return Matrix3::translation(winSizeHalf) * Matrix3::scaling({ winSizeHalf.x(), -winSizeHalf.y() }) * Magnum::Matrix3::projection(cameraSize);
}*/


//void ds_rd_draw_line(vec2 p0, vec2 p1) {
//	// TODO This is to make the camera
//	//SDL_Rect r = { .x = 0 };
//	//r.y = 0;
//	//r.w = 300;
//	//r.h = 275;
//	//SDL_RenderSetViewport(SDL_GetRenderer(ap_sdl_app_window()), &r);
//
//
//
//	// compute model matrix
//	//const auto modelMatrix = (Matrix3::translation(tr.position) * Matrix3::rotation(Magnum::Deg(tr.rotation))) * Matrix3::scaling(tr.scale);
//	mat3 model = GLM_MAT3_IDENTITY_INIT;
//
//
//	mat3 camera = GLM_MAT3_IDENTITY_INIT;
//	glm_mat3_inv(camera, camera);
//
//
//	// Projection matrix: (Without SDL conversion) (just only 2/camera_size)
//	int ws_x;
//	int ws_y;
//	SDL_GetWindowSize(ap_sdl_app_window(), &ws_x, &ws_y);
//	vec2 window_size = { (float)ws_x, (float)ws_y };
//
//	mat3 projection = GLM_MAT3_IDENTITY_INIT;
//	projection[0][0] = 2 / window_size[0];
//	projection[1][1] = 2 / window_size[1];
//
//
//	// Conversion TO SDL space (0,0 in the middle and x+ right, y+ up)
//	//vec2 v= Vector2(framebufferSize(window)) / 2.f;
//	//return Matrix3::translation(winSizeHalf) * Matrix3::scaling({ winSizeHalf.x(), -winSizeHalf.y() }) * Magnum::Matrix3::projection(cameraSize)
//	mat3 proj_sdl_space = GLM_MAT3_IDENTITY_INIT;
//	vec2 window_size_div2;
//	glm_vec2_divs(window_size, 2, window_size_div2);
//
//	glm_translate2d_make(proj_sdl_space, window_size_div2);
//	glm_scale2d(proj_sdl_space, (vec2) { window_size_div2[0], -window_size_div2[1] });
//	
//
//	// MVP 
//	//glm::mat4 MVPmatrix = proj_sdl * projection * view * model; // Remember : inverted !
//	//mat3 mvp =  ((proj_sdl * projection ) * camera) * model;
//	mat3 mvp = GLM_MAT3_IDENTITY_INIT;
//	glm_mat3_mul(proj_sdl_space, projection, mvp);
//	glm_mat3_mul(mvp, camera, mvp);
//	glm_mat3_mul(mvp, model, mvp);
//
//
//
//	// Transform vertices
//	/*
//	transformed_vertex = MVP * in_vertex;
//	*/
//
//	vec3 point0 = { p0[0], p0[1], 1 }; // z = 1 means position, 0 direction
//	vec3 point1 = { p1[0], p1[1], 1 }; // z = 1 means position, 0 direction
//	glm_mat3_mulv(mvp, point0, point0);
//	glm_mat3_mulv(mvp, point1, point1);
//
//	//SDL_SetRenderDrawColor(SDL_GetRenderer(ap_sdl_app_window()), 255, 0, 0, 255);
//	//SDL_RenderDrawLineF(SDL_GetRenderer(ap_sdl_app_window()), point0[0], point0[1], point1[0], point1[1]);
//		
//}
//
//SDL_Rect ds_make_rect(int origx, int origy, int size_x, int size_y) {
//	SDL_Rect r = { origx, origy, size_x, size_y };
//	return r;
//}
//
//SDL_FRect ds_make_frect(vec2 origin, vec2 size) {
//	SDL_FRect r = { origin[0], origin[1], size[0], size[1]};
//	return r;
//}
//
//// calculates the cetner of the rect
//void ds_rect_center(SDL_FRect r, vec2 center) {
//	center[0] = r.x + (r.w / 2.0f);
//	center[1] = r.y + (r.h / 2.0f);
//}

//void ds_sdl_rect_tl(SDL_FRect r, vec2 tl) {
//	vec2 center;
//	ds_rect_center(r, center);
//
//	tl[0] = center[0] + ()
//}

//
//
//void ds_rd_draw_sprite(SDL_Texture *texture) {
//	vec2 position = {0,0};
//	float rot_deg = 180;
//	vec2 scale = { 1,1 };
//
//	// compute model matrix
//	//const auto modelMatrix = (Matrix3::translation(tr.position) * Matrix3::rotation(Magnum::Deg(tr.rotation))) * Matrix3::scaling(tr.scale);
//	mat3 model;
//	glm_translate2d_make(model, position);
//	// Here we don't apply the rotation!!! Because it's something SDL will do internally
//	//glm_scale2d(model, scale);
//
//	// Camera matrix
//	mat3 camera = GLM_MAT3_IDENTITY_INIT;
//	glm_mat3_inv(camera, camera);
//
//	// Projection matrix: (Without SDL conversion) (just only 2/camera_size)
//	int ws_x, ws_y; SDL_GetWindowSize(ap_sdl_app_window(), &ws_x, &ws_y);
//	vec2 window_size = { (float)ws_x, (float)ws_y };
//	mat3 projection = GLM_MAT3_IDENTITY_INIT;	projection[0][0] = 2 / window_size[0];	projection[1][1] = 2 / window_size[1];
//
//
//	// Conversion TO SDL space (0,0 in the middle and x+ right, y+ up)
//	//vec2 v= Vector2(framebufferSize(window)) / 2.f;
//	//return Matrix3::translation(winSizeHalf) * Matrix3::scaling({ winSizeHalf.x(), -winSizeHalf.y() }) * Magnum::Matrix3::projection(cameraSize)
//	mat3 proj_sdl_space = GLM_MAT3_IDENTITY_INIT;
//	vec2 window_size_div2;
//	glm_vec2_divs(window_size, 2, window_size_div2);
//	glm_translate2d_make(proj_sdl_space, window_size_div2);
//	glm_scale2d(proj_sdl_space, (vec2) { window_size_div2[0], -window_size_div2[1] });
//
//
//	// MVP 
//	//glm::mat4 MVPmatrix = proj_sdl * projection * view * model; // Remember : inverted !
//	//mat3 mvp =  ((proj_sdl * projection ) * camera) * model;
//	mat3 mvp = GLM_MAT3_IDENTITY_INIT;
//	glm_mat3_mul(proj_sdl_space, projection, mvp);
//	glm_mat3_mul(mvp, camera, mvp);
//	glm_mat3_mul(mvp, model, mvp);
//
//
//
//	// Transform vertices
//	/*
//	transformed_vertex = MVP * in_vertex;
//	*/
//
//	//vec3 point0 = { p0[0], p0[1], 1 }; // z = 1 means position, 0 direction
//	//vec3 point1 = { p1[0], p1[1], 1 }; // z = 1 means position, 0 direction
//	//glm_mat3_mulv(mvp, point0, point0);
//	//glm_mat3_mulv(mvp, point1, point1);
//
//
//
//
//
//	struct ds_sprite {
//		/** Location of the Sprite in the original Texture, specified in pixels.
//		 * 	{0,0}	----	{x+,0}
//		 *	|				|
//		 *	{0,y+}	----	{x+,y+}
//		 */
//		SDL_Rect tex_src;
//
//		/* pivot point. {0.5, 0.5} centers the sprite.
//		{0,1} ---- {1,1}
//		|			|
//		{0,0} ---- {1,0}
//		*/
//		vec2 pivot;
//
//	};
//
//	struct ds_sprite spr = { 0 };
//	SDL_Point texture_size;
//	SDL_QueryTexture(texture, NULL, NULL, &texture_size.x, &texture_size.y);
//	spr.tex_src = ds_make_rect(0, 0, texture_size.x, texture_size.y);
//	spr.tex_src = ds_make_rect(0, 0, 100, 100);
//	
//	vec3 sdl_space_center = { 0,0,1 };
//	glm_mat3_mulv(mvp, sdl_space_center, sdl_space_center);
//
//
//	// center of the sprite
//	vec3 top_left_vertice = { -spr.tex_src.w / 2.0f, spr.tex_src.h / 2.0f, 1.0f};
//	//vec3 bottom_right_vertice = { spr.tex_src.w / 2.0f, spr.tex_src.h / 2.0f, 1.0f };
//
//	glm_mat3_mulv(mvp, top_left_vertice, top_left_vertice);
//	SDL_FRect dst_rect = { .x = top_left_vertice[0], .y = top_left_vertice[1], .w = (float)spr.tex_src.w * scale[0], .h = (float)spr.tex_src.h * scale[1] };
//
//
//
//
//	//
//	mat3 inverse_sdl_mat = GLM_MAT3_IDENTITY_INIT;
//	glm_mat3_inv(proj_sdl_space,inverse_sdl_mat);
//
//	vec3 sprite_center = { 0,0,1 };
//	ds_rect_center(dst_rect, sprite_center);
//	glm_mat3_mulv(inverse_sdl_mat, sprite_center, sprite_center);
//	SDL_FPoint center = { -sprite_center[0] , sprite_center[1] };
//
//	if (1) {
//	/*	if (SDL_RenderCopyExF(SDL_GetRenderer(ap_sdl_app_window()),
//			texture, &spr.tex_src, &dst_rect, rot_deg, &center, SDL_FLIP_NONE) != 0) {
//			AP_TRACE("Hola");
//		}*/
//	}
//
//	//SDL_SetRenderDrawColor(SDL_GetRenderer(ap_sdl_app_window()), 255, 0, 255, 255);
//	//SDL_RenderDrawPointF(SDL_GetRenderer(ap_sdl_app_window()), center.x, center.y);
//}




//void ds_rd_draw() {
//	int ws_x, ws_y; SDL_GetWindowSize(ap_sdl_app_window(), &ws_x, &ws_y);
//	//vec2 window_size = { (float)ws_x, (float)ws_y };
//
//	
//	/*Magnum::Range2Di cameraViewport{ Vec2i{Vec2{{fbSize.x() * cam.viewport.x(), fbSize.y() * cam.viewport.y()}}},
//		Vec2i{Vec2{fbSize.x() * cam.viewport.z(),fbSize.y() * cam.viewport.w()}} };*/
//	//std::cout << cameraViewport.bottomLeft().x() << " " << cameraViewport.bottomLeft().y() << " " << cameraViewport.sizeX() << " " << cameraViewport.sizeY() << std::endl;
//	//glEnable(GL_DEBUG_OUTPUT);
//	glEnable(GL_SCISSOR_TEST);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	// Setup viewport and scissor for this camera
//	glViewport(0, 0, ws_x, ws_y);
//	glScissor(0, 0, ws_x, ws_x);
//
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(-ws_x / 2, ws_x / 2, -ws_y / 2, ws_y / 2,0, 1);
//	//glOrtho(-1, 1, -1, 1, -1.0f, 1.0f);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	glClearColor(0.2f, 0.2f, 0.2f, 0.f);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	ap_gl_draw_rect(0, 0, 300, 200, (vec4) { 0, 1, 1, 1 });
//	ap_gl_draw_line((float[8]){0, 0, 100,100, 0,150, 123,-120}, 4, (vec4){ 1, 0, 1, 1 });
//
//	ap_gl_draw_sprite(1, (vec4) { 1, 1, 1, 1 });
//	GLenum err;
//	while ((err = glGetError()) != GL_NO_ERROR) {
//		AP_WARNING("OpenGL error: %d", err);
//	}
//
//	//// Set up projection
//	//// compute camera world units size and set the projection
//	//const Vec2 cameraSize{ cam.aspectRatio * cam.halfVSize * 2.0f, 2.0f * cam.halfVSize };
//	//const auto projMatrix = Magnum::Matrix4::orthographicProjection(cameraSize, 1, -1);
//	//glMatrixMode(GL_PROJECTION);
//	//glLoadMatrixf(projMatrix.data());
//
//	//////// Computes View Matrix and pushes it to modelview matrix
//	//glMatrixMode(GL_MODELVIEW);
//	//const auto viewMatrix = tr.localToWorld;
//	//glLoadMatrixf(Magnum::Matrix4(viewMatrix).inverted().data());
//
//	//drawEntitiesGL(r, window);
//
//}
//
