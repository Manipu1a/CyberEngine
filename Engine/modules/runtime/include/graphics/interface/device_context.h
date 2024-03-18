#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics_types.h"
#include "frame_buffer.h"
#include "render_device.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CYBER_GRAPHICS_API CEDeviceContext
        {
        public:
            CEDeviceContext(IRenderDevice* device);
            virtual ~CEDeviceContext();

        public:
            void BeginRenderPass(const RenderPassDesc& desc);
            void EndRenderPass();

            IRenderDevice* get_device() const { return m_pRenderDevice; }
        protected:
            IRenderDevice* m_pRenderDevice;
            RenderPassDesc m_activeRenderPassDesc;
            IFrameBuffer* m_pBoundFrameBuffer = nullptr;
        };
    }
}


