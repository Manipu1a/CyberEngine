#include "graphics/features/scene_color.h"

#include "gameruntime/pipeline_builder.h"
#include "graphics/interface/device_context.h"
#include "graphics/interface/frame_buffer.h"
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/render_pass.h"
#include "graphics/interface/sampler.h"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "graphics/rendergraph/render_graph.h"
#include "graphics/rendergraph/render_graph_builder.h"

#include <cstddef>

namespace Cyber::Renderer
{
    namespace
    {
        struct ForwardVertex
        {
            float3 position;
            float3 normal;
            float2 uv;
        };
    }

    SceneColorPass::SceneColorPass(const Resources& pass_resources, ForwardPassContext* context)
        : ForwardRenderPass(context)
        , resources(pass_resources)
    {
        create_resources();
        create_pipeline();
        create_render_pass();
    }

    void SceneColorPass::setup(render_graph::RenderGraphBuilder&)
    {
        add_input(u8"ShadowMap", resources.shadow_map);
        set_depthstencil_read_only(resources.depth);
        add_render_target(0, resources.color, LOAD_ACTION_CLEAR,
            { 0.690196097f, 0.768627524f, 0.870588303f, 1.0f }, STORE_ACTION_STORE);
    }

    void SceneColorPass::create_resources()
    {
        if (!pass_context || !pass_context->device)
            return;

        const uint8_t white_pixel[] = { 255, 255, 255, 255 };
        RenderObject::TextureSubResData white_subresource = {};
        white_subresource.pData = white_pixel;
        white_subresource.stride = sizeof(white_pixel);
        white_subresource.depthStride = sizeof(white_pixel);

        RenderObject::TextureData white_data = {};
        white_data.pSubResources = &white_subresource;
        white_data.numSubResources = 1;

        RenderObject::TextureCreateDesc white_desc = {};
        white_desc.m_name = u8"Forward_WhiteTexture";
        white_desc.m_width = 1;
        white_desc.m_height = 1;
        white_desc.m_depth = 1;
        white_desc.m_arraySize = 1;
        white_desc.m_mipLevels = 1;
        white_desc.m_dimension = TEX_DIMENSION_2D;
        white_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
        white_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
        white_desc.m_flags = TCF_FORCE_2D;
        white_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
        RenderObject::ITexture* raw_white_texture = nullptr;
        pass_context->device->create_texture(white_desc, &white_data, &raw_white_texture);
        white_texture.attach(raw_white_texture);

        RenderObject::SamplerCreateDesc sampler_desc = {};
        sampler_desc.min_filter = FILTER_TYPE_LINEAR;
        sampler_desc.mag_filter = FILTER_TYPE_LINEAR;
        sampler_desc.mip_filter = FILTER_TYPE_LINEAR;
        sampler_desc.address_u = ADDRESS_MODE_WRAP;
        sampler_desc.address_v = ADDRESS_MODE_WRAP;
        sampler_desc.address_w = ADDRESS_MODE_WRAP;
        sampler_desc.compare_mode = CMP_NEVER;
        sampler = RefCntAutoPtr<RenderObject::ISampler>(pass_context->device->create_sampler(sampler_desc));
    }

    void SceneColorPass::create_pipeline()
    {
        if (!pass_context || !pass_context->device || !resources.color || !resources.color->texture ||
            !resources.depth || !resources.depth->texture || !sampler)
            return;

        RenderObject::VertexAttribute vertex_attributes[] = {
            {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, position)},
            {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, normal)},
            {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, uv)},
        };

        pipeline = PipelineBuilder(pass_context->device)
            .vertex_shader(CYBER_UTF8("shaders/DX12/forward_color_vs.hlsl"))
            .pixel_shader(CYBER_UTF8("shaders/DX12/forward_color_ps.hlsl"))
            .vertex_layout(vertex_attributes, 3)
            .static_sampler(CYBER_UTF8("Texture_sampler"), sampler)
            .blend_opaque()
            .depth_test(true, false, CMP_LESS_EQUAL)
            .render_target_format(resources.color->texture->get_create_desc().m_format)
            .depth_format(resources.depth->texture->get_create_desc().m_format)
            .build();
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
            draw_color(view_proj, eye, light_dir, light_color, light_intensity, pipeline, white_texture);
        }

        pass_context->command_context->cmd_end_render_pass();
    }
}
