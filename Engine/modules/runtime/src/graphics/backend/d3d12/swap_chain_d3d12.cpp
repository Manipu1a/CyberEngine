#include "graphics/backend/d3d12/swap_chain_d3d12.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        SwapChain_D3D12_Impl::SwapChain_D3D12_Impl(RenderObject::RenderDevice_D3D12_Impl* device, SwapChainDesc desc) : TSwapChainBase(device, desc)
        {
            m_pDxSwapChain = nullptr;
            m_dxSyncInterval = 1;
            m_flags = 0;
            m_imageCount = 0;
            m_enableVsync = 0;
        }

        void SwapChain_D3D12_Impl::free()
        {
            SAFE_RELEASE(m_pDxSwapChain);
        }
    }
}