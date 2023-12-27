#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        //class CERenderDevice;
        struct CYBER_GRAPHICS_API ITextureView
        {

        };

        struct CYBER_GRAPHICS_API TextureViewCreateDesc
        {
            const char8_t* name;
            ERHIFormat format;
            ERHITextureViewUsages usages;
            ERHITextureViewAspect aspects;
            ERHITextureDimension dimension;
            uint32_t base_array_layer;
            uint32_t array_layer_count;
            uint32_t base_mip_level;
            uint32_t mip_level_count;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture_View : public RenderObjectBase<typename EngineImplTraits::TextureViewInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            const TextureViewCreateDesc get_create_desc() const { return create_desc; }
        protected:
            TextureViewCreateDesc create_desc;

            RenderDeviceImplType* device;

            friend RenderDeviceImplType;
        };
    }
}
