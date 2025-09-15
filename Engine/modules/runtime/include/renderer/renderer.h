#pragma once

#include "graphics/interface/graphics_types.h"
#include "graphics/interface/render_device.hpp"
#include "math/basic_math.hpp"
#include "cyber_runtime.config.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IDeviceContext;
    }

    namespace Core
    {
        class Application;
    }
    namespace Renderer
    {
        struct CYBER_RUNTIME_API SceneTarget
        {
            RefCntAutoPtr<RenderObject::ITexture> color_buffer = nullptr;
            RefCntAutoPtr<RenderObject::ITexture> depth_buffer = nullptr;
        };

        class CYBER_RUNTIME_API Renderer
        {
        public:
            Renderer();
            ~Renderer();

            void initialize();
            void run();
            void update(float deltaTime);
            void finalize();
            
            void create_render_device(GRAPHICS_BACKEND backend);
            void create_gfx_objects();

            void resize_swap_chain(uint32_t width, uint32_t height);
            void resize_viewport(uint32_t width, uint32_t height);
            
            float4x4 get_adjusted_projection_matrix(float fov, float near_plane, float far_plane);

            CYBER_FORCE_INLINE RenderObject::IRenderDevice* get_render_device() const { return m_pRenderDevice; }
            CYBER_FORCE_INLINE RenderObject::IDeviceContext* get_device_context(size_t id = 0) const { return device_contexts[id]; }
            CYBER_FORCE_INLINE RenderObject::IInstance* get_instance() const { return m_pInstance; }
            CYBER_FORCE_INLINE RenderObject::IAdapter* get_adapter() const { return m_pAdapter; }
            CYBER_FORCE_INLINE RenderObject::ISwapChain* get_swap_chain() const { return m_pSwapChain; }
            CYBER_FORCE_INLINE RenderObject::IRenderPass* get_render_pass() const { return m_pRenderPass; }
            CYBER_FORCE_INLINE RenderObject::IFrameBuffer* get_frame_buffer() const { return frame_buffer; }
            CYBER_FORCE_INLINE void set_render_pass(RenderObject::IRenderPass* pass) { m_pRenderPass = pass; }
            CYBER_FORCE_INLINE Surface* get_surface() const { return m_pSurface; }
            CYBER_FORCE_INLINE uint32_t get_back_buffer_index() const { return m_backBufferIndex; }
            CYBER_FORCE_INLINE void set_back_buffer_index(uint32_t index) { m_backBufferIndex = index; }
            
            SceneTarget& get_scene_target(uint32_t index)
            {
                if(index < MAX_FRAMES_IN_FLIGHT)
                {
                    return scene_target[index];
                }
                else
                {
                    CB_CORE_ERROR("Index out of bounds in get_scene_target: {0}", index);
                    return scene_target[0]; // Return first target as fallback
                }
            }

            static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
            static const uint32_t BACK_BUFFER_COUNT = 3;

        protected:
            ///-------------------------------------
            RenderObject::IRenderDevice* m_pRenderDevice = nullptr;
            eastl::vector<RenderObject::IDeviceContext*> device_contexts;
            RenderObject::IInstance* m_pInstance = nullptr;
            RenderObject::IAdapter* m_pAdapter = nullptr;
            RenderObject::ISwapChain* m_pSwapChain = nullptr;

            SceneTarget scene_target[MAX_FRAMES_IN_FLIGHT];
            
            RenderObject::IFrameBuffer* frame_buffer = nullptr;
            RenderObject::IRenderPass* m_pRenderPass = nullptr;
            Surface* m_pSurface = nullptr;
            RenderObject::IFence* m_pPresentSwmaphore = nullptr; 
            uint32_t m_backBufferIndex = 0;
        };
    }
}