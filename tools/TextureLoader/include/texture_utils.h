#pragma once
#include "texture_loader.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

void create_texture_from_file(const char8_t* file_path, const TextureLoadInfo& tex_load_info, RenderObject::ITexture** texture, RenderObject::IRenderDevice* device);

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE