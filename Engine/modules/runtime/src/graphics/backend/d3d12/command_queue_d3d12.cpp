#include "backend/d3d12/command_queue_d3d12.h"
#include "backend/d3d12/fence_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        CommandQueue_D3D12_Impl::CommandQueue_D3D12_Impl(class RenderDevice_D3D12_Impl* device, ID3D12CommandQueue* native_queue)
            : TCommandQueueBase(device), 
            command_queue(native_queue),
            command_queue_desc{native_queue->GetDesc()},
            next_fence_value(1)
        {

        }

        void CommandQueue_D3D12_Impl::signal_fence(class IFence* fence, uint64_t value)
        {
            Fence_D3D12_Impl* fenceD3D = static_cast<Fence_D3D12_Impl*>(fence);
            command_queue->Signal(fenceD3D->get_native_fence(), value);
        }

        void CommandQueue_D3D12_Impl::wait_fence(class IFence* fence, uint64_t value)
        {
            Fence_D3D12_Impl* fenceD3D = static_cast<Fence_D3D12_Impl*>(fence);
            command_queue->Wait(fenceD3D->get_native_fence(), value);
        }

        uint64_t CommandQueue_D3D12_Impl::submit(uint32_t num_cmd_lists, ID3D12CommandList* const* command_lists)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            auto fence_value = next_fence_value.fetch_add(1);

            if(num_cmd_lists != 0 && command_lists != nullptr)
            {
                command_queue->ExecuteCommandLists(num_cmd_lists, command_lists);
            }

            //command_queue->Signal(d3d12_fence, fence_value);

            return fence_value;
        }

    }
}