#include "sponza_sample.h"
#include "platform/memory.h"
#include "model_loader.h"
#include "texture_utils.h"
#include "gameruntime/pipeline_builder.h"
#include "gameruntime/scene_serializer.h"
#include "gameruntime/world.h"
#include "graphics/interface/device_context.h"
#include "core/file_helper.hpp"
#include "log/Log.h"

#include "component/mesh_component.h"
#include "component/camera_component.h"
#include "component/directional_light_component.h"

namespace Cyber
{
    namespace Samples
    {
        using Component::MeshComponent;
        using Component::CameraComponent;
        using Component::DirectionalLightComponent;

        CYBER_GAME_API SampleApp* create_sponza_app()
        {
            return Cyber::cyber_new<Cyber::Samples::SponzaApp>();
        }

        SponzaApp::SponzaApp() {}
        SponzaApp::~SponzaApp()
        {
            // MeshComponent doesn't own ModelLoader::Model (dep cycle — see
            // mesh_component.h). Walk components and free the raw pointer
            // here before the world/nodes are destroyed.
            if (m_world)
            {
                m_world->for_each_component_of<MeshComponent>(
                    [](SceneNode&, MeshComponent& mc, uint32_t /*idx*/)
                    {
                        if (mc.model)
                        {
                            cyber_delete(mc.model);
                            mc.model = nullptr;
                        }
                    });
            }
        }

        void SponzaApp::on_create_gfx_objects()
        {
            create_default_render_pass(u8"Sponza RenderPass", render_pass);
        }

        void SponzaApp::on_create_pipelines()
        {
            auto render_device = get_render_device();
            auto& scene_target = get_renderer()->get_scene_target(0);
            auto sampler = create_default_sampler();

            RenderObject::VertexAttribute vertex_attributes[] = {
                {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(SponzaVertex, position)},
                {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(SponzaVertex, normal)},
                {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(SponzaVertex, uv)},
            };

            pipeline = PipelineBuilder(render_device)
                .vertex_shader(CYBER_UTF8("samples/sponza/assets/shaders/sponza_vs.hlsl"))
                .pixel_shader(CYBER_UTF8("samples/sponza/assets/shaders/sponza_ps.hlsl"))
                .vertex_layout(vertex_attributes, 3)
                .static_sampler(CYBER_UTF8("Texture_sampler"), sampler)
                .blend_opaque()
                .depth_test()
                .render_target_format(scene_target.color_buffer->get_create_desc().m_format)
                .depth_format(scene_target.depth_buffer->get_create_desc().m_format)
                .build();
        }

        void SponzaApp::ensure_camera_and_sun()
        {
            bool has_camera = false;
            bool has_sun = false;
            m_world->for_each_component_of<CameraComponent>(
                [&](SceneNode&, CameraComponent&, uint32_t){ has_camera = true; });
            m_world->for_each_component_of<DirectionalLightComponent>(
                [&](SceneNode&, DirectionalLightComponent&, uint32_t){ has_sun = true; });

            if (!has_camera)
            {
                SceneNode cam_node;
                cam_node.name = "MainCamera";
                auto cc = Scope<CameraComponent>(new CameraComponent(float3(10.0f, 5.0f, 0.0f)));
                cc->fov_deg = 60.0f;
                cc->set_yaw(-1.5707963f); // face -X toward the atrium
                cam_node.components.push_back(Scope<Component::Primitive>(cc.release()));
                m_world->add_node(std::move(cam_node));
            }
            if (!has_sun)
            {
                SceneNode sun_node;
                sun_node.name = "Sun";
                auto dl = Scope<DirectionalLightComponent>(new DirectionalLightComponent());
                dl->set_direction(float3(-0.3f, -1.0f, -0.2f));
                dl->intensity = 3.0f;
                sun_node.components.push_back(Scope<Component::Primitive>(dl.release()));
                m_world->add_node(std::move(sun_node));
            }
        }

        void SponzaApp::on_load_data()
        {
            if (!SceneSerializer::load(*m_world, "samples/sponza/assets/sponza.scene"))
            {
                cyber_error("Failed to load sponza.scene");
                return;
            }

            ensure_camera_and_sun();

            // CPU-only pre-load of every MeshComponent's glTF data.
            m_world->for_each_component_of<MeshComponent>(
                [](SceneNode&, MeshComponent& mc, uint32_t /*idx*/)
                {
                    if (mc.model_resource.empty())
                        return;

                    eastl::string resolved = Core::FileHelper::resolve_path_public(mc.model_resource.c_str());
                    ModelLoader::ModelCreateInfo ci;
                    ci.file_path = resolved.c_str();
                    mc.model = cyber_new<ModelLoader::Model>(ci);
                    mc.model->load_data(ci);
                });
        }

        bool SponzaApp::build_render_mesh_for_component(SceneNode& node, uint32_t component_index,
                                                       MeshComponent& mc, RenderMesh& out)
        {
            if (!mc.model || !mc.model->is_valid())
            {
                CB_ERROR("Skipping mesh component on node '%s' (id=%u) — model not loaded",
                         node.name.c_str(), node.id);
                return false;
            }

            auto render_device = get_render_device();
            mc.model->create_gpu_textures(render_device);

            const auto vertex_count = mc.model->get_vertex_count();
            const auto index_count  = mc.model->get_index_count();
            const auto model_verts  = mc.model->get_vertex_data();
            const auto model_idx    = mc.model->get_index_data();

            eastl::vector<SponzaVertex> vertices(vertex_count);
            for (size_t i = 0; i < vertex_count; ++i)
            {
                vertices[i].position = model_verts[i].pos;
                vertices[i].normal   = model_verts[i].normal;
                vertices[i].uv       = model_verts[i].uv0;
            }

            eastl::vector<uint32_t> indices(index_count);
            for (size_t i = 0; i < index_count; ++i)
                indices[i] = model_idx[i];

            create_vertex_buffer(vertices.data(), vertex_count * sizeof(SponzaVertex), out.vertex_buffer);
            create_index_buffer(indices.data(), index_count * sizeof(uint32_t), out.index_buffer);

            const auto& meshes    = mc.model->get_meshes();
            const auto& materials = mc.model->get_materials();

            for (auto& mesh : meshes)
            {
                for (auto& prim : mesh.primitives)
                {
                    DrawPrimitive dp;
                    dp.first_index = prim.first_index;
                    dp.index_count = prim.index_count;
                    dp.base_color_view = nullptr;

                    if (prim.material_id < materials.size())
                    {
                        auto& material = materials[prim.material_id];
                        int tex_id = material.texture_ids[ModelLoader::DefaultBaseColorTextureAttribId];
                        if (tex_id >= 0)
                        {
                            auto* tex = mc.model->get_texture(tex_id);
                            if (tex)
                                dp.base_color_view = tex->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                    }
                    out.primitives.push_back(dp);
                }
            }

            out.node_id = node.id;
            out.component_index = component_index;
            mc.gpu_ready = true;
            return true;
        }

        void SponzaApp::on_create_resources()
        {
            create_constant_buffer(sizeof(SceneConstants), scene_cbuffer);

            render_meshes.clear();
            m_world->for_each_component_of<MeshComponent>(
                [this](SceneNode& node, MeshComponent& mc, uint32_t idx)
                {
                    if (mc.model_resource.empty() || !mc.model)
                        return;

                    RenderMesh rm;
                    if (build_render_mesh_for_component(node, idx, mc, rm))
                        render_meshes.push_back(std::move(rm));
                });

            m_world->pending_loads().clear();
        }

        void SponzaApp::process_pending_loads()
        {
            if (!m_world)
                return;

            auto& pending = m_world->pending_loads();
            if (pending.empty())
                return;

            for (auto& p : pending)
            {
                SceneNode* node = m_world->find_node(p.node_id);
                if (!node)
                    continue;
                if (p.component_index >= node->components.size())
                    continue;

                auto* comp = node->components[p.component_index].get();
                auto* mc = dynamic_cast<MeshComponent*>(comp);
                if (!mc)
                    continue;

                if (!mc->model)
                {
                    eastl::string resolved = Core::FileHelper::resolve_path_public(p.model_resource.c_str());
                    ModelLoader::ModelCreateInfo ci;
                    ci.file_path = resolved.c_str();
                    mc->model = cyber_new<ModelLoader::Model>(ci);
                    mc->model->load_data(ci);
                }

                RenderMesh rm;
                if (build_render_mesh_for_component(*node, p.component_index, *mc, rm))
                    render_meshes.push_back(std::move(rm));
            }
            pending.clear();
        }

        void SponzaApp::update(float deltaTime)
        {
            process_pending_loads();

            CameraComponent* camera = nullptr;
            m_world->for_each_component_of<CameraComponent>(
                [&](SceneNode&, CameraComponent& cc, uint32_t)
                {
                    if (!camera && cc.enabled) camera = &cc;
                });

            DirectionalLightComponent* sun = nullptr;
            m_world->for_each_component_of<DirectionalLightComponent>(
                [&](SceneNode&, DirectionalLightComponent& dl, uint32_t)
                {
                    if (!sun && dl.enabled) sun = &dl;
                });

            if (!camera)
                return;   // nothing sensible to render without a camera

            if (camera_orbit_speed > 0.0f)
            {
                camera->set_yaw(camera->get_yaw() + camera_orbit_speed * deltaTime);
                m_world->set_dirty(true);
            }

            // Drive yaw/pitch + WASD translation from user input, and refresh
            // the camera's view matrix for this frame.
            camera->update(deltaTime);

            const float3 eye = camera->get_camera_position();
            float4x4 view_matrix = camera->get_view_matrix();
            float4x4 projection_matrix =
                get_renderer()->get_adjusted_projection_matrix(camera->fov_deg * (3.14159265f / 180.0f),
                                                               camera->near_z, camera->far_z);

            float3 light_dir = sun ? sun->get_direction() : float3(-0.3f, -1.0f, -0.2f);
            float3 light_col = sun ? sun->color     : float3(1.0f, 1.0f, 1.0f);
            float  light_int = sun ? sun->intensity : 1.0f;

            auto ctx = begin_frame(render_pass);
            auto device_context = ctx.device_context;

            device_context->render_encoder_bind_pipeline(pipeline);

            for (auto& rm : render_meshes)
            {
                const SceneNode* node = m_world->find_node(rm.node_id);
                if (!node || rm.component_index >= node->components.size())
                    continue;
                const auto* comp = node->components[rm.component_index].get();
                if (!comp)
                    continue;

                float4x4 model_matrix = comp->local_matrix();

                SceneConstants scene_data;
                scene_data.view_proj_matrix = (view_matrix * projection_matrix).transpose();
                scene_data.model_matrix     = model_matrix.transpose();
                scene_data.camera_pos       = float4(eye, 1.0f);
                scene_data.light_direction  = float4(light_dir, 0.0f);
                scene_data.light_color      = float4(light_col * light_int, 1.0f);
                update_buffer(scene_cbuffer, scene_data);

                RenderObject::IBuffer* vb[] = { rm.vertex_buffer };
                uint32_t strides[] = { sizeof(SponzaVertex) };
                device_context->render_encoder_bind_vertex_buffer(1, vb, strides, nullptr);
                device_context->render_encoder_bind_index_buffer(rm.index_buffer, sizeof(uint32_t), 0);

                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, scene_cbuffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, scene_cbuffer);

                for (auto& dp : rm.primitives)
                {
                    if (dp.base_color_view)
                        device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, dp.base_color_view);

                    device_context->prepare_for_rendering();
                    device_context->render_encoder_draw_indexed(dp.index_count, dp.first_index, 0);
                }
            }

            end_frame();
        }

        void SponzaApp::draw_ui(ImGuiContext* in_imgui_context)
        {
            if (!in_imgui_context)
                return;
            ImGui::SetCurrentContext(in_imgui_context);

            if (ImGui::Begin("Sponza Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                CameraComponent* camera = nullptr;
                m_world->for_each_component_of<CameraComponent>(
                    [&](SceneNode&, CameraComponent& cc, uint32_t){ if (!camera) camera = &cc; });
                DirectionalLightComponent* sun = nullptr;
                m_world->for_each_component_of<DirectionalLightComponent>(
                    [&](SceneNode&, DirectionalLightComponent& dl, uint32_t){ if (!sun) sun = &dl; });

                if (camera && ImGui::TreeNode("Camera"))
                {
                    bool changed = false;
                    float yaw = camera->get_yaw();
                    float pitch = camera->get_pitch();
                    if (ImGui::SliderFloat("Yaw",   &yaw,   -3.14159f, 3.14159f))
                    {
                        camera->set_yaw(yaw);
                        changed = true;
                    }
                    if (ImGui::SliderFloat("Pitch", &pitch, -1.55334f, 1.55334f))
                    {
                        camera->set_pitch(pitch);
                        changed = true;
                    }
                    changed |= ImGui::SliderFloat3("Position", camera->position.data(), -500.0f, 500.0f);
                    changed |= ImGui::SliderFloat("FOV (deg)", &camera->fov_deg, 20.0f, 120.0f);
                    changed |= ImGui::SliderFloat("Orbit",     &camera_orbit_speed, 0.0f, 2.0f);
                    ImGui::TextDisabled("Hold LMB to look; WASD to fly, E/Q up/down");
                    if (changed)
                        m_world->set_dirty(true);
                    ImGui::TreePop();
                }

                if (sun && ImGui::TreeNode("Lighting"))
                {
                    bool changed = false;
                    changed |= ImGui::ColorEdit3("Light Color", sun->color.data());
                    changed |= ImGui::SliderFloat("Intensity",  &sun->intensity, 0.0f, 10.0f);
                    float3 dir = sun->get_direction();
                    if (ImGui::SliderFloat3("Light Dir", dir.data(), -1.0f, 1.0f))
                    {
                        sun->set_direction(dir);
                        changed = true;
                    }
                    if (changed)
                        m_world->set_dirty(true);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Scene"))
                {
                    ImGui::Text("Source: %s",
                        m_world->source_path().empty() ? "(unsaved)" : m_world->source_path().c_str());
                    ImGui::Text("Nodes     : %zu", (size_t)m_world->get_nodes().size());
                    ImGui::Text("Render meshes: %zu", (size_t)render_meshes.size());
                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }
    }
}
