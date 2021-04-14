#include <destral.h>




/* application state */
static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

int main() {
	ds::app::config cfg;
    ds::ecs::registry_create();


    cfg.on_init = []() {
        /* a vertex buffer with 3 vertices */
        float vertices[] = {
            // positions            // colors
             0.0f,  0.2f, 0.5f,     0.76f, 0.38f, 0.23f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
        };

        sg_buffer_desc sbdesc = { 0 };
        sbdesc.data.ptr = &vertices[0];
        sbdesc.data.size = sizeof(float) * (9 + 12);
        

        sbdesc.label = "triangle-vertices";
        

        state.bind.vertex_buffers[0] = sg_make_buffer(sbdesc);

        /* create shader from code-generated sg_shader_desc */
        sg_shader_desc sh = { 0 };
        sh.vs.source =
            " #version 330\n "
            "    layout (location = 0) in vec3 position;\n              "
            "    layout (location = 1) in vec4 color0;\n                "
            "                                   "
            "    out vec4 color;\n                "
            "                                   "
            "    void main() {\n                  "
            "        gl_Position = vec4(position,1);\n    "
            "        color = color0;\n            "
            "    }\n                              ";

        sh.fs.source =
            " #version 330\n"
            "in vec4 color;\n          "
            "out vec4 frag_color;\n    "
            "                        "
            "void main() {\n           "
            "    frag_color = color;\n "
            "}\n                       ";

        sg_shader shd = sg_make_shader(sh);

        /* create a pipeline object (default render states are fine for triangle) */
        sg_pipeline_desc spipdesc = { 0 };
        spipdesc.shader = shd;
        spipdesc.label = "triangle-pipeline";
        spipdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
        spipdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
        state.pip = sg_make_pipeline(spipdesc);

        /* a pass action to framebuffer to black */
        sg_pass_action apassaction = { 0 };
        apassaction.colors[0].action = SG_ACTION_CLEAR;
        apassaction.colors[0].value = {0.0f, 0.0f, 0.0f, 1.0f };

        state.pass_action = apassaction;

    };

    cfg.on_render = []() {
        sg_begin_default_pass(&state.pass_action, 1280 , 720);
        sg_apply_pipeline(state.pip);
        sg_apply_bindings(&state.bind);
        sg_draw(0, 3, 1);
        sg_end_pass();
    };

	ds::app::run(cfg);
}



