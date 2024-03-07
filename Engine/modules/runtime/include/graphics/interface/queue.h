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
            class ICommandBuffer** pCmds;
            class IFence* mSignalFence;
            class ISemaphore** pWaitSemaphores;
            class ISemaphore** pSignalSemaphores;
            uint32_t mCmdsCount;
            uint32_t mWaitSemaphoreCount;
            uint32_t mSignalSemaphoreCount;
        };

        struct CYBER_GRAPHICS_API QueuePresentDesc
        {
            class ISwapChain* swap_chain;
            const ISemaphore** wait_semaphores;
            uint32_t wait_semaphore_count;
            uint32_t index;
        };

        struct CYBER_GRAPHICS_API IQueue
        {
            virtual void signal_fence(class IFence* fence, uint64_t value) = 0;
        };

        template<typename EngineImplTraits>
        class QueueBase : public RenderObjectBase<typename EngineImplTraits::QueueInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            QueueBase(RenderDeviceImplType* device);

            virtual ~QueueBase() = default;

        protected:
            QUEUE_TYPE m_Type;
            QueueIndex m_Idex;
        };
    }

}