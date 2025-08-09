#include "pbr_demo.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "application/application.h"
#include "texture_utils.h"
#include "resource/vertex.h"
#include "component/light_component.h"
#include "component/camera_component.h"

namespace Cyber
{
    namespace Samples
    {
        const static CubeVertex cube_verts[8] = {
            { {-1, -1, -1}},
            { {-1, 1, -1}},
            { {1, 1, -1}},
            { {1, -1, -1}},

            { {-1, -1, 1}},
            { {-1, 1, 1}},
            { {1, 1, 1}},
            { {1, -1, 1}},
        };

        const static uint32_t cube_indices[36] = {
        2,0,1, 2,3,0,
        4,6,5, 4,7,6,
        0,7,4, 0,3,7,
        1,0,4, 1,4,5,
        1,5,2, 5,6,2,
        3,6,7, 3,2,6
        };

        Cyber::Samples::SampleApp* Cyber::Samples::SampleApp::create_sample_app()
        {
            return Cyber::cyber_new<Cyber::Samples::PBRApp>();
        }

        PBRApp::PBRApp()
        {

        }

        PBRApp::~PBRApp()
        {
            
        }

        void PBRApp::initialize()
        {
            SampleApp::initialize();
            m_pApp = Cyber::Core::Application::getApp();
            cyber_check(m_pApp);
            create_gfx_objects();

            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            camera_component = cyber_new<Component::CameraComponent>();

            create_resource();
            create_render_pipeline();

            precompute_environment_map();
        }

        void PBRApp::run()
        {
            //m_pApp->run();
        }

        void PBRApp::update(float deltaTime)
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            static float time = 0.0f;
            time += deltaTime;

            float4x4 InvYAxis = float4x4::Identity();
            InvYAxis.m[1][1] = -1.0f;
            Math::Quaternion<float> rotation_x = Math::Quaternion<float>::rotation_from_axis_angle({ 1.0f, 0.0f, 0.0f }, -PI_F / 2.0f);
            Math::Quaternion<float> rotation_y = Math::Quaternion<float>::rotation_from_axis_angle({ 0.0f, 1.0f, 0.0f }, PI_F);
            float4x4 rotation_matrix = rotation_x.to_matrix();
            float4x4 model_matrix = float4x4::scale(model_scale) * InvYAxis * rotation_matrix * float4x4::translation(model_position.x, model_position.y, model_position.z);
            //model_matrix = float4x4::scale(0.7)* float4x4::RotationY(static_cast<float>(time) * 1.0f) * float4x4::RotationX(static_cast<float>(time) * 1.0f);

            float3 camera_target = { 0.0f, 0.0f, 0.0f };
            float3 camera_up = { 0.0f, 1.0f, 0.0f };
            
            float4x4 view_matrix = float4x4::look_at(camera_position, camera_target, camera_up);
            float4x4 projection_matrix = renderer->get_adjusted_projection_matrix(PI_ / 4.0f, 0.1f, 100.0f);
            float4x4 view_projection_matrix = view_matrix * projection_matrix;
            //float4x4 world_view_proj_matrix = model_matrix * view_matrix * projection_matrix;

            for(auto& binding : model_resource_bindings)
            {
                // map vertex constant buffer
                void* const_resource = render_device->map_buffer(binding.vertex_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
                ConstantMatrix* const_ptr = (ConstantMatrix*)const_resource;
                const_ptr->ModelMatrix = model_matrix.transpose();
                const_ptr->ViewMatrix = view_matrix.transpose();
                const_ptr->ProjectionMatrix = projection_matrix.transpose();
                render_device->unmap_buffer(binding.vertex_constant_buffer, MAP_WRITE);
                binding.vertex_constant_buffer->set_buffer_size(sizeof(ConstantMatrix));
            }

            Component::LightAttribs light_attribs;
            light_attribs.light_direction = light_direction;
            light_attribs.light_intensity = light_color;
            light_attribs.camera_position = float4(camera_position, 1.0f);

            void* light_resource = render_device->map_buffer(light_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            Component::LightAttribs* light_ptr = (Component::LightAttribs*)light_resource;
            *light_ptr = light_attribs;
            render_device->unmap_buffer(light_constant_buffer, MAP_WRITE);

            void* camera_resource = render_device->map_buffer(camera_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            Component::CameraAttribs* camera_ptr = (Component::CameraAttribs*)camera_resource;
            camera_ptr->camera_position = float4(0.0f, 0.0f, -10.0f, 1.0f);
            //camera_ptr->view_matrix = view_matrix;
            //camera_ptr->projection_matrix = projection_matrix;
            //camera_ptr->view_projection_matrix = view_matrix * projection_matrix;
            camera_ptr->inverse_view_projection_matrix = view_projection_matrix.inverse();
            render_device->unmap_buffer(camera_constant_buffer, MAP_WRITE);

            raster_draw();
        }

        void PBRApp::raster_draw()
        {
            //precompute_environment_map();

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

            // record
            device_context->cmd_begin();

            auto clear_value =  GRAPHICS_CLEAR_VALUE{ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f};
            GRAPHICS_CLEAR_VALUE* clear_values = { &clear_value };
            RenderObject::ITexture_View* attachment_resources[2] = { back_buffer_view, back_depth_buffer_view};
            frame_buffer->update_attachments(attachment_resources, 2);
            RenderObject::BeginRenderPassAttribs RenderPassBeginInfo
            {
                .pFramebuffer = frame_buffer,
                .pRenderPass = render_pass,
                .ClearValueCount = 1,
                .color_clear_values = &clear_value,
                .depth_stencil_clear_value = { 1.0f, 0 },
                .TransitionMode = RenderObject::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
            };

            TextureBarrier draw_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
            };
            TextureBarrier depth_barrier = {
                .texture = back_depth_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE,
                .subresource_barrier = 0
            };
            
            ResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc0);
            ResourceBarrierDesc barrier_desc1 = { .texture_barriers = &depth_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc1);

            device_context->cmd_begin_render_pass(RenderPassBeginInfo);
            device_context->set_frame_buffer(frame_buffer);
            RenderObject::Viewport viewport;
            viewport.top_left_x = 0.0f;
            viewport.top_left_y = 0.0f;
            viewport.width = back_buffer->get_create_desc().m_width;
            viewport.height = back_buffer->get_create_desc().m_height;
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;
            device_context->render_encoder_set_viewport(1, &viewport);
            RenderObject::Rect scissor
            {
                0, 0, 
                (int32_t)back_buffer->get_create_desc().m_width, 
                (int32_t)back_buffer->get_create_desc().m_height
            };

            device_context->render_encoder_set_scissor( 1, &scissor);

            for(const auto& binding : model_resource_bindings)
            {
                RenderObject::IBuffer* vertex_buffers[] = { binding.vertex_buffer };
                uint32_t strides[] = { sizeof(ModelVertex) };
                device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
                device_context->render_encoder_bind_index_buffer(binding.index_buffer, sizeof(uint32_t), 0);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, binding.vertex_constant_buffer);
                device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, light_constant_buffer);
                const auto& meshs = binding.model->get_meshes();
                for (const auto& mesh : meshs)
                {
                    if(mesh.primitives.size() > 0)
                    {
                        const auto& primitives = mesh.primitives;
                        for (const auto& primitive : primitives)
                        {
                            auto& primitive_material = binding.material_bindings[primitive.material_id];
                            device_context->render_encoder_bind_pipeline( primitive_material.model_pipeline);
                            bind_material_resources(device_context, primitive_material);
                            device_context->prepare_for_rendering();
                            device_context->render_encoder_draw_indexed(primitive.index_count, primitive.first_index, 0);
                        }
                    }
                }
            }
            
            device_context->cmd_next_sub_pass();
            // draw environment map
            device_context->render_encoder_set_viewport(1, &viewport);
            device_context->render_encoder_set_scissor( 1, &scissor);
            device_context->render_encoder_bind_pipeline( environment_pipeline);
            device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, camera_constant_buffer);
            device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, environment_texture_view);
            device_context->prepare_for_rendering();
            device_context->render_encoder_draw(3, 0);

            device_context->cmd_end_render_pass();

            TextureBarrier present_barrier = {
                .texture = back_buffer,
                .src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE
            };
            ResourceBarrierDesc barrier_desc2 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
            device_context->cmd_resource_barrier(barrier_desc2);
        }

        void PBRApp::present()
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

        void PBRApp::create_gfx_objects()
        {
            //m_pApp->get_renderer()->create_gfx_objects();
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();
            auto device_context = renderer->get_device_context();
            //RenderObject::RenderPassAttachmentDesc attachments[1] = {};
            attachment_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            RenderObject::AttachmentReference attachment_ref[4];
            attachment_ref[0].m_attachmentIndex = 0;
            attachment_ref[0].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[0].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[0].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[0].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[0].m_finalState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            
            attachment_ref[1].m_attachmentIndex = 1;
            attachment_ref[1].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[1].m_loadAction = LOAD_ACTION_CLEAR;
            attachment_ref[1].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[1].m_initialState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_ref[1].m_finalState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            attachment_ref[1].m_stencilLoadAction = LOAD_ACTION_DONT_CARE;
            attachment_ref[1].m_stencilStoreAction = STORE_ACTION_STORE;

            // for environment map
            attachment_ref[2].m_attachmentIndex = 0;
            attachment_ref[2].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[2].m_loadAction = LOAD_ACTION_LOAD;
            attachment_ref[2].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[2].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[2].m_finalState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[3].m_attachmentIndex = 1;
            attachment_ref[3].m_sampleCount = SAMPLE_COUNT_1;
            attachment_ref[3].m_loadAction = LOAD_ACTION_LOAD;
            attachment_ref[3].m_storeAction = STORE_ACTION_STORE;
            attachment_ref[3].m_initialState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[3].m_finalState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
            attachment_ref[3].m_stencilLoadAction = LOAD_ACTION_DONT_CARE;
            attachment_ref[3].m_stencilStoreAction = STORE_ACTION_STORE;
            
            subpass_desc[0].m_name = u8"Main Subpass";
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = &attachment_ref[1];
            subpass_desc[0].m_renderTargetCount = 1;
            subpass_desc[0].m_pRenderTargetAttachments = &attachment_ref[0];
            
            subpass_desc[1].m_name = u8"Environment Map Subpass";
            subpass_desc[1].m_inputAttachmentCount = 0;
            subpass_desc[1].m_pInputAttachments = nullptr;
            subpass_desc[1].m_pDepthStencilAttachment = &attachment_ref[3];
            subpass_desc[1].m_renderTargetCount = 1;
            subpass_desc[1].m_pRenderTargetAttachments = &attachment_ref[2];
            
            RenderObject::RenderPassDesc rp_desc = {
                .m_name = u8"PBR RenderPass",
                .m_attachmentCount = 1,
                .m_pAttachments = &attachment_desc,
                .m_subpassCount = 2,
                .m_pSubpasses = subpass_desc
            };  

            render_pass = device_context->create_render_pass(rp_desc);
        }

        void PBRApp::create_resource()
        {
            auto render_device = m_pApp->get_renderer()->get_render_device();
            auto renderer = m_pApp->get_renderer();
            auto device_context = renderer->get_device_context();
            
            // load model
            {
                auto& model_resource_binding = model_resource_bindings.emplace_back();
                ModelLoader::ModelCreateInfo create_info;
                create_info.file_path = "../../../../samples/pbrdemo/assets/MetalRoughSpheres/MetalRoughSpheres.gltf";
                ModelLoader::Model* model_loader = cyber_new<ModelLoader::Model>(render_device, device_context, create_info);
                model_resource_binding.model = model_loader;

                auto model_bound_box = model_loader->compute_bounding_box(model_loader->get_root_transform());
                float max_dim = 0;
                float3 model_dim{model_bound_box.Max - model_bound_box.Min};
                max_dim = std::max(max_dim, model_dim.x);
                max_dim = std::max(max_dim, model_dim.y);
                max_dim = std::max(max_dim, model_dim.z);
                float model_scale = (1.0f / std::max(max_dim, 0.01f)) * 0.5f;
                float3 model_position = -model_bound_box.Min - 0.5f * model_dim;
                model_resource_binding.model_transform = model_loader->get_root_transform();

                if (!model_loader->is_valid())
                {
                    cyber_error(false, "Failed to load model: {0}", create_info.file_path);
                }
                auto& meshes = model_loader->get_meshes();
                if (meshes.empty())
                {
                    cyber_error(false, "No meshes found in the model: {0}", create_info.file_path);
                }
                auto& materials = model_loader->get_materials();

                auto vertex_count = model_loader->get_vertex_count();
                auto index_count = model_loader->get_index_count();
                auto model_verts = model_loader->get_vertex_data();
                auto model_indices = model_loader->get_index_data();

                ModelVertex* model_vertex = cyber_new_n<ModelVertex>(vertex_count);
                for(size_t i = 0; i < vertex_count; ++i)
                {
                    model_vertex[i].position = model_verts[i].pos; // Assuming position is in float3, adding w component
                    model_vertex[i].normal = model_verts[i].normal; // Default normal, can be adjusted based on model data.
                    model_vertex[i].tangent = model_verts[i].tangent;
                    model_vertex[i].uv = model_verts[i].uv0;
                }

                uint32_t* model_indices_data = cyber_new_n<uint32_t>(index_count);
                for(size_t i = 0; i < index_count; ++i)
                {
                    model_indices_data[i] = model_indices[i];
                }
                
                RenderObject::BufferCreateDesc buffer_desc = {};
                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER;
                buffer_desc.size = vertex_count * sizeof(ModelVertex);
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                RenderObject::BufferData vertex_buffer_data = {};
                vertex_buffer_data.data = model_vertex;
                vertex_buffer_data.data_size = vertex_count * sizeof(ModelVertex);
                model_resource_binding.vertex_buffer = render_device->create_buffer(buffer_desc, &vertex_buffer_data);
                
                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
                buffer_desc.size = index_count * sizeof(uint32_t);
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                RenderObject::BufferData index_buffer_data = {};
                index_buffer_data.data = model_indices_data;
                index_buffer_data.data_size = index_count * sizeof(uint32_t);
                model_resource_binding.index_buffer = render_device->create_buffer(buffer_desc, &index_buffer_data);

                buffer_desc.size = sizeof(ConstantMatrix);
                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC; 
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                model_resource_binding.vertex_constant_buffer = render_device->create_buffer(buffer_desc);

                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER;
                buffer_desc.size = 8 * sizeof(float3);
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                vertex_buffer_data.data = cube_verts;
                vertex_buffer_data.data_size = 8 * sizeof(float3);
                cube_vertex_buffer = render_device->create_buffer(buffer_desc, &vertex_buffer_data);

                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
                buffer_desc.size = 36 * sizeof(uint32_t);
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                index_buffer_data.data = cube_indices;
                index_buffer_data.data_size = 36 * sizeof(uint32_t);
                cube_index_buffer = render_device->create_buffer(buffer_desc, &index_buffer_data);

                buffer_desc.size = sizeof(ConstantMatrix);
                buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
                buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC; 
                buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
                cube_vertex_constant_buffer = render_device->create_buffer(buffer_desc);

                // create texture
                if(materials.size() > 0)
                {
                    for(size_t i = 0; i < materials.size(); ++i)
                    {
                        auto& material = materials[i];
                        auto& material_resource_binding = model_resource_binding.material_bindings.emplace_back();
                        material_resource_binding.attribs = material.attribs;

                        if(material.texture_ids[ModelLoader::DefaultBaseColorTextureAttribId] != -1)
                        {
                            material_resource_binding.base_color_texture_view = model_loader->get_texture(material.texture_ids[ModelLoader::DefaultBaseColorTextureAttribId])->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                        if(material.texture_ids[ModelLoader::DefaultNormalTextureAttribId] != -1)
                        {
                            material_resource_binding.normal_texture_view = model_loader->get_texture(material.texture_ids[ModelLoader::DefaultNormalTextureAttribId])->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                        if(material.texture_ids[ModelLoader::DefaultMetallicRoughnessTextureAttribId] != -1)
                        {
                            material_resource_binding.metallic_roughness_texture_view = model_loader->get_texture(material.texture_ids[ModelLoader::DefaultMetallicRoughnessTextureAttribId])->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                        if(material.texture_ids[ModelLoader::DefaultEmissiveTextureAttribId] != -1)
                        {
                            material_resource_binding.emissive_texture_view = model_loader->get_texture(material.texture_ids[ModelLoader::DefaultEmissiveTextureAttribId])->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                        if(material.texture_ids[ModelLoader::DefaultOcclusionTextureAttribId] != -1)
                        {
                            material_resource_binding.occlusion_texture_view = model_loader->get_texture(material.texture_ids[ModelLoader::DefaultOcclusionTextureAttribId])->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                        }
                    }
                }
            }
            
            RenderObject::BufferCreateDesc buffer_desc = {};
            buffer_desc.size = sizeof(Component::LightAttribs);
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            light_constant_buffer = render_device->create_buffer(buffer_desc);

            buffer_desc.size = sizeof(Component::CameraAttribs);
            camera_constant_buffer = render_device->create_buffer(buffer_desc);

            buffer_desc.size = sizeof(EnvironmentConstant);
            env_map_constant_buffer = render_device->create_buffer(buffer_desc);

            TextureLoader::TextureLoadInfo env_texture_load_info{
                "EnvironmentMap",
                GRAPHICS_RESOURCE_USAGE_IMMUTABLE,
                GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE,
                0,
                CPU_ACCESS_NONE,
                true,
                false,
                TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN,
                false,
                FILTER_TYPE::FILTER_TYPE_ANISOTROPIC
                };
            //texture_load_info.name = CYBER_UTF8("EnvironmentMap");
            RenderObject::ITexture* environment_texture = nullptr;
            TextureLoader::create_texture_from_file(
                "samples/pbrdemo/assets/minedump_flats_4k.hdr",
                env_texture_load_info,
                &environment_texture, render_device
            );

            environment_texture_view = environment_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
            uint32_t cube_size = environment_texture->get_create_desc().m_height / 2;
            RenderObject::TextureCreateDesc textureDesc = {};

            textureDesc.m_name = "IrradianceCube";
            textureDesc.m_width = irradiance_cube_size;
            textureDesc.m_height = irradiance_cube_size;
            textureDesc.m_depth = 1;
            textureDesc.m_arraySize = 6;
            textureDesc.m_mipLevels = 7;
            textureDesc.m_dimension = TEXTURE_DIMENSION::TEX_DIMENSION_CUBE;
            textureDesc.m_usage = GRAPHICS_RESOURCE_USAGE::GRAPHICS_RESOURCE_USAGE_DEFAULT;
            textureDesc.m_bindFlags = GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE | GRAPHICS_RESOURCE_BIND_RENDER_TARGET;
            textureDesc.m_cpuAccessFlags = CPU_ACCESS_NONE;
            textureDesc.m_format = TEX_FORMAT_RGBA32_FLOAT;
            textureDesc.m_sampleCount = SAMPLE_COUNT_1;
            irradiance_cube_texture = render_device->create_texture(textureDesc);

            textureDesc.m_name = "EnvironmentMapCube";
            textureDesc.m_width = cube_size;
            textureDesc.m_height = cube_size;
            textureDesc.m_depth = 1;
            textureDesc.m_arraySize = 6;
            textureDesc.m_mipLevels = 1;
            textureDesc.m_dimension = TEXTURE_DIMENSION::TEX_DIMENSION_CUBE;
            textureDesc.m_usage = GRAPHICS_RESOURCE_USAGE::GRAPHICS_RESOURCE_USAGE_DEFAULT;
            textureDesc.m_bindFlags = GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE | GRAPHICS_RESOURCE_BIND_RENDER_TARGET;
            textureDesc.m_cpuAccessFlags = CPU_ACCESS_NONE;
            textureDesc.m_format = TEX_FORMAT_RGBA32_FLOAT;
            textureDesc.m_sampleCount = SAMPLE_COUNT_1;
            environment_cube_texture = render_device->create_texture(textureDesc);
        }

        void PBRApp::precompute_environment_map()
        {
            auto render_device = m_pApp->get_renderer()->get_render_device();
            auto renderer = m_pApp->get_renderer();
            auto device_context = renderer->get_device_context();

            const eastl::array<float4x4, 6> rotation_matrices = {
                float4x4::RotationY(PI_ / 2.0f), // +X
                float4x4::RotationY(-PI_ / 2.0f), // -X
                float4x4::RotationX(-PI_ / 2.0f), // +Y
                float4x4::RotationX(PI_ / 2.0f), // -Y
                float4x4::Identity(), // +Z
                float4x4::RotationY(PI_) // -Z
            };
            
            if(equirectangular_to_cubemap_pipeline)
            {
                uint32_t height = environment_texture_view->get_create_desc().p_texture->get_create_desc().m_height;
                uint32_t cube_size = height / 2;

                RenderObject::Viewport viewport;
                viewport.top_left_x = 0.0f;
                viewport.top_left_y = 0.0f;
                viewport.width = (float)cube_size;
                viewport.height = (float)cube_size;
                viewport.min_depth = 0.0f;
                viewport.max_depth = 1.0f;

                RenderObject::Rect scissor = {
                    0, 0, 
                    (int32_t)cube_size, 
                    (int32_t)cube_size
                };

                TextureBarrier draw_barrier = {
                .texture = environment_cube_texture,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
                };

                ResourceBarrierDesc barrier_desc = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
                device_context->cmd_resource_barrier(barrier_desc);
                device_context->cmd_begin();
                device_context->render_encoder_bind_pipeline(equirectangular_to_cubemap_pipeline);
                uint32_t strides[] = { sizeof(float3) };
                device_context->render_encoder_bind_vertex_buffer(1, &cube_vertex_buffer, strides, nullptr);
                device_context->render_encoder_bind_index_buffer(cube_index_buffer, sizeof(uint32_t), 0);
                device_context->render_encoder_set_viewport(1, &viewport);
                device_context->render_encoder_set_scissor(1, &scissor);
                
                auto environment_cube_texture_view = environment_cube_texture->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
                {
                    for(uint32_t face = 0; face < 6; ++face)
                    {
                        void* const_resource = render_device->map_buffer(cube_vertex_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
                        ConstantMatrix* const_ptr = (ConstantMatrix*)const_resource;
                        const_ptr->ModelMatrix = float4x4::Identity();
                        const_ptr->ViewMatrix = rotation_matrices[face];
                        const_ptr->ProjectionMatrix = float4x4::Identity();
                        render_device->unmap_buffer(cube_vertex_constant_buffer, MAP_WRITE);
                        cube_vertex_constant_buffer->set_buffer_size(sizeof(ConstantMatrix));
                        RenderObject::TextureViewCreateDesc rtv_desc = {};
                        rtv_desc.p_texture = environment_cube_texture;
                        rtv_desc.view_type = TEXTURE_VIEW_RENDER_TARGET;
                        rtv_desc.format = environment_cube_texture->get_create_desc().m_format;
                        rtv_desc.dimension = TEX_DIMENSION_2D_ARRAY; // 重要：单个面是2D
                        rtv_desc.baseArrayLayer = face;        // 关键：指定要渲染的面
                        rtv_desc.arrayLayerCount = 1;          // 只渲染一个面
                        rtv_desc.baseMipLevel = 0;
                        rtv_desc.mipLevelCount = 1;
                        environment_cube_texture_view = render_device->create_texture_view(rtv_desc);

                        device_context->set_render_target(1, &environment_cube_texture_view, nullptr);
                        device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, cube_vertex_constant_buffer);
                        device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, environment_texture_view);
                        device_context->prepare_for_rendering();
                        device_context->render_encoder_draw_indexed(36, 0, 0);
                    }
                }

                draw_barrier.src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
                draw_barrier.dst_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;
                device_context->cmd_resource_barrier(barrier_desc);
                device_context->cmd_end();
                device_context->flush();
            }
            
            if(irradiance_pipeline)
            {
                auto environment_cube_texture_view = environment_cube_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                RenderObject::Viewport viewport;
                viewport.top_left_x = 0.0f;
                viewport.top_left_y = 0.0f;
                viewport.width = (float)irradiance_cube_size;
                viewport.height = (float)irradiance_cube_size;
                viewport.min_depth = 0.0f;
                viewport.max_depth = 1.0f;
                RenderObject::Rect scissor = {
                    0, 0, 
                    (int32_t)irradiance_cube_size, 
                    (int32_t)irradiance_cube_size
                };
                TextureBarrier draw_barrier = {
                .texture = irradiance_cube_texture,
                .src_state = GRAPHICS_RESOURCE_STATE_COMMON,
                .dst_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET,
                .subresource_barrier = 0
                };
                ResourceBarrierDesc barrier_desc = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
                device_context->cmd_resource_barrier(barrier_desc);
                device_context->cmd_begin();
                device_context->render_encoder_bind_pipeline(irradiance_pipeline);
                device_context->render_encoder_set_viewport(1, &viewport);
                device_context->render_encoder_set_scissor(1, &scissor);

                const auto& irradiance_cube_desc = irradiance_cube_texture->get_create_desc();
                auto irradiance_cube_texture_view = irradiance_cube_texture->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);

                for(uint32_t mip = 0; mip < irradiance_cube_desc.m_mipLevels; ++mip)
                {
                    for(uint32_t face = 0; face < 6; ++face)
                    {
                        void* mapped_data = render_device->map_buffer(env_map_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
                        EnvironmentConstant* precompute_attribs = (EnvironmentConstant*)mapped_data;
                        precompute_attribs->rotation = rotation_matrices[face];
                        render_device->unmap_buffer(env_map_constant_buffer, MAP_WRITE);
                        RenderObject::TextureViewCreateDesc rtv_desc = {};
                        rtv_desc.p_texture = irradiance_cube_texture;
                        rtv_desc.view_type = TEXTURE_VIEW_RENDER_TARGET;
                        rtv_desc.format = irradiance_cube_texture->get_create_desc().m_format;
                        rtv_desc.dimension = TEX_DIMENSION_2D_ARRAY; // 重要：单个面是2D
                        rtv_desc.baseArrayLayer = face;        // 关键：指定要渲染的面
                        rtv_desc.arrayLayerCount = 1;          // 只渲染一个面
                        rtv_desc.baseMipLevel = mip;
                        rtv_desc.mipLevelCount = 1;
                        irradiance_cube_texture_view = render_device->create_texture_view(rtv_desc);
                        device_context->set_render_target(1, &irradiance_cube_texture_view, nullptr);
                        
                        device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, env_map_constant_buffer);
                        device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, environment_cube_texture_view);
                        
                        device_context->prepare_for_rendering();
                        device_context->render_encoder_draw(4, 0);
                    }
                }
                draw_barrier.src_state = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
                draw_barrier.dst_state = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;
                device_context->cmd_resource_barrier(barrier_desc);
                device_context->cmd_end();
                device_context->flush();
            }
            
        }

        void PBRApp::bind_material_resources(RenderObject::IDeviceContext* device_context, const MaterialResourceBinding& material_binding)
        {
            uint32_t material_resource_start_index = 0;
            if(material_binding.base_color_texture_view && material_binding.material_usage.use_base_color_texture)
            {
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, material_resource_start_index++, material_binding.base_color_texture_view);
            }
            if(material_binding.metallic_roughness_texture_view && material_binding.material_usage.use_metallic_roughness_texture)
            {
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, material_resource_start_index++, material_binding.metallic_roughness_texture_view);
            }
            if(material_binding.normal_texture_view && material_binding.material_usage.use_normal_texture)
            {
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, material_resource_start_index++, material_binding.normal_texture_view);
            }
            if(material_binding.emissive_texture_view && material_binding.material_usage.use_emissive_texture)
            {
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, material_resource_start_index++, material_binding.emissive_texture_view);
            }
            if(material_binding.occlusion_texture_view && material_binding.material_usage.use_occlusion_texture)
            {
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, material_resource_start_index++, material_binding.occlusion_texture_view);
            }
            if(irradiance_cube_texture)
            {
                //auto environment_cube_texture_view = environment_cube_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                auto irradiance_cube_texture_view = irradiance_cube_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                device_context->set_shader_resource_view(SHADER_STAGE_FRAG, material_resource_start_index++, irradiance_cube_texture_view);
            }
        }

        void PBRApp::create_ui()
        {

        }

        void PBRApp::draw_ui(ImGuiContext* in_imgui_context)
        {
            if(in_imgui_context)
            {
                ImGui::SetCurrentContext(in_imgui_context);

                if(ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if(ImGui::TreeNode("Lightning"))
                    {
                        ImGui::ColorEdit3("Light Color", light_color.data());
                        ImGui::InputFloat3("Light Direction", light_direction.data());
                        ImGui::TreePop();
                    }

                    if(ImGui::TreeNode("Model Transform"))
                    {

                        ImGui::InputFloat3("Position", model_position.data());
                        ImGui::SliderFloat3("Rotation", model_rotation.data(), 0.0f, 360.0f);
                        ImGui::SliderFloat3("Scale", model_scale.data(), 0.0f, 10.0f);
                        ImGui::SliderFloat3("Camera Position", camera_position.data(), -100.0f, 100.0f);
                        ImGui::TreePop();
                    }
                }
                ImGui::End();
            }
        }

        eastl::vector<ShaderMacro> PBRApp::get_shader_macros(const MaterialResourceBinding& material_binding) const
        {
            eastl::vector<ShaderMacro> macros;
            macros.push_back({ "USE_NORMAL_MAP", material_binding.normal_texture_view == nullptr ? "0" : "1" });
            macros.push_back({ "USE_METALLIC_ROUGHNESS_MAP", material_binding.metallic_roughness_texture_view == nullptr ? "0" : "1" });
            macros.push_back({ "USE_EMISSIVE_MAP", material_binding.emissive_texture_view == nullptr ? "0" : "1" });
            macros.push_back({ "USE_OCCLUSION_MAP", material_binding.occlusion_texture_view == nullptr ? "0" : "1" });

            return macros;
        }

        void PBRApp::reflect_material(MaterialResourceBinding& material_binding, RenderObject::IShaderLibrary* shader_library)
        {
            if(shader_library)
            {
                shader_library->get_material_resource_usage(material_binding.material_usage);
            }
        }

        void PBRApp::create_render_pipeline()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

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

            RasterizerStateCreateDesc rasterizer_state_desc = {};
            rasterizer_state_desc.fill_mode = FILL_MODE_SOLID;
            rasterizer_state_desc.cull_mode = CULL_MODE_BACK;
            rasterizer_state_desc.front_face = FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer_state_desc.depth_bias = 0;
            rasterizer_state_desc.slope_scaled_depth_bias = 0;
            rasterizer_state_desc.enable_multisample = false;
            rasterizer_state_desc.enable_depth_clip = true;
            rasterizer_state_desc.enable_scissor = false;

            DepthStateCreateDesc depth_stencil_state_desc = {};
            depth_stencil_state_desc.depth_test = true;
            depth_stencil_state_desc.depth_write = true;
            depth_stencil_state_desc.depth_func = CMP_LESS_EQUAL;
            depth_stencil_state_desc.stencil_test = false;

            //todo: remove sampler to static
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

            RenderObject::ISampler* samplers[] = { sampler, sampler };

            const char8_t* sampler_names[] = { CYBER_UTF8("Texture_sampler") };
            auto& scene_target = renderer->get_scene_target(0);

            // Model Binding
            for(auto& model_binding : model_resource_bindings)
            {
                for(auto& material_binding : model_binding.material_bindings)
                {
                    auto shader_macros = get_shader_macros(model_binding.material_bindings[0]);
                    // create shader
                    ResourceLoader::ShaderLoadDesc vs_load_desc = {};
                    vs_load_desc.target = SHADER_TARGET_6_0;
                    vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                        .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/cube_vs.hlsl"),
                        .stage = SHADER_STAGE_VERT,
                        .macros = shader_macros,
                        .entry_point_name = CYBER_UTF8("VSMain"),
                    };
                    eastl::shared_ptr<RenderObject::IShaderLibrary> vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

                    ResourceLoader::ShaderLoadDesc ps_load_desc = {};
                    ps_load_desc.target = SHADER_TARGET_6_0;
                    ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                        .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/cube_ps.hlsl"),
                        .stage = SHADER_STAGE_FRAG,
                        .macros = shader_macros,
                        .entry_point_name = CYBER_UTF8("PSMain"),
                    };
                    eastl::shared_ptr<RenderObject::IShaderLibrary> ps_shader = ResourceLoader::add_shader(render_device, ps_load_desc);

                    reflect_material(material_binding, vs_shader.get());
                    reflect_material(material_binding, ps_shader.get());

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
                        {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ModelVertex, position)},
                        {"ATTRIB", 1, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ModelVertex, normal)},
                        {"ATTRIB", 2, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ModelVertex, tangent)},
                        {"ATTRIB", 3, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(ModelVertex, uv)},
                    };
                    RenderObject::VertexLayoutDesc vertex_layout_desc = {4, vertex_attributes};

                    RenderObject::RenderPipelineCreateDesc rp_desc = 
                    {
                        .vertex_shader = pipeline_shader_create_desc[0],
                        .pixel_shader = pipeline_shader_create_desc[1],
                        .vertex_layout = &vertex_layout_desc,
                        .blend_state = &blend_state_desc,
                        .depth_stencil_state = &depth_stencil_state_desc,
                        .rasterizer_state = &rasterizer_state_desc,
                        .m_staticSamplers = &sampler,
                        .m_staticSamplerNames = sampler_names,
                        .m_staticSamplerCount = 1,
                        .color_formats = &scene_target.color_buffer->get_create_desc().m_format,
                        .render_target_count = 1,
                        .depth_stencil_format = scene_target.depth_buffer->get_create_desc().m_format,
                        .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
                    };
                    material_binding.model_pipeline = render_device->create_render_pipeline(rp_desc);

                    vs_shader->free();
                    ps_shader->free();
                }
            }

            ResourceLoader::ShaderLoadDesc cube_vs_load_desc = {};
            cube_vs_load_desc.target = SHADER_TARGET_6_0;
            cube_vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/equirectangular_to_cubemap_vs.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> cube_vs_shader = ResourceLoader::add_shader(render_device, cube_vs_load_desc);

            ResourceLoader::ShaderLoadDesc cube_ps_load_desc = {};
            cube_ps_load_desc.target = SHADER_TARGET_6_0;
            cube_ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/equirectangular_to_cubemap_ps.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("PSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> cube_ps_shader = ResourceLoader::add_shader(render_device, cube_ps_load_desc);

            RenderObject::PipelineShaderCreateDesc* cube_pipeline_shader_create_desc[2];
            cube_pipeline_shader_create_desc[0] = new RenderObject::PipelineShaderCreateDesc();
            cube_pipeline_shader_create_desc[1] = new RenderObject::PipelineShaderCreateDesc();
            cube_pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            cube_pipeline_shader_create_desc[0]->m_library = cube_vs_shader;
            cube_pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("VSMain");
            cube_pipeline_shader_create_desc[1]->m_stage = SHADER_STAGE_FRAG;
            cube_pipeline_shader_create_desc[1]->m_library = cube_ps_shader;
            cube_pipeline_shader_create_desc[1]->m_entry = CYBER_UTF8("PSMain");

            RenderObject::VertexAttribute vertex_attributes[] = {
            {"ATTRIB", 0, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(ModelVertex, position)}
            };
            RenderObject::VertexLayoutDesc vertex_layout_desc = {1, vertex_attributes};

            DepthStateCreateDesc cube_map_depth_stencil_state_desc = {};
            cube_map_depth_stencil_state_desc.depth_test = false;
            cube_map_depth_stencil_state_desc.depth_write = false;
            cube_map_depth_stencil_state_desc.depth_func = CMP_ALWAYS;
            cube_map_depth_stencil_state_desc.stencil_test = false;

            RenderObject::RenderPipelineCreateDesc cube_rp_desc = 
            {
                .vertex_shader = cube_pipeline_shader_create_desc[0],
                .pixel_shader = cube_pipeline_shader_create_desc[1],
                .vertex_layout = &vertex_layout_desc,
                .blend_state = &blend_state_desc,
                .depth_stencil_state = &cube_map_depth_stencil_state_desc,
                .m_staticSamplers = &sampler,
                .m_staticSamplerNames = sampler_names,
                .m_staticSamplerCount = 1,
                .color_formats = &environment_cube_texture->get_create_desc().m_format,
                .render_target_count = 1,
                .depth_stencil_format = TEX_FORMAT_UNKNOWN,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
            };

            equirectangular_to_cubemap_pipeline = render_device->create_render_pipeline(cube_rp_desc);

            ResourceLoader::ShaderLoadDesc env_vs_load_desc = {};
            env_vs_load_desc.target = SHADER_TARGET_6_0;
            env_vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/environment_map_vs.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> env_vs_shader = ResourceLoader::add_shader(render_device, env_vs_load_desc);

            ResourceLoader::ShaderLoadDesc env_ps_load_desc = {};
            env_ps_load_desc.target = SHADER_TARGET_6_0;
            env_ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/environment_map_ps.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("PSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> env_ps_shader = ResourceLoader::add_shader(render_device, env_ps_load_desc);

            RenderObject::PipelineShaderCreateDesc* env_pipeline_shader_create_desc[2];
            env_pipeline_shader_create_desc[0] = new RenderObject::PipelineShaderCreateDesc();
            env_pipeline_shader_create_desc[1] = new RenderObject::PipelineShaderCreateDesc();
            env_pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            env_pipeline_shader_create_desc[0]->m_library = env_vs_shader;
            env_pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("VSMain");
            env_pipeline_shader_create_desc[1]->m_stage = SHADER_STAGE_FRAG;
            env_pipeline_shader_create_desc[1]->m_library = env_ps_shader;
            env_pipeline_shader_create_desc[1]->m_entry = CYBER_UTF8("PSMain");

            RenderObject::RenderPipelineCreateDesc env_rp_desc = 
            {
                .vertex_shader = env_pipeline_shader_create_desc[0],
                .pixel_shader = env_pipeline_shader_create_desc[1],
                .vertex_layout = nullptr,
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
            environment_pipeline = render_device->create_render_pipeline(env_rp_desc);

            ResourceLoader::ShaderLoadDesc irr_vs_load_desc = {};
            irr_vs_load_desc.target = SHADER_TARGET_6_0;
            irr_vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/cubemap.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> irr_vs_shader = ResourceLoader::add_shader(render_device, irr_vs_load_desc);

            ResourceLoader::ShaderLoadDesc irr_ps_load_desc = {};
            irr_ps_load_desc.target = SHADER_TARGET_6_0;
            irr_ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/compute_irradiance_map.hlsl"),
                .stage = SHADER_STAGE_FRAG,
                .entry_point_name = CYBER_UTF8("PSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> irr_ps_shader = ResourceLoader::add_shader(render_device, irr_ps_load_desc);
            RenderObject::PipelineShaderCreateDesc* irr_pipeline_shader_create_desc[2];
            irr_pipeline_shader_create_desc[0] = new RenderObject::PipelineShaderCreateDesc();
            irr_pipeline_shader_create_desc[1] = new RenderObject::PipelineShaderCreateDesc();
            irr_pipeline_shader_create_desc[0]->m_stage = SHADER_STAGE_VERT;
            irr_pipeline_shader_create_desc[0]->m_library = irr_vs_shader;
            irr_pipeline_shader_create_desc[0]->m_entry = CYBER_UTF8("VSMain");
            irr_pipeline_shader_create_desc[1]->m_stage = SHADER_STAGE_FRAG;
            irr_pipeline_shader_create_desc[1]->m_library = irr_ps_shader;
            irr_pipeline_shader_create_desc[1]->m_entry = CYBER_UTF8("PSMain");

            RenderObject::RenderPipelineCreateDesc irr_rp_desc = 
            {
                .vertex_shader = irr_pipeline_shader_create_desc[0],
                .pixel_shader = irr_pipeline_shader_create_desc[1],
                .vertex_layout = nullptr,
                .blend_state = &blend_state_desc,
                .depth_stencil_state = &cube_map_depth_stencil_state_desc,
                .m_staticSamplers = &sampler,
                .m_staticSamplerNames = sampler_names,
                .m_staticSamplerCount = 1,
                .color_formats = &irradiance_cube_texture->get_create_desc().m_format,
                .render_target_count = 1,
                .depth_stencil_format = TEX_FORMAT_UNKNOWN,
                .prim_topology = PRIM_TOPO_TRIANGLE_STRIP,
            };
            irradiance_pipeline = render_device->create_render_pipeline(irr_rp_desc);

            env_vs_shader->free();
            env_ps_shader->free();
        }

        void PBRApp::finalize()
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
            for(auto& model_binding : model_resource_bindings)
            {
                if(model_binding.vertex_buffer)
                {
                    render_device->free_buffer(model_binding.vertex_buffer);
                }
                if(model_binding.index_buffer)
                {
                    render_device->free_buffer(model_binding.index_buffer);
                }
                for(auto& material_binding : model_binding.material_bindings)
                {
                    if(material_binding.base_color_texture_view)
                    {
                        render_device->free_texture_view(material_binding.base_color_texture_view);
                    }
                    if(material_binding.normal_texture_view)
                    {
                        render_device->free_texture_view(material_binding.normal_texture_view);
                    }
                    if(material_binding.metallic_roughness_texture_view)
                    {
                        render_device->free_texture_view(material_binding.metallic_roughness_texture_view);
                    }
                    if(material_binding.emissive_texture_view)
                    {
                        render_device->free_texture_view(material_binding.emissive_texture_view);
                    }
                    if(material_binding.occlusion_texture_view)
                    {
                        render_device->free_texture_view(material_binding.occlusion_texture_view);
                    }
                }
            }
            render_device->free_device();
            render_device->free_instance(instance);
        }
    }
}