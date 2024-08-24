#include "core/interface/data_blob.h"
#include "platform/configure.h"
#include "graphics/interface/graphics_types.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Image)
CYBER_TYPED_ENUM(IMAGE_FILE_FORMAT, uint8_t)
{
    IMAGE_FILE_FORMAT_UNKNOWN = 0,

    IMAGE_FILE_FORMAT_JPEG = 1,

    IMAGE_FILE_FORMAT_PNG = 2,

    IMAGE_FILE_FORMAT_TIFF = 3,

    IMAGE_FILE_FORMAT_DDS = 4,

    IMAGE_FILE_FORMAT_KTX = 5,

    IMAGE_FILE_FORMAT_SGI = 6,
};

struct ImageLoadInfo
{
    IMAGE_FILE_FORMAT format;
};

struct ImageDesc
{
    uint32_t width;

    uint32_t height;

    VALUE_TYPE componentType;

    uint32_t numComponents;

    uint32_t rowStride;
};

class Image
{
public:
    static void create_from_data_blob(const ImageLoadInfo& imageLoadInfo, IDataBlob* dataBlob, Image** image);

    static void create_from_memory(const ImageDesc& imageDesc, IDataBlob* dataBlob, Image** image);

    struct EncodeInfo
    {
        uint32_t width;
        uint32_t height;
        TEXTURE_FORMAT textureFormat;
        bool keepAlpha;
        const void* data;
        uint32_t stride;
        IMAGE_FILE_FORMAT fileFormat;
        int jepgQuality;
    };

    static void encode(const EncodeInfo& encodeInfo, IDataBlob** dataBlob);

    const ImageDesc& get_desc() const { return m_desc; }

    IDataBlob* get_data_blob() { return m_dataBlob; }
    
    const IDataBlob* get_data_blob() const { return m_dataBlob; }

    static IMAGE_FILE_FORMAT GetFileFormat(const uint8_t* data, size_t dataSize, const char8_t* filePath);

public:
    Image(const ImageDesc& desc, IDataBlob* dataBlob);
    Image(const ImageLoadInfo& loadInfo, IDataBlob* dataBlob);

private:
    ImageDesc m_desc;

    IDataBlob* m_dataBlob;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
