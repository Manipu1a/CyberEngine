#include "tools/texture_loader/texture_utils.h"
#include "tools/texture_loader/texture_loader.h"
#include "platform/memory.h"
#include "CyberLog/Log.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace Cyber
{
    namespace Tools
    {
        void create_texture_from_file(const wchar_t* path)
        {
            TextureLoader* textureLoader = cyber_new<TextureLoader>();
            int x, y, comp;
            char* data;
            stbi_convert_wchar_to_utf8(data, sizeof(path), path);
            auto result = stbi_load(data, &x, &y, &comp, 0);

            CB_INFO("Texture loaded from file: {0} , Result: {1}", data, result);
        }
    }
}