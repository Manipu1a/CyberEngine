#include "editor/imgui_renderer.h"
#include "resource/resource_loader.h"
#include "graphics/interface/render_pipeline.h"
#include "graphics/interface/root_signature.hpp"
#include "graphics/interface/render_device.hpp"
#include "platform/memory.h"
#include "core/Application.h"
#include "common/basic_math.hpp"

namespace Cyber
{
    namespace Editor
    {
        ImGuiRenderer::ImGuiRenderer(const EditorCreateInfo& createInfo)
            : m_pDevice(createInfo.pDevice)
            , m_backBufferFmt(createInfo.BackBufferFmt)
            , m_depthBufferFmt(createInfo.DepthBufferFmt)
            , m_colorConversionMode(createInfo.ColorConversionMode)
            , vertex_buffer_size(createInfo.InitialVBSize)
            , index_buffer_size(createInfo.InitialIBSize)
        {

        }

        ImGuiRenderer::~ImGuiRenderer()
        {

        }

        void ImGuiRenderer::initialize()
        {
            if(!render_pipeline)
            {
                create_device_objects();
            }
        }

        void ImGuiRenderer::new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {

        }
        void ImGuiRenderer::end_frame()
        {

        }
        void ImGuiRenderer::render_draw_data(ImDrawData* draw_data)
        {
            if(draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f || draw_data->CmdListsCount == 0)
            {
                return;
            }

            if(!vertex_buffer || static_cast<int>(vertex_buffer_size) < draw_data->TotalVtxCount)
            {
                if(vertex_buffer)
                {
                    vertex_buffer->free();
                    vertex_buffer = nullptr;
                }
                while(static_cast<int>(vertex_buffer_size) < draw_data->TotalVtxCount)
                {
                    vertex_buffer_size *= 2;
                }
                RenderObject::BufferCreateDesc buffer_desc = {};
                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER;
                buffer_desc.size = vertex_buffer_size * sizeof(ImDrawVert);
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                vertex_buffer = m_pDevice->create_buffer(buffer_desc);
            }

            if(!index_buffer || static_cast<int>(index_buffer_size) < draw_data->TotalIdxCount)
            {
                if(index_buffer)
                {
                    index_buffer->free();
                    index_buffer = nullptr;
                }
                while(static_cast<int>(index_buffer_size) < draw_data->TotalIdxCount)
                {
                    index_buffer_size *= 2;
                }
                RenderObject::BufferCreateDesc buffer_desc = {};
                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
                buffer_desc.size = index_buffer_size * sizeof(ImDrawIdx);
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                index_buffer = m_pDevice->create_buffer(buffer_desc);
                
            }
            BufferRange range;
            range.offset = 0;
            range.size = 0;
            void* vtx_resource = m_pDevice->map_buffer(vertex_buffer,MAP_WRITE, MAP_FLAG_DISCARD);
            void* idx_resource = m_pDevice->map_buffer(index_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            
            ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
            ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;
            for(int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
            m_pDevice->unmap_buffer(vertex_buffer, MAP_WRITE);
            m_pDevice->unmap_buffer(index_buffer, MAP_WRITE);

            // Setup orthographic projection matrix into our constant buffer
            // Our visible imgui space lies from pDrawData->DisplayPos (top left) to pDrawData->DisplayPos+data_data->DisplaySize (bottom right).
            // DisplayPos is (0,0) for single viewport setup
            {
                // DisplaySize always refers to the logical dimensions that account for pre-transform, hence
                // the aspect ratio will be correct after applying appropriate rotation.
                float L = draw_data->DisplayPos.x;
                float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
                float T = draw_data->DisplayPos.y;
                float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

                float4x4 projection
                {
                2.0f / (R - L),                  0.0f,   0.0f,   0.0f,
                0.0f,                  2.0f / (T - B),   0.0f,   0.0f,
                0.0f,                            0.0f,   0.5f,   0.0f,
                (R + L) / (L - R),  (T + B) / (B - T),   0.5f,   1.0f
                };
            }
            
        }
        void ImGuiRenderer::invalidate_device_objects()
        {
            /*
            m_pVB->free();
            m_pIB->free();
            m_pVertexConstantBuffer->free();
            m_pPipeline->free();
            m_pFontSRV->free();
            m_pSRB->free();
            */
        }
        void ImGuiRenderer::create_device_objects()
        {
            invalidate_device_objects();

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

            RenderObject::VertexAttribute attri = { 0, 0, 2, VALUE_TYPE_FLOAT32 };

            RenderObject::VertexAttribute vertex_attributes[] = {
                {0, 0, 2, VALUE_TYPE_FLOAT32},
                {1, 0, 2, VALUE_TYPE_FLOAT32},
                {2, 0, 4, VALUE_TYPE_UINT8, true}
            };
            RenderObject::VertexLayoutDesc vertex_layout = {3, vertex_attributes};

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
            render_pipeline = m_pDevice->create_render_pipeline(rp_desc);
            vs_shader->free();
            ps_shader->free();

            RenderObject::BufferCreateDesc buffer_desc = {};
            buffer_desc.size = sizeof(float4x4);
            vertex_constant_buffer = m_pDevice->create_buffer(buffer_desc);
            
            create_fonts_texture();
        }

        void ImGuiRenderer::create_fonts_texture()
        {
            // Build texture atlas
            ImGuiIO& IO = ImGui::GetIO();

            unsigned char* pData  = nullptr;
            int            Width  = 0;
            int            Weight = 0;
            IO.Fonts->GetTexDataAsRGBA32(&pData, &Width, &Weight);

            RenderObject::TextureCreateDesc texture_desc = {};
            texture_desc.m_width = Width;
            texture_desc.m_height = Weight;
            texture_desc.m_depth = 1;
            texture_desc.m_mipLevels = 1;
            texture_desc.m_arraySize = 1;
            texture_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            texture_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
            texture_desc.m_initializeState = GRAPHICS_RESOURCE_STATE_COPY_DEST;
            texture_desc.m_sampleCount = SAMPLE_COUNT_1;
            texture_desc.m_sampleQuality = 1;

            RenderObject::TextureSubResData sub_res_data = {};
            sub_res_data.pData = pData;
            sub_res_data.stride = Width * 4;
            sub_res_data.depthStride = 0;
            sub_res_data.srcOffset = 0;

            RenderObject::TextureData texture_data = {};
            texture_data.pSubResources = &sub_res_data;
            texture_data.numSubResources = 1;
            texture_data.pDevice = m_pDevice;
            //texture_data.pCommandBuffer = Core::Application::getApp()->get_renderer()->get_command_buffer();

            auto texture = m_pDevice->create_texture(texture_desc, &texture_data);
            font_srv = texture->get_default_texture_view(TVU_SRV);
            
            IO.Fonts->TexID = (ImTextureID)font_srv;
        }
    }
}