#include "texture_loader_impl.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)


static RenderObject::TextureCreateDesc TexDescFromTexLoadInfo()
{

}

TextureLoaderImpl::TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize, IDataBlob* dataBlob)
    : m_name(texLoadInfo.name), m_dataBlob(dataBlob)
{

}
TextureLoaderImpl::TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, class Image* image)
{
    
}

void TextureLoaderImpl::create_texture(RenderObject::IRenderDevice* device, RenderObject::ITexture* texture)
{

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

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE