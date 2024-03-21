#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API CommandBufferCreateDesc
        {
            bool m_isSecondary : 1;
        };

        struct CYBER_GRAPHICS_API ICommandBuffer
        {
            virtual CommandBufferCreateDesc get_create_desc() = 0;
            virtual ICommandPool* get_pool() = 0;
            virtual PIPELINE_TYPE get_current_dispatch() = 0;
        };

        template<typename EngineImplTraits>
        class CommandBufferBase : public DeviceObjectBase<typename EngineImplTraits::CommandBufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using CommandBufferInterface = typename EngineImplTraits::CommandBufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TCommandBufferBase = typename DeviceObjectBase<CommandBufferInterface, RenderDeviceImplType>;

            CommandBufferBase(RenderDeviceImplType* device, const CommandBufferCreateDesc& desc) : TCommandBufferBase(device), m_desc(desc) {  };

            virtual ~CommandBufferBase() = default;

            virtual CommandBufferCreateDesc get_create_desc() override
            {
                return m_desc;
            }

            virtual ICommandPool* get_pool() override
            {
                return m_pPool;
            }

            virtual PIPELINE_TYPE get_current_dispatch() override
            {
                return m_currentDispatch;
            }
            
        protected:
            CommandBufferCreateDesc m_desc;
            class ICommandPool* m_pPool;
            PIPELINE_TYPE m_currentDispatch;
        };
    }

}