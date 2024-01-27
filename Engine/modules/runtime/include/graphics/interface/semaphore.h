#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API ISemaphore
        {
            
        };

        template<typename EngineImplTraits>
        class SemaphoreBase : public RenderObjectBase<typename EngineImplTraits::SemaphoreInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            SemaphoreBase(RenderDeviceImplType* device);
            virtual ~SemaphoreBase() = default;
        protected:
        
        };
    }

}