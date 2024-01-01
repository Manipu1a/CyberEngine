#pragma once
#include "common/cyber_graphics_config.h"
#include "rhi.h"
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

            IRenderDevice* get_device() const { return render_device; }
        protected:
            IRenderDevice* render_device;
            RenderPassDesc active_render_pass_desc;
            IFrameBuffer* bound_frame_buffer = nullptr;
        };
    }
}


