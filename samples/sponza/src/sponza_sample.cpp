#include "sponza_sample.h"
#include "platform/memory.h"
#include "model_loader.h"
#include "texture_utils.h"
#include "gameruntime/scene_serializer.h"
#include "gameruntime/project_settings.h"
#include "gameruntime/world.h"
#include "asset/mesh_importer.h"
#include "graphics/interface/device_context.h"
#include "core/file_helper.hpp"
#include "log/Log.h"

#include "component/mesh_component.h"
#include "component/camera_component.h"
#include "component/directional_light_component.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace Cyber
{
    namespace Samples
    {
        using Component::MeshComponent;
        using Component::CameraComponent;
        using Component::DirectionalLightComponent;

        namespace
        {
            namespace fs = std::filesystem;

            void destroy_model_loader_model(ModelLoader::Model* model)
            {
                cyber_delete(model);
            }

            std::string lowercase_extension(const fs::path& path)
            {
                std::string ext = path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return ext;
            }

            eastl::string resolve_model_resource_for_display(const eastl::string& model_resource)
            {
                if (model_resource.empty())
                    return {};

                fs::path resolved_path(Core::FileHelper::resolve_path_public(model_resource.c_str()).c_str());
                resolved_path = resolved_path.lexically_normal();

                if (lowercase_extension(resolved_path) != ".meshasset")
                {
                    CB_ERROR("MeshComponent model_resource only accepts .meshasset files: %s",
                             resolved_path.string().c_str());
                    return {};
                }

                MeshEditorAssetInfo mesh_info;
                if (!MeshImporter::ReadInfo(resolved_path, mesh_info))
                {
                    CB_ERROR("Mesh asset is invalid: %s", resolved_path.string().c_str());
                    return {};
                }
                if (!mesh_info.isCooked)
                {
                    CB_ERROR("Mesh asset uses legacy payload v1 and must be reimported: %s",
                             resolved_path.string().c_str());
                    return {};
                }
                return eastl::string(resolved_path.string().c_str());
            }
        }

        CYBER_GAME_API SampleApp* create_sponza_app()
        {
            return Cyber::cyber_new<Cyber::Samples::SponzaApp>();
        }

        SponzaApp::SponzaApp() {}
        SponzaApp::~SponzaApp() = default;

        void SponzaApp::on_create_gfx_objects()
        {
        }

        void SponzaApp::on_create_pipelines()
        {
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
            static constexpr const char* kFallbackScene = "samples/sponza/assets/sponza.scene";

            ProjectSettings project_settings;
            const char* startup_scene = kFallbackScene;
            if (ProjectSettingsIO::load(project_settings) && !project_settings.startup_scene.empty())
                startup_scene = project_settings.startup_scene.c_str();

            if (!SceneSerializer::load(*m_world, startup_scene))
            {
                cyber_error("Failed to load startup scene");
                return;
            }

            ensure_camera_and_sun();

            // CPU-only pre-load of every MeshComponent's glTF data.
            m_world->for_each_component_of<MeshComponent>(
                [](SceneNode&, MeshComponent& mc, uint32_t /*idx*/)
                {
                    if (mc.model_resource.empty())
                        return;

                    eastl::string resolved = resolve_model_resource_for_display(mc.model_resource);
                    if (resolved.empty())
                        return;

                    ModelLoader::ModelCreateInfo ci;
                    ci.file_path = resolved.c_str();
                    mc.set_runtime_model(cyber_new<ModelLoader::Model>(ci), destroy_model_loader_model);
                    mc.model->load_data(ci);
                });
        }

        bool SponzaApp::build_render_mesh_for_component(SceneNode& node, MeshComponent& mc)
        {
            mc.gpu_ready = false;
            mc.runtime_vertex_count = 0;
            mc.runtime_index_count = 0;
            mc.runtime_bounds_valid = false;

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
            float3 bounds_min{0.0f, 0.0f, 0.0f};
            float3 bounds_max{0.0f, 0.0f, 0.0f};
            for (size_t i = 0; i < vertex_count; ++i)
            {
                vertices[i].position = model_verts[i].pos;
                vertices[i].normal   = model_verts[i].normal;
                vertices[i].uv       = model_verts[i].uv0;
                if (i == 0)
                {
                    bounds_min = model_verts[i].pos;
                    bounds_max = model_verts[i].pos;
                }
                else
                {
                    bounds_min = Math::min(bounds_min, model_verts[i].pos);
                    bounds_max = Math::max(bounds_max, model_verts[i].pos);
                }
            }

            eastl::vector<uint32_t> indices(index_count);
            for (size_t i = 0; i < index_count; ++i)
                indices[i] = model_idx[i];

            create_vertex_buffer(vertices.data(), vertex_count * sizeof(SponzaVertex), mc.vertex_buffer);
            create_index_buffer(indices.data(), index_count * sizeof(uint32_t), mc.index_buffer);
            if (!mc.vertex_buffer || !mc.index_buffer)
            {
                CB_ERROR("MeshComponent GPU buffer creation failed: node='{}', vertices={}, indices={}",
                         node.name.c_str(), vertex_count, index_count);
                mc.vertex_buffer = nullptr;
                mc.index_buffer = nullptr;
                return false;
            }

            mc.vertex_stride = sizeof(SponzaVertex);
            mc.draw_primitives.clear();

            const auto& meshes    = mc.model->get_meshes();
            const auto& materials = mc.model->get_materials();

            for (auto& mesh : meshes)
            {
                for (auto& prim : mesh.primitives)
                {
                    Cyber::RefCntAutoPtr<RenderObject::ITexture_View> base_color_view = nullptr;

                    if (prim.material_id < materials.size())
                    {
                        auto& material = materials[prim.material_id];
                        int tex_id = material.texture_ids[ModelLoader::DefaultBaseColorTextureAttribId];
                        if (tex_id >= 0)
                        {
                            auto* tex = mc.model->get_texture(tex_id);
                            if (tex)
                                base_color_view = tex->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                    }

                    Component::MeshDrawPrimitive engine_dp;
                    engine_dp.first_index = prim.first_index;
                    engine_dp.index_count = prim.index_count;
                    engine_dp.base_color_view = base_color_view;
                    mc.draw_primitives.push_back(engine_dp);
                }
            }

            if (mc.draw_primitives.empty())
            {
                CB_ERROR("MeshComponent has no renderable primitives: node='{}'", node.name.c_str());
                mc.vertex_buffer = nullptr;
                mc.index_buffer = nullptr;
                return false;
            }

            mc.runtime_vertex_count = vertex_count;
            mc.runtime_index_count = index_count;
            mc.runtime_bounds_min = bounds_min;
            mc.runtime_bounds_max = bounds_max;
            mc.runtime_bounds_valid = vertex_count > 0;
            mc.gpu_ready = true;
            CB_INFO("MeshComponent GPU ready: node='{}', vertices={}, indices={}, primitives={}",
                    node.name.c_str(), vertex_count, index_count, mc.draw_primitives.size());
            return true;
        }

        void SponzaApp::on_create_resources()
        {
            m_world->for_each_component_of<MeshComponent>(
                [this](SceneNode& node, MeshComponent& mc, uint32_t)
                {
                    if (mc.model_resource.empty() || !mc.model)
                        return;

                    build_render_mesh_for_component(node, mc);
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
                    eastl::string resolved = resolve_model_resource_for_display(p.model_resource);
                    if (resolved.empty())
                    {
                        CB_WARN("Pending mesh load skipped: stored='{}' could not be resolved",
                                p.model_resource.c_str());
                        continue;
                    }

                    CB_INFO("Pending mesh load: stored='{}', loader_path='{}'",
                            p.model_resource.c_str(),
                            resolved.c_str());

                    ModelLoader::ModelCreateInfo ci;
                    ci.file_path = resolved.c_str();
                    mc->set_runtime_model(cyber_new<ModelLoader::Model>(ci), destroy_model_loader_model);
                    mc->model->load_data(ci);
                }

                build_render_mesh_for_component(*node, *mc);
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
                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }
    }
}
