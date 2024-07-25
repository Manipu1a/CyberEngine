#pragma once

#include "graphics/interface/texture.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

struct TextureLoadInfo
{
    const char8_t* name;

    GRAPHICS_RESOURCE_USAGE usage;

    GRAPHICS_RESOURCE_STATE flags;

    uint32_t mipLevels;

    CPU_ACCESS_FLAGS cpuAccessFlags;

    bool isSRGB;

    bool isGenerateMips;

    TEXTURE_FORMAT format;

    float alphaCutoff;

    FILTER_TYPE filter;

    explicit TextureLoadInfo(const char8_t* name,
                             GRAPHICS_RESOURCE_USAGE usage, 
                             GRAPHICS_RESOURCE_STATE flags, 
                             uint32_t mipLevels, 
                             CPU_ACCESS_FLAGS cpuAccessFlags, 
                             bool isSRGB, 
                             bool isGenerateMips, 
                             TEXTURE_FORMAT format, 
                             float alphaCutoff, 
                             FILTER_TYPE filter)
        : name(name), 
        usage(usage), 
        flags(flags), 
        mipLevels(mipLevels), 
        cpuAccessFlags(cpuAccessFlags), 
        isSRGB(isSRGB), 
        isGenerateMips(isGenerateMips), 
        format(format), 
        alphaCutoff(alphaCutoff), 
        filter(filter)
    {
    }

    TextureLoadInfo(){}
};

class ITextureLoader
{
    virtual void create_texture(class RenderObject::IRenderDevice* device, class RenderObject::ITexture* texture) = 0;
    virtual const RenderObject::TextureCreateDesc& get_texture_desc() = 0;
    virtual const RenderObject::TextureSubResData& get_texture_sub_res_data() = 0;
};

void create_texture_loader_from_image();
void create_texture_loader_from_file();
void create_texture_loader_from_memory();

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
