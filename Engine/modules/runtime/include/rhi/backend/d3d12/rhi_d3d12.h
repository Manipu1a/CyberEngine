#include "rhi/rhi.h"

namespace Cyber
{
    typedef int32_t DxDescriptorID;
    #define IID_ARGS IID_PPV_ARGS
    #define D3D12_DESCRIPTOR_ID_NONE ((int32_t)-1)

    class RHIDevice_D3D12 : public RHIDevice
    {
    public:
        HRESULT hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize);
        HRESULT hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource);

        ID3D12Device* pDeviceImpl;
        class D3D12MA::Allocator* pResourceAllocator;
    };

    class RHIAdapter_D3D12 : public RHIAdapter
    {
    public:
        RHIAdapter_D3D12();
        
    };

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
        DxDescriptorID mDescriptors;
        /// Offset from mDxDescriptors for srv descriptor handle
        uint8_t mSrvDescriptorOffset;
        /// Offset from mDxDescriptors for uav descriptor handle
        uint8_t mUavDescriptorOffset;
        /// Native handle of the underlying resource
        ID3D12Resource* pDxResource;
        /// Contains resource allocation info such as parent heap, offset in heap
        D3D12MA::Allocation* pDxAllocation;
    };

    class RHI_D3D12 : public RHI
    {
    public:
        RHI_D3D12();
        virtual ~RHI_D3D12();
    public:
        virtual Texture2DRHIRef rhi_create_texture(const Renderer& pRenderer, const TextureCreationDesc& pDesc) override;
        virtual BufferRHIRef rhi_create_buffer(const Renderer& pRenderer, const BufferCreateDesc& pDesc) override;
    };
}