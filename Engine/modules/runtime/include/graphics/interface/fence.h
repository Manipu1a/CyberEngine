#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IFence
        {
            
        };

        template<typename EngineImplTraits>
        class FenceBase : public RenderObjectBase<typename EngineImplTraits::CommandPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            FenceBase(RenderDeviceImplType* device);
            virtual ~FenceBase() = default;
        protected:
        
        };
    }

}