#pragma once
#include "texture_loader.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

void create_texture_from_file(const char* file_path, const TextureLoadInfo& tex_load_info, RenderObject::ITexture** texture, RenderObject::IRenderDevice* device);

struct CopyPixelsAttribs
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t component_size = 0;
    const void* src_pixels = nullptr;
    uint32_t src_stride = 0;
    uint32_t src_comp_count = 0;
    void* dst_pixels = nullptr;
    uint32_t dst_stride = 0;
    uint32_t dst_comp_count = 0;
};
void copy_pixels(const CopyPixelsAttribs& attribs);

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE