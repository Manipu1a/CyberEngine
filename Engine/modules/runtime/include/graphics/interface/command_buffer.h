#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "render_pass.h"

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
        class CommandBufferBase : public RenderObjectBase<typename EngineImplTraits::CommandPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            CommandBufferBase(RenderDeviceImplType* device);

            virtual ~CommandBufferBase() = default;
        protected:
            class ICommandPool* pPool;
            ERHIPipelineType mCurrentDispatch;
        };
    }

}