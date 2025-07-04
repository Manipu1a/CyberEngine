#include "editor/imgui_renderer.h"
#include "resource/resource_loader.h"
#include "graphics/interface/render_pipeline.h"
#include "graphics/interface/root_signature.hpp"
#include "graphics/interface/render_device.hpp"
#include "graphics/resource/resource_loader.h"
#include "platform/memory.h"
#include "application/application.h"
#include "editor/editor.h"

namespace Cyber
{
    namespace Editor
    {
        ImGuiRenderer::ImGuiRenderer(const EditorCreateInfo& createInfo)
            : render_device(createInfo.pDevice)
            , swap_chain(createInfo.swap_chain)
            , m_backBufferFmt(createInfo.BackBufferFmt)
            , m_depthBufferFmt(createInfo.DepthBufferFmt)
            , m_colorConversionMode(createInfo.ColorConversionMode)
            , vertex_buffer_size(createInfo.InitialVBSize)
            , index_buffer_size(createInfo.InitialIBSize)
        {
            // todo
            m_baseVertexSupported = true;
        }

        ImGuiRenderer::~ImGuiRenderer()
        {

        }

        void ImGuiRenderer::initialize(Editor* _editor)
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
        void ImGuiRenderer::render_draw_data(RenderObject::IDeviceContext* device_context, ImDrawData* draw_data)
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
                vertex_buffer = render_device->create_buffer(buffer_desc);
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
                index_buffer = render_device->create_buffer(buffer_desc);
            }
            BufferRange range;
            range.offset = 0;
            range.size = 0;
            void* vtx_resource = render_device->map_buffer(vertex_buffer,MAP_WRITE, MAP_FLAG_DISCARD);
            void* idx_resource = render_device->map_buffer(index_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            //int32_t vertex_buffer_size = 0;
            //int32_t index_buffer_size = 0;
            ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
            ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;
            for(int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                //vertex_buffer_size += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                //index_buffer_size += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
            render_device->unmap_buffer(vertex_buffer, MAP_WRITE);
            render_device->unmap_buffer(index_buffer, MAP_WRITE);

            vertex_buffer->set_buffer_size(vertex_buffer_size * sizeof(ImDrawVert));
            index_buffer->set_buffer_size(index_buffer_size * sizeof(ImDrawIdx));
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

                void* constant_data = render_device->map_buffer(vertex_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
                float4x4 * pConstantData = (float4x4*)constant_data;
                *pConstantData = projection;
                render_device->unmap_buffer(vertex_constant_buffer, MAP_WRITE);
            }
            
            auto SetupRenderState = [&]()
            {
                RenderObject::IBuffer* vertex_buffers[] = { vertex_buffer };
                uint32_t strides[] = { sizeof(ImDrawVert) };
                device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
                device_context->render_encoder_bind_index_buffer(index_buffer, sizeof(ImDrawIdx), 0);
                device_context->render_encoder_bind_pipeline(render_pipeline);

                const float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                device_context->render_encoder_set_blend_factor(blend_factor);

                RenderObject::Viewport vp;
                vp.top_left_x = 0;
                vp.top_left_y = 0;
                vp.width = static_cast<float>(draw_data->DisplaySize.x);
                vp.height = static_cast<float>(draw_data->DisplaySize.y);
                vp.min_depth = 0.0f;
                vp.max_depth = 1.0f;

                device_context->render_encoder_set_viewport(1, &vp);
            };

            auto* app = Core::Application::getApp();
            auto* renderer = app->get_renderer();
            m_backBufferIndex = renderer->get_back_buffer_index();

            auto back_buffer = swap_chain->get_back_buffer(m_backBufferIndex);
            auto back_buffer_view = swap_chain->get_back_buffer_srv_view(m_backBufferIndex);
            auto back_depth_buffer_view = swap_chain->get_back_buffer_dsv();
            auto frame_buffer = renderer->get_frame_buffer();

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
            };
            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc0);

            RenderObject::ITexture_View* attachment_resources[1] = { back_buffer_view  };
            frame_buffer->update_attachments(attachment_resources, 1);
            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
            GRAPHICS_CLEAR_VALUE* clear_values = { &clear_value };
            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
                .ClearValueCount = 1,
                .color_clear_values = &clear_value,
                .depth_stencil_clear_value = { 1.0f, 0 },
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            device_context->cmd_begin_render_pass(RenderPassBeginInfo);

            //todo use native render pipeline
            //device_context->cmd_next_sub_pass();

            SetupRenderState();

            // Render command lists
            uint32_t global_index_offset = 0;
            uint32_t global_vertex_offset = 0;
            RenderObject::ITexture_View* last_texture_view = nullptr;
            for(uint32_t cmd_list_id = 0; cmd_list_id < draw_data->CmdListsCount; cmd_list_id++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[cmd_list_id];
                for(uint32_t cmd_id = 0; cmd_id < cmd_list->CmdBuffer.Size; cmd_id++)
                {
                    const ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_id];
                    if(cmd->UserCallback)
                    {
                        // User callback, registered via ImDrawList::AddCallback()
                        if(cmd->UserCallback == ImDrawCallback_ResetRenderState)
                        {
                            // Reset render state
                            SetupRenderState();
                        }
                        else
                        {
                            // Call user callback
                            cmd->UserCallback(cmd_list, cmd);
                        }
                    }
                    else
                    {
                        if(cmd->ElemCount == 0)
                        {
                            // No elements to render
                            continue;
                        }
                        
                        float4 clip_rect = {
                            (cmd->ClipRect.x - draw_data->DisplayPos.x) * draw_data->FramebufferScale.x,
                            (cmd->ClipRect.y - draw_data->DisplayPos.y) * draw_data->FramebufferScale.y,
                            (cmd->ClipRect.z - draw_data->DisplayPos.x) * draw_data->FramebufferScale.x,
                            (cmd->ClipRect.w - draw_data->DisplayPos.y) * draw_data->FramebufferScale.y
                        };

                        clip_rect = transform_clip_rect(draw_data->DisplaySize, clip_rect);
                        RenderObject::Rect scissor
                        {
                            static_cast<int32_t>(clip_rect.x),
                            static_cast<int32_t>(clip_rect.y),
                            static_cast<int32_t>(clip_rect.z),
                            static_cast<int32_t>(clip_rect.w)
                        };
                        scissor.left = std::max(scissor.left, 0);
                        scissor.top = std::max(scissor.top, 0);
                        scissor.right = std::min(scissor.right, static_cast<int32_t>(draw_data->DisplaySize.x));
                        scissor.bottom = std::min(scissor.bottom, static_cast<int32_t>(draw_data->DisplaySize.y));

                        if(!scissor.is_valid())
                        {
                            // Invalid scissor rectangle, skip rendering
                            continue;
                        }

                        device_context->render_encoder_set_scissor(1, &scissor);
                        // Render command
                        RenderObject::ITexture_View* curr_texture_view = (RenderObject::ITexture_View*)cmd->TextureId;
                        //RenderObject::ITexture_View* curr_texture_view = font_srv;
                        if(curr_texture_view != last_texture_view)
                        {
                            last_texture_view = curr_texture_view;

                            TextureBarrier draw_barrier = {
                            .texture = font_texture,
                            .src_state = GRAPHICS_RESOURCE_STATE_COPY_DEST,
                            .dst_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE,
                            .subresource_barrier = 0
                            };
                            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
                            device_context->cmd_resource_barrier(barrier_desc0);
                        }                   
                        // shader variable binding
                        device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, vertex_constant_buffer);
                        //device_context->set_constant_buffer_view(SHADER_STAGE_VERT, 0, vertex_constant_buffer);
                        device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, curr_texture_view);
                        //render_device->update_descriptor_set(descriptor_set, descriptor_data, descriptor_set_idx);
                        uint32_t vertex_offset = 0;
                        if(m_baseVertexSupported)
                        {
                            vertex_offset = cmd->VtxOffset + global_vertex_offset;
                        }
                        else 
                        {
                            RenderObject::IBuffer* vertex_buffers[] = { vertex_buffer};
                            uint64_t offsets[] = { sizeof(ImDrawVert) * (size_t{cmd->VtxOffset} + size_t{global_vertex_offset}) };
                            uint32_t strides[] = { sizeof(ImDrawVert) };
                            device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, offsets);
                        }
                        //device_context->render_encoder_bind_descriptor_set(descriptor_set);
                        device_context->prepare_for_rendering(root_signature);
                        device_context->render_encoder_draw_indexed(cmd->ElemCount, cmd->IdxOffset + global_index_offset, vertex_offset);
                    }
                }
                global_index_offset += cmd_list->IdxBuffer.Size;
                global_vertex_offset += cmd_list->VtxBuffer.Size;
            }

            device_context->cmd_end_render_pass();
            
            TextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GRAPHICS_RESOURCE_STATE_PRESENT
            };
            ResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc2);
            
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
                .file_name = CYBER_UTF8("shaders/DX12/imgui_vs.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("main")
            };

            eastl::shared_ptr<RenderObject::IShaderLibrary> vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);
            //RenderObject::IShaderLibrary* vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

            ResourceLoader::ShaderLoadDesc ps_load_desc = {};
            ps_load_desc.target = SHADER_TARGET_6_0;
            ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("shaders/DX12/imgui_ps.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("main")
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> ps_shader = ResourceLoader::add_shader(render_device, ps_load_desc);

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
            auto sampler = render_device->create_sampler(sampler_create_desc);
            const char8_t* sampler_names[] = { CYBER_UTF8("Texture_sampler") };
            const char8_t* root_descriptor_names[] = { CYBER_UTF8("Constants") };

            //todo: not use reflection
            RenderObject::RootSignatureCreateDesc root_signature_create_desc = {
                .vertex_shader = pipeline_shader_create_desc[0],
                .pixel_shader = pipeline_shader_create_desc[1],
                .m_staticSamplers = &sampler,
                .m_staticSamplerNames = sampler_names,
                .m_staticSamplerCount = 1,
                .root_descriptor_names = root_descriptor_names,
                .root_descriptor_count = 1,
            };
            // todo 针对特定类型constent buffer选择不同更新方式
            root_signature = render_device->create_root_signature(root_signature_create_desc);
            /*
            RenderObject::DescriptorSetCreateDesc desc_set_create_desc = {
                .root_signature = root_signature,
                .set_index = 0
            };
            */
            //descriptor_set = render_device->create_descriptor_set(desc_set_create_desc);

            attachment_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            attachment_ref[0].m_attachmentIndex = 0;
            attachment_ref[0].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[0].m_loadAction = LOAD_ACTION_LOAD;
            attachment_ref[0].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[0].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[0].m_finalState = GRAPHICS_RESOURCE_STATE_PRESENT;
            
            subpass_desc[0].m_name = u8"UI Subpass";
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = nullptr;
            subpass_desc[0].m_renderTargetCount = 1;
            subpass_desc[0].m_pRenderTargetAttachments = &attachment_ref[0];
            
            RenderObject::RenderPassDesc rp_desc1 = {
                .m_name = u8"UI RenderPass",
                .m_attachmentCount = 1,
                .m_pAttachments = &attachment_desc,
                .m_subpassCount = 1,
                .m_pSubpasses = subpass_desc
            };

            Renderer::Renderer* renderer = Core::Application::getApp()->get_renderer();
            auto device_context = renderer->get_device_context();
            render_pass = device_context->create_render_pass(rp_desc1);

            RenderObject::VertexAttribute attri = { 0, 0, 2, VALUE_TYPE_FLOAT32 };

            RenderObject::VertexAttribute vertex_attributes[] = {
                {0, 0, 2, VALUE_TYPE_FLOAT32},
                {1, 0, 2, VALUE_TYPE_FLOAT32},
                {2, 0, 4, VALUE_TYPE_UINT8, true}
            };
            RenderObject::VertexLayoutDesc vertex_layout = {3, vertex_attributes};
            BlendStateCreateDesc blend_state_desc = {};
            blend_state_desc.render_target_count = 1;
            blend_state_desc.src_factors[0] = BLEND_CONSTANT_SRC_ALPHA;
            blend_state_desc.dst_factors[0] = BLEND_CONSTANT_ONE_MINUS_SRC_ALPHA;
            blend_state_desc.blend_modes[0] = BLEND_MODE_ADD;
            blend_state_desc.src_alpha_factors[0] = BLEND_CONSTANT_ONE;
            blend_state_desc.dst_alpha_factors[0] = BLEND_CONSTANT_ONE_MINUS_SRC_ALPHA;
            blend_state_desc.blend_alpha_modes[0] = BLEND_MODE_ADD;
            blend_state_desc.alpha_to_coverage = false;
            blend_state_desc.masks[0] = COLOR_WRITE_MASK_ALL;

            RenderObject::RenderPipelineCreateDesc rp_desc = {
                .root_signature = root_signature,
                .vertex_shader = pipeline_shader_create_desc[0],
                .fragment_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout,
                .blend_state = &blend_state_desc,
                .color_formats = &swap_chain->get_back_buffer(0)->get_create_desc().m_format,
                .render_target_count = 1,
                //.depth_stencil_format = m_depthBufferFmt,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST
            };
            render_pipeline = render_device->create_render_pipeline(rp_desc);
            vs_shader->free();
            ps_shader->free();

            RenderObject::BufferCreateDesc buffer_desc = {};
            buffer_desc.size = sizeof(float4x4);
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            vertex_constant_buffer = render_device->create_buffer(buffer_desc);
            
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
            texture_data.pDevice = render_device;
            //texture_data.pCommandBuffer = Core::Application::getApp()->get_renderer()->get_command_buffer();

            font_texture = render_device->create_texture(texture_desc, &texture_data);
            font_srv = font_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
            
            IO.Fonts->TexID = (ImTextureID)font_srv;
        }

        float4 ImGuiRenderer::transform_clip_rect(const ImVec2& display_size, const float4 rect) const
        {
            return rect;
        }
    }
}