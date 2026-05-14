#include "shadow_sample.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "model_loader.h"
#include "application/application.h"
#include "texture_utils.h"
#include "resource/vertex.h"
#include "gameruntime/pipeline_builder.h"

namespace Cyber
{
    // Helper: row-vector * matrix multiplication (float4 * float4x4)
    static float4 mul_vec_mat(const float4& v, const float4x4& m)
    {
        return float4(
            v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
            v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
            v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
            v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33
        );
    }

    namespace Samples
    {
        CYBER_GAME_API SampleApp* create_shadow_app()
        {
            return Cyber::cyber_new<Cyber::Samples::ShadowApp>();
        }

        ShadowApp::ShadowApp()
        {
        }

        ShadowApp::~ShadowApp()
        {
        }

        void ShadowApp::calculate_cascade_splits(float near_plane, float far_plane, float splits[MAX_CASCADE_COUNT])
        {
            for (uint32_t i = 0; i < cascade_count; ++i)
            {
                float p = (float)(i + 1) / (float)cascade_count;
                float log_split = near_plane * std::pow(far_plane / near_plane, p);
                float linear_split = near_plane + (far_plane - near_plane) * p;
                splits[i] = split_lambda * log_split + (1.0f - split_lambda) * linear_split;
            }
        }

        void ShadowApp::calculate_cascade_matrix(const float4x4& camera_view, const float4x4& camera_proj,
                                                   float near_split, float far_split,
                                                   const float3& light_dir,
                                                   float4x4& out_light_vp)
        {
            float4x4 inv_view_proj = (camera_view * camera_proj).inverse();

            float3 ndc_corners[8] = {
                {-1, -1, 0}, { 1, -1, 0}, { 1,  1, 0}, {-1,  1, 0},
                {-1, -1, 1}, { 1, -1, 1}, { 1,  1, 1}, {-1,  1, 1},
            };

            float3 world_corners[8];
            for (int i = 0; i < 8; ++i)
            {
                float4 corner = float4(ndc_corners[i], 1.0f);
                float4 world = mul_vec_mat(corner, inv_view_proj);
                world_corners[i] = float3(world.x / world.w, world.y / world.w, world.z / world.w);
            }

            float3 cascade_corners[8];
            for (int i = 0; i < 4; ++i)
            {
                float3 near_corner = world_corners[i];
                float3 far_corner = world_corners[i + 4];
                float3 dir = far_corner - near_corner;
                cascade_corners[i] = near_corner + dir * near_split;
                cascade_corners[i + 4] = near_corner + dir * far_split;
            }

            float3 frustum_center = float3(0, 0, 0);
            for (int i = 0; i < 8; ++i)
                frustum_center = frustum_center + cascade_corners[i];
            frustum_center = frustum_center * (1.0f / 8.0f);

            float3 normalized_light_dir = normalize(light_dir);
            float3 light_space_x, light_space_y;
            float min_cmp = std::min(std::min(std::abs(light_dir.x), std::abs(light_dir.y)), std::abs(light_dir.z));
            if (min_cmp == std::abs(light_dir.x))
                light_space_x = float3(1.0f, 0.0f, 0.0f);
            else if (min_cmp == std::abs(light_dir.y))
                light_space_x = float3(0.0f, 1.0f, 0.0f);
            else
                light_space_x = float3(0.0f, 0.0f, 1.0f);
            light_space_y = normalize(cross(normalized_light_dir, light_space_x));
            light_space_x = normalize(cross(light_space_y, normalized_light_dir));
            float4x4 light_view = float4x4::view_from_basis(light_space_x, light_space_y, normalized_light_dir);

            float min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
            float max_x = -FLT_MAX, max_y = -FLT_MAX, max_z = -FLT_MAX;
            for (int i = 0; i < 8; ++i)
            {
                float4 ls = mul_vec_mat(float4(cascade_corners[i], 1.0f), light_view);
                min_x = std::min(min_x, ls.x); max_x = std::max(max_x, ls.x);
                min_y = std::min(min_y, ls.y); max_y = std::max(max_y, ls.y);
                min_z = std::min(min_z, ls.z); max_z = std::max(max_z, ls.z);
            }

            float z_margin = (max_z - min_z) * 2.0f;
            min_z -= z_margin;

            float extent_x = max_x - min_x;
            float extent_y = max_y - min_y;

            float texels_per_unit_x = (float)shadow_resolution / extent_x;
            float texels_per_unit_y = (float)shadow_resolution / extent_y;

            min_x = std::floor(min_x * texels_per_unit_x) / texels_per_unit_x;
            max_x = min_x + extent_x;
            min_y = std::floor(min_y * texels_per_unit_y) / texels_per_unit_y;
            max_y = min_y + extent_y;

            float4 light_space_scale;
            light_space_scale.x = 2.0f / (max_x - min_x);
            light_space_scale.y = 2.0f / (max_y - min_y);
            light_space_scale.z = 1.0f / (max_z - min_z);

            float4 light_space_scale_bias;
            light_space_scale_bias.x = -min_x * light_space_scale.x - 1.0f;
            light_space_scale_bias.y = -min_y * light_space_scale.y - 1.0f;
            light_space_scale_bias.z = -min_z * light_space_scale.z;

            float4x4 scale_matrix = float4x4::scale(light_space_scale);
            float4x4 scale_bias_matrix = float4x4::translation(light_space_scale_bias.x, light_space_scale_bias.y, light_space_scale_bias.z);

            out_light_vp = light_view * scale_matrix * scale_bias_matrix;
        }

        void ShadowApp::update(float deltaTime)
        {
            if (needs_rebuild)
            {
                rebuild_shadow_resources();
                needs_rebuild = false;
            }

            auto render_device = get_render_device();

            static float time = 0.0f;
            time += deltaTime;
            float3 camera_pos = float3(0.0f, 5.0f, -10.0f);
            float3 camera_target = float3(0.0f, 0.0f, 0.0f);
            float3 camera_up = { 0.0f, 1.0f, 0.0f };
            float4x4 view_matrix = float4x4::look_at(camera_pos, camera_target, camera_up);

            float4x4 model_matrix = float4x4::RotationY(static_cast<float>(time) * 1.0f);
            float4x4 projection_matrix = get_renderer()->get_adjusted_projection_matrix(PI_ / 4.0f, camera_near, camera_far);
            float4x4 view_proj_matrix = model_matrix * view_matrix * projection_matrix;

            // Calculate cascade splits
            float splits[MAX_CASCADE_COUNT] = {};
            calculate_cascade_splits(camera_near, camera_far, splits);

            // Calculate per-cascade light view-projection matrices
            float4x4 cascade_vp[MAX_CASCADE_COUNT] = {};
            for (uint32_t i = 0; i < cascade_count; ++i)
            {
                float near_frac = (i == 0) ? 0.0f : (splits[i - 1] - camera_near) / (camera_far - camera_near);
                float far_frac = (splits[i] - camera_near) / (camera_far - camera_near);
                calculate_cascade_matrix(view_matrix, projection_matrix, near_frac, far_frac, light_direction, cascade_vp[i]);
            }

            cube_item.model_matrix = model_matrix;

            // Update light constants
            LightConstants light_data;
            light_data.light_direction = float4(-light_direction, 0.0f);
            light_data.light_color = float4(light_color, 1.0f);
            update_buffer(light_constant_buffer, light_data);

            // Update CSM constants
            CSMConstants csm_data = {};
            for (uint32_t i = 0; i < MAX_CASCADE_COUNT; ++i)
            {
                if (i < cascade_count)
                    csm_data.cascade_view_proj_matrices[i] = cascade_vp[i].transpose();
                else
                    csm_data.cascade_view_proj_matrices[i] = float4x4::Identity();
            }
            csm_data.cascade_splits = float4(
                cascade_count > 0 ? splits[0] : camera_far,
                cascade_count > 1 ? splits[1] : camera_far,
                cascade_count > 2 ? splits[2] : camera_far,
                cascade_count > 3 ? splits[3] : camera_far
            );
            csm_data.shadow_params = float4(
                (float)shadow_resolution,
                1.0f / (float)shadow_resolution,
                (float)cascade_count,
                depth_bias
            );
            csm_data.debug_params = float4(show_cascade_debug ? 1.0f : 0.0f, 0, 0, 0);
            update_buffer(csm_constant_buffer, csm_data);

            // Update render items' constants
            update_render_item_constants(cube_item, view_matrix, view_matrix * projection_matrix, cascade_vp);
            update_render_item_constants(plane_item, view_matrix, view_matrix * projection_matrix, cascade_vp);

            raster_draw();
        }

        void ShadowApp::raster_draw()
        {
            auto renderer = get_renderer();
            auto render_device = get_render_device();
            auto device_context = get_device_context();
            auto swap_chain = renderer->get_swap_chain();
            auto frame_buffer = renderer->get_frame_buffer();

            AcquireNextDesc acquire_desc = {};
            m_backBufferIndex = render_device->acquire_next_image(swap_chain, acquire_desc);
            renderer->set_back_buffer_index(m_backBufferIndex);
            auto& scene_target = renderer->get_scene_target(m_backBufferIndex);
            auto back_buffer = scene_target.color_buffer;
            auto back_depth_buffer = scene_target.depth_buffer;
            auto back_buffer_view = scene_target.color_buffer->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
            auto back_depth_buffer_view = scene_target.depth_buffer->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);

            // Build attachment list: [color, scene_depth, cascade0_dsv, cascade1_dsv, ...]
            uint32_t total_attachments = 2 + cascade_count;
            eastl::vector<RenderObject::ITexture_View*> attachment_resources(total_attachments);
            attachment_resources[0] = back_buffer_view;
            attachment_resources[1] = back_depth_buffer_view;
            for (uint32_t i = 0; i < cascade_count; ++i)
                attachment_resources[2 + i] = cascade_dsv_views[i];

            device_context->cmd_begin();

            auto clear_value = GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
            frame_buffer->update_attachments(attachment_resources.data(), total_attachments);
            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
                .ClearValueCount = 1,
                .color_clear_values = &clear_value,
                .depth_stencil_clear_value = { 1.0f, 0 },
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            device_context->set_frame_buffer(frame_buffer);
            device_context->cmd_begin_render_pass(RenderPassBeginInfo);

            // Shadow passes for each cascade
            for (uint32_t cascade = 0; cascade < cascade_count; ++cascade)
            {
                if (cascade > 0)
                    device_context->cmd_next_sub_pass();

                // Update cascade index buffer
                uint32_t cascade_data[4] = { cascade, 0, 0, 0 };
                update_buffer(cascade_index_buffer, cascade_data, sizeof(cascade_data));

                RenderObject::Viewport viewport;
                viewport.top_left_x = 0.0f;
                viewport.top_left_y = 0.0f;
                viewport.width = (float)shadow_resolution;
                viewport.height = (float)shadow_resolution;
                viewport.min_depth = 0.0f;
                viewport.max_depth = 1.0f;
                device_context->render_encoder_set_viewport(1, &viewport);
                RenderObject::Rect scissor{ 0, 0, (int32_t)shadow_resolution, (int32_t)shadow_resolution };
                device_context->render_encoder_set_scissor(1, &scissor);
                device_context->render_encoder_bind_pipeline(shadow_pipeline);

                draw_render_item(device_context, cube_item, true, cascade);
                draw_render_item(device_context, plane_item, true, cascade);
            }

            // Main Pass
            device_context->cmd_next_sub_pass();
            RenderObject::Viewport main_viewport;
            main_viewport.top_left_x = 0.0f;
            main_viewport.top_left_y = 0.0f;
            main_viewport.width = (float)back_buffer->get_create_desc().m_width;
            main_viewport.height = (float)back_buffer->get_create_desc().m_height;
            main_viewport.min_depth = 0.0f;
            main_viewport.max_depth = 1.0f;
            device_context->render_encoder_set_viewport(1, &main_viewport);
            RenderObject::Rect scene_scissor
            {
                0, 0,
                (int32_t)back_buffer->get_create_desc().m_width,
                (int32_t)back_buffer->get_create_desc().m_height
            };
            device_context->render_encoder_set_scissor(1, &scene_scissor);
            device_context->render_encoder_bind_pipeline(pipeline);

            draw_render_item(device_context, cube_item, false);
            draw_render_item(device_context, plane_item, false);

            device_context->cmd_end_render_pass();
        }

        void ShadowApp::on_create_gfx_objects()
        {
            auto render_device = get_render_device();
            auto device_context = get_device_context();
            auto& scene_target = get_renderer()->get_scene_target(m_backBufferIndex);
            auto back_buffer = scene_target.color_buffer;
            auto back_depth_buffer = scene_target.depth_buffer;

            // Create shadow depth texture array
            RenderObject::TextureCreateDesc depth_buffer_desc;
            depth_buffer_desc.m_name = u8"CSMShadowDepthArray";
            depth_buffer_desc.m_format = TEX_FORMAT_D32_FLOAT;
            depth_buffer_desc.m_width = shadow_resolution;
            depth_buffer_desc.m_height = shadow_resolution;
            depth_buffer_desc.m_depth = 1;
            depth_buffer_desc.m_arraySize = cascade_count;
            depth_buffer_desc.m_mipLevels = 1;
            depth_buffer_desc.m_dimension = TEX_DIMENSION_2D_ARRAY;
            depth_buffer_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
            depth_buffer_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL | GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
            depth_buffer_desc.m_pNativeHandle = nullptr;
            render_device->create_texture(depth_buffer_desc, nullptr, &shadow_depth);

            // Create per-cascade DSV views
            for (uint32_t i = 0; i < cascade_count; ++i)
            {
                RenderObject::TextureViewCreateDesc dsv_desc = {};
                dsv_desc.name = u8"CascadeDSV";
                dsv_desc.p_texture = shadow_depth;
                dsv_desc.format = TEX_FORMAT_D32_FLOAT;
                dsv_desc.view_type = TEXTURE_VIEW_DEPTH_STENCIL;
                dsv_desc.dimension = TEX_DIMENSION_2D_ARRAY;
                dsv_desc.baseArrayLayer = i;
                dsv_desc.arrayLayerCount = 1;
                dsv_desc.baseMipLevel = 0;
                dsv_desc.mipLevelCount = 1;
                cascade_dsv_views[i] = render_device->create_texture_view(dsv_desc);
            }

            shadow_array_srv = shadow_depth->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);

            // Build render pass with cascade_count + 1 subpasses
            uint32_t total_attachments = 2 + cascade_count;
            eastl::vector<RenderObject::RenderPassAttachmentDesc> attachment_desc(total_attachments);

            attachment_desc[0].format = back_buffer->get_create_desc().m_format;
            attachment_desc[0].sample_count = SAMPLE_COUNT_1;
            attachment_desc[0].load_action = LOAD_ACTION_CLEAR;
            attachment_desc[0].store_action = STORE_ACTION_STORE;
            attachment_desc[0].initial_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_desc[0].final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            attachment_desc[1].format = back_depth_buffer->get_create_desc().m_format;
            attachment_desc[1].sample_count = SAMPLE_COUNT_1;
            attachment_desc[1].load_action = LOAD_ACTION_CLEAR;
            attachment_desc[1].store_action = STORE_ACTION_STORE;
            attachment_desc[1].initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_desc[1].final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            for (uint32_t i = 0; i < cascade_count; ++i)
            {
                attachment_desc[2 + i].format = TEX_FORMAT_D32_FLOAT;
                attachment_desc[2 + i].sample_count = SAMPLE_COUNT_1;
                attachment_desc[2 + i].load_action = LOAD_ACTION_CLEAR;
                attachment_desc[2 + i].store_action = STORE_ACTION_STORE;
                attachment_desc[2 + i].initial_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
                attachment_desc[2 + i].final_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;
            }

            uint32_t total_subpasses = cascade_count + 1;

            eastl::vector<RenderObject::AttachmentReference> cascade_depth_refs(cascade_count);
            for (uint32_t i = 0; i < cascade_count; ++i)
                cascade_depth_refs[i] = { 2 + i, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };

            RenderObject::AttachmentReference main_color_ref = { 0, GRAPHICS_RESOURCE_STATE_RENDER_TARGET };
            RenderObject::AttachmentReference main_depth_ref = { 1, GRAPHICS_RESOURCE_STATE_DEPTH_WRITE };

            eastl::vector<RenderObject::AttachmentReference> main_input_refs(cascade_count);
            for (uint32_t i = 0; i < cascade_count; ++i)
                main_input_refs[i] = { 2 + i, GRAPHICS_RESOURCE_STATE_INPUT_ATTACHMENT };

            eastl::vector<RenderObject::RenderSubpassDesc> subpass_descs(total_subpasses);

            for (uint32_t i = 0; i < cascade_count; ++i)
            {
                subpass_descs[i].m_name = u8"Shadow Cascade Subpass";
                subpass_descs[i].m_inputAttachmentCount = 0;
                subpass_descs[i].m_pInputAttachments = nullptr;
                subpass_descs[i].m_pDepthStencilAttachment = &cascade_depth_refs[i];
                subpass_descs[i].m_renderTargetCount = 0;
                subpass_descs[i].m_pRenderTargetAttachments = nullptr;
            }

            subpass_descs[cascade_count].m_name = u8"Main Subpass";
            subpass_descs[cascade_count].m_inputAttachmentCount = cascade_count;
            subpass_descs[cascade_count].m_pInputAttachments = main_input_refs.data();
            subpass_descs[cascade_count].m_pDepthStencilAttachment = &main_depth_ref;
            subpass_descs[cascade_count].m_renderTargetCount = 1;
            subpass_descs[cascade_count].m_pRenderTargetAttachments = &main_color_ref;

            RenderObject::RenderPassDesc rp_desc = {
                .m_name = u8"CSM RenderPass",
                .m_attachmentCount = total_attachments,
                .m_pAttachments = attachment_desc.data(),
                .m_subpassCount = total_subpasses,
                .m_pSubpasses = subpass_descs.data()
            };

            device_context->create_render_pass(rp_desc, &render_pass);
        }

        void ShadowApp::on_create_resources()
        {
            auto render_device = get_render_device();
            auto device_context = get_device_context();

            // Create plane geometry
            CubeVertex plane_verts[4] = {
                { {-5.0f, -2.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
                { { 5.0f, -2.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {5.0f, 0.0f} },
                { { 5.0f, -2.0f,  5.0f}, {0.0f, 1.0f, 0.0f}, {5.0f, 5.0f} },
                { {-5.0f, -2.0f,  5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 5.0f} }
            };
            uint32_t plane_indices[6] = { 1, 0, 2, 2, 0, 3 };

            create_vertex_buffer(plane_verts, sizeof(plane_verts), plane_item.vertex_buffer);
            create_index_buffer(plane_indices, sizeof(plane_indices), plane_item.index_buffer);
            plane_item.index_count = 6;

            // Load cube model
            ModelLoader::ModelCreateInfo create_info;
            create_info.file_path = "../../../../samples/shadow/assets/Cube/Cube.gltf";
            ModelLoader::Model* model_loader = cyber_new<ModelLoader::Model>(render_device, device_context, create_info);
            if (!model_loader->is_valid())
                cyber_error("Failed to load model: {0}", create_info.file_path);

            auto vertex_count = model_loader->get_vertex_count();
            auto index_count = model_loader->get_index_count();
            auto model_verts = model_loader->get_vertex_data();
            auto model_indices = model_loader->get_index_data();

            CubeVertex* cube_verts = cyber_new_n<CubeVertex>(vertex_count);
            for(size_t i = 0; i < vertex_count; ++i)
            {
                cube_verts[i].position = model_verts[i].pos;
                cube_verts[i].normal = model_verts[i].normal;
                cube_verts[i].uv = model_verts[i].uv0;
            }
            uint32_t* cube_indices = cyber_new_n<uint32_t>(index_count);
            for(size_t i = 0; i < index_count; ++i)
                cube_indices[i] = model_indices[i];

            create_vertex_buffer(cube_verts, vertex_count * sizeof(CubeVertex), cube_item.vertex_buffer);
            create_index_buffer(cube_indices, index_count * sizeof(uint32_t), cube_item.index_buffer);
            cube_item.index_count = index_count;

            // Constant buffers
            create_constant_buffer(sizeof(ViewConstants), cube_item.constant_buffer);
            create_constant_buffer(sizeof(ShadowMVPConstants), cube_item.shadow_constant_buffer);
            create_constant_buffer(sizeof(ViewConstants), plane_item.constant_buffer);
            create_constant_buffer(sizeof(ShadowMVPConstants), plane_item.shadow_constant_buffer);
            create_constant_buffer(sizeof(LightConstants), light_constant_buffer);
            create_constant_buffer(sizeof(CSMConstants), csm_constant_buffer);
            create_constant_buffer(sizeof(uint32_t) * 4, cascade_index_buffer);

            // Load texture from model materials
            auto& materials = model_loader->get_materials();
            if(materials.size() > 0)
            {
                for(size_t i = 0; i < materials.size(); ++i)
                {
                    auto& material = materials[i];
                    if(material.texture_ids[ModelLoader::DefaultBaseColorTextureAttribId] != -1)
                    {
                        test_texture_view = model_loader->get_texture(material.texture_ids[ModelLoader::DefaultBaseColorTextureAttribId])->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                    }
                }
            }
        }

        void ShadowApp::on_create_pipelines()
        {
            auto render_device = get_render_device();
            auto& scene_target = get_renderer()->get_scene_target(0);
            auto sampler = create_default_sampler();

            RenderObject::VertexAttribute vertex_attributes[] = {
                {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, position)},
                {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, normal)},
                {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, uv)},
            };

            pipeline = PipelineBuilder(render_device)
                .vertex_shader(CYBER_UTF8("samples/shadow/assets/shaders/cube_vs.hlsl"))
                .pixel_shader(CYBER_UTF8("samples/shadow/assets/shaders/cube_ps.hlsl"))
                .vertex_layout(vertex_attributes, 3)
                .static_sampler(CYBER_UTF8("Texture_sampler"), sampler)
                .blend_opaque()
                .depth_test()
                .render_target_format(scene_target.color_buffer->get_create_desc().m_format)
                .depth_format(scene_target.depth_buffer->get_create_desc().m_format)
                .build();

            create_shadow_pipeline();
        }

        void ShadowApp::create_shadow_pipeline()
        {
            auto render_device = get_render_device();

            RenderObject::VertexAttribute vertex_attributes[] = {
                {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, position)},
                {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, normal)},
                {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, uv)},
            };

            shadow_pipeline = PipelineBuilder(render_device)
                .vertex_shader(CYBER_UTF8("samples/shadow/assets/shaders/shadow.hlsl"), CYBER_UTF8("main"))
                .vertex_layout(vertex_attributes, 3)
                .depth_test()
                .depth_format(shadow_depth->get_create_desc().m_format)
                .build();
        }

        void ShadowApp::draw_render_item(RenderObject::IDeviceContext* device_context, const RenderItem& item, bool is_shadow_pass, uint32_t cascade_index)
        {
            RenderObject::IBuffer* vertex_buffers[] = { item.vertex_buffer };
            uint32_t strides[] = { item.vertex_stride };
            device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
            device_context->render_encoder_bind_index_buffer(item.index_buffer, sizeof(uint32_t), 0);

            if (is_shadow_pass)
            {
                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, item.shadow_constant_buffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 1, cascade_index_buffer);
            }
            else
            {
                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, item.constant_buffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, light_constant_buffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 1, csm_constant_buffer);
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, test_texture_view);
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 1, shadow_array_srv);
            }

            device_context->prepare_for_rendering();
            device_context->render_encoder_draw_indexed(item.index_count, 0, 0);
        }

        void ShadowApp::update_render_item_constants(const RenderItem& item, const float4x4& view_matrix, const float4x4& view_proj_matrix, const float4x4 cascade_shadow_vp[MAX_CASCADE_COUNT])
        {
            // Update main constants
            ViewConstants view_data;
            view_data.model_matrix = item.model_matrix.transpose();
            view_data.view_projection_matrix = (item.model_matrix * view_proj_matrix).transpose();
            view_data.view_matrix = view_matrix.transpose();
            update_buffer(item.constant_buffer, view_data);

            // Update shadow MVP constants
            ShadowMVPConstants shadow_data = {};
            for (uint32_t i = 0; i < MAX_CASCADE_COUNT; ++i)
            {
                if (i < cascade_count)
                    shadow_data.shadow_mvps[i] = item.model_matrix * cascade_shadow_vp[i];
                else
                    shadow_data.shadow_mvps[i] = float4x4::Identity();
            }
            update_buffer(item.shadow_constant_buffer, shadow_data);
        }

        void ShadowApp::rebuild_shadow_resources()
        {
            auto device_context = get_device_context();
            device_context->flush();

            for (uint32_t i = 0; i < MAX_CASCADE_COUNT; ++i)
                cascade_dsv_views[i].reset();
            shadow_array_srv.reset();
            shadow_depth.reset();
            render_pass.reset();

            on_create_gfx_objects();
            create_shadow_pipeline();
        }

        void ShadowApp::draw_ui(ImGuiContext* in_imgui_context)
        {
            if(in_imgui_context)
            {
                ImGui::SetCurrentContext(in_imgui_context);
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::BeginFrame();

                if(ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if(ImGui::TreeNode("Lightning"))
                    {
                        ImGui::ColorEdit3("Light Color", light_color.data());
                        float3 transform = light_direction;
                        if(ImGui::gizmo3D("Gizmo", transform))
                            light_direction = transform;
                        ImGui::TreePop();
                    }

                    if(ImGui::TreeNode("CSM Settings"))
                    {
                        int count = (int)cascade_count;
                        if (ImGui::SliderInt("Cascade Count", &count, 1, (int)MAX_CASCADE_COUNT))
                        {
                            cascade_count = (uint32_t)count;
                            needs_rebuild = true;
                        }

                        ImGui::SliderFloat("Split Lambda", &split_lambda, 0.0f, 1.0f, "%.2f");

                        const char* res_items[] = { "256", "512", "1024", "2048" };
                        uint32_t res_values[] = { 256, 512, 1024, 2048 };
                        int current_res = 2;
                        for (int i = 0; i < 4; ++i)
                            if (res_values[i] == shadow_resolution) current_res = i;
                        if (ImGui::Combo("Resolution", &current_res, res_items, 4))
                        {
                            shadow_resolution = res_values[current_res];
                            needs_rebuild = true;
                        }

                        ImGui::SliderFloat("Depth Bias", &depth_bias, 0.0f, 0.01f, "%.4f");
                        ImGui::SliderFloat("Near Plane", &camera_near, 0.01f, 1.0f, "%.2f");
                        ImGui::SliderFloat("Far Plane", &camera_far, 10.0f, 500.0f, "%.1f");
                        ImGui::Checkbox("Debug Cascade Colors", &show_cascade_debug);

                        float splits[MAX_CASCADE_COUNT] = {};
                        calculate_cascade_splits(camera_near, camera_far, splits);
                        ImGui::Separator();
                        ImGui::Text("Cascade Splits:");
                        for (uint32_t i = 0; i < cascade_count; ++i)
                        {
                            float near_dist = (i == 0) ? camera_near : splits[i - 1];
                            ImGui::Text("  Cascade %d: %.2f - %.2f", i, near_dist, splits[i]);
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::End();
            }
        }
    }
}
