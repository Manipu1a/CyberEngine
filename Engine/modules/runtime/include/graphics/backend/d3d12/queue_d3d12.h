#pragma once

#include "graphics/interface/queue.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IQueue_D3D12 : public IQueue
        {
            
        };

        class CYBER_GRAPHICS_API Queue_D3D12_Impl : public QueueBase<EngineD3D12ImplTraits>
        {
        public:
            using TQueueBase = QueueBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Queue_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TQueueBase(device) {}

            virtual void signal_fence(class IFence* fence, uint64_t value) override;
            virtual void wait_fence(class IFence* fence, uint64_t value) override;

            ID3D12CommandQueue* get_native_queue() const { return m_pCommandQueue; }
        protected:
            ID3D12CommandQueue* m_pCommandQueue;
            IFence* m_pFence;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
