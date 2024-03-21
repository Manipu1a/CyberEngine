#pragma once

#include "graphics/interface/swap_chain.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;

        struct ISwapChain_D3D12 : public ISwapChain 
        {

        };

        class CYBER_GRAPHICS_API SwapChain_D3D12_Impl final : public SwapChainBase<EngineD3D12ImplTraits>
        {
        public:
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
            
            SwapChain_D3D12_Impl(class RenderDevice_D3D12_Impl* device);

            virtual ~SwapChain_D3D12_Impl()
            {

            }

        protected:
            IDXGISwapChain3* pDxSwapChain;
            uint32_t mDxSyncInterval : 3;
            uint32_t mFlags : 10;
            uint32_t mImageCount : 3;
            uint32_t mEnableVsync : 1;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}
