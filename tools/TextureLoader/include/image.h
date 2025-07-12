#pragma once
#include "log/Log.h"
#include "core/interface/data_blob.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)
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

CYBER_TYPED_ENUM(IMAGE_VALUE_TYPE, uint8_t)
{
    IMAGE_VALUE_TYPE_UNDEFINED = 0,
    IMAGE_VALUE_TYPE_INT8,
    IMAGE_VALUE_TYPE_INT16,
    IMAGE_VALUE_TYPE_INT32,
    IMAGE_VALUE_TYPE_UINT8,
    IMAGE_VALUE_TYPE_UINT16,
    IMAGE_VALUE_TYPE_UINT32,
    IMAGE_VALUE_TYPE_FLOAT16,
    IMAGE_VALUE_TYPE_FLOAT32,
    IMAGE_VALUE_TYPE_FLOAT64,
    IMAGE_VALUE_TYPE_COUNT,
};

static uint32_t get_value_size(IMAGE_VALUE_TYPE Val)
{
    switch (Val)
    {
        case IMAGE_VALUE_TYPE_INT8: return sizeof(int8_t);
        case IMAGE_VALUE_TYPE_INT16: return sizeof(int16_t);
        case IMAGE_VALUE_TYPE_INT32: return sizeof(int32_t);
        case IMAGE_VALUE_TYPE_UINT8: return sizeof(uint8_t);
        case IMAGE_VALUE_TYPE_UINT16: return sizeof(uint16_t);
        case IMAGE_VALUE_TYPE_UINT32: return sizeof(uint32_t);
        case IMAGE_VALUE_TYPE_FLOAT16: return sizeof(uint16_t); // 16-bit float
        case IMAGE_VALUE_TYPE_FLOAT32: return sizeof(float);
        case IMAGE_VALUE_TYPE_FLOAT64: return sizeof(double);
        case IMAGE_VALUE_TYPE_UNDEFINED:
        case IMAGE_VALUE_TYPE_COUNT:
        default:
            cyber_assert(false, "Invalid IMAGE_VALUE_TYPE: {0}", static_cast<uint8_t>(Val));
            return 0; // Should never reach here, but just in case
    }
}

struct ImageLoadInfo
{
    IMAGE_FILE_FORMAT format;
};

struct ImageDesc
{
    uint32_t width;

    uint32_t height;

    IMAGE_VALUE_TYPE componentType;

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
        //TEXTURE_FORMAT textureFormat;
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

    static IMAGE_FILE_FORMAT get_file_format(const uint8_t* data, size_t dataSize, const char8_t* filePath = nullptr);

public:
    Image(const ImageDesc& desc, IDataBlob* dataBlob);
    Image(const ImageLoadInfo& loadInfo, IDataBlob* dataBlob);

private:
    ImageDesc m_desc;

    IDataBlob* m_dataBlob;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
