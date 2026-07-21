#include "graphics/features/scene_color.h"

#include "graphics/interface/device_context.h"
#include "graphics/interface/frame_buffer.h"
#include "graphics/interface/render_pass.h"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "graphics/rendergraph/render_graph.h"
#include "graphics/rendergraph/render_graph_builder.h"

namespace Cyber::Renderer
{
    SceneColorPass::SceneColorPass(const Resources& pass_resources, ForwardPassContext* context)
        : ForwardRenderPass(context)
        , resources(pass_resources)
    {
        create_render_pass();
    }

    void SceneColorPass::setup(render_graph::RenderGraphBuilder&)
    {
        add_input(u8"ShadowMap", resources.shadow_map);
        set_depthstencil_read_only(resources.depth);
        add_render_target(0, resources.color, LOAD_ACTION_CLEAR,
            { 0.690196097f, 0.768627524f, 0.870588303f, 1.0f }, STORE_ACTION_STORE);
    }

    void SceneColorPass::create_render_pass()
    {
        if (!pass_context || !pass_context->command_context || !resources.color || !resources.color->texture ||
            !resources.depth || !resources.depth->texture)
            return;

        RenderObject::RenderPassAttachmentDesc attachments[2] = {};
        attachments[0].format = resources.color->texture->get_create_desc().m_format;
        attachments[0].sample_count = SAMPLE_COUNT_1;
        attachments[0].load_action = LOAD_ACTION_CLEAR;
        attachments[0].store_action = STORE_ACTION_STORE;
        attachments[0].initial_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
        attachments[0].final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

        attachments[1].format = resources.depth->texture->get_create_desc().m_format;
        attachments[1].sample_count = SAMPLE_COUNT_1;
        attachments[1].load_action = LOAD_ACTION_LOAD;
        attachments[1].store_action = STORE_ACTION_STORE;
        attachments[1].initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_READ;
        attachments[1].final_state = GRAPHICS_RESOURCE_STATE_DEPTH_READ;

        RenderObject::AttachmentReference color_ref = { 0, GRAPHICS_RESOURCE_STATE_RENDER_TARGET };
        RenderObject::AttachmentReference depth_ref = { 1, GRAPHICS_RESOURCE_STATE_DEPTH_READ };
        RenderObject::RenderSubpassDesc subpass = {};
        subpass.m_name = u8"Forward Color";
        subpass.m_pDepthStencilAttachment = &depth_ref;
        subpass.m_renderTargetCount = 1;
        subpass.m_pRenderTargetAttachments = &color_ref;

        RenderObject::RenderPassDesc pass_desc = {};
        pass_desc.m_name = u8"Forward Color Pass";
        pass_desc.m_attachmentCount = 2;
        pass_desc.m_pAttachments = attachments;
        pass_desc.m_subpassCount = 1;
        pass_desc.m_pSubpasses = &subpass;
        pass_context->command_context->create_render_pass(pass_desc, &render_pass);
    }

    void SceneColorPass::execute(render_graph::RenderGraph&, render_graph::RenderPassContext&)
    {
        if (!pass_context || !pass_context->command_context || !render_pass ||
            !resources.color || !resources.color->texture || !resources.depth || !resources.depth->texture ||
            !pass_context->frame.frame_buffer)
            return;

        auto* color_texture = resources.color->texture;
        auto* depth_texture = resources.depth->texture;
        auto* color_view = color_texture->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
        auto* depth_view = depth_texture->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
        RenderObject::ITexture_View* attachments[2] = { color_view, depth_view };
        pass_context->frame.frame_buffer->update_attachments(attachments, 2);

        GRAPHICS_CLEAR_VALUE clear_value = { 0.690196097f, 0.768627524f, 0.870588303f, 1.0f };
        RenderObject::BeginRenderPassAttribs begin_info = {};
        begin_info.pFramebuffer = pass_context->frame.frame_buffer;
        begin_info.pRenderPass = render_pass;
        begin_info.ClearValueCount = 1;
        begin_info.color_clear_values = &clear_value;
        begin_info.depth_stencil_clear_value = { 1.0f, 0 };
        begin_info.TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        pass_context->command_context->set_frame_buffer(pass_context->frame.frame_buffer);
        pass_context->command_context->cmd_begin_render_pass(begin_info);
        set_default_viewport(color_texture->get_create_desc().m_width, color_texture->get_create_desc().m_height);

        float4x4 view_proj = float4x4::Identity();
        float3 eye = float3(0.0f, 0.0f, 0.0f);
        float3 light_dir = float3(-0.3f, -1.0f, -0.2f);
        float3 light_color = float3(1.0f, 1.0f, 1.0f);
        float light_intensity = 1.0f;

        if (find_scene_view(view_proj, eye))
        {
            find_main_light(light_dir, light_color, light_intensity);
            draw_color(view_proj, eye, light_dir, light_color, light_intensity);
        }

        pass_context->command_context->cmd_end_render_pass();
    }
}
