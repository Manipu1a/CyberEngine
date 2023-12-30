#include "renderpass.h"
#include "platform/memory.h"
#include "graphics/rendergraph/render_graph_resource.h"
#include "graphics/rendergraph/render_graph_builder.h"
#include "graphics/resource/resource_loader.h"

namespace Cyber
{
    namespace Samples
    {
        RenderPassApp::RenderPassApp()
        {
            
        }

        RenderPassApp::~RenderPassApp()
        {

        }

        void RenderPassApp::initialize(Cyber::WindowDesc& desc)
        {
            GameApplication::initialize(desc);
            //RHI::createRHI(ERHIBackend::RHI_BACKEND_D3D12);

            create_gfx_objects();

            // Create views
            swap_chain->mBackBufferSRVViews = (RenderObject::ITextureView**)cyber_malloc(sizeof(RenderObject::ITextureView*) * swap_chain->mBufferSRVCount);
            for(uint32_t i = 0; i < swap_chain->mBufferSRVCount; ++i)
            {
                eastl::basic_string<char8_t> swap_chain_name(eastl::basic_string<char8_t>::CtorSprintf(), u8"backbuffer_%d", i); // CYBER_UTF8("backbuffer_%d", i
                RenderObject::TextureViewCreateDesc view_desc = {
                    .name = swap_chain_name.c_str(),
                    .texture = swap_chain->mBackBufferSRVs[i],
                    .format = (ERHIFormat)swap_chain->mBackBufferSRVs[i]->get_create_desc().format,
                    .usages = RHI_TVU_RTV_DSV,
                    .aspects = RHI_TVA_COLOR,
                    .dimension = RHI_TEX_DIMENSION_2D,
                    .array_layer_count = 1
                };
                swap_chain->mBackBufferSRVViews[i] = device->create_texture_view(view_desc);
            }
            
            device->cmd_begin(cmd);
            {
                RHITextureBarrier depth_barrier = {
                    .texture = swap_chain->mBackBufferDSV,
                    .src_state = RHI_RESOURCE_STATE_COMMON,
                    .dst_state = RHI_RESOURCE_STATE_DEPTH_WRITE
                };
                RHIResourceBarrierDesc barrier_desc1 = { .texture_barriers = &depth_barrier, .texture_barrier_count = 1 };
                device->cmd_resource_barrier(cmd, barrier_desc1);
            }
            device->cmd_end(cmd);

            create_render_pipeline();
        }

        void RenderPassApp::run()
        {
            GameApplication::run();

        }

        void RenderPassApp::update(float deltaTime)
        {
            GameApplication::update(deltaTime);
            
            raster_draw();
        }

        void RenderPassApp::create_gfx_objects()
        {
            auto window = getWindow();
        }

        void RenderPassApp::create_render_pipeline()
        {
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
            root_signature = device->create_root_signature(root_signature_create_desc);
            // create descriptor set

            RHIDescriptorSetCreateDesc desc_set_create_desc = {
                .root_signature = root_signature,
                .set_index = 0
            };
            descriptor_set = device->create_descriptor_set(desc_set_create_desc);

            RHIVertexLayout vertex_layout = {.attribute_count = 0};
            auto color_format = swap_chain->mBackBufferSRVViews[0]->get_create_desc().format;
            RHIRenderPipelineCreateDesc rp_desc = 
            {
                .root_signature = root_signature,
                .vertex_shader = pipeline_shader_create_desc[0],
                .fragment_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout,
                //.rasterizer_state = {},
                .color_formats = &color_format,
                .render_target_count = 1,
                .prim_topology = RHI_PRIM_TOPO_TRIANGLE_LIST,
            };
            pipeline = device->create_render_pipeline(rp_desc);
            device->free_shader_library(vs_shader);
            device->free_shader_library(ps_shader);
        }

        void RenderPassApp::create_render_pass()
        {
            // attachment 1 - color
            // attachment 2 - depth
            // attachment 3 - final color
            constexpr uint32_t num_attachments = 3;

            RenderObject::RenderPassAttachmentDesc attachments[num_attachments] = {};
            attachments[0].format = ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;
            attachments[0].load_action = RHI_LOAD_ACTION_CLEAR;
            attachments[0].store_action = RHI_STORE_ACTION_STORE;
            attachments[0].initial_state = RHI_RESOURCE_STATE_RENDER_TARGET;
            attachments[0].final_state = RHI_RESOURCE_STATE_RENDER_TARGET;
            
            attachments[1].format = ERHIFormat::RHI_FORMAT_D24_UNORM_S8_UINT;
            attachments[1].load_action = RHI_LOAD_ACTION_CLEAR;
            attachments[1].store_action = RHI_STORE_ACTION_STORE;
            attachments[1].initial_state = RHI_RESOURCE_STATE_COMMON;
            attachments[1].final_state = RHI_RESOURCE_STATE_DEPTH_WRITE;

            attachments[2].format = (ERHIFormat)swap_chain->mBackBufferSRVs[0]->get_create_desc().format;
            attachments[2].load_action = RHI_LOAD_ACTION_CLEAR;
            attachments[2].store_action = RHI_STORE_ACTION_STORE;
            attachments[2].initial_state = RHI_RESOURCE_STATE_RENDER_TARGET;
            attachments[2].final_state = RHI_RESOURCE_STATE_RENDER_TARGET;

            constexpr uint32_t num_subpasses = 2;
            RenderObject::RenderSubpassDesc subpasses[num_subpasses] = {};
            // subpass 0 attachments
            RenderObject::AttachmentReference rt_attachment_ref0[] = 
            {
                {0, RHI_RESOURCE_STATE_RENDER_TARGET}
            };

            RenderObject::AttachmentReference depth_attachment_ref0 = {1, RHI_RESOURCE_STATE_DEPTH_WRITE};
            // subpass 1 attachments
            RenderObject::AttachmentReference rt_attachment_ref1[] =
            {
                {2, RHI_RESOURCE_STATE_RENDER_TARGET}
            };
            RenderObject::AttachmentReference depth_attachment_ref1 = { 1, RHI_RESOURCE_STATE_DEPTH_WRITE };
            RenderObject::AttachmentReference input_attachment_ref1[] = 
            {
                { 0, RHI_RESOURCE_STATE_SHADER_RESOURCE }
            };

            subpasses[0].render_target_count = _countof(rt_attachment_ref0);
            subpasses[0].render_target_attachments = rt_attachment_ref0;
            subpasses[0].depth_stencil_attachment = &depth_attachment_ref0;

            subpasses[1].render_target_count = _countof(rt_attachment_ref1);
            subpasses[1].render_target_attachments = rt_attachment_ref1;
            subpasses[1].depth_stencil_attachment = &depth_attachment_ref1;
            subpasses[1].input_attachment_count = _countof(input_attachment_ref1);
            subpasses[1].input_attachments = input_attachment_ref1;

            RenderObject::RenderPassDesc renderpass_desc;
            renderpass_desc.attachments = attachments;
            renderpass_desc.attachment_count = num_attachments;
            renderpass_desc.subpass_count = num_subpasses;
            renderpass_desc.subpasses = subpasses;

            render_pass = device->create_render_pass(renderpass_desc);
            cyber_assert(render_pass != nullptr, "create render pass failed");
        }

        void RenderPassApp::create_resource()
        {
            const auto& rp_desc = render_pass->get_create_desc();
            
            // Create color texture
            RenderObject::TextureCreateDesc texture_desc = {
                .name = CYBER_UTF8("color"),
                .width = getWindow()->getWidth(),
                .height = getWindow()->getHeight(),
                .depth = 1,
                .array_size = 1,
                .mip_levels = 1,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.000000000f },
                .descriptors = RHI_DESCRIPTOR_TYPE_UNDEFINED,
                .format = rp_desc.attachments[0].format,
                .start_state = RHI_RESOURCE_STATE_RENDER_TARGET | RHI_RESOURCE_STATE_SHADER_RESOURCE,
            };

            if(gbuffer.base_color_texture != nullptr)
                gbuffer.base_color_texture = device->create_texture(texture_desc);

            // Create depth texture
            texture_desc.name = CYBER_UTF8("depth zbuffer");
            texture_desc.format = rp_desc.attachments[1].format;
            texture_desc.clear_value = {1.0f, 1.0f, 1.0f, 1.0f};

            if(gbuffer.depth_texture != nullptr)
                gbuffer.depth_texture = device->create_texture(texture_desc);

            // Create final color texture
            texture_desc.name = CYBER_UTF8("final color");
            texture_desc.format = rp_desc.attachments[2].format;
            texture_desc.clear_value = { 0.0f, 0.0f, 0.0f, 1.0f };

            if(gbuffer.final_color_texture != nullptr)
                gbuffer.final_color_texture = device->create_texture(texture_desc);

            
            RenderObject::TextureViewCreateDesc view_desc = {
                .name = CYBER_UTF8("color_view"),
                .texture = gbuffer.base_color_texture,
                .format = ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM,
                .usages = RHI_TVU_RTV_DSV,
                .aspects = RHI_TVA_COLOR,
                .dimension = RHI_TEX_DIMENSION_2D,
                .array_layer_count = 1
            };

            auto base_color_tex_view = device->create_texture_view(view_desc);

            RenderObject::FrameBuffserDesc frame_buffer_desc = {
                .name = CYBER_UTF8("frame_buffer"),
                .render_pass = nullptr,
                .attachment_count = 1,
                .attachments = base_color_tex_view
            };
            frame_buffer = device->create_frame_buffer(frame_buffer_desc);
            
        }

        void RenderPassApp::raster_draw()
        {
            RHIAcquireNextDesc acquire_desc = {
                .fence = present_fence
            };
            backbuffer_index = device->acquire_next_image(swap_chain, acquire_desc);
            auto back_buffer = swap_chain->mBackBufferSRVs[backbuffer_index];
            auto back_buffer_view = swap_chain->mBackBufferSRVViews[backbuffer_index];
            auto back_depth_buffer_view = swap_chain->mBackBufferDSVView;
            device->reset_command_pool(pool);
            // record
            device->cmd_begin(cmd);
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

            RenderObject::RenderPassDesc rp_desc = {
                /*
                .sample_count = RHI_SAMPLE_COUNT_1,
                .color_attachments = &screen_attachment,
                .depth_stencil_attachment = &depth_attachment,
                .render_target_count = 1,
                */
            };
            RHITextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = RHI_RESOURCE_STATE_PRESENT,
                .dst_state = RHI_RESOURCE_STATE_RENDER_TARGET
            };

            RHIResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            device->cmd_resource_barrier(cmd, barrier_desc0);
            RHIRenderPassEncoder* rp_encoder = device->cmd_begin_render_pass(cmd, rp_desc);
            device->render_encoder_set_viewport(rp_encoder, 0, 0, back_buffer->get_create_desc().width, back_buffer->get_create_desc().height, 0.0f, 1.0f);
            device->render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->get_create_desc().width, back_buffer->get_create_desc().height);
            device->render_encoder_bind_pipeline(rp_encoder, pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            device->render_encoder_draw(rp_encoder, 3, 0);
            device->cmd_end_render_pass(cmd);

            screen_attachment.load_action = RHI_LOAD_ACTION_LOAD;
            depth_attachment.depth_load_action = RHI_LOAD_ACTION_LOAD;
            depth_attachment.stencil_load_action = RHI_LOAD_ACTION_LOAD;
            RenderObject::RenderPassDesc ui_rp_desc = {
                //.sample_count = RHI_SAMPLE_COUNT_1,
                //.color_attachments = &screen_attachment,
                //.depth_stencil_attachment = &depth_attachment,
                //.render_target_count = 1,
            };

            RHIRenderPassEncoder* rp_ui_encoder = device->cmd_begin_render_pass(cmd, ui_rp_desc);
            device->render_encoder_set_viewport(rp_ui_encoder, 0, 0, back_buffer->get_create_desc().width, back_buffer->get_create_desc().height, 0.0f, 1.0f);
            device->render_encoder_set_scissor(rp_ui_encoder, 0, 0, back_buffer->get_create_desc().width, back_buffer->get_create_desc().height);
            device->cmd_end_render_pass(cmd);

            RHITextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = RHI_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = RHI_RESOURCE_STATE_PRESENT
            };
            RHIResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            device->cmd_resource_barrier(cmd, barrier_desc2);
            device->cmd_end(cmd);

            // submit
            RHIQueueSubmitDesc submit_desc = {
                .pCmds = &cmd,
                .mSignalFence = present_fence,
                .mCmdsCount = 1
            };
            device->submit_queue(queue, submit_desc);

            // present
            RHIQueuePresentDesc present_desc = {
                .swap_chain = swap_chain,
                .wait_semaphores = nullptr,
                .wait_semaphore_count = 0,
                .index = backbuffer_index,
            };
            device->present_queue(queue, present_desc);

            // sync & reset
            device->wait_fences(&present_fence, 1);
        }

        void RenderPassApp::finalize()
        {

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
    Cyber::Log::initLog();
    CB_CORE_INFO("initLog");

    Cyber::WindowDesc desc;
    desc.title = L"Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 720;
    desc.hInstance = hInstance;
    desc.cmdShow = nCmdShow;
    Cyber::Samples::RenderPassApp app;
    app.initialize(desc);
    app.run();

    FreeConsole();
    return 1;
}