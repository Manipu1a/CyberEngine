#pragma once

#include "common/cyber_graphics_config.h"
#include "object_base.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CYBER_GRAPHICS_API IDeviceObject
        {
        public:
            virtual void free() = 0;
        };

        template<class BaseInterface, typename RenderDeviceType> 
        class CYBER_GRAPHICS_API DeviceObjectBase : public ObjectBase<BaseInterface>
        {
        public:
            using TBaseInterface = ObjectBase<BaseInterface>;
            using TRenderDeviceType = RenderDeviceType;
            DeviceObjectBase(RenderDeviceType* device) :TBaseInterface(), render_device(device) { }
            virtual ~DeviceObjectBase()
            {
            }

            RenderDeviceType* get_device() const { return render_device; }

            virtual void free() override
            {
                cyber_delete(this);
            }

        protected:
            DeviceObjectBase() = default;
            RenderDeviceType* render_device = nullptr;
        };
    }
}