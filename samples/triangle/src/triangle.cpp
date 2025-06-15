#include "triangle.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "application/application.h"

namespace Cyber
{
    namespace Samples
    {

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
            auto renderpass = renderer->get_render_pass();
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
            RenderObject::ITexture_View* attachment_resources[1] = { back_buffer_view  };
            frame_buffer->update_attachments(attachment_resources, 1);

            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = renderpass,
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
            device_context->render_encoder_bind_pipeline( pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            device_context->render_encoder_draw(3, 0);
            
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

            attachment_ref[1].m_attachmentIndex = 0;
            attachment_ref[1].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[1].m_loadAction = LOAD_ACTION_LOAD;
            attachment_ref[1].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[1].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[1].m_finalState = GRAPHICS_RESOURCE_STATE_PRESENT;

            //RenderObject::RenderSubpassDesc subpass_desc[1] = {};
            //subpass_desc.m_sampleCount = SAMPLE_COUNT_1;
            subpass_desc[0].m_name = u8"Main Subpass";
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = nullptr;
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
                .m_subpassCount = 2,
                .m_pSubpasses = subpass_desc
            };
            
            auto render_pass = device_context->create_render_pass(rp_desc1);
            renderer->set_render_pass(render_pass);
        }

        void TrignaleApp::create_resource()
        {

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
            auto swap_chain = renderer->get_swap_chain();
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
                .file_name = CYBER_UTF8("vertex_shader.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            RenderObject::IShaderLibrary* vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

            ResourceLoader::ShaderLoadDesc ps_load_desc = {};
            ps_load_desc.target = SHADER_TARGET_6_0;
            ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("pixel_shader.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("PSMain"),
            };
            RenderObject::IShaderLibrary* ps_shader = ResourceLoader::add_shader(render_device, ps_load_desc);

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
            .m_ppShaders = pipeline_shader_create_desc,
            .m_shaderCount = 2,
            };
            root_signature = render_device->create_root_signature(root_signature_create_desc);
            // create descriptor set

            RenderObject::DescriptorSetCreateDesc desc_set_create_desc = {
                .root_signature = root_signature,
                .set_index = 0
            };
            descriptor_set = render_device->create_descriptor_set(desc_set_create_desc);

           RenderObject::VertexLayoutDesc vertex_layout_desc = {
           };
            
            RenderObject::RenderPipelineCreateDesc rp_desc = 
            {
                .root_signature = root_signature,
                .vertex_shader = pipeline_shader_create_desc[0],
                .fragment_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout_desc,
                //.rasterizer_state = {},
                .color_formats = &swap_chain->get_back_buffer(0)->get_create_desc().m_format,
                .render_target_count = 1,
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