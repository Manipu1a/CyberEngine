#include "jpeg_codec.h"
#include "jpeglib.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Image)

DECODE_JPEG_RESULT decode_jpeg(IDataBlob* srcJpegBits, IDataBlob* dstPixels, ImageDesc& imageDesc)
{

    return DECODE_JPEG_RESULT_OK;
}

ENCODE_JPEG_RESULT encode_jpeg(uint8_t* srcRGBPixels, uint32_t width, uint32_t height, int32_t quality, IDataBlob* dstJpegBits)
{
    return ENCODE_JPEG_RESULT_OK;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE