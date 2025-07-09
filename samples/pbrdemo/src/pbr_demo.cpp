#include "pbr_demo.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_resource.h"
#include "rendergraph/render_graph_builder.h"
#include "resource/resource_loader.h"
#include "model_loader.h"
#include "application/application.h"
#include "texture_utils.h"
#include "resource/vertex.h"
#include "component/light_component.h"
#include "component/camera_component.h"

namespace Cyber
{
    namespace Samples
    {
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

            create_render_pipeline();
            create_resource();
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
            float4x4 model_matrix = float4x4::RotationY(static_cast<float>(time) * 1.0f);
            float4x4 view_matrix = float4x4::translation(0.0f, 0.0f, 5.0f);
            float4x4 projection_matrix = renderer->get_adjusted_projection_matrix(PI_ / 4.0f, 0.1f, 100.0f);
            float4x4 world_view_proj_matrix = model_matrix * view_matrix * projection_matrix;
            // map vertex constant buffer
            void* const_resource = render_device->map_buffer(vertex_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            ConstantMatrix* const_ptr = (ConstantMatrix*)const_resource;
            const_ptr->ModelMatrix = model_matrix;
            const_ptr->ViewMatrix = view_matrix;
            const_ptr->ProjectionMatrix = projection_matrix;
            render_device->unmap_buffer(vertex_constant_buffer, MAP_WRITE);
            vertex_constant_buffer->set_buffer_size(sizeof(ConstantMatrix));

            Component::LightAttribs light_attribs;
            light_attribs.light_direction = LightDirection;
            light_attribs.light_intensity = LightColor;

            void* light_resource = render_device->map_buffer(light_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            Component::LightAttribs* light_ptr = (Component::LightAttribs*)light_resource;
            *light_ptr = light_attribs;
            render_device->unmap_buffer(light_constant_buffer, MAP_WRITE);

            raster_draw();
        }

        void PBRApp::raster_draw()
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
            RenderObject::IBuffer* vertex_buffers[] = { vertex_buffer };
            uint32_t strides[] = { sizeof(CubeVertex) };
            device_context->render_encoder_bind_vertex_buffer(1, vertex_buffers, strides, nullptr);
            device_context->render_encoder_bind_index_buffer(index_buffer, sizeof(uint32_t), 0);
            device_context->render_encoder_bind_pipeline( pipeline);
            device_context->set_root_constant_buffer_view(SHADER_STAGE_VERT, 0, vertex_constant_buffer);
            device_context->set_root_constant_buffer_view(SHADER_STAGE_FRAG, 0, light_constant_buffer);
            device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 0, base_color_texture_view);
            device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 1, normal_texture_view);
            device_context->set_shader_resource_view(SHADER_STAGE_FRAG, 2, environment_texture_view);
            device_context->prepare_for_rendering();
            device_context->render_encoder_draw_indexed(36, 0, 0);
            device_context->cmd_next_sub_pass();
            // draw environment map
            device_context->render_encoder_set_viewport(1, &viewport);
            device_context->render_encoder_set_scissor( 1, &scissor);
            device_context->render_encoder_bind_pipeline( environment_pipeline);
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

            subpass_desc[0].m_name = u8"Main Subpass";
            subpass_desc[0].m_inputAttachmentCount = 0;
            subpass_desc[0].m_pInputAttachments = nullptr;
            subpass_desc[0].m_pDepthStencilAttachment = &attachment_ref[1];
            subpass_desc[0].m_renderTargetCount = 1;
            subpass_desc[0].m_pRenderTargetAttachments = &attachment_ref[0];
            
            subpass_desc[1].m_name = u8"Environment Map Subpass";
            subpass_desc[1].m_inputAttachmentCount = 0;
            subpass_desc[1].m_pInputAttachments = nullptr;
            subpass_desc[1].m_pDepthStencilAttachment = nullptr;
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
            ModelLoader::ModelCreateInfo create_info;
            create_info.file_path = "../../../../samples/pbrdemo/assets/Cube/Cube.gltf";
            ModelLoader::ModelLoader model_loader(render_device, device_context, create_info);
            if (!model_loader.is_valid())
            {
                cyber_error(false, "Failed to load model: {0}", create_info.file_path);
            }
            auto& meshes = model_loader.get_meshes();
            if (meshes.empty())
            {
                cyber_error(false, "No meshes found in the model: {0}", create_info.file_path);
            }

            auto vertex_count = meshes[0].model_data.size();
            auto index_count = meshes[0].indices_data.size();
            auto model_verts = meshes[0].model_data.data();
            auto model_indices = meshes[0].indices_data.data();

            CubeVertex* cube_verts = cyber_new_n<CubeVertex>(vertex_count);
            for(size_t i = 0; i < vertex_count; ++i)
            {
                cube_verts[i].position = model_verts[i].pos; // Assuming position is in float3, adding w component
                cube_verts[i].normal = model_verts[i].normal; // Default normal, can be adjusted based on model data.
                cube_verts[i].tangent = model_verts[i].tangent;
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
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_STAGING;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            vertex_buffer = render_device->create_buffer(buffer_desc);
            
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_INDEX_BUFFER;
            buffer_desc.size = index_count * sizeof(uint32_t);
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_STAGING;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            index_buffer = render_device->create_buffer(buffer_desc);

            buffer_desc.size = sizeof(ConstantMatrix);
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            vertex_constant_buffer = render_device->create_buffer(buffer_desc);
            
            buffer_desc.size = sizeof(Component::LightAttribs);
            buffer_desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            buffer_desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            buffer_desc.cpu_access_flags = CPU_ACCESS_WRITE;
            light_constant_buffer = render_device->create_buffer(buffer_desc);

            void* vtx_resource = render_device->map_buffer(vertex_buffer,MAP_WRITE, MAP_FLAG_DISCARD);
            // map vertex buffer
            CubeVertex* vertices_ptr = (CubeVertex*)vtx_resource;
            memcpy(vertices_ptr, cube_verts, vertex_count * sizeof(CubeVertex));
            render_device->unmap_buffer(vertex_buffer, MAP_WRITE);
            vertex_buffer->set_buffer_size(vertex_count * sizeof(CubeVertex));

            // map index buffer
            void* idx_resource = render_device->map_buffer(index_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            uint32_t* indices_ptr = (uint32_t*)idx_resource;
            memcpy(indices_ptr, cube_indices, index_count * sizeof(uint32_t));
            render_device->unmap_buffer(index_buffer, MAP_WRITE);
            index_buffer->set_buffer_size(index_count * sizeof(uint32_t));

            // create texture
            if(meshes[0].image_paths.size() > 0)
            {
                RenderObject::ITexture* test_texture = nullptr;

                eastl::string texture_path(eastl::string::CtorSprintf(), "samples/pbrdemo/assets/Cube/%s", meshes[0].image_paths[0].c_str());

                TextureLoader::TextureLoadInfo texture_load_info{
                CYBER_UTF8("BaseColor"),
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

                TextureLoader::create_texture_from_file(
                    texture_path.c_str(),
                    texture_load_info,
                    &test_texture, render_device
                );

                base_color_texture_view = test_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
            }

            TextureLoader::TextureLoadInfo texture_load_info{
                CYBER_UTF8("Normal"),
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

            RenderObject::ITexture* normal_texture = nullptr;

            TextureLoader::create_texture_from_file(
                "samples/pbrdemo/assets/Cube/normal_mapping_normal_map.png",
                texture_load_info,
                &normal_texture, render_device
            );

            normal_texture_view = normal_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);

            texture_load_info.name = CYBER_UTF8("EnvironmentMap");
            RenderObject::ITexture* environment_texture = nullptr;
            TextureLoader::create_texture_from_file(
                "samples/pbrdemo/assets/minedump_flats_4k.hdr",
                texture_load_info,
                &environment_texture, render_device
            );

            environment_texture_view = environment_texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
        }

        void PBRApp::create_ui()
        {

        }

        void PBRApp::draw_ui()
        {

        }

        void PBRApp::create_render_pipeline()
        {
            auto renderer = m_pApp->get_renderer();
            auto render_device = renderer->get_render_device();

            // create shader
            ResourceLoader::ShaderLoadDesc vs_load_desc = {};
            vs_load_desc.target = SHADER_TARGET_6_0;
            vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/cube_vs.hlsl"),
                .stage = SHADER_STAGE_VERT,
                .entry_point_name = CYBER_UTF8("VSMain"),
            };
            eastl::shared_ptr<RenderObject::IShaderLibrary> vs_shader = ResourceLoader::add_shader(render_device, vs_load_desc);

            ResourceLoader::ShaderLoadDesc ps_load_desc = {};
            ps_load_desc.target = SHADER_TARGET_6_0;
            ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
                .file_name = CYBER_UTF8("samples/pbrdemo/assets/shaders/cube_ps.hlsl"),
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

            RenderObject::ISampler* samplers[] = { sampler, sampler };

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
                {"ATTRIB", 2, 0, 3, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, tangent)},
                {"ATTRIB", 3, 0, 2, VALUE_TYPE_FLOAT32, false, offsetof(CubeVertex, uv)},
            };
            RenderObject::VertexLayoutDesc vertex_layout_desc = {4, vertex_attributes};
            
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
                .vertex_layout = &vertex_layout_desc,
                .blend_state = &blend_state_desc,
                .depth_stencil_state = &depth_stencil_state_desc,
                .m_staticSamplerCount = 0,
                .color_formats = &scene_target.color_buffer->get_create_desc().m_format,
                .render_target_count = 1,
                .depth_stencil_format = scene_target.depth_buffer->get_create_desc().m_format,
                .prim_topology = PRIM_TOPO_TRIANGLE_LIST,
            };

            environment_pipeline = render_device->create_render_pipeline(env_rp_desc);

            vs_shader->free();
            ps_shader->free();
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
            render_device->free_render_pipeline(pipeline);
            render_device->free_device();
            render_device->free_instance(instance);
        }
    }
}