#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/semaphore.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API ISemaphore_D3D12 : public ISemaphore
        {
            
        };

        /// DirectX12 does not have a concept of a semaphore. We emulate it using a fence.
        class CYBER_GRAPHICS_API Semaphtore_D3D12_Impl : public SemaphoreBase<EngineD3D12ImplTraits>
        {
        public:
            using TSemaphtoreBase = SemaphoreBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Semaphtore_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TSemaphtoreBase(device) {}
        protected:
            ID3D12Fence* dx_fence;
            HANDLE dx_wait_idle_fence_event;
            uint64_t fence_value;
            uint64_t pad_a;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}