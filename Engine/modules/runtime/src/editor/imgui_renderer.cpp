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
        void ImGuiRenderer::render_draw_data(RenderObject::IRenderDevice* device, ImDrawData* drawData)
        {
            if(drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f || drawData->CmdListsCount == 0)
            {
                return;
            }

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

            RenderObject::SamplerCreateDesc sampler_create_desc = {};
            sampler_create_desc.min_filter = FILTER_TYPE_LINEAR;
            sampler_create_desc.mag_filter = FILTER_TYPE_LINEAR;
            sampler_create_desc.mip_filter = FILTER_TYPE_LINEAR;
            sampler_create_desc.address_u = ADDRESS_MODE_CLAMP;
            sampler_create_desc.address_v = ADDRESS_MODE_CLAMP;
            sampler_create_desc.address_w = ADDRESS_MODE_CLAMP;
            sampler_create_desc.flags = SAMPLER_FLAG_NONE;
            sampler_create_desc.unnormalized_coordinates = false;
            sampler_create_desc.mip_lod_bias = 0.0f;
            sampler_create_desc.max_anisotropy = 0;
            sampler_create_desc.compare_mode = CMP_NEVER;
            sampler_create_desc.border_color = { 0.0f, 0.0f, 0.0f, 0.0f };
            sampler_create_desc.min_lod = 0.0f;
            sampler_create_desc.max_lod = 0.0f;
            auto sampler = m_pDevice->create_sampler(sampler_create_desc);
            const char8_t* sampler_names[] = { CYBER_UTF8("Texture_sampler") };

            RenderObject::RootSignatureCreateDesc root_signature_create_desc = {
                .m_ppShaders = pipeline_shader_create_desc,
                .m_shaderCount = 2,
                .m_staticSamplers = &sampler,
                .m_staticSamplerNames = sampler_names,
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
            // Build texture atlas
            ImGuiIO& IO = ImGui::GetIO();

            unsigned char* pData  = nullptr;
            int            Width  = 0;
            int            Weight = 0;
            IO.Fonts->GetTexDataAsRGBA32(&pData, &Width, &Weight);

            
        }
    }
}