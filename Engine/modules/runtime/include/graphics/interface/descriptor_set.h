#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API IDescriptorSet
        {
            
        };

        template<typename EngineImplTraits>
        class DescriptorSetBase : public DeviceObjectBase<typename EngineImplTraits::DescriptorSetInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using DescriptorSetInterface = typename EngineImplTraits::DescriptorSetInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TDescriptorSetBase = typename DeviceObjectBase<DescriptorSetInterface, RenderDeviceImplType>;
            DescriptorSetBase(RenderDeviceImplType* device) : TDescriptorSetBase(device) {  };

            virtual ~DescriptorSetBase() = default;
        protected:
            class IRootSignature* m_pRootSignature;
            uint32_t m_setIndex;
        };
    }

}