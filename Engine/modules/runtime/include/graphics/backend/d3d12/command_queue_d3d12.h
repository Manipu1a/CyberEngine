#pragma once

#include "graphics/interface/command_queue.h"
#include "engine_impl_traits_d3d12.hpp"
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

            CommandQueue_D3D12_Impl(class RenderDevice_D3D12_Impl* device, ID3D12CommandQueue* native_queue);

            virtual void signal_fence(class IFence* fence, uint64_t value) override;
            virtual void wait_fence(class IFence* fence, uint64_t value) override;
            uint64_t submit(uint32_t num_cmd_lists, ID3D12CommandList* const* command_lists);

            ID3D12CommandQueue* get_native_queue() const { return command_queue; }
        protected:
            // A value that will be signaled by the command queue next
            std::atomic<uint64_t> next_fence_value{1};
            // Last fence value completed by the GPU
            std::atomic<uint64_t> last_fence_value{0};

            std::mutex queue_mutex;
            ID3D12CommandQueue* command_queue;
            const D3D12_COMMAND_QUEUE_DESC command_queue_desc;

            ID3D12Fence* d3d12_fence;

            IFence* m_pFence;
            
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
