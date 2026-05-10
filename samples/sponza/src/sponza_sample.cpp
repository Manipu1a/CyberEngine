#include "sponza_sample.h"
#include "platform/memory.h"
#include "model_loader.h"
#include "texture_utils.h"
#include "gameruntime/pipeline_builder.h"
#include "graphics/interface/device_context.h"

namespace Cyber
{
    namespace Samples
    {
        Cyber::Samples::SampleApp* Cyber::Samples::SampleApp::create_sample_app()
        {
            return Cyber::cyber_new<Cyber::Samples::SponzaApp>();
        }

        SponzaApp::SponzaApp() {}
        SponzaApp::~SponzaApp() {}

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

        void SponzaApp::on_load_data()
        {
            // CPU-only: parse glTF, load meshes/materials (runs on loading thread)
            ModelLoader::ModelCreateInfo create_info;
            create_info.file_path = "../../../../samples/sponza/assets/main_sponza/NewSponza_Main_glTF_003.gltf";
            model = cyber_new<ModelLoader::Model>(create_info);
            model->load_data(create_info);
        }

        void SponzaApp::on_create_resources()
        {
            // GPU: create textures and buffers (runs on main thread)
            auto render_device = get_render_device();

            if (!model || !model->is_valid())
            {
                cyber_error("Failed to load Sponza model");
                return;
            }

            model->create_gpu_textures(render_device);

            // Convert vertices to our format
            auto vertex_count = model->get_vertex_count();
            auto index_count = model->get_index_count();
            auto model_verts = model->get_vertex_data();
            auto model_indices = model->get_index_data();

            eastl::vector<SponzaVertex> vertices(vertex_count);
            for (size_t i = 0; i < vertex_count; ++i)
            {
                vertices[i].position = model_verts[i].pos;
                vertices[i].normal = model_verts[i].normal;
                vertices[i].uv = model_verts[i].uv0;
            }

            eastl::vector<uint32_t> indices(index_count);
            for (size_t i = 0; i < index_count; ++i)
                indices[i] = model_indices[i];

            create_vertex_buffer(vertices.data(), vertex_count * sizeof(SponzaVertex), vertex_buffer);
            create_index_buffer(indices.data(), index_count * sizeof(uint32_t), index_buffer);

            // Create scene constant buffer
            create_constant_buffer(sizeof(SceneConstants), scene_cbuffer);

            // Build draw primitives from meshes
            auto& meshes = model->get_meshes();
            auto& materials = model->get_materials();

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
                            auto* tex = model->get_texture(tex_id);
                            if (tex)
                                dp.base_color_view = tex->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                    }

                    draw_primitives.push_back(dp);
                }
            }
        }

        void SponzaApp::update(float deltaTime)
        {
            // Simple camera orbit
            static float time = 0.0f;
            time += deltaTime;

            float radius = 120.0f;
            camera_pos = float3(
                radius * cosf(camera_yaw),
                50.0f,
                radius * sinf(camera_yaw)
            );
            float3 camera_target = float3(0.0f, 30.0f, 0.0f);
            float3 camera_up = { 0.0f, 1.0f, 0.0f };
            float4x4 view_matrix = float4x4::look_at(camera_pos, camera_target, camera_up);
            float4x4 projection_matrix = get_renderer()->get_adjusted_projection_matrix(PI_ / 4.0f, 0.1f, 1000.0f);

            float4x4 model_matrix = float4x4::scale(float4(scene_scale, scene_scale, scene_scale, 1.0f));

            // Update scene constants
            SceneConstants scene_data;
            scene_data.view_proj_matrix = (view_matrix * projection_matrix).transpose();
            scene_data.model_matrix = model_matrix.transpose();
            scene_data.camera_pos = float4(camera_pos, 1.0f);
            scene_data.light_direction = float4(light_direction, 0.0f);
            scene_data.light_color = float4(light_color, 1.0f);
            update_buffer(scene_cbuffer, scene_data);

            // Render
            auto ctx = begin_frame(render_pass);
            auto device_context = ctx.device_context;

            device_context->render_encoder_bind_pipeline(pipeline);

            RenderObject::IBuffer* vb[] = { vertex_buffer };
            uint32_t strides[] = { sizeof(SponzaVertex) };
            device_context->render_encoder_bind_vertex_buffer(1, vb, strides, nullptr);
            device_context->render_encoder_bind_index_buffer(index_buffer, sizeof(uint32_t), 0);

            // Bind scene constant buffer
            device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, scene_cbuffer);
            device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, scene_cbuffer);

            // Draw each primitive
            for (auto& dp : draw_primitives)
            {
                if (dp.base_color_view)
                    device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, dp.base_color_view);

                device_context->prepare_for_rendering();
                device_context->render_encoder_draw_indexed(dp.index_count, dp.first_index, 0);
            }

            end_frame();
        }

        void SponzaApp::draw_ui(ImGuiContext* in_imgui_context)
        {
            if (in_imgui_context)
            {
                ImGui::SetCurrentContext(in_imgui_context);

                if (ImGui::Begin("Sponza Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if (ImGui::TreeNode("Camera"))
                    {
                        ImGui::SliderFloat("Yaw", &camera_yaw, -PI_, PI_);
                        ImGui::SliderFloat("Speed", &camera_speed, 1.0f, 50.0f);
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Lighting"))
                    {
                        ImGui::ColorEdit3("Light Color", light_color.data());
                        float3 dir = light_direction;
                        ImGui::SliderFloat3("Light Dir", dir.data(), -1.0f, 1.0f);
                        light_direction = normalize(dir);
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Scene"))
                    {
                        ImGui::SliderFloat("Scale", &scene_scale, 0.01f, 1.0f, "%.3f");
                        ImGui::Text("Primitives: %u", (uint32_t)draw_primitives.size());
                        ImGui::TreePop();
                    }
                }
                ImGui::End();
            }
        }
    }
}
