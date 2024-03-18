#pragma once

#include "common/cyber_graphics_config.h"
#include "object_base.h"

namespace Cyber
{
    namespace RenderObject
    {
        template<class BaseInterface, typename RenderDeviceType> 
        class CYBER_GRAPHICS_API DeviceObjectBase : public ObjectBase<BaseInterface>
        {
        public:
            using TBaseInterface = ObjectBase<BaseInterface>;
            using TRenderDeviceType = RenderDeviceType;
            DeviceObjectBase(RenderDeviceType* device) :TBaseInterface(), m_pRenderDevice(device) { }
            virtual ~DeviceObjectBase() = default;

            RenderDeviceType* get_device() const { return m_pRenderDevice; }

        private:
            DeviceObjectBase() = default;
            RenderDeviceType* m_pRenderDevice = nullptr;
        };
    }
}