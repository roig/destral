#ifndef AP_OPENGL_H
#define AP_OPENGL_H

#ifdef AP_OPENGL_IMPL
#define AP_GLLOADER_IMPL
#endif
#include "ap_gl33compat.h"

/*
    Astral Pixel Opengl utilities header

*/

/** Draw filled rectangle */
void ap_gl_draw_rect(float x, float y, float w, float h, float color[4]);

/** Draw filled rectangle */
void ap_gl_draw_rect_lines(float x, float y, float w, float h, float color[4]);

/** 
	Draw a line given an array of points.
	points is an array of floats where each point is two floats:

	Example a line with three points: p0, p1 and p2
	points => {p0x, p0y, p1x, p1y , p2x, p2y.. }.
	points_count is the number of points (two floats) in that array (NOT THE NUMBER OF FLOATS)
	For example {p0x, p0y, p1x, p1y , p2x, p2y }  will have points_count = 3

*/
void ap_gl_draw_line(float *points , size_t points_count, float color[4]);

/** Draw filled rectangle */
void ap_gl_draw_rect_lines(float x, float y, float w, float h, float color[4]);




//void DrawTexture(Texture2D texture, int posX, int posY, Color tint);                               // Draw a Texture2D
//void DrawTextureV(Texture2D texture, Vector2 position, Color tint);                                // Draw a Texture2D with position defined as Vector2
//void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);  // Draw a Texture2D with extended parameters
//void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint);         // Draw a part of a texture defined by a rectangle
//void DrawTextureQuad(Texture2D texture, Vector2 tiling, Vector2 offset, Rectangle quad, Color tint);  // Draw texture quad with tiling and offset parameters
//void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint);       // Draw a part of a texture defined by a rectangle with 'pro' parameters
//void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle destRec, Vector2 origin, float rotation, Color tint);  // Draws a texture (or part of it) that stretches or shrinks nicely
//






/*
	GL TEXTURES
*/

/** 
	Loads a png file and creates an opengl texture for it and returns the GL texture identifier.
	WARNING: This only works for png files.
	On error returns GL_INVALID_INDEX. 
*/
GLuint ap_gl_texture_load(const char* file_name);

/** Retrieves the size of the a texture, result is not cached (always opengl is called for info) */
void ap_gl_texture_size(GLuint tex_gl_id, int* w, int* h);


typedef enum ap_gl_vertex_fmt {
	AP_GL_VRTX_FMT_P2U2C4 = 0,
	AP_GL_VRTX_FMT_P2C4,
} ap_gl_vertex_fmt;

typedef struct ap_gl_mesh_data {
	float* positions; // 2 floats per vertex
	float* uvs; // 2 floats per vertex
	float* colors; // 4 floats per vertex
	size_t vertices_count; // vertex count
	ap_gl_vertex_fmt vrtx_fmt;
	GLenum topology; // the topology of the mesh GL_TRIANGLES GL_POINTS... etc..
} ap_gl_mesh_data;

typedef struct ap_gl_material {
	GLuint tex_id;  // GL_INVALID_INDEX no texture set
} ap_gl_material;

void ap_gl_draw_mesh(ap_gl_mesh_data* mesh, ap_gl_material* mat);

struct ap_gl_sprite_mesh {
	float positions[8]; // 2 floats per vertex
	float uvs[8]; // 2 floats per vertex
	float color[16]; // 4 floats per vertex
	GLuint texture_id; // The gl texture id, set to 0 if no texture to apply
};

void ap_gl_draw_sprite(GLuint tex, float tint_color[4]);

#ifdef AP_OPENGL_IMPL

void ap_gl_draw_mesh(ap_gl_mesh_data* mesh, ap_gl_material* mat) {
	assert(mesh);

	// set up material
	if (mat) {
		if (mat->tex_id != 0) {
			// textured mode
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, mat->tex_id);
		} else {
			glDisable(GL_TEXTURE_2D);
		}
	}

	switch (mesh->vrtx_fmt) {

	case AP_GL_VRTX_FMT_P2U2C4: {
		glBegin(mesh->topology);
		for (size_t v_idx = 0; v_idx < mesh->vertices_count; v_idx++) {
			glColor4fv(&mesh->colors[v_idx * 4]);
			glTexCoord2fv(&mesh->uvs[v_idx * 2]);
			glVertex2fv(&mesh->positions[v_idx * 2]);
		}
		glEnd();
	}
		break;

	case AP_GL_VRTX_FMT_P2C4: {
		glBegin(mesh->topology);
		for (size_t v_idx = 0; v_idx < mesh->vertices_count; v_idx++) {
			glColor4fv(&mesh->colors[v_idx * 4]);
			glVertex2fv(&mesh->positions[v_idx * 2]);
		}
		glEnd();
	}
		break;

	}
}

void ap_gl_draw_sprite(GLuint tex, float tint_color[4]) {
	float tex_size[2] = { 480, 320};
	//float position[] = { 0,0 };

	/* vertices positions
	v1 ---- v3      uv01 ---- uv11      
	|	p	|		|		   |
	v0 ---- v2		uv00 ---- uv10
	*/
	float positions[] = { 0,0,  0,tex_size[1],   tex_size[0],0,    tex_size[0],tex_size[1] };
	float uvs[] = {0,0,  0,1,  1,0, 1,1 };
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_TRIANGLE_STRIP);
	glColor4fv(tint_color);
	for (size_t v_idx = 0; v_idx < 8; v_idx = v_idx +2) {
		glTexCoord2fv(&uvs[v_idx]);
		glVertex2fv(&positions[v_idx]);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	ap_gl_draw_line(positions, 4, (float[]) { 1, 0, 0, 1 });
}

//
//void ap_gl_draw_mesh(ap_gl_mesh_data* m) {

//}




void ap_gl_draw_line(float* points, size_t points_count, float color[4]) {
	ap_gl_material mat;
	mat.tex_id = GL_INVALID_INDEX;
	
	ap_gl_mesh_data mesh = { 0 };
	mesh.vertices_count = points_count;
	mesh.positions = points;
	mesh.topology = GL_LINE_STRIP;
	mesh.vrtx_fmt = AP_GL_VRTX_FMT_P2C4;
	mesh.colors = malloc(sizeof(float) * points_count * 4);
	for (size_t i = 0; i < points_count; i++) {
		memcpy(&mesh.colors[i * 4], color, sizeof(float) * 4);
	}
	ap_gl_draw_mesh(&mesh,&mat);
	free(mesh.colors);
}

void ap_gl_draw_rect(float x, float y, float w, float h, float color[4]) {
	/** 
	 * 	1{x,y}	----	3{x+w,y}
	 *	|				|
	 *	0{x,y-h}	----2{x+w,y-h}
	 */
	
	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(color[0], color[1], color[2], color[3]);
	glVertex2f(x,y-h);
	glVertex2f(x,y);
	glVertex2f(x+w,y-h);
	glVertex2f(x+w,y);
	glEnd();
}

void ap_gl_draw_rect_lines(float x, float y, float w, float h, float color[4]) {
	/**
	 * 	{x,y}	----	{x+w,y}
	 *	|				|
	 *	{x,y-h}	----	{x+w,y-h}
	 */
	ap_gl_draw_line((float[10]) { x,y-h, x,y, x+w,y-h, x+w,y,  x,y-h }, 10, color);
}







void ap_gl_texture_size(GLuint tex_gl_id, int* w, int* h) {
	GLint miplevel = 0;
	glBindTexture(GL_TEXTURE_2D, tex_gl_id);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, h);
}

//// CUTE TEXTURE LOADING CODE
#pragma warning( push )
#pragma warning(disable : 4996) // disable fopen warning in cute_png..
#define CUTE_PNG_IMPLEMENTATION
#include "cute_png.h"
#pragma warning( pop )



GLuint ap_gl_texture_load(const char* file_name) {
	assert(file_name);
	cp_image_t img = cp_load_png(file_name);
	if (!img.pix) {
		printf("Error loading texture file: %s  :  %s", file_name, cp_error_reason);
		return GL_INVALID_INDEX;
	}

	cp_flip_image_horizontal(&img);
	

	GLuint tex_gl_id;
	glGenTextures(1, &tex_gl_id);
	glBindTexture(GL_TEXTURE_2D, tex_gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// upload data to texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.w, img.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.pix);

	cp_free_png(&img);
	return tex_gl_id;
}



#endif //AP_OPENGL_IMPL
#endif // AP_OPENGL_H




