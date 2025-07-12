#include "backend/d3d12/command_queue_d3d12.h"
#include "backend/d3d12/fence_d3d12.h"
#include "log/Log.h"
namespace Cyber
{
    namespace RenderObject
    {
        CommandQueue_D3D12_Impl::CommandQueue_D3D12_Impl(class RenderDevice_D3D12_Impl* device, ID3D12CommandQueue* native_queue, ID3D12Fence* d3d12_fence)
            : TCommandQueueBase(device), 
            command_queue(native_queue),
            command_queue_desc{native_queue->GetDesc()},
            next_fence_value(1),
            d3d12_fence(d3d12_fence),
            wait_for_gpu_event_handle(CreateEvent(NULL, FALSE, FALSE, NULL))
        {
            d3d12_fence->Signal(0);
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

            command_queue->Signal(d3d12_fence, fence_value);

            return fence_value;
        }

        uint64_t CommandQueue_D3D12_Impl::wait_for_idle()
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            auto last_signal_fence_value = next_fence_value.fetch_add(1);

            command_queue->Signal(d3d12_fence, last_signal_fence_value);

            if(get_completed_fence_value() < last_signal_fence_value)
            {
                d3d12_fence->SetEventOnCompletion(last_signal_fence_value, wait_for_gpu_event_handle);
                WaitForSingleObject(wait_for_gpu_event_handle, INFINITE);
                cyber_check(get_completed_fence_value() == last_signal_fence_value, "Unexpected signaled fence value");
            }
            return last_signal_fence_value;
        }

        uint64_t CommandQueue_D3D12_Impl::get_completed_fence_value()
        {
            auto completed_value = d3d12_fence->GetCompletedValue();

            auto current_value = last_fence_value.load();
            while(!last_fence_value.compare_exchange_weak(current_value, eastl::max(current_value, completed_value)))
            {

            }

            return last_fence_value.load();
        }
    }
}