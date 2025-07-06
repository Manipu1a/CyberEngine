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

template<typename ChannelType>
void copy_pixel_impl(const CopyPixelsAttribs& attribs)
{
    cyber_check(sizeof(ChannelType) == attribs.component_size);

    auto process_row_pixels = [&attribs](auto&& Handler) {
        for(size_t row = 0; row < size_t(attribs.height); ++row)
        {
            const auto* src_row = reinterpret_cast<const ChannelType*>(static_cast<const uint8_t*> (attribs.src_pixels) + row * attribs.src_stride);
            auto* dst_row = reinterpret_cast<ChannelType*>(static_cast<uint8_t*>(attribs.dst_pixels) + row * attribs.dst_stride);
            Handler(src_row, dst_row);
        }
    };

    const auto row_size = attribs.width * attribs.component_size * attribs.src_comp_count;
    if(attribs.src_comp_count == attribs.dst_comp_count)
    {
        if(row_size == attribs.src_stride && row_size == attribs.dst_stride)
        {
            memcpy(attribs.dst_pixels, attribs.src_pixels, row_size * attribs.height);
        }
        else 
        {
            process_row_pixels([row_size](const ChannelType* src_row, ChannelType* dst_row) {
                memcpy(dst_row, src_row, row_size);
            });
        }
    }
    else if(attribs.dst_comp_count < attribs.src_comp_count)
    {
        process_row_pixels([&attribs](const ChannelType* src_row, ChannelType* dst_row) {
            for(size_t i = 0; i < attribs.width; ++i)
            {
                auto* dst_pixel = dst_row + i * attribs.dst_comp_count;
                const auto* src_pixel = src_row + i * attribs.src_comp_count;
                for(size_t j = 0; j < attribs.dst_comp_count; ++j)
                {
                    dst_pixel[j] = src_pixel[j];
                }
            }
        });
    }
    else 
    {
        process_row_pixels([&attribs](const ChannelType* src_row, ChannelType* dst_row) {
            for(size_t i = 0; i < attribs.width; ++i)
            {
                auto* dst_pixel = dst_row + i * attribs.dst_comp_count;
                const auto* src_pixel = src_row + i * attribs.src_comp_count;

                for(size_t j = 0; j < attribs.src_comp_count; ++j)
                {
                    dst_pixel[j] = src_pixel[j];
                }

                for(size_t j = attribs.src_comp_count; j < attribs.dst_comp_count; ++j)
                {
                    dst_pixel[j] = j < 3 ? (attribs.src_comp_count == 1 ? src_pixel[0] : 0) : // 单通道贴图使用r填充
                                        std::numeric_limits<ChannelType>::max(); // alpha通道填充为1
                }
            }
        });
    }
}

void copy_pixels(const CopyPixelsAttribs& attribs)
{
    switch(attribs.component_size)
    {
        case 1: copy_pixel_impl<uint8_t>(attribs); break;
        case 2: copy_pixel_impl<uint16_t>(attribs); break;
        case 4: copy_pixel_impl<uint32_t>(attribs); break;
        default: cyber_assert(false,  "Unsupported component size");
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE