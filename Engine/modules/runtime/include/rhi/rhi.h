#pragma once
#include "cyber_rhi_config.h"
#include "flags.h"
#include <EASTL/vector.h>
#include "core/Core.h"

namespace Cyber
{
    #define RHI_ARRAY_LEN(array) ((sizeof(array)/ sizeof(array[0])))
    
    struct RHITexture;
    typedef Ref<RHITexture> TextureRHIRef;
    struct RHITexture2D;
    typedef Ref<RHITexture2D> Texture2DRHIRef;
    struct RHIBuffer;
    typedef Ref<RHIBuffer> BufferRHIRef;
    struct RHIQueue;
    typedef Ref<RHIQueue> QueueRHIRef;
    struct RHIFence;
    typedef Ref<RHIFence> FenceRHIRef;
    struct RHIInstance;
    typedef Ref<RHIInstance> InstanceRHIRef;

    typedef enum ERHIBackend
    {
        RHI_BACKEND_D3D12,
        RHI_BACKEND_VULKAN,
        RHI_BACKEND_METAL
    } ERHIBackend;

    typedef enum ERHIQueueType
    {
        RHI_QUEUE_TYPE_GRAPHICS = 0,
        RHI_QUEUE_TYPE_COMPUTE = 1,
        RHI_QUEUE_TYPE_TRANSFER = 2,
        RHI_QUEUE_TYPE_COUNT,
        MAX_QUEUE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
    } ERHIQueueType;

    enum ERHIQueueFlag 
    {
        RHI_QUEUE_FLAG_NONE = 0x0,
        RHI_QUEUE_FLAG_DISABLE_GPU_TIMEOUT = 0x1,
        RHI_QUEUE_FLAG_INIT_MICROPROFILE = 0x2,
        MAX_QUEUE_FLAG = 0xFFFFFFFF
    };

    union ClearValue
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
    };
    
    enum ERHIIndirectArgumentType
    {
        RHI_INDIRECT_ARG_INVALID,
        RHI_INDIRECT_DRAW,
        RHI_INDIRECT_DRAW_INDEX,
        RHI_INDIRECT_DISPATCH,
        RHI_INDIRECT_VERTEX_BUFFER,
        RHI_INDIRECT_INDEX_BUFFER,
        RHI_INDIRECT_CONSTANT,
        RHI_INDIRECT_CONSTANT_BUFFER_VIEW,     // only for dx
        RHI_INDIRECT_SHADER_RESOURCE_VIEW,     // only for dx
        RHI_INDIRECT_UNORDERED_ACCESS_VIEW,    // only for dx
        RHI_INDIRECT_COMMAND_BUFFER,            // metal ICB
        RHI_INDIRECT_COMMAND_BUFFER_RESET,      // metal ICB reset
        RHI_INDIRECT_COMMAND_BUFFER_OPTIMIZE    // metal ICB optimization
    };

    enum ERHIDescriptorType
    {
        RHI_DESCRIPTOR_TYPE_UNDEFINED = 0,
        RHI_DESCRIPTOR_TYPE_SAMPLER = 0x01,
        // SRV Read only texture
        RHI_DESCRIPTOR_TYPE_TEXTURE = (RHI_DESCRIPTOR_TYPE_SAMPLER << 1),
        /// UAV Texture
        RHI_DESCRIPTOR_TYPE_RW_TEXTURE = (RHI_DESCRIPTOR_TYPE_TEXTURE << 1),
        // SRV Read only buffer
        RHI_DESCRIPTOR_TYPE_BUFFER = (RHI_DESCRIPTOR_TYPE_RW_TEXTURE << 1),
        RHI_DESCRIPTOR_TYPE_BUFFER_RAW = (RHI_DESCRIPTOR_TYPE_BUFFER | (RHI_DESCRIPTOR_TYPE_BUFFER << 1)),
        /// UAV Buffer
        RHI_DESCRIPTOR_TYPE_RW_BUFFER = (RHI_DESCRIPTOR_TYPE_BUFFER << 2),
        RHI_DESCRIPTOR_TYPE_RW_BUFFER_RAW = (RHI_DESCRIPTOR_TYPE_RW_BUFFER | (RHI_DESCRIPTOR_TYPE_RW_BUFFER << 1)),
        /// Uniform buffer
        RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER = (RHI_DESCRIPTOR_TYPE_RW_BUFFER << 2),
        /// Push constant / Root constant
        RHI_DESCRIPTOR_TYPE_ROOT_CONSTANT = (RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER << 1),
        /// IA
        RHI_DESCRIPTOR_TYPE_VERTEX_BUFFER = (RHI_DESCRIPTOR_TYPE_ROOT_CONSTANT << 1),
        RHI_DESCRIPTOR_TYPE_INDEX_BUFFER = (RHI_DESCRIPTOR_TYPE_VERTEX_BUFFER << 1),
        RHI_DESCRIPTOR_TYPE_INDIRECT_BUFFER = (RHI_DESCRIPTOR_TYPE_INDEX_BUFFER << 1),
        /// Cubemap SRV
        RHI_DESCRIPTOR_TYPE_TEXTURE_CUBE = (RHI_DESCRIPTOR_TYPE_TEXTURE | (RHI_DESCRIPTOR_TYPE_INDIRECT_BUFFER << 1)),
        /// RTV / DSV per mip slice
        RHI_DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES = (RHI_DESCRIPTOR_TYPE_INDIRECT_BUFFER << 2),
        /// RTV / DSV per array slice
        RHI_DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES = (RHI_DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES << 1),
        /// RTV / DSV per depth slice
        RHI_DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES = (RHI_DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
        RHI_DESCRIPTOR_TYPE_RAY_TRACING = (RHI_DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
        RHI_DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER = (RHI_DESCRIPTOR_TYPE_RAY_TRACING << 1),
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

    enum ERHITextureCreationFlag
    {
        /// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
        RHI_TCF_NONE = 0,
        /// Texture will allocate its own memory (COMMITTED resource)
        RHI_TCF_OWN_MEMORY_BIT = 0x01,
        /// Texture will be allocated in memory which can be shared among multiple processes
        RHI_TCF_EXPORT_BIT = 0x02,
        /// Texture will be allocated in memory which can be shared among multiple gpus
        RHI_TCF_EXPORT_ADAPTER_BIT = 0x04,
        /// Use on-tile memory to store this texture
        RHI_TCF_ON_TILE = 0x08,
        /// Prevent compression meta data from generating (XBox)
        RHI_TCF_NO_COMPRESSION = 0x10,
        /// Force 2D instead of automatically determining dimension based on width, height, depth
        RHI_TCF_FORCE_2D = 0x20,
        /// Force 3D instead of automatically determining dimension based on width, height, depth
        RHI_TCF_FORCE_3D = 0x40,
        /// Display target
        RHI_TCF_FORCE_ALLOW_DISPLAY_TARGET = 0x80,
        /// Create an sRGB texture.
        RHI_TCF_SRGB = 0x100,
        /// Create a normal map texture
        RHI_TCF_NORMAL_MAP = 0x200,
        /// Fragment mask
        RHI_TCF_FRAG_MASK = 0x400,

        RHI_TCF_USABLE_MAX = 0x40000,

        RHI_TCF_MAX_ENUM_BIT = 0x7FFFFFFF
    };
    typedef uint32_t RHITextureCreationFlag;

    enum ERHITextureSampleCount
    {
        RHI_SAMPLE_COUNT_1 = 1,
        RHI_SAMPLE_COUNT_2 = 2,
        RHI_SAMPLE_COUNT_4 = 4,
        RHI_SAMPLE_COUNT_8 = 8,
        RHI_SAMPLE_COUNT_16 = 16,
        RHI_SAMPLE_COUNT_COUNT = 5
    };

    enum ERHIFormat
    {
        RHI_FORMAT_UNDEFINED = 0,
        RHI_FORMAT_R1_UNORM = 1,
        RHI_FORMAT_R2_UNORM = 2,   
        RHI_FORMAT_R4_UNORM = 3,
        RHI_FORMAT_R4G4_UNORM = 4,
        RHI_FORMAT_G4R4_UNORM = 5,
        RHI_FORMAT_A8_UNORM = 6,
        RHI_FORMAT_R8_UNORM = 7,
        RHI_FORMAT_R8_SNORM = 8,
        RHI_FORMAT_R8_UINT = 9,
        RHI_FORMAT_R8_SINT = 10,
        RHI_FORMAT_R8_SRGB = 11,
        RHI_FORMAT_B2G3R3_UNORM = 12,
        RHI_FORMAT_R4G4B4A4_UNORM = 13,
        RHI_FORMAT_R4G4B4X4_UNORM = 14,
        RHI_FORMAT_B4G4R4A4_UNORM = 15,
        RHI_FORMAT_B4G4R4X4_UNORM = 16,
        RHI_FORMAT_A4R4G4B4_UNORM = 17,
        RHI_FORMAT_X4R4G4B4_UNORM = 18,
        RHI_FORMAT_A4B4G4R4_UNORM = 19,
        RHI_FORMAT_X4B4G4R4_UNORM = 20,
        RHI_FORMAT_R5G6B5_UNORM = 21,
        RHI_FORMAT_B5G6R5_UNORM = 22,
        RHI_FORMAT_R5G5B5A1_UNORM = 23,
        RHI_FORMAT_B5G5R5A1_UNORM = 24,
        RHI_FORMAT_A1B5G5R5_UNORM = 25,
        RHI_FORMAT_A1R5G5B5_UNORM = 26,
        RHI_FORMAT_R5G5B5X1_UNORM = 27,
        RHI_FORMAT_B5G5R5X1_UNORM = 28,
        RHI_FORMAT_X1R5G5B5_UNORM = 29,
        RHI_FORMAT_X1B5G5R5_UNORM = 30,
        RHI_FORMAT_B2G3R3A8_UNORM = 31,
        RHI_FORMAT_R8G8_UNORM = 32,
        RHI_FORMAT_R8G8_SNORM = 33,
        RHI_FORMAT_G8R8_UNORM = 34,
        RHI_FORMAT_G8R8_SNORM = 35,
        RHI_FORMAT_R8G8_UINT = 36,
        RHI_FORMAT_R8G8_SINT = 37,
        RHI_FORMAT_R8G8_SRGB = 38,
        RHI_FORMAT_R16_UNORM = 39,
        RHI_FORMAT_R16_SNORM = 40,
        RHI_FORMAT_R16_UINT = 41,
        RHI_FORMAT_R16_SINT = 42,
        RHI_FORMAT_R16_SFLOAT = 43,
        RHI_FORMAT_R16_SBFLOAT = 44,
        RHI_FORMAT_R8G8B8_UNORM = 45,
        RHI_FORMAT_R8G8B8_SNORM = 46,
        RHI_FORMAT_R8G8B8_UINT = 47,
        RHI_FORMAT_R8G8B8_SINT = 48,
        RHI_FORMAT_R8G8B8_SRGB = 49,
        RHI_FORMAT_B8G8R8_UNORM = 50,
        RHI_FORMAT_B8G8R8_SNORM = 51,
        RHI_FORMAT_B8G8R8_UINT = 52,
        RHI_FORMAT_B8G8R8_SINT = 53,
        RHI_FORMAT_B8G8R8_SRGB = 54,
        RHI_FORMAT_R8G8B8A8_UNORM = 55,
        RHI_FORMAT_R8G8B8A8_SNORM = 56,
        RHI_FORMAT_R8G8B8A8_UINT = 57,
        RHI_FORMAT_R8G8B8A8_SINT = 58,
        RHI_FORMAT_R8G8B8A8_SRGB = 59,
        RHI_FORMAT_B8G8R8A8_UNORM = 60,
        RHI_FORMAT_B8G8R8A8_SNORM = 61,
        RHI_FORMAT_B8G8R8A8_UINT = 62,
        RHI_FORMAT_B8G8R8A8_SINT = 63,
        RHI_FORMAT_B8G8R8A8_SRGB = 64,
        RHI_FORMAT_R8G8B8X8_UNORM = 65,
        RHI_FORMAT_B8G8R8X8_UNORM = 66,
        RHI_FORMAT_R16G16_UNORM = 67,
        RHI_FORMAT_G16R16_UNORM = 68,
        RHI_FORMAT_R16G16_SNORM = 69,
        RHI_FORMAT_G16R16_SNORM = 70,
        RHI_FORMAT_R16G16_UINT = 71,
        RHI_FORMAT_R16G16_SINT = 72,
        RHI_FORMAT_R16G16_SFLOAT = 73,
        RHI_FORMAT_R16G16_SBFLOAT = 74,
        RHI_FORMAT_R32_UINT = 75,
        RHI_FORMAT_R32_SINT = 76,
        RHI_FORMAT_R32_SFLOAT = 77,
        RHI_FORMAT_A2R10G10B10_UNORM = 78,
        RHI_FORMAT_A2R10G10B10_UINT = 79,
        RHI_FORMAT_A2R10G10B10_SNORM = 80,
        RHI_FORMAT_A2R10G10B10_SINT = 81,
        RHI_FORMAT_A2B10G10R10_UNORM = 82,
        RHI_FORMAT_A2B10G10R10_UINT = 83,
        RHI_FORMAT_A2B10G10R10_SNORM = 84,
        RHI_FORMAT_A2B10G10R10_SINT = 85,
        RHI_FORMAT_R10G10B10A2_UNORM = 86,
        RHI_FORMAT_R10G10B10A2_UINT = 87,
        RHI_FORMAT_R10G10B10A2_SNORM = 88,
        RHI_FORMAT_R10G10B10A2_SINT = 89,
        RHI_FORMAT_B10G10R10A2_UNORM = 90,
        RHI_FORMAT_B10G10R10A2_UINT = 91,
        RHI_FORMAT_B10G10R10A2_SNORM = 92,
        RHI_FORMAT_B10G10R10A2_SINT = 93,
        RHI_FORMAT_B10G11R11_UFLOAT = 94,
        RHI_FORMAT_E5B9G9R9_UFLOAT = 95,
        RHI_FORMAT_R16G16B16_UNORM = 96,
        RHI_FORMAT_R16G16B16_SNORM = 97,
        RHI_FORMAT_R16G16B16_UINT = 98,
        RHI_FORMAT_R16G16B16_SINT = 99,
        RHI_FORMAT_R16G16B16_SFLOAT = 100,
        RHI_FORMAT_R16G16B16_SBFLOAT = 101,
        RHI_FORMAT_R16G16B16A16_UNORM = 102,
        RHI_FORMAT_R16G16B16A16_SNORM = 103,
        RHI_FORMAT_R16G16B16A16_UINT = 104,
        RHI_FORMAT_R16G16B16A16_SINT = 105,
        RHI_FORMAT_R16G16B16A16_SFLOAT = 106,
        RHI_FORMAT_R16G16B16A16_SBFLOAT = 107,
        RHI_FORMAT_R32G32_UINT = 108,
        RHI_FORMAT_R32G32_SINT = 109,
        RHI_FORMAT_R32G32_SFLOAT = 110,
        RHI_FORMAT_R32G32B32_UINT = 111,
        RHI_FORMAT_R32G32B32_SINT = 112,
        RHI_FORMAT_R32G32B32_SFLOAT = 113,
        RHI_FORMAT_R32G32B32A32_UINT = 114,
        RHI_FORMAT_R32G32B32A32_SINT = 115,
        RHI_FORMAT_R32G32B32A32_SFLOAT = 116,
        RHI_FORMAT_R64_UINT = 117,
        RHI_FORMAT_R64_SINT = 118,
        RHI_FORMAT_R64_SFLOAT = 119,
        RHI_FORMAT_R64G64_UINT = 120,
        RHI_FORMAT_R64G64_SINT = 121,
        RHI_FORMAT_R64G64_SFLOAT = 122,
        RHI_FORMAT_R64G64B64_UINT = 123,
        RHI_FORMAT_R64G64B64_SINT = 124,
        RHI_FORMAT_R64G64B64_SFLOAT = 125,
        RHI_FORMAT_R64G64B64A64_UINT = 126,
        RHI_FORMAT_R64G64B64A64_SINT = 127,
        RHI_FORMAT_R64G64B64A64_SFLOAT = 128,
        RHI_FORMAT_D16_UNORM = 129,
        RHI_FORMAT_X8_D24_UNORM = 130,
        RHI_FORMAT_D32_SFLOAT = 131,
        RHI_FORMAT_S8_UINT = 132,
        RHI_FORMAT_D16_UNORM_S8_UINT = 133,
        RHI_FORMAT_D24_UNORM_S8_UINT = 134,
        RHI_FORMAT_D32_SFLOAT_S8_UINT = 135,
        RHI_FORMAT_DXBC1_RGB_UNORM = 136,
        RHI_FORMAT_DXBC1_RGB_SRGB = 137,
        RHI_FORMAT_DXBC1_RGBA_UNORM = 138,
        RHI_FORMAT_DXBC1_RGBA_SRGB = 139,
        RHI_FORMAT_DXBC2_UNORM = 140,
        RHI_FORMAT_DXBC2_SRGB = 141,
        RHI_FORMAT_DXBC3_UNORM = 142,
        RHI_FORMAT_DXBC3_SRGB = 143,
        RHI_FORMAT_DXBC4_UNORM = 144,
        RHI_FORMAT_DXBC4_SNORM = 145,
        RHI_FORMAT_DXBC5_UNORM = 146,
        RHI_FORMAT_DXBC5_SNORM = 147,
        RHI_FORMAT_DXBC6H_UFLOAT = 148,
        RHI_FORMAT_DXBC6H_SFLOAT = 149,
        RHI_FORMAT_DXBC7_UNORM = 150,
        RHI_FORMAT_DXBC7_SRGB = 151,
        RHI_FORMAT_PVRTC1_2BPP_UNORM = 152,
        RHI_FORMAT_PVRTC1_4BPP_UNORM = 153,
        RHI_FORMAT_PVRTC2_2BPP_UNORM = 154,
        RHI_FORMAT_PVRTC2_4BPP_UNORM = 155,
        RHI_FORMAT_PVRTC1_2BPP_SRGB = 156,
        RHI_FORMAT_PVRTC1_4BPP_SRGB = 157,
        RHI_FORMAT_PVRTC2_2BPP_SRGB = 158,
        RHI_FORMAT_PVRTC2_4BPP_SRGB = 159,
        RHI_FORMAT_ETC2_R8G8B8_UNORM = 160,
        RHI_FORMAT_ETC2_R8G8B8_SRGB = 161,
        RHI_FORMAT_ETC2_R8G8B8A1_UNORM = 162,
        RHI_FORMAT_ETC2_R8G8B8A1_SRGB = 163,
        RHI_FORMAT_ETC2_R8G8B8A8_UNORM = 164,
        RHI_FORMAT_ETC2_R8G8B8A8_SRGB = 165,
        RHI_FORMAT_ETC2_EAC_R11_UNORM = 166,
        RHI_FORMAT_ETC2_EAC_R11_SNORM = 167,
        RHI_FORMAT_ETC2_EAC_R11G11_UNORM = 168,
        RHI_FORMAT_ETC2_EAC_R11G11_SNORM = 169,
        RHI_FORMAT_ASTC_4x4_UNORM = 170,
        RHI_FORMAT_ASTC_4x4_SRGB = 171,
        RHI_FORMAT_ASTC_5x4_UNORM = 172,
        RHI_FORMAT_ASTC_5x4_SRGB = 173,
        RHI_FORMAT_ASTC_5x5_UNORM = 174,
        RHI_FORMAT_ASTC_5x5_SRGB = 175,
        RHI_FORMAT_ASTC_6x5_UNORM = 176,
        RHI_FORMAT_ASTC_6x5_SRGB = 177,
        RHI_FORMAT_ASTC_6x6_UNORM = 178,
        RHI_FORMAT_ASTC_6x6_SRGB = 179,
        RHI_FORMAT_ASTC_8x5_UNORM = 180,
        RHI_FORMAT_ASTC_8x5_SRGB = 181,
        RHI_FORMAT_ASTC_8x6_UNORM = 182,
        RHI_FORMAT_ASTC_8x6_SRGB = 183,
        RHI_FORMAT_ASTC_8x8_UNORM = 184,
        RHI_FORMAT_ASTC_8x8_SRGB = 185,
        RHI_FORMAT_ASTC_10x5_UNORM = 186,
        RHI_FORMAT_ASTC_10x5_SRGB = 187,
        RHI_FORMAT_ASTC_10x6_UNORM = 188,
        RHI_FORMAT_ASTC_10x6_SRGB = 189,
        RHI_FORMAT_ASTC_10x8_UNORM = 190,
        RHI_FORMAT_ASTC_10x8_SRGB = 191,
        RHI_FORMAT_ASTC_10x10_UNORM = 192,
        RHI_FORMAT_ASTC_10x10_SRGB = 193,
        RHI_FORMAT_ASTC_12x10_UNORM = 194,
        RHI_FORMAT_ASTC_12x10_SRGB = 195,
        RHI_FORMAT_ASTC_12x12_UNORM = 196,
        RHI_FORMAT_ASTC_12x12_SRGB = 197,
        RHI_FORMAT_CLUT_P4 = 198,
        RHI_FORMAT_CLUT_P4A4 = 199,
        RHI_FORMAT_CLUT_P8 = 200,
        RHI_FORMAT_CLUT_P8A8 = 201,
        RHI_FORMAT_R4G4B4A4_UNORM_PACK16 = 202,
        RHI_FORMAT_B4G4R4A4_UNORM_PACK16 = 203,
        RHI_FORMAT_R5G6B5_UNORM_PACK16 = 204,
        RHI_FORMAT_B5G6R5_UNORM_PACK16 = 205,
        RHI_FORMAT_R5G5B5A1_UNORM_PACK16 = 206,
        RHI_FORMAT_B5G5R5A1_UNORM_PACK16 = 207,
        RHI_FORMAT_A1R5G5B5_UNORM_PACK16 = 208,
        RHI_FORMAT_G16B16G16R16_422_UNORM = 209,
        RHI_FORMAT_B16G16R16G16_422_UNORM = 210,
        RHI_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 211,
        RHI_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 212,
        RHI_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 213,
        RHI_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 214,
        RHI_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 215,
        RHI_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 216,
        RHI_FORMAT_G8B8G8R8_422_UNORM = 217,
        RHI_FORMAT_B8G8R8G8_422_UNORM = 218,
        RHI_FORMAT_G8_B8_R8_3PLANE_420_UNORM = 219,
        RHI_FORMAT_G8_B8R8_2PLANE_420_UNORM = 220,
        RHI_FORMAT_G8_B8_R8_3PLANE_422_UNORM = 221,
        RHI_FORMAT_G8_B8R8_2PLANE_422_UNORM = 222,
        RHI_FORMAT_G8_B8_R8_3PLANE_444_UNORM = 223,
        RHI_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 224,
        RHI_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 225,
        RHI_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 226,
        RHI_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 227,
        RHI_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 228,
        RHI_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 229,
        RHI_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 230,
        RHI_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 231,
        RHI_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 232,
        RHI_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 233,
        RHI_FORMAT_G16_B16_R16_3PLANE_420_UNORM = 234,
        RHI_FORMAT_G16_B16_R16_3PLANE_422_UNORM = 235,
        RHI_FORMAT_G16_B16_R16_3PLANE_444_UNORM = 236,
        RHI_FORMAT_G16_B16R16_2PLANE_420_UNORM = 237,
        RHI_FORMAT_G16_B16R16_2PLANE_422_UNORM = 238,
        RHI_FORMAT_COUNT = RHI_FORMAT_G16_B16R16_2PLANE_422_UNORM + 1,
        RHI_FORMAT_MAX_ENUM_BIT = 0x7FFFFFFF
    };

    enum ERHIResourceState
    {
        RHI_RESOURCE_STATE_UNDEFINED = 0,
        RHI_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
        RHI_RESOURCE_STATE_INDEX_BUFFER = 0x2,
        RHI_RESOURCE_STATE_RENDER_TARGET = 0x4,
        RHI_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
        RHI_RESOURCE_STATE_DEPTH_WRITE = 0x10,
        RHI_RESOURCE_STATE_DEPTH_READ = 0x20,
        RHI_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
        RHI_RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
        RHI_RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
        RHI_RESOURCE_STATE_STREAM_OUT = 0x100,
        RHI_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
        RHI_RESOURCE_STATE_COPY_DEST = 0x400,
        RHI_RESOURCE_STATE_COPY_SOURCE = 0x800,
        RHI_RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
        RHI_RESOURCE_STATE_PRESENT = 0x1000,
        RHI_RESOURCE_STATE_COMMON = 0x2000,
        RHI_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
        RHI_RESOURCE_STATE_SHADING_RATE_SOURCE = 0x8000,
    };
    typedef uint32_t RHIResourceState;

    enum ERHIResourceMemoryUsage
    {
        /// No intended memory usage specified.
        RHI_RESOURCE_MEMORY_USAGE_UNKNOWN = 0,
        /// Memory will be used on device only, no need to be mapped on host.
        RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY = 1,
        /// Memory will be mapped on host. Could be used for transfer to device.
        RHI_RESOURCE_MEMORY_USAGE_CPU_ONLY = 2,
        /// Memory will be used for frequent (dynamic) updates from host and reads on device.
        RHI_RESOURCE_MEMORY_USAGE_CPU_TO_GPU = 3,
        /// Memory will be used for writing on device and readback on host.
        RHI_RESOURCE_MEMORY_USAGE_GPU_TO_CPU = 4,
        RHI_RESOURCE_MEMORY_USAGE_COUNT,
        RHI_RESOURCE_MEMORY_USAGE_MAX_ENUM = 0x7FFFFFFF
    };

    enum ERHIBufferCreationFlags
    {
        /// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
        RHI_BCF_NONE = 0x01,
        /// Buffer will allocate its own memory (COMMITTED resource)
        RHI_BCF_OWN_MEMORY_BIT = 0x02,
        /// Buffer will be persistently mapped
        RHI_BCF_PERSISTENT_MAP_BIT = 0x04,
        /// Use ESRAM to store this buffer
        RHI_BCF_ESRAM = 0x08,
        /// Flag to specify not to allocate descriptors for the resource
        RHI_BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,

    #ifdef VULKAN
        /* Memory Host Flags */
        RHI_BCF_HOST_VISIBLE = 0x100,
        RHI_BCF_HOST_COHERENT = 0x200,
    #endif
    };
    typedef uint32_t RHIBufferCreationFlags;

    /// Texture group
    struct CYBER_RHI_API TextureCreationDesc
    {
        /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
        ClearValue mClearValue;
        /// Pointer to native texture handle if the texture does not own underlying resource
        void* mNativeHandle;
        /// Debug name used in gpu profile
        const char* pName;

        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;
        uint32_t mArraySize;
        uint32_t mMipLevels;
        ERHIDescriptorType mDescriptors;
        RHITextureCreationFlag mFlags;
        /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
        ERHITextureSampleCount mSampleCount;
        /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
        uint32_t mSampleQuality;
        /// Image format
        ERHIFormat mFormat;
        /// What state will the texture get created in
        ERHIResourceState mStartState;
    };

    struct CYBER_RHI_API RHITexture
    {
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;
        uint32_t mArraySize;
        uint32_t mMipLevels;
        void* mNativeHandle;
    };

    struct RHITexture2D : public RHITexture
    {

    };

    struct RHITexture3D : public RHITexture
    {

    };

    /// Buffer Group
    struct CYBER_RHI_API BufferCreateDesc
    {
        /// Size of the buffer (in bytes)
        uint64_t mSize;
        /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
        struct RHIBuffer* pCounterBuffer;
        /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
        uint64_t mFirstElement;
        /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
        uint64_t mElementCount;
        /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
        uint64_t mStructStride;
        /// Debug name used in gpu profile
        const char* pName;
        uint32_t* pSharedNodeIndices;
        /// Alignment
        uint32_t mAlignment;
        /// Decides which memory heap buffer will use (default, upload, readback)
        ERHIResourceMemoryUsage mMemoryUsage;
        /// Creation flags of the buffer
        ERHIBufferCreationFlags mFlags;
        /// What type of queue the buffer is owned by
        ERHIQueueType mQueueType;
        /// What state will the buffer get created in
        ERHIResourceState mStartState;
        /// ICB draw type
        ERHIIndirectArgumentType mICBDrawType;
        /// ICB max vertex buffers slots count
        uint32_t mICBMaxCommandCount;
        /// Image format
        ERHIFormat mFormat;
        /// Descriptor creation
        ERHIDescriptorType mDescriptors;
    };

    struct CYBER_RHI_API RHIBuffer
    {
        /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
        void* pCpuMappedAddress;
        uint64_t mSize : 32;
        uint64_t mDescriptors : 20;
        uint64_t mMemoryUsage : 3;
        uint64_t mNodeIndex : 4;
    };

    struct CYBER_RHI_API RHIInstanceCreateDesc
    {
        ERHIBackend mBackend;
        bool mEnableDebugLayer;
        bool mEnableGpuBasedValidation;
        bool mEnableSetName;
    };

    // Objects
    class CYBER_RHI_API RHIInstance
    {
        ERHIBackend mBackend;
        ERHINvAPI_Status mNvAPIStatus;
        ERHIAGSReturenCode mAgsStatus;
        bool mEnableSetName;
    };
    /// Device Group
    struct CYBER_RHI_API QueueGroupDesc
    {
        ERHIQueueType mQueueType;
        uint32_t mQueueCount;
    };

    struct CYBER_RHI_API RHIAdapterDetail
    {
        uint32_t mUniformBufferAlignment;
        uint32_t mUploadBufferTextureAlignment;
        uint32_t mUploadBufferTextureRowAlignment;
        uint32_t mMaxVertexInputBindings;
        uint32_t mWaveLaneCount;
        uint32_t mHostVisibleVRamBudget;
        bool mSupportHostVisibleVRam : 1;
        bool mMultiDrawIndirect : 1;
        bool mSupportGeomShader : 1;
        bool mSupportTessellation : 1;
        bool mIsUma : 1;
        bool mIsVirtual : 1;
        bool mIsCpu : 1;
    };

    class CYBER_RHI_API RHIAdapter
    {
    public:
        Cyber::Ref<RHIInstance> pInstance;
    };

    struct CYBER_RHI_API DeviceCreateDesc
    {
        bool bDisablePipelineCache;
        eastl::vector<QueueGroupDesc> mQueueGroupsDesc;
        uint32_t mQueueGroupCount;
    };

    class CYBER_RHI_API RHIDevice
    {
    public:
        Cyber::Ref<RHIAdapter> pAdapter;
    };

    class CYBER_RHI_API RHIFence
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
    };

    class CYBER_RHI_API RHISemaphore
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
    };

    class CYBER_RHI_API RHIQueue
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
    };

    class CYBER_RHI_API RHICommandPool
    {
    public:
        Cyber::Ref<RHIQueue> pQueue;
    };

    class CYBER_RHI_API RHICommandBuffer
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        Cyber::Ref<RHICommandPool> pPool;
        ERHIPipelineType mCurrentDispatch;
    };

    class CYBER_RHI_API RHIQueryPool
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        uint32_t mCount;
    };

    struct CYBER_RHI_API RHIQueueSubmitDesc
    {
        Ref<RHICommandBuffer> pCmds;
        RHIFence mSignalFence;
        RHISemaphore* pWaitSemaphores;
        RHISemaphore* pSignalSemaphores;
        uint32_t mCmdsCount;
        uint32_t mWaitSemaphoreCount;
        uint32_t mSignalSemaphoreCount;
    };

    struct CYBER_RHI_API RHIQueueGroupDesc
    {
        ERHIQueueType mQueueType;
        uint32_t mQueueCount;
    };

    struct CYBER_RHI_API RHIQueuePresentDesc
    {
        
    };

    class CYBER_RHI_API RHIRenderer
    {
    public:
        Cyber::Scope<RHIDevice> pDevice;
        Cyber::Scope<RHIAdapter> pAdapter;
    };

    class CYBER_RHI_API RHI
    {

    public:
        static void createRHI(ERHIBackend backend);
        inline static RHI& GetRHIContext()
        {
            cyber_assert(globalRHI, "RHI has not been initialized!");
            return *gloablRHI;
        }

    protected:
        static RHI* gloablRHI;

    public:
        // Instance APIs
        virtual InstanceRHIRef rhi_create_instance(Ref<RHIDevice> pDevice, const RHIInstanceCreateDesc& instanceDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_instance!");
            return CreateRef<RHIInstance>();
        }

        // Device APIS
        virtual void rhi_create_device(Ref<RHIDevice> pDevice, Ref<RHIAdapter> pAdapter, const DeviceCreateDesc& deviceDesc) {}

        // API Object APIs
        virtual FenceRHIRef rhi_create_fence(Ref<RHIDevice> pDevice) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_fence!");
            return CreateRef<RHIFence>();
        };
        virtual void rhi_wait_fences(const RHIFence* pFences, uint32_t fenceCount) {}
        virtual void rhi_query_fence_status(Ref<RHIFence> pFence) {}

        // Queue APIs
        virtual QueueRHIRef rhi_get_queue(Ref<RHIDevice> pDevice, ERHIQueueType type, uint32_t index) 
        { 
            cyber_core_assert(false, "Empty implement rhi_get_queue!");
            return CreateRef<RHIQueue>();
        }
        //virtual void rhi_submit_queue(Ref<RHIQueue> pQueue, const )

        // Resource APIs
        virtual Texture2DRHIRef rhi_create_texture(Ref<RHIDevice> pDevice, const TextureCreationDesc& textureDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture!");
            return CreateRef<RHITexture2D>();
        }
        virtual void rhi_remove_texture() {}
        
        virtual BufferRHIRef rhi_create_buffer(Ref<RHIDevice> pDevice, const BufferCreateDesc& bufferDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture!");
            return CreateRef<RHIBuffer>();
        }

        virtual void rhi_remove_buffer() {}
        virtual void rhi_map_buffer() {}

        virtual void rhi_create_rendertarget() {}
    };

    #define RHI_SINGLE_GPU_NODE_COUNT 1
    #define RHI_SINGLE_GPU_NODE_MASK 1
    #define RHI_SINGLE_GPU_NODE_INDEX 0

}
