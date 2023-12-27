#pragma once

#include "common/cyber_graphics_config.h"

namespace Cyber
{
    namespace RenderObject
    {
        template<class BaseInterface, typename RenderDeviceType> 
        class CYBER_GRAPHICS_API RenderObjectBase : public BaseInterface
        {
        public:
            RenderObjectBase(RenderDeviceType* device) : render_device(device) { }
            virtual ~RenderObjectBase() = default;

            RenderDeviceType* get_device() const { return render_device; }

        private:
            RenderObjectBase() = default;
            RenderDeviceType* render_device = nullptr;
        };
    }
}