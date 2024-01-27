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
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            CommandPoolBase(RenderDeviceImplType* device);

            virtual ~CommandPoolBase() = default;
        protected:
            class IQueue* pQueue;
            CommandPoolCreateDesc create_desc;
        };
    }

}