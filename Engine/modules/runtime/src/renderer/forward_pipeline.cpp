#include "renderer/forward_pipeline.h"

#include "component/camera_component.h"
#include "component/directional_light_component.h"
#include "component/mesh_component.h"
#include "gameruntime/pipeline_builder.h"
#include "gameruntime/world.h"
#include "graphics/interface/buffer.h"
#include "graphics/interface/device_context.h"
#include "graphics/interface/frame_buffer.h"
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/render_pass.h"
#include "graphics/interface/sampler.h"
#include "graphics/interface/swap_chain.hpp"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "renderer/renderer.h"

#include <cstddef>
#include <cstring>

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

        void create_constant_buffer(RenderObject::IRenderDevice* device, uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer)
        {
            RenderObject::BufferCreateDesc desc = {};
            desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            desc.size = size;
            desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            desc.cpu_access_flags = CPU_ACCESS_WRITE;
            device->create_buffer(desc, nullptr, &out_buffer);
        }
    }

    ForwardPipeline::ForwardPipeline(Renderer* renderer)
        : m_renderer(renderer)
    {
    }

    ForwardPipeline::~ForwardPipeline() = default;

    void ForwardPipeline::initialize()
    {
        m_device = m_renderer->get_render_device();
        m_context = m_renderer->get_device_context();

        create_resources();
        create_render_passes();
        create_pipelines();
    }

    void ForwardPipeline::resize(uint32_t width, uint32_t height)
    {
        (void)width;
        (void)height;
        create_render_passes();
    }

    void ForwardPipeline::render(World* world, float delta_time)
    {
        (void)delta_time;
        if (!m_device || !m_context)
            return;

        ForwardFrameContext frame_context = begin_frame();
        execute_predepth_pass(world, frame_context);
        execute_shadow_pass(world);
        execute_color_pass(world, frame_context);
    }

    void ForwardPipeline::create_resources()
    {
        create_constant_buffer(m_device, sizeof(SceneConstants), m_scene_constants);

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
        m_device->create_texture(white_desc, &white_data, &raw_white_texture);
        m_white_texture.attach(raw_white_texture);

        RenderObject::SamplerCreateDesc sampler_desc = {};
        sampler_desc.min_filter = FILTER_TYPE_LINEAR;
        sampler_desc.mag_filter = FILTER_TYPE_LINEAR;
        sampler_desc.mip_filter = FILTER_TYPE_LINEAR;
        sampler_desc.address_u = ADDRESS_MODE_WRAP;
        sampler_desc.address_v = ADDRESS_MODE_WRAP;
        sampler_desc.address_w = ADDRESS_MODE_WRAP;
        sampler_desc.compare_mode = CMP_NEVER;
        m_sampler = RefCntAutoPtr<RenderObject::ISampler>(m_device->create_sampler(sampler_desc));

        RenderObject::TextureCreateDesc shadow_desc = {};
        shadow_desc.m_name = u8"Forward_ShadowMap";
        shadow_desc.m_format = TEX_FORMAT_D32_FLOAT;
        shadow_desc.m_width = m_shadow_resolution;
        shadow_desc.m_height = m_shadow_resolution;
        shadow_desc.m_depth = 1;
        shadow_desc.m_arraySize = 1;
        shadow_desc.m_mipLevels = 1;
        shadow_desc.m_dimension = TEX_DIMENSION_2D;
        shadow_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
        shadow_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL | GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
        shadow_desc.m_clearValue = fastclear_0000;
        RenderObject::ITexture* raw_shadow = nullptr;
        m_device->create_texture(shadow_desc, nullptr, &raw_shadow);
        m_shadow_map.attach(raw_shadow);

        auto* shadow_dsv = m_shadow_map->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
        RenderObject::ITexture_View* shadow_attachments[1] = { shadow_dsv };
        RenderObject::FrameBufferDesc shadow_fb_desc = {};
        shadow_fb_desc.m_name = u8"Forward_ShadowFrameBuffer";
        shadow_fb_desc.m_attachmentCount = 1;
        shadow_fb_desc.m_ppAttachments = shadow_attachments;
        m_shadow_frame_buffer = m_device->create_frame_buffer(shadow_fb_desc);
    }

    void ForwardPipeline::create_render_passes()
    {
        auto& scene_target = m_renderer->get_scene_target(0);
        const auto color_format = scene_target.color_buffer->get_create_desc().m_format;
        const auto depth_format = scene_target.depth_buffer->get_create_desc().m_format;

        {
            RenderObject::RenderPassAttachmentDesc depth_attachment = {};
            depth_attachment.format = depth_format;
            depth_attachment.sample_count = SAMPLE_COUNT_1;
            depth_attachment.load_action = LOAD_ACTION_CLEAR;
            depth_attachment.store_action = STORE_ACTION_STORE;
            depth_attachment.initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            depth_attachment.final_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;

            RenderObject::AttachmentReference depth_ref = { 0, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };
            RenderObject::RenderSubpassDesc subpass = {};
            subpass.m_name = u8"Forward PreDepth";
            subpass.m_pDepthStencilAttachment = &depth_ref;
            subpass.m_renderTargetCount = 0;
            subpass.m_pRenderTargetAttachments = nullptr;

            RenderObject::RenderPassDesc pass_desc = {};
            pass_desc.m_name = u8"Forward PreDepth Pass";
            pass_desc.m_attachmentCount = 1;
            pass_desc.m_pAttachments = &depth_attachment;
            pass_desc.m_subpassCount = 1;
            pass_desc.m_pSubpasses = &subpass;
            m_context->create_render_pass(pass_desc, &m_predepth_pass);
        }

        {
            RenderObject::RenderPassAttachmentDesc shadow_attachment = {};
            shadow_attachment.format = m_shadow_map->get_create_desc().m_format;
            shadow_attachment.sample_count = SAMPLE_COUNT_1;
            shadow_attachment.load_action = LOAD_ACTION_CLEAR;
            shadow_attachment.store_action = STORE_ACTION_STORE;
            shadow_attachment.initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            shadow_attachment.final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            RenderObject::AttachmentReference shadow_ref = { 0, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };
            RenderObject::RenderSubpassDesc subpass = {};
            subpass.m_name = u8"Forward Shadow";
            subpass.m_pDepthStencilAttachment = &shadow_ref;
            subpass.m_renderTargetCount = 0;
            subpass.m_pRenderTargetAttachments = nullptr;

            RenderObject::RenderPassDesc pass_desc = {};
            pass_desc.m_name = u8"Forward Shadow Pass";
            pass_desc.m_attachmentCount = 1;
            pass_desc.m_pAttachments = &shadow_attachment;
            pass_desc.m_subpassCount = 1;
            pass_desc.m_pSubpasses = &subpass;
            m_context->create_render_pass(pass_desc, &m_shadow_pass);
        }

        {
            RenderObject::RenderPassAttachmentDesc attachments[2] = {};
            attachments[0].format = color_format;
            attachments[0].sample_count = SAMPLE_COUNT_1;
            attachments[0].load_action = LOAD_ACTION_CLEAR;
            attachments[0].store_action = STORE_ACTION_STORE;
            attachments[0].initial_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachments[0].final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            attachments[1].format = depth_format;
            attachments[1].sample_count = SAMPLE_COUNT_1;
            attachments[1].load_action = LOAD_ACTION_LOAD;
            attachments[1].store_action = STORE_ACTION_STORE;
            attachments[1].initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachments[1].final_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;

            RenderObject::AttachmentReference color_ref = { 0, GRAPHICS_RESOURCE_STATE_RENDER_TARGET };
            RenderObject::AttachmentReference depth_ref = { 1, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };
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
            m_context->create_render_pass(pass_desc, &m_color_pass);
        }
    }

    void ForwardPipeline::create_pipelines()
    {
        auto& scene_target = m_renderer->get_scene_target(0);

        RenderObject::VertexAttribute vertex_attributes[] = {
            {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, position)},
            {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, normal)},
            {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, uv)},
        };

        m_depth_pipeline = PipelineBuilder(m_device)
            .vertex_shader(CYBER_UTF8("shaders/DX12/forward_depth_vs.hlsl"))
            .vertex_layout(vertex_attributes, 3)
            .blend_opaque()
            .depth_test(true, true, CMP_LESS_EQUAL)
            .render_target_count(0)
            .depth_format(scene_target.depth_buffer->get_create_desc().m_format)
            .build();

        m_color_pipeline = PipelineBuilder(m_device)
            .vertex_shader(CYBER_UTF8("shaders/DX12/forward_color_vs.hlsl"))
            .pixel_shader(CYBER_UTF8("shaders/DX12/forward_color_ps.hlsl"))
            .vertex_layout(vertex_attributes, 3)
            .static_sampler(CYBER_UTF8("Texture_sampler"), m_sampler)
            .blend_opaque()
            .depth_test(true, false, CMP_LESS_EQUAL)
            .render_target_format(scene_target.color_buffer->get_create_desc().m_format)
            .depth_format(scene_target.depth_buffer->get_create_desc().m_format)
            .build();
    }

    ForwardFrameContext ForwardPipeline::begin_frame()
    {
        ForwardFrameContext frame_context = {};

        auto* swap_chain = m_renderer->get_swap_chain();
        AcquireNextDesc acquire_desc = {};
        frame_context.back_buffer_index = m_device->acquire_next_image(swap_chain, acquire_desc);
        m_renderer->set_back_buffer_index(frame_context.back_buffer_index);

        auto& scene_target = m_renderer->get_scene_target(frame_context.back_buffer_index);
        frame_context.frame_buffer = m_renderer->get_frame_buffer();
        frame_context.color_buffer = scene_target.color_buffer;
        frame_context.depth_buffer = scene_target.depth_buffer;
        frame_context.color_view = scene_target.color_buffer->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
        frame_context.depth_view = scene_target.depth_buffer->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);

        m_context->cmd_begin();
        return frame_context;
    }

    void ForwardPipeline::execute_predepth_pass(World* world, const ForwardFrameContext& frame_context)
    {
        RenderObject::ITexture_View* attachments[1] = { frame_context.depth_view };
        frame_context.frame_buffer->update_attachments(attachments, 1);

        RenderObject::BeginRenderPassAttribs begin_info = {};
        begin_info.pFramebuffer = frame_context.frame_buffer;
        begin_info.pRenderPass = m_predepth_pass;
        begin_info.ClearValueCount = 0;
        begin_info.depth_stencil_clear_value = { 1.0f, 0 };
        begin_info.TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_context->set_frame_buffer(frame_context.frame_buffer);
        m_context->cmd_begin_render_pass(begin_info);
        set_default_viewport(frame_context.depth_buffer->get_create_desc().m_width, frame_context.depth_buffer->get_create_desc().m_height);

        float4x4 view_proj = float4x4::Identity();
        float3 eye = float3(0.0f, 0.0f, 0.0f);
        if (find_scene_view(world, view_proj, eye))
            draw_depth_only(world, view_proj, m_depth_pipeline);

        m_context->cmd_end_render_pass();
    }

    void ForwardPipeline::execute_shadow_pass(World* world)
    {
        auto* shadow_view = m_shadow_map->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
        RenderObject::ITexture_View* attachments[1] = { shadow_view };
        m_shadow_frame_buffer->update_attachments(attachments, 1);

        RenderObject::BeginRenderPassAttribs begin_info = {};
        begin_info.pFramebuffer = m_shadow_frame_buffer;
        begin_info.pRenderPass = m_shadow_pass;
        begin_info.ClearValueCount = 0;
        begin_info.depth_stencil_clear_value = { 1.0f, 0 };
        begin_info.TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_context->set_frame_buffer(m_shadow_frame_buffer);
        m_context->cmd_begin_render_pass(begin_info);
        set_default_viewport(m_shadow_resolution, m_shadow_resolution);

        float3 light_dir;
        float3 light_color;
        float light_intensity;
        if (find_main_light(world, light_dir, light_color, light_intensity))
        {
            draw_depth_only(world, build_shadow_view_projection(light_dir), m_depth_pipeline);
        }

        m_context->cmd_end_render_pass();
    }

    void ForwardPipeline::execute_color_pass(World* world, const ForwardFrameContext& frame_context)
    {
        RenderObject::ITexture_View* attachments[2] = { frame_context.color_view, frame_context.depth_view };
        frame_context.frame_buffer->update_attachments(attachments, 2);

        GRAPHICS_CLEAR_VALUE clear_value = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
        RenderObject::BeginRenderPassAttribs begin_info = {};
        begin_info.pFramebuffer = frame_context.frame_buffer;
        begin_info.pRenderPass = m_color_pass;
        begin_info.ClearValueCount = 1;
        begin_info.color_clear_values = &clear_value;
        begin_info.depth_stencil_clear_value = { 1.0f, 0 };
        begin_info.TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_context->set_frame_buffer(frame_context.frame_buffer);
        m_context->cmd_begin_render_pass(begin_info);
        set_default_viewport(frame_context.color_buffer->get_create_desc().m_width, frame_context.color_buffer->get_create_desc().m_height);

        float4x4 view_proj = float4x4::Identity();
        float3 eye = float3(0.0f, 0.0f, 0.0f);
        float3 light_dir = float3(-0.3f, -1.0f, -0.2f);
        float3 light_color = float3(1.0f, 1.0f, 1.0f);
        float light_intensity = 1.0f;

        if (find_scene_view(world, view_proj, eye))
        {
            find_main_light(world, light_dir, light_color, light_intensity);
            draw_color(world, view_proj, eye, light_dir, light_color, light_intensity);
        }

        m_context->cmd_end_render_pass();
    }

    void ForwardPipeline::draw_depth_only(World* world, const float4x4& view_proj, RenderObject::IRenderPipeline* pipeline)
    {
        if (!world || !pipeline)
            return;

        m_context->render_encoder_bind_pipeline(pipeline);
        world->for_each_component_of<Component::MeshComponent>(
            [&](SceneNode&, Component::MeshComponent& mesh, uint32_t)
            {
                if (!mesh.enabled || !mesh.is_render_ready())
                    return;

                SceneConstants constants = {};
                constants.view_proj_matrix = view_proj.transpose();
                constants.model_matrix = mesh.local_matrix().transpose();
                constants.camera_pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
                constants.light_direction = float4(0.0f, -1.0f, 0.0f, 0.0f);
                constants.light_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
                update_scene_constants(constants);

                RenderObject::IBuffer* vertex_buffers[] = { mesh.vertex_buffer };
                uint32_t strides[] = { mesh.vertex_stride };
                m_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
                m_context->render_encoder_bind_index_buffer(mesh.index_buffer, sizeof(uint32_t), 0);
                m_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, m_scene_constants);

                for (const auto& primitive : mesh.draw_primitives)
                {
                    m_context->prepare_for_rendering();
                    m_context->render_encoder_draw_indexed(primitive.index_count, primitive.first_index, 0);
                }
            });
    }

    void ForwardPipeline::draw_color(World* world, const float4x4& view_proj, const float3& eye, const float3& light_dir, const float3& light_color, float light_intensity)
    {
        if (!world || !m_color_pipeline)
            return;

        m_context->render_encoder_bind_pipeline(m_color_pipeline);
        world->for_each_component_of<Component::MeshComponent>(
            [&](SceneNode&, Component::MeshComponent& mesh, uint32_t)
            {
                if (!mesh.enabled || !mesh.is_render_ready())
                    return;

                SceneConstants constants = {};
                constants.view_proj_matrix = view_proj.transpose();
                constants.model_matrix = mesh.local_matrix().transpose();
                constants.camera_pos = float4(eye, 1.0f);
                constants.light_direction = float4(light_dir, 0.0f);
                constants.light_color = float4(light_color * light_intensity, 1.0f);
                update_scene_constants(constants);

                RenderObject::IBuffer* vertex_buffers[] = { mesh.vertex_buffer };
                uint32_t strides[] = { mesh.vertex_stride };
                m_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
                m_context->render_encoder_bind_index_buffer(mesh.index_buffer, sizeof(uint32_t), 0);
                m_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, m_scene_constants);
                m_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, m_scene_constants);

                RenderObject::ITexture_View* fallback_base_color = m_white_texture
                    ? m_white_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE)
                    : nullptr;

                for (const auto& primitive : mesh.draw_primitives)
                {
                    RenderObject::ITexture_View* base_color_view = primitive.base_color_view;
                    if (!base_color_view)
                        base_color_view = fallback_base_color;
                    if (!base_color_view)
                        continue;

                    m_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, base_color_view);
                    m_context->prepare_for_rendering();
                    m_context->render_encoder_draw_indexed(primitive.index_count, primitive.first_index, 0);
                }
            });
    }

    void ForwardPipeline::set_default_viewport(uint32_t width, uint32_t height)
    {
        RenderObject::Viewport viewport = {};
        viewport.top_left_x = 0.0f;
        viewport.top_left_y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;
        m_context->render_encoder_set_viewport(1, &viewport);

        RenderObject::Rect scissor = { 0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height) };
        m_context->render_encoder_set_scissor(1, &scissor);
    }

    void ForwardPipeline::update_scene_constants(const SceneConstants& constants)
    {
        void* mapped = m_device->map_buffer(m_scene_constants, MAP_WRITE, MAP_FLAG_DISCARD);
        std::memcpy(mapped, &constants, sizeof(constants));
        m_device->unmap_buffer(m_scene_constants, MAP_WRITE);
        m_scene_constants->set_buffer_size(sizeof(constants));
    }

    bool ForwardPipeline::find_scene_view(World* world, float4x4& view_proj, float3& eye) const
    {
        if (!world)
            return false;

        Component::CameraComponent* camera = nullptr;
        world->for_each_component_of<Component::CameraComponent>(
            [&](SceneNode&, Component::CameraComponent& candidate, uint32_t)
            {
                if (!camera && candidate.enabled)
                    camera = &candidate;
            });

        if (!camera)
            return false;

        eye = camera->get_camera_position();
        const float fov = camera->fov_deg * (3.14159265f / 180.0f);
        view_proj = camera->get_view_matrix() * m_renderer->get_adjusted_projection_matrix(fov, camera->near_z, camera->far_z);
        return true;
    }

    bool ForwardPipeline::find_main_light(World* world, float3& light_dir, float3& light_color, float& intensity) const
    {
        light_dir = float3(-0.3f, -1.0f, -0.2f);
        light_color = float3(1.0f, 1.0f, 1.0f);
        intensity = 1.0f;

        if (!world)
            return false;

        bool found = false;
        world->for_each_component_of<Component::DirectionalLightComponent>(
            [&](SceneNode&, Component::DirectionalLightComponent& light, uint32_t)
            {
                if (!found && light.enabled)
                {
                    light_dir = light.get_direction();
                    light_color = light.color;
                    intensity = light.intensity;
                    found = true;
                }
            });
        return found;
    }

    float4x4 ForwardPipeline::build_shadow_view_projection(const float3& light_dir) const
    {
        float3 dir = normalize(light_dir);
        if (length(dir) == 0.0f)
            dir = normalize(float3(-0.3f, -1.0f, -0.2f));

        const float3 center = float3(0.0f, 0.0f, 0.0f);
        const float3 eye = center - dir * 80.0f;
        const float4x4 view = float4x4::look_at(eye, center, float3(0.0f, 1.0f, 0.0f));
        const float4x4 projection = float4x4::ortho_off_center(-80.0f, 80.0f, -80.0f, 80.0f, 0.1f, 200.0f);
        return view * projection;
    }
}
