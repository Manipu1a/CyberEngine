#pragma once

#include "graphics/interface/texture.hpp"
#include "image.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

struct TextureLoadInfo
{
    const char8_t* name;

    GRAPHICS_RESOURCE_USAGE usage;

    GRAPHICS_RESOURCE_BIND_FLAGS bindFlags;

    uint32_t mipLevels;

    CPU_ACCESS_FLAGS cpuAccessFlags;

    bool isSRGB;

    bool isGenerateMips;

    TEXTURE_FORMAT format;

    float alphaCutoff;

    FILTER_TYPE filter;

    explicit TextureLoadInfo(const char8_t* name,
                             GRAPHICS_RESOURCE_USAGE usage, 
                             GRAPHICS_RESOURCE_BIND_FLAGS bindFlags, 
                             uint32_t mipLevels, 
                             CPU_ACCESS_FLAGS cpuAccessFlags, 
                             bool isSRGB, 
                             bool isGenerateMips, 
                             TEXTURE_FORMAT format, 
                             float alphaCutoff, 
                             FILTER_TYPE filter)
        : name(name), 
        usage(usage), 
        bindFlags(bindFlags), 
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
    virtual const RenderObject::TextureSubResData& get_texture_sub_res_data(uint32_t mipLevel, uint32_t arraySlice) = 0;
    virtual RenderObject::TextureData get_texture_data() = 0;
};

void create_texture_loader_from_image();
void create_texture_loader_from_file(const char8_t* file_path, IMAGE_FILE_FORMAT image_format, const TextureLoadInfo& tex_load_info, ITextureLoader** texture_loader);
void create_texture_loader_from_memory();

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
