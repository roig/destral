#include "destral/renderer.h"
#include "destral/transform.h"
#include <entt/entity/registry.hpp>


#include "ap_gl33.h"
#define SOKOL_NO_DEPRECATED
#define SOKOL_GLCORE33
#include "sokol_gfx.h"


namespace ds::cp {
    // internal components

    /**
    * @brief Just a flag to hold debug render parameters
    */
    struct DebugRenderer {
        // Holds the remaining time this entity should be alive
        float remainingTime = 0.0f;
    };

}


namespace ds::render {

     
    sg_shader gShaderStdNonTextured;

    // Creates a pipeline for the shader
    sg_pipeline getStandardShaderPipelineDesc() {
        // This pipeline should be the same for all shaders
        /*
            "#version 330\n"
            "layout(location=0) in vec2 position;\n"
            "layout(location=1) in vec2 uv;\n"
            "layout(location=2) in vec4 color0;\n"
        */

        sg_pipeline_desc pipdesc = { 0 };
        pipdesc.index_type = SG_INDEXTYPE_NONE;
        pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;    //[ATTR_vs_position]
        pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;    //[ATTR_vs_uv]
        pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT4;    //[ATTR_vs_color0]
        return sg_make_pipeline(&pipdesc);
    }

    void init() {
        gladLoadGL();

        sg_desc desc = {0};
        sg_setup(&desc);
        /* a shader (use separate shader sources here */
        sg_shader_desc shdesc = {0};
        shdesc.vs.source =
            "#version 330\n"
            "layout(location=0) in vec2 position;\n"
            "layout(location=1) in vec2 uv;\n"
            "layout(location=2) in vec4 color0;\n"
            "out vec4 color;\n"
            "out vec2 out_uv;\n"
            "void main() {\n"
            "  gl_Position = vec4(position,0,1);\n"
            "  color = color0;\n"
            "  out_uv = uv;\n"
            "}\n";
        shdesc.fs.source =
            "#version 330\n"
            "in vec4 color;\n"
            "in vec2 uv;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n";
        gShaderStdNonTextured = sg_make_shader(&shdesc);
    }

    void cleanup() {
        sg_shutdown();
    }


    //struct MeshData {
    //    GLenum primitive = GL_TRIANGLES;

    //    // layout will be: 
    //    //position{ x, y }, uv = {u, v}, color = {r, g, b, a} // 8 floats
    //    std::vector<float> vertices;
    //    void addVertice(const Vec2& position, const Vec2& uv, const Vec4& color) {
    //        vertices.push_back(position.x());
    //        vertices.push_back(position.y());
    //        vertices.push_back(uv.x());
    //        vertices.push_back(uv.y());
    //        vertices.push_back(color.r());
    //        vertices.push_back(color.g());
    //        vertices.push_back(color.b());
    //        vertices.push_back(color.a());
    //    }
    //};


    struct MeshData {
        std::vector<Vec2> positions;
        std::vector<Vec2> uvs;
        std::vector<Vec4> colors;

        enum class Topology {
            TRIANGLES = 0,
            LINES,
            LINESTRIP
        };
        Topology topology = Topology::TRIANGLES;
    };

    //https://docs.unity3d.com/Manual/Shaders.html
    // Zlayer -> Material(shader) -> MeshData
    std::unordered_map<int, std::unordered_map<uint32_t, std::vector<std::pair<MeshData, Mat3> > > > gMeshesToDraw;
    void drawMesh(const MeshData& mesh, const Mat3& ltw, int layerZ, sg_shader shader) {
        gMeshesToDraw[layerZ][shader.id].push_back({ mesh, ltw });
    }


    MeshData createLineMeshData(const cp::Transform& model_tr, const cp::LineRenderer& line) {
        MeshData md;
        md.topology = MeshData::Topology::LINESTRIP;
        md.positions = line.points;
        if (line.is_looped && md.positions.size()) {
            md.positions.push_back(line.points.at(0));
        }
        md.colors.insert(md.colors.begin(), line.points.size(), line.color);
        md.uvs.insert(md.uvs.begin(), line.points.size(), {0,0});
        return md;
    }

   

    void drawEntitiesFromCamera(entt::registry& r, const cp::Transform& cam_tr, const cp::Camera& cam) {
       
        

       

        
        std::unordered_map<int, std::vector<MeshData> > meshesPerDepth;
        MeshData md;
        auto viewLines = r.view<cp::Transform, cp::LineRenderer>();
        for (auto entity : viewLines) {
            auto& tr = viewLines.get<cp::Transform>(entity);
            auto& line = viewLines.get<cp::LineRenderer>(entity);
            meshesPerDepth[0].push_back(createLineMeshData(tr, line));
        }


        // now create buffers
        
        
        for (auto meshes_it : meshesPerDepth) {
            std::vector<float> vertices_buff_data;

            auto depth = meshes_it.first;
            auto& meshes = meshes_it.second;
            for (auto& mesh : meshes) {
                //vertices_buff_data.insert(vertices_buff_data.end(), mesh.vertices.begin(), mesh.vertices.end());
            }

            //sg_buffer_desc buffdesc = { 0 };
            //buffdesc.size = vertices_buff_data.size();
            //buffdesc.content = vertices_buff_data.data();
            //buffdesc.label = "all-vertices";
            //sg_bindings bind = { 0 };
            //bind.vertex_buffers[0] = sg_make_buffer(&buffdesc);

            ///* an index buffer with 2 triangles */
            //uint32_t indices[] = { 0, 1, 2,   0, 2, 3, 1 };
            //sg_buffer_desc idxbuffdesc = { 0 };
            //idxbuffdesc.type = SG_BUFFERTYPE_INDEXBUFFER;
            //idxbuffdesc.size = sizeof(indices);
            //idxbuffdesc.content = indices;
            //idxbuffdesc.label = "quad-indices";
            //gl_state.bind.index_buffer = sg_make_buffer(&idxbuffdesc);

            //sg_apply_pipeline(gContext.basePipeline);
            //sg_apply_bindings(bind);
            //sg_draw(0, 6, 1);


            //sg_destroy_buffer(bind.vertex_buffers[0]);
            //sg_destroy_buffer(bind.index_buffer);
        }

        /* a vertex buffer */
        float vertices[] = {
            // positions    // uv     // colors
            -0.5f,  0.5f,   0, 0,     1.0f, 0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,   0, 0,     0.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f,   0, 0,     0.0f, 0.0f, 1.0f, 1.0f,

            -0.5f,  0.5f,   0, 0,     1.0f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f,   0, 0,     0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f,   0, 0,     1.0f, 1.0f, 0.0f, 1.0f,

        };

        sg_buffer_desc buffdesc = { 0 };
        buffdesc.size = sizeof(vertices);
        buffdesc.content = vertices;
        buffdesc.label = "quad-vertices";
       

        /* an index buffer with 2 triangles */
        uint32_t indices[] = { 0, 1, 2,   0, 2, 3 };
        sg_buffer_desc idxbuffdesc = { 0 };
        //idxbuffdesc.type = SG_BUFFERTYPE_INDEXBUFFER;
        idxbuffdesc.size = sizeof(indices);
        idxbuffdesc.content = indices;
        idxbuffdesc.label = "quad-indices";
       


        sg_pipeline_desc pipdesc = { 0 };
        pipdesc.shader = gShaderStdNonTextured;
        pipdesc.index_type = SG_INDEXTYPE_NONE;
        pipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;    //[ATTR_vs_position]
        pipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;    //[ATTR_vs_uv]
        pipdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT4;    //[ATTR_vs_color0]
        pipdesc.label = "SHIT";
        pipdesc.primitive_type = sg_primitive_type::SG_PRIMITIVETYPE_TRIANGLES;
        sg_pipeline pip = sg_make_pipeline(pipdesc);
        sg_apply_pipeline(pip);

        sg_bindings bind = { 0 };
        bind.vertex_buffers[0] = sg_make_buffer(&buffdesc);
        //bind.index_buffer = sg_make_buffer(&idxbuffdesc);
        
        sg_apply_bindings(bind);
        sg_draw(0, 6, 1);

        
        sg_destroy_buffer(bind.vertex_buffers[0]);
        sg_destroy_buffer(bind.index_buffer);
        sg_destroy_pipeline(pip);


        for (auto layer_map : gMeshesToDraw) {
            for (auto shd_map : layer_map.second) {
                for (auto mesh_pair : shd_map.second) {
                   // MeshData& md = mesh_pair.first;
                    //Mat3& md = mesh_pair.second;


                }
            }
        }
    }

    void drawCamera(entt::registry& r, SDL_Window* window, const cp::Transform& tr, const cp::Camera& cam) {
        Vec2i vp_size;
        SDL_GL_GetDrawableSize(window, &vp_size.x(), &vp_size.y());
        
   
        Magnum::Range2Di cam_vp{
            Vec2i{ vp_size * Vec2{ cam.viewport.x(), cam.viewport.y() } },
            Vec2i{ vp_size * Vec2{ cam.viewport.z(), cam.viewport.w() } }
        };

        // Pass configuration
        sg_pass_action pass_action;
        pass_action = { 0 };
        pass_action.colors[0].action = sg_action::SG_ACTION_CLEAR;
        std::memcpy(pass_action.colors[0].val, cam.clear_color.data(), sizeof(float) * 4);
        sg_begin_default_pass(&pass_action, vp_size.x(), vp_size.y());
        sg_apply_viewport(cam_vp.bottomLeft().x(), cam_vp.bottomLeft().y(), cam_vp.sizeX(), cam_vp.sizeY(), false);
        sg_apply_scissor_rect(cam_vp.bottomLeft().x(), cam_vp.bottomLeft().y(), cam_vp.sizeX(), cam_vp.sizeY(), false);
        

        drawEntitiesFromCamera(r, tr, cam);




        sg_end_pass();
        sg_commit();

    }





    void updateDebugRendererComponents(entt::registry& r) {
        auto debugsView = r.view<cp::DebugRenderer>();
        for (auto entity : debugsView) {
            auto& dr = debugsView.get<cp::DebugRenderer>(entity);
            dr.remainingTime = dr.remainingTime - APS_AppDelta();
            if (dr.remainingTime <= 0.0f) {
                r.destroy(entity);
            }
        }
    }

	void drawCameras(entt::registry& r, SDL_Window* window) {


        auto viewCameras = r.view<cp::Transform, cp::Camera>();
        for (auto entity : viewCameras) {
            auto& tr = viewCameras.get<cp::Transform>(entity);
            auto& cam = viewCameras.get<cp::Camera>(entity);
            if (cam.is_active) {
                drawCamera(r, window, tr, cam);
            }
        }

        updateDebugRendererComponents(r);
	}



    void debugLine(entt::registry& r, const Vec2& start, const Vec2& end, Color color, float duration) {
        auto e = r.create();
        r.emplace<cp::DebugRenderer>(e, duration);
        cp::LineRenderer lr;
        lr.color = color;
        lr.is_looped = false;
        lr.points.push_back(start);
        lr.points.push_back(end);
        r.emplace<cp::LineRenderer>(e, lr);
        r.emplace<cp::Transform>(e);
    }

    void debugLines(entt::registry& r, const std::vector<Vec2>& points, bool isLooped, Color color, float duration) {
        auto e = r.create();
        r.emplace<cp::DebugRenderer>(e, duration);
        cp::LineRenderer lr;
        lr.color = color;
        lr.is_looped = isLooped;
        lr.points = points;
        r.emplace<cp::LineRenderer>(e, lr);
        r.emplace<cp::Transform>(e);
    }

    void debugCircle(entt::registry& r, const Vec2& position, float radius, Color color, float duration) {
        auto e = r.create();
        r.emplace<cp::DebugRenderer>(e, duration);
        cp::CircleRenderer cr;
        cr.color = color;
        cr.is_filled= false;
        cr.radius = radius;
        r.emplace<cp::CircleRenderer>(e, cr);
        r.emplace<cp::Transform>(e);
    }



    entt::entity createCameraEntity(entt::registry& r) {
        auto e = r.create();
        auto& c = r.emplace<cp::Camera>(e);
        c.viewport = { 0.5, 0.5, 1, 1 };
        c.clear_color = Color::blue();
        r.emplace<cp::Transform>(e);
        return e;
    }
  
}