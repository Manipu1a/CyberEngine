#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "render_object.h"
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
            
        };

        template<typename EngineImplTraits>
        class CommandBufferBase : public RenderObjectBase<typename EngineImplTraits::CommandBufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using CommandBufferInterface = typename EngineImplTraits::CommandBufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TCommandBufferBase = typename CommandBufferBase<CommandBufferInterface, RenderDeviceImplType>;

            CommandBufferBase(RenderDeviceImplType* device, const commandBufferDesc& desc) : TCommandBufferBase(device), m_desc(desc) {  };

            virtual ~CommandBufferBase() = default;
        protected:
            CommandBufferCreateDesc m_desc;
            class ICommandPool* m_pPool;
            PIPELINE_TYPE m_currentDispatch;
        };
    }

}