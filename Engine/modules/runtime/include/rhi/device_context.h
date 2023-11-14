#pragma once

#include "rhi/rhi.h"
#include "rhi/frame_buffer.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CYBER_RHI_API CDeviceContext
        {
        public:
            CDeviceContext(RHIDevice* device);
            virtual ~CDeviceContext();

        public:
            void BeginRenderPass(const RHIRenderPassDesc& desc);
            void EndRenderPass();
        protected:
            RHIDevice* pDevice;
            RHIRenderPassDesc mActiveRenderPassDesc;
            CFameBuffer* mBoundFrameBuffer;
        };
    }
}


