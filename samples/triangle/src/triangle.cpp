#include "triangle.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "core/Application.h"

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
            auto cmd = renderer->get_command_buffer();

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
            /*
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            //auto cmd = renderer->get_command_buffer();
            auto swap_chain = renderer->get_swap_chain();
            auto present_fence = renderer->get_present_semaphore();
            auto pool = renderer->get_command_pool();
            auto queue = renderer->get_queue();
            auto renderpass = renderer->get_render_pass();

            AcquireNextDesc acquire_desc = {
                .fence = present_fence
            };
            m_backBufferIndex = render_device->acquire_next_image(swap_chain, acquire_desc);
            renderer->set_back_buffer_index(m_backBufferIndex);
            auto back_buffer = swap_chain->get_back_buffer(m_backBufferIndex);
            auto back_buffer_view = swap_chain->get_back_buffer_srv_view(m_backBufferIndex);
            auto back_depth_buffer_view = swap_chain->get_back_buffer_dsv();
            render_device->reset_command_pool(pool);
            // record
            render_device->cmd_begin(cmd);

            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
            
            RenderObject::ITextureView* attachment_resources[1] = { back_buffer_view  };
            RenderObject::FrameBuffserDesc frame_buffer_desc = {
                .m_pRenderPass = renderpass,
                .m_attachmentCount = 1,
                .m_ppAttachments = attachment_resources
            };
            auto frame_buffer = render_device->create_frame_buffer(frame_buffer_desc);

            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = renderpass,
                .ClearValueCount = 1,
                .pClearValues = &clear_value,
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
            };
            TextureBarrier depth_barrier = {
                .texture = swap_chain->get_back_buffer_depth(),
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE,
                .subresource_barrier = 0
            };

            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            render_device->cmd_resource_barrier(cmd, barrier_desc0);
            render_device->cmd_begin_render_pass(cmd, RenderPassBeginInfo);
            render_device->render_encoder_set_viewport(cmd, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height, 0.0f, 1.0f);
            render_device->render_encoder_set_scissor(cmd, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height);
            render_device->render_encoder_bind_pipeline(cmd, pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            render_device->render_encoder_draw(cmd, 3, 0);
            render_device->cmd_end_render_pass(cmd);
            
            // ui pass
            //render_device->cmd_next_sub_pass(cmd);
            //RenderPassEncoder* rp_ui_encoder = m_pRenderDevice->cmd_begin_render_pass(m_pCmd, RenderPassBeginInfo);
            //render_device->render_encoder_set_viewport(cmd, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height, 0.0f, 1.0f);
            //render_device->render_encoder_set_scissor(cmd, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height);
            // draw ui
            /*
            render_device->set_render_target(cmd, 1, &back_buffer_view, nullptr);
            auto editor = m_pApp->get_editor();
            editor->update(cmd, 0.0f);
            */

            
        }

        void TrignaleApp::present()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = render_device->get_device_context();
            auto cmd = renderer->get_command_buffer();
            auto swap_chain = renderer->get_swap_chain();
            auto present_fence = renderer->get_present_semaphore();
            auto pool = renderer->get_command_pool();
            auto queue = renderer->get_queue();
            auto renderpass = renderer->get_render_pass();
            auto back_buffer = swap_chain->get_back_buffer(m_backBufferIndex);

            TextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GRAPHICS_RESOURCE_STATE_PRESENT
            };
            ResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            render_device->cmd_resource_barrier(cmd, barrier_desc2);
            render_device->cmd_end(cmd);
            // submit
            RenderObject::QueueSubmitDesc submit_desc = {
                .m_ppCmds = &cmd,
                .m_pSignalFence = renderer->get_present_semaphore(),
                .m_cmdsCount = 1
            };
            render_device->submit_queue(queue, submit_desc);

            // present
            RenderObject::QueuePresentDesc present_desc = {
                .m_pSwapChain = swap_chain,
                .m_ppwaitSemaphores = nullptr,
                .m_waitSemaphoreCount = 0,
                .m_index = m_backBufferIndex,
            };
            render_device->present_queue(queue, present_desc);

            // sync & reset
            //m_pRenderDevice->wait_queue_idle(m_pQueue);
            queue->signal_fence(present_fence, present_fence->get_fence_value());
            
            render_device->wait_fences(&present_fence, 1);
        }

        void TrignaleApp::create_gfx_objects()
        {
            //m_pApp->get_renderer()->create_gfx_objects();
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            //RenderObject::RenderPassAttachmentDesc attachments[1] = {};
            attachment_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            //attachments[1].m_sampleCount = 1;
            //attachments[1].m_format = TEXTURE_FORMAT_D24_UNORM_S8_UINT;
            //attachments[1].m_loadAction = LOAD_ACTION_CLEAR;
            //attachments[1].m_storeAction = STORE_ACTION_STORE;
            //attachments[1].m_initialState = GRAPHICS_RESOURCE_STATE_COMMON;
            //attachments[1].m_finalState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            /*
            RenderObject::AttachmentReference attachment_ref[] = 
            {
                {
                    .m_attachmentIndex = 0,
                    .m_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET
                },
                {
                    .m_attachmentIndex = 1,
                    .m_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE
                }
            };
            */
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
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = nullptr;
            subpass_desc[0].m_renderTargetCount = 1;
            subpass_desc[0].m_pRenderTargetAttachments = &attachment_ref[0];

            subpass_desc[1].m_inputAttachmentCount = 0;
            subpass_desc[1].m_pInputAttachments = nullptr;
            subpass_desc[1].m_pDepthStencilAttachment = nullptr;
            subpass_desc[1].m_renderTargetCount = 1;
            subpass_desc[1].m_pRenderTargetAttachments = &attachment_ref[1];
            
            RenderObject::RenderPassDesc rp_desc1 = {
                .m_attachmentCount = 1,
                .m_pAttachments = &attachment_desc,
                .m_subpassCount = 2,
                .m_pSubpasses = subpass_desc
            };
            
            auto render_pass = render_device->create_render_pass(rp_desc1);
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
            auto cmd = renderer->get_command_buffer();
            auto swap_chain = renderer->get_swap_chain();
            auto present_fence = renderer->get_present_semaphore();
            auto pool = renderer->get_command_pool();
            auto queue = renderer->get_queue();
            auto renderpass = renderer->get_render_pass();
            auto surface = renderer->get_surface();
            auto instance = renderer->get_instance();

            render_device->wait_queue_idle(queue);
            render_device->wait_fences(&present_fence, 1);
            render_device->free_fence(present_fence);
            for(uint32_t i = 0;i < swap_chain->get_buffer_srv_count(); ++i)
            {
                render_device->free_texture_view(swap_chain->get_back_buffer_srv_view(i));
            }
            render_device->free_swap_chain(swap_chain);
            render_device->free_surface(surface);
            render_device->free_command_buffer(cmd);
            render_device->free_command_pool(pool);
            render_device->free_render_pipeline(pipeline);
            render_device->free_root_signature(root_signature);
            render_device->free_queue(queue);
            render_device->free_device();
            render_device->free_instance(instance);
        }

    }
}

/*
int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR    lpCmdLine,
                     int       nCmdShow)
{
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CON", "r", stdin);
    freopen_s(&stream, "CON", "w", stdout);
    SetConsoleTitle(L"Console");
    //std::cout << "test log" << std::endl;
    Cyber::Log::initLog();
    CB_CORE_INFO("initLog");

    Cyber::WindowDesc desc;
    desc.title = L"Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 720;
    desc.hInstance = hInstance;
    desc.cmdShow = nCmdShow;

    Cyber::Samples::TrignaleApp app;
    app.initialize(desc);
    app.run();

    while(Cyber::Core::Application::getApp().is_running())
    {
        app.update(0.0f);
    }

    FreeConsole();
    return 1;
}
*/