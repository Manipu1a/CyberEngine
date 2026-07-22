#include "graphics/features/shadow.h"

#include "graphics/interface/device_context.h"
#include "graphics/interface/frame_buffer.h"
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/render_pass.h"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "graphics/rendergraph/render_graph.h"
#include "graphics/rendergraph/render_graph_builder.h"

namespace Cyber::Renderer
{
    ShadowPass::ShadowPass(const Resources& pass_resources, ForwardPassContext* context)
        : ForwardRenderPass(context)
        , resources(pass_resources)
    {
        create_pipeline();
        create_render_pass();
        create_frame_buffer();
    }

    void ShadowPass::setup(render_graph::RenderGraphBuilder&)
    {
        set_depthstencil(resources.shadow_map, LOAD_ACTION_CLEAR, STORE_ACTION_STORE);
    }

    void ShadowPass::create_pipeline()
    {
        if (!pass_context || !pass_context->pipeline_cache ||
            !resources.shadow_map || !resources.shadow_map->texture)
            return;

        pipeline = pass_context->pipeline_cache->get_depth_only(
            resources.shadow_map->texture->get_create_desc().m_format);
    }

    void ShadowPass::create_render_pass()
    {
        if (!pass_context || !pass_context->command_context ||
            !resources.shadow_map || !resources.shadow_map->texture)
            return;

        RenderObject::RenderPassAttachmentDesc shadow_attachment = {};
        shadow_attachment.format = resources.shadow_map->texture->get_create_desc().m_format;
        shadow_attachment.sample_count = SAMPLE_COUNT_1;
        shadow_attachment.load_action = LOAD_ACTION_CLEAR;
        shadow_attachment.store_action = STORE_ACTION_STORE;
        shadow_attachment.initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
        shadow_attachment.final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

        RenderObject::AttachmentReference shadow_ref = { 0, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };
        RenderObject::RenderSubpassDesc subpass = {};
        subpass.m_name = u8"Forward Shadow";
        subpass.m_pDepthStencilAttachment = &shadow_ref;

        RenderObject::RenderPassDesc pass_desc = {};
        pass_desc.m_name = u8"Forward Shadow Pass";
        pass_desc.m_attachmentCount = 1;
        pass_desc.m_pAttachments = &shadow_attachment;
        pass_desc.m_subpassCount = 1;
        pass_desc.m_pSubpasses = &subpass;
        pass_context->command_context->create_render_pass(pass_desc, &render_pass);
    }

    void ShadowPass::create_frame_buffer()
    {
        if (!pass_context || !pass_context->device || !resources.shadow_map || !resources.shadow_map->texture)
            return;

        auto* shadow_view = resources.shadow_map->texture->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
        RenderObject::ITexture_View* attachments[1] = { shadow_view };
        RenderObject::FrameBufferDesc frame_buffer_desc = {};
        frame_buffer_desc.m_name = u8"Forward_ShadowFrameBuffer";
        frame_buffer_desc.m_attachmentCount = 1;
        frame_buffer_desc.m_ppAttachments = attachments;
        frame_buffer = pass_context->device->create_frame_buffer(frame_buffer_desc);
    }

    void ShadowPass::execute(render_graph::RenderGraph&, render_graph::RenderPassContext&)
    {
        if (!pass_context || !pass_context->command_context || !render_pass || !frame_buffer ||
            !resources.shadow_map || !resources.shadow_map->texture)
            return;

        auto* shadow_texture = resources.shadow_map->texture;
        auto* shadow_view = shadow_texture->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
        RenderObject::ITexture_View* attachments[1] = { shadow_view };
        frame_buffer->update_attachments(attachments, 1);

        RenderObject::BeginRenderPassAttribs begin_info = {};
        begin_info.pFramebuffer = frame_buffer;
        begin_info.pRenderPass = render_pass;
        begin_info.depth_stencil_clear_value = { 1.0f, 0 };
        begin_info.TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        pass_context->command_context->set_frame_buffer(frame_buffer);
        pass_context->command_context->cmd_begin_render_pass(begin_info);
        set_default_viewport(pass_context->shadow_resolution, pass_context->shadow_resolution);

        float3 light_dir;
        float3 light_color;
        float light_intensity;
        if (find_main_light(light_dir, light_color, light_intensity))
            draw_depth_only(build_shadow_view_projection(light_dir), pipeline);

        pass_context->command_context->cmd_end_render_pass();
    }
}
