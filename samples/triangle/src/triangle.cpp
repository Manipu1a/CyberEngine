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
            
            gui_app->initialize(device, getWindow()->getNativeWindow());

            // Create views
            auto back_buffer_views = (RenderObject::ITextureView**)cyber_malloc(sizeof(RenderObject::ITextureView*) * swap_chain->get_buffer_srv_count());
            swap_chain->set_back_buffer_srv_views(back_buffer_views);
            for(uint32_t i = 0; i < swap_chain->get_buffer_srv_count(); ++i)
            {
                eastl::basic_string<char8_t> swap_chain_name(eastl::basic_string<char8_t>::CtorSprintf(), u8"backbuffer_%d", i); // CYBER_UTF8("backbuffer_%d", i
                RenderObject::TextureViewCreateDesc view_desc = {
                    .m_name = swap_chain_name.c_str(),
                    .m_pTexture = swap_chain->get_back_buffer(i),
                    .m_format = swap_chain->get_back_buffer(i)->get_create_desc().m_format,
                    .m_usages = TVU_RTV_DSV,
                    .m_aspects = TVA_COLOR,
                    .m_dimension = TEX_DIMENSION_2D,
                    .m_arrayLayerCount = 1
                };
                auto tex_view = device->create_texture_view(view_desc);
                swap_chain->set_back_buffer_srv_view(tex_view, i);
            }
            
            device->cmd_begin(cmd);
            {
                TextureBarrier depth_barrier = {
                    .texture = swap_chain->get_back_buffer_depth(),
                    .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                    .dst_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE
                };
                ResourceBarrierDesc barrier_desc1 = { .texture_barriers = &depth_barrier, .texture_barrier_count = 1 };
                device->cmd_resource_barrier(cmd, barrier_desc1);
            }
            device->cmd_end(cmd);

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
                .fence = present_fence
            };
            backbuffer_index = device->acquire_next_image(swap_chain, acquire_desc);
            auto back_buffer = swap_chain->get_back_buffer(backbuffer_index);
            auto back_buffer_view = swap_chain->get_back_buffer_srv_view(backbuffer_index);
            auto back_depth_buffer_view = swap_chain->get_back_buffer_dsv();
            device->reset_command_pool(pool);
            // record
            device->cmd_begin(cmd);
            ColorAttachment screen_attachment = {
                .view = back_buffer_view,
                .load_action = LOAD_ACTION_CLEAR,
                .store_action = STORE_ACTION_STORE,
                .clear_value = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f }
            };

            DepthStencilAttachment depth_attachment = {
                .view = back_depth_buffer_view,
                .depth_load_action = LOAD_ACTION_CLEAR,
                .depth_store_action = STORE_ACTION_STORE,
                .clear_depth = 0.0f,
                .write_depth = 1,
                .stencil_load_action = LOAD_ACTION_CLEAR,
                .stencil_store_action = STORE_ACTION_STORE,
                .clear_stencil = 0,
                .write_stencil = 0
            };
            
            RenderObject::AttachmentReference color_attachment_ref = {
                .m_attachmentIndex = 0,
                .m_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET
            };

            RenderObject::AttachmentReference depth_attachment_ref = {
                .m_attachmentIndex = 1,
                .m_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE
            };

            RenderObject::RenderSubpassDesc subpass_desc = {
                .m_sampleCount = SAMPLE_COUNT_1,
                .m_inputAttachmentCount = 0,
                .m_pInputAttachments = nullptr,
                .m_pDepthStencilAttachment = &depth_attachment_ref,
                .m_renderTargetCount = 1,
                .m_pRenderTargetAttachments = &color_attachment_ref
            };

            RenderObject::RenderPassDesc rp_desc1 = {
                .m_attachmentCount = 0,
                .m_pAttachments = nullptr,
                .m_subpassCount = 1,
                .m_pSubpasses = nullptr
            };
            
            auto renderpass = device->create_render_pass(rp_desc1);
            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
           
            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = nullptr,
                .pRenderPass = renderpass,
                .ClearValueCount = 1,
                .pClearValues = &clear_value,
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_NONE
            };

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_PRESENT,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET
            };

            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            device->cmd_resource_barrier(cmd, barrier_desc0);
            RenderPassEncoder* rp_encoder = device->cmd_begin_render_pass(cmd, RenderPassBeginInfo);
            device->render_encoder_set_viewport(rp_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height, 0.0f, 1.0f);
            device->render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height);
            device->render_encoder_bind_pipeline(rp_encoder, pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            device->render_encoder_draw(rp_encoder, 3, 0);
            device->cmd_end_render_pass(cmd);

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

            RHITextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = RHI_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = RHI_RESOURCE_STATE_PRESENT
            };
            RHIResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            rhi_cmd_resource_barrier(cmd, barrier_desc2);
            rhi_cmd_end(cmd);
            */
            // submit
            RenderObject::QueueSubmitDesc submit_desc = {
                .m_ppCmds = &cmd,
                .m_pSignalFence = present_fence,
                .m_cmdsCount = 1
            };
            device->submit_queue(queue, submit_desc);

            // present
            RenderObject::QueuePresentDesc present_desc = {
                .m_pSwapChain = swap_chain,
                .m_ppwaitSemaphores = nullptr,
                .m_waitSemaphoreCount = 0,
                .m_index = backbuffer_index,
            };
            device->present_queue(queue, present_desc);

            // sync & reset
            device->wait_fences(&present_fence, 1);
        }

        void TrignaleApp::create_gfx_objects()
        {
            // Create instance
            DECLARE_ZERO(RenderObject::InstanceCreateDesc, instance_desc);
            instance_desc.m_enableDebugLayer = true;
            instance_desc.m_enableGpuBasedValidation = false;
            instance_desc.m_enableSetName = true;
            instance = device->create_instance(instance_desc);

            // Filter adapters
            uint32_t adapter_count = 0;
            device->enum_adapters(instance, nullptr, &adapter_count);
            RenderObject::IAdapter* adapters[64];
            device->enum_adapters(instance, adapters, &adapter_count);
            adapter = adapters[0];

            // Create device
            DECLARE_ZERO(QueueGroupDesc, queue_group_desc);
            queue_group_desc.m_queueCount = 1;
            queue_group_desc.m_queueType = QUEUE_TYPE_GRAPHICS;
            DECLARE_ZERO(RenderObject::RenderDeviceCreateDesc, device_desc);
            device_desc.m_queueGroupCount = 1;
            device_desc.m_queueGroups = { queue_group_desc };
            device->create_device(adapter, device_desc);
            queue = device->get_queue(QUEUE_TYPE_GRAPHICS, 0);
            present_fence = device->create_fence();

            auto window = getWindow();

            // Create swapchain
        #if defined (_WIN32) || defined (_WIN64)
            surface = device->surface_from_hwnd(getWindow()->getNativeWindow());
        #elif defined(_APPLE_)
        #endif
            DECLARE_ZERO(RenderObject::SwapChainDesc, chain_desc);
            chain_desc.m_pSurface = surface;
            chain_desc.m_width = getWindow()->getWidth();
            chain_desc.m_height = getWindow()->getHeight();
            chain_desc.m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
            chain_desc.m_imageCount = 3;
            chain_desc.m_presentQueue = queue;
            chain_desc.m_presentQueueCount = 1;
            chain_desc.m_enableVsync = true;
            swap_chain = device->create_swap_chain(chain_desc);

            present_swmaphore = device->create_fence();
            pool = device->create_command_pool(queue, RenderObject::CommandPoolCreateDesc());

            RenderObject::CommandBufferCreateDesc cmd_buffer_desc = {.m_isSecondary = false};
            cmd =  device->create_command_buffer(pool, cmd_buffer_desc);
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
                RenderObject::IShaderLibrary* vs_shader = ResourceLoader::add_shader(device, vs_load_desc);

                ResourceLoader::ShaderLoadDesc ps_load_desc = {};
                ps_load_desc.target = SHADER_TARGET_6_0;
                ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                    .file_name = CYBER_UTF8("pixel_shader.hlsl"),
                    .stage = SHADER_STAGE_FRAG,
                    .entry_point_name = CYBER_UTF8("PSMain"),
                };
                RenderObject::IShaderLibrary* ps_shader = ResourceLoader::add_shader(device, ps_load_desc);

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
                root_signature = device->create_root_signature(root_signature_create_desc);
                // create descriptor set

                RenderObject::DescriptorSetCreateDesc desc_set_create_desc = {
                    .root_signature = root_signature,
                    .set_index = 0
                };
                descriptor_set = device->create_descriptor_set(desc_set_create_desc);

                VertexLayout vertex_layout = {.attribute_count = 0};
                RenderObject::RenderPipelineCreateDesc rp_desc = 
                {
                    .root_signature = root_signature,
                    .vertex_shader = pipeline_shader_create_desc[0],
                    .fragment_shader = pipeline_shader_create_desc[1],
                    .vertex_layout = &vertex_layout,
                    //.rasterizer_state = {},
                    .color_formats = &swap_chain->get_back_buffer_srv_view(0)->get_create_desc().m_format,
                    .render_target_count = 1,
                    .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
                };
                pipeline = device->create_render_pipeline(rp_desc);
                vs_shader->free();
                ps_shader->free();
        }

        void TrignaleApp::finalize()
        {
            device->wait_queue_idle(queue);
            device->wait_fences(&present_fence, 1);
            device->free_fence(present_fence);
            for(uint32_t i = 0;i < swap_chain->get_buffer_srv_count(); ++i)
            {
                device->free_texture_view(swap_chain->get_back_buffer_srv_view(i));
            }
            device->free_swap_chain(swap_chain);
            device->free_surface(surface);
            device->free_command_buffer(cmd);
            device->free_command_pool(pool);
            device->free_render_pipeline(pipeline);
            device->free_root_signature(root_signature);
            device->free_queue(queue);
            device->free_device();
            device->free_instance(instance);
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