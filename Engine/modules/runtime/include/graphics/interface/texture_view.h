#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CERenderDevice;
        class Texture;

        struct CYBER_GRAPHICS_API TextureViewCreateDesc
        {
            const char8_t* name;
            Texture* texture;
            ERHIFormat format;
            ERHITextureViewUsages usages;
            ERHITextureViewAspect aspects;
            ERHITextureDimension dimension;
            uint32_t base_array_layer;
            uint32_t array_layer_count;
            uint32_t base_mip_level;
            uint32_t mip_level_count;
        };


        class CYBER_GRAPHICS_API Texture_View
        {
        public:

            const TextureViewCreateDesc get_create_desc() const { return create_desc; }
        protected:
            TextureViewCreateDesc create_desc;
            RenderObject::CERenderDevice* device;

            friend class RenderObject::CERenderDevice;

        };
    }
}
