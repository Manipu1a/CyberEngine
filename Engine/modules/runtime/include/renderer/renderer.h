#pragma once

#include "graphics/interface/graphics_types.h"
#include "graphics/interface/render_device.hpp"
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

            CYBER_FORCE_INLINE RenderObject::IRenderDevice* get_render_device() const { return m_pRenderDevice; }
            CYBER_FORCE_INLINE RenderObject::IDeviceContext* get_device_context(size_t id = 0) const { return device_contexts[id]; }
            CYBER_FORCE_INLINE RenderObject::IInstance* get_instance() const { return m_pInstance; }
            CYBER_FORCE_INLINE RenderObject::IAdapter* get_adapter() const { return m_pAdapter; }
            CYBER_FORCE_INLINE RenderObject::ISwapChain* get_swap_chain() const { return m_pSwapChain; }
            CYBER_FORCE_INLINE RenderObject::IRenderPass* get_render_pass() const { return m_pRenderPass; }
            CYBER_FORCE_INLINE void set_render_pass(RenderObject::IRenderPass* pass) { m_pRenderPass = pass; }
            CYBER_FORCE_INLINE Surface* get_surface() const { return m_pSurface; }
            CYBER_FORCE_INLINE uint32_t get_back_buffer_index() const { return m_backBufferIndex; }
            CYBER_FORCE_INLINE void set_back_buffer_index(uint32_t index) { m_backBufferIndex = index; }

        protected:
            ///-------------------------------------
            static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
            static const uint32_t BACK_BUFFER_COUNT = 3;
            RenderObject::IRenderDevice* m_pRenderDevice = nullptr;
            eastl::vector<RenderObject::IDeviceContext*> device_contexts;
            RenderObject::IInstance* m_pInstance = nullptr;
            RenderObject::IAdapter* m_pAdapter = nullptr;
            RenderObject::ISwapChain* m_pSwapChain = nullptr;
            RenderObject::IRenderPass* m_pRenderPass = nullptr;
            Surface* m_pSurface = nullptr;
            RenderObject::IFence* m_pPresentSwmaphore = nullptr; 
            uint32_t m_backBufferIndex = 0;
        };
    }
}