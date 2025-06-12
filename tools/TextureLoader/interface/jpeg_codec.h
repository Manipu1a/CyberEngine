#pragma once

#include "core/interface/data_blob.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

CYBER_TYPED_ENUM(DECODE_JPEG_RESULT, uint32_t)
{
    /// JPEG image was decoded successfully.
    DECODE_JPEG_RESULT_OK = 0,

    /// Invalid arguments (e.g. null pointers).
    DECODE_JPEG_RESULT_INVALID_ARGUMENTS,

    /// Failed to initialize the decoder.
    DECODE_JPEG_RESULT_INITIALIZATION_FAILED,

    /// An unexpected error occurred during decoding.
    DECODE_JPEG_RESULT_DECODING_ERROR
};

CYBER_TYPED_ENUM(ENCODE_JPEG_RESULT, uint32_t)
{
    /// JPEG image was encoded successfully.
    ENCODE_JPEG_RESULT_OK = 0,

    /// Invalid arguments (e.g. null pointers).
    ENCODE_JPEG_RESULT_INVALID_ARGUMENTS,

    /// Failed to initialize the encoder.
    ENCODE_JPEG_RESULT_INITIALIZATION_FAILED,

    /// An unexpected error occurred during encoding.
    ENCODE_JPEG_RESULT_ENCODING_ERROR
};

DECODE_JPEG_RESULT decode_jpeg(IDataBlob* srcJpegBits, IDataBlob* dstPixels, ImageDesc& imageDesc);

ENCODE_JPEG_RESULT encode_jpeg(uint8_t* srcRGBPixels, uint32_t width, uint32_t height, int32_t quality, IDataBlob* dstJpegBits);

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
