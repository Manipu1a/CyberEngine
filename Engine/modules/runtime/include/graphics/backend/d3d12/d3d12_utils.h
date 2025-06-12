#pragma once
#include "graphics/backend/d3d12/graphics_types_d3d12.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include <dxcapi.h>

namespace Cyber
{
    namespace RenderObject
    {
        class ShaderLibrary_D3D12_Impl;
        class RenderDevice_D3D12_Impl;
        class Queue_D3D12_Impl;
        class Adapter_D3D12_Impl;
    };

    #define CALC_SUBRESOURCE_INDEX(MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize) (MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize)

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
    
    static DXGI_FORMAT DXGIUtil_TranslatePixelFormat(const TEXTURE_FORMAT fmt, bool ShaderResource = false)
    {
        switch(fmt)
        {
            case TEX_FORMAT_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
            case TEX_FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM; 
            case TEX_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
            case TEX_FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
            case TEX_FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
            case TEX_FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
            case TEX_FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
            case TEX_FORMAT_RG8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
            case TEX_FORMAT_RG8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
            case TEX_FORMAT_RG8_UINT: return DXGI_FORMAT_R8G8_UINT;
            case TEX_FORMAT_RG8_SINT: return DXGI_FORMAT_R8G8_SINT;
            case TEX_FORMAT_BGRA4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;

            case TEX_FORMAT_RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TEX_FORMAT_RGBA8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
            case TEX_FORMAT_RGBA8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
            case TEX_FORMAT_RGBA8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
            case TEX_FORMAT_RGBA8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            case TEX_FORMAT_BGRA8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case TEX_FORMAT_BGRX8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
            case TEX_FORMAT_BGRA8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

            case TEX_FORMAT_RGB10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM; 
	        case TEX_FORMAT_RGB10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;

            case TEX_FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
            case TEX_FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
            case TEX_FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
            case TEX_FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
            case TEX_FORMAT_R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
            case TEX_FORMAT_RG16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
            case TEX_FORMAT_RG16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
            case TEX_FORMAT_RG16_UINT: return DXGI_FORMAT_R16G16_UINT;
            case TEX_FORMAT_RG16_SINT: return DXGI_FORMAT_R16G16_SINT;
            case TEX_FORMAT_RG16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
            case TEX_FORMAT_RGBA16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case TEX_FORMAT_RGBA16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
            case TEX_FORMAT_RGBA16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
            case TEX_FORMAT_RGBA16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
            case TEX_FORMAT_RGBA16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TEX_FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
            case TEX_FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
            case TEX_FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
            case TEX_FORMAT_D32_FLOAT: return  DXGI_FORMAT_D32_FLOAT;
            case TEX_FORMAT_RG32_UINT: return DXGI_FORMAT_R32G32_UINT;
            case TEX_FORMAT_RG32_SINT: return DXGI_FORMAT_R32G32_SINT;
            case TEX_FORMAT_RG32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
            case TEX_FORMAT_RGB32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
            case TEX_FORMAT_RGB32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
            case TEX_FORMAT_RGB32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
            case TEX_FORMAT_RGBA32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
            case TEX_FORMAT_RGBA32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
            case TEX_FORMAT_RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TEX_FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT; 
            case TEX_FORMAT_RGB9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case TEX_FORMAT_D16_UNORM: return DXGI_FORMAT_D16_UNORM;
            case TEX_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case TEX_FORMAT_R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case TEX_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            case TEX_FORMAT_BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
            case TEX_FORMAT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
            case TEX_FORMAT_BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
            case TEX_FORMAT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
            case TEX_FORMAT_BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
            case TEX_FORMAT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
            case TEX_FORMAT_BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
            case TEX_FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
            case TEX_FORMAT_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
            case TEX_FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
            case TEX_FORMAT_BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
            case TEX_FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
            case TEX_FORMAT_BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
            case TEX_FORMAT_BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
            default: return DXGI_FORMAT_UNKNOWN;
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    static TEXTURE_DIMENSION DXGIUtil_TranslateSRVDimension(D3D_SRV_DIMENSION dimension)
    {
        switch (dimension) {
            case D3D_SRV_DIMENSION_UNKNOWN: return TEX_DIMENSION_UNDEFINED;
            case D3D_SRV_DIMENSION_TEXTURE1D: return TEX_DIMENSION_1D;
            case D3D_SRV_DIMENSION_TEXTURE1DARRAY: return TEX_DIMENSION_1D_ARRAY;
            case D3D_SRV_DIMENSION_TEXTURE2D: return TEX_DIMENSION_2D;
            case D3D_SRV_DIMENSION_TEXTURE2DARRAY: return TEX_DIMENSION_2D_ARRAY;
            case D3D_SRV_DIMENSION_TEXTURE3D: return TEX_DIMENSION_3D;
            case D3D_SRV_DIMENSION_TEXTURECUBE: return TEX_DIMENSION_CUBE;
            case D3D_SRV_DIMENSION_TEXTURECUBEARRAY: return TEX_DIMENSION_CUBE_ARRAY;
            default: return TEX_DIMENSION_UNDEFINED;
        }
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

    static D3D12_COMMAND_LIST_TYPE D3D12Util_TranslateCommandQueueType(COMMAND_QUEUE_TYPE type)
    {
        switch(type)
        {
            case COMMAND_QUEUE_TYPE_GRAPHICS: return D3D12_COMMAND_LIST_TYPE_DIRECT;
            case COMMAND_QUEUE_TYPE_COMPUTE: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case COMMAND_QUEUE_TYPE_TRANSFER: return D3D12_COMMAND_LIST_TYPE_COPY;
            default: cyber_assert(false, "Unsupported command queue type"); return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    static DXGI_FORMAT D3D12Util_TypeToDXGI(VALUE_TYPE value_type, uint32_t num_components, bool is_normalized)
    {
        switch(value_type)
        {
            case VALUE_TYPE_FLOAT16:
            {
                cyber_assert(!is_normalized, "Floating point formats cannot be normalized");
                switch(num_components)
                {
                    case 1: return DXGI_FORMAT_R16_FLOAT;
                    case 2: return DXGI_FORMAT_R16G16_FLOAT;
                    case 4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
                    default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                }
            }

            case VALUE_TYPE_FLOAT32:
            {
                cyber_assert(!is_normalized, "Floating point formats cannot be normalized");
                switch(num_components)
                {
                    case 1: return DXGI_FORMAT_R32_FLOAT;
                    case 2: return DXGI_FORMAT_R32G32_FLOAT;
                    case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
                    case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
                    default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                }
            }

            case VALUE_TYPE_INT32:
            {
                cyber_assert(!is_normalized, "32-bit UNORM formats are not supported, Use R32_FLOAT instead");
                switch(num_components)
                {
                    case 1: return DXGI_FORMAT_R32_SINT;
                    case 2: return DXGI_FORMAT_R32G32_SINT;
                    case 3: return DXGI_FORMAT_R32G32B32_SINT;
                    case 4: return DXGI_FORMAT_R32G32B32A32_SINT;
                    default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                }
            }

            case VALUE_TYPE_INT16:
            {
                if(is_normalized)
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R16_SNORM;
                        case 2: return DXGI_FORMAT_R16G16_SNORM;
                        case 4: return DXGI_FORMAT_R16G16B16A16_SNORM;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
                else
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R16_SINT;
                        case 2: return DXGI_FORMAT_R16G16_SINT;
                        case 4: return DXGI_FORMAT_R16G16B16A16_SINT;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
            }

            case VALUE_TYPE_UINT16:
            {
                if(is_normalized)
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R16_UNORM;
                        case 2: return DXGI_FORMAT_R16G16_UNORM;
                        case 4: return DXGI_FORMAT_R16G16B16A16_UNORM;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
                else
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R16_UINT;
                        case 2: return DXGI_FORMAT_R16G16_UINT;
                        case 4: return DXGI_FORMAT_R16G16B16A16_UINT;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
            }

            case VALUE_TYPE_INT8:
            {
                if(is_normalized)
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R8_SNORM;
                        case 2: return DXGI_FORMAT_R8G8_SNORM;
                        case 4: return DXGI_FORMAT_R8G8B8A8_SNORM;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
                else
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R8_SINT;
                        case 2: return DXGI_FORMAT_R8G8_SINT;
                        case 4: return DXGI_FORMAT_R8G8B8A8_SINT;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
            }

            case VALUE_TYPE_UINT8:
            {
                if(is_normalized)
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R8_UNORM;
                        case 2: return DXGI_FORMAT_R8G8_UNORM;
                        case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
                else
                {
                    switch(num_components)
                    {
                        case 1: return DXGI_FORMAT_R8_UINT;
                        case 2: return DXGI_FORMAT_R8G8_UINT;
                        case 4: return DXGI_FORMAT_R8G8B8A8_UINT;
                        default: cyber_assert(false, "Unsupported number of components"); return DXGI_FORMAT_UNKNOWN;
                    }
                }
            }

            default: cyber_assert(false, "Unsupported value type"); return DXGI_FORMAT_UNKNOWN;
        }
    }

    static D3D12_RESOURCE_STATES D3D12Util_TranslateResourceState(GRAPHICS_RESOURCE_STATE state)
    {
        switch(state)
        {
            case GRAPHICS_RESOURCE_STATE_UNDEFINED: return D3D12_RESOURCE_STATE_COMMON;
            case GRAPHICS_RESOURCE_STATE_VERTEX_BUFFER: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            case GRAPHICS_RESOURCE_STATE_CONSTANT_BUFFER: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            case GRAPHICS_RESOURCE_STATE_INDEX_BUFFER: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
            case GRAPHICS_RESOURCE_STATE_RENDER_TARGET: return D3D12_RESOURCE_STATE_RENDER_TARGET;
            case GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case GRAPHICS_RESOURCE_STATE_DEPTH_WRITE: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
            case GRAPHICS_RESOURCE_STATE_DEPTH_READ: return D3D12_RESOURCE_STATE_DEPTH_READ;
            case GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            case GRAPHICS_RESOURCE_STATE_STREAM_OUT: return D3D12_RESOURCE_STATE_STREAM_OUT;
            case GRAPHICS_RESOURCE_STATE_INDIRECT_ARGUMENT: return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            case GRAPHICS_RESOURCE_STATE_COPY_DEST: return D3D12_RESOURCE_STATE_COPY_DEST;
            case GRAPHICS_RESOURCE_STATE_COPY_SOURCE: return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case GRAPHICS_RESOURCE_STATE_RESOLVE_DEST: return D3D12_RESOURCE_STATE_RESOLVE_DEST;
            case GRAPHICS_RESOURCE_STATE_RESOLVE_SOURCE: return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
            case GRAPHICS_RESOURCE_STATE_INPUT_ATTACHMENT: return D3D12_RESOURCE_STATE_RENDER_TARGET;
            case GRAPHICS_RESOURCE_STATE_PRESENT: return D3D12_RESOURCE_STATE_PRESENT;
            case GRAPHICS_RESOURCE_STATE_BUILD_AS_READ: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case GRAPHICS_RESOURCE_STATE_BUILD_AS_WRITE: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case GRAPHICS_RESOURCE_STATE_RAY_TRACING: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case GRAPHICS_RESOURCE_STATE_COMMON: return D3D12_RESOURCE_STATE_COMMON;
            case GRAPHICS_RESOURCE_STATE_SHADING_RATE: return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
            default: cyber_assert(false, "Invalid Resource State"); return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    static D3D12_DESCRIPTOR_RANGE_TYPE D3D12Util_ResourceTypeToDescriptorRangeType(GRAPHICS_RESOURCE_TYPE type)
    {
        switch (type)
        {
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_PUSH_CONTANT:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_RW_BUFFER:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_RW_TEXTURE:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_SAMPLER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_RAY_TRACING:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_TEXTURE:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_BUFFER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        default:
            cyber_assert(false, "Invalid DescriptorInfo Type");
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }

    static D3D12_ROOT_PARAMETER_TYPE D3D12Util_ResourceTypeToRootParameterType(GRAPHICS_RESOURCE_TYPE type)
    {
        switch (type)
        {
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_PUSH_CONTANT:
            return D3D12_ROOT_PARAMETER_TYPE_CBV;
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_RW_BUFFER:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_RW_TEXTURE:
            return D3D12_ROOT_PARAMETER_TYPE_UAV;
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_RAY_TRACING:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_TEXTURE:
        case GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_BUFFER:
            return D3D12_ROOT_PARAMETER_TYPE_SRV;
        default:
            cyber_assert(false, "Invalid DescriptorInfo Type");
            return D3D12_ROOT_PARAMETER_TYPE_SRV;
        }
    }

    static D3D12_SHADER_VISIBILITY D3D12Util_TranslateShaderStage(SHADER_STAGE stages)
    {
        D3D12_SHADER_VISIBILITY res = D3D12_SHADER_VISIBILITY_ALL;
        uint32_t stageCount = 0;
        if(stages == SHADER_STAGE::SHADER_STAGE_COMPUTE)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        if(stages = SHADER_STAGE::SHADER_STAGE_RAYTRACING)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        if(stages & SHADER_STAGE::SHADER_STAGE_VERT)
        {
            res = D3D12_SHADER_VISIBILITY_VERTEX;
            stageCount++;
        }
        if(stages & SHADER_STAGE::SHADER_STAGE_GEOM)
        {
            res = D3D12_SHADER_VISIBILITY_GEOMETRY;
            stageCount++;
        }
        if(stages & SHADER_STAGE::SHADER_STAGE_HULL)
        {
            res = D3D12_SHADER_VISIBILITY_HULL;
            stageCount++;
        }
        if(stages & SHADER_STAGE::SHADER_STAGE_DOMAIN)
        {
            res = D3D12_SHADER_VISIBILITY_DOMAIN;
            stageCount++;
        }
        if(stages & SHADER_STAGE::SHADER_STAGE_FRAG)
        {
            res = D3D12_SHADER_VISIBILITY_PIXEL;
            stageCount++;
        }
        cyber_assert(stageCount > 0, "Invalid Shader Stage");
        return stageCount > 1 ? D3D12_SHADER_VISIBILITY_ALL : res;
    }

    static const D3D12_BLEND_OP gDx12BlendOpTranlator[BLEND_MODE_COUNT] = 
    {
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_OP_SUBTRACT,
        D3D12_BLEND_OP_REV_SUBTRACT,
        D3D12_BLEND_OP_MIN,
        D3D12_BLEND_OP_MAX,
    };

    static const D3D12_COLOR_WRITE_ENABLE D3D12Util_TranslateColorMask(COLOR_WRITE_MASK mask)
    {
        switch(mask)
        {
            case COLOR_WRITE_MASK_RED: return D3D12_COLOR_WRITE_ENABLE_RED;
            case COLOR_WRITE_MASK_GREEN: return D3D12_COLOR_WRITE_ENABLE_GREEN;
            case COLOR_WRITE_MASK_BLUE: return D3D12_COLOR_WRITE_ENABLE_BLUE;
            case COLOR_WRITE_MASK_ALPHA: return D3D12_COLOR_WRITE_ENABLE_ALPHA;
            case COLOR_WRITE_MASK_ALL: return D3D12_COLOR_WRITE_ENABLE_ALL;
            default: cyber_assert(false, "Invalid Color Write Mask"); return D3D12_COLOR_WRITE_ENABLE_ALL;
        }
    }

    static const D3D12_BLEND gDx12BlendConstantTranslator[BLEND_CONSTANT_COUNT] = 
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

    static const D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE gDx12PassBeginOpTranslator[LOAD_ACTION_COUNT] = {
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD,
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE,
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR,
    };

    static const D3D12_RENDER_PASS_ENDING_ACCESS_TYPE gDx12PassEndOpTranslator[STORE_ACTION_COUNT] = {
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE,
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD,
    };

    static const D3D12_STENCIL_OP gDx12StencilOpTranslator[STENCIL_OP_COUNT] = 
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

    static const D3D12_CULL_MODE gDx12CullModeTranslator[CULL_MODE_COUNT] = 
    {
        D3D12_CULL_MODE_NONE,
        D3D12_CULL_MODE_FRONT,
        D3D12_CULL_MODE_BACK,
    };

    static const D3D12_FILL_MODE gDx12FillModeTranslator[FILL_MODE_COUNT] = 
    {
        D3D12_FILL_MODE_SOLID,
        D3D12_FILL_MODE_WIREFRAME,
    };

    static const D3D12_COMPARISON_FUNC gDx12ComparisonFuncTranslator[CMP_COUNT] = 
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

    D3D12_FILTER D3D12Util_TranslateFilter(FILTER_TYPE minFilter, FILTER_TYPE magFilter, FILTER_TYPE mipfilter);

    D3D12_TEXTURE_ADDRESS_MODE D3D12Util_TranslateAddressMode(ADDRESS_MODE mode);
    
    D3D12_COMPARISON_FUNC D3D12Util_TranslateCompareMode(COMPARE_MODE mode);
    
    D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const BlendStateCreateDesc* pDesc);

    D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const RasterizerStateCreateDesc* pDesc);

    D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDepthStencilState(const DepthStateCreateDesc* pDesc);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopologyType(PRIMITIVE_TOPOLOGY topology);

    uint32_t d3d12_command_list_type_to_queue_id(D3D12_COMMAND_LIST_TYPE type);
    D3D12_COMMAND_LIST_TYPE d3d12_queue_id_to_command_list_type(uint32_t queue_id);

    inline void MemcpySubresource(D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSrc, uint64_t RowSizeInBytes, uint32_t numRows, uint32_t numSlices)
    {
        for(uint32_t z = 0; z < numSlices; ++z)
        {
            BYTE* pDestSlice = reinterpret_cast<BYTE*>(pDest->pData) + pDest->SlicePitch * z;
            const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * z;
            for(uint32_t y = 0; y < numRows; ++y)
            {
                memcpy(pDestSlice + pDest->RowPitch * y, pSrcSlice + pSrc->RowPitch * y, RowSizeInBytes);
            }
        }
    }

    #if !defined (XBOX) && defined (_WIN32)
        struct D3D12Util_DXCLoader
        {
            void Load()
            {
                dxcLibrary = LoadLibrary(L"dxcompiler.dll");
                pDxcCreateInstance = (void*)::GetProcAddress((HMODULE)dxcLibrary, "DxcCreateInstance");
            }
            void Unload()
            {
                pDxcCreateInstance = nullptr;
                ::FreeLibrary(dxcLibrary);
            }

            DxcCreateInstanceProc Get()
            {
                return (DxcCreateInstanceProc)pDxcCreateInstance;
            }

            HMODULE dxcLibrary;
            void* pDxcCreateInstance;
            uint32_t mMajorVersion;
            uint32_t mMinorVersion;
            uint32_t shader_model_major;
            uint32_t shader_model_minor;
        };

        void d3d12_util_load_dxc_dll();
        void d3d12_util_unload_dxc_dll();
        DxcCreateInstanceProc d3d12_util_get_dxc_create_instance_proc();
    #endif

    static constexpr uint8_t d3d12_queue_index_graphics{0};
    static constexpr uint8_t d3d12_queue_index_compute{1};
    static constexpr uint8_t d3d12_queue_index_copy{2};
}