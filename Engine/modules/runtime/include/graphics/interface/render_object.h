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
            RenderObjectBase(RenderDeviceType* device) : m_pRenderDevice(device) { }
            virtual ~RenderObjectBase() = default;

            RenderDeviceType* get_device() const { return m_pRenderDevice; }

        private:
            RenderObjectBase() = default;
            RenderDeviceType* m_pRenderDevice = nullptr;
        };

        
    }
}