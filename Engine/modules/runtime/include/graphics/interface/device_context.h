#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics_types.h"
#include "frame_buffer.h"
#include "render_device.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        struct DeviceContextDesc
        {

        };

        struct CYBER_GRAPHICS_API IDeviceContext
        {

        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API DeviceContextBase : public ObjectBase<EngineImplTraits::DeviceContextInterface>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            DeviceContextBase(RenderDeviceImplType* device, const DeviceContextDesc& desc) : m_pRenderDevice(device) {}

        private:
            RenderDeviceImplType* m_pRenderDevice;
        };
    }
}


