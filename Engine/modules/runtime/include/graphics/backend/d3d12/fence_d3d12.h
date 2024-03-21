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

            ID3D12Fence* get_native_fence() const { return m_pDxFence; }
        protected:
            ID3D12Fence* m_pDxFence;
            HANDLE m_dxWaitIdleFenceEvent;
            uint64_t m_padA;
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}