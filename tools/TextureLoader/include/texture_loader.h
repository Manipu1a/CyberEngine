#pragma once

#include "graphics/interface/texture.hpp"

namespace Cyber
{
    struct ITextureLoader
    {
        virtual void create_texture(class RenderObject::IRenderDevice* device, class RenderObject::ITexture* texture) = 0;
        virtual const RenderObject::TextureCreateDesc& get_texture_desc() = 0;
        virtual const RenderObject::TextureSubResData& get_texture_sub_res_data() = 0;
        
    };
}
