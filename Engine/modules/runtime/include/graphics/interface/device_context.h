#pragma once
#include "common/cyber_graphics_config.h"
#include "rhi.h"
#include "frame_buffer.h"
#include "render_device.h"

namespace Cyber
{
    namespace RenderObject
    {
        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API CEDeviceContext
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            
            CEDeviceContext(CERenderDevice* device);
            virtual ~CEDeviceContext();

        public:
            void BeginRenderPass(const RenderPassDesc& desc);
            void EndRenderPass();

            RenderDeviceImplType* get_device() const { return render_device; }
        protected:
            RenderDeviceImplType* render_device;
            RenderPassDesc active_render_pass_desc;
            CEFrameBuffer* bound_frame_buffer = nullptr;
        };
    }
}


