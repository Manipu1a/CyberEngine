#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "common/flags.h"

namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IVertexInput
        {
            
        };

        template<typename EngineImplTraits>
        class VertexInputBase : public RenderObjectBase<typename EngineImplTraits::VertexInputInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            VertexInputBase(RenderDeviceImplType* device);
            virtual ~VertexInputBase() = default;
        protected:
            // resource name
            const char8_t* name;
            const char8_t* semantics_name;
            uint32_t semantics_index;
            uint32_t binding;
            ERHIFormat format;
        };
    }

}
