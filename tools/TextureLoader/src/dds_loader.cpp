#include "texture_loader_impl.hpp"
#include "image.h"
#include "graphics/interface/render_device.hpp"
#include "common/graphics_utils.hpp"
#include "texture_utils.h"
#include <algorithm>
#include "core/common.h"
#include "core/file_helper.hpp"
#include "dds_loader.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

enum RESOURCE_DIMENSION
{
    RESOURCE_DIMENSION_UNKNOWN = 0,
    RESOURCE_DIMENSION_BUFFER = 1,
    RESOURCE_DIMENSION_TEXTURE1D = 2,
    RESOURCE_DIMENSION_TEXTURE2D = 3,
    RESOURCE_DIMENSION_TEXTURE3D = 4
};
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
     ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif

static size_t bits_per_pixel(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return 32;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return 16;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
        return 8;

    case DXGI_FORMAT_R1_UNORM:
        return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 8;

    default:
        return 0;
    }
}

static TEXTURE_FORMAT DXGI_FORMAT_TO_TEXFMT(DXGI_FORMAT format)
{
    switch(format)
    {
        case DXGI_FORMAT_UNKNOWN:                       return TEX_FORMAT_UNKNOWN;

        case DXGI_FORMAT_R32G32B32A32_TYPELESS:         return TEX_FORMAT_RGBA32_TYPELESS; 
        case DXGI_FORMAT_R32G32B32A32_FLOAT:            return TEX_FORMAT_RGBA32_FLOAT; 
        case DXGI_FORMAT_R32G32B32A32_UINT:             return TEX_FORMAT_RGBA32_UINT; 
        case DXGI_FORMAT_R32G32B32A32_SINT:             return TEX_FORMAT_RGBA32_SINT; 

        case DXGI_FORMAT_R32G32B32_TYPELESS:            return TEX_FORMAT_RGB32_TYPELESS; 
        case DXGI_FORMAT_R32G32B32_FLOAT:               return TEX_FORMAT_RGB32_FLOAT; 
        case DXGI_FORMAT_R32G32B32_UINT:                return TEX_FORMAT_RGB32_UINT; 
        case DXGI_FORMAT_R32G32B32_SINT:                return TEX_FORMAT_RGB32_SINT; 

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:         return TEX_FORMAT_RGBA16_TYPELESS;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:            return TEX_FORMAT_RGBA16_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_UNORM:            return TEX_FORMAT_RGBA16_UNORM;
        case DXGI_FORMAT_R16G16B16A16_UINT:             return TEX_FORMAT_RGBA16_UINT;
        case DXGI_FORMAT_R16G16B16A16_SNORM:            return TEX_FORMAT_RGBA16_SNORM;
        case DXGI_FORMAT_R16G16B16A16_SINT:             return TEX_FORMAT_RGBA16_SINT;

        case DXGI_FORMAT_R32G32_TYPELESS:               return TEX_FORMAT_RG32_TYPELESS; 
        case DXGI_FORMAT_R32G32_FLOAT:                  return TEX_FORMAT_RG32_FLOAT; 
        case DXGI_FORMAT_R32G32_UINT:                   return TEX_FORMAT_RG32_UINT; 
        case DXGI_FORMAT_R32G32_SINT:                   return TEX_FORMAT_RG32_SINT; 

        case DXGI_FORMAT_R32G8X24_TYPELESS:             return TEX_FORMAT_R32G8X24_TYPELESS; 
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:          return TEX_FORMAT_D32_FLOAT_S8X24_UINT; 
        case  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:     return TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:       return TEX_FORMAT_X32_TYPELESS_G8X24_UINT; 

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:          return TEX_FORMAT_RGB10A2_TYPELESS; 
        case DXGI_FORMAT_R10G10B10A2_UNORM:             return TEX_FORMAT_RGB10A2_UNORM; 
        case DXGI_FORMAT_R10G10B10A2_UINT:              return TEX_FORMAT_RGB10A2_UINT; 

        case DXGI_FORMAT_R11G11B10_FLOAT:               return TEX_FORMAT_R11G11B10_FLOAT; 

        case DXGI_FORMAT_R8G8B8A8_TYPELESS:             return TEX_FORMAT_RGBA8_TYPELESS; 
        case DXGI_FORMAT_R8G8B8A8_UNORM:                return TEX_FORMAT_RGBA8_UNORM; 
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:           return TEX_FORMAT_RGBA8_UNORM_SRGB; 
        case DXGI_FORMAT_R8G8B8A8_UINT:                 return TEX_FORMAT_RGBA8_UINT; 
        case DXGI_FORMAT_R8G8B8A8_SNORM:                return TEX_FORMAT_RGBA8_SNORM; 
        case DXGI_FORMAT_R8G8B8A8_SINT:                 return TEX_FORMAT_RGBA8_SINT; 

        case DXGI_FORMAT_R16G16_TYPELESS:               return TEX_FORMAT_RG16_TYPELESS; 
        case DXGI_FORMAT_R16G16_FLOAT:                  return TEX_FORMAT_RG16_FLOAT; 
        case DXGI_FORMAT_R16G16_UNORM:                  return TEX_FORMAT_RG16_UNORM; 
        case DXGI_FORMAT_R16G16_UINT:                   return TEX_FORMAT_RG16_UINT; 
        case DXGI_FORMAT_R16G16_SNORM:                  return TEX_FORMAT_RG16_SNORM; 
        case DXGI_FORMAT_R16G16_SINT:                   return TEX_FORMAT_RG16_SINT; 

        case DXGI_FORMAT_R32_TYPELESS:                  return TEX_FORMAT_R32_TYPELESS; 
        case DXGI_FORMAT_D32_FLOAT:                     return TEX_FORMAT_D32_FLOAT; 
        case DXGI_FORMAT_R32_FLOAT:                     return TEX_FORMAT_R32_FLOAT; 
        case DXGI_FORMAT_R32_UINT:                      return TEX_FORMAT_R32_UINT; 
        case DXGI_FORMAT_R32_SINT:                      return TEX_FORMAT_R32_SINT; 

        case DXGI_FORMAT_R24G8_TYPELESS:                return TEX_FORMAT_R24G8_TYPELESS; 
        case DXGI_FORMAT_D24_UNORM_S8_UINT:             return TEX_FORMAT_D24_UNORM_S8_UINT; 
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:         return TEX_FORMAT_R24_UNORM_X8_TYPELESS; 
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:          return TEX_FORMAT_X24_TYPELESS_G8_UINT; 

        case DXGI_FORMAT_R8G8_TYPELESS:                 return TEX_FORMAT_RG8_TYPELESS; 
        case DXGI_FORMAT_R8G8_UNORM:                    return TEX_FORMAT_RG8_UNORM; 
        case DXGI_FORMAT_R8G8_UINT:                     return TEX_FORMAT_RG8_UINT; 
        case DXGI_FORMAT_R8G8_SNORM:                    return TEX_FORMAT_RG8_SNORM; 
        case DXGI_FORMAT_R8G8_SINT:                     return TEX_FORMAT_RG8_SINT; 

        case DXGI_FORMAT_R16_TYPELESS:                  return TEX_FORMAT_R16_TYPELESS; 
        case DXGI_FORMAT_R16_FLOAT:                     return TEX_FORMAT_R16_FLOAT; 
        case DXGI_FORMAT_D16_UNORM:                     return TEX_FORMAT_D16_UNORM; 
        case DXGI_FORMAT_R16_UNORM:                     return TEX_FORMAT_R16_UNORM; 
        case DXGI_FORMAT_R16_UINT:                      return TEX_FORMAT_R16_UINT; 
        case DXGI_FORMAT_R16_SNORM:                     return TEX_FORMAT_R16_SNORM; 
        case DXGI_FORMAT_R16_SINT:                      return TEX_FORMAT_R16_SINT; 

        case DXGI_FORMAT_R8_TYPELESS:                   return TEX_FORMAT_R8_TYPELESS; 
        case DXGI_FORMAT_R8_UNORM:                      return TEX_FORMAT_R8_UNORM; 
        case DXGI_FORMAT_R8_UINT:                       return TEX_FORMAT_R8_UINT; 
        case DXGI_FORMAT_R8_SNORM:                      return TEX_FORMAT_R8_SNORM; 
        case DXGI_FORMAT_R8_SINT:                       return TEX_FORMAT_R8_SINT; 
        case DXGI_FORMAT_A8_UNORM:                      return TEX_FORMAT_A8_UNORM; 

        case DXGI_FORMAT_R1_UNORM :                     return TEX_FORMAT_R1_UNORM; 
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:            return TEX_FORMAT_RGB9E5_SHAREDEXP; 
        case DXGI_FORMAT_R8G8_B8G8_UNORM:               return TEX_FORMAT_RG8_B8G8_UNORM; 
        case DXGI_FORMAT_G8R8_G8B8_UNORM:               return TEX_FORMAT_G8R8_G8B8_UNORM; 

        case DXGI_FORMAT_BC1_TYPELESS:                  return TEX_FORMAT_BC1_TYPELESS; 
        case DXGI_FORMAT_BC1_UNORM:                     return TEX_FORMAT_BC1_UNORM; 
        case DXGI_FORMAT_BC1_UNORM_SRGB:                return TEX_FORMAT_BC1_UNORM_SRGB; 
        case DXGI_FORMAT_BC2_TYPELESS:                  return TEX_FORMAT_BC2_TYPELESS; 
        case DXGI_FORMAT_BC2_UNORM:                     return TEX_FORMAT_BC2_UNORM; 
        case DXGI_FORMAT_BC2_UNORM_SRGB:                return TEX_FORMAT_BC2_UNORM_SRGB; 
        case DXGI_FORMAT_BC3_TYPELESS:                  return TEX_FORMAT_BC3_TYPELESS; 
        case DXGI_FORMAT_BC3_UNORM:                     return TEX_FORMAT_BC3_UNORM; 
        case DXGI_FORMAT_BC3_UNORM_SRGB:                return TEX_FORMAT_BC3_UNORM_SRGB; 
        case DXGI_FORMAT_BC4_TYPELESS:                  return TEX_FORMAT_BC4_TYPELESS; 
        case DXGI_FORMAT_BC4_UNORM:                     return TEX_FORMAT_BC4_UNORM; 
        case DXGI_FORMAT_BC4_SNORM:                     return TEX_FORMAT_BC4_SNORM; 
        case DXGI_FORMAT_BC5_TYPELESS:                  return TEX_FORMAT_BC5_TYPELESS; 
        case DXGI_FORMAT_BC5_UNORM:                     return TEX_FORMAT_BC5_UNORM; 
        case DXGI_FORMAT_BC5_SNORM:                     return TEX_FORMAT_BC5_SNORM; 

        case DXGI_FORMAT_B5G6R5_UNORM:                  return TEX_FORMAT_B5G6R5_UNORM; 
        case DXGI_FORMAT_B5G5R5A1_UNORM:                return TEX_FORMAT_B5G5R5A1_UNORM; 
        case DXGI_FORMAT_B8G8R8A8_UNORM:                return TEX_FORMAT_BGRA8_UNORM; 
        case DXGI_FORMAT_B8G8R8X8_UNORM:                return TEX_FORMAT_BGRX8_UNORM; 

        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:    return TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

        case DXGI_FORMAT_B8G8R8A8_TYPELESS:             return TEX_FORMAT_BGRA8_TYPELESS; 
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:           return TEX_FORMAT_BGRA8_UNORM_SRGB; 
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:             return TEX_FORMAT_BGRX8_TYPELESS; 
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:           return TEX_FORMAT_BGRX8_UNORM_SRGB; 

        case DXGI_FORMAT_BC6H_TYPELESS:                 return TEX_FORMAT_BC6H_TYPELESS; 
        case DXGI_FORMAT_BC6H_UF16:                     return TEX_FORMAT_BC6H_UF16; 
        case DXGI_FORMAT_BC6H_SF16:                     return TEX_FORMAT_BC6H_SF16; 
        case DXGI_FORMAT_BC7_TYPELESS :                 return TEX_FORMAT_BC7_TYPELESS; 
        case DXGI_FORMAT_BC7_UNORM:                     return TEX_FORMAT_BC7_UNORM; 
        case DXGI_FORMAT_BC7_UNORM_SRGB:                return TEX_FORMAT_BC7_UNORM_SRGB; 

        default:                                        return TEX_FORMAT_UNKNOWN;
    }
}

#define ISBITMASK(r, g, b, a) \
    (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a)

static DXGI_FORMAT get_dxgi_format(const DDS_PIXELFORMAT& ddpf)
{
    if(ddpf.flags & DDS_RGB)
    {
        switch(ddpf.RGBBitCount)
        {
        case 32:
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
            {
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
            {
                return DXGI_FORMAT_B8G8R8X8_UNORM;
            }

            if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            }

            if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
            {
                return DXGI_FORMAT_R16G16_UNORM;
            }

            if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
            {
                // Only 32-bit color channel format in D3D9 was R32F
                return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
            }
            break;
        case 24:
            break;
        
        case 16:
            if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
            {
                return DXGI_FORMAT_B5G5R5A1_UNORM;
            }
            if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
            {
                return DXGI_FORMAT_B5G6R5_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x0000) aka D3DFMT_X1R5G5B5
            if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
            {
                return DXGI_FORMAT_B4G4R4A4_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x0f00, 0x00f0, 0x000f, 0x0000) aka D3DFMT_X4R4G4B4

            // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
            break;
        }
    }
    else if(ddpf.flags & DDS_ALPHA)
    {
        if(ddpf.RGBBitCount == 8)
        {
            return DXGI_FORMAT_A8_UNORM;
        }
    }
    else if(ddpf.flags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC1_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC3_UNORM;
        }

        // While pre-mulitplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC3_UNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_SNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
        {
            return DXGI_FORMAT_R8G8_B8G8_UNORM;
        }
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
        {
            return DXGI_FORMAT_G8R8_G8B8_UNORM;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf.fourCC)
        {
        case 36: // D3DFMT_A16B16G16R16
            return DXGI_FORMAT_R16G16B16A16_UNORM;

        case 110: // D3DFMT_Q16W16V16U16
            return DXGI_FORMAT_R16G16B16A16_SNORM;

        case 111: // D3DFMT_R16F
            return DXGI_FORMAT_R16_FLOAT;

        case 112: // D3DFMT_G16R16F
            return DXGI_FORMAT_R16G16_FLOAT;

        case 113: // D3DFMT_A16B16G16R16F
            return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case 114: // D3DFMT_R32F
            return DXGI_FORMAT_R32_FLOAT;

        case 115: // D3DFMT_G32R32F
            return DXGI_FORMAT_R32G32_FLOAT;

        case 116: // D3DFMT_A32B32G32R32F
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    return DXGI_FORMAT_UNKNOWN;
}

static void get_surface_info(size_t width, size_t height, DXGI_FORMAT fmt, size_t* out_num_bytes, size_t* out_row_bytes, size_t* out_num_rows)
{
    size_t num_bytes = 0;
    size_t row_bytes = 0;
    size_t num_rows = 0;

    bool bc = false;
    bool packed = false;
    size_t bc_num_bytes_per_block = 0;
    switch(fmt)
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        bc = true;
        bc_num_bytes_per_block = 8;
        break;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bc_num_bytes_per_block = 16;
        break;

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
        packed = true;
        break;
    default: break;
    }

    if(bc)
    {
        size_t num_blocks_wide = 0;
        if(width > 0)
        {
            num_blocks_wide = std::max<size_t>(1, (width + 3) / 4);
        }
        size_t num_blocks_high = 0;
        if(height > 0)
        {
            num_blocks_high = std::max<size_t>(1, (height + 3) / 4);
        }

        row_bytes = num_blocks_wide * bc_num_bytes_per_block;
        num_rows = num_blocks_high;
    }
    else if(packed)
    {
        row_bytes = ((width + 1) >> 1) * 4; // 2 pixels per 4 bytes
        num_rows = height;
    }
    else
    {
        size_t bpp = bits_per_pixel(fmt);
        row_bytes = (width * bpp + 7) / 8; // round up to nearest byte
        num_rows = height;
    }

    num_bytes = row_bytes * num_rows;
    if(out_num_bytes)
    {
        *out_num_bytes = num_bytes;
    }
    if(out_row_bytes)
    {
        *out_row_bytes = row_bytes;
    }
    if(out_num_rows)
    {
        *out_num_rows = num_rows;
    }
}

static void fill_data(uint32_t width, uint32_t height, uint32_t depth, uint32_t src_mip_count, uint32_t dst_mip_count, uint32_t array_size, DXGI_FORMAT format, size_t bit_size, const uint8_t* bit_data, RenderObject::TextureSubResData** init_data)
{
    const uint8_t* src_bits = bit_data;
    const uint8_t* src_bits_end = bit_data + bit_size;

    size_t index = 0;
    for(size_t slice = 0; slice < array_size; ++slice)
    {
        for(size_t mip = 0;mip < src_mip_count; ++mip)
        {
            const auto w = std::max(width >> mip, 1u);
            const auto h = std::max(height >> mip, 1u);
            const auto d = std::max(depth >> mip, 1u);
            
            size_t num_bytes = 0;
            size_t row_bytes = 0;
            size_t num_rows = 0;

            get_surface_info(w, h, format, &num_bytes, &row_bytes, &num_rows);

            if(mip < dst_mip_count)
            {
                init_data[index]->pData = src_bits;
                init_data[index]->stride = (uint32_t)row_bytes;
                init_data[index]->depthStride = (uint32_t)num_bytes;
                init_data[index]->srcOffset = 0;
                ++index;
            }

            src_bits += num_bytes * d;
            if(src_bits > src_bits_end)
            {
                cyber_error("DDS data exceeds the provided size.");
                return;
            }
        }
    }

    if(!index)
    {
        cyber_error("DDS file is missing mipmap data.");
    }
};

void TextureLoaderImpl::load_from_dds(const TextureLoadInfo& texLoadInfo, const uint8_t* data, size_t dataSize)
{
    // validate dds file in memory
    if(dataSize < sizeof(uint32_t) + sizeof(DDS_HEADER))
    {
        cyber_error("DDS file is too small to be valid.");
        return;
    }

    uint32_t magic_number = *(const uint32_t*)data;
    if(magic_number != DDS_MAGIC)
    {
        cyber_error("Invalid DDS magic number: {0}", magic_number);
        return;
    }

    const auto* header = reinterpret_cast<const DDS_HEADER*>(data + sizeof(uint32_t));

    // verify header to validate dds file
    if(header->size != sizeof(DDS_HEADER) || header->ddspf.size != sizeof(DDS_PIXELFORMAT))
    {
        cyber_error("Invalid DDS header size.");
        return;
    }

    // check for DX10 extension
    bool dxt10_ext = false;
    if((header->ddspf.flags & DDS_FOURCC) && header->ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '1') == header->ddspf.fourCC)
    {
        if(dataSize < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10)))
        {
            cyber_error("DDS file is too small to contain DXT10 header.");
            return;
        }

        dxt10_ext = true;
    }

    m_textureCreateDesc.m_width = header->width;
    m_textureCreateDesc.m_height = header->height;
    uint32_t depth = header->depth;
    uint32_t array_size = 1;

    uint64_t sub_res_data_offset = sizeof(uint32_t) + sizeof(DDS_HEADER) + (dxt10_ext ? sizeof(DDS_HEADER_DXT10) : 0);

    bool is_cubemap = false;
    uint32_t res_dimension = RESOURCE_DIMENSION_UNKNOWN;
    DXGI_FORMAT dxgi_format = DXGI_FORMAT_UNKNOWN;

    const auto src_mip_count = std::max(header->mipMapCount, 1u);
    m_textureCreateDesc.m_mipLevels = src_mip_count;
    if(texLoadInfo.mipLevels > 0)
    {
        m_textureCreateDesc.m_mipLevels = std::min(texLoadInfo.mipLevels, src_mip_count);
    }

    if(false)
    {

    }
    else
    {
        dxgi_format = get_dxgi_format(header->ddspf);
        if(dxgi_format == DXGI_FORMAT_UNKNOWN)
        {
            cyber_error("Unsupported DDS format: {0}", header->ddspf.fourCC);
            return;
        }

        if(header->flags & DDS_HEADER_FLAGS_VOLUME)
        {
            res_dimension = RESOURCE_DIMENSION_TEXTURE3D;
        }
        else 
        {
            if(header->caps & DDS_CUBEMAP)
            {
                // required all six faces
                if((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
                {
                    cyber_error("DDS cubemap is missing some faces.");
                    return;
                }

                array_size = 6;
                is_cubemap = true;
            }

            res_dimension = RESOURCE_DIMENSION_TEXTURE2D;
        }

        switch (res_dimension)
        {
            case RESOURCE_DIMENSION_TEXTURE1D:
            {
                m_textureCreateDesc.m_arraySize = array_size;
                m_textureCreateDesc.m_height = 1;
                m_textureCreateDesc.m_dimension = array_size > 1 ? TEXTURE_DIMENSION::TEX_DIMENSION_1D_ARRAY : TEXTURE_DIMENSION::TEX_DIMENSION_1D;
                break;
            }
            case RESOURCE_DIMENSION_TEXTURE2D:
            {
                m_textureCreateDesc.m_arraySize = array_size;
                m_textureCreateDesc.m_dimension = is_cubemap ? (array_size > 6 ? TEXTURE_DIMENSION::TEX_DIMENSION_CUBE_ARRAY : TEXTURE_DIMENSION::TEX_DIMENSION_CUBE) 
                                                             : (array_size > 1 ? TEXTURE_DIMENSION::TEX_DIMENSION_2D_ARRAY : TEXTURE_DIMENSION::TEX_DIMENSION_2D);
                break;
            }
            case RESOURCE_DIMENSION_TEXTURE3D:
            {
                m_textureCreateDesc.m_arraySize = 1;
                m_textureCreateDesc.m_dimension = TEXTURE_DIMENSION::TEX_DIMENSION_3D;
                break;
            }
        }
        m_textureCreateDesc.m_format = DXGI_FORMAT_TO_TEXFMT(dxgi_format);

        m_textureSubResData.resize(size_t(array_size) * size_t(m_textureCreateDesc.m_mipLevels));
        auto* sub_res_data = m_textureSubResData.data();
        // fill data
        fill_data(m_textureCreateDesc.m_width, m_textureCreateDesc.m_height, depth, src_mip_count,
            m_textureCreateDesc.m_mipLevels, array_size, dxgi_format, dataSize - sub_res_data_offset, data + sub_res_data_offset, &sub_res_data);
    }

}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE