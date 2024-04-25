#pragma once

#include "graphics/interface/graphics_types.h"
#include "graphics/interface/render_device.hpp"
#include "cyber_runtime.config.h"

namespace Cyber
{
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
        protected:
            ///-------------------------------------
            static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
            static const uint32_t BACK_BUFFER_COUNT = 3;
            RenderObject::IRenderDevice* m_pRenderDevice = nullptr;
            RenderObject::IInstance* m_pInstance = nullptr;
            RenderObject::IAdapter* m_pAdapter = nullptr;
            RenderObject::IFence* m_pPresentFence = nullptr;
            RenderObject::IQueue* m_pQueue = nullptr;
            RenderObject::ICommandPool* m_pPool = nullptr;
            RenderObject::ICommandBuffer* m_pCmd = nullptr;
            RenderObject::ISwapChain* m_pSwapChain = nullptr;
            RenderObject::IRenderPass* m_pRenderPass = nullptr;
            Surface* m_pSurface = nullptr;
            RenderObject::IFence* m_pPresentSwmaphore = nullptr; 
            uint32_t m_backBufferIndex = 0;
        };
    }
}