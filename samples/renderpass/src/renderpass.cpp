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
            auto backBufferSRV = (RenderObject::ITexture_View**)cyber_malloc(sizeof(RenderObject::ITexture_View*) * m_pSwapChain->get_buffer_srv_count());
            m_pSwapChain->set_back_buffer_srv_views(backBufferSRV); 
            for(uint32_t i = 0; i < m_pSwapChain->get_buffer_srv_count(); ++i)
            {
                eastl::basic_string<char8_t> swap_chain_name(eastl::basic_string<char8_t>::CtorSprintf(), u8"backbuffer_%d", i); // CYBER_UTF8("backbuffer_%d", i
                RenderObject::TextureViewCreateDesc view_desc = {
                    .m_name = swap_chain_name.c_str(),
                    .m_pTexture = m_pSwapChain->get_back_buffer(i),
                    .m_format = (TEXTURE_FORMAT)m_pSwapChain->get_back_buffer(i)->get_create_desc().m_format,
                    .m_usages = TVU_RTV_DSV,
                    .m_aspects = TVA_COLOR,
                    .m_dimension = TEX_DIMENSION_2D,
                    .m_arrayLayerCount = 1
                };
                auto view = m_pRenderDevice->create_texture_view(view_desc);
                m_pSwapChain->set_back_buffer_srv_view(view, i);
            }
            
            m_pRenderDevice->cmd_begin(m_pCmd);
            {
                TextureBarrier depth_barrier = {
                    .texture = m_pSwapChain->get_back_buffer_depth(),
                    .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                    .dst_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE
                };
                ResourceBarrierDesc barrier_desc1 = { .texture_barriers = &depth_barrier, .texture_barrier_count = 1 };
                m_pRenderDevice->cmd_resource_barrier(m_pCmd, barrier_desc1);
            }
            m_pRenderDevice->cmd_end(m_pCmd);

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
            auto color_format = m_pSwapChain->get_back_buffer_srv_view(0)->get_create_desc().m_format;
            RenderObject::RenderPipelineCreateDesc rp_desc = 
            {
                .root_signature = root_signature,
                .vertex_shader = pipeline_shader_create_desc[0],
                .fragment_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout,
                //.rasterizer_state = {},
                .color_formats = &color_format,
                .render_target_count = 1,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
            };
            pipeline = m_pRenderDevice->create_render_pipeline(rp_desc);
            m_pRenderDevice->free_shader_library(vs_shader);
            m_pRenderDevice->free_shader_library(ps_shader);
        }

        void RenderPassApp::create_render_pass()
        {
            // attachment 1 - color
            // attachment 2 - depth
            // attachment 3 - final color
            constexpr uint32_t num_attachments = 3;

            RenderObject::RenderPassAttachmentDesc attachments[num_attachments] = {};
            attachments[0].m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
            attachments[0].m_loadAction = LOAD_ACTION_CLEAR;
            attachments[0].m_storeAction = STORE_ACTION_STORE;
            attachments[0].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachments[0].m_finalState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            
            attachments[1].m_format = TEXTURE_FORMAT_D24_UNORM_S8_UINT;
            attachments[1].m_loadAction = LOAD_ACTION_CLEAR;
            attachments[1].m_storeAction = STORE_ACTION_STORE;
            attachments[1].m_initialState = GRAPHICS_RESOURCE_STATE_COMMON;
            attachments[1].m_finalState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;

            attachments[2].m_format = m_pSwapChain->get_back_buffer_srv_view(0)->get_create_desc().m_format;
            attachments[2].m_loadAction = LOAD_ACTION_CLEAR;
            attachments[2].m_storeAction = STORE_ACTION_STORE;
            attachments[2].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachments[2].m_finalState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;

            constexpr uint32_t num_subpasses = 2;
            RenderObject::RenderSubpassDesc subpasses[num_subpasses] = {};
            // subpass 0 attachments
            RenderObject::AttachmentReference rt_attachment_ref0[] = 
            {
                {0, GRAPHICS_RESOURCE_STATE_RENDER_TARGET}
            };

            RenderObject::AttachmentReference depth_attachment_ref0 = {1, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE};
            // subpass 1 attachments
            RenderObject::AttachmentReference rt_attachment_ref1[] =
            {
                {2, GRAPHICS_RESOURCE_STATE_RENDER_TARGET}
            };
            RenderObject::AttachmentReference depth_attachment_ref1 = { 1, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };
            RenderObject::AttachmentReference input_attachment_ref1[] = 
            {
                { 0, GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE }
            };

            subpasses[0].m_renderTargetCount = _countof(rt_attachment_ref0);
            subpasses[0].m_pRenderTargetAttachments = rt_attachment_ref0;
            subpasses[0].m_pDepthStencilAttachment = &depth_attachment_ref0;

            subpasses[1].m_renderTargetCount = _countof(rt_attachment_ref1);
            subpasses[1].m_pRenderTargetAttachments = rt_attachment_ref1;
            subpasses[1].m_pDepthStencilAttachment = &depth_attachment_ref1;
            subpasses[1].m_inputAttachmentCount = _countof(input_attachment_ref1);
            subpasses[1].m_pInputAttachments = input_attachment_ref1;

            RenderObject::RenderPassDesc renderpass_desc;
            renderpass_desc.m_pAttachments = attachments;
            renderpass_desc.m_attachmentCount = num_attachments;
            renderpass_desc.m_subpassCount = num_subpasses;
            renderpass_desc.m_pSubpasses = subpasses;

            render_pass = m_pRenderDevice->create_render_pass(renderpass_desc);
            cyber_assert(render_pass != nullptr, "create render pass failed");
        }

        void RenderPassApp::create_resource()
        {
            const auto& rp_desc = render_pass->get_create_desc();
            
            // Create color texture
            RenderObject::TextureCreateDesc texture_desc = {
                .m_name = CYBER_UTF8("color"),
                .m_width = getWindow()->getWidth(),
                .m_height = getWindow()->getHeight(),
                .m_depth = 1,
                .m_arraySize = 1,
                .m_mipLevels = 1,
                .m_clearValue = { 0.0f, 0.0f, 0.0f, 1.000000000f },
                .m_descriptors = DESCRIPTOR_TYPE_UNDEFINED,
                .m_format = rp_desc.m_pAttachments[0].m_format,
                .m_startState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET | GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE,
            };

            if(gbuffer.base_color_texture != nullptr)
                gbuffer.base_color_texture = m_pRenderDevice->create_texture(texture_desc);

            // Create depth texture
            texture_desc.m_name = CYBER_UTF8("depth zbuffer");
            texture_desc.m_format = rp_desc.m_pAttachments[1].m_format;
            texture_desc.m_clearValue = {1.0f, 1.0f, 1.0f, 1.0f};

            if(gbuffer.depth_texture != nullptr)
                gbuffer.depth_texture = m_pRenderDevice->create_texture(texture_desc);

            // Create final color texture
            texture_desc.m_name = CYBER_UTF8("final color");
            texture_desc.m_format = rp_desc.m_pAttachments[2].m_format;
            texture_desc.m_clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

            if(gbuffer.final_color_texture != nullptr)
                gbuffer.final_color_texture = m_pRenderDevice->create_texture(texture_desc);
            
            RenderObject::TextureViewCreateDesc view_desc = {
                .m_name = CYBER_UTF8("color_view"),
                .m_pTexture = gbuffer.base_color_texture,
                .m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM,
                .m_usages = TVU_RTV_DSV,
                .m_aspects = TVA_COLOR,
                .m_dimension = TEX_DIMENSION_2D,
                .m_arrayLayerCount = 1
            };

            RenderObject::ITexture_View* views[] = 
            {
                gbuffer.base_color_texture->get_default_texture_view(TVU_RTV_DSV),
                gbuffer.depth_texture->get_default_texture_view(TVU_RTV_DSV),
                gbuffer.final_color_texture->get_default_texture_view(TVU_RTV_DSV)
            };

            RenderObject::FrameBuffserDesc frame_buffer_desc = {
                .m_name = CYBER_UTF8("frame_buffer"),
                .m_pRenderPass = nullptr,
                .m_attachmentCount = 3,
                .m_ppAttachments = views
            };
            frame_buffer = m_pRenderDevice->create_frame_buffer(frame_buffer_desc);
        }

        void RenderPassApp::raster_draw()
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

            RenderObject::RenderPassDesc rp_desc = {
                /*
                .sample_count = RHI_SAMPLE_COUNT_1,
                .color_attachments = &screen_attachment,
                .depth_stencil_attachment = &depth_attachment,
                .render_target_count = 1,
                */
            };

            RenderObject::BeginRenderPassAttribs rp_begin_desc = {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
            };

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_PRESENT,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET
            };

            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            m_pRenderDevice->cmd_resource_barrier(m_pCmd, barrier_desc0);
            RenderPassEncoder* rp_encoder = m_pRenderDevice->cmd_begin_render_pass(m_pCmd, rp_begin_desc);
            m_pRenderDevice->render_encoder_set_viewport(rp_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height, 0.0f, 1.0f);
            m_pRenderDevice->render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height);
            m_pRenderDevice->render_encoder_bind_pipeline(rp_encoder, pipeline);
            //rhi_render_encoder_bind_vertex_buffer(rp_encoder, 1, );
            m_pRenderDevice->render_encoder_draw(rp_encoder, 3, 0);
            m_pRenderDevice->cmd_end_render_pass(m_pCmd);

            screen_attachment.load_action = LOAD_ACTION_LOAD;
            depth_attachment.depth_load_action = LOAD_ACTION_LOAD;
            depth_attachment.stencil_load_action = LOAD_ACTION_LOAD;

            RenderObject::RenderPassDesc ui_rp_desc = {
                //.sample_count = RHI_SAMPLE_COUNT_1,
                //.color_attachments = &screen_attachment,
                //.depth_stencil_attachment = &depth_attachment,
                //.render_target_count = 1,
            };

            RenderObject::BeginRenderPassAttribs ui_rp_begin_desc = {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
            };

            RenderPassEncoder* rp_ui_encoder = m_pRenderDevice->cmd_begin_render_pass(m_pCmd, ui_rp_begin_desc);
            m_pRenderDevice->render_encoder_set_viewport(rp_ui_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height, 0.0f, 1.0f);
            m_pRenderDevice->render_encoder_set_scissor(rp_ui_encoder, 0, 0, back_buffer->get_create_desc().m_width, back_buffer->get_create_desc().m_height);
            m_pRenderDevice->cmd_end_render_pass(m_pCmd);

            TextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GRAPHICS_RESOURCE_STATE_PRESENT
            };
            ResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            m_pRenderDevice->cmd_resource_barrier(m_pCmd, barrier_desc2);
            m_pRenderDevice->cmd_end(m_pCmd);

            // submit
            RenderObject::QueueSubmitDesc submit_desc = {
                .m_ppCmds = &m_pCmd,
                .m_pSignalFence = m_pPresentFence,
                .m_cmdsCount = 1
            };

            // present
            RenderObject::QueuePresentDesc present_desc = {
                .m_pSwapChain = m_pSwapChain,
                .m_ppwaitSemaphores = nullptr,
                .m_waitSemaphoreCount = 0,
                .m_index = m_backBufferIndex,
            };
            m_pRenderDevice->present_queue(m_pQueue, present_desc);

            // sync & reset
            m_pRenderDevice->wait_fences(&m_pPresentFence, 1);
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