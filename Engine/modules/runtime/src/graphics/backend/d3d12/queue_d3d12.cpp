#include "backend/d3d12/queue_d3d12.h"
#include "backend/d3d12/fence_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        void Queue_D3D12_Impl::signal_fence(class IFence* fence, uint64_t value)
        {
            Fence_D3D12_Impl* fenceD3D = static_cast<Fence_D3D12_Impl*>(fence);
            m_pCommandQueue->Signal(fenceD3D->get_native_fence(), value);
        }

        void Queue_D3D12_Impl::wait_fence(class IFence* fence, uint64_t value)
        {
            Fence_D3D12_Impl* fenceD3D = static_cast<Fence_D3D12_Impl*>(fence);
            m_pCommandQueue->Wait(fenceD3D->get_native_fence(), value);
        }
    }
}