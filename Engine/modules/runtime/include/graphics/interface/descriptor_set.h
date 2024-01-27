#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API IDescriptorSet
        {
            
        };

        template<typename EngineImplTraits>
        class DescriptorSetBase : public RenderObjectBase<typename EngineImplTraits::DescriptorSetInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            DescriptorSetBase(RenderDeviceImplType* device);

            virtual ~DescriptorSetBase() = default;
        protected:
            class IRootSignature* root_signature;
            uint32_t set_index;
        };
    }

}