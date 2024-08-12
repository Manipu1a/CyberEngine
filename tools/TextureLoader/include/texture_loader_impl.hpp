#include "texture_loader.h"
#include "core/data_blob_impl.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

class TextureLoaderImpl final : public ITextureLoader
{
public:
    TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize, IDataBlob* dataBlob);
    TextureLoaderImpl(const TextureLoadInfo& texLoadInfo, class Image* image);

    virtual void create_texture(RenderObject::IRenderDevice* device, RenderObject::ITexture* texture) override;
    virtual const RenderObject::TextureCreateDesc& get_texture_desc() override;
    virtual const RenderObject::TextureSubResData& get_texture_sub_res_data(uint32_t mipLevel, uint32_t arraySlice) override;

private:
    void load_from_image(const TextureLoadInfo& texLoadInfo);
    void load_from_ktx(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize);
    void load_from_dds(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize);
    
private:
    const char8_t* m_name;
    class IDataBlob* m_dataBlob;
    RenderObject::TextureCreateDesc m_textureCreateDesc;
    eastl::vector<RenderObject::TextureSubResData> m_textureSubResData;
    eastl::vector<eastl::vector<uint8_t>> m_mips;
};
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE