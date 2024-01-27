#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/fence.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IFence_D3D12 : public IFence
        {
            
        };

        class CYBER_GRAPHICS_API Fence_D3D12_Impl : public FenceBase<EngineD3D12ImplTraits>
        {
        public:
            using TFenceBase = FenceBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Fence_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TFenceBase(device) {}

        protected:
            ID3D12Fence* pDxFence;
            HANDLE pDxWaitIdleFenceEvent;
            uint64_t mFenceValue;
            uint64_t mPadA;
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}