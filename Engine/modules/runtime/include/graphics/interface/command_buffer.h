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
            bool is_secondary : 1;
        };

        struct CYBER_GRAPHICS_API ICommandBuffer
        {
            
        };

        template<typename EngineImplTraits>
        class CommandBufferBase : public RenderObjectBase<typename EngineImplTraits::CommandBufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            CommandBufferBase(RenderDeviceImplType* device);

            virtual ~CommandBufferBase() = default;
        protected:
            class ICommandPool* pPool;
            PIPELINE_TYPE mCurrentDispatch;
        };
    }

}