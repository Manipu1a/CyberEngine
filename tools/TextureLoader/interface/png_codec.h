#pragma once

#include "image.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Image)

CYBER_TYPED_ENUM(DECODE_PNG_RESULT, uint32_t)
{
    // Decodeing was successful.
    DECODE_PNG_RESULT_OK = 0,
    // Invalid arguments (e.g. null pointers).
    DECODE_PNG_RESULT_INVALID_ARGUMENTS,
    // Invalid signature (the encoded file is not a PNG file)
    DECODE_PNG_RESULT_INVALID_SIGNATURE,
    // Failed to initialize the decoder.
    DECODE_PNG_RESULT_INITIALIZATION_FAILED,
    // Invalid bit depth
    DECODE_PNG_RESULT_INVALID_BIT_DEPTH,
    // An unexpected error occurred during decoding.
    DECODE_PNG_RESULT_DECODING_ERROR
};

CYBER_TYPED_ENUM(ENCODE_PNG_RESULT, uint32_t)
{
    // Encoding was successful.
    ENCODE_PNG_RESULT_OK = 0,
    // Invalid arguments (e.g. null pointers).
    ENCODE_PNG_RESULT_INVALID_ARGUMENTS,
    // Failed to initialize the encoder.
    ENCODE_PNG_RESULT_INITIALIZATION_FAILED,
};

DECODE_PNG_RESULT decode_png(IDataBlob* srcPngBits, IDataBlob* dstPixels, ImageDesc& imageDesc);

ENCODE_PNG_RESULT encode_png(uint8_t* srcRGBPixels, uint32_t width, uint32_t height, uint32_t strideInBytes, int pngColorType, IDataBlob* dstPngBits);

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE