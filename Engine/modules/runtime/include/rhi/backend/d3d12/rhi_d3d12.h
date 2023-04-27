#pragma once
#include "rhi/rhi.h"
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

    struct RHITexture_D3D12 : public RHITexture
    {
        ID3D12Resource* pDxResource;
        D3D12MA::Allocation* pDxAllocation;
    };

    struct RHITexture2D_D3D12 : public RHITexture2D
    {
        DxDescriptorID mDescriptors;
        ID3D12Resource* pDxResource;
        D3D12MA::Allocation* pDxAllocation;
    };

    struct RHITextureView_D3D12 : public RHITextureView
    {
        D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
        /// Offset from mDxDescriptors for srv descriptor handle
        uint8_t mSrvDescriptorOffset;
        /// Offset from mDxDescriptors for uav descriptor handle
        uint8_t mUavDescriptorOffset;
        /// Offset from mDxDescriptors for rtv descriptor handle
        D3D12_CPU_DESCRIPTOR_HANDLE mRtvDsvDescriptorHandle;
    };

    struct RHIBuffer_D3D12 : public RHIBuffer
    {
        /// GPU Address - Cache to avoid calls to ID3D12Resource::GetGpuVirtualAddress
        D3D12_GPU_VIRTUAL_ADDRESS mDxGpuAddress;
        /// Descriptor handle of the CBV in a CPU visible descriptor heap (applicable to BUFFER_USAGE_UNIFORM)
        D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
        /// Offset from mDxDescriptors for srv descriptor handle
        uint8_t mSrvDescriptorOffset;
        /// Offset from mDxDescriptors for uav descriptor handle
        uint8_t mUavDescriptorOffset;
        /// Native handle of the underlying resource
        ID3D12Resource* pDxResource;
        /// Contains resource allocation info such as parent heap, offset in heap
        D3D12MA::Allocation* pDxAllocation;
    };

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

    class RHIDevice_D3D12 : public RHIDevice
    {
    public:
        HRESULT hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize);
        HRESULT hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource);

        class D3D12MA::Allocator* pResourceAllocator;

        // API specific descriptor heap and memory allocator
        eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mCPUDescriptorHeaps;
        eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mSamplerHeaps;
        eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mCbvSrvUavHeaps;
        struct RHIEmptyDescriptors_D3D12* pNullDescriptors;
        eastl::map<uint32_t, ID3D12CommandQueue**> ppCommandQueues;
        eastl::map<uint32_t, uint32_t> pCommandQueueCounts;
        IDXGIFactory6* pDXGIFactory;
        IDXGIAdapter4* pDxActiveGPU;
        ID3D12Device* pDxDevice;

        // PSO Cache
        ID3D12PipelineLibrary* pPipelineLibrary;
        void* pPSOCacheData;
        
        uint64_t mPadA;
        ID3D12Debug* pDxDebug;
        #if defined (_WINDOWS)
            ID3D12InfoQueue* pDxDebugValidation;
        #endif
    };

    class RHIInstance_D3D12 : public RHIInstance
    {
    public:
    #if defined(XBOX)
    #elif defined(_WINDOWS)
        struct IDXGIFactory6* pDXGIFactory;
    #endif
        struct ID3D12Debug* pDXDebug;
        struct RHIAdapter_D3D12* pAdapters;
        uint32_t mAdaptersCount;
    };

    class RHIAdapter_D3D12 : public RHIAdapter
    {
    public:
        RHIAdapter_D3D12();
    #if defined(XBOX)
    #elif defined(_WINDOWS)
        struct IDXGIAdapter4* pDxActiveGPU;
    #endif
        D3D_FEATURE_LEVEL mFeatureLevel;
        RHIAdapterDetail mAdapterDetail;
        bool mEnhanceBarrierSupported : 1;
    };

    class RHIFence_D3D12 : public RHIFence
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        ID3D12Fence* pDxFence;
        HANDLE pDxWaitIdleFenceEvent;
        uint64_t mFenceValue;
        uint64_t mPadA;
    };

    /// DirectX12 does not have a concept of a semaphore. We emulate it using a fence.
    class RHISemaphore_D3D12 : public RHISemaphore
    {
    public:
        ID3D12Fence* dx_fence;
        HANDLE dx_wait_idle_fence_event;
        uint64_t fence_value;
        uint64_t pad_a;
    };

    class RHIQueue_D3D12: public RHIQueue
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        ID3D12CommandQueue* pCommandQueue;
        Cyber::Ref<RHIFence> pFence;
    };

    class RHICommandPool_D3D12: public RHICommandPool
    {
    public:
        struct ID3D12CommandAllocator* pDxCmdAlloc;
    };

    class RHICommandBuffer_D3D12: public RHICommandBuffer
    {
    public:
        ID3D12GraphicsCommandList* pDxCmdList;
        // Cached in beginCmd to avoid fetching them during rendering
        struct RHIDescriptorHeap_D3D12* pBoundHeaps[2];
        D3D12_GPU_DESCRIPTOR_HANDLE mBoundHeapStartHandles[2];
        // Command buffer state
        const ID3D12RootSignature* pBoundRootSignature;
        uint32_t mType;
        uint32_t mNodeIndex;
        Cyber::Ref<RHICommandPool> pCmdPool;
        D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS mSubResolveResource[RHI_MAX_MRT_COUNT];
    };

    class RHIQueryPool_D3D12: public RHIQueryPool
    {
    public:
        uint32_t mCount;
    };

    class RHISwapChain_D3D12 : public RHISwapChain 
    {
    public:
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
        struct IDxcBlobEncoding* shader_blob;
    };

    class RHI_D3D12 : public RHI
    {
    public:
        RHI_D3D12();
        virtual ~RHI_D3D12();
    public:
        // Device APIs
        virtual Ref<RHIDevice> rhi_create_device(Ref<RHIAdapter> pAdapter, const RHIDeviceCreateDesc& deviceDesc) override;
        virtual void rhi_free_device(Ref<RHIDevice> pDevice) override;
        // API Object APIs
        virtual Ref<RHISurface> rhi_surface_from_hwnd(Ref<RHIDevice> pDevice, HWND hwnd) override;
        virtual FenceRHIRef rhi_create_fence(Ref<RHIDevice> pDevice) override;
        virtual void rhi_wait_fences(const Ref<RHIFence>* fences, uint32_t fenceCount) override;
        virtual void rhi_free_fence(Ref<RHIFence> fence) override;
        virtual ERHIFenceStatus rhi_query_fence_status(Ref<RHIFence> pFence) override;
        virtual SwapChainRef rhi_create_swap_chain(Ref<RHIDevice> pDevice, const RHISwapChainCreateDesc& swapchainDesc) override;
        virtual void rhi_free_swap_chain(Ref<RHISwapChain> pSwapChain) override;
        virtual void rhi_enum_adapters(Ref<RHIInstance> instance, RHIAdapter* const adapters, uint32_t* adapterCount) override;
        virtual uint32_t rhi_acquire_next_image(Ref<RHISwapChain> pSwapChain, const RHIAcquireNextDesc& acquireDesc) override;
        // Queue APIs
        virtual QueueRHIRef rhi_get_queue(Ref<RHIDevice> pDevice, ERHIQueueType type, uint32_t index) override;
        virtual void rhi_submit_queue(Ref<RHIQueue> queue, const RHIQueueSubmitDesc& submitDesc) override;
        virtual void rhi_present_queue(Ref<RHIQueue> queue, const RHIQueuePresentDesc& presentDesc) override;
        virtual void rhi_wait_queue_idle(Ref<RHIQueue> queue) override;
        virtual CommandPoolRef rhi_create_command_pool(Ref<RHIQueue> pQueue, const CommandPoolCreateDesc& commandPoolDesc) override;
        virtual void rhi_reset_command_pool(Ref<RHICommandPool> pPool) override;
        virtual void rhi_free_command_pool(Ref<RHICommandPool> pPool) override;
        virtual CommandBufferRef rhi_create_command_buffer(Ref<RHICommandPool> pPool, const CommandBufferCreateDesc& commandBufferDesc) override;
        virtual void rhi_free_command_buffer(Ref<RHICommandBuffer> pCommandBuffer) override;

        virtual void rhi_cmd_begin(Ref<RHICommandBuffer> pCommandBuffer) override;
        virtual void rhi_cmd_end(Ref<RHICommandBuffer> pCommandBuffer) override;
        virtual void rhi_cmd_resource_barrier(Ref<RHICommandBuffer> cmd, const RHIResourceBarrierDesc& barrierDesc) override;

        virtual Ref<RHIRenderPassEncoder> rhi_cmd_begin_render_pass(Ref<RHICommandBuffer> pCommandBuffer, const RHIRenderPassDesc& beginRenderPassDesc) override;
        virtual void rhi_cmd_end_render_pass(Ref<RHICommandBuffer> pCommandBuffer) override;
        virtual void rhi_render_encoder_bind_descriptor_set(Ref<RHIRenderPassEncoder> encoder, Ref<RHIDescriptorSet> descriptorSet);
        virtual void rhi_render_encoder_set_viewport(Ref<RHIRenderPassEncoder> encoder, float x, float y, float width, float height, float min_depth, float max_depth);
        virtual void rhi_render_encoder_set_scissor(Ref<RHIRenderPassEncoder> encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        virtual void rhi_render_encoder_bind_pipeline(Ref<RHIRenderPassEncoder> encoder, Ref<RHIRenderPipeline> pipeline);
        virtual void rhi_render_encoder_bind_vertex_buffer(Ref<RHIRenderPassEncoder> encoder, uint32_t buffer_count, Ref<RHIBuffer>* buffers,const uint32_t* strides, const uint32_t* offsets);
        virtual void rhi_render_encoder_bind_index_buffer(Ref<RHIRenderPassEncoder> encoder, Ref<RHIBuffer> buffer, uint32_t index_stride, uint64_t offset);
        virtual void rhi_render_encoder_push_constants(Ref<RHIRenderPassEncoder> encoder, Ref<RHIRootSignature> rs, const char8_t* name, const void* data);
        virtual void rhi_render_encoder_draw(Ref<RHIRenderPassEncoder> encoder, uint32_t vertex_count, uint32_t first_vertex);
        virtual void rhi_render_encoder_draw_instanced(Ref<RHIRenderPassEncoder> encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
        virtual void rhi_render_encoder_draw_indexed(Ref<RHIRenderPassEncoder> encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
        virtual void rhi_render_encoder_draw_indexed_instanced(Ref<RHIRenderPassEncoder> encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
        
        virtual RootSignatureRHIRef rhi_create_root_signature(Ref<RHIDevice> pDevice, const RHIRootSignatureCreateDesc& rootSigDesc) override;
        virtual DescriptorSetRHIRef rhi_create_descriptor_set(Ref<RHIDevice> pDevice, const RHIDescriptorSetCreateDesc& dSetDesc) override;
        virtual void rhi_update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData& updateDesc, uint32_t count) override;

        virtual Ref<RHIRenderPipeline> rhi_create_render_pipeline(Ref<RHIDevice> pDevice, const RHIRenderPipelineCreateDesc& pipelineDesc) override;
        virtual void rhi_free_render_pipeline(Ref<RHIRenderPipeline> pipeline) override;
        virtual InstanceRHIRef rhi_create_instance(const RHIInstanceCreateDesc& instanceDesc) override;
        virtual void rhi_free_instance(Ref<RHIInstance> instance) override;

        virtual Ref<RHITextureView> rhi_create_texture_view(Ref<RHIDevice> pDevice, const RHITextureViewCreateDesc& viewDesc) override;
        virtual void rhi_free_texture_view(Ref<RHITextureView> view) override;
        virtual Texture2DRHIRef rhi_create_texture(Ref<RHIDevice> pDevice, const TextureCreationDesc& textureDesc) override;
        virtual BufferRHIRef rhi_create_buffer(Ref<RHIDevice> pDevice, const BufferCreateDesc& bufferDesc) override;

        virtual ShaderLibraryRHIRef rhi_create_shader_library(Ref<RHIDevice> device, const struct RHIShaderLibraryCreateDesc& desc) override;
        virtual void rhi_free_shader_library(Ref<RHIShaderLibrary> shaderLibrary) override;

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