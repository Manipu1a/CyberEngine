#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API ISemaphore
        {
            
        };

        template<typename EngineImplTraits>
        class SemaphoreBase : public RenderObjectBase<typename EngineImplTraits::CommandPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            SemaphoreBase(RenderDeviceImplType* device);
            virtual ~SemaphoreBase() = default;
        protected:
        
        };
    }

}