#include "shadow_sample.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "model_loader.h"
#include "application/application.h"
#include "texture_utils.h"
#include "resource/vertex.h"

namespace Cyber
{
    namespace Samples
    {
        Cyber::Samples::SampleApp* Cyber::Samples::SampleApp::create_sample_app()
        {
            return Cyber::cyber_new<Cyber::Samples::ShadowApp>();
        }

        ShadowApp::ShadowApp()
        {

        }

        ShadowApp::~ShadowApp()
        {
            
        }

        void ShadowApp::initialize()
        {
            SampleApp::initialize();
            m_pApp = Cyber::Core::Application::getApp();
            cyber_check(m_pApp);
            create_gfx_objects();

            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            create_render_pipeline();
            create_resource();
        }

        void ShadowApp::run()
        {
            //m_pApp->run();
        }

        void ShadowApp::update(float deltaTime)
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            static float time = 0.0f;
            time += deltaTime;
            float3 camera_pos = float3(0.0f, 5.0f, -10.0f);
            float3 camera_target = float3(0.0f, 0.0f, 0.0f);
            float3 camera_up = { 0.0f, 1.0f, 0.0f };
            //float4x4 view_matrix = float4x4::translation(-camera_pos.x, -camera_pos.y, -camera_pos.z);
            float4x4 view_matrix = float4x4::look_at(camera_pos, camera_target, camera_up);

            float4x4 model_matrix = float4x4::RotationY(static_cast<float>(time) * 1.0f);
            float4x4 projection_matrix = renderer->get_adjusted_projection_matrix(PI_ / 4.0f, 0.1f, 100.0f);
            float4x4 view_proj_matrix = model_matrix * view_matrix * projection_matrix;

            // Calculate shadow projection matrix from directional light
            float3 normalized_light_dir = normalize(light_direction);
            float3 light_space_x, light_space_y;
            float min_cmp = std::min(std::min(std::abs(light_direction.x), std::abs(light_direction.y)), std::abs(light_direction.z));
            if(min_cmp == std::abs(light_direction.x))
            {
                light_space_x = float3(1.0f, 0.0f, 0.0f);
            }
            else if(min_cmp == std::abs(light_direction.y))
            {
                light_space_x = float3(0.0f, 1.0f, 0.0f);
            }
            else
            {
                light_space_x = float3(0.0f, 0.0f, 1.0f);
            }
            light_space_y = normalize(cross(normalized_light_dir, light_space_x));
            light_space_x = normalize(cross(light_space_y, normalized_light_dir));
            float4x4 light_view_matrix = float4x4::view_from_basis(light_space_x, light_space_y, normalized_light_dir);

            float3 scene_center = float3(0.0f, 0.0f, 0.0f);
            float scene_radius = 5.0f;
            float3 min_xyz = scene_center - float3(scene_radius);
            float3 max_xyz = scene_center + float3(scene_radius, scene_radius, scene_radius * 5);
            float3 scene_extent = max_xyz - min_xyz;
            float4 light_space_scale;
            light_space_scale.x = 2.0f / scene_extent.x;
            light_space_scale.y = 2.0f / scene_extent.y;
            light_space_scale.z = 1.0f / scene_extent.z;
            float4 light_space_scale_bias;
            light_space_scale_bias.x = -min_xyz.x * light_space_scale.x - 1.f;
            light_space_scale_bias.y = -min_xyz.y * light_space_scale.y - 1.f;
            light_space_scale_bias.z = -min_xyz.z * light_space_scale.z;

            float4x4 scale_matrix = float4x4::scale(light_space_scale);
            float4x4 scale_bias_matrix = float4x4::translation(light_space_scale_bias.x, light_space_scale_bias.y, light_space_scale_bias.z);

            float4x4 shadow_projection_matrix = scale_matrix * scale_bias_matrix;

            // Shadow MVP matrix for cube: transforms from world space to light's clip space
            float4x4 shadow_mvp_matrix = model_matrix * light_view_matrix * shadow_projection_matrix;
            float4x4 shadow_view_proj_matrix = light_view_matrix * shadow_projection_matrix;
            
            // Update cube transform
            cube_item.model_matrix = model_matrix;
            
            // Update light constants
            void* light_resource = render_device->map_buffer(light_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            LightConstants* light_ptr = (LightConstants*)light_resource;
            light_ptr->light_direction = float4(-light_direction, 0.0f);
            light_ptr->light_color = float4(light_color, 1.0f);
            render_device->unmap_buffer(light_constant_buffer, MAP_WRITE);
            light_constant_buffer->set_buffer_size(sizeof(LightConstants));
            
            // Update render items' constants
            update_render_item_constants(cube_item, view_matrix * projection_matrix, shadow_view_proj_matrix);
            update_render_item_constants(plane_item, view_matrix * projection_matrix, shadow_view_proj_matrix);

            raster_draw();
        }

        void ShadowApp::raster_draw()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
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
            auto shadow_buffer_view = shadow_depth->get_default_texture_view(TEXTURE_VIEW_DEPTH_STENCIL);
            
            // record
            device_context->cmd_begin();

            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
            GRAPHICS_CLEAR_VALUE* clear_values = { &clear_value };
            RenderObject::ITexture_View* attachment_resources[3] = { back_buffer_view, back_depth_buffer_view, shadow_buffer_view };
            frame_buffer->update_attachments(attachment_resources, 3);
            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
                .ClearValueCount = 1,
                .color_clear_values = &clear_value,
                .depth_stencil_clear_value = { 1.0f, 0 },
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            // BeginRenderPass will handle transitions automatically with RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            device_context->set_frame_buffer(frame_buffer);
            // Shadow Pass
            device_context->cmd_begin_render_pass(RenderPassBeginInfo);
            RenderObject::Viewport viewport;
            viewport.top_left_x = 0.0f;
            viewport.top_left_y = 0.0f;
            viewport.width = shadow_depth->get_create_desc().m_width;
            viewport.height = shadow_depth->get_create_desc().m_height;
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;
            device_context->render_encoder_set_viewport(1, &viewport);
            RenderObject::Rect scissor
            {
                0, 0, 
                (int32_t)shadow_depth->get_create_desc().m_width, 
                (int32_t)shadow_depth->get_create_desc().m_height
            };
            
            device_context->render_encoder_set_scissor( 1, &scissor);
            device_context->render_encoder_bind_pipeline(shadow_pipeline);
            
            // Draw shadows for all render items
            draw_render_item(device_context, cube_item, true);
            
            // Base Pass
            device_context->cmd_next_sub_pass();
            viewport.width = back_buffer->get_create_desc().m_width;
            viewport.height = back_buffer->get_create_desc().m_height;
            device_context->render_encoder_set_viewport(1, &viewport);
            RenderObject::Rect scene_scissor
            {
                0, 0, 
                (int32_t)back_buffer->get_create_desc().m_width, 
                (int32_t)back_buffer->get_create_desc().m_height
            };
            device_context->render_encoder_set_scissor( 1, &scene_scissor);
            // Resources are already in correct states within render pass - no barriers needed
            device_context->render_encoder_bind_pipeline( pipeline);
            
            // Draw all render items
            draw_render_item(device_context, cube_item, false);
            draw_render_item(device_context, plane_item, false);
            
            device_context->cmd_end_render_pass();
            // EndRenderPass will handle transitions back to final state automatically
        }

        void ShadowApp::present()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            auto swap_chain = renderer->get_swap_chain();
            auto back_buffer = swap_chain->get_back_buffer(m_backBufferIndex);

            //device_context->cmd_end_render_pass();
            device_context->cmd_end();

            // submit
            device_context->flush();

            // present
            render_device->present(swap_chain);
        }

        void ShadowApp::create_gfx_objects()
        {
            //m_pApp->get_renderer()->create_gfx_objects();
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            //RenderObject::RenderPassAttachmentDesc attachments[1] = {};
            attachment_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            attachment_ref[0].m_attachmentIndex = 0;
            attachment_ref[0].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[0].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[0].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[0].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[0].m_finalState = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;
            
            attachment_ref[1].m_attachmentIndex = 1;
            attachment_ref[1].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[1].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[1].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[1].m_initialState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_ref[1].m_finalState = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            attachment_ref[2].m_attachmentIndex = 2;
            attachment_ref[2].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[2].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[2].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[2].m_initialState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_ref[2].m_finalState = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;

            subpass_desc[0].m_name = u8"Shadow Subpass";
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = &attachment_ref[2];
            subpass_desc[0].m_renderTargetCount = 0;
            subpass_desc[0].m_pRenderTargetAttachments = nullptr;
            
            subpass_desc[1].m_name = u8"Main Subpass";
            subpass_desc[1].m_inputAttachmentCount = 0;
            subpass_desc[1].m_pInputAttachments = nullptr;
            subpass_desc[1].m_pDepthStencilAttachment = &attachment_ref[1];
            subpass_desc[1].m_renderTargetCount = 1;
            subpass_desc[1].m_pRenderTargetAttachments = &attachment_ref[0];
            
            RenderObject::RenderPassDesc rp_desc1 = {
                .m_name = u8"Triangle RenderPass",
                .m_attachmentCount = 1,
                .m_pAttachments = &attachment_desc,
                .m_subpassCount = 2,
                .m_pSubpasses = subpass_desc
            };
            
            render_pass = device_context->create_render_pass(rp_desc1);
            auto& scene_target = renderer->get_scene_target(0);
            auto& depth_rt_desc = scene_target.depth_buffer->get_create_desc();
            RenderObject::TextureCreateDesc depth_buffer_desc;
            depth_buffer_desc.m_name = u8"ShadowDepthBuffer";
            depth_buffer_desc.m_format = TEX_FORMAT_D32_FLOAT;
            depth_buffer_desc.m_width = 512;
            depth_buffer_desc.m_height = 512;
            depth_buffer_desc.m_depth = 1;
            depth_buffer_desc.m_arraySize = 1;
            depth_buffer_desc.m_mipLevels = 1;
            depth_buffer_desc.m_dimension = TEX_DIMENSION_2D;
            depth_buffer_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
            depth_buffer_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL | GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
            depth_buffer_desc.m_pNativeHandle = nullptr;
            shadow_depth = render_device->create_texture(depth_buffer_desc);
        }

        void ShadowApp::create_resource()
        {
            auto render_device = m_pApp->get_renderer()->get_render_device();
            auto renderer = m_pApp->get_renderer();
            auto device_context = renderer->get_device_context();
            
            // Create plane vertices and indices
            CubeVertex plane_verts[4] = {
                { {-5.0f, -2.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
                { { 5.0f, -2.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {5.0f, 0.0f} },
                { { 5.0f, -2.0f,  5.0f}, {0.0f, 1.0f, 0.0f}, {5.0f, 5.0f} },
                { {-5.0f, -2.0f,  5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 5.0f} }
            };
            
            uint32_t plane_indices[6] = {
                1, 0, 2,
                2, 0, 3
            };
            
            // Create plane vertex buffer
            RenderObject::BufferCreateDesc plane_vb_desc = {};
            plane_vb_desc.bind_flags = GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER;
            plane_vb_desc.size = 4 * sizeof(CubeVertex);
            plane_vb_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
            plane_vb_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            RenderObject::BufferData plane_vertex_data = {};
            plane_vertex_data.data = plane_verts;
            plane_vertex_data.data_size = 4 * sizeof(CubeVertex);
            plane_item.vertex_buffer = render_device->create_buffer(plane_vb_desc, &plane_vertex_data);
            
            // Create plane index buffer
            RenderObject::BufferCreateDesc plane_ib_desc = {};
            plane_ib_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
            plane_ib_desc.size = 6 * sizeof(uint32_t);
            plane_ib_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
            plane_ib_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            RenderObject::BufferData plane_index_data = {};
            plane_index_data.data = plane_indices;
            plane_index_data.data_size = 6 * sizeof(uint32_t);
            plane_item.index_buffer = render_device->create_buffer(plane_ib_desc, &plane_index_data);
            plane_item.index_count = 6;
            
            // load model
            ModelLoader::ModelCreateInfo create_info;
            create_info.file_path = "../../../../samples/shadow/assets/Cube/Cube.gltf";
            ModelLoader::Model* model_loader = cyber_new<ModelLoader::Model>(render_device, device_context, create_info);
            if (!model_loader->is_valid())
            {
                cyber_error("Failed to load model: {0}", create_info.file_path);
            }
            auto& meshes = model_loader->get_meshes();
            if (meshes.empty())
            {
                cyber_error("No meshes found in the model: {0}", create_info.file_path);
            }

            auto vertex_count = model_loader->get_vertex_count();
            auto index_count = model_loader->get_index_count();
            auto model_verts = model_loader->get_vertex_data();
            auto model_indices = model_loader->get_index_data();

            CubeVertex* cube_verts = cyber_new_n<CubeVertex>(vertex_count);
            for(size_t i = 0; i < vertex_count; ++i)
            {
                cube_verts[i].position = model_verts[i].pos; // Assuming position is in float3, adding w component
                cube_verts[i].normal = model_verts[i].normal; // Default normal, can be adjusted based on model data
                cube_verts[i].uv = model_verts[i].uv0;
            }

            uint32_t* cube_indices = cyber_new_n<uint32_t>(index_count);
            for(size_t i = 0; i < index_count; ++i)
            {
                cube_indices[i] = model_indices[i];
            }
            
            RenderObject::BufferCreateDesc buffer_desc = {};
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER;
            buffer_desc.size = vertex_count * sizeof(CubeVertex);
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            RenderObject::BufferData vertex_buffer_data = {};
            vertex_buffer_data.data = cube_verts;
            vertex_buffer_data.data_size = vertex_count * sizeof(CubeVertex);
            cube_item.vertex_buffer = render_device->create_buffer(buffer_desc, &vertex_buffer_data);
            
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
            buffer_desc.size = index_count * sizeof(uint32_t);
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            RenderObject::BufferData index_buffer_data = {};
            index_buffer_data.data = cube_indices;
            index_buffer_data.data_size = index_count * sizeof(uint32_t);
            cube_item.index_buffer = render_device->create_buffer(buffer_desc, &index_buffer_data);
            cube_item.index_count = index_count;

            buffer_desc.size = sizeof(ViewConstants);
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            cube_item.constant_buffer = render_device->create_buffer(buffer_desc);

            buffer_desc.size = sizeof(float4x4);
            cube_item.shadow_constant_buffer = render_device->create_buffer(buffer_desc);
            
            // Create plane constant buffers
            buffer_desc.size = sizeof(ViewConstants);
            plane_item.constant_buffer = render_device->create_buffer(buffer_desc);
            buffer_desc.size = sizeof(float4x4);
            plane_item.shadow_constant_buffer = render_device->create_buffer(buffer_desc);

            buffer_desc.size = sizeof(LightConstants);
            light_constant_buffer = render_device->create_buffer(buffer_desc);

            auto& materials = model_loader->get_materials();
            // create texture
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

        void ShadowApp::create_ui()
        {

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
                        {
                            light_direction = transform;
                        }

                        ImGui::TreePop();
                    }
                }
                ImGui::End();
            }
        }

        void ShadowApp::create_render_pipeline()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            // create shader
            ResourceLoader::ShaderLoadDesc vs_load_desc = {};
            vs_load_desc.target = SHADER_TARGET_6_0;
            vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/shadow/assets/shaders/cube_vs.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

            ResourceLoader::ShaderLoadDesc ps_load_desc = {};
            ps_load_desc.target = SHADER_TARGET_6_0;
            ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/shadow/assets/shaders/cube_ps.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("PSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> ps_shader = ResourceLoader::add_shader(render_device, ps_load_desc);

            RenderObject::SamplerCreateDesc sampler_create_desc = {};
            sampler_create_desc.min_filter = FILTER_TYPE_LINEAR;
            sampler_create_desc.mag_filter = FILTER_TYPE_LINEAR;
            sampler_create_desc.mip_filter = FILTER_TYPE_LINEAR;
            sampler_create_desc.address_u = ADDRESS_MODE_WRAP;
            sampler_create_desc.address_v = ADDRESS_MODE_WRAP;
            sampler_create_desc.address_w = ADDRESS_MODE_WRAP;
            sampler_create_desc.flags = SAMPLER_FLAG_NONE;
            sampler_create_desc.unnormalized_coordinates = false;
            sampler_create_desc.mip_lod_bias = 0.0f;
            sampler_create_desc.max_anisotropy = 0;
            sampler_create_desc.compare_mode = CMP_NEVER;
            sampler_create_desc.border_color = { 0.0f, 0.0f, 0.0f, 0.0f };
            sampler_create_desc.min_lod = 0.0f;
            sampler_create_desc.max_lod = 0.0f;
            auto sampler = render_device->create_sampler(sampler_create_desc);
            const char8_t* sampler_names[] = { CYBER_UTF8("Texture_sampler") };

            // create root signature
            RenderObject::PipelineShaderCreateDesc* pipeline_shader_create_desc[2];
            pipeline_shader_create_desc[0] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            pipeline_shader_create_desc[0]->m_library = vs_shader;
            pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("VSMain");
            pipeline_shader_create_desc[1] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[1]->m_stage = SHADER_STAGE_FRAG;
            pipeline_shader_create_desc[1]->m_library = ps_shader;
            pipeline_shader_create_desc[1]->m_entry = CYBER_UTF8("PSMain");
            
            RenderObject::VertexAttribute vertex_attributes[] = {
                {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, position)},
                {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, normal)},
                {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, uv)},
            };

            RenderObject::VertexLayoutDesc vertex_layout_desc = {3, vertex_attributes};
            
            BlendStateCreateDesc blend_state_desc = {};
            blend_state_desc.render_target_count = 1;
            blend_state_desc.src_factors[0] = BLEND_CONSTANT_ONE;
            blend_state_desc.dst_factors[0] = BLEND_CONSTANT_ZERO;
            blend_state_desc.blend_modes[0] = BLEND_MODE_ADD;
            blend_state_desc.src_alpha_factors[0] = BLEND_CONSTANT_ONE;
            blend_state_desc.dst_alpha_factors[0] = BLEND_CONSTANT_ZERO;
            blend_state_desc.blend_alpha_modes[0] = BLEND_MODE_ADD;
            blend_state_desc.alpha_to_coverage = false;
            blend_state_desc.masks[0] = COLOR_WRITE_MASK_ALL;

            DepthStateCreateDesc depth_stencil_state_desc = {};
            depth_stencil_state_desc.depth_test = true;
            depth_stencil_state_desc.depth_write = true;
            depth_stencil_state_desc.depth_func = CMP_LESS_EQUAL;
            depth_stencil_state_desc.stencil_test = false;

            auto& scene_target = renderer->get_scene_target(0);
            
            RenderObject::RenderPipelineCreateDesc rp_desc = 
            {
                .vertex_shader = pipeline_shader_create_desc[0],
                .pixel_shader = pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout_desc,
                .blend_state = &blend_state_desc,
                .depth_stencil_state = &depth_stencil_state_desc,
                .m_staticSamplers = &sampler,
                .m_staticSamplerNames = sampler_names,
                .m_staticSamplerCount = 1,
                .color_formats = &scene_target.color_buffer->get_create_desc().m_format,
                .render_target_count = 1,
                .depth_stencil_format = scene_target.depth_buffer->get_create_desc().m_format,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
            };
            pipeline = render_device->create_render_pipeline(rp_desc);

            vs_shader->free();
            ps_shader->free();

            create_shadow_pipeline();
        }

        void ShadowApp::create_shadow_pipeline()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            // create shader
            ResourceLoader::ShaderLoadDesc vs_load_desc = {};
            vs_load_desc.target = SHADER_TARGET_6_0;
            vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/shadow/assets/shaders/shadow.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("main"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

            RenderObject::PipelineShaderCreateDesc* pipeline_shader_create_desc[1];
            pipeline_shader_create_desc[0] = cyber_new<RenderObject::PipelineShaderCreateDesc>();
            pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            pipeline_shader_create_desc[0]->m_library = vs_shader;
            pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("VSMain");

            RenderObject::VertexAttribute vertex_attributes[] = {
                {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, position)},
                {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, normal)},
                {"ATTRIB", 2, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, uv)},
            };
            RenderObject::VertexLayoutDesc vertex_layout_desc = {3, vertex_attributes};

            RenderObject::RenderPipelineCreateDesc rp_desc = 
            {
                .vertex_shader = pipeline_shader_create_desc[0],
                .vertex_layout = &vertex_layout_desc,
                .depth_stencil_format = shadow_depth->get_create_desc().m_format,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
            };
            shadow_pipeline = render_device->create_render_pipeline(rp_desc);

            vs_shader->free();
        }

        void ShadowApp::draw_render_item(RenderObject::IDeviceContext* device_context, const RenderItem& item, bool is_shadow_pass)
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            
            // Bind vertex and index buffers
            RenderObject::IBuffer* vertex_buffers[] = { item.vertex_buffer };
            uint32_t strides[] = { item.vertex_stride };
            device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
            device_context->render_encoder_bind_index_buffer(item.index_buffer, sizeof(uint32_t), 0);
            
            if (is_shadow_pass)
            {
                // Shadow pass only needs shadow matrix
                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, item.shadow_constant_buffer);
            }
            else
            {
                // Main render pass needs full constants
                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, item.constant_buffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, light_constant_buffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 1, item.constant_buffer);
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, test_texture_view);
                auto shadow_depth_srv = shadow_depth->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 1, shadow_depth_srv);
            }
            
            device_context->prepare_for_rendering();
            device_context->render_encoder_draw_indexed(item.index_count, 0, 0);
        }
        
        void ShadowApp::update_render_item_constants(const RenderItem& item, const float4x4& view_proj_matrix, const float4x4& shadow_view_proj_matrix)
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            
            // Update main constants
            float4x4 mvp_matrix = item.model_matrix * view_proj_matrix;
            
            void* const_resource = render_device->map_buffer(item.constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            ViewConstants* const_ptr = (ViewConstants*)const_resource;
            const_ptr->model_matrix = item.model_matrix.transpose();
            const_ptr->view_projection_matrix = mvp_matrix.transpose();
            const_ptr->shadow_matrix = shadow_view_proj_matrix.transpose();
            render_device->unmap_buffer(item.constant_buffer, MAP_WRITE);
            item.constant_buffer->set_buffer_size(sizeof(ViewConstants));
            
            // Update shadow constants
            float4x4 shadow_mvp_matrix = item.model_matrix * shadow_view_proj_matrix;
            
            void* shadow_resource = render_device->map_buffer(item.shadow_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            float4x4* shadow_ptr = (float4x4*)shadow_resource;
            *shadow_ptr = shadow_mvp_matrix;
            render_device->unmap_buffer(item.shadow_constant_buffer, MAP_WRITE);
            item.shadow_constant_buffer->set_buffer_size(sizeof(float4x4));
        }

        void ShadowApp::finalize()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto swap_chain = renderer->get_swap_chain();
            auto renderpass = renderer->get_render_pass();
            auto surface = renderer->get_surface();
            auto instance = renderer->get_instance();
            for(uint32_t i = 0;i < swap_chain->get_buffer_srv_count(); ++i)
            {
                render_device->free_texture_view(swap_chain->get_back_buffer_srv_view(i));
            }
            render_device->free_swap_chain(swap_chain);
            render_device->free_surface(surface);
            render_device->free_render_pipeline(pipeline);
            render_device->free_root_signature(root_signature);
            render_device->free_device();
            render_device->free_instance(instance);
        }
    }
}