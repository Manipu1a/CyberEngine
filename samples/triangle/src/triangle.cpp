#include "triangle.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "application/application.h"
#include "texture_utils.h"
#include "math/matrix.h"
#include "resource/vertex.h"

namespace Cyber
{
    namespace Samples
    {
        Cyber::Samples::SampleApp* Cyber::Samples::SampleApp::create_sample_app()
        {
            return Cyber::cyber_new<Cyber::Samples::TrignaleApp>();
        }

        const static Vertex vertices[] = {
                { { -0.5f, -0.5f, 0.0f }, {0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f} }, // Vertex 1: Position (-0.5, -0.5), Color Red
                { {  0.5f, -0.5f, 0.0f }, {1.f, 0.0f},{ 0.0f, 1.0f, 0.0f, 1.0f } }, // Vertex 2: Position (0.5, -0.5), Color Green
                { {  0.0f,  0.5f, 0.0f }, {1.f, 1.f},{ 0.0f, 0.0f, 1.0f, 1.0f } }  // Vertex 3: Position (0.0, 0.5), Color Blue
            };

        const static uint32_t indices[] = {
                2, 1, 0 // Triangle made of the three vertices
            };
        
        TrignaleApp::TrignaleApp()
        {

        }

        TrignaleApp::~TrignaleApp()
        {
            
        }

        void TrignaleApp::initialize()
        {
            SampleApp::initialize();
            m_pApp = Cyber::Core::Application::getApp();
            cyber_check(m_pApp);
            create_gfx_objects();

            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            create_render_pipeline();
            create_resource();
        }

        void TrignaleApp::run()
        {
            //m_pApp->run();
        }

        void TrignaleApp::update(float deltaTime)
        {
           // m_pApp->update(deltaTime);
            
            raster_draw();
        }

        void TrignaleApp::raster_draw()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            auto swap_chain = renderer->get_swap_chain();
            auto frame_buffer = renderer->get_frame_buffer();

            AcquireNextDesc acquire_desc = {};
            m_backBufferIndex = render_device->acquire_next_image(swap_chain, acquire_desc);
            renderer->set_back_buffer_index(m_backBufferIndex);
            auto& scene_target = renderer->get_scene_target(m_backBufferIndex);
            auto back_buffer = scene_target.color_buffer;
            auto back_depth_buffer = scene_target.depth_buffer;
            auto back_buffer_view = scene_target.color_buffer->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
            auto back_depth_buffer_view = scene_target.depth_buffer->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);

            // record
            device_context->cmd_begin();

            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
            GRAPHICS_CLEAR_VALUE* clear_values = { &clear_value };
            RenderObject::ITexture_View* attachment_resources[2] = { back_buffer_view, back_depth_buffer_view};
            frame_buffer->update_attachments(attachment_resources, 2);
            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
                .ClearValueCount = 1,
                .color_clear_values = &clear_value,
                .depth_stencil_clear_value = { 1.0f, 0 },
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
            };
            TextureBarrier depth_barrier = {
                .texture = back_depth_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE,
                .subresource_barrier = 0
            };
            
            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc0);
            ResourceBarrierDesc barrier_desc1 = { .texture_barriers = &depth_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc1);

            device_context->cmd_begin_render_pass(RenderPassBeginInfo);
            device_context->set_frame_buffer(frame_buffer);
            RenderObject::Viewport viewport;
            viewport.top_left_x = 0.0f;
            viewport.top_left_y = 0.0f;
            viewport.width = back_buffer->get_create_desc().m_width;
            viewport.height = back_buffer->get_create_desc().m_height;
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;
            device_context->render_encoder_set_viewport(1, &viewport);
            RenderObject::Rect scissor
            {
                0, 0, 
                (int32_t)back_buffer->get_create_desc().m_width, 
                (int32_t)back_buffer->get_create_desc().m_height
            };

            device_context->render_encoder_set_scissor( 1, &scissor);
            RenderObject::IBuffer* vertex_buffers[] = { vertex_buffer };
            uint32_t strides[] = { sizeof(Vertex) };
            device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
            device_context->render_encoder_bind_index_buffer(index_buffer, sizeof(uint32_t), 0);
            device_context->render_encoder_bind_pipeline( pipeline);
            device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, test_texture_view);

            device_context->prepare_for_rendering(root_signature);
            device_context->render_encoder_draw_indexed(3, 0, 0);
            //device_context->render_encoder_draw(3, 0);
            device_context->cmd_end_render_pass();

            TextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE
            };
            ResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc2);
            //device_context->set_render_target( 1, &back_buffer_view, nullptr);     
        }

        void TrignaleApp::present()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            auto swap_chain = renderer->get_swap_chain();
            auto back_buffer = swap_chain->get_back_buffer(m_backBufferIndex);

            //device_context->cmd_end_render_pass();
            device_context->cmd_end();

            // submit
            device_context->flush();

            // present
            render_device->present(swap_chain);
        }

        void TrignaleApp::create_gfx_objects()
        {
            //m_pApp->get_renderer()->create_gfx_objects();
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            //RenderObject::RenderPassAttachmentDesc attachments[1] = {};
            attachment_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            attachment_ref[0].m_attachmentIndex = 0;
            attachment_ref[0].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[0].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[0].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[0].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[0].m_finalState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            
            attachment_ref[1].m_attachmentIndex = 1;
            attachment_ref[1].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[1].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[1].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[1].m_initialState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_ref[1].m_finalState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            /*
            attachment_ref[1].m_attachmentIndex = 0;
            attachment_ref[1].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[1].m_loadAction = LOAD_ACTION_LOAD;
            attachment_ref[1].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[1].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[1].m_finalState = GRAPHICS_RESOURCE_STATE_PRESENT;
            */
            //RenderObject::RenderSubpassDesc subpass_desc[1] = {};
            //subpass_desc.m_sampleCount = SAMPLE_COUNT_1;
            subpass_desc[0].m_name = u8"Main Subpass";
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = &attachment_ref[1];
            subpass_desc[0].m_renderTargetCount = 1;
            subpass_desc[0].m_pRenderTargetAttachments = &attachment_ref[0];
            
            subpass_desc[1].m_name = u8"UI Subpass";
            subpass_desc[1].m_inputAttachmentCount = 0;
            subpass_desc[1].m_pInputAttachments = nullptr;
            subpass_desc[1].m_pDepthStencilAttachment = nullptr;
            subpass_desc[1].m_renderTargetCount = 1;
            subpass_desc[1].m_pRenderTargetAttachments = &attachment_ref[1];
            
            RenderObject::RenderPassDesc rp_desc1 = {
                .m_name = u8"Triangle RenderPass",
                .m_attachmentCount = 1,
                .m_pAttachments = &attachment_desc,
                .m_subpassCount = 1,
                .m_pSubpasses = subpass_desc
            };
            
            render_pass = device_context->create_render_pass(rp_desc1);
            //renderer->set_render_pass(render_pass);
        }

        void TrignaleApp::create_resource()
        {
            auto render_device = m_pApp->get_renderer()->get_render_device();

            RenderObject::BufferCreateDesc buffer_desc = {};
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER;
            buffer_desc.size = 3 * sizeof(Vertex);
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_STAGING;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            vertex_buffer = render_device->create_buffer(buffer_desc);
            
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
            buffer_desc.size = 3 * sizeof(uint32_t);
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_STAGING;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            index_buffer = render_device->create_buffer(buffer_desc);

            // map vertex buffer
            void* vtx_resource = render_device->map_buffer(vertex_buffer,MAP_WRITE, MAP_FLAG_DISCARD);
            Vertex* vertices_ptr = (Vertex*)vtx_resource;
            memcpy(vertices_ptr, vertices, sizeof(vertices));
            render_device->unmap_buffer(vertex_buffer, MAP_WRITE);
            vertex_buffer->set_buffer_size(3 * sizeof(Vertex));

            // map index buffer
            void* idx_resource = render_device->map_buffer(index_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            uint32_t* indices_ptr = (uint32_t*)idx_resource;
            memcpy(indices_ptr, indices, sizeof(indices));
            render_device->unmap_buffer(index_buffer, MAP_WRITE);
            index_buffer->set_buffer_size(3 * sizeof(uint32_t));

            // create texture
            RenderObject::ITexture* test_texture = nullptr;
            TextureLoader::TextureLoadInfo texture_load_info{
                CYBER_UTF8("TEST"),
                GRAPHICS_RESOURCE_USAGE_IMMUTABLE,
                GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE,
                0,
                CPU_ACCESS_NONE,
                true,
                false,
                TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN,
                false,
                FILTER_TYPE::FILTER_TYPE_LINEAR
            };

            TextureLoader::create_texture_from_file(
                ("samples/triangle/assets/1.png"),
                texture_load_info,
                &test_texture, render_device
            );

            test_texture_view = test_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
        }

        void TrignaleApp::create_ui()
        {

        }

        void TrignaleApp::draw_ui()
        {

        }

        void TrignaleApp::create_render_pipeline()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            //render_graph::RenderGraph* graph = cyber_new<render_graph::RenderGraph>();
            /*
            namespace render_graph = Cyber::render_graph;
            render_graph::RenderGraph* graph = render_graph::RenderGraph::create([=](render_graph::RenderGraphBuilder& builder)
            {
                builder.with_device(device)
                .backend_api(ERHIBackend::RHI_BACKEND_D3D12);
            });
            
            auto builder = graph->get_builder();
            auto tex = builder->create_texture(
                render_graph::RGTextureCreateDesc{ 
                .mWidth = 1920, .mHeight = 1080 , .mFormat = RHI_FORMAT_R8G8B8A8_SRGB }
                , u8"tex");

            auto backbuffer = builder->create_texture(
                render_graph::RGTextureCreateDesc{ 
                .mWidth = 1920, .mHeight = 1080 , .mFormat = RHI_FORMAT_R8G8B8A8_SRGB }
                , u8"color");

            auto depth = builder->create_texture(
                render_graph::RGTextureCreateDesc{ 
                .mWidth = 1920, .mHeight = 1080 , .mFormat = RHI_FORMAT_R8G8B8A8_SRGB }
                , u8"depth");

            builder->add_render_pass(
                u8"ShadowPass",
                 [=](render_graph::RGRenderPass& pass)
                {
                    pass.set_pipeline(nullptr)
                    .add_render_target(0, depth);
                },
                [=](render_graph::RenderGraph& rg, render_graph::RenderPassContext& context)
                {
                    // execute context
                    CB_CORE_INFO("test render graph : execute context");
                });

            builder->add_render_pass(
                u8"ColorPass",
                 [=](render_graph::RGRenderPass& pass)
                {
                    pass.add_input(u8"Tex", tex)
                    .add_input(u8"Depth", depth)
                    .set_pipeline(nullptr)
                    .add_render_target(0, backbuffer);
                },
                [=](render_graph::RenderGraph& rg, render_graph::RenderPassContext& context)
                {
                    // execute context
                    CB_CORE_INFO("test render graph : execute context");
                });

            builder->add_render_pass(
                u8"PostPass",
                 [=](render_graph::RGRenderPass& pass)
                {
                    pass.add_input(u8"Color", backbuffer)
                    .set_pipeline(nullptr)
                    .add_render_target(0, backbuffer);
                },
                [=](render_graph::RenderGraph& rg, render_graph::RenderPassContext& context)
                {
                    // execute context
                    CB_CORE_INFO("test render graph : execute context");
                });

            
            //graph->add_custom_phase<render_graph::RenderGraphPhase_Prepare>();
            //graph->add_custom_phase<render_graph::RenderGraphPhase_Render>();

            graph->execute();
            */

            // create shader
            ResourceLoader::ShaderLoadDesc vs_load_desc = {};
            vs_load_desc.target = SHADER_TARGET_6_0;
            vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/triangle/assets/vertex_shader.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

            ResourceLoader::ShaderLoadDesc ps_load_desc = {};
            ps_load_desc.target = SHADER_TARGET_6_0;
            ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/triangle/assets/pixel_shader.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("PSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> ps_shader = ResourceLoader::add_shader(render_device, ps_load_desc);

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

            // create root signature
            RenderObject::PipelineShaderCreateDesc* pipeline_shader_create_desc[2];
            pipeline_shader_create_desc[0] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            pipeline_shader_create_desc[0]->m_library = vs_shader;
            pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("VSMain");
            pipeline_shader_create_desc[1] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[1]->m_stage = SHADER_STAGE_FRAG;
            pipeline_shader_create_desc[1]->m_library = ps_shader;
            pipeline_shader_create_desc[1]->m_entry = CYBER_UTF8("PSMain");
            
            RenderObject::RootSignatureCreateDesc root_signature_create_desc = {
                .vertex_shader = pipeline_shader_create_desc[0],
                .pixel_shader = pipeline_shader_create_desc[1],
                .m_staticSamplers = &sampler,
                .m_staticSamplerNames = sampler_names,
                .m_staticSamplerCount = 1,
            };
            root_signature = render_device->create_root_signature(root_signature_create_desc);
            // create descriptor set

            RenderObject::DescriptorSetCreateDesc desc_set_create_desc = {
                .root_signature = root_signature,
                .set_index = 0
            };
            //descriptor_set = render_device->create_descriptor_set(desc_set_create_desc);
            RenderObject::VertexAttribute vertex_attributes[] = {
                {0, 0, 3, VALUE_TYPE_FLOAT32},
                {1, 0, 2, VALUE_TYPE_FLOAT32},
                {2, 0, 4, VALUE_TYPE_UINT8, true}
            };
            RenderObject::VertexLayoutDesc vertex_layout_desc = {3, vertex_attributes};
            
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

            auto& scene_target = renderer->get_scene_target(0);
            
            RenderObject::RenderPipelineCreateDesc rp_desc = 
            {
                .root_signature = root_signature,
                .vertex_shader = pipeline_shader_create_desc[0],
                .fragment_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout_desc,
                .blend_state = &blend_state_desc,
                .color_formats = &scene_target.color_buffer->get_create_desc().m_format,
                .render_target_count = 1,
                .depth_stencil_format = scene_target.depth_buffer->get_create_desc().m_format,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
            };
            pipeline = render_device->create_render_pipeline(rp_desc);

            vs_shader->free();
            ps_shader->free();
        }

        void TrignaleApp::finalize()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto swap_chain = renderer->get_swap_chain();
            auto renderpass = renderer->get_render_pass();
            auto surface = renderer->get_surface();
            auto instance = renderer->get_instance();
            for(uint32_t i = 0;i < swap_chain->get_buffer_srv_count(); ++i)
            {
                render_device->free_texture_view(swap_chain->get_back_buffer_srv_view(i));
            }
            render_device->free_swap_chain(swap_chain);
            render_device->free_surface(surface);
            render_device->free_render_pipeline(pipeline);
            render_device->free_root_signature(root_signature);
            render_device->free_device();
            render_device->free_instance(instance);
        }
    }
}