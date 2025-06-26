#pragma once
#include "interface/graphics_types.h"
#include "EASTL/map.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdint.h>

namespace Cyber
{
    typedef int32_t DxDescriptorID;
    #define D3D12_DESCRIPTOR_ID_NONE (D3D12_CPU_DESCRIPTOR_HANDLE{(size_t)~0})

    #define D3D12_GPU_VIRTUAL_ADDRESS_NULL ((D3D12_GPU_VIRTUAL_ADDRESS)0)
    #define D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

    #define MAX_SRVS 64
    #define MAX_SAMPLERS 32
    #define MAX_UAVS 16
    #define MAX_CBVS 16
    #define MAX_CBS 16

    struct CD3DX12_DEFAULT {};
    extern const DECLSPEC_SELECTANY CD3DX12_DEFAULT D3D12_DEFAULT;

    struct CD3D12_BLEND_DESC : public D3D12_BLEND_DESC
    {
        CD3D12_BLEND_DESC() {}

        explicit CD3D12_BLEND_DESC( const D3D12_BLEND_DESC& o ) :
            D3D12_BLEND_DESC( o )
        {}

        explicit CD3D12_BLEND_DESC(CD3DX12_DEFAULT)
        {
            AlphaToCoverageEnable = false;
            IndependentBlendEnable = false;

            const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
            {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
            };

            for(uint32_t i = 0;i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            {
                RenderTarget[i] = defaultRenderTargetBlendDesc;
            }
        }

        ~CD3D12_BLEND_DESC() {}
    };

    struct CD3D12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC
    {
        CD3D12_RASTERIZER_DESC() {}

        explicit CD3D12_RASTERIZER_DESC( const D3D12_RASTERIZER_DESC& o ) :
            D3D12_RASTERIZER_DESC( o )
        {}

        explicit CD3D12_RASTERIZER_DESC(CD3DX12_DEFAULT)
        {
            FillMode = D3D12_FILL_MODE_SOLID;
            CullMode = D3D12_CULL_MODE_BACK;
            FrontCounterClockwise = FALSE;
            DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            DepthClipEnable = TRUE;
            MultisampleEnable = FALSE;
            AntialiasedLineEnable = FALSE;
            ForcedSampleCount = 0;
            ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        explicit CD3D12_RASTERIZER_DESC(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode, BOOL frontCounterClockwise, INT depthBias, FLOAT depthBiasClamp,
                                        FLOAT slopeScaledDepthBias, BOOL depthClipEnable, BOOL multisampleEnable, BOOL antialiasedLineEnable, 
                                        UINT forcedSampleCount, D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster)
        {
            FillMode = fillMode;
            CullMode = cullMode;
            FrontCounterClockwise = frontCounterClockwise;
            DepthBias = depthBias;
            DepthBiasClamp = depthBiasClamp;
            SlopeScaledDepthBias = slopeScaledDepthBias;
            DepthClipEnable = depthClipEnable;
            MultisampleEnable = multisampleEnable;
            AntialiasedLineEnable = antialiasedLineEnable;
            ForcedSampleCount = forcedSampleCount;
            ConservativeRaster = conservativeRaster;
        }

        ~CD3D12_RASTERIZER_DESC() {}
    };
    
    static const D3D_FEATURE_LEVEL d3d_feature_levels[] = 
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    
    
    enum ShaderVisibility
    {
        SV_VERTEX,
        SV_PIXEL,
        SV_GEOMETRY,
        SV_MESH,
        SV_ALL,
        SV_SHADERVISIBILITY_COUNT
    };

}