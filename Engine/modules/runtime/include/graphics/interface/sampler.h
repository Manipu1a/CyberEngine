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
            using SamplerInterface = typename EngineImplTraits::SamplerInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TSamplerBase = typename SamplerBase<SamplerInterface, RenderDeviceImplType>;

            SamplerBase(RenderDeviceImplType* device) : TSamplerBase(device) {  };
            virtual ~SamplerBase() = default;
        protected:
            
        };
    }

}