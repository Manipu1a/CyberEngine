#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API ISemaphore : public IDeviceObject
        {
            
        };

        template<typename EngineImplTraits>
        class SemaphoreBase : public DeviceObjectBase<typename EngineImplTraits::SemaphoreInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using SemaphoreInterface = typename EngineImplTraits::SemaphoreInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TSemaphoreBase = typename DeviceObjectBase<SemaphoreInterface, RenderDeviceImplType>;

            SemaphoreBase(RenderDeviceImplType* device) : TSemaphoreBase(device) {  };
            virtual ~SemaphoreBase() = default;
        protected:
        
        };
    }

}