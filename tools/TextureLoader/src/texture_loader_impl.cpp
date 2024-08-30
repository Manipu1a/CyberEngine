#include "texture_loader_impl.hpp"
#include "image.h"
#include "graphics/interface/render_device.hpp"
#include "common/graphics_utils.h"
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
    const auto channelDepth = GetValueSize(imgDesc.componentType) * 8;

    m_textureCreateDesc;
    m_textureCreateDesc.m_width = imgDesc.width;

}
void TextureLoaderImpl::load_from_ktx(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize)
{

}
void TextureLoaderImpl::load_from_dds(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize)
{

}
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE