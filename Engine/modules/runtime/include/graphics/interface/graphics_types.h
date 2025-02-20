#pragma once
#include "graphics/common/cyber_graphics_config.h"
#include "constants.h"
#include <EASTL/vector.h>
#include <stdint.h>

namespace Cyber
{
    #define GRAPHICS_MAX_VERTEX_ATTRIBUTES 15
    #define GRAPHICS_MAX_VERTEX_BINDINGS 15
    #define GRAPHICS_MAX_MRT_COUNT 8u
    #define GRAPHICS_ARRAY_LEN(array) ((sizeof(array)/ sizeof(array[0])))
    

    namespace RenderObject
    {
        class IRenderDevice;
        class ITexture;
        class ITextureView;
        class IBuffer;
        class IFrameBuffer;
        class ISwapChain;
        class ISemaphore;
        class IFence;
        class ICommandPool;
        class ICommandBuffer;
        class IShaderResource;
        class IShaderLibrary;
        class IRootSignature;
        class IRenderPass;
        class IRootSignaturePool;
    }

    typedef uint32_t QueueIndex;
    
    CYBER_TYPED_ENUM(GRAPHICS_BACKEND, uint8_t)
    {
        GRAPHICS_BACKEND_D3D12 = 0,
        GRAPHICS_BACKEND_VULKAN = 1,
        GRAPHICS_BACKEND_METAL = 2,
    };

    CYBER_TYPED_ENUM(VALUE_TYPE, uint8_t)
    {
        VALUE_TYPE_UNDEFINED = 0,
        VALUE_TYPE_INT8,
        VALUE_TYPE_INT16,
        VALUE_TYPE_INT32,
        VALUE_TYPE_UINT8,
        VALUE_TYPE_UINT16,
        VALUE_TYPE_UINT32,
        VALUE_TYPE_FLOAT16,
        VALUE_TYPE_FLOAT32,
        VALUE_TYPE_FLOAT64,
        VALUE_TYPE_COUNT,
    };

    CYBER_TYPED_ENUM(NVAPI_STATUS, int8_t)
    {
        NVAPI_NONE = 0,
        NVAPI_OK = 1,
        NVAPI_ERROR = -1
    };

    CYBER_TYPED_ENUM(AGS_RETURN_CODE, uint8_t)
    {
        AGS_NONE = 0,
        AGS_SUCCESS,                    ///< Successful function call
        AGS_FAILURE,                    ///< Failed to complete call for some unspecified reason
        AGS_INVALID_ARGS,               ///< Invalid arguments into the function
        AGS_OUT_OF_MEMORY,              ///< Out of memory when allocating space internally
        AGS_MISSING_D3D_DLL,            ///< Returned when a D3D dll fails to load
        AGS_LEGACY_DRIVER,              ///< Returned if a feature is not present in the installed driver
        AGS_NO_AMD_DRIVER_INSTALLED,    ///< Returned if the AMD GPU driver does not appear to be installed
        AGS_EXTENSION_NOT_SUPPORTED,    ///< Returned if the driver does not support the requested driver extension
        AGS_ADL_FAILURE,                ///< Failure in ADL (the AMD Display Library)
        AGS_DX_FAILURE,                 ///< Failure from DirectX runtime
    };

    CYBER_TYPED_ENUM(TEXTURE_DIMENSION, uint8_t)
    {
        TEX_DIMENSION_UNDEFINED = 0,
        TEX_DIMENSION_1D,
        TEX_DIMENSION_2D,
        TEX_DIMENSION_2DMS,
        TEX_DIMENSION_3D,
        TEX_DIMENSION_CUBE,
        TEX_DIMENSION_1D_ARRAY,
        TEX_DIMENSION_2D_ARRAY,
        TEX_DIMENSION_2DMS_ARRAY,
        TEX_DIMENSION_CUBE_ARRAY,
        TEX_DIMENSION_COUNT,
    };

    CYBER_TYPED_ENUM(PIPELINE_TYPE, uint8_t)
    {
        PIPELINE_TYPE_NONE = 0,
        PIPELINE_TYPE_COMPUTE,
        PIPELINE_TYPE_GRAPHICS,
        PIPELINE_TYPE_RAYTRACING,
        PIPELINE_TYPE_COUNT,
    };

    CYBER_TYPED_ENUM(QUEUE_TYPE, uint8_t)
    {
        QUEUE_TYPE_GRAPHICS = 0,
        QUEUE_TYPE_COMPUTE,
        QUEUE_TYPE_TRANSFER,
        QUEUE_TYPE_COUNT,
    };

    CYBER_TYPED_ENUM(QUEUE_FLAG, uint8_t)
    {
        QUEUE_FLAG_NONE = 0x0,
        QUEUE_FLAG_DISABLE_GPU_TIMEOUT = 0x1,
        QUEUE_FLAG_INIT_MICROPROFILE = 0x2,
    };

    CYBER_TYPED_ENUM(INDIRECT_ARGUMENT_TYPE, uint8_t)
    {
        INDIRECT_ARG_INVALID,
        INDIRECT_DRAW,
        INDIRECT_DRAW_INDEX,
        INDIRECT_DISPATCH,
        INDIRECT_VERTEX_BUFFER,
        INDIRECT_INDEX_BUFFER,
        INDIRECT_CONSTANT,
        INDIRECT_CONSTANT_BUFFER_VIEW,     // only for dx
        INDIRECT_SHADER_RESOURCE_VIEW,     // only for dx
        INDIRECT_UNORDERED_ACCESS_VIEW,    // only for dx
        INDIRECT_COMMAND_BUFFER,            // metal ICB
        INDIRECT_COMMAND_BUFFER_RESET,      // metal ICB reset
        INDIRECT_COMMAND_BUFFER_OPTIMIZE    // metal ICB optimization
    };
    
    CYBER_TYPED_ENUM(DESCRIPTOR_TYPE, uint32_t)
    {
        DESCRIPTOR_TYPE_UNDEFINED = 0,
        DESCRIPTOR_TYPE_SAMPLER = 0x01,
        // SRV Read only texture
        DESCRIPTOR_TYPE_TEXTURE = (DESCRIPTOR_TYPE_SAMPLER << 1),
        /// UAV Texture
        DESCRIPTOR_TYPE_RW_TEXTURE = (DESCRIPTOR_TYPE_TEXTURE << 1),
        // SRV Read only buffer
        DESCRIPTOR_TYPE_BUFFER = (DESCRIPTOR_TYPE_RW_TEXTURE << 1),
        DESCRIPTOR_TYPE_BUFFER_RAW = (DESCRIPTOR_TYPE_BUFFER | (DESCRIPTOR_TYPE_BUFFER << 1)),
        /// UAV Buffer
        DESCRIPTOR_TYPE_RW_BUFFER = (DESCRIPTOR_TYPE_BUFFER << 2),
        DESCRIPTOR_TYPE_RW_BUFFER_RAW = (DESCRIPTOR_TYPE_RW_BUFFER | (DESCRIPTOR_TYPE_RW_BUFFER << 1)),
        /// Uniform buffer
        DESCRIPTOR_TYPE_UNIFORM_BUFFER = (DESCRIPTOR_TYPE_RW_BUFFER << 2),
        /// Push constant / Root constant
        DESCRIPTOR_TYPE_ROOT_CONSTANT = (DESCRIPTOR_TYPE_UNIFORM_BUFFER << 1),
        /// IA
        DESCRIPTOR_TYPE_VERTEX_BUFFER = (DESCRIPTOR_TYPE_ROOT_CONSTANT << 1),
        DESCRIPTOR_TYPE_INDEX_BUFFER = (DESCRIPTOR_TYPE_VERTEX_BUFFER << 1),
        DESCRIPTOR_TYPE_INDIRECT_BUFFER = (DESCRIPTOR_TYPE_INDEX_BUFFER << 1),
        /// Cubemap SRV
        DESCRIPTOR_TYPE_TEXTURE_CUBE = (DESCRIPTOR_TYPE_TEXTURE | (DESCRIPTOR_TYPE_INDIRECT_BUFFER << 1)),
        /// RTV / DSV per mip slice
        DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES = (DESCRIPTOR_TYPE_INDIRECT_BUFFER << 2),
        /// RTV / DSV per array slice
        DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES = (DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES << 1),
        /// RTV / DSV per depth slice
        DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES = (DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
        DESCRIPTOR_TYPE_RAY_TRACING = (DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
        DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER = (DESCRIPTOR_TYPE_RAY_TRACING << 1),
    #if defined(VULKAN)
        /// Subpass input (descriptor type only available in Vulkan)
        DESCRIPTOR_TYPE_INPUT_ATTACHMENT = (DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER << 1),
        DESCRIPTOR_TYPE_TEXEL_BUFFER = (DESCRIPTOR_TYPE_INPUT_ATTACHMENT << 1),
        DESCRIPTOR_TYPE_RW_TEXEL_BUFFER = (DESCRIPTOR_TYPE_TEXEL_BUFFER << 1),
        DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = (DESCRIPTOR_TYPE_RW_TEXEL_BUFFER << 1),
        
        /// Khronos extension ray tracing
        DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE = (DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER << 1),
        DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_BUILD_INPUT = (DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE << 1),
        DESCRIPTOR_TYPE_SHADER_DEVICE_ADDRESS = (DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_BUILD_INPUT << 1),
        DESCRIPTOR_TYPE_SHADER_BINDING_TABLE = (DESCRIPTOR_TYPE_SHADER_DEVICE_ADDRESS << 1),
    #endif
    };

    CYBER_TYPED_ENUM(TEXTURE_CREATE_FLAG, uint32_t)
    {
        /// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
        TCF_NONE = 0,
        /// Texture will allocate its own memory (COMMITTED resource)
        TCF_OWN_MEMORY_BIT = 0x01,
        /// Texture will be allocated in memory which can be shared among multiple processes
        TCF_EXPORT_BIT = 0x02,
        /// Texture will be allocated in memory which can be shared among multiple gpus
        TCF_EXPORT_ADAPTER_BIT = 0x04,
        /// Use on-tile memory to store this texture
        TCF_ON_TILE = 0x08,
        /// Prevent compression meta data from generating (XBox)
        TCF_NO_COMPRESSION = 0x10,
        /// Force 2D instead of automatically determining dimension based on width, height, depth
        TCF_FORCE_2D = 0x20,
        /// Force 3D instead of automatically determining dimension based on width, height, depth
        TCF_FORCE_3D = 0x40,
        /// Display target
        TCF_FORCE_ALLOW_DISPLAY_TARGET = 0x80,
        /// Create an sRGB texture.
        TCF_SRGB = 0x100,
        /// Create a normal map texture
        TCF_NORMAL_MAP = 0x200,
        /// Fragment mask
        TCF_FRAG_MASK = 0x400,
        TCF_USABLE_MAX = 0x40000,
        TCF_MAX_ENUM_BIT = 0x7FFFFFFF
    };

    CYBER_TYPED_ENUM(TEXTURE_SAMPLE_COUNT, uint8_t)
    {
        SAMPLE_COUNT_1 = 1,
        SAMPLE_COUNT_2 = 2,
        SAMPLE_COUNT_4 = 4,
        SAMPLE_COUNT_8 = 8,
        SAMPLE_COUNT_16 = 16,
        SAMPLE_COUNT_COUNT = 5
    };

    CYBER_TYPED_ENUM(TEXTURE_FORMAT, uint32_t)
    {
        // Unknown format
        TEX_FORMAT_UNKNOWN = 0,
        
        // Four-component, 128-bit typeless format with 32-bit channels.
        TEX_FORMAT_RGBA32_TYPELESS,

        // Four-component, 128-bit floating-point format with 32-bit channels.
        TEX_FORMAT_RGBA32_FLOAT,

        // Four-component, 128-bit unsigned-integer format with 32-bit channels.
        TEX_FORMAT_RGBA32_UINT,

        // Four-component, 128-bit signed-integer format with 32-bit channels.
        TEX_FORMAT_RGBA32_SINT,

        // Three-component, 96-bit typeless format with 32-bit channels.
        TEX_FORMAT_RGB32_TYPELESS,
        
        // Three-component, 96-bit floating-point format with 32-bit channels.
        TEX_FORMAT_RGB32_FLOAT,

        // Three-component, 96-bit unsigned-integer format with 32-bit channels.
        TEX_FORMAT_RGB32_UINT,

        // Three-component, 96-bit signed-integer format with 32-bit channels.
        TEX_FORMAT_RGB32_SINT,

        // Four-component, 64-bit typeless format with 16-bit channels.
        TEX_FORMAT_RGBA16_TYPELESS,

        // Four-component, 64-bit floating-point format with 16-bit channels.
        TEX_FORMAT_RGBA16_FLOAT,

        // Four-component, 64-bit unsigned-normalized-integer format with 16-bit channels.
        TEX_FORMAT_RGBA16_UNORM,

        // Four-component, 64-bit unsigned-integer format with 16-bit channels.
        TEX_FORMAT_RGBA16_UINT,

        // Four-component, 64-bit signed-normalized-integer format with 16-bit channels.
        TEX_FORMAT_RGBA16_SNORM,

        // Four-component, 64-bit signed-integer format with 16-bit channels.
        TEX_FORMAT_RGBA16_SINT,

        // Two-component, 64-bit typeless format with 32-bit channels.
        TEX_FORMAT_RG32_TYPELESS,
        
        // Two-component, 64-bit floating-point format with 32-bit channels.
        TEX_FORMAT_RG32_FLOAT,

        // Two-component, 64-bit unsigned-integer format with 32-bit channels.
        TEX_FORMAT_RG32_UINT,

        // Two-component, 64-bit signed-integer format with 32-bit channels.
        TEX_FORMAT_RG32_SINT,

        // Two-component, 64-bit typeless format with 32-bits for R channel and 8-bits for G channel.
        TEX_FORMAT_R32G8X24_TYPELESS,

        // Two-component, 64-bit format with 32-bit floating-point depth channel and 8-bit integer stencil channel.
        TEX_FORMAT_D32_FLOAT_S8X24_UINT,

        // Two-component, 64-bit format with 32-bit floating-point R channel and 8+24-bits of typeless data.
        TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS,

        // Two-component, 64-bit format with 32-bit typeless data and 8-bit G channel
        TEX_FORMAT_X32_TYPELESS_G8X24_UINT,

        // Four-component, 32-bit typeless format with 10 bits for RGB and 2 bits for alpha channel.
        TEX_FORMAT_RGB10A2_TYPELESS,

        // Four-component, 32-bit unsigned-normalized-integer format with 10 bits for RGB and 2 bits for alpha channel.
        TEX_FORMAT_RGB10A2_UNORM,

        // Four-component, 32-bit unsigned-integer format with 10 bits for RGB and 2 bits for alpha channel.
        TEX_FORMAT_RGB10A2_UINT,

        // Three-component, 32-bit format encoding three partial precision channels using 11 bits for R and G and 10 bits for B channel.
        TEX_FORMAT_R11G11B10_FLOAT,

        // Three-component, 32-bit typeless format with 8-bit channels.
        TEX_FORMAT_RGBA8_TYPELESS,

        // Three-component, 32-bit unsigned-normalized-integer format with 8-bit channels.
        TEX_FORMAT_RGBA8_UNORM,

        // Three-component, 32-bit unsigned-normalized-integer format with 8-bit sRGB gamma corrected channels.
        TEX_FORMAT_RGBA8_UNORM_SRGB,

        // Three-component, 32-bit unsigned-integer format with 8-bit channels.
        TEX_FORMAT_RGBA8_UINT,

        // Three-component, 32-bit signed-normalized-integer format with 8-bit channels.
        TEX_FORMAT_RGBA8_SNORM,

        // Three-component, 32-bit signed-integer format with 8-bit channels.
        TEX_FORMAT_RGBA8_SINT,

        // Two-component, 32-bit typeless format with 16-bit channels.
        TEX_FORMAT_RG16_TYPELESS,

        // Two-component, 32-bit half-precision floating-point format with 16-bit channels.
        TEX_FORMAT_RG16_FLOAT,

        // Two-component, 32-bit unsigned-normalized-integer format with 16-bit channels.
        TEX_FORMAT_RG16_UNORM,

        // Two-component, 32-bit unsigned-integer format with 16-bit channels.
        TEX_FORMAT_RG16_UINT,

        // Two-component, 32-bit signed-normalized-integer format with 16-bit channels.
        TEX_FORMAT_RG16_SNORM,

        // Two-component, 32-bit signed-integer format with 16-bit channels.
        TEX_FORMAT_RG16_SINT,

        // Single-component, 32-bit typeless format with 32-bit channel.
        TEX_FORMAT_R32_TYPELESS,

        // Single-component, 32-bit floating-point depth format
        TEX_FORMAT_D32_FLOAT,

        // Single-component, 32-bit floating-point format with 32-bit channel.
        TEX_FORMAT_R32_FLOAT,

        // Single-component, 32-bit unsigned-integer format with 32-bit channel.
        TEX_FORMAT_R32_UINT,

        // Single-component, 32-bit signed-integer format with 32-bit channel.
        TEX_FORMAT_R32_SINT,

        // Two-component, 32-bit typeless format with 16-bit channel for R and 16-bit channel for G.
        TEX_FORMAT_R24G8_TYPELESS,

        // Two-component, 32-bit format with 24-bit unsigned-normalized-integer depth channel and 8-bit unsigned-integer stencil channel.
        TEX_FORMAT_D24_UNORM_S8_UINT,

        // Two-component, 32-bit format with 24-bit unsigned-normalized-integer depth channel and 8-bit unreferenced data.
        TEX_FORMAT_R24_UNORM_X8_TYPELESS,

        // Two-component, 32-bit format with 24-bit unreferenced data and 8-bit unsigned-integer data.
        TEX_FORMAT_X24_TYPELESS_G8_UINT,

        // Two-component, 16-bit typeless format with 8-bit channel for R and 8-bit channel for G.
        TEX_FORMAT_RG8_TYPELESS,

        // Two-component, 16-bit unsigned-normalized-integer format with 8-bit channels.
        TEX_FORMAT_RG8_UNORM,

        // Two-component, 16-bit unsigned-integer format with 8-bit channels.
        TEX_FORMAT_RG8_UINT,

        // Two-component, 16-bit signed-normalized-integer format with 8-bit channels.
        TEX_FORMAT_RG8_SNORM,

        // Two-component, 16-bit signed-integer format with 8-bit channels.
        TEX_FORMAT_RG8_SINT,

        // Single-component, 16-bit typeless format with 16-bit channel.
        TEX_FORMAT_R16_TYPELESS,

        // Single-component, 16-bit half-precision floating-point format.
        TEX_FORMAT_R16_FLOAT,

        // Single-component, 16-bit unsigned-normalized-integer depth format.
        TEX_FORMAT_D16_UNORM,

        // Single-component, 16-bit unsigned-normalized-integer format.
        TEX_FORMAT_R16_UNORM,

        // Single-component, 16-bit unsigned-integer format.
        TEX_FORMAT_R16_UINT,

        // Single-component, 16-bit signed-normalized-integer format.
        TEX_FORMAT_R16_SNORM,

        // Single-component, 16-bit signed-integer format.
        TEX_FORMAT_R16_SINT,

        // Single-component, 8-bit typeless format.
        TEX_FORMAT_R8_TYPELESS,

        // Single-component, 8-bit unsigned-normalized-integer format.
        TEX_FORMAT_R8_UNORM,

        // Single-component, 8-bit unsigned-integer format.
        TEX_FORMAT_R8_UINT,

        // Single-component, 8-bit signed-normalized-integer format.
        TEX_FORMAT_R8_SNORM,

        // Single-component, 8-bit signed-integer format.
        TEX_FORMAT_R8_SINT,

        // Single-component, 8-bit unsigned-normalized-integer format for alpha only.
        TEX_FORMAT_A8_UNORM,

        // Single-component, 1-bit format.
        TEX_FORMAT_R1_UNORM,

        // Three partial-precision floating pointer numbers sharing single exponent encoded into a 32-bit value.
        TEX_FORMAT_RGB9E5_SHAREDEXP,

        // Four-component, unsigned-normalized integer format analogous to UYVY encoding.
        TEX_FORMAT_RG8_B8G8_UNORM,

        // Four-component, unsigned-normalized integer format analogous to YUY2 encoding.
        TEX_FORMAT_G8R8_G8B8_UNORM,

        // Four-component typeless block-compression format with 1:8 compression ratio.
        TEX_FORMAT_BC1_TYPELESS,

        // Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits for G, 5 bits for B, and 0 or 1 bit for A channel.
        // The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format.
        TEX_FORMAT_BC1_UNORM,

        // Four-component unsigned-normalized-integer block-compression sRGB format with 5 bits for R, 6 bits for G, 5 bits for B, and 0 or 1 bit for A channel.
        // The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format.
        TEX_FORMAT_BC1_UNORM_SRGB,

        // Four component typeless block-compression format with 1:4 compression ratio.\n
        // D3D counterpart: DXGI_FORMAT_BC2_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT is used.
        TEX_FORMAT_BC2_TYPELESS,

        // Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits for G, 5 bits for B, and 0 or 1 bit for A channel.
        // The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format
        TEX_FORMAT_BC2_UNORM,
        
        // Four-component unsigned-normalized-integer block-compression sRGB format with 5 bits for R, 6 bits for G, 5 bits for B, and 0 or 1 bit for A channel. \n
        // The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format
        TEX_FORMAT_BC2_UNORM_SRGB,

        // Four component typeless block-compression format with 1:4 compression ratio.\n
        // D3D counterpart: DXGI_FORMAT_BC2_TYPELESS. OpenGL does not have direct counterpart, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT is used.
        TEX_FORMAT_BC3_TYPELESS,

        // Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits for G, 5 bits for B, and 8 bits for highly-coherent A channel.
        // The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8 format.
        TEX_FORMAT_BC3_UNORM,

        // Four-component unsigned-normalized-integer block-compression sRGB format with 5 bits for R, 6 bits for G, 5 bits for B, and 8 bits for highly-coherent A channel.
        // The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8 format. 
        TEX_FORMAT_BC3_UNORM_SRGB,

        // One-component typeless block-compression format with 1:2 compression ratio.
        TEX_FORMAT_BC4_TYPELESS,

        // One-component unsigned-normalized-integer block-compression format with 8 bits for R channel.
        // The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:2 compression ratio against R8 format.
        TEX_FORMAT_BC4_UNORM,

        // One-component signed-normalized-integer block-compression format with 8 bits for R channel.
        // The pixel data is encoded using 8 bytes per 4x4 block (4 bits per pixel) providing 1:2 compression ratio against R8 format.
        TEX_FORMAT_BC4_SNORM,

        // Two-component typeless block-compression format with 1:2 compression ratio.
        TEX_FORMAT_BC5_TYPELESS,

        // Two-component unsigned-normalized-integer block-compression format with 8 bits for R and 8 bits for G channel.
        // The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:2 compression ratio against RG8 format.
        TEX_FORMAT_BC5_UNORM,

        // Two-component signed-normalized-integer block-compression format with 8 bits for R and 8 bits for G channel.
        // The pixel data is encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:2 compression ratio against RG8 format.
        TEX_FORMAT_BC5_SNORM,

        // Three-component 16-bit unsigned-normalized-integer format with 5 bits for blue, 6 bits for green, and 5 bits for red channel.
        TEX_FORMAT_B5G6R5_UNORM,

        // Four-component 16-bit unsigned-normalized-integer format with 5 bits for each color channel and 1-bit alpha.
        TEX_FORMAT_B5G5R5A1_UNORM,

        // Four-component 32-bit unsigned-normalized-integer format with 8 bits for each channel.
        TEX_FORMAT_BGRA8_UNORM,

        // Four-component 32-bit unsigned-normalized-integer format with 8 bits for each color channel and 8 bits unused.
        TEX_FORMAT_BGRX8_UNORM,

        // Four-component 32-bit 2.8-biased fixed-point format with 10 bits for each color channel and 2-bit alpha.
        TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,

        // Four-component 32-bit typeless format with 8 bits for each channel.
        TEX_FORMAT_BGRA8_TYPELESS,

        // Four-component 32-bit unsigned-normalized sRGB format with 8 bits for each channel.
        TEX_FORMAT_BGRA8_UNORM_SRGB,

        // Four-component 32-bit typeless format that with 8 bits for each color channel, and 8 bits are unused.
        TEX_FORMAT_BGRX8_TYPELESS,

        // Four-component 32-bit unsigned-normalized sRGB format with 8 bits for each color channel, and 8 bits are unused.
        TEX_FORMAT_BGRX8_UNORM_SRGB,
        
        // Four-component 16-bit unsigned-normalized-integet that supports 4 bits for each channel including alpha.
        TEX_FORMAT_BGRA4_UNORM, 

        // Three-component typeless block-compression format.
        TEX_FORMAT_BC6H_TYPELESS,

        // Three-component unsigned half-precision floating-point format with 16 bits for each channel.
        TEX_FORMAT_BC6H_UF16,

        // Three-channel signed half-precision floating-point format with 16 bits per each channel.
        TEX_FORMAT_BC6H_SF16,

        // Three-component typeless block-compression format.
        TEX_FORMAT_BC7_TYPELESS,

        // Three-component block-compression unsigned-normalized-integer format with 4 to 7 bits per color channel and 0 to 8 bits of alpha.
        TEX_FORMAT_BC7_UNORM,

        // Three-component block-compression unsigned-normalized-integer sRGB format with 4 to 7 bits per color channel and 0 to 8 bits of alpha.
        TEX_FORMAT_BC7_UNORM_SRGB,

        TEX_FORMAT_NUM_FORMATS
    };  

    static CYBER_FORCE_INLINE uint32_t FormatUtil_BitSizeOfBlock(TEXTURE_FORMAT fmt)
    {
        switch (fmt) 
        {
            case TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN: return 0;
            case TEXTURE_FORMAT::TEX_FORMAT_A8_UNORM: return 8;
            case TEXTURE_FORMAT::TEX_FORMAT_R8_UNORM: return 8;
            case TEXTURE_FORMAT::TEX_FORMAT_R8_SNORM: return 8;
            case TEXTURE_FORMAT::TEX_FORMAT_R8_UINT: return 8;
            case TEXTURE_FORMAT::TEX_FORMAT_R8_SINT: return 8;
            case TEXTURE_FORMAT::TEX_FORMAT_B5G6R5_UNORM: return 16;
            case TEXTURE_FORMAT::TEX_FORMAT_B5G5R5A1_UNORM: return 16;
            case TEXTURE_FORMAT::TEX_FORMAT_R16_UNORM: return 16;
            case TEXTURE_FORMAT::TEX_FORMAT_R16_SNORM: return 16;
            case TEXTURE_FORMAT::TEX_FORMAT_R16_UINT: return 16;
            case TEXTURE_FORMAT::TEX_FORMAT_R16_SINT: return 16;
            case TEXTURE_FORMAT::TEX_FORMAT_R32_UINT: return 32;
            case TEXTURE_FORMAT::TEX_FORMAT_R32_SINT: return 32;

            default: return 32;
        }
    }

    static CYBER_FORCE_INLINE bool FormatUtil_IsDepthStencilFormat(TEXTURE_FORMAT const fmt)
    {
        switch (fmt)
        {
            case TEX_FORMAT_D32_FLOAT:
            case TEX_FORMAT_D24_UNORM_S8_UINT:
            case TEX_FORMAT_D32_FLOAT_S8X24_UINT:
            case TEX_FORMAT_D16_UNORM:
                return true;
            default:
                return false;
        }
    }

    CYBER_TYPED_ENUM(TEXTURE_CHANNET_BIT, uint32_t)
    {
        TEXTURE_CHANNEL_INVALID = 0,
        TEXTURE_CHANNEL_R = 0x1,
        TEXTURE_CHANNEL_G = 0x2,
        TEXTURE_CHANNEL_B = 0x4,
        TEXTURE_CHANNEL_A = 0x8,
        TEXTURE_CHANNEL_RG = TEXTURE_CHANNEL_R | TEXTURE_CHANNEL_G,
        TEXTURE_CHANNEL_RGB = TEXTURE_CHANNEL_R | TEXTURE_CHANNEL_G | TEXTURE_CHANNEL_B,
        TEXTURE_CHANNEL_RGBA = TEXTURE_CHANNEL_R | TEXTURE_CHANNEL_G | TEXTURE_CHANNEL_B | TEXTURE_CHANNEL_A,
    };

    // Describes texture format component type
    CYBER_TYPED_ENUM(COMPONENT_TYPE, uint8_t)
    {
        // Undefined component type
        COMPONENT_TYPE_UNDEFINED = 0,
        // Floating point component type
        COMPONENT_TYPE_FLOAT,
        // Signed-normalized-integer component type
        COMPONENT_TYPE_SNORM,
        // Unsigned-normalized-integer component type
        COMPONENT_TYPE_UNORM,
        // Unsigned-normalized-integer component type with sRGB gamma correction
        COMPONENT_TYPE_UNORM_SRGB,
        // Signed integer component type
        COMPONENT_TYPE_SINT,
        // Unsigned integer component type
        COMPONENT_TYPE_UINT,
        // Depth component type
        COMPONENT_TYPE_DEPTH,
        // Stencil component type
        COMPONENT_TYPE_DEPTH_STENCIL,
        // Compound component type (example texture formats: TEXTURE_FORMAT_R11G11B10_FLOAT or TEX_FORMAT_RGB9E5_SHAREDEXP)
        COMPONENT_TYPE_COMPOUND,

        COMPONENT_TYPE_COMPRESSED,
    };

    CYBER_TYPED_ENUM(SLOT_MASK_BIT, uint16_t)
    {
        SLOT_MASK_0 = 0x1,
        SLOT_MASK_1 = 0x2,
        SLOT_MASK_2 = 0x4,
        SLOT_MASK_3 = 0x8,
        SLOT_MASK_4 = 0x10,
        SLOT_MASK_5 = 0x20,
        SLOT_MASK_6 = 0x40,
        SLOT_MASK_7 = 0x80,
    };

    CYBER_TYPED_ENUM(GRAPHICS_RESOURCE_STATE, uint32_t)
    {
        // Unknown resource state
        GRAPHICS_RESOURCE_STATE_UNKNOWN = 0,
        // Undefined resource state
        GRAPHICS_RESOURCE_STATE_UNDEFINED = 1U << 0U,
        // Resource is used as a vertex buffer
        GRAPHICS_RESOURCE_STATE_VERTEX_BUFFER = 1U << 1U,
        // Resource is used as a constant buffer
        GRAPHICS_RESOURCE_STATE_CONSTANT_BUFFER = 1U << 2U,
        // Resource is used as an index buffer
        GRAPHICS_RESOURCE_STATE_INDEX_BUFFER = 1U << 3U,
        // Resource is used as a render target
        GRAPHICS_RESOURCE_STATE_RENDER_TARGET = 1U << 4U,
        // Resource is used for unordered access
        GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS = 1U << 5U,
        // Resource is used for depth write operations
        GRAPHICS_RESOURCE_STATE_DEPTH_WRITE = 1U << 6U,
        // Resource is used for depth read operations
        GRAPHICS_RESOURCE_STATE_DEPTH_READ = 1U << 7U,
        // Resource is used as a shader resource
        GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE = 1U << 8U,
        // Resource is used for stream output
        GRAPHICS_RESOURCE_STATE_STREAM_OUT = 1U << 9U,
        // Resource is used as an indirect argument buffer
        GRAPHICS_RESOURCE_STATE_INDIRECT_ARGUMENT = 1U << 10U,
        // Resource is used as a copy destination
        GRAPHICS_RESOURCE_STATE_COPY_DEST = 1U << 11U,
        // Resource is used as a copy source
        GRAPHICS_RESOURCE_STATE_COPY_SOURCE = 1U << 12U,
        // Resource is used as a resolve destination
        GRAPHICS_RESOURCE_STATE_RESOLVE_DEST = 1U << 13U,
        // Resource is used as a resolve source
        GRAPHICS_RESOURCE_STATE_RESOLVE_SOURCE = 1U << 14U,
        // Resource is used as an input attachment
        GRAPHICS_RESOURCE_STATE_INPUT_ATTACHMENT = 1U << 15U,
        // Resource is in present state
        GRAPHICS_RESOURCE_STATE_PRESENT = 1U << 16U,
        // Resource is used for acceleration structure read
        GRAPHICS_RESOURCE_STATE_BUILD_AS_READ = 1U << 17U,
        // Resource is used for acceleration structure write
        GRAPHICS_RESOURCE_STATE_BUILD_AS_WRITE = 1U << 18U,
        // Resource is used for ray tracing
        GRAPHICS_RESOURCE_STATE_RAY_TRACING = 1U << 19U,
        // Resource is in common state
        GRAPHICS_RESOURCE_STATE_COMMON = 1U << 20U,
        // Resource is used for variable rate shading
        GRAPHICS_RESOURCE_STATE_SHADING_RATE = 1U << 21U,
        // Maximum state bit value
        GRAPHICS_RESOURCE_STATE_MAX_BIT = GRAPHICS_RESOURCE_STATE_SHADING_RATE,
        // Combined state for generic read operations
        GRAPHICS_RESOURCE_STATE_GENERIC_READ = GRAPHICS_RESOURCE_STATE_VERTEX_BUFFER | 
                                                GRAPHICS_RESOURCE_STATE_CONSTANT_BUFFER | 
                                                GRAPHICS_RESOURCE_STATE_INDEX_BUFFER | 
                                                GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE | 
                                                GRAPHICS_RESOURCE_STATE_INDIRECT_ARGUMENT | 
                                                GRAPHICS_RESOURCE_STATE_COPY_SOURCE,

    };
    
    DEFINE_FLAG_ENUM_OPERATORS(GRAPHICS_RESOURCE_STATE);

    CYBER_TYPED_ENUM(GRAPHICS_RESOURCE_MEMORY_USAGE, uint32_t)
    {
        /// No intended memory usage specified.
        GRAPHICS_RESOURCE_MEMORY_USAGE_UNKNOWN = 0,
        /// Memory will be used on device only, no need to be mapped on host.
        GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_ONLY = 1,
        /// Memory will be mapped on host. Could be used for transfer to device.
        GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_ONLY = 2,
        /// Memory will be used for frequent (dynamic) updates from host and reads on device.
        GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_TO_GPU = 3,
        /// Memory will be used for writing on device and readback on host.
        GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_TO_CPU = 4,

    };
    
    /// Specifies how a resource will be used and where it should be allocated
    CYBER_TYPED_ENUM(GRAPHICS_RESOURCE_USAGE, uint32_t)
    {
        GRAPHICS_RESOURCE_USAGE_UNIFIED = 0,
        /// A resource that can only be read on the GPU, cannot be written by the GPU.
        /// and cannot be accessed at all by the CPU. This type of resource must be 
        /// initialized when it is created, since it cannot be altered after creation.
        GRAPHICS_RESOURCE_USAGE_IMMUTABLE,
        /// A resource that can be read by the GPU and written to by the GPU and can also
        /// be accasionally written by the CPU.
        GRAPHICS_RESOURCE_USAGE_DEFAULT,
        /// A resource that can be read by the GPU and written at least once per frame by the CPU.
        GRAPHICS_RESOURCE_USAGE_DYNAMIC,
        /// A resource that supports data transfer (copy) from the CPU to the GPU.
        GRAPHICS_RESOURCE_USAGE_STAGING,
        /// A resource that can be partially committed to physical memory.
        GRAPHICS_RESOURCE_USAGE_SPARSE,
    };

    CYBER_TYPED_ENUM(GRAPHICS_RESOURCE_BIND_FLAGS, uint32_t)
    {
        // Undefined binding.
        GRAPHICS_RESOURCE_BIND_NONE = 0,
        // A buffer can be bound as a vertex buffer.
        GRAPHICS_RESOURCE_BIND_VERTEX_BUFFER = 1u << 0u,
        // A buffer can be bound as an index buffer.
        GRAPHICS_RESOURCE_BIND_INDEX_BUFFER = 1u << 1u,
        // A buffer can be bound as a uniform buffer.
        GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER = 1u << 2u,
        // A buffer or a texture can be bound as a shader resource.
        GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE = 1u << 3u,
        // A buffer can be bound as a target for stream output stage.
        GRAPHICS_RESOURCE_BIND_STREAM_OUTPUT = 1u << 4u,
        // A texture can be bound as a render target.
        GRAPHICS_RESOURCE_BIND_RENDER_TARGET = 1u << 5u,
        // A texture can be bound as a depth-stencil target.
        GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL = 1u << 6u,
        // A buffer or a texture can be bound as an unordered access view.
        GRAPHICS_RESOURCE_BIND_UNORDERED_ACCESS = 1u << 7u,
        // A buffer can be bound as the source buffer for indirect draw commands.
        GRAPHICS_RESOURCE_BIND_DRAW_ARGS = 1u << 8u,
        // A texture can be used as render pass input attachment.
        GRAPHICS_RESOURCE_BIND_INPUT_ATTACHMENT = 1u << 9u,
        // A buffer can be used as a scratch buffer or as the source of primitive data
        // for acceleration structures building.
        GRAPHICS_RESOURCE_BIND_RAY_TRACING = 1u << 10u,
        // A texture can be used as a shading rate source.
        GRAPHICS_RESOURCE_BIND_SHADING_RATE = 1u << 11u,
        
        GRAPHICS_RESOURCE_BIND_LAST = GRAPHICS_RESOURCE_BIND_SHADING_RATE
    };
    CYBER_TYPED_ENUM(CPU_ACCESS_FLAGS, uint8_t)
    {
        CPU_ACCESS_NONE = 0, ///< No CPU access
        CPU_ACCESS_READ = 0x01, ///< A resource can be mapped for reading
        CPU_ACCESS_WRITE = 0x02, ///< A resource can be mapped for writing
    };

    CYBER_TYPED_ENUM(BUFFER_CREATION_FLAG, uint32_t)
    {
        /// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
        BCF_NONE = 0x01,
        /// Buffer will allocate its own memory (COMMITTED resource)
        BCF_OWN_MEMORY_BIT = 0x02,
        /// Buffer will be persistently mapped
        BCF_PERSISTENT_MAP_BIT = 0x04,
        /// Use ESRAM to store this buffer
        BCF_ESRAM = 0x08,
        /// Flag to specify not to allocate descriptors for the resource
        BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,
    #ifdef VULKAN
        /* Memory Host Flags */
        BCF_HOST_VISIBLE = 0x100,
        BCF_HOST_COHERENT = 0x200,
    #endif
    };

    CYBER_TYPED_ENUM(SCANLINE_ORDER, uint8_t)
    {
        SCANLINE_ORDER_UNSPECIFIED = 0,
        SCANLINE_ORDER_PROGRESSIVE = 1,
        SCANLINE_ORDER_UPPER_FIELD_FIRST = 2,
        SCANLINE_ORDER_LOWER_FIELD_FIRST = 3,
    };

    CYBER_TYPED_ENUM(FENCE_STATUS, uint8_t)
    {
        FENCE_STATUS_COMPLETE = 0,
        FENCE_STATUS_INCOMPLETE,
        FENCE_STATUS_NOTSUBMITTED,
    };

    CYBER_TYPED_ENUM(QUERY_TYPE, uint8_t)
    {
        QUERY_TYPE_TIMESTAMP = 0,
        QUERY_TYPE_PIPELINE_STATISTICS,
        QUERY_TYPE_OCCLUSION,
        QUERY_TYPE_COUNT,
    };

    CYBER_TYPED_ENUM(GRAPHICS_RESOURCE_TYPE, uint32_t)
    {
        GRAPHICS_RESOURCE_TYPE_NONE = 0,
        GRAPHICS_RESOURCE_TYPE_SAMPLER = 0x00000001,
        /// SRV Read only texture
        GRAPHICS_RESOURCE_TYPE_TEXTURE = (GRAPHICS_RESOURCE_TYPE_SAMPLER << 1),
        /// RTV Texture
        GRAPHICS_RESOURCE_TYPE_RENDER_TARGET = (GRAPHICS_RESOURCE_TYPE_TEXTURE << 1),
        /// DSV Texture
        GRAPHICS_RESOURCE_TYPE_DEPTH_STENCIL = (GRAPHICS_RESOURCE_TYPE_RENDER_TARGET << 1),
        /// UAV Texture
        GRAPHICS_RESOURCE_TYPE_RW_TEXTURE = (GRAPHICS_RESOURCE_TYPE_DEPTH_STENCIL << 1),
        /// SRV Read only buffer
        GRAPHICS_RESOURCE_TYPE_BUFFER = (GRAPHICS_RESOURCE_TYPE_RW_TEXTURE << 1),
        GRAPHICS_RESOURCE_TYPE_BUFFER_RAW = (GRAPHICS_RESOURCE_TYPE_BUFFER | (GRAPHICS_RESOURCE_TYPE_BUFFER << 1)),
        /// UAV Buffer
        GRAPHICS_RESOURCE_TYPE_RW_BUFFER = (GRAPHICS_RESOURCE_TYPE_BUFFER << 2),
        GRAPHICS_RESOURCE_TYPE_RW_BUFFER_RAW = (GRAPHICS_RESOURCE_TYPE_RW_BUFFER | (GRAPHICS_RESOURCE_TYPE_RW_BUFFER << 1)),
        /// CBV Uniform buffer
        GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER = (GRAPHICS_RESOURCE_TYPE_RW_BUFFER << 2),
        /// Push constant / Root constant
        GRAPHICS_RESOURCE_TYPE_PUSH_CONTANT = (GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER << 1),
        /// IA
        GRAPHICS_RESOURCE_TYPE_VERTEX_BUFFER = (GRAPHICS_RESOURCE_TYPE_PUSH_CONTANT << 1),
        GRAPHICS_RESOURCE_TYPE_INDEX_BUFFER = (GRAPHICS_RESOURCE_TYPE_VERTEX_BUFFER << 1),
        GRAPHICS_RESOURCE_TYPE_INDIRECT_BUFFER = (GRAPHICS_RESOURCE_TYPE_INDEX_BUFFER << 1),
        /// Cubemap SRV
        GRAPHICS_RESOURCE_TYPE_TEXTURE_CUBE = (GRAPHICS_RESOURCE_TYPE_INDIRECT_BUFFER << 1),
        /// RTV / DSV per mip slice
        GRAPHICS_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES = (GRAPHICS_RESOURCE_TYPE_TEXTURE_CUBE << 1),
        /// RTV / DSV per array slice
        GRAPHICS_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES = (GRAPHICS_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES << 1),
        /// RTV / DSV per depth slice
        GRAPHICS_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES = (GRAPHICS_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
        GRAPHICS_RESOURCE_TYPE_RAY_TRACING = (GRAPHICS_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
    #if defined (GRAPHICS_USE_VULKAN)
        /// Subpass input (descriptor type only available in Vulkan)
        RESOURCE_TYPE_INPUT_ATTACHMENT = (RESOURCE_TYPE_RAY_TRACING << 1),
        RESOURCE_TYPE_TEXEL_BUFFER = (RESOURCE_TYPE_INPUT_ATTACHMENT << 1),
        RESOURCE_TYPE_RW_TEXEL_BUFFER = (RESOURCE_TYPE_TEXEL_BUFFER << 1),
        RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = (RESOURCE_TYPE_RW_TEXEL_BUFFER << 1),
    #endif
    };

    DEFINE_FLAG_ENUM_OPERATORS(GRAPHICS_RESOURCE_TYPE);
    
    CYBER_TYPED_ENUM(TEXTURE_VIEW_USAGE, uint8_t)
    {
        TVU_SRV = 0x01,
        TVU_RTV_DSV = 0x02,
        TVU_UAV = 0x04,
    };

    CYBER_TYPED_ENUM(TEXTURE_VIEW_ASPECT, uint8_t)
    {
        TVA_COLOR = 0x01,
        TVA_DEPTH = 0x02,
        TVA_STENCIL = 0x04,
    };

    // Same value as Vulkan Enumeration Bits.
    CYBER_TYPED_ENUM(SHADER_STAGE, uint32_t)
    {
        SHADER_STAGE_NONE = 0,
        SHADER_STAGE_VERT = 0X00000001,
        SHADER_STAGE_TESC = 0X00000002,
        SHADER_STAGE_TESE = 0X00000004,
        SHADER_STAGE_GEOM = 0X00000008,
        SHADER_STAGE_FRAG = 0X00000010,
        SHADER_STAGE_COMPUTE = 0X00000020,
        SHADER_STAGE_AMPLIFICATION = 0X00000040,
        SHADER_STAGE_MESH = 0X00000080,
        SHADER_STAGE_RAYTRACING = 0X00000100,

        SHADER_STAGE_ALL_GRAPHCIS = (uint32_t)SHADER_STAGE_VERT | (uint32_t)SHADER_STAGE_TESC | (uint32_t)SHADER_STAGE_GEOM | (uint32_t)SHADER_STAGE_FRAG,
        SHADER_STAGE_HULL = SHADER_STAGE_TESC,
        SHADER_STAGE_DOMAIN = SHADER_STAGE_TESE,
        SHADER_STAGE_COUNT = 6,
    };
    DEFINE_FLAG_ENUM_OPERATORS(SHADER_STAGE);

    CYBER_TYPED_ENUM(SHADER_TARGET, uint8_t)
    {
    #if defined (DIRECT3D11)
        SHADER_TARGET_5_0,
    #endif
        SHADER_TARGET_5_1,
        SHADER_TARGET_6_0,
        SHADER_TARGET_6_1,
        SHADER_TARGET_6_2,
        SHADER_TARGET_6_3,
        SHADER_TARGET_6_4,
    };

    CYBER_TYPED_ENUM(SHADER_COMPILER, uint8_t)
    {
        SHADER_COMPILER_DEFAULT = 0,
        SHADER_COMPILER_GLSLANG,
        SHADER_COMPILER_DXC,
        SHADER_COMPILER_FXC,
        SHADER_COMPILER_COUNT=4,
    };

    CYBER_TYPED_ENUM(PRIMITIVE_TOPOLOGY, uint8_t)
    {
        PRIM_TOPO_POINT_LIST = 0,
        PRIM_TOPO_LINE_LIST,
        PRIM_TOPO_LINE_STRIP,
        PRIM_TOPO_TRIANGLE_LIST,
        PRIM_TOPO_TRIANGLE_STRIP,
        PRIM_TOPO_PATCH_LIST,
        PRIM_TOPO_COUNT,
    };

    CYBER_TYPED_ENUM(BLEND_MODE, uint8_t)
    {
        BLEND_MODE_ADD = 0,
        BLEND_MODE_SUBTRACT,
        BLEND_MODE_REVERSE_SUBTRACT,
        BLEND_MODE_MIN,
        BLEND_MODE_MAX,
        BLEND_MODE_COUNT,
    };

    CYBER_TYPED_ENUM(BLEND_CONSTANT, uint8_t)
    {
        BLEND_CONSTANT_ZERO = 0,
        BLEND_CONSTANT_ONE,
        BLEND_CONSTANT_SRC_COLOR,
        BLEND_CONSTANT_ONE_MINUS_SRC_COLOR,
        BLEND_CONSTANT_DST_COLOR,
        BLEND_CONSTANT_ONE_MINUS_DST_COLOR,
        BLEND_CONSTANT_SRC_ALPHA,
        BLEND_CONSTANT_ONE_MINUS_SRC_ALPHA,
        BLEND_CONSTANT_DST_ALPHA,
        BLEND_CONSTANT_ONE_MINUS_DST_ALPHA,
        BLEND_CONSTANT_SRC_ALPHA_SATURATE,
        BLEND_CONSTANT_BLEND_FACTOR,
        BLEND_CONSTANT_ONE_MINUS_BLEND_FACTOR,
        BLEND_CONSTANT_COUNT,
    };

    CYBER_TYPED_ENUM(CULL_MODE, uint8_t)
    {
        CULL_MODE_NONE = 0,
        CULL_MODE_BACK,
        CULL_MODE_FRONT,
        CLUU_MODE_BOTH,
        CULL_MODE_COUNT,
    };

    CYBER_TYPED_ENUM(FRONT_FACE, uint8_t)
    {
        FRONT_FACE_COUNTER_CLOCKWISE = 0,
        FRONT_FACE_CLOCKWISE,
    };

    CYBER_TYPED_ENUM(FILL_MODE, uint8_t)
    {
        FILL_MODE_SOLID = 0,
        FILL_MODE_WIREFRAME,
        FILL_MODE_COUNT,
    };

    CYBER_TYPED_ENUM(COMPARE_MODE, uint8_t)
    {
        CMP_NEVER = 0,
        CMP_LESS,
        CMP_EQUAL,
        CMP_LESS_EQUAL,
        CMP_GREATER,
        CMP_NOT_EQUAL,
        CMP_GREATER_EQUAL,
        CMP_ALWAYS,
        CMP_COUNT,
    };

    CYBER_TYPED_ENUM(STENCIL_OPERATION, uint8_t)
    {
        STENCIL_OP_KEEP = 0,
        STENCIL_OP_ZERO,
        STENCIL_OP_REPLACE,
        STENCIL_OP_INVERT,
        STENCIL_OP_INCR,
        STENCIL_OP_DECR,
        STENCIL_OP_INCR_SAT,
        STENCIL_OP_DECR_SAT,
        STENCIL_OP_COUNT,
    };

    CYBER_TYPED_ENUM(FILTER_TYPE, uint8_t)
    {
        FILTER_TYPE_UNKNOWN = 0,
        FILTER_TYPE_POINT,
        FILTER_TYPE_LINEAR,
        FILTER_TYPE_ANISOTROPIC,
        FILTER_TYPE_COMPARISON_POINT,
        FILTER_TYPE_COMPARISON_LINEAR,
        FILTER_TYPE_COMPARISON_ANISOTROPIC,
        FILTER_TYPE_MINIMUM_POINT,
        FILTER_TYPE_MINIMUM_LINEAR,
        FILTER_TYPE_MINIMUM_ANISOTROPIC,
        FILTER_TYPE_MAXIMUM_POINT,
        FILTER_TYPE_MAXIMUM_LINEAR,
        FILTER_TYPE_MAXIMUM_ANISOTROPIC,
        FILTER_TYPE_COUNT
    };

    CYBER_TYPED_ENUM(ADDRESS_MODE, uint8_t)
    {
        ADDRESS_MODE_UNKNOWN = 0,
        ADDRESS_MODE_WRAP,
        ADDRESS_MODE_MIRROR,
        ADDRESS_MODE_CLAMP,
        ADDRESS_MODE_BORDER,
        ADDRESS_MODE_MIRROR_ONCE,
        ADDRESS_MODE_COUNT
    };

    // used for vk sampler
    CYBER_TYPED_ENUM(SAMPLER_FLAG, uint8_t)
    {
        SAMPLER_FLAG_NONE = 0,
        SAMPLER_FLAG_SUBSAMPLED = 1U << 0,
        SAMPLER_FLAG_SUBSAMPLED_COARSE_RECONSTRUCTION = 1U << 1,
        SAMPLER_FLAG_LAST = SAMPLER_FLAG_SUBSAMPLED_COARSE_RECONSTRUCTION
    };

    CYBER_TYPED_ENUM(MIPMAP_MODE, uint8_t)
    {
        MIPMAP_MODE_NEAREST = 0,
        MIPMAP_MODE_LINEAR,
    };

    CYBER_TYPED_ENUM(LOAD_ACTION, uint8_t)
    {
        LOAD_ACTION_DONT_CARE = 0,
        LOAD_ACTION_LOAD,
        LOAD_ACTION_CLEAR,
        LOAD_ACTION_COUNT,
    };

    CYBER_TYPED_ENUM(STORE_ACTION, uint8_t)
    {
        STORE_ACTION_STORE = 0,
        STORE_ACTION_DISCARD,
        STORE_ACTION_COUNT,
    };

    /// These flags mirror [VkPipelineStageFlagBits](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkPipelineStageFlagBits)
    /// enum and only have effect in Vulkan backend.
    CYBER_TYPED_ENUM(PIPELINE_STAGE_FLAG, uint32_t)
    {
        /// Undefined stage
        PIPELINE_STAGE_FLAG_UNDEFINED = 0x00000000,

        /// The top of the pipeline.
        PIPELINE_STAGE_FLAG_TOP_OF_PIPE = 0x00000001,

        /// The stage of the pipeline where Draw/DispatchIndirect data structures are consumed.
        PIPELINE_STAGE_FLAG_DRAW_INDIRECT = 0x00000002,

        /// The stage of the pipeline where vertex and index buffers are consumed.
        PIPELINE_STAGE_FLAG_VERTEX_INPUT = 0x00000004,

        /// Vertex shader stage.
        PIPELINE_STAGE_FLAG_VERTEX_SHADER = 0x00000008,

        /// Hull shader stage.
        PIPELINE_STAGE_FLAG_HULL_SHADER = 0x00000010,

        /// Domain shader stage.
        PIPELINE_STAGE_FLAG_DOMAIN_SHADER = 0x00000020,

        /// Geometry shader stage.
        PIPELINE_STAGE_FLAG_GEOMETRY_SHADER = 0x00000040,

        /// Pixel shader stage.
        PIPELINE_STAGE_FLAG_PIXEL_SHADER = 0x00000080,

        /// The stage of the pipeline where early fragment tests (depth and
        /// stencil tests before fragment shading) are performed. This stage
        /// also includes subpass load operations for framebuffer attachments
        /// with a depth/stencil format.
        PIPELINE_STAGE_FLAG_EARLY_FRAGMENT_TESTS = 0x00000100,

        /// The stage of the pipeline where late fragment tests (depth and
        /// stencil tests after fragment shading) are performed. This stage
        /// also includes subpass store operations for framebuffer attachments
        /// with a depth/stencil format.
        PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS = 0x00000200,

        /// The stage of the pipeline after blending where the final color values
        /// are output from the pipeline. This stage also includes subpass load
        /// and store operations and multisample resolve operations for framebuffer
        /// attachments with a color or depth/stencil format.
        PIPELINE_STAGE_FLAG_RENDER_TARGET = 0x00000400,

        /// Compute shader stage.
        PIPELINE_STAGE_FLAG_COMPUTE_SHADER = 0x00000800,

        /// The stage where all copy and outside-of-renderpass
        /// resolve and clear operations happen.
        PIPELINE_STAGE_FLAG_TRANSFER = 0x00001000,

        /// The bottom of the pipeline.
        PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE = 0x00002000,

        /// A pseudo-stage indicating execution on the host of reads/writes
        /// of device memory. This stage is not invoked by any commands recorded
        /// in a command buffer.
        PIPELINE_STAGE_FLAG_HOST = 0x00004000,

        /// The stage of the pipeline where the predicate of conditional rendering is consumed.
        PIPELINE_STAGE_FLAG_CONDITIONAL_RENDERING = 0x00040000,

        /// The stage of the pipeline where the shading rate texture is
        /// read to determine the shading rate for portions of a rasterized primitive.
        PIPELINE_STAGE_SHADING_RATE_TEXTURE = 0x00400000,

        /// Ray tracing shader.
        PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER = 0x00200000,

        /// Acceleration structure build shader.
        PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD = 0x02000000,

        /// Task shader stage.
        PIPELINE_STAGE_FLAG_TASK_SHADER = 0x00080000,

        /// Mesh shader stage.
        PIPELINE_STAGE_FLAG_MESH_SHADER = 0x00100000,

        /// The stage of the pipeline where the fragment density map is read to generate the fragment areas.
        PIPELINE_STAGE_FLAG_FRAGMENT_DENSITY_PROCESS = 0x00800000,

        /// Default pipeline stage that is determined by the resource state.
        /// For example, RESOURCE_STATE_RENDER_TARGET corresponds to
        /// PIPELINE_STAGE_FLAG_RENDER_TARGET pipeline stage.
        PIPELINE_STAGE_FLAG_DEFAULT = 0x80000000
    };


    /// The flags mirror [VkAccessFlags](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkAccessFlags) enum
    /// and only have effect in Vulkan backend.
    CYBER_TYPED_ENUM(ACCESS_FLAG, uint32_t)
    {
        /// No access
        ACCESS_FLAG_NONE                         = 0x00000000,

        /// Read access to indirect command data read as part of an indirect
        /// drawing or dispatch command.
        ACCESS_FLAG_INDIRECT_COMMAND_READ        = 0x00000001,

        /// Read access to an index buffer as part of an indexed drawing command
        ACCESS_FLAG_INDEX_READ                   = 0x00000002,

        /// Read access to a vertex buffer as part of a drawing command
        ACCESS_FLAG_VERTEX_READ                  = 0x00000004,

        /// Read access to a uniform buffer
        ACCESS_FLAG_UNIFORM_READ                 = 0x00000008,

        /// Read access to an input attachment within a render pass during fragment shading
        ACCESS_FLAG_INPUT_ATTACHMENT_READ        = 0x00000010,

        /// Read access from a shader resource, formatted buffer, UAV
        ACCESS_FLAG_SHADER_READ                  = 0x00000020,

        /// Write access to a UAV
        ACCESS_FLAG_SHADER_WRITE                 = 0x00000040,

        /// Read access to a color render target, such as via blending,
        /// logic operations, or via certain subpass load operations.
        ACCESS_FLAG_RENDER_TARGET_READ           = 0x00000080,

        /// Write access to a color render target, resolve, or depth/stencil resolve
        /// attachment during a render pass or via certain subpass load and store operations.
        ACCESS_FLAG_RENDER_TARGET_WRITE          = 0x00000100,

        /// Read access to a depth/stencil buffer, via depth or stencil operations
        /// or via certain subpass load operations
        ACCESS_FLAG_DEPTH_STENCIL_READ           = 0x00000200,

        /// Write access to a depth/stencil buffer, via depth or stencil operations
        /// or via certain subpass load and store operations
        ACCESS_FLAG_DEPTH_STENCIL_WRITE          = 0x00000400,

        /// Read access to an texture or buffer in a copy operation.
        ACCESS_FLAG_COPY_SRC                     = 0x00000800,

        /// Write access to an texture or buffer in a copy operation.
        ACCESS_FLAG_COPY_DST                     = 0x00001000,

        /// Read access by a host operation. Accesses of this type are
        /// not performed through a resource, but directly on memory.
        ACCESS_FLAG_HOST_READ                    = 0x00002000,

        /// Write access by a host operation. Accesses of this type are
        /// not performed through a resource, but directly on memory.
        ACCESS_FLAG_HOST_WRITE                   = 0x00004000,

        /// All read accesses. It is always valid in any access mask,
        /// and is treated as equivalent to setting all READ access flags
        /// that are valid where it is used.
        ACCESS_FLAG_MEMORY_READ                  = 0x00008000,

        /// All write accesses. It is always valid in any access mask,
        /// and is treated as equivalent to setting all WRITE access
        // flags that are valid where it is used.
        ACCESS_FLAG_MEMORY_WRITE                 = 0x00010000,

        /// Read access to a predicate as part of conditional rendering.
        ACCESS_FLAG_CONDITIONAL_RENDERING_READ   = 0x00100000,

        /// Read access to a shading rate texture as part of a drawing command.
        ACCESS_FLAG_SHADING_RATE_TEXTURE_READ    = 0x00800000,

        /// Read access to an acceleration structure as part of a trace or build command.
        ACCESS_FLAG_ACCELERATION_STRUCTURE_READ  = 0x00200000,

        /// Write access to an acceleration structure or acceleration structure
        /// scratch buffer as part of a build command.
        ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE = 0x00400000,

        /// Read access to a fragment density map attachment during
        /// dynamic fragment density map operations.
        ACCESS_FLAG_FRAGMENT_DENSITY_MAP_READ    = 0x01000000,

        /// Default access type that is determined by the resource state.
        /// For example, RESOURCE_STATE_RENDER_TARGET corresponds to
        /// ACCESS_FLAG_RENDER_TARGET_WRITE access type.
        ACCESS_FLAG_DEFAULT                      = 0x80000000

    };

    typedef struct GRAPHICS_COLOR
    {
        float r;
        float g;
        float b;
        float a;
    } GRAPHICS_COLOR;
    
    typedef union GRAPHICS_CLEAR_VALUE
    {
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
        struct
        {
            float depth;
            uint32_t stencil;
        };
    } GRAPHICS_CLEAR_VALUE;
    
    static const GRAPHICS_CLEAR_VALUE fastclear_0000 = {
        {0.f, 0.f, 0.f, 0.f}
    };
    static const GRAPHICS_CLEAR_VALUE fastclear_0001 = {
        {0.f, 0.f, 0.f, 1.f}
    };
    static const GRAPHICS_CLEAR_VALUE fastclear_1110 = {
        {1.f, 1.f, 1.f, 0.f}
    };
    static const GRAPHICS_CLEAR_VALUE fastclear_1111 = {
        {1.f, 1.f, 1.f, 1.f}
    };

    struct CYBER_GRAPHICS_API Surface
    {
        HWND handle;
    };

    /// Shaders
    struct CYBER_GRAPHICS_API ShaderVariable
    {
        // Variable name 
        const char* pNmae;
        // parents resource index
        uint32_t parent_index;
    };
    struct CYBER_GRAPHICS_API ShaderMacro
    {
        const char* definition;
        const char* value;
    };
    
    struct CYBER_GRAPHICS_API DescriptorSet
    {

    };

    struct CYBER_GRAPHICS_API RenderPipeline
    {
        RenderObject::IRenderDevice* device;
        RenderObject::IRootSignature* root_signature;
    };
    /// Shader Reflection
    struct CYBER_GRAPHICS_API ShaderConstant
    {
        const void* value;
        uint32_t index;
        uint32_t size;
    };

    struct CYBER_GRAPHICS_API ShaderByteCodeBuffer 
    {
        static CYBER_CONSTEXPR const uint32_t stack_size = 128u * 1024;
        // Stack memory, no need to deallocate it. Used first, if a shader if too big we allocate heap mempry
        void* stack_memory;
        uint32_t stack_used;
    };

    struct CYBER_GRAPHICS_API ShaderVersion
    {
        uint32_t major = 0;
        uint32_t minor = 0;

        CYBER_CONSTEXPR  ShaderVersion() CYBER_NOEXCEPT
        {

        }

        CYBER_CONSTEXPR ShaderVersion(uint32_t major, uint32_t minor) CYBER_NOEXCEPT
            : major(major)
            , minor(minor)
        {

        }

        CYBER_CONSTEXPR bool operator==(const ShaderVersion& other) const
        {
            return major == other.major && minor == other.minor;
        }

        CYBER_CONSTEXPR bool operator!=(const ShaderVersion& other) const
        {
            return !(*this == other);
        }

        CYBER_CONSTEXPR bool operator<(const ShaderVersion& other) const
        {
            return major < other.major || (major == other.major && minor < other.minor);
        }

        CYBER_CONSTEXPR bool operator>(const ShaderVersion& other) const
        {
            return major > other.major || (major == other.major && minor > other.minor);
        }

        CYBER_CONSTEXPR bool operator<=(const ShaderVersion& other) const
        {
            return !(*this > other);
        }

        CYBER_CONSTEXPR bool operator>=(const ShaderVersion& other) const
        {
            return !(*this < other);
        }
    };

    struct CYBER_GRAPHICS_API DescriptorData
    {
        // Update Via shader reflection
        const char8_t* name;
        // Update Via binding slot
        uint32_t binding;
        GRAPHICS_RESOURCE_TYPE binding_type;
        union
        {
            struct
            {
                /// Offset to bind the buffer descriptor
                const uint64_t* offsets;
                const uint64_t* sizes;
            } buffers_params;
            struct
            {
                uint32_t uav_mip_slice;
                bool blend_mip_chain;
            } uav_params;
            bool enable_stencil_resource;
        };
        union
        {
            const void** ptrs;
            /// Array of texture descriptors (srv and uav textures)
            RenderObject::ITextureView** textures;
            /// Array of sampler descriptors
            struct ISampler** samplers;
            /// Array of buffer descriptors (srv uav and cbv buffers)
            RenderObject::IBuffer** buffers;
            /// Array of pipeline descriptors
            RenderPipeline** render_pipelines;

            /// DescriptorSet buffer extraction
            DescriptorSet** descriptor_sets;
        };
        uint32_t count;
    };
    
    struct CYBER_GRAPHICS_API BufferRange
    {
        uint64_t offset;
        uint64_t size;
    };

    struct CYBER_GRAPHICS_API BufferBarrier
    {
        RenderObject::IBuffer* buffer;
        GRAPHICS_RESOURCE_STATE src_state;
        GRAPHICS_RESOURCE_STATE dst_state;
        uint8_t queue_acquire : 1;
        uint8_t queue_release : 1;
        QUEUE_TYPE queue_type : 5;
        struct {
            uint8_t begin_only : 1;
            uint8_t end_only : 1;
        } d3d12;
    };

    struct CYBER_GRAPHICS_API TextureBarrier
    {
        RenderObject::ITexture* texture;
        GRAPHICS_RESOURCE_STATE src_state;
        GRAPHICS_RESOURCE_STATE dst_state;
        uint8_t queue_acquire : 1;
        uint8_t queue_release : 1;
        QUEUE_TYPE queue_type : 5;
        /// Specify whether following barrier targets particular subresource
        uint8_t subresource_barrier : 1;
        /// Following values are ignored if subresource_barrier is false
        uint8_t mip_level : 7;
        uint16_t array_layer;
        struct {
            uint8_t begin_only : 1;
            uint8_t end_only : 1;
        } d3d12;
    };

    struct CYBER_GRAPHICS_API ResourceBarrierDesc
    {
        const BufferBarrier* buffer_barriers;
        uint32_t buffer_barrier_count;
        const TextureBarrier* texture_barriers;
        uint32_t texture_barrier_count;
    };

    /// Device Group
    struct CYBER_GRAPHICS_API QueueGroupDesc
    {
        QUEUE_TYPE m_queueType;
        uint32_t m_queueCount;
    };

    using RenderPassEncoder = RenderObject::ICommandBuffer;
    using ComputePassEncoder = RenderObject::ICommandBuffer;

    struct CYBER_GRAPHICS_API AcquireNextDesc
    {
        RenderObject::ISemaphore* signal_semaphore;
        RenderObject::IFence* fence;
    };

    struct CYBER_GRAPHICS_API ColorAttachment
    {
        RenderObject::ITextureView* view;
        RenderObject::ITextureView* resolve_view;
        LOAD_ACTION load_action;
        STORE_ACTION store_action;
        GRAPHICS_CLEAR_VALUE clear_value;
    };

    struct CYBER_GRAPHICS_API DepthStencilAttachment
    {
        RenderObject::ITextureView* view;
        LOAD_ACTION depth_load_action;
        STORE_ACTION depth_store_action;
        float clear_depth;
        uint8_t write_depth;
        LOAD_ACTION stencil_load_action;
        STORE_ACTION stencil_store_action;
        uint32_t clear_stencil;
        uint8_t write_stencil;
    };

    struct CYBER_GRAPHICS_API BlendStateCreateDesc
    {
        /// Source blend factor per render target
        BLEND_CONSTANT src_factors[GRAPHICS_MAX_MRT_COUNT];
        /// Destination blend factor per render target
        BLEND_CONSTANT dst_factors[GRAPHICS_MAX_MRT_COUNT];
        /// Source blend alpha factor per render target
        BLEND_CONSTANT src_alpha_factors[GRAPHICS_MAX_MRT_COUNT];
        /// Destination blend alpha factor per render target
        BLEND_CONSTANT dst_alpha_factors[GRAPHICS_MAX_MRT_COUNT];
        /// Blend mode per render target
        BLEND_MODE blend_modes[GRAPHICS_MAX_MRT_COUNT];
        /// Blend alpha mode per render target
        BLEND_MODE blend_alpha_modes[GRAPHICS_MAX_MRT_COUNT];
        /// Color write mask per render target
        int32_t masks[GRAPHICS_MAX_MRT_COUNT];
        /// Enable alpha to coverage
        bool alpha_to_coverage : 1;
        /// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for all render targets.
        bool independent_blend : 1;
    };

    struct CYBER_GRAPHICS_API DepthStateCreateDesc
    {
        bool depth_test : 1;
        bool depth_write : 1;
        COMPARE_MODE depth_func;
        bool stencil_test : 1;
        uint8_t stencil_read_mask;
        uint8_t stencil_write_mask;
        COMPARE_MODE stencil_front_func;
        STENCIL_OPERATION stencil_front_fail_op;
        STENCIL_OPERATION stencil_front_depth_fail_op;
        STENCIL_OPERATION stencil_front_pass_op;
        COMPARE_MODE stencil_back_func;
        STENCIL_OPERATION stencil_back_fail_op;
        STENCIL_OPERATION stencil_back_depth_fail_op;
        STENCIL_OPERATION stencil_back_pass_op;   
    };

    struct CYBER_GRAPHICS_API RasterizerStateCreateDesc
    {  
        CULL_MODE cull_mode;
        int32_t depth_bias;
        float slope_scaled_depth_bias;
        FILL_MODE fill_mode;
        FRONT_FACE front_face;
        bool enable_multisample : 1;
        bool enable_scissor : 1;
        bool enable_depth_clip : 1;
    };


    #define GRAPHICS_SINGLE_GPU_NODE_COUNT 1
    #define GRAPHICS_SINGLE_GPU_NODE_MASK 1
    #define GRAPHICS_SINGLE_GPU_NODE_INDEX 0
}
