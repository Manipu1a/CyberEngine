#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "device_object.h"
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

        struct CYBER_GRAPHICS_API ICommandQueue : public IDeviceObject
        {
            virtual void signal_fence(class IFence* fence, uint64_t value) = 0;
            virtual void wait_fence(class IFence* fence, uint64_t value) = 0;
            virtual COMMAND_QUEUE_TYPE get_type() const = 0;
            virtual void set_type(COMMAND_QUEUE_TYPE type) = 0;
            virtual QueueIndex get_index() const = 0;
            virtual void set_index(QueueIndex index) = 0;
        };

        template<typename EngineImplTraits>
        class CommandQueueBase : public DeviceObjectBase<typename EngineImplTraits::CommandQueueInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using CommandQueueInterface = typename EngineImplTraits::CommandQueueInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TCommandQueueBase = DeviceObjectBase<CommandQueueInterface, RenderDeviceImplType>;

            CommandQueueBase(RenderDeviceImplType* device) : TCommandQueueBase(device) {  };

            virtual ~CommandQueueBase() = default;

            CYBER_FORCE_INLINE virtual COMMAND_QUEUE_TYPE get_type() const override
            {
                return m_type;
            }

            CYBER_FORCE_INLINE virtual void set_type(COMMAND_QUEUE_TYPE type) override
            {
                m_type = type;
            }

            CYBER_FORCE_INLINE virtual QueueIndex get_index() const override
            {
                return m_index;
            }

            CYBER_FORCE_INLINE virtual void set_index(QueueIndex index) override
            {
                m_index = index;
            }
        protected:
            COMMAND_QUEUE_TYPE m_type;
            QueueIndex m_index;
        };
    }

}