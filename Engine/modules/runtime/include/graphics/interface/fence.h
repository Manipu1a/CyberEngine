#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IFence
        {
            virtual uint64_t get_fence_value() const = 0;
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

            virtual uint64_t get_fence_value() const override
            {
                return m_fenceValue;
            }
        protected:
            uint64_t m_fenceValue;
        };
    }

}