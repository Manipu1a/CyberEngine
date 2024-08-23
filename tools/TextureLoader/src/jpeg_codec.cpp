#include "jpeg_codec.h"
#include "jpeglib.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Image)

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

DECODE_JPEG_RESULT decode_jpeg(IDataBlob* srcJpegBits, IDataBlob* dstPixels, ImageDesc& imageDesc)
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
    unsigned char* pSrcPtr = IDataBlob_GetDataPtr(srcJpegBits);
    unsigned long  SrcSize = (unsigned long)IDataBlob_GetSize(pSrcJpegBits);
    jpeg_mem_src(&cinfo, pSrcPtr, SrcSize);

    return DECODE_JPEG_RESULT_OK;
}

ENCODE_JPEG_RESULT encode_jpeg(uint8_t* srcRGBPixels, uint32_t width, uint32_t height, int32_t quality, IDataBlob* dstJpegBits)
{
    return ENCODE_JPEG_RESULT_OK;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE