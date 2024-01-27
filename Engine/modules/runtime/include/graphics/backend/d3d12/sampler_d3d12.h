#pragma once

#include "graphics/interface/sampler.h"
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
        struct CYBER_GRAPHICS_API ISampler_D3D12 : public ISampler
        {
            
        };

        class CYBER_GRAPHICS_API Sampler_D3D12_Impl : public SamplerBase<EngineD3D12ImplTraits>
        {
        public:
            using TSamplerBase = SamplerBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Sampler_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TSamplerBase(device) {}
        protected:
            /// Description for creating the sampler descriptor for this sampler
            D3D12_SAMPLER_DESC dxSamplerDesc;
            /// Descriptor handle of the Sampler in a CPU visible descriptor heap
            D3D12_CPU_DESCRIPTOR_HANDLE mDxHandle;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
