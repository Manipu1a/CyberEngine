#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IFence
        {
            
        };

        template<typename EngineImplTraits>
        class FenceBase : public DeviceObjectBase<typename EngineImplTraits::FenceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using FenceInterface = typename EngineImplTraits::FenceInterface;
            using TFenceBase = typename DeviceObjectBase<FenceInterface, RenderDeviceImplType>;

            FenceBase(RenderDeviceImplType* device) : TFenceBase(device) {  };
            virtual ~FenceBase() = default;
        protected:
        
        };
    }

}