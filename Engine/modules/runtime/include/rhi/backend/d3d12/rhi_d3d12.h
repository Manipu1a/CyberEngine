#pragma once
#include "rhi/rhi.h"
#include "EASTL/map.h"
#include "EASTL/vector.h"

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

    struct RHITexture2D_D3D12 : public RHITexture2D
    {
        DxDescriptorID mDescriptors;
        ID3D12Resource* pDxResource;
        D3D12MA::Allocation* pDxAllocation;
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

        ID3D12Device* pDeviceImpl;
        class D3D12MA::Allocator* pResourceAllocator;

        // API specific descriptor heap and memory allocator
        eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mCPUDescriptorHeaps;
        eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mSamplerHeaps;
        eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mCbvSrvUavHeaps;
        struct RHIEmptyDescriptors_D3D12* pNullDescriptors;
        eastl::map<ERHIQueueType, ID3D12CommandQueue*> mCommandQueues;
        eastl::map<ERHIQueueType, uint32_t> mCommandQueueCounts;
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
    };

    class RHISemaphore_D3D12 : public RHISemaphore
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
    };

    class RHIQueue_D3D12: public RHIQueue
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
    };

    class RHICommandPool_D3D12: public RHICommandPool
    {
    public:
        Cyber::Ref<RHIQueue> pQueue;
    };

    class RHICommandBuffer_D3D12: public RHICommandBuffer
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        Cyber::Ref<RHICommandPool> pPool;
        ERHIPipelineType mCurrentDispatch;
    };

    class RHIQueryPool_D3D12: public RHIQueryPool
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        uint32_t mCount;
    };

    class RHI_D3D12 : public RHI
    {
    public:
        RHI_D3D12();
        virtual ~RHI_D3D12();
    public:
        // Device APIs
        virtual void rhi_create_device(Ref<RHIDevice> pDevice, Ref<RHIAdapter> pAdapter, const DeviceCreateDesc& deviceDesc) override;
        // API Object APIs

        virtual void rhi_create_instance(Ref<RHIInstance> pInstance, const RHIInstanceCreateDesc& instanceDesc) override;
        virtual Texture2DRHIRef rhi_create_texture(Ref<RHIDevice> pDevice, const TextureCreationDesc& textureDesc) override;
        virtual BufferRHIRef rhi_create_buffer(Ref<RHIDevice> pDevice, const BufferCreateDesc& bufferDesc) override;
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