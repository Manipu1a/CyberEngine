#include "renderer/renderer.h"
#include "graphics/interface/render_device.hpp"
#include "graphics/backend/d3d12/instance_d3d12.h"
#include "application/application.h"

namespace Cyber
{
    namespace Renderer 
    {
        Renderer::Renderer()
        {

        }
        
        Renderer::~Renderer()
        {

        }
        void Renderer::initialize()
        {
            create_gfx_objects();
        }
        void Renderer::run()
        {

        }
        void Renderer::update(float deltaTime)
        {

        }

        void Renderer::finalize()
        {

        }

        void Renderer::create_render_device(GRAPHICS_BACKEND backend)
        {
            // Create instance
            DECLARE_ZERO(RenderObject::InstanceCreateDesc, instance_desc);
            instance_desc.m_enableDebugLayer = true;
            instance_desc.m_enableGpuBasedValidation = false;
            instance_desc.m_enableSetName = true;

            if(backend == GRAPHICS_BACKEND_D3D12)
            {
                m_pInstance = cyber_new<RenderObject::Instance_D3D12_Impl>(instance_desc);
            }
            
        }
        
        void Renderer::create_gfx_objects()
        {
            create_render_device(GRAPHICS_BACKEND_D3D12);

            // Filter adapters
            uint32_t adapter_count = 0;
            m_pInstance->enum_adapters(nullptr, &adapter_count);
            RenderObject::IAdapter* adapters[64];
            m_pInstance->enum_adapters(adapters, &adapter_count);
            m_pAdapter = adapters[0];

            // Create device
            DECLARE_ZERO(QueueGroupDesc, queue_group_desc);
            queue_group_desc.m_queueCount = 1;
            queue_group_desc.m_queueType = COMMAND_QUEUE_TYPE_GRAPHICS;

            DECLARE_ZERO(EngineCreateDesc, engine_desc);
            engine_desc.m_queueGroupCount = 1;
            engine_desc.m_queueGroups = { queue_group_desc };
            engine_desc.context_id = 0;
            engine_desc.queue_type = COMMAND_QUEUE_TYPE_GRAPHICS;
            engine_desc.is_deferrd_context = false;

            uint32_t num_immediate_contexts = engine_desc.num_immediate_contexts > 0 ? engine_desc.num_immediate_contexts : 1;
            device_contexts.resize(num_immediate_contexts + engine_desc.num_deferred_contexts);
            m_pInstance->create_device_and_context(m_pAdapter, engine_desc, &m_pRenderDevice, device_contexts.data());

            // Create swapchain
        #if defined (_WIN32) || defined (_WIN64)
            m_pSurface = m_pRenderDevice->surface_from_hwnd(Core::Application::getApp()->get_window()->get_native_window());
        #elif defined(_APPLE_)
        #endif
            DECLARE_ZERO(RenderObject::SwapChainDesc, chain_desc);
            chain_desc.m_pSurface = m_pSurface;
            chain_desc.m_width = Core::Application::getApp()->get_window()->get_width();
            chain_desc.m_height = Core::Application::getApp()->get_window()->get_height();
            chain_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
            chain_desc.m_imageCount = 3;
            chain_desc.m_enableVsync = true;
            m_pSwapChain = m_pRenderDevice->create_swap_chain(chain_desc);

            for(uint32_t i = 0;i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                RenderObject::TextureCreateDesc color_buffer_desc;
                color_buffer_desc.m_name = u8"ColorBuffer";
                color_buffer_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
                color_buffer_desc.m_width = chain_desc.m_width;
                color_buffer_desc.m_height = chain_desc.m_height;
                color_buffer_desc.m_depth = 1;
                color_buffer_desc.m_arraySize = 1;
                color_buffer_desc.m_mipLevels = 1;
                color_buffer_desc.m_dimension = TEX_DIMENSION_2D;
                color_buffer_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
                color_buffer_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_RENDER_TARGET | GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
                color_buffer_desc.m_pNativeHandle = nullptr;
                color_buffer_desc.m_clearValue = fastclear_1111;
                scene_target[i].color_buffer = m_pRenderDevice->create_texture(color_buffer_desc);

                RenderObject::TextureCreateDesc depth_buffer_desc;
                depth_buffer_desc.m_name = u8"DepthBuffer";
                depth_buffer_desc.m_format = TEX_FORMAT_D32_FLOAT;
                depth_buffer_desc.m_width = chain_desc.m_width;
                depth_buffer_desc.m_height = chain_desc.m_height;
                depth_buffer_desc.m_depth = 1;
                depth_buffer_desc.m_arraySize = 1;
                depth_buffer_desc.m_mipLevels = 1;
                depth_buffer_desc.m_dimension = TEX_DIMENSION_2D;
                depth_buffer_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
                depth_buffer_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL;
                depth_buffer_desc.m_pNativeHandle = nullptr;
                scene_target[i].depth_buffer = m_pRenderDevice->create_texture(depth_buffer_desc);
            }

            auto back_buffer_view = scene_target[0].color_buffer->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
            RenderObject::ITexture_View* attachment_resources[1] = { back_buffer_view  };
            RenderObject::FrameBuffserDesc frame_buffer_desc;
            frame_buffer_desc.m_name = u8"FrameBuffer";
            frame_buffer_desc.m_attachmentCount = 1;
            frame_buffer_desc.m_ppAttachments = attachment_resources;
            frame_buffer = m_pRenderDevice->create_frame_buffer(frame_buffer_desc);
        }

        void Renderer::resize_swap_chain(uint32_t width, uint32_t height)
        {
            if(m_pSwapChain)
            {
                m_pSwapChain->resize(width, height);
            }
        }

        void Renderer::resize_viewport(uint32_t width, uint32_t height)
        {
            RenderObject::TextureCreateDesc color_buffer_desc = scene_target[0].color_buffer->get_create_desc();
            RenderObject::TextureCreateDesc depth_buffer_desc = scene_target[0].depth_buffer->get_create_desc();

            if(color_buffer_desc.m_width != width || color_buffer_desc.m_height != height)
            {
                color_buffer_desc.m_width = width;
                color_buffer_desc.m_height = height;

                depth_buffer_desc.m_width = width;
                depth_buffer_desc.m_height = height;
                // Resize color and depth buffers
                for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                {
                    scene_target[i].color_buffer->free();
                    scene_target[i].color_buffer = nullptr;
                    scene_target[i].depth_buffer->free();
                    scene_target[i].depth_buffer = nullptr;

                    scene_target[i].color_buffer = m_pRenderDevice->create_texture(color_buffer_desc);
                    scene_target[i].depth_buffer = m_pRenderDevice->create_texture(depth_buffer_desc);
                }

                // Update frame buffer
                auto back_buffer_view = scene_target[0].color_buffer->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
                RenderObject::ITexture_View* attachment_resources[1] = { back_buffer_view };
                frame_buffer->update_attachments(attachment_resources, 1);
            }
        }

        float4x4 Renderer::get_adjusted_projection_matrix(float fov, float near_plane, float far_plane)
        {
            RenderObject::TextureCreateDesc color_buffer_desc = scene_target[0].color_buffer->get_create_desc();

            float aspect_ratio = static_cast<float>(color_buffer_desc.m_width) / static_cast<float>(color_buffer_desc.m_height);
            float YScale = 1.0f / std::tan(fov * 0.5f);
            float XScale = YScale / aspect_ratio;

            float4x4 projection_matrix = {
                XScale, 0.0f, 0.0f, 0.0f,
                0.0f, YScale, 0.0f, 0.0f,
                0.0f, 0.0f, far_plane / (far_plane - near_plane), 1.0f,
                0.0f, 0.0f, -near_plane * far_plane / (far_plane - near_plane), 0.0f
            };
            return projection_matrix;
        }

    }
}