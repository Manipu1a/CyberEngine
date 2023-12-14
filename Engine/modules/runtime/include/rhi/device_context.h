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

            CERenderDevice* GetRenderDevice() const { return render_device; }
        protected:
            CERenderDevice* render_device;
            RHIRenderPassDesc active_render_pass_desc;
            CEFrameBuffer* bound_frame_buffer = nullptr;
        };
    }
}


