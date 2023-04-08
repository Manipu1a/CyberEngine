#pragma once
#include "rhi/backend/d3d12/rhi_d3d12.h"

namespace Cyber
{
    struct DescriptorHeapProperties
    {
        uint32_t mMaxDescriptors;
        D3D12_DESCRIPTOR_HEAP_FLAGS mFlags;
    };

    static const DescriptorHeapProperties gCpuDescriptorHeapProerties[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {
        {1024 * 256, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},  //CBV SRV UAV
        {2048, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},        // Sampler
        {1024*64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},     // Rtv
        {1024*64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE}      // Dsv
    };
    
    static DXGI_FORMAT DXGIUtil_TranslatePixelFormat(const ERHIFormat fmt, bool ShaderResource = false)
    {
        switch(fmt)
        {
            case RHI_FORMAT_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
            case RHI_FORMAT_R5G6B5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
            case RHI_FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
            case RHI_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
            case RHI_FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
            case RHI_FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
            case RHI_FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
            case RHI_FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
            case RHI_FORMAT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
            case RHI_FORMAT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
            case RHI_FORMAT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
            case RHI_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
            case RHI_FORMAT_B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM; 

            case RHI_FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case RHI_FORMAT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
            case RHI_FORMAT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
            case RHI_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
            case RHI_FORMAT_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            case RHI_FORMAT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case RHI_FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
            case RHI_FORMAT_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

            case RHI_FORMAT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
	        case RHI_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;

            case RHI_FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
            case RHI_FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
            case RHI_FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
            case RHI_FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
            case RHI_FORMAT_R16_SFLOAT: return DXGI_FORMAT_R16_FLOAT;
            case RHI_FORMAT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
            case RHI_FORMAT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
            case RHI_FORMAT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
            case RHI_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
            case RHI_FORMAT_R16G16_SFLOAT: return DXGI_FORMAT_R16G16_FLOAT;
            case RHI_FORMAT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case RHI_FORMAT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
            case RHI_FORMAT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
            case RHI_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
            case RHI_FORMAT_R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case RHI_FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
            case RHI_FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
            case RHI_FORMAT_R32_SFLOAT: return DXGI_FORMAT_R32_FLOAT;
            case RHI_FORMAT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
            case RHI_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
            case RHI_FORMAT_R32G32_SFLOAT: return DXGI_FORMAT_R32G32_FLOAT;
            case RHI_FORMAT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
            case RHI_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
            case RHI_FORMAT_R32G32B32_SFLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
            case RHI_FORMAT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
            case RHI_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
            case RHI_FORMAT_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case RHI_FORMAT_B10G11R11_UFLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
            case RHI_FORMAT_E5B9G9R9_UFLOAT: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case RHI_FORMAT_D16_UNORM: return ShaderResource ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_D16_UNORM;
            case RHI_FORMAT_X8_D24_UNORM: return ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
            case RHI_FORMAT_D32_SFLOAT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT :DXGI_FORMAT_D32_FLOAT;
            case RHI_FORMAT_D24_UNORM_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS :DXGI_FORMAT_D24_UNORM_S8_UINT;
            case RHI_FORMAT_D32_SFLOAT_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS :DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case RHI_FORMAT_DXBC1_RGB_UNORM: return DXGI_FORMAT_BC1_UNORM;
            case RHI_FORMAT_DXBC1_RGB_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
            case RHI_FORMAT_DXBC1_RGBA_UNORM: return DXGI_FORMAT_BC1_UNORM;
            case RHI_FORMAT_DXBC1_RGBA_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
            case RHI_FORMAT_DXBC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
            case RHI_FORMAT_DXBC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
            case RHI_FORMAT_DXBC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
            case RHI_FORMAT_DXBC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
            case RHI_FORMAT_DXBC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
            case RHI_FORMAT_DXBC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
            case RHI_FORMAT_DXBC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
            case RHI_FORMAT_DXBC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
            case RHI_FORMAT_DXBC6H_UFLOAT: return DXGI_FORMAT_BC6H_UF16;
            case RHI_FORMAT_DXBC6H_SFLOAT: return DXGI_FORMAT_BC6H_SF16;
            case RHI_FORMAT_DXBC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
            case RHI_FORMAT_DXBC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

            case RHI_FORMAT_D16_UNORM_S8_UINT:
            case RHI_FORMAT_R4G4_UNORM: 
            default: return DXGI_FORMAT_UNKNOWN;
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    static DXGI_FORMAT DXGIUtil_FormatToTypeless(DXGI_FORMAT fmt)
    {
        switch (fmt) {
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_UINT:
            case DXGI_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case DXGI_FORMAT_R32G32B32_FLOAT:
            case DXGI_FORMAT_R32G32B32_UINT:
            case DXGI_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_TYPELESS;

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R16G16B16A16_UNORM:
            case DXGI_FORMAT_R16G16B16A16_UINT:
            case DXGI_FORMAT_R16G16B16A16_SNORM:
            case DXGI_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;

            case DXGI_FORMAT_R32G32_FLOAT:
            case DXGI_FORMAT_R32G32_UINT:
            case DXGI_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_TYPELESS;

            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_TYPELESS;

            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case DXGI_FORMAT_R16G16_FLOAT:
            case DXGI_FORMAT_R16G16_UNORM:
            case DXGI_FORMAT_R16G16_UINT:
            case DXGI_FORMAT_R16G16_SNORM:
            case DXGI_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_TYPELESS;

            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R32_FLOAT:
            case DXGI_FORMAT_R32_UINT:
            case DXGI_FORMAT_R32_SINT: return DXGI_FORMAT_R32_TYPELESS;
            case DXGI_FORMAT_R8G8_UNORM:
            case DXGI_FORMAT_R8G8_UINT:
            case DXGI_FORMAT_R8G8_SNORM:
            case DXGI_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_TYPELESS;
            case DXGI_FORMAT_B4G4R4A4_UNORM: // just treats a 16 raw bits
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_R16_FLOAT:
            case DXGI_FORMAT_R16_UNORM:
            case DXGI_FORMAT_R16_UINT:
            case DXGI_FORMAT_R16_SNORM:
            case DXGI_FORMAT_R16_SINT: return DXGI_FORMAT_R16_TYPELESS;
            case DXGI_FORMAT_A8_UNORM:
            case DXGI_FORMAT_R8_UNORM:
            case DXGI_FORMAT_R8_UINT:
            case DXGI_FORMAT_R8_SNORM:
            case DXGI_FORMAT_R8_SINT: return DXGI_FORMAT_R8_TYPELESS;
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_TYPELESS;
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_TYPELESS;
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_TYPELESS;
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_TYPELESS;
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_TYPELESS;
            case DXGI_FORMAT_B5G6R5_UNORM:
            case DXGI_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_R16_TYPELESS;

            case DXGI_FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;

            case DXGI_FORMAT_B8G8R8X8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_TYPELESS;

            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;

            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_TYPELESS;

            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_TYPELESS;

            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
            case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;

                // typeless just return the input format
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            case DXGI_FORMAT_R32G32B32_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            case DXGI_FORMAT_R32G32_TYPELESS:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R16G16_TYPELESS:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R16_TYPELESS:
            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC7_TYPELESS: return fmt;

            case DXGI_FORMAT_R1_UNORM:
            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
            case DXGI_FORMAT_AYUV:
            case DXGI_FORMAT_Y410:
            case DXGI_FORMAT_Y416:
            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
            case DXGI_FORMAT_420_OPAQUE:
            case DXGI_FORMAT_YUY2:
            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
            case DXGI_FORMAT_NV11:
            case DXGI_FORMAT_AI44:
            case DXGI_FORMAT_IA44:
            case DXGI_FORMAT_P8:
            case DXGI_FORMAT_A8P8:
            case DXGI_FORMAT_P208:
            case DXGI_FORMAT_V208:
            case DXGI_FORMAT_V408:
            case DXGI_FORMAT_UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            }
            return DXGI_FORMAT_UNKNOWN;
    }

    static D3D12_RESOURCE_STATES D3D12Util_TranslateResourceState(RHIResourceState state)
    {
        D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

        // These states cannot be combined with other states so we just do an == check
        if (state == RHI_RESOURCE_STATE_GENERIC_READ)
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        if (state == RHI_RESOURCE_STATE_COMMON)
            return D3D12_RESOURCE_STATE_COMMON;
        if (state == RHI_RESOURCE_STATE_PRESENT)
            return D3D12_RESOURCE_STATE_PRESENT;

        if (state & RHI_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (state & RHI_RESOURCE_STATE_INDEX_BUFFER)
            ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (state & RHI_RESOURCE_STATE_RENDER_TARGET)
            ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (state & RHI_RESOURCE_STATE_UNORDERED_ACCESS)
            ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (state & RHI_RESOURCE_STATE_DEPTH_WRITE)
            ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (state & RHI_RESOURCE_STATE_DEPTH_READ)
            ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (state & RHI_RESOURCE_STATE_STREAM_OUT)
            ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
        if (state & RHI_RESOURCE_STATE_INDIRECT_ARGUMENT)
            ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (state & RHI_RESOURCE_STATE_COPY_DEST)
            ret |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (state & RHI_RESOURCE_STATE_COPY_SOURCE)
            ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (state & RHI_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
            ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (state & RHI_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
            ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    #ifdef D3D12_RAYTRACING_AVAILABLE
        if (state & RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
            ret |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    #endif

    #ifdef ENABLE_VRS
        if (state & RESOURCE_STATE_SHADING_RATE_SOURCE)
            ret |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
    #endif

        return ret;
    }

    static D3D12_DESCRIPTOR_RANGE_TYPE D3D12Util_ResourceTypeToDescriptorRangeType(ERHIResourceType type)
    {
        switch (type)
        {
        case RHI_RESOURCE_TYPE_UNIFORM_BUFFER:
        case RHI_RESOURCE_TYPE_PUSH_CONTANT:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case RHI_RESOURCE_TYPE_RW_BUFFER:
        case RHI_RESOURCE_TYPE_RW_TEXTURE:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        case RHI_RESOURCE_TYPE_SAMPLER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        case RHI_RESOURCE_TYPE_RAY_TRACING:
        case RHI_RESOURCE_TYPE_TEXTURE:
        case RHI_RESOURCE_TYPE_BUFFER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        default:
            cyber_assert(false, "Invalid DescriptorInfo Type");
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }

    static D3D12_SHADER_VISIBILITY D3D12Util_TranslateShaderStage(ERHIShaderStages stages)
    {
        D3D12_SHADER_VISIBILITY res = D3D12_SHADER_VISIBILITY_ALL;
        uint32_t stageCount = 0;
        if(stages == RHI_SHADER_STAGE_COMPUTE)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        if(stages = RHI_SHADER_STAGE_RAYTRACING)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        if(stages & RHI_SHADER_STAGE_VERT)
        {
            res = D3D12_SHADER_VISIBILITY_VERTEX;
            stageCount++;
        }
        if(stages & RHI_SHADER_STAGE_GEOM)
        {
            res = D3D12_SHADER_VISIBILITY_GEOMETRY;
            stageCount++;
        }
        if(stages & RHI_SHADER_STAGE_HULL)
        {
            res = D3D12_SHADER_VISIBILITY_HULL;
            stageCount++;
        }
        if(stages & RHI_SHADER_STAGE_DOMAIN)
        {
            res = D3D12_SHADER_VISIBILITY_DOMAIN;
            stageCount++;
        }
        if(stages & RHI_SHADER_STAGE_FRAG)
        {
            res = D3D12_SHADER_VISIBILITY_PIXEL;
            stageCount++;
        }
        cyber_assert(stageCount > 0, "Invalid Shader Stage");
        return stageCount > 1 ? D3D12_SHADER_VISIBILITY_ALL : res;
    }

    static const D3D12_BLEND_OP gDx12BlendOpTranlator[RHI_BLEND_MODE_COUNT] = 
    {
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_OP_SUBTRACT,
        D3D12_BLEND_OP_REV_SUBTRACT,
        D3D12_BLEND_OP_MIN,
        D3D12_BLEND_OP_MAX,
    };

    static const D3D12_BLEND gDx12BlendConstantTranslator[RHI_BLEND_CONST_COUNT] = 
    {
        D3D12_BLEND_ZERO,
        D3D12_BLEND_ONE,
        D3D12_BLEND_SRC_COLOR,
        D3D12_BLEND_INV_SRC_COLOR,
        D3D12_BLEND_DEST_COLOR,
        D3D12_BLEND_INV_DEST_COLOR,
        D3D12_BLEND_SRC_ALPHA,
        D3D12_BLEND_INV_SRC_ALPHA,
        D3D12_BLEND_DEST_ALPHA,
        D3D12_BLEND_INV_DEST_ALPHA,
        D3D12_BLEND_SRC_ALPHA_SAT,
        D3D12_BLEND_BLEND_FACTOR,
        D3D12_BLEND_INV_BLEND_FACTOR,
    };
    static const D3D12_STENCIL_OP gDx12StencilOpTranslator[RHI_STENCIL_OP_COUNT] = 
    {
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_ZERO,
        D3D12_STENCIL_OP_REPLACE,
        D3D12_STENCIL_OP_INVERT,
        D3D12_STENCIL_OP_INCR,
        D3D12_STENCIL_OP_DECR,
        D3D12_STENCIL_OP_INCR_SAT,
        D3D12_STENCIL_OP_DECR_SAT,
    };

    static const D3D12_CULL_MODE gDx12CullModeTranslator[RHI_CULL_MODE_COUNT] = 
    {
        D3D12_CULL_MODE_NONE,
        D3D12_CULL_MODE_FRONT,
        D3D12_CULL_MODE_BACK,
    };

    static const D3D12_FILL_MODE gDx12FillModeTranslator[RHI_FILL_MODE_COUNT] = 
    {
        D3D12_FILL_MODE_SOLID,
        D3D12_FILL_MODE_WIREFRAME,
    };

    static const D3D12_COMPARISON_FUNC gDx12ComparisonFuncTranslator[RHI_CMP_COUNT] = 
    {
        D3D12_COMPARISON_FUNC_NEVER,
        D3D12_COMPARISON_FUNC_LESS,
        D3D12_COMPARISON_FUNC_EQUAL,
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER,
        D3D12_COMPARISON_FUNC_NOT_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER_EQUAL,
        D3D12_COMPARISON_FUNC_ALWAYS,
    };

    #if !defined (XBOX) && defined (_WIN32)
    #include <dxcapi.h>

    void D3D12Util_LoadDxcDLL();
    void D3D12Util_UnloadDxcDLL();
    DxcCreateInstanceProc D3D12Util_GetDxcCreateInstanceProc();
    #endif

    DescriptorHandle D3D12Util_ConsumeDescriptorHandles(RHIDescriptorHeap_D3D12* pHeap, uint32_t descriptorCount);
    void D3D12Util_CopyDescriptorHandle(RHIDescriptorHeap_D3D12* dstHeap, const D3D12_CPU_DESCRIPTOR_HANDLE& srcHandle, const uint64_t& dstHandle, uint32_t index);

    void D3D12Util_InitializeEnvironment(RHIInstance* pInst);

    void D3D12Util_DeInitializeEnvironment(RHIInstance* pInst);

    void D3D12Util_Optionalenable_debug_layer(RHIInstance_D3D12* result, const RHIInstanceCreateDesc& instanceDesc);

    void D3D12Util_QueryAllAdapters(Ref<RHIInstance_D3D12> pInstance, uint32_t& count, bool& foundSoftwareAdapter);

    void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& pDesc, struct RHIDescriptorHeap_D3D12** ppDescHeap);

    void D3D12Util_CreateDMAAllocator(RHIInstance_D3D12* pInstance, RHIAdapter_D3D12* pAdapter, RHIDevice_D3D12* pDevice);

    void D3D12Util_InitializeShaderReflection(ID3D12Device* device, RHIShaderLibrary_D3D12* library, const RHIShaderLibraryCreateDesc& desc);
    
    void D3D12Util_CreateCBV(RHIDevice_D3D12* pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pCbvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

    void D3D12Util_CreateSRV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

    void D3D12Util_CreateUAV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

    void D3D12Util_CreateRTV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

    void D3D12Util_CreateDSV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

    D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const RHIBlendStateCreateDesc* pDesc);

    D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const RHIRasterizerStateCreateDesc* pDesc);

    D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDepthStencilState(const RHIDepthStateCreateDesc* pDesc);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopologyType(ERHIPrimitiveTopology topology);
}