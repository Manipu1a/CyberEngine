#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        typedef uint32_t RHIQueueIndex;
        
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
        };

        template<typename EngineImplTraits>
        class QueueBase : public RenderObjectBase<typename EngineImplTraits::QueueInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            QueueBase(RenderDeviceImplType* device);

            virtual ~QueueBase() = default;
        protected:
            ERHIQueueType mType;
            RHIQueueIndex mIdex;
        };
    }

}