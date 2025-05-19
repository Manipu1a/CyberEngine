#pragma once

#include "graphics/interface/descriptor_set.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IDescriptorSet_D3D12 : public IDescriptorSet
        {
            
        };

        class CYBER_GRAPHICS_API DescriptorSet_D3D12_Impl : public DescriptorSetBase<EngineD3D12ImplTraits>
        {
        public:
            using TDescriptorSetBase = DescriptorSetBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            DescriptorSet_D3D12_Impl(class RenderDevice_D3D12_Impl* device, DescriptorSetCreateDesc desc) : TDescriptorSetBase(device, desc) {}

        private:
            /// Start handle to cbv srv uav descriptor table
            uint64_t cbv_srv_uav_handle;
            /// Stride of the cbv srv uav descriptor table (number of descriptors * descriptor size)
            uint32_t cbv_srv_uav_stride;
            /// Start handle to sampler descriptor table
            uint64_t sampler_handle;
            /// Stride of the sampler descriptor table (number of descriptors * descriptor size)
            uint32_t sampler_stride;

            friend class RenderObject::DeviceContext_D3D12_Impl;
        };

    }
}
