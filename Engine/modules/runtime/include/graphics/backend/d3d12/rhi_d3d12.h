#pragma once
#include "interface/rhi.h"
#include "EASTL/map.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdint.h>

namespace Cyber
{
    typedef int32_t DxDescriptorID;
    #define IID_ARGS IID_PPV_ARGS
    #define D3D12_DESCRIPTOR_ID_NONE (D3D12_CPU_DESCRIPTOR_HANDLE{(size_t)~0})

    #define D3D12_GPU_VIRTUAL_ADDRESS_NULL ((D3D12_GPU_VIRTUAL_ADDRESS)0)
    #define D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

    #ifndef SAFE_RELEASE
        #define SAFE_RELEASE(p_var) \
            if(p_var)               \
            {                       \
                p_var->Release();   \
                p_var = NULL;       \
            }
    #endif

    /************************************************************************/
    // Descriptor Heap Structures
    /************************************************************************/

    struct DescriptorHandle
    {
        D3D12_CPU_DESCRIPTOR_HANDLE mCpu;
        D3D12_GPU_DESCRIPTOR_HANDLE mGpu;
    };

    /// CPU Visible Heap to store all the resources needing CPU read / write operations - Textures/Buffers/RTV
    struct RHIDescriptorHeap_D3D12
    {
        /// DX Heap
        ID3D12DescriptorHeap* pCurrentHeap;

        ID3D12Device* pDevice;
        D3D12_CPU_DESCRIPTOR_HANDLE* pHandles;
        /// Start position in the heap
        DescriptorHandle mStartHandle;
        /// Free List used for CPU only descriptor heaps
        eastl::vector<DescriptorHandle> mFreeList;
        // Bitmask to track free regions (set bit means occupied)
        uint32_t* pFlags;
        /// Description
        D3D12_DESCRIPTOR_HEAP_DESC mDesc;
        D3D12_DESCRIPTOR_HEAP_TYPE mType;
        uint32_t mNumDescriptors;
        /// Descriptor Increment Size
        uint32_t mDescriptorSize;
        // Usage
        uint32_t mUsedDescriptors;
    };

    struct RHIEmptyDescriptors_D3D12
    {
        D3D12_CPU_DESCRIPTOR_HANDLE Sampler;
        D3D12_CPU_DESCRIPTOR_HANDLE TextureSRV[RHI_TEX_DIMENSION_COUNT];
        D3D12_CPU_DESCRIPTOR_HANDLE TextureUAV[RHI_TEX_DIMENSION_COUNT];
        D3D12_CPU_DESCRIPTOR_HANDLE BufferSRV;
        D3D12_CPU_DESCRIPTOR_HANDLE BufferUAV;
        D3D12_CPU_DESCRIPTOR_HANDLE BufferCBV;
    };

    struct RHIInstance_D3D12 : public RHIInstance
    {
    #if defined(XBOX)
    #elif defined(_WINDOWS)
        struct IDXGIFactory6* pDXGIFactory;
    #endif
        struct ID3D12Debug* pDXDebug;
        struct RHIAdapter_D3D12* pAdapters;
        uint32_t mAdaptersCount;
    };

    struct RHIAdapter_D3D12 : public RHIAdapter
    {
        RHIAdapter_D3D12()
        {
            CB_CORE_INFO("RHIAdapter_D3D12::RHIAdapter_D3D12()");
            pInstance = nullptr;
        }
    #if defined(XBOX)
    #elif defined(_WINDOWS)
        struct IDXGIAdapter4* pDxActiveGPU;
    #endif
        D3D_FEATURE_LEVEL mFeatureLevel;
        RHIAdapterDetail mAdapterDetail;
        bool mEnhanceBarrierSupported : 1;
    };

    struct RHIFence_D3D12 : public RHIFence
    {
    public:
        ID3D12Fence* pDxFence;
        HANDLE pDxWaitIdleFenceEvent;
        uint64_t mFenceValue;
        uint64_t mPadA;
    };

    /// DirectX12 does not have a concept of a semaphore. We emulate it using a fence.
    struct RHISemaphore_D3D12 : public RHISemaphore
    {
        ID3D12Fence* dx_fence;
        HANDLE dx_wait_idle_fence_event;
        uint64_t fence_value;
        uint64_t pad_a;
    };

    struct RHIQueue_D3D12: public RHIQueue
    {
        ID3D12CommandQueue* pCommandQueue;
        RHIFence* pFence;
    };

    struct RHICommandPool_D3D12: public RHICommandPool
    {
        struct ID3D12CommandAllocator* pDxCmdAlloc;
    };

    struct RHICommandBuffer_D3D12: public RHICommandBuffer
    {
        ID3D12GraphicsCommandList* pDxCmdList;
        // Cached in beginCmd to avoid fetching them during rendering
        struct RHIDescriptorHeap_D3D12* pBoundHeaps[2];
        D3D12_GPU_DESCRIPTOR_HANDLE mBoundHeapStartHandles[2];
        // Command buffer state
        const ID3D12RootSignature* pBoundRootSignature;
        uint32_t mType;
        uint32_t mNodeIndex;
        RHICommandPool* pCmdPool;
        D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS mSubResolveResource[RHI_MAX_MRT_COUNT];
    };

    struct RHIQueryPool_D3D12: public RHIQueryPool
    {
        uint32_t mCount;
    };

    struct RHISwapChain_D3D12 : public RHISwapChain 
    {
        IDXGISwapChain3* pDxSwapChain;
        uint32_t mDxSyncInterval : 3;
        uint32_t mFlags : 10;
        uint32_t mImageCount : 3;
        uint32_t mEnableVsync : 1;
    };

    struct RHIRootSignature_D3D12 : public RHIRootSignature
    {
        ID3D12RootSignature* dxRootSignature;
        D3D12_ROOT_PARAMETER1 root_constant_parameter;
        uint32_t root_parameter_index;
    };

    struct RHIDescriptorSet_D3D12 : public RHIDescriptorSet
    {
        /// Start handle to cbv srv uav descriptor table
        uint64_t cbv_srv_uav_handle;
        /// Stride of the cbv srv uav descriptor table (number of descriptors * descriptor size)
        uint32_t cbv_srv_uav_stride;
        /// Start handle to sampler descriptor table
        uint64_t sampler_handle;
        /// Stride of the sampler descriptor table (number of descriptors * descriptor size)
        uint32_t sampler_stride;
    };

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

    struct RHIRenderPipeline_D3D12 : public RHIRenderPipeline
    {
        ID3D12PipelineState* pDxPipelineState;
        ID3D12RootSignature* pDxRootSignature;
        D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopologyType;
    };

    struct RHISampler_D3D12 : public RHISampler
    {
        /// Description for creating the sampler descriptor for this sampler
        D3D12_SAMPLER_DESC dxSamplerDesc;
        /// Descriptor handle of the Sampler in a CPU visible descriptor heap
        D3D12_CPU_DESCRIPTOR_HANDLE mDxHandle;
    };
    
    struct RHIShaderLibrary_D3D12 : public RHIShaderLibrary
    {
        ID3DBlob* shader_blob;
        //struct IDxcBlobEncoding* shader_dxc_blob;
        struct IDxcResult* shader_result;
    };

    static const D3D_FEATURE_LEVEL d3d_feature_levels[] = 
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    static const D3D12_COMMAND_LIST_TYPE gDx12CmdTypeTranslator[RHI_QUEUE_TYPE_COUNT] = {
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_TYPE_COMPUTE,
        D3D12_COMMAND_LIST_TYPE_COPY
    };
    
}