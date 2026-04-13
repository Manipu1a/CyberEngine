#include "gameruntime/sampleapp.h"
#include "platform/memory.h"
#include "application/application.h"
#include "renderer/renderer.h"
#include "graphics/interface/device_context.h"
#include "graphics/interface/swap_chain.hpp"
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/render_pass.h"
#include "graphics/interface/frame_buffer.h"
#include "graphics/interface/buffer.h"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "graphics/interface/sampler.h"
#include "resource/resource_loader.h"

namespace Cyber
{
    namespace Samples
    {
        SampleApp::SampleApp()
        {
        }

        SampleApp::~SampleApp()
        {
        }

        void SampleApp::initialize()
        {
            m_pApp = Core::Application::getApp();
            cyber_check(m_pApp);
            on_create_gfx_objects();
            on_create_pipelines();
            on_create_resources();
        }

        void SampleApp::run()
        {
        }

        void SampleApp::update(float deltaTime)
        {
        }

        void SampleApp::present()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            auto swap_chain = renderer->get_swap_chain();

            device_context->cmd_end();
            device_context->flush();
            render_device->present(swap_chain);
        }

        // --- Convenience accessors ---

        Renderer::Renderer* SampleApp::get_renderer() const
        {
            return m_pApp->get_renderer();
        }

        RenderObject::IRenderDevice* SampleApp::get_render_device() const
        {
            return m_pApp->get_renderer()->get_render_device();
        }

        RenderObject::IDeviceContext* SampleApp::get_device_context() const
        {
            return m_pApp->get_renderer()->get_device_context();
        }

        // --- Frame rendering helpers ---

        FrameContext SampleApp::begin_frame(
            RenderObject::IRenderPass* render_pass,
            const GRAPHICS_CLEAR_VALUE& clear_color)
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
            auto back_buffer_view = back_buffer->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
            auto back_depth_buffer_view = back_depth_buffer->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);

            device_context->cmd_begin();

            RenderObject::ITexture_View* attachment_resources[2] = { back_buffer_view, back_depth_buffer_view };
            frame_buffer->update_attachments(attachment_resources, 2);

            GRAPHICS_CLEAR_VALUE clear_value = clear_color;
            RenderObject::BeginRenderPassAttribs render_pass_begin_info
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
                .ClearValueCount = 1,
                .color_clear_values = &clear_value,
                .depth_stencil_clear_value = { 1.0f, 0 },
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            device_context->cmd_resource_barrier(back_buffer, GRAPHICS_RESOURCE_STATE_UNKNOWN, GRAPHICS_RESOURCE_STATE_RENDER_TARGET);
            device_context->cmd_resource_barrier(back_depth_buffer, GRAPHICS_RESOURCE_STATE_UNKNOWN, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE);

            device_context->cmd_begin_render_pass(render_pass_begin_info);
            device_context->set_frame_buffer(frame_buffer);

            RenderObject::Viewport viewport;
            viewport.top_left_x = 0.0f;
            viewport.top_left_y = 0.0f;
            viewport.width = (float)back_buffer->get_create_desc().m_width;
            viewport.height = (float)back_buffer->get_create_desc().m_height;
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;
            device_context->render_encoder_set_viewport(1, &viewport);

            RenderObject::Rect scissor
            {
                0, 0,
                (int32_t)back_buffer->get_create_desc().m_width,
                (int32_t)back_buffer->get_create_desc().m_height
            };
            device_context->render_encoder_set_scissor(1, &scissor);

            FrameContext ctx;
            ctx.render_device = render_device;
            ctx.device_context = device_context;
            ctx.swap_chain = swap_chain;
            ctx.frame_buffer = frame_buffer;
            ctx.color_buffer = back_buffer;
            ctx.depth_buffer = back_depth_buffer;
            ctx.color_view = back_buffer_view;
            ctx.depth_view = back_depth_buffer_view;
            return ctx;
        }

        void SampleApp::end_frame()
        {
            auto device_context = get_device_context();
            device_context->cmd_end_render_pass();
        }

        // --- Resource creation helpers ---

        RefCntAutoPtr<RenderObject::IShaderLibrary> SampleApp::load_shader(const ShaderDesc& desc)
        {
            auto render_device = get_render_device();

            ResourceLoader::ShaderLoadDesc load_desc = {};
            load_desc.target = desc.target;
            load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = desc.path,
                .stage = desc.stage,
                .entry_point_name = desc.entry_point,
            };
            return ResourceLoader::add_shader(render_device, load_desc);
        }

        void SampleApp::create_buffer(
            GRAPHICS_RESOURCE_BIND_FLAGS bind_flags,
            uint32_t size,
            GRAPHICS_RESOURCE_USAGE usage,
            CPU_ACCESS_FLAGS cpu_access,
            const void* initial_data,
            RefCntAutoPtr<RenderObject::IBuffer>& out_buffer)
        {
            auto render_device = get_render_device();

            RenderObject::BufferCreateDesc buffer_desc = {};
            buffer_desc.bind_flags = bind_flags;
            buffer_desc.size = size;
            buffer_desc.usage = usage;
            buffer_desc.cpu_access_flags = cpu_access;

            if (initial_data)
            {
                RenderObject::BufferData buffer_data = {};
                buffer_data.data = initial_data;
                buffer_data.data_size = size;
                render_device->create_buffer(buffer_desc, &buffer_data, &out_buffer);
            }
            else
            {
                render_device->create_buffer(buffer_desc, nullptr, &out_buffer);
            }
        }

        void SampleApp::create_vertex_buffer(const void* data, uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer)
        {
            create_buffer(
                GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER,
                size,
                GRAPHICS_RESOURCE_USAGE_DEFAULT,
                CPU_ACCESS_WRITE,
                data,
                out_buffer);
        }

        void SampleApp::create_index_buffer(const void* data, uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer)
        {
            create_buffer(
                GRAPHICS_RESOURCE_BIND_INDEX_BUFFER,
                size,
                GRAPHICS_RESOURCE_USAGE_DEFAULT,
                CPU_ACCESS_WRITE,
                data,
                out_buffer);
        }

        void SampleApp::create_constant_buffer(uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer)
        {
            create_buffer(
                GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER,
                size,
                GRAPHICS_RESOURCE_USAGE_DYNAMIC,
                CPU_ACCESS_WRITE,
                nullptr,
                out_buffer);
        }

        void SampleApp::update_buffer(RenderObject::IBuffer* buffer, const void* data, uint32_t size)
        {
            auto render_device = get_render_device();
            void* mapped = render_device->map_buffer(buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            memcpy(mapped, data, size);
            render_device->unmap_buffer(buffer, MAP_WRITE);
            buffer->set_buffer_size(size);
        }

        void SampleApp::create_default_render_pass(const char8_t* name, RefCntAutoPtr<RenderObject::IRenderPass>& out_render_pass)
        {
            auto renderer = m_pApp->get_renderer();
            auto device_context = renderer->get_device_context();
            auto& scene_target = renderer->get_scene_target(0);

            RenderObject::RenderPassAttachmentDesc attachment_descs[2] = {};

            // Color attachment
            attachment_descs[0].format = scene_target.color_buffer->get_create_desc().m_format;
            attachment_descs[0].sample_count = SAMPLE_COUNT_1;
            attachment_descs[0].load_action = LOAD_ACTION_CLEAR;
            attachment_descs[0].store_action = STORE_ACTION_STORE;
            attachment_descs[0].initial_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_descs[0].final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            // Depth attachment
            attachment_descs[1].format = scene_target.depth_buffer->get_create_desc().m_format;
            attachment_descs[1].sample_count = SAMPLE_COUNT_1;
            attachment_descs[1].load_action = LOAD_ACTION_CLEAR;
            attachment_descs[1].store_action = STORE_ACTION_STORE;
            attachment_descs[1].initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_descs[1].final_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;

            RenderObject::AttachmentReference color_ref = { 0, GRAPHICS_RESOURCE_STATE_RENDER_TARGET };
            RenderObject::AttachmentReference depth_ref = { 1, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };

            RenderObject::RenderSubpassDesc subpass_desc = {};
            subpass_desc.m_name = u8"Main Subpass";
            subpass_desc.m_inputAttachmentCount = 0;
            subpass_desc.m_pInputAttachments = nullptr;
            subpass_desc.m_pDepthStencilAttachment = &depth_ref;
            subpass_desc.m_renderTargetCount = 1;
            subpass_desc.m_pRenderTargetAttachments = &color_ref;

            RenderObject::RenderPassDesc rp_desc = {
                .m_name = name,
                .m_attachmentCount = 2,
                .m_pAttachments = attachment_descs,
                .m_subpassCount = 1,
                .m_pSubpasses = &subpass_desc
            };

            device_context->create_render_pass(rp_desc, &out_render_pass);
        }

        RefCntAutoPtr<RenderObject::ISampler> SampleApp::create_default_sampler()
        {
            auto render_device = get_render_device();

            RenderObject::SamplerCreateDesc sampler_desc = {};
            sampler_desc.min_filter = FILTER_TYPE_LINEAR;
            sampler_desc.mag_filter = FILTER_TYPE_LINEAR;
            sampler_desc.mip_filter = FILTER_TYPE_LINEAR;
            sampler_desc.address_u = ADDRESS_MODE_WRAP;
            sampler_desc.address_v = ADDRESS_MODE_WRAP;
            sampler_desc.address_w = ADDRESS_MODE_WRAP;
            sampler_desc.flags = SAMPLER_FLAG_NONE;
            sampler_desc.unnormalized_coordinates = false;
            sampler_desc.mip_lod_bias = 0.0f;
            sampler_desc.max_anisotropy = 0;
            sampler_desc.compare_mode = CMP_NEVER;
            sampler_desc.border_color = { 0.0f, 0.0f, 0.0f, 0.0f };
            sampler_desc.min_lod = 0.0f;
            sampler_desc.max_lod = 0.0f;
            return RefCntAutoPtr<RenderObject::ISampler>(render_device->create_sampler(sampler_desc));
        }
    }
}
