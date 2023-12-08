#pragma once
#include "cyber_rhi_config.h"
#include "rhi/rhi.h"
#include "rhi/frame_buffer.h"
#include "rhi/render_device.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CYBER_RHI_API CEDeviceContext
        {
        public:
            CEDeviceContext(CERenderDevice* device);
            virtual ~CEDeviceContext();

        public:
            void BeginRenderPass(const RHIRenderPassDesc& desc);
            void EndRenderPass();

            CERenderDevice* GetRenderDevice() const { return pRenderDevice; }
        protected:
            CERenderDevice* pRenderDevice;
            RHIRenderPassDesc mActiveRenderPassDesc;
            CEFameBuffer* mBoundFrameBuffer;
        };
    }
}


