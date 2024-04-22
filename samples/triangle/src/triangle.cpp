#include "triangle.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "gui/cyber_gui.h"

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

        void TrignaleApp::initialize(Cyber::WindowDesc& desc)
        {
            GameApplication::initialize(desc);
            gui_app = cyber_new<GUI::GUIApplication>();

            create_gfx_objects();
            
            gui_app->initialize(m_pRenderDevice, getWindow()->getNativeWindow());

            // Create views
            /*
            auto back_buffer_views = (RenderObject::ITextureView**)cyber_malloc(sizeof(RenderObject::ITextureView*) * m_pSwapChain->get_buffer_srv_count());
            m_pSwapChain->set_back_buffer_srv_views(back_buffer_views);
            for(uint32_t i = 0; i < m_pSwapChain->get_buffer_srv_count(); ++i)
            {
                eastl::basic_string<char8_t> swap_chain_name(eastl::basic_string<char8_t>::CtorSprintf(), u8"backbuffer_%d", i); // CYBER_UTF8("backbuffer_%d", i
                RenderObject::TextureViewCreateDesc view_desc = {
                    .m_name = swap_chain_name.c_str(),
                    .m_pTexture = m_pSwapChain->get_back_buffer(i),
                    .m_format = m_pSwapChain->get_back_buffer(i)->get_create_desc().m_format,
                    .m_usages = TVU_RTV_DSV,
                    .m_aspects = TVA_COLOR,
                    .m_dimension = TEX_DIMENSION_2D,
                    .m_arrayLayerCount = 1
                };
                auto tex_view = m_pRenderDevice->create_texture_view(view_desc);
                m_pSwapChain->set_back_buffer_srv_view(tex_view, i);
            }
            */
            
            m_pRenderDevice->cmd_begin(m_pCmd);
            {

            }
            m_pRenderDevice->cmd_end(m_pCmd);

            create_render_pipeline();

        }

        void TrignaleApp::run()
        {
            GameApplication::run();
        }

        void TrignaleApp::update(float deltaTime)
        {
            GameApplication::update(deltaTime);
            
            raster_draw();
        }

        void TrignaleApp::raster_draw()
        {
            AcquireNextDesc acquire_desc = {
                .fence = m_pPresentFence
            };
            m_backBufferIndex = m_pRenderDevice->acquire_next_image(m_pSwapChain, acquire_desc);
            auto back_buffer = m_pSwapChain->get_back_buffer(m_backBufferIndex);
            auto back_buffer_view = m_pSwapChain->get_back_buffer_srv_view(m_backBufferIndex);
            auto back_depth_buffer_view = m_pSwapChain->get_back_buffer_dsv();
            m_pRenderDevice->reset_command_pool(m_pPool);
            // record
            m_pRenderDevice->cmd_begin(m_pCmd);

            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
            
            RenderObject::ITextureView* attachment_resources[1] = { back_buffer_view  };
            RenderObject::FrameBuffserDesc frame_buffer_desc = {
                .m_pRenderPass = m_pRenderPass,
                .m_attachmentCount = 1,
                .m_ppAttachments = attachment_resources
            };
            auto frame_buffer = m_pRenderDevice->create_frame_buffer(frame_buffer_desc);

            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = m_pRenderPass,
                .ClearValueCount = 1,
                .pClearValues = &clear_value,
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_NONE
            };

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
            };
            TextureBarrier depth_barrier = {
                .texture = m_pSwapChain->get_back_buffer_depth(),
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE,
                .subresource_barrier = 0
            };
            TextureBarrier barriers[2];
            barriers[0] = draw_barrier;
            barriers[1] = depth_barrier;

            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            m_pRenderDevice->cmd_resource_barrier(m_pCmd, barrier_desc0);
            RenderPassEncoder* rp_encoder = m_pRenderDevice->cmd_begin_render_pass(m_pCmd, RenderPassBeginInfo);
            m_pRenderDevice->render_encoder_set_viewport(rp_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height, 0.0f, 1.0f);
            m_pRenderDevice->render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height);
            m_pRenderDevice->render_encoder_bind_pipeline(rp_encoder, pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            m_pRenderDevice->render_encoder_draw(rp_encoder, 3, 0);
            m_pRenderDevice->cmd_end_render_pass(m_pCmd);
            TextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GRAPHICS_RESOURCE_STATE_PRESENT
            };
            ResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            m_pRenderDevice->cmd_resource_barrier(m_pCmd, barrier_desc2);
            m_pRenderDevice->cmd_end(m_pCmd);
            // ui pass
            /*
            screen_attachment.load_action = LOAD_ACTION_LOAD;
            depth_attachment.depth_load_action = LOAD_ACTION_LOAD;
            depth_attachment.stencil_load_action = LOAD_ACTION_LOAD;

            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = nullptr,
                .pRenderPass = renderpass,
                .pClearValues = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f },
                .ClearValueCount = 1,
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_NONE
            };

            RenderObject::RenderPassDesc ui_rp_desc = {
                .sam = SAMPLE_COUNT_1,
                .color_attachments = &screen_attachment,
                .depth_stencil_attachment = &depth_attachment,
                .render_target_count = 1,
            };

            RenderPassEncoder* rp_ui_encoder = rhi_cmd_begin_render_pass(cmd, ui_rp_desc);
            rhi_render_encoder_set_viewport(rp_ui_encoder, 0, 0, back_buffer->mWidth, back_buffer->mHeight, 0.0f, 1.0f);
            rhi_render_encoder_set_scissor(rp_ui_encoder, 0, 0, back_buffer->mWidth, back_buffer->mHeight);
            // draw ui
            gui_app->update(cmd, 0.0f);
            rhi_cmd_end_render_pass(cmd);
            rhi_cmd_end(cmd);
            */
            // submit
            RenderObject::QueueSubmitDesc submit_desc = {
                .m_ppCmds = &m_pCmd,
                .m_pSignalFence = m_pPresentFence,
                .m_cmdsCount = 1
            };
            m_pRenderDevice->submit_queue(m_pQueue, submit_desc);

            // present
            RenderObject::QueuePresentDesc present_desc = {
                .m_pSwapChain = m_pSwapChain,
                .m_ppwaitSemaphores = nullptr,
                .m_waitSemaphoreCount = 0,
                .m_index = m_backBufferIndex,
            };
            m_pRenderDevice->present_queue(m_pQueue, present_desc);

            // sync & reset
            //m_pRenderDevice->wait_queue_idle(m_pQueue);
            m_pQueue->signal_fence(m_pPresentFence, m_pPresentFence->get_fence_value());
            
            m_pRenderDevice->wait_fences(&m_pPresentFence, 1);
        }

        void TrignaleApp::create_gfx_objects()
        {
            GameApplication::create_gfx_objects();

            //RenderObject::RenderPassAttachmentDesc attachments[1] = {};
            attachment_desc.m_sampleCount = 1;
            attachment_desc.m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
            attachment_desc.m_loadAction = LOAD_ACTION_CLEAR;
            attachment_desc.m_storeAction = STORE_ACTION_STORE;
            attachment_desc.m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_desc.m_finalState = GRAPHICS_RESOURCE_STATE_PRESENT;
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
            attachment_ref = { 0, GRAPHICS_RESOURCE_STATE_RENDER_TARGET };

            //RenderObject::RenderSubpassDesc subpass_desc[1] = {};
            subpass_desc.m_sampleCount = SAMPLE_COUNT_1;
            subpass_desc.m_inputAttachmentCount = 0;
            subpass_desc.m_pInputAttachments = nullptr;
            subpass_desc.m_pDepthStencilAttachment = nullptr;
            subpass_desc.m_renderTargetCount = 1;
            subpass_desc.m_pRenderTargetAttachments = &attachment_ref;

            RenderObject::RenderPassDesc rp_desc1 = {
                .m_attachmentCount = 1,
                .m_pAttachments = &attachment_desc,
                .m_subpassCount = 1,
                .m_pSubpasses = &subpass_desc
            };
            
            m_pRenderPass = m_pRenderDevice->create_render_pass(rp_desc1);
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
                RenderObject::IShaderLibrary* vs_shader = ResourceLoader::add_shader(m_pRenderDevice, vs_load_desc);

                ResourceLoader::ShaderLoadDesc ps_load_desc = {};
                ps_load_desc.target = SHADER_TARGET_6_0;
                ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                    .file_name = CYBER_UTF8("pixel_shader.hlsl"),
                    .stage = SHADER_STAGE_FRAG,
                    .entry_point_name = CYBER_UTF8("PSMain"),
                };
                RenderObject::IShaderLibrary* ps_shader = ResourceLoader::add_shader(m_pRenderDevice, ps_load_desc);

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
                root_signature = m_pRenderDevice->create_root_signature(root_signature_create_desc);
                // create descriptor set

                RenderObject::DescriptorSetCreateDesc desc_set_create_desc = {
                    .root_signature = root_signature,
                    .set_index = 0
                };
                descriptor_set = m_pRenderDevice->create_descriptor_set(desc_set_create_desc);

                VertexLayout vertex_layout = {.attribute_count = 0};
                RenderObject::RenderPipelineCreateDesc rp_desc = 
                {
                    .root_signature = root_signature,
                    .vertex_shader = pipeline_shader_create_desc[0],
                    .fragment_shader = pipeline_shader_create_desc[1],
                    .vertex_layout = &vertex_layout,
                    //.rasterizer_state = {},
                    .color_formats = &m_pSwapChain->get_back_buffer(0)->get_create_desc().m_format,
                    .render_target_count = 1,
                    .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
                };
                pipeline = m_pRenderDevice->create_render_pipeline(rp_desc);
                vs_shader->free();
                ps_shader->free();
        }

        void TrignaleApp::finalize()
        {
            m_pRenderDevice->wait_queue_idle(m_pQueue);
            m_pRenderDevice->wait_fences(&m_pPresentFence, 1);
            m_pRenderDevice->free_fence(m_pPresentFence);
            for(uint32_t i = 0;i < m_pSwapChain->get_buffer_srv_count(); ++i)
            {
                m_pRenderDevice->free_texture_view(m_pSwapChain->get_back_buffer_srv_view(i));
            }
            m_pRenderDevice->free_swap_chain(m_pSwapChain);
            m_pRenderDevice->free_surface(m_pSurface);
            m_pRenderDevice->free_command_buffer(m_pCmd);
            m_pRenderDevice->free_command_pool(m_pPool);
            m_pRenderDevice->free_render_pipeline(pipeline);
            m_pRenderDevice->free_root_signature(root_signature);
            m_pRenderDevice->free_queue(m_pQueue);
            m_pRenderDevice->free_device();
            m_pRenderDevice->free_instance(m_pInstance);
        }

    }
}


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

    FreeConsole();
    return 1;
}