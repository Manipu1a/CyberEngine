#include "texture_loader_impl.hpp"
#include "image.h"
#include "graphics/interface/render_device.hpp"
#include "common/graphics_utils.hpp"

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
        Image::create_from_data_blob(imgLoadInfo, dataBlob, &m_image);

    }


}

TextureLoaderImpl::TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, class Image* image)
{
    
}

void TextureLoaderImpl::create_texture(RenderObject::IRenderDevice* device, RenderObject::ITexture* texture)
{
    RenderObject::TextureData InitData = get_texture_data();
    texture = device->create_texture(m_textureCreateDesc, &InitData);
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
    m_textureCreateDesc.m_mipLevels = compute_mip_levels_count(imgDesc.width, imgDesc.height);
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
                default: cyber_assert(false, "Unsupported number of components ({0})", num_components);
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
        
    }

}
void TextureLoaderImpl::load_from_ktx(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize)
{

}
void TextureLoaderImpl::load_from_dds(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize)
{

}

void create_texture_loader_from_image()
{

}
void create_texture_loader_from_file(const char8_t* file_path, IMAGE_FILE_FORMAT image_format, const TextureLoadInfo& tex_load_info, ITextureLoader** texture_loader)
{
    const char8_t* sample_path = u8"../../../../samples/triangle";
    eastl::string fileNameAPI(eastl::string::CtorSprintf(), "%s/%s", sample_path, file_path);
    
    //const char* file_path_str = reinterpret_cast<const char*>(file_path);

    FILE* file = fopen(fileNameAPI.c_str(), "rb");
    uint8_t* bytes = nullptr;

    if(file)
    {
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        bytes = (uint8_t*)cyber_malloc(file_size);
        fread(bytes, file_size, 1, file);
        fclose(file);

        if(bytes != nullptr)
        {
            TextureLoaderImpl* impl = new TextureLoaderImpl(tex_load_info, bytes, file_size, nullptr);
            *texture_loader = impl;
        }
    }

}
void create_texture_loader_from_memory()
{

}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE