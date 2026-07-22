#include "graphics/features/forward_pass.h"

#include "component/camera_component.h"
#include "component/directional_light_component.h"
#include "component/mesh_component.h"
#include "gameruntime/pipeline_builder.h"
#include "gameruntime/world.h"
#include "graphics/interface/buffer.h"
#include "graphics/interface/device_context.h"
#include "graphics/interface/render_device.hpp"
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
    }

    void ForwardPassPipelineCache::initialize(RenderObject::IRenderDevice* render_device)
    {
        if (device == render_device)
            return;

        depth_pipelines.clear();
        device = render_device;
    }

    RenderObject::IRenderPipeline* ForwardPassPipelineCache::get_depth_only(TEXTURE_FORMAT depth_format)
    {
        if (!device)
            return nullptr;

        const auto existing = depth_pipelines.find(depth_format);
        if (existing != depth_pipelines.end())
            return existing->second;

        RenderObject::VertexAttribute vertex_attributes[] = {
            {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, position)},
            {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, normal)},
            {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(ForwardVertex, uv)},
        };

        RefCntAutoPtr<RenderObject::IRenderPipeline> pipeline = PipelineBuilder(device)
            .vertex_shader(CYBER_UTF8("shaders/DX12/forward_depth_vs.hlsl"))
            .vertex_layout(vertex_attributes, 3)
            .blend_opaque()
            .depth_test(true, true, CMP_LESS_EQUAL)
            .render_target_count(0)
            .depth_format(depth_format)
            .build();

        depth_pipelines[depth_format] = pipeline;
        return pipeline;
    }

    ForwardRenderPass::ForwardRenderPass(ForwardPassContext* context)
        : pass_context(context)
    {
    }

    void ForwardRenderPass::set_default_viewport(uint32_t width, uint32_t height) const
    {
        if (!pass_context || !pass_context->command_context)
            return;

        RenderObject::Viewport viewport = {};
        viewport.top_left_x = 0.0f;
        viewport.top_left_y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;
        pass_context->command_context->render_encoder_set_viewport(1, &viewport);

        RenderObject::Rect scissor = { 0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height) };
        pass_context->command_context->render_encoder_set_scissor(1, &scissor);
    }

    void ForwardRenderPass::update_scene_constants(const ForwardSceneConstants& constants) const
    {
        if (!pass_context || !pass_context->device || !pass_context->scene_constants)
            return;

        void* mapped = pass_context->device->map_buffer(pass_context->scene_constants, MAP_WRITE, MAP_FLAG_DISCARD);
        if (!mapped)
            return;

        std::memcpy(mapped, &constants, sizeof(constants));
        pass_context->device->unmap_buffer(pass_context->scene_constants, MAP_WRITE);
        pass_context->scene_constants->set_buffer_size(sizeof(constants));
    }

    void ForwardRenderPass::draw_depth_only(const float4x4& view_proj, RenderObject::IRenderPipeline* pipeline) const
    {
        if (!pass_context || !pass_context->frame.world || !pass_context->command_context || !pipeline)
            return;

        auto* command_context = pass_context->command_context;
        command_context->render_encoder_bind_pipeline(pipeline);
        pass_context->frame.world->for_each_component_of<Component::MeshComponent>(
            [&](SceneNode&, Component::MeshComponent& mesh, uint32_t)
            {
                if (!mesh.enabled || !mesh.is_render_ready())
                    return;

                ForwardSceneConstants constants = {};
                constants.view_proj_matrix = view_proj.transpose();
                constants.model_matrix = mesh.local_matrix().transpose();
                constants.camera_pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
                constants.light_direction = float4(0.0f, -1.0f, 0.0f, 0.0f);
                constants.light_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
                update_scene_constants(constants);

                RenderObject::IBuffer* vertex_buffers[] = { mesh.vertex_buffer };
                uint32_t strides[] = { mesh.vertex_stride };
                command_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
                command_context->render_encoder_bind_index_buffer(mesh.index_buffer, sizeof(uint32_t), 0);
                command_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, pass_context->scene_constants);

                for (const auto& primitive : mesh.draw_primitives)
                {
                    command_context->prepare_for_rendering();
                    command_context->render_encoder_draw_indexed(primitive.index_count, primitive.first_index, 0);
                }
            });
    }

    void ForwardRenderPass::draw_color(const float4x4& view_proj, const float3& eye,
        const float3& light_dir, const float3& light_color, float light_intensity,
        RenderObject::IRenderPipeline* pipeline, RenderObject::ITexture* fallback_texture) const
    {
        if (!pass_context || !pass_context->frame.world || !pass_context->command_context ||
            !pipeline)
            return;

        auto* command_context = pass_context->command_context;
        command_context->render_encoder_bind_pipeline(pipeline);
        pass_context->frame.world->for_each_component_of<Component::MeshComponent>(
            [&](SceneNode&, Component::MeshComponent& mesh, uint32_t)
            {
                if (!mesh.enabled || !mesh.is_render_ready())
                    return;

                ForwardSceneConstants constants = {};
                constants.view_proj_matrix = view_proj.transpose();
                constants.model_matrix = mesh.local_matrix().transpose();
                constants.camera_pos = float4(eye, 1.0f);
                constants.light_direction = float4(light_dir, 0.0f);
                constants.light_color = float4(light_color * light_intensity, 1.0f);
                update_scene_constants(constants);

                RenderObject::IBuffer* vertex_buffers[] = { mesh.vertex_buffer };
                uint32_t strides[] = { mesh.vertex_stride };
                command_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
                command_context->render_encoder_bind_index_buffer(mesh.index_buffer, sizeof(uint32_t), 0);
                command_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, pass_context->scene_constants);
                command_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, pass_context->scene_constants);

                RenderObject::ITexture_View* fallback_base_color = fallback_texture
                    ? fallback_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE)
                    : nullptr;

                for (const auto& primitive : mesh.draw_primitives)
                {
                    RenderObject::ITexture_View* base_color_view = primitive.base_color_view;
                    if (!base_color_view)
                        base_color_view = fallback_base_color;
                    if (!base_color_view)
                        continue;

                    command_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, base_color_view);
                    command_context->prepare_for_rendering();
                    command_context->render_encoder_draw_indexed(primitive.index_count, primitive.first_index, 0);
                }
            });
    }

    bool ForwardRenderPass::find_scene_view(float4x4& view_proj, float3& eye) const
    {
        if (!pass_context || !pass_context->renderer || !pass_context->frame.world)
            return false;

        Component::CameraComponent* camera = nullptr;
        pass_context->frame.world->for_each_component_of<Component::CameraComponent>(
            [&](SceneNode&, Component::CameraComponent& candidate, uint32_t)
            {
                if (!camera && candidate.enabled)
                    camera = &candidate;
            });

        if (!camera)
            return false;

        eye = camera->get_camera_position();
        const float fov = camera->fov_deg * (3.14159265f / 180.0f);
        view_proj = camera->get_view_matrix() *
            pass_context->renderer->get_adjusted_projection_matrix(fov, camera->near_z, camera->far_z);
        return true;
    }

    bool ForwardRenderPass::find_main_light(float3& light_dir, float3& light_color, float& intensity) const
    {
        light_dir = float3(-0.3f, -1.0f, -0.2f);
        light_color = float3(1.0f, 1.0f, 1.0f);
        intensity = 1.0f;

        if (!pass_context || !pass_context->frame.world)
            return false;

        bool found = false;
        pass_context->frame.world->for_each_component_of<Component::DirectionalLightComponent>(
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

    float4x4 ForwardRenderPass::build_shadow_view_projection(const float3& light_dir) const
    {
        float3 direction = normalize(light_dir);
        if (length(direction) == 0.0f)
            direction = normalize(float3(-0.3f, -1.0f, -0.2f));

        const float3 center = float3(0.0f, 0.0f, 0.0f);
        const float3 eye = center - direction * 80.0f;
        const float4x4 view = float4x4::look_at(eye, center, float3(0.0f, 1.0f, 0.0f));
        const float4x4 projection = float4x4::ortho_off_center(
            -80.0f, 80.0f, -80.0f, 80.0f, 0.1f, 200.0f);
        return view * projection;
    }
}
