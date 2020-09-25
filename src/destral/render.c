#include "destral/render.h"
#include <cglm/affine2d.h>

#include <ap_debug.h>
#include <ap_sdl.h>
#include <sokol_gfx.h>

void ds_rd_init_sokol();
void ds_rd_init() {
	sg_desc d = { 0 };
	sg_setup(&d);
    ds_rd_init_sokol();

}



sg_pipeline g_base_pip = { 0 };
//sg_bindings g_binds = { 0 };
sg_shader g_shd_base = { 0 };
sg_shader g_shd_tex = { 0 };


void ds_rd_init_sokol() {



    ///https://github.com/floooh/sokol-samples/blob/master/glfw/texcube-glfw.c
    g_shd_tex = sg_make_shader(&(sg_shader_desc) {
        .vs.source =
        "#version 330\n"
        "layout(location=0) in vec2 position;\n"
        "layout(location=1) in vec4 color0;\n"
        "layout(location = 2) in vec2 texcoord0;\n"
        "out vec4 color;\n"
        "out vec2 uv;"
        "void main() {\n"
        "  gl_Position = vec4(position,0,1);\n"
        "  color = color0;\n"
        "  uv = texcoord0 * 5.0;\n"
        "}\n",
        .fs.source =
        "#version 330\n"
        "uniform sampler2D tex;"
        "in vec4 color;\n"
        "in vec2 uv;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  frag_color = texture(tex, uv) * color;\n"
        "}\n"
    });


    /* a shader */
    g_shd_base = sg_make_shader(&(sg_shader_desc) {
        .vs.source =
        "#version 330\n"
        "layout(location=0) in vec2 position;\n"
        "layout(location=1) in vec4 color0;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  gl_Position = vec4(position,0,1);\n"
        "  color = color0;\n"
        "}\n",
        .fs.source =
        "#version 330\n"
        "in vec4 color;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "  frag_color = color;\n"
        "}\n"
    });


    /* a pipeline state object (default render states are fine for triangle) */
    g_base_pip = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = g_shd_base,
            .layout = {
                .attrs = {
                    [0].format = SG_VERTEXFORMAT_FLOAT2,
                    [1].format = SG_VERTEXFORMAT_FLOAT4
                }
        }
    });

    /* create pipeline for texture shader object */
    //g_tex_pip = sg_make_pipeline(&(sg_pipeline_desc) {
    //    .layout = {
    //        .attrs = {
    //            [0] = {.format = SG_VERTEXFORMAT_FLOAT3 },
    //            [1] = {.format = SG_VERTEXFORMAT_FLOAT4 },
    //            [2] = {.format = SG_VERTEXFORMAT_FLOAT2 }
    //        }
    //    },
    //        .shader = g_shd_tex/*,
    //       .depth_stencil = {
    //            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
    //            .depth_write_enabled = true
    //    },
    //        .rasterizer.cull_mode = SG_CULLMODE_BACK*/
    //});

}


void ds_rd_draw_all() {


    /* default pass action (clear to grey) */
    sg_pass_action pass_action = { 0 };

    /* draw loop */

    int ws_x, ws_y; SDL_GetWindowSize(ap_sdl_app_window(), &ws_x, &ws_y);
    //vec2 window_size = { (float)ws_x, (float)ws_y };
    sg_begin_default_pass(&pass_action, ws_x, ws_y);



    /* a vertex buffer */
    const float vertices[] = {
        // positions            // colors
         0.0f,  0.5f,      1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,      0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,      0.0f, 0.0f, 1.0f, 1.0f
    };

    sg_buffer_desc buff_desc = { 0 };
    buff_desc.size = sizeof(vertices);
    buff_desc.content = vertices;

    sg_buffer vbuf = sg_make_buffer(&buff_desc);





    sg_apply_pipeline(g_base_pip);



    /* resource bindings */
    sg_bindings g_binds = { 0 };
    g_binds.vertex_buffers[0] = vbuf;

    sg_apply_bindings(&g_binds);
    sg_draw(0, 3, 1);



    sg_destroy_buffer(vbuf);







    sg_end_pass();
    sg_commit();
       


}


void ds_rd_shutdown() {
    /* cleanup */
    sg_shutdown();

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
