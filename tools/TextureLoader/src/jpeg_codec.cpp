#include "jpeg_codec.h"
#include "jpeglib.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

struct my_jpeg_error_mgr
{
    struct jpeg_error_mgr pub;   
    char padding[8];
    jmp_buf setjmp_buffer;       
};
typedef struct my_jpeg_error_mgr my_jpeg_error_mgr;

// Here's the routine that will replace the standard error_exit method:
METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    // cinfo->err really points to a my_jpeg_error_mgr struct, so coerce pointer
    my_jpeg_error_mgr* myerr = (my_jpeg_error_mgr*)cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    // Return control to the setjmp point
    longjmp(myerr->setjmp_buffer, 1);
}

DECODE_JPEG_RESULT decode_jpeg(IDataBlob* srcJpegBits, IDataBlob* dstPixels, ImageDesc& dstImageDesc)
{
    if( srcJpegBits == nullptr || dstPixels == nullptr )
    {
        return DECODE_JPEG_RESULT_INVALID_ARGUMENTS;
    }

    // This struct contains the JPEG decompression parameters and pointers to
    // working space (which is allocated as needed by the JPEG library).
    struct jpeg_decompress_struct cinfo;

    // We use our private extension JPEG error handler.
    // Note that this struct must live as long as the main JPEG parameter
    // struct, to avoid dangling-pointer problems.
    my_jpeg_error_mgr jerr;

    // Step 1: allocate and initialize JPEG decompression object

    // We set up the normal JPEG error routines, then override error_exit.
    cinfo.err           = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    // Establish the setjmp return context for my_error_exit to use.
    if (setjmp(jerr.setjmp_buffer))
    {
        // If we get here, the JPEG code has signaled an error.
        // We need to clean up the JPEG object, close the input file, and return.
        jpeg_destroy_decompress(&cinfo);
        return DECODE_JPEG_RESULT_INITIALIZATION_FAILED;
    }

    // Now we can initialize the JPEG decompression object.
    jpeg_create_decompress(&cinfo);

    // Step 2: specify data source
    unsigned char* pSrcPtr = reinterpret_cast<unsigned char*>(srcJpegBits->get_data_ptr());
    unsigned long  SrcSize = srcJpegBits->get_size();
    jpeg_mem_src(&cinfo, pSrcPtr, SrcSize);

    // Step 3: read file parameters with jpeg_read_header()
    jpeg_read_header(&cinfo, TRUE);
    // We can ignore the return value from jpeg_read_header since
    //   (a) suspension is not possible with the memory data source, and
    //   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    // See libjpeg.txt for more info.

    // Step 4: set parameters for decompression

    // In this example, we don't need to change any of the defaults set by
    // jpeg_read_header(), so we do nothing here.

    // Step 5: Start decompressor
    jpeg_start_decompress(&cinfo);
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // We may need to do some setup of our own at this point before reading
    // the data.  After jpeg_start_decompress() we have the correct scaled
    // output image dimensions available, as well as the output colormap
    // if we asked for color quantization.
    dstImageDesc.width = cinfo.output_width;
    dstImageDesc.height = cinfo.output_height;
    dstImageDesc.componentType = VALUE_TYPE_UINT8;
    dstImageDesc.numComponents = cinfo.output_components;
    dstImageDesc.rowStride = cinfo.output_width * cinfo.output_components;
    dstImageDesc.rowStride = (dstImageDesc.rowStride + 3u) & ~3u;
    
    dstPixels->resize((size_t)dstImageDesc.rowStride * dstImageDesc.height);
    // Step 6: while (scan lines remain to be read)
    //           jpeg_read_scanlines(...);

    // Here we use the library's state variable cinfo.output_scanline as the
    // loop counter, so that we don't have to keep track ourselves.
    while(cinfo.output_scanline < cinfo.output_height)
    {
        // jpeg_read_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could ask for
        // more than one scanline at a time if that's more convenient.

        uint8_t* pScanline0 = reinterpret_cast<uint8_t*>(dstPixels->get_data_ptr());
        uint8_t* pDstScanline = pScanline0 + cinfo.output_scanline * dstImageDesc.rowStride;
        JSAMPROW rowPtrs[1];
        rowPtrs[0] = (JSAMPROW)pDstScanline;
        jpeg_read_scanlines(&cinfo, rowPtrs, 1);
    }

    // Step 7: Finish decompression
    jpeg_finish_decompress(&cinfo);
    // We can ignore the return value since suspension is not possible
    // with the stdio data source.

    // Step 8: Release JPEG decompression object
    // This is an important step since it will release a good deal of memory.
    jpeg_destroy_decompress(&cinfo);

    // At this point you may want to check to see whether any corrupt-data
    // warnings occurred (test whether jerr.pub.num_warnings is nonzero).
    return DECODE_JPEG_RESULT_OK;
}

ENCODE_JPEG_RESULT encode_jpeg(uint8_t* srcRGBPixels, uint32_t width, uint32_t height, int32_t quality, IDataBlob* dstJpegBits)
{
    if( srcRGBPixels == nullptr || dstJpegBits == nullptr || width == 0 || height == 0 || quality < 0)
    {
        return ENCODE_JPEG_RESULT_INVALID_ARGUMENTS;
    }

    // This struct contains the JPEG compression parameters and pointers to
    // working space (which is allocated as needed by the JPEG library).
    // It is possible to have several such structures, representing multiple
    // compression/decompression processes, in existence at once. We refer to
    // any one struct (and its associated working data) as a "JPEG object".
    struct jpeg_compress_struct cinfo;

    // This struct represents a JPEG error handler. It is declared separately
    // because applications often want to supply a specialized error handler
    // (see the second half of this file for an example). But here we just
    // take the easy way out and use the standard error handler, which will
    // print messages to stderr and call exit() if compression fails.
    // Note that this struct must live as long as the main JPEG parameter
    // struct, to avoid dangling-pointer problems.
    struct jpeg_error_mgr jerr;

    // Step 1: allocate and initialize JPEG compression object

    // We have to set up the error handler first, in case the initialization
    // step fails.  (Unlikely, but it could happen if you are out of memory.)
    // This routine fills in the contents of struct jerr, and returns jerr's
    // address which we place into the link field in cinfo.
    cinfo.err = jpeg_std_error(&jerr);
    // Now we can initialize the JPEG compression object.
    jpeg_create_compress(&cinfo);

    // Step 2: specify data destination (eg, a file)
    // Note: steps 2 and 3 can be done in either order.
    unsigned char* mem = nullptr;
    unsigned long mem_size = 0;
    jpeg_mem_dest(&cinfo, &mem, &mem_size);

    // Step 3: set parameters for compression
    // First we supply a description of the input image.
    // Four fields of the cinfo struct must be filled in:
    cinfo.image_width = width; // image width and height, in pixels
    cinfo.image_height = height;
    cinfo.input_components = 3; // # of color components per pixel
    cinfo.in_color_space = JCS_RGB; // colorspace of input image
    // Now use the library's routine to set default compression parameters.
    // (You must set at least cinfo.in_color_space before calling this,
    // since the defaults depend on the source color space.)
    jpeg_set_defaults(&cinfo);
    // Now you can set any non-default parameters you wish to.
    // Here we just illustrate the use of quality (quantization table) scaling:
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    // Step 4: Start compressor
    // TRUE ensures that we will write a complete interchange-JPEG file.
    // Pass TRUE unless you are very sure of what you're doing.
    jpeg_start_compress(&cinfo, TRUE);

    // Step 5: while (scan lines remain to be written)
    //          jpeg_write_scanlines(...);

    // Here we use the library's state variable cinfo.next_scanline as the
    // loop counter, so that we don't have to keep track ourselves.
    // To keep things simple, we pass one scanline per call; you can pass
    // more if you wish, though.
    uint32_t row_stride = width * 3; // JSAMPLEs per row in srcRGBPixels

    while(cinfo.next_scanline < cinfo.image_height)
    {
        // jpeg_write_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could pass
        // more than one scanline at a time if that's more convenient.
        JSAMPROW row_pointer[1];
        row_pointer[0] = &srcRGBPixels[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Step 6: Finish compression
    jpeg_finish_compress(&cinfo);

    dstJpegBits->resize(mem_size);
    void* pDstPtr = dstJpegBits->get_data_ptr();
    memcpy(pDstPtr, mem, mem_size);

    // After finish_compress, we can close the output file.
    free(mem);

    // Step 7: release JPEG compression object

    // This is an important step since it will release a good deal of memory.
    jpeg_destroy_compress(&cinfo);
    return ENCODE_JPEG_RESULT_OK;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE