#include "graphics/features/pre_depth.h"

#include "graphics/interface/device_context.h"
#include "graphics/interface/frame_buffer.h"
#include "graphics/interface/render_pass.h"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "graphics/rendergraph/render_graph.h"
#include "graphics/rendergraph/render_graph_builder.h"

namespace Cyber::Renderer
{
    PreDepthPass::PreDepthPass(const Resources& pass_resources, ForwardPassContext* context)
        : ForwardRenderPass(context)
        , resources(pass_resources)
    {
        create_pipeline();
        create_render_pass();
    }

    void PreDepthPass::setup(render_graph::RenderGraphBuilder&)
    {
        set_depthstencil(resources.depth, LOAD_ACTION_CLEAR, STORE_ACTION_STORE);
    }

    void PreDepthPass::create_pipeline()
    {
        if (!pass_context || !pass_context->pipeline_cache || !resources.depth || !resources.depth->texture)
            return;

        pipeline = pass_context->pipeline_cache->get_depth_only(
            resources.depth->texture->get_create_desc().m_format);
    }

    void PreDepthPass::create_render_pass()
    {
        if (!pass_context || !pass_context->command_context || !resources.depth || !resources.depth->texture)
            return;

        RenderObject::RenderPassAttachmentDesc depth_attachment = {};
        depth_attachment.format = resources.depth->texture->get_create_desc().m_format;
        depth_attachment.sample_count = SAMPLE_COUNT_1;
        depth_attachment.load_action = LOAD_ACTION_CLEAR;
        depth_attachment.store_action = STORE_ACTION_STORE;
        depth_attachment.initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
        depth_attachment.final_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;

        RenderObject::AttachmentReference depth_ref = { 0, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };
        RenderObject::RenderSubpassDesc subpass = {};
        subpass.m_name = u8"Forward PreDepth";
        subpass.m_pDepthStencilAttachment = &depth_ref;

        RenderObject::RenderPassDesc pass_desc = {};
        pass_desc.m_name = u8"Forward PreDepth Pass";
        pass_desc.m_attachmentCount = 1;
        pass_desc.m_pAttachments = &depth_attachment;
        pass_desc.m_subpassCount = 1;
        pass_desc.m_pSubpasses = &subpass;
        pass_context->command_context->create_render_pass(pass_desc, &render_pass);
    }

    void PreDepthPass::execute(render_graph::RenderGraph&, render_graph::RenderPassContext&)
    {
        if (!pass_context || !pass_context->command_context || !render_pass ||
            !resources.depth || !resources.depth->texture || !pass_context->frame.frame_buffer)
            return;

        auto* depth_texture = resources.depth->texture;
        auto* depth_view = depth_texture->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
        RenderObject::ITexture_View* attachments[1] = { depth_view };
        pass_context->frame.frame_buffer->update_attachments(attachments, 1);

        RenderObject::BeginRenderPassAttribs begin_info = {};
        begin_info.pFramebuffer = pass_context->frame.frame_buffer;
        begin_info.pRenderPass = render_pass;
        begin_info.depth_stencil_clear_value = { 1.0f, 0 };
        begin_info.TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        pass_context->command_context->set_frame_buffer(pass_context->frame.frame_buffer);
        pass_context->command_context->cmd_begin_render_pass(begin_info);
        set_default_viewport(depth_texture->get_create_desc().m_width, depth_texture->get_create_desc().m_height);

        float4x4 view_proj = float4x4::Identity();
        float3 eye = float3(0.0f, 0.0f, 0.0f);
        if (find_scene_view(view_proj, eye))
            draw_depth_only(view_proj, pipeline);

        pass_context->command_context->cmd_end_render_pass();
    }
}
