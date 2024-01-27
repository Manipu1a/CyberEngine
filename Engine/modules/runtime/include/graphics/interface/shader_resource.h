#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "rhi.h"

namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IShaderResource
        {
            
        };

        template<typename EngineImplTraits>
        class ShaderResourceBase : public RenderObjectBase<typename EngineImplTraits::ShaderResourceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            ShaderResourceBase(RenderDeviceImplType* device);
            virtual ~ShaderResourceBase() = default;
        protected:
            const char8_t* name;
            uint64_t name_hash;
            ERHIResourceType type;
            ERHITextureDimension dimension;
            uint32_t set;
            uint32_t binding;
            uint32_t size;
            uint32_t offset;
            ERHIShaderStages stages;
        };
    }

}
