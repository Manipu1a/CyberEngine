#include "image.h"
#include "platform/memory.h"
#include "core/data_blob_impl.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Image)

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

IMAGE_FILE_FORMAT Image::GetFileFormat(const uint8_t* data, size_t dataSize, const char8_t* filePath)
{
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