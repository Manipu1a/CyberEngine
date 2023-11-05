#include "triangle.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "rhi/backend/d3d12/rhi_d3d12.h"
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
            RHI::createRHI(ERHIBackend::RHI_BACKEND_D3D12);
            gui_app = cyber_new<GUI::GUIApplication>();

            create_gfx_objects();
            
            gui_app->initialize(device, getWindow()->getNativeWindow());

            // Create views
            swap_chain->mBackBufferSRVViews = (RHITextureView**)cyber_malloc(sizeof(RHITextureView*) * swap_chain->mBufferSRVCount);
            for(uint32_t i = 0; i < swap_chain->mBufferSRVCount; ++i)
            {
                eastl::basic_string<char8_t> swap_chain_name(eastl::basic_string<char8_t>::CtorSprintf(), u8"backbuffer_%d", i); // CYBER_UTF8("backbuffer_%d", i
                TextureViewCreateDesc view_desc = {
                    .name = swap_chain_name.c_str(),
                    .texture = swap_chain->mBackBufferSRVs[i],
                    .format = (ERHIFormat)swap_chain->mBackBufferSRVs[i]->mFormat,
                    .usages = RHI_TVU_RTV_DSV,
                    .aspects = RHI_TVA_COLOR,
                    .dimension = RHI_TEX_DIMENSION_2D,
                    .array_layer_count = 1
                };
                swap_chain->mBackBufferSRVViews[i] = rhi_create_texture_view(device, view_desc);
            }
            
            rhi_cmd_begin(cmd);
            {
                RHITextureBarrier depth_barrier = {
                    .texture = swap_chain->mBackBufferDSV,
                    .src_state = RHI_RESOURCE_STATE_COMMON,
                    .dst_state = RHI_RESOURCE_STATE_DEPTH_WRITE
                };
                RHIResourceBarrierDesc barrier_desc1 = { .texture_barriers = &depth_barrier, .texture_barrier_count = 1 };
                rhi_cmd_resource_barrier(cmd, barrier_desc1);
            }
            rhi_cmd_end(cmd);

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
            RHIAcquireNextDesc acquire_desc = {
                .fence = present_fence
            };
            backbuffer_index = rhi_acquire_next_image(swap_chain, acquire_desc);
            auto back_buffer = swap_chain->mBackBufferSRVs[backbuffer_index];
            auto back_buffer_view = swap_chain->mBackBufferSRVViews[backbuffer_index];
            auto back_depth_buffer_view = swap_chain->mBackBufferDSVView;
            rhi_reset_command_pool(pool);
            // record
            rhi_cmd_begin(cmd);
            RHIColorAttachment screen_attachment = {
                .view = back_buffer_view,
                .load_action = RHI_LOAD_ACTION_CLEAR,
                .store_action = RHI_STORE_ACTION_STORE,
                .clear_value = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f }
            };

            RHIDepthStencilAttachment depth_attachment = {
                .view = back_depth_buffer_view,
                .depth_load_action = RHI_LOAD_ACTION_CLEAR,
                .depth_store_action = RHI_STORE_ACTION_STORE,
                .clear_depth = 0.0f,
                .write_depth = 1,
                .stencil_load_action = RHI_LOAD_ACTION_CLEAR,
                .stencil_store_action = RHI_STORE_ACTION_STORE,
                .clear_stencil = 0,
                .write_stencil = 0
            };

            RHIRenderPassDesc rp_desc = {
                .sample_count = RHI_SAMPLE_COUNT_1,
                .color_attachments = &screen_attachment,
                .depth_stencil_attachment = &depth_attachment,
                .render_target_count = 1,
            };
            RHITextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = RHI_RESOURCE_STATE_PRESENT,
                .dst_state = RHI_RESOURCE_STATE_RENDER_TARGET
            };

            RHIResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            rhi_cmd_resource_barrier(cmd, barrier_desc0);
            RHIRenderPassEncoder* rp_encoder = rhi_cmd_begin_render_pass(cmd, rp_desc);
            rhi_render_encoder_set_viewport(rp_encoder, 0, 0, back_buffer->mWidth, back_buffer->mHeight, 0.0f, 1.0f);
            rhi_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->mWidth, back_buffer->mHeight);
            rhi_render_encoder_bind_pipeline(rp_encoder, pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            rhi_render_encoder_draw(rp_encoder, 3, 0);
            rhi_cmd_end_render_pass(cmd);

            // ui pass
            screen_attachment.load_action = RHI_LOAD_ACTION_LOAD;
            depth_attachment.depth_load_action = RHI_LOAD_ACTION_LOAD;
            depth_attachment.stencil_load_action = RHI_LOAD_ACTION_LOAD;
            RHIRenderPassDesc ui_rp_desc = {
                .sample_count = RHI_SAMPLE_COUNT_1,
                .color_attachments = &screen_attachment,
                .depth_stencil_attachment = &depth_attachment,
                .render_target_count = 1,
            };

            RHIRenderPassEncoder* rp_ui_encoder = rhi_cmd_begin_render_pass(cmd, ui_rp_desc);
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

            // submit
            RHIQueueSubmitDesc submit_desc = {
                .pCmds = &cmd,
                .mSignalFence = present_fence,
                .mCmdsCount = 1
            };
            rhi_submit_queue(queue, submit_desc);

            // present
            RHIQueuePresentDesc present_desc = {
                .swap_chain = swap_chain,
                .wait_semaphores = nullptr,
                .wait_semaphore_count = 0,
                .index = backbuffer_index,
            };
            rhi_present_queue(queue, present_desc);

            // sync & reset
            rhi_wait_fences(&present_fence, 1);
        }

        void TrignaleApp::create_gfx_objects()
        {
            // Create instance
            DECLARE_ZERO(RHIInstanceCreateDesc, instance_desc);
            instance_desc.enable_debug_layer = true;
            instance_desc.enable_gpu_based_validation = false;
            instance_desc.enable_set_name = true;
            instance = rhi_create_instance(instance_desc);

            // Filter adapters
            uint32_t adapter_count = 0;
            rhi_enum_adapters(instance, nullptr, &adapter_count);
            RHIAdapter* adapters[64];
            rhi_enum_adapters(instance, adapters, &adapter_count);
            adapter = adapters[0];

            // Create device
            DECLARE_ZERO(RHIQueueGroupDesc, queue_group_desc);
            queue_group_desc.queue_count = 1;
            queue_group_desc.queue_type = RHI_QUEUE_TYPE_GRAPHICS;
            DECLARE_ZERO(RHIDeviceCreateDesc, device_desc);
            device_desc.queue_group_count = 1;
            device_desc.queue_groups = { queue_group_desc };
            device = rhi_create_device(adapter, device_desc);
            queue = rhi_get_queue(device, RHI_QUEUE_TYPE_GRAPHICS, 0);
            present_fence = rhi_create_fence(device);

            auto window = getWindow();

            // Create swapchain
        #if defined (_WIN32) || defined (_WIN64)
            surface = rhi_surface_from_hwnd(device, getWindow()->getNativeWindow());
        #elif defined(_APPLE_)
        #endif
            DECLARE_ZERO(RHISwapChainCreateDesc, chain_desc);
            chain_desc.surface = surface;
            chain_desc.mWidth = getWindow()->getWidth();
            chain_desc.mHeight = getWindow()->getHeight();
            chain_desc.mFormat = RHI_FORMAT_R8G8B8A8_UNORM;
            chain_desc.mImageCount = 3;
            chain_desc.mPresentQueue = queue;
            chain_desc.mPresentQueueCount = 1;
            chain_desc.mEnableVsync = true;
            swap_chain = rhi_create_swap_chain(device, chain_desc);

            present_swmaphore = rhi_create_fence(device);
            pool = rhi_create_command_pool(queue, CommandPoolCreateDesc());

            CommandBufferCreateDesc cmd_buffer_desc = {.is_secondary = false};
            cmd =  rhi_create_command_buffer(pool, cmd_buffer_desc);
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
                vs_load_desc.target = shader_target_6_0;
                vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                    .file_name = CYBER_UTF8("vertex_shader.hlsl"),
                    .stage = RHI_SHADER_STAGE_VERT,
                    .entry_point_name = CYBER_UTF8("VSMain"),
                };
                RHIShaderLibrary* vs_shader = ResourceLoader::add_shader(device, vs_load_desc);

                ResourceLoader::ShaderLoadDesc ps_load_desc = {};
                ps_load_desc.target = shader_target_6_0;
                ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                    .file_name = CYBER_UTF8("pixel_shader.hlsl"),
                    .stage = RHI_SHADER_STAGE_FRAG,
                    .entry_point_name = CYBER_UTF8("PSMain"),
                };
                RHIShaderLibrary* ps_shader = ResourceLoader::add_shader(device, ps_load_desc);

                // create root signature
                RHIPipelineShaderCreateDesc* pipeline_shader_create_desc[2];
                pipeline_shader_create_desc[0] = cyber_new<RHIPipelineShaderCreateDesc>();
                pipeline_shader_create_desc[0]->stage = RHI_SHADER_STAGE_VERT;
                pipeline_shader_create_desc[0]->library = vs_shader;
                pipeline_shader_create_desc[0]->entry = CYBER_UTF8("VSMain");
                pipeline_shader_create_desc[1] = cyber_new<RHIPipelineShaderCreateDesc>();
                pipeline_shader_create_desc[1]->stage = RHI_SHADER_STAGE_FRAG;
                pipeline_shader_create_desc[1]->library = ps_shader;
                pipeline_shader_create_desc[1]->entry = CYBER_UTF8("PSMain");
                RHIRootSignatureCreateDesc root_signature_create_desc = {
                .shaders = pipeline_shader_create_desc,
                .shader_count = 2,
                };
                root_signature = rhi_create_root_signature(device, root_signature_create_desc);
                // create descriptor set

                RHIDescriptorSetCreateDesc desc_set_create_desc = {
                    .root_signature = root_signature,
                    .set_index = 0
                };
                descriptor_set = rhi_create_descriptor_set(device, desc_set_create_desc);

                RHIVertexLayout vertex_layout = {.attribute_count = 0};
                RHIRenderPipelineCreateDesc rp_desc = 
                {
                    .root_signature = root_signature,
                    .vertex_shader = pipeline_shader_create_desc[0],
                    .fragment_shader = pipeline_shader_create_desc[1],
                    .vertex_layout = &vertex_layout,
                    //.rasterizer_state = {},
                    .color_formats = &swap_chain->mBackBufferSRVViews[0]->create_info.format,
                    .render_target_count = 1,
                    .prim_topology = RHI_PRIM_TOPO_TRIANGLE_LIST,
                };
                pipeline = rhi_create_render_pipeline(device, rp_desc);
                rhi_free_shader_library(vs_shader);
                rhi_free_shader_library(ps_shader);
        }

        void TrignaleApp::finalize()
        {
            rhi_wait_queue_idle(queue);
            rhi_wait_fences(&present_fence, 1);
            rhi_free_fence(present_fence);
            for(uint32_t i = 0;i < swap_chain->mBufferSRVCount; ++i)
            {
                rhi_free_texture_view(swap_chain->mBackBufferSRVViews[i]);
            }
            rhi_free_swap_chain(swap_chain);
            rhi_free_surface(surface);
            rhi_free_command_buffer(cmd);
            rhi_free_command_pool(pool);
            rhi_free_render_pipeline(pipeline);
            rhi_free_root_signature(root_signature);
            rhi_free_queue(queue);
            rhi_free_device(device);
            rhi_free_instance(instance);
        }

    }
}


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR    lpCmdLine,
                     int       nCmdShow)
{
    //auto app = create_sample();
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