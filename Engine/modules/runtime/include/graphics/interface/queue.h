#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "render_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        typedef uint32_t QueueIndex;
        
        struct CYBER_GRAPHICS_API QueueSubmitDesc
        {
            class ICommandBuffer** m_ppCmds;
            class IFence* m_pSignalFence;
            class ISemaphore** m_ppWaitSemaphores;
            class ISemaphore** m_ppSignalSemaphores;
            uint32_t m_cmdsCount;
            uint32_t m_waitSemaphoreCount;
            uint32_t m_signalSemaphoreCount;
        };

        struct CYBER_GRAPHICS_API QueuePresentDesc
        {
            class ISwapChain* m_pSwapChain;
            const ISemaphore** m_ppwaitSemaphores;
            uint32_t m_waitSemaphoreCount;
            uint32_t m_index;
        };

        struct CYBER_GRAPHICS_API IQueue
        {
            virtual void signal_fence(class IFence* fence, uint64_t value) = 0;
        };

        template<typename EngineImplTraits>
        class QueueBase : public RenderObjectBase<typename EngineImplTraits::QueueInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using QueueInterface = typename EngineImplTraits::QueueInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TQueueBase = typename QueueBase<QueueInterface, RenderDeviceImplType>;

            QueueBase(RenderDeviceImplType* device) : TQueueBase(device) {  };

            virtual ~QueueBase() = default;

        protected:
            QUEUE_TYPE m_type;
            QueueIndex m_index;
        };
    }

}