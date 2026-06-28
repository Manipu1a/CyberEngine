#include "sponza_sample.h"
#include "platform/memory.h"
#include "model_loader.h"
#include "texture_utils.h"
#include "gameruntime/scene_serializer.h"
#include "gameruntime/world.h"
#include "asset/asset_database.h"
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

            std::string lowercase_extension(const fs::path& path)
            {
                std::string ext = path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return ext;
            }

            bool is_displayable_model_extension(const fs::path& path)
            {
                const std::string ext = lowercase_extension(path);
                return ext == ".gltf" || ext == ".glb";
            }

            fs::path resolve_stored_path(const fs::path& content_root, const std::string& stored_path)
            {
                fs::path path(AssetRegistry::NormalizePath(stored_path));
                if (path.is_absolute())
                    return path.lexically_normal();
                return (content_root / path).lexically_normal();
            }

            fs::path find_content_root_for_asset(const fs::path& asset_path)
            {
                fs::path dir = asset_path.parent_path();
                while (!dir.empty())
                {
                    std::error_code ec;
                    const fs::path registry_path = dir / "Registry" / "AssetRegistry.json";
                    if (fs::exists(registry_path, ec) && fs::is_regular_file(registry_path, ec))
                        return dir.lexically_normal();

                    const fs::path parent = dir.parent_path();
                    if (parent == dir)
                        break;
                    dir = parent;
                }

                return {};
            }

            bool try_resolve_meshasset_source_from_registry(const fs::path& mesh_asset_path,
                                                            fs::path& out_source_path)
            {
                out_source_path.clear();

                const fs::path content_root = find_content_root_for_asset(mesh_asset_path);
                if (content_root.empty())
                    return false;

                AssetDatabase database(content_root);
                if (!database.Load())
                    return false;

                const std::string stored_asset_path = database.MakeStoredPath(mesh_asset_path);
                const AssetRegistryRecord* record = database.Registry().FindByAssetPath(stored_asset_path);
                if (!record || record->sourcePath.empty())
                    return false;

                fs::path source_path = resolve_stored_path(content_root, record->sourcePath);
                std::error_code ec;
                if (!fs::exists(source_path, ec) || fs::is_directory(source_path, ec))
                    return false;

                if (!is_displayable_model_extension(source_path))
                    return false;

                out_source_path = source_path.lexically_normal();
                return true;
            }

            eastl::string resolve_model_resource_for_display(const eastl::string& model_resource)
            {
                if (model_resource.empty())
                    return {};

                fs::path resolved_path(Core::FileHelper::resolve_path_public(model_resource.c_str()).c_str());
                resolved_path = resolved_path.lexically_normal();

                if (lowercase_extension(resolved_path) != ".meshasset")
                    return eastl::string(resolved_path.string().c_str());

                MeshEditorAssetInfo mesh_info;
                if (!MeshImporter::ReadInfo(resolved_path, mesh_info))
                {
                    CB_ERROR("Mesh asset is invalid: %s", resolved_path.string().c_str());
                    return {};
                }

                std::string source_ext = mesh_info.sourceExtension;
                std::transform(source_ext.begin(), source_ext.end(), source_ext.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (source_ext != ".gltf" && source_ext != ".glb")
                {
                    CB_ERROR("Mesh asset source format is not displayable yet: %s (%s)",
                             resolved_path.string().c_str(),
                             source_ext.c_str());
                    return {};
                }

                fs::path source_path;
                if (try_resolve_meshasset_source_from_registry(resolved_path, source_path))
                    return eastl::string(source_path.string().c_str());

                const fs::path project_root = fs::path(Core::FileHelper::get_project_root()).lexically_normal();
                const fs::path cache_root = project_root / "Saved" / "EditorAssetCache" / "MeshSources";
                fs::path extracted_path;
                if (!MeshImporter::WriteEmbeddedSourceToCache(resolved_path, cache_root, extracted_path))
                {
                    CB_ERROR("Failed to extract mesh asset source for display: %s",
                             resolved_path.string().c_str());
                    return {};
                }

                if (source_ext == ".gltf")
                {
                    CB_WARN("Using extracted glTF from mesh asset; external .bin/textures still require the original source directory.");
                }

                return eastl::string(extracted_path.lexically_normal().string().c_str());
            }
        }

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

                    eastl::string resolved = resolve_model_resource_for_display(mc.model_resource);
                    if (resolved.empty())
                        return;

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
            mc.vertex_buffer = out.vertex_buffer;
            mc.index_buffer = out.index_buffer;
            mc.vertex_stride = sizeof(SponzaVertex);
            mc.draw_primitives.clear();

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

                    Component::MeshDrawPrimitive engine_dp;
                    engine_dp.first_index = dp.first_index;
                    engine_dp.index_count = dp.index_count;
                    engine_dp.base_color_view = dp.base_color_view;
                    mc.draw_primitives.push_back(engine_dp);
                }
            }

            out.node_id = node.id;
            out.component_index = component_index;
            mc.gpu_ready = true;
            return true;
        }

        void SponzaApp::on_create_resources()
        {
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
                    eastl::string resolved = resolve_model_resource_for_display(p.model_resource);
                    if (resolved.empty())
                        continue;

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
                    ImGui::Text("Render meshes: %zu", (size_t)render_meshes.size());
                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }
    }
}
