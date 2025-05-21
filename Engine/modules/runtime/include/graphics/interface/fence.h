#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IFence : public IDeviceObject
        {
            virtual uint64_t get_fence_value() const = 0;
            virtual void add_fence_value() = 0;
        };

        template<typename EngineImplTraits>
        class FenceBase : public DeviceObjectBase<typename EngineImplTraits::FenceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using FenceInterface = typename EngineImplTraits::FenceInterface;
            using TFenceBase = typename DeviceObjectBase<FenceInterface, RenderDeviceImplType>;

            FenceBase(RenderDeviceImplType* device) : TFenceBase(device) 
            {
                m_fenceValue = 0;
            };
            virtual ~FenceBase() = default;

            virtual uint64_t get_fence_value() const override
            {
                return m_fenceValue;
            }

            virtual void add_fence_value() override
            {
                m_fenceValue++;
            }
        protected:
            uint64_t m_fenceValue;
        };
    }

}