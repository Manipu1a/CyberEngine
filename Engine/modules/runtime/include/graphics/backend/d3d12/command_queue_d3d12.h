#pragma once

#include "graphics/interface/command_queue.h"
#include "engine_impl_traits_d3d12.hpp"
#include "eastl/atomic.h"
#include <mutex>

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API ICommandQueue_D3D12 : public ICommandQueue
        {
            
        };

        class CYBER_GRAPHICS_API CommandQueue_D3D12_Impl : public CommandQueueBase<EngineD3D12ImplTraits>
        {
        public:
            using TCommandQueueBase = CommandQueueBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            CommandQueue_D3D12_Impl(class RenderDevice_D3D12_Impl* device, ID3D12CommandQueue* native_queue, ID3D12Fence* d3d12_fence);

            virtual void signal_fence(class IFence* fence, uint64_t value) override;
            virtual void wait_fence(class IFence* fence, uint64_t value) override;
            uint64_t submit(uint32_t num_cmd_lists, ID3D12CommandList* const* command_lists);

            ID3D12CommandQueue* get_native_queue() const { return command_queue; }
            void set_native_queue(ID3D12CommandQueue* queue) { command_queue = queue; }

            virtual uint64_t wait_for_idle() override;
            virtual uint64_t get_completed_fence_value() override;
        protected:
            // A value that will be signaled by the command queue next
            eastl::atomic<uint64_t> next_fence_value{1};
            // Last fence value completed by the GPU
            eastl::atomic<uint64_t> last_fence_value{0};

            std::mutex queue_mutex;
            ID3D12CommandQueue* command_queue;
            const D3D12_COMMAND_QUEUE_DESC command_queue_desc;

            ID3D12Fence* d3d12_fence;
            
            HANDLE wait_for_gpu_event_handle = {};
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
