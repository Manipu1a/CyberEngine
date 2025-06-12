#include "png_codec.h"
#include "png.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

struct PNGReadFnState
{
    IDataBlob* pngBits;
    size_t offset;
};
typedef struct PNGReadFnState PNGReadFnState;

static void PngReadCallback(png_structp pngPtr, png_bytep data, png_size_t length)
{
    PNGReadFnState* state = (PNGReadFnState*)(png_get_io_ptr(pngPtr));
    uint8_t* dstPtr = (uint8_t*)state->pngBits->get_data_ptr() + state->offset;
    memcpy(data, dstPtr, length);
    state->offset += length;
}

DECODE_PNG_RESULT decode_png(IDataBlob* srcPngBits, IDataBlob* dstPixels, ImageDesc& dstImageDesc)
{
    if( srcPngBits == nullptr || dstPixels == nullptr )
    {
        return DECODE_PNG_RESULT_INVALID_ARGUMENTS;
    }

    const size_t pngSigSize = 0;
    png_const_bytep pngsig = (png_const_bytep)srcPngBits->get_data_ptr();
    // Let LibPNG check the signature. If this function returns 0, everything is fine.
    if(png_sig_cmp(pngsig, 0, pngSigSize) != 0)
    {
        return DECODE_PNG_RESULT_INVALID_SIGNATURE;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png)
    {
        return DECODE_PNG_RESULT_INITIALIZATION_FAILED;
    }

    png_infop info = png_create_info_struct(png);
    if(!info)
    {
        png_destroy_read_struct(&png, NULL, NULL);
        return DECODE_PNG_RESULT_INITIALIZATION_FAILED;
    }

    png_bytep* rowPtrs = NULL;
    if(setjmp(png_jmpbuf(png)))
    {
        if(rowPtrs)
            free(rowPtrs);
        // When an error occurs during parsing, libPNG will jump to here
        png_destroy_read_struct(&png, &info, (png_infopp)0);
        return DECODE_PNG_RESULT_DECODING_ERROR;
    }

    PNGReadFnState ReadState;
    ReadState.pngBits = srcPngBits;
    ReadState.offset = 0;

    png_set_read_fn(png, &ReadState, PngReadCallback);

    png_read_info(png, info);

    png_byte bit_depth = png_get_bit_depth(png, info);

    // PNG files store 16-bit pixels in network byte order (big-endian, ie
    // the most significant byte first). png_set_swap() shall swich the byte-order
    // to little-endian (ie, least significant byte first).
    if(bit_depth == 16)
    {
        png_set_swap(png);
    }

    png_byte color_type = png_get_color_type(png, info);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
    {
        // Transform paletted images into 8-bit rgba
        png_set_palette_to_rgb(png);
        png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    }

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16 bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        // Expand 1, 2, or 4-bit images to 8-bit
        png_set_expand_gray_1_2_4_to_8(png);
    }

    if(png_get_valid(png, info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png);
    }

    png_read_update_info(png, info);

    bit_depth = png_get_bit_depth(png, info);
    dstImageDesc.width = png_get_image_width(png, info);
    dstImageDesc.height = png_get_image_height(png, info);
    dstImageDesc.numComponents = png_get_channels(png, info);
    switch(bit_depth)
    {
        case 8: dstImageDesc.componentType = IMAGE_VALUE_TYPE_UINT8; break;
        case 16: dstImageDesc.componentType = IMAGE_VALUE_TYPE_UINT16; break;
        case 32: dstImageDesc.componentType = IMAGE_VALUE_TYPE_UINT32; break;
        default:
        {
            png_destroy_read_struct(&png, &info, (png_infopp)0);
            return DECODE_PNG_RESULT_INVALID_BIT_DEPTH;
        }
    }

    // Array of row pointers. One for every row.
    rowPtrs = (png_bytep*)malloc(sizeof(png_bytep) * dstImageDesc.height);

    // Allocate a buffer with enough space
    dstImageDesc.rowStride = dstImageDesc.width * (uint32_t)bit_depth * dstImageDesc.numComponents / 8u;

    // Align stride to 4 bytes
    dstImageDesc.rowStride = (dstImageDesc.rowStride + 3u) & ~3u;

    dstPixels->resize(dstImageDesc.height * (size_t)dstImageDesc.rowStride);
    png_bytep pRow0 = reinterpret_cast<png_bytep>(dstPixels->get_data_ptr());
    for(size_t i = 0;i < dstImageDesc.height; i++)
    {
        rowPtrs[i] = pRow0 + i * dstImageDesc.rowStride;
    }

    // Read the imagedata and write it to the addresses pointed to
    // by rowptrs (in other words: our image databuffer)
    png_read_image(png, rowPtrs);

    free(rowPtrs);
    png_destroy_read_struct(&png, &info, (png_infopp)0);

    return DECODE_PNG_RESULT_OK;
}

static void PngWriteCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    IDataBlob* pEncodedData = (IDataBlob*)png_get_io_ptr(png_ptr);
    size_t prevSize = pEncodedData->get_size();
    pEncodedData->resize(prevSize + length);
    uint8_t* bytes = (uint8_t*)pEncodedData->get_data_ptr();
    memcpy(bytes + prevSize, data, length);
}

ENCODE_PNG_RESULT encode_png(uint8_t* srcRGBPixels, uint32_t width, uint32_t height, uint32_t strideInBytes, int pngColorType, IDataBlob* dstPngBits)
{
    if( srcRGBPixels == nullptr || dstPngBits == nullptr || width == 0 || height == 0 || strideInBytes == 0)
    {
        return ENCODE_PNG_RESULT_INVALID_ARGUMENTS;
    }

    png_struct* strct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!strct)
    {
        return ENCODE_PNG_RESULT_INITIALIZATION_FAILED;
    }

    png_info* info = png_create_info_struct(strct);
    if(!info)
    {
        png_destroy_write_struct(&strct, NULL);
        return ENCODE_PNG_RESULT_INITIALIZATION_FAILED;
    }

    png_bytep* rowPtrs = NULL;
    if(setjmp(png_jmpbuf(strct)))
    {
        if(rowPtrs)
            free(rowPtrs);
        png_destroy_write_struct(&strct, &info);
        return ENCODE_PNG_RESULT_INITIALIZATION_FAILED;
    }

    png_set_IHDR(strct, info, width, height, 8, 
        pngColorType, 
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, 
        PNG_FILTER_TYPE_BASE);

    rowPtrs = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(size_t i = 0; i < height; i++)
    {
        rowPtrs[i] = (uint8_t*)srcRGBPixels + i * strideInBytes;
    }

    png_set_rows(strct, info, rowPtrs);

    png_set_write_fn(strct, dstPngBits, PngWriteCallback, NULL);
    png_write_png(strct, info, PNG_TRANSFORM_IDENTITY, NULL);

    free(rowPtrs);

    png_destroy_write_struct(&strct, &info);

    return ENCODE_PNG_RESULT_OK;
}
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE