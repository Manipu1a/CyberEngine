#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API CommandPoolCreateDesc
        {
            uint32_t ___nothing_and_useless__;
        };

        struct CYBER_GRAPHICS_API ICommandPool
        {
            virtual void set_queue(class IQueue* queue) = 0;
            virtual IQueue* get_queue() const = 0;
        };

        template<typename EngineImplTraits>
        class CommandPoolBase : public DeviceObjectBase<typename EngineImplTraits::CommandPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using CommandPoolInterface = typename EngineImplTraits::CommandPoolInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TCommandPoolBase = typename DeviceObjectBase<CommandPoolInterface, RenderDeviceImplType>;

            CommandPoolBase(RenderDeviceImplType* device, const CommandPoolCreateDesc& desc) : TCommandPoolBase(device), m_desc(desc) {  };

            virtual ~CommandPoolBase() = default;

            virtual void set_queue(class IQueue* queue) override
            {
                m_pQueue = queue;
            }

            virtual IQueue* get_queue() const override
            {
                return m_pQueue;
            }
        protected:
            class IQueue* m_pQueue;
            CommandPoolCreateDesc m_desc;
        };
    }

}