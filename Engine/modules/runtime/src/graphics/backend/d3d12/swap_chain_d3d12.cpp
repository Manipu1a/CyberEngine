#include "graphics/backend/d3d12/swap_chain_d3d12.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        void SwapChain_D3D12_Impl::free()
        {
            SAFE_RELEASE(m_pDxSwapChain);
        }
    }
}