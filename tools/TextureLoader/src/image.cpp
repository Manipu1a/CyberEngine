#include "image.h"
#include "platform/memory.h"
#include "core/data_blob_impl.hpp"
#include "CyberLog/Log.h"
#include "eastl/string.h"
#include "png_codec.h"
#include "jpeg_codec.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

void Image::create_from_data_blob(const ImageLoadInfo& imageLoadInfo, IDataBlob* dataBlob, Image** image)
{
    *image = cyber_new<Image>(imageLoadInfo, dataBlob);
}

void Image::create_from_memory(const ImageDesc& imageDesc, IDataBlob* dataBlob, Image** image)
{
    *image = cyber_new<Image>(imageDesc, dataBlob);
    
}

void Image::encode(const EncodeInfo& encodeInfo, IDataBlob** dataBlob)
{

}

IMAGE_FILE_FORMAT Image::get_file_format(const uint8_t* data, size_t dataSize, const char8_t* filePath)
{
    if(data != nullptr)
    {
        if(dataSize >= 4)
        {
            if(dataSize >= 8 && data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 
                && data[4] == 0x0D && data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A)
            {
                return IMAGE_FILE_FORMAT_PNG;
            }
            else if(dataSize >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF)
            {
                return IMAGE_FILE_FORMAT_JPEG;
            }
            else if(dataSize >=4 && (data[0] == 0x49 && data[1] == 0x20 && data[2] == 0x49 ) ||
                (data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
                (data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A) ||
                (data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2B))
            {
                return IMAGE_FILE_FORMAT_TIFF;
            }
            else if(dataSize >= 2 && data[0] == 0x01 && data[1] == 0xDA)
            {
                return IMAGE_FILE_FORMAT_SGI;
            }
            else if(dataSize >=4 && data[0] == 0x44 && data[1] == 0x44 && data[2] == 0x53 && data[3] == 0x20)
            {
                return IMAGE_FILE_FORMAT_DDS;
            }

            static constexpr uint8_t KTX10_IDENTIFIER[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
            static constexpr uint8_t KTX20_IDENTIFIER[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
            if(dataSize >= 12 && (memcmp(data, KTX10_IDENTIFIER, sizeof(KTX10_IDENTIFIER)) == 0 || memcmp(data, KTX20_IDENTIFIER, 12) == 0))
            {
                return IMAGE_FILE_FORMAT_KTX;
            }

        }
        else
        {
            return IMAGE_FILE_FORMAT_UNKNOWN;
        }
    }


    if(filePath != nullptr)
    {
        const char* dotPos = strrchr((const char*)filePath, '.');
        
        if(dotPos == nullptr)
        {
            CB_ERROR("Unable to recognize file format: file name {0} does not contain extension", (char*)filePath);
            return IMAGE_FILE_FORMAT_UNKNOWN;
        }

        auto* extension = dotPos + 1;
        if(*extension == 0)
        {
            CB_ERROR("Unable to recognize file format: file name {0} contain empty extension", (char*)filePath);
            return IMAGE_FILE_FORMAT_UNKNOWN;
        }

        eastl::string ext(extension);
        ext.make_lower();
        if(ext == "png")
        {
            return IMAGE_FILE_FORMAT_PNG;
        }
        else if(ext == "jpg" || ext == "jpeg")
        {
            return IMAGE_FILE_FORMAT_JPEG;
        }
        else if(ext == "tiff" || ext == "tif")
        {
            return IMAGE_FILE_FORMAT_TIFF;
        }
        else if(ext == "dds")
        {
            return IMAGE_FILE_FORMAT_DDS;
        }
        else if(ext == "ktx")
        {
            return IMAGE_FILE_FORMAT_KTX;
        }
        else if(ext == "sgi" || ext == "rgb" || ext == "rgba" || ext == "bw" || ext == "int" || ext == "inta")
        {
            return IMAGE_FILE_FORMAT_SGI;
        }
    }

    return IMAGE_FILE_FORMAT_UNKNOWN;
}

Image::Image(const ImageDesc& desc, IDataBlob* dataBlob) : m_desc(desc), m_dataBlob(dataBlob)
{

}

Image::Image(const ImageLoadInfo& loadInfo, IDataBlob* dataBlob) : m_dataBlob(Core::DataBlobImpl::create())
{
    if(loadInfo.format == IMAGE_FILE_FORMAT_TIFF)
    {
        
    }
    else if(loadInfo.format == IMAGE_FILE_FORMAT_PNG)
    {
        auto res = decode_png(dataBlob, m_dataBlob, m_desc);
        if(res != DECODE_PNG_RESULT_OK)
        {
             CB_ERROR("Failed to decode PNG image: {0}", (uint32_t)res);
             m_dataBlob = nullptr;
             return;
        }
    }
    else if(loadInfo.format == IMAGE_FILE_FORMAT_JPEG)
    {

    }
    else if(loadInfo.format == IMAGE_FILE_FORMAT_SGI)
    {

    }
    else if(loadInfo.format == IMAGE_FILE_FORMAT_DDS)
    {

    }
    else if(loadInfo.format == IMAGE_FILE_FORMAT_KTX)
    {

    }
    else {
    
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE