#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"

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
            
        };

        template<typename EngineImplTraits>
        class CommandPoolBase : public RenderObjectBase<typename EngineImplTraits::CommandPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using CommandPoolInterface = typename EngineImplTraits::CommandPoolInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TCommandPoolBase = typename CommandPoolBase<CommandPoolInterface, RenderDeviceImplType>;

            CommandPoolBase(RenderDeviceImplType* device, const CommandPoolCreateDesc& desc) : TCommandPoolBase(device), m_desc(desc) {  };

            virtual ~CommandPoolBase() = default;
        protected:
            class IQueue* m_pQueue;
            CommandPoolCreateDesc m_desc;
        };
    }

}