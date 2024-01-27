#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API ISampler
        {
            
        };

        template<typename EngineImplTraits>
        class SamplerBase : public RenderObjectBase<typename EngineImplTraits::SamplerInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            SamplerBase(RenderDeviceImplType* device);
            virtual ~SamplerBase() = default;
        protected:
            
        };
    }

}