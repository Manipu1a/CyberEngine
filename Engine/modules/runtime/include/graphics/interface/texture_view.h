#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        struct CYBER_GRAPHICS_API TextureViewCreateDesc
        {
            const char8_t* name;
            class ITexture* texture;
            ERHIFormat format;
            ERHITextureViewUsages usages;
            ERHITextureViewAspect aspects;
            ERHITextureDimension dimension;
            uint32_t base_array_layer;
            uint32_t array_layer_count;
            uint32_t base_mip_level;
            uint32_t mip_level_count;
        };

        //class CERenderDevice;
        struct CYBER_GRAPHICS_API ITextureView
        {
            TextureViewCreateDesc& get_create_desc() { return create_desc; }
            TextureViewCreateDesc create_desc;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture_View : public RenderObjectBase<typename EngineImplTraits::TextureViewInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TextureViewInterface = typename EngineImplTraits::TextureViewInterface;
            using TTexture_View = RenderObjectBase<TextureViewInterface, RenderDeviceImplType>;
            Texture_View(RenderDeviceImplType* device) : TTexture_View(device) { }
            virtual ~Texture_View() {}
            
        protected:
            RenderDeviceImplType* device;

            friend RenderDeviceImplType;
        };
    }
}
