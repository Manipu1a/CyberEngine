#include "texture_loader_impl.hpp"
#include "image.h"
#include "graphics/interface/render_device.hpp"
#include "common/graphics_utils.hpp"
#include "texture_utils.h"
#include <algorithm>
#include "math/common.h"
#include "core/file_helper.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)


static RenderObject::TextureCreateDesc TexDescFromTexLoadInfo(const TextureLoadInfo& texLoadInfo)
{
    RenderObject::TextureCreateDesc createDesc;
    createDesc.m_name = texLoadInfo.name;
    createDesc.m_format = texLoadInfo.format;
    createDesc.m_usage = texLoadInfo.usage;
    createDesc.m_bindFlags = texLoadInfo.bindFlags;
    createDesc.m_cpuAccessFlags = texLoadInfo.cpuAccessFlags;
    return createDesc;
}

TextureLoaderImpl::TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize, IDataBlob* dataBlob)
    :  m_name(texLoadInfo.name), m_dataBlob(dataBlob), m_textureCreateDesc(TexDescFromTexLoadInfo(texLoadInfo))
{
    const auto imgFileFormat = Image::get_file_format(data, dataSize);
    if(imgFileFormat == IMAGE_FILE_FORMAT_UNKNOWN)
    {
        cyber_assert(false, "Unable to derive image format.");
    }

    if(imgFileFormat == IMAGE_FILE_FORMAT_PNG || 
       imgFileFormat == IMAGE_FILE_FORMAT_JPEG || 
       imgFileFormat == IMAGE_FILE_FORMAT_TIFF || 
       imgFileFormat == IMAGE_FILE_FORMAT_SGI)
    {
        ImageLoadInfo imgLoadInfo;
        imgLoadInfo.format = imgFileFormat;
        if(!dataBlob)
        {
            m_dataBlob = Core::DataBlobImpl::create(dataSize, data);
        }
        Image::create_from_data_blob(imgLoadInfo, m_dataBlob, &m_image);
        load_from_image(texLoadInfo);
    }
}

TextureLoaderImpl::TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, class Image* image)
{
    
}

void TextureLoaderImpl::create_texture(RenderObject::IRenderDevice* device, RenderObject::ITexture** texture)
{
    RenderObject::TextureData InitData = get_texture_data();
    *texture = device->create_texture(m_textureCreateDesc, &InitData);
}

const RenderObject::TextureCreateDesc& TextureLoaderImpl::get_texture_desc()
{
    return m_textureCreateDesc;
}

const RenderObject::TextureSubResData& TextureLoaderImpl::get_texture_sub_res_data(uint32_t mipLevel, uint32_t arraySlice)
{
    const auto subResIndex = arraySlice * m_textureCreateDesc.m_mipLevels + mipLevel;  
    cyber_check(subResIndex < m_textureSubResData.size());
    return m_textureSubResData[subResIndex];
}

void TextureLoaderImpl::load_from_image(const TextureLoadInfo& texLoadInfo)
{
    const auto& imgDesc = m_image->get_desc();
    const auto channelDepth = get_value_size(imgDesc.componentType) * 8;

    m_textureCreateDesc.m_dimension = TEX_DIMENSION_2D;
    m_textureCreateDesc.m_width = imgDesc.width;
    m_textureCreateDesc.m_height = imgDesc.height;
    
    if(texLoadInfo.isGenerateMips)
    {
        m_textureCreateDesc.m_mipLevels = compute_mip_levels_count(imgDesc.width, imgDesc.height);
    }
    if(texLoadInfo.mipLevels > 0)
    {
        m_textureCreateDesc.m_mipLevels = std::min(m_textureCreateDesc.m_mipLevels, texLoadInfo.mipLevels);
    }

    uint32_t num_components = 0;
    if(m_textureCreateDesc.m_format == TEX_FORMAT_UNKNOWN)
    {
        num_components = imgDesc.numComponents == 3 ? 4 : imgDesc.numComponents;
        if(channelDepth == 8)
        {
            switch(num_components)
            {
                case 1 : m_textureCreateDesc.m_format = TEX_FORMAT_R8_UNORM; break; 
                case 2 : m_textureCreateDesc.m_format = TEX_FORMAT_RG8_UNORM; break;
                case 4 : m_textureCreateDesc.m_format = texLoadInfo.isSRGB ? TEX_FORMAT_RGBA8_UNORM_SRGB : TEX_FORMAT_RGBA8_UNORM; break; 
                cyber_warn(false, "Unsupported number of components ({0})", num_components);
            }
        }
        else if(channelDepth == 16)
        {
            switch(num_components)
            {
                case 1 : m_textureCreateDesc.m_format = TEX_FORMAT_R16_UNORM; break; 
                case 2 : m_textureCreateDesc.m_format = TEX_FORMAT_RG16_UNORM; break; 
                case 4 : m_textureCreateDesc.m_format = TEX_FORMAT_RGBA16_UNORM; break; 
                default: cyber_assert(false, "Unsupported number of components ({0})", num_components);
            }
        }
        else 
        {
            cyber_assert(false, "Unsupported channel depth ({0})", channelDepth);
        }
    }
    else
    {
        const auto& tex_fmt_desc = get_texture_format_attribs(m_textureCreateDesc.m_format);

        num_components = tex_fmt_desc.num_components;

        if(tex_fmt_desc.component_size != channelDepth / 8)
        {
            cyber_assert(false, "Image channel depth ({0}) does not match texture format ({1})", channelDepth, tex_fmt_desc.component_size);
        }
    }

    m_textureSubResData.resize(m_textureCreateDesc.m_mipLevels);
    m_mips.resize(m_textureCreateDesc.m_mipLevels);

    if(imgDesc.numComponents != num_components)
    {
        auto dst_stride = imgDesc.width * num_components * (channelDepth / 8);
        dst_stride = align_up(dst_stride, (uint32_t)4);
        m_mips[0].resize(size_t(dst_stride) * size_t(imgDesc.height));
        m_textureSubResData[0].pData = m_mips[0].data();
        m_textureSubResData[0].stride = dst_stride;
        CopyPixelsAttribs copyAttribs;
        copyAttribs.width = imgDesc.width;
        copyAttribs.height = imgDesc.height;
        copyAttribs.component_size = channelDepth / 8;
        copyAttribs.src_pixels = m_image->get_data_blob()->get_data_ptr();
        copyAttribs.src_stride = imgDesc.rowStride;
        copyAttribs.src_comp_count = imgDesc.numComponents;
        copyAttribs.dst_pixels = m_mips[0].data();
        copyAttribs.dst_stride = dst_stride;
        copyAttribs.dst_comp_count = num_components;
        copy_pixels(copyAttribs);
    }
    else
    {
        m_textureSubResData[0].pData = m_image->get_data_blob()->get_data_ptr();
        m_textureSubResData[0].stride = imgDesc.rowStride;
    }

    // 处理mipmaps
    for(uint32_t m = 1; m < m_textureCreateDesc.m_mipLevels; ++m)
    {

    }
}

void TextureLoaderImpl::load_from_dds(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize)
{

}

void create_texture_loader_from_image()
{

}

void create_texture_loader_from_file(const char* file_path, IMAGE_FILE_FORMAT image_format, const TextureLoadInfo& tex_load_info, ITextureLoader** texture_loader)
{
    Core::FileHelper texture_file(file_path, Core::FILE_ACCESS_MODE::FILE_ACCESS_READ);
    IDataBlob* data_blob = nullptr;
    texture_file->read(&data_blob);

    //const char8_t* sample_path = u8"../../../../samples/triangle";
    //eastl::string fileNameAPI(eastl::string::CtorSprintf(), "%s/%s", sample_path, file_path);
    
    //const char* file_path_str = reinterpret_cast<const char*>(file_path);

    //FILE* file = fopen(fileNameAPI.c_str(), "rb");
    if(data_blob)
    {
        TextureLoaderImpl* impl = new TextureLoaderImpl(tex_load_info, (uint8_t*)data_blob->get_data_ptr(), data_blob->get_size(), data_blob);
        *texture_loader = impl;
    }
}

void create_texture_loader_from_memory()
{

}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE