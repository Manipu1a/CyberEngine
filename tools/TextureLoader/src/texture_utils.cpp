#include "texture_utils.h"
#include "graphics/interface/render_device.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

void create_texture_from_file(const char* file_path, const TextureLoadInfo& tex_load_info, RenderObject::ITexture** texture, RenderObject::IRenderDevice* device)
{
    ITextureLoader* textureLoader = nullptr;

    create_texture_loader_from_file(file_path,IMAGE_FILE_FORMAT_UNKNOWN, tex_load_info, &textureLoader);

    textureLoader->create_texture(device, texture);
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE