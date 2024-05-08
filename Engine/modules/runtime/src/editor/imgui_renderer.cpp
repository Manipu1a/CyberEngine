#include "editor/imgui_renderer.h"
#include "resource/resource_loader.h"
#include "graphics/interface/render_pipeline.h"
#include "graphics/interface/root_signature.hpp"
#include "graphics/interface/render_device.hpp"
#include "platform/memory.h"
namespace Cyber
{
    namespace Editor
    {
        ImGuiRenderer::ImGuiRenderer(const EditorCreateInfo& createInfo)
            : m_pDevice(createInfo.pDevice)
            , m_backBufferFmt(createInfo.BackBufferFmt)
            , m_depthBufferFmt(createInfo.DepthBufferFmt)
            , m_colorConversionMode(createInfo.ColorConversionMode)
        {

        }

        ImGuiRenderer::~ImGuiRenderer()
        {

        }

        void ImGuiRenderer::new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {
            if(!m_pPipeline)
            {
                create_device_objects();
            }
        }
        void ImGuiRenderer::end_frame()
        {

        }
        void ImGuiRenderer::render_draw_data()
        {

        }
        void ImGuiRenderer::invalidate_device_objects()
        {

        }
        void ImGuiRenderer::create_device_objects()
        {
            // create shader
            ResourceLoader::ShaderLoadDesc vs_load_desc = {};
            vs_load_desc.target = SHADER_TARGET_6_0;
            vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("imgui_vs.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("main")
            };
            RenderObject::IShaderLibrary* vs_shader = ResourceLoader::add_shader(m_pDevice, vs_load_desc);

            ResourceLoader::ShaderLoadDesc ps_load_desc = {};
            ps_load_desc.target = SHADER_TARGET_6_0;
            ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("imgui_ps.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("main")
            };
            RenderObject::IShaderLibrary* ps_shader = ResourceLoader::add_shader(m_pDevice, ps_load_desc);

            RenderObject::PipelineShaderCreateDesc* pipeline_shader_create_desc[2];
            pipeline_shader_create_desc[0] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[0]->m_library = vs_shader;
            pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("main");
            pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            pipeline_shader_create_desc[1] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[1]->m_library = ps_shader;
            pipeline_shader_create_desc[1]->m_entry = CYBER_UTF8("main");
            pipeline_shader_create_desc[1]->m_stage = SHADER_STAGE_FRAG;
            RenderObject::RootSignatureCreateDesc root_signature_create_desc = {
                .m_ppShaders = pipeline_shader_create_desc,
                .m_shaderCount = 2,
                .m_staticSamplers = nullptr,
                .m_staticSamplerCount = 1
            };
            RenderObject::IRootSignature* root_signature = m_pDevice->create_root_signature(root_signature_create_desc);

            VertexLayout vertex_layout = {};
            RenderObject::RenderPipelineCreateDesc rp_desc = {
                .root_signature = root_signature,
                .vertex_shader = pipeline_shader_create_desc[0],
                .fragment_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout,
                .color_formats = &m_backBufferFmt,
                .render_target_count = 1,
                .depth_stencil_format = m_depthBufferFmt,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST
            };
            m_pPipeline = m_pDevice->create_render_pipeline(rp_desc);
            vs_shader->free();
            ps_shader->free();
        }

        void ImGuiRenderer::create_fonts_texture()
        {

        }
    }
}