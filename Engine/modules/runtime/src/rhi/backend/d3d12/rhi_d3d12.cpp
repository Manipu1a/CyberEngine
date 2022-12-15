#include "rhi/backend/d3d12/rhi_d3d12.h"
#include "CyberMemory/Memory.h"
#include "d3d12_utils.h"
#include "CyberLog/Log.h"
#include "math/common.h"

namespace Cyber
{
    #define DECLARE_ZERO(type, var) type var = {};

    HRESULT RHIDevice_D3D12::hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize)
    {
        return pDeviceImpl->CheckFeatureSupport(pFeature, pFeatureSupportData, pFeatureSupportDataSize);
    }

    HRESULT RHIDevice_D3D12::hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource)
    {
        return pDeviceImpl->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

    RHI_D3D12::RHI_D3D12()
    {
        
    }

    RHI_D3D12::~RHI_D3D12()
    {

    }

    Texture2DRHIRef RHI_D3D12::rhi_create_texture(const Renderer& pRenderer, const TextureCreationDesc& pDesc)
    {
        //RHID3D12Texture* pTexture = (RHID3D12Texture*)cb_calloc_memalign(1, alignof(RHID3D12Texture), sizeof(RHID3D12Texture));
        RHIDevice_D3D12* DxDevice = static_cast<RHIDevice_D3D12*>(pRenderer.mDevice.get());

        Ref<RHITexture2D_D3D12> pTexture = CreateRef<RHITexture2D_D3D12>();
        CB_ASSERTS(pTexture != nullptr, "rhi texture create failed!");

        D3D12_RESOURCE_DESC desc = {};

        //TODO:
        DXGI_FORMAT dxFormat = DXGIUtil_TranslatePixelFormat(pDesc.mFormat);
        
        ERHIDescriptorType descriptors = pDesc.mDescriptors;

        if(pDesc.mNativeHandle == nullptr)
        {
            D3D12_RESOURCE_DIMENSION res_dim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
            if(pDesc.mFlags & RHI_TCF_FORCE_2D)
            {
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }
            else if(pDesc.mFlags & RHI_TCF_FORCE_3D)
            {
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            }
            else
            {
                if(pDesc.mDepth > 1)
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                else if(pDesc.mHeight > 1)
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                else
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            }

            desc.Dimension = res_dim;
            desc.Alignment = (UINT)pDesc.mSampleCount > 1 ? D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT : 0;
            desc.Width = pDesc.mWidth;
            desc.Height = pDesc.mHeight;
            desc.DepthOrArraySize = (UINT)(pDesc.mArraySize != 1 ? pDesc.mArraySize : pDesc.mDepth);
            desc.MipLevels = (UINT)pDesc.mMipLevels;
            desc.Format = DXGIUtil_FormatToTypeless(dxFormat);
            desc.SampleDesc.Count = (UINT)pDesc.mSampleCount;
            desc.SampleDesc.Quality = (UINT)pDesc.mSampleQuality;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS data;
            data.Format = desc.Format;
            data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
            data.SampleCount = desc.SampleDesc.Count;
            DxDevice->hook_CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(data));
            while(data.NumQualityLevels == 0 && data.SampleCount > 0)
            {
                CB_CORE_WARN("Sample Count [0] not supported. Trying a lower sample count [1]", data.SampleCount, data.SampleCount / 2);
                data.SampleCount = desc.SampleDesc.Count / 2;
                DxDevice->hook_CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(data));
            }

            desc.SampleDesc.Count = data.SampleCount;

            RHIResourceState actualStartState = pDesc.mStartState;

            // Decide UAV flags
            if(descriptors & RHI_DESCRIPTOR_TYPE_RW_TEXTURE)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            // Decide render target flags
            if(pDesc.mStartState & RHI_RESOURCE_STATE_RENDER_TARGET)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                actualStartState = (pDesc.mStartState > RHI_RESOURCE_STATE_RENDER_TARGET)
                                    ? (pDesc.mStartState & (ERHIResourceState)~RHI_RESOURCE_STATE_RENDER_TARGET)
                                    : RHI_RESOURCE_STATE_RENDER_TARGET;
            }
            else if(pDesc.mStartState & RHI_RESOURCE_STATE_DEPTH_WRITE)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                actualStartState = (pDesc.mStartState > RHI_RESOURCE_STATE_DEPTH_WRITE)
                                    ? (pDesc.mStartState & (ERHIResourceState)~RHI_RESOURCE_STATE_DEPTH_WRITE)
                                    : RHI_RESOURCE_STATE_DEPTH_WRITE;
            }

            // Decide sharing flags for multi adapter
            if(pDesc.mFlags & RHI_TCF_EXPORT_ADAPTER_BIT)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            }

            if(pDesc.mFlags & RHI_TCF_FORCE_ALLOW_DISPLAY_TARGET)
            {
                actualStartState = RHI_RESOURCE_STATE_PRESENT;
            }

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = dxFormat;
            if(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                clearValue.DepthStencil.Depth = pDesc.mClearValue.depth;
                clearValue.DepthStencil.Stencil = (UINT8)pDesc.mClearValue.stencil;
            }
            else
            {
                clearValue.Color[0] = pDesc.mClearValue.r;
                clearValue.Color[1] = pDesc.mClearValue.g;
                clearValue.Color[2] = pDesc.mClearValue.b;
                clearValue.Color[3] = pDesc.mClearValue.a;
            }

            D3D12_CLEAR_VALUE* pClearValue = nullptr;
            D3D12_RESOURCE_STATES res_states = D3D12Util_TranslateResourceState(actualStartState);

            if((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            {
                pClearValue = &clearValue;
            }

            D3D12MA::ALLOCATION_DESC alloc_desc = {};
            alloc_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            if(pDesc.mFlags & RHI_TCF_OWN_MEMORY_BIT)
                alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;

            // Create resource
            auto hRes = DxDevice->pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, pClearValue, &pTexture->pDxAllocation, IID_ARGS(&pTexture->pDxResource));
            if(hRes != S_OK)
            {
                auto fallbackhRes = hRes;
                CB_CORE_ERROR("[D3D12] Create Texture Resource Failed With HRESULT [0]! \n\t With Name: [1] \n\t Size: [2][3] \n\t Format: [4] \n\t Sample Count: [5]", 
                                hRes, pDesc.pName ? pDesc.pName : "", pDesc.mWidth, pDesc.mHeight,
                                pDesc.mFormat, pDesc.mSampleCount);
                const bool use_fallback_commited = true;
                if(use_fallback_commited)
                {
                    D3D12_HEAP_PROPERTIES heapProps = {};
                    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
                    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    heapProps.CreationNodeMask = RHI_SINGLE_GPU_NODE_MASK;
                    heapProps.VisibleNodeMask = RHI_SINGLE_GPU_NODE_MASK;
                    fallbackhRes = DxDevice->hook_CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, &desc, res_states, pClearValue, IID_ARGS(&pTexture->pDxResource));
                    if(fallbackhRes == S_OK)
                    {
                        CB_CORE_TRACE("[D3D12] Create Texture With Fallback Driver API Succeed!");
                    }
                    else 
                    {
                         CB_CORE_TRACE("[D3D12] Create Texture With Fallback Driver API Failed! Please Update Your Driver or Contact With us!");
                    }
                }
            }
            else
            {
                CB_CORE_TRACE("[D3D12] Create Texture Resource Succeed! \n\t With Name: [0]\n\t Size: [1]x[2] \n\t Format: [3] \n\t Sample Count: [4]", 
                                pDesc.pName ? pDesc.pName : "", pDesc.mWidth, pDesc.mHeight,
                                pDesc.mFormat, pDesc.mSampleCount);
            }


        }

        return pTexture;
    }

    BufferRHIRef RHI_D3D12::rhi_create_buffer(const Renderer& pRenderer, const BufferCreateDesc& pDesc)
    {
        RHIDevice_D3D12* DxDevice = static_cast<RHIDevice_D3D12*>(pRenderer.mDevice.get());
        RHIAdapter_D3D12* DxAdapter = static_cast<RHIAdapter_D3D12*>(pRenderer.mAdapter.get());

        Ref<RHIBuffer_D3D12> pBuffer = CreateRef<RHIBuffer_D3D12>();
        pBuffer->mDescriptors = D3D12_DESCRIPTOR_ID_NONE;

        uint64_t allocationSize = pDesc.mSize;
        // Align the buffer size to multiples of the dynamic uniform buffer minimum size
        if(pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            allocationSize = round_up_64(allocationSize, DxAdapter->mAdapterDetail.mUniformBufferAlignment);
        }

        DECLARE_ZERO(D3D12_RESOURCE_DESC, desc);
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        //Alignment must be 64KB (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) or 0, which is effectively 64KB.
        desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        desc.Width = allocationSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        if(pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_RW_BUFFER)
        {
            // UAV
            desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // Adjust for padding
        uint64_t padded_size = 0;
        DxDevice->pDeviceImpl->GetCopyableFootprints(&desc, 0, 1, 0, NULL, NULL, NULL, &padded_size);
        allocationSize = (uint64_t)padded_size;
        // Buffer is 1D
        desc.Width = padded_size;

        RHIResourceState start_state = pDesc.mStartState;
        if(pDesc.mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_CPU_TO_GPU || pDesc.mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_CPU_ONLY)
        {
            // Your application should generally avoid transitioning to D3D12_RESOURCE_STATE_GENERIC_READ when possible, 
            // since that can result in premature cache flushes, or resource layout changes (for example, compress/decompress),
            // causing unnecessary pipeline stalls.
            start_state = RHI_RESOURCE_STATE_GENERIC_READ;
        }
        else if(pDesc.mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_GPU_TO_CPU)
        {
            start_state = RHI_RESOURCE_STATE_COPY_DEST;
        }

        D3D12_RESOURCE_STATES res_states = D3D12Util_TranslateResourceState(start_state);

        D3D12MA::ALLOCATION_DESC alloc_desc = {};

        if(pDesc.mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_CPU_ONLY || pDesc.mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_CPU_TO_GPU)
        {
            alloc_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        }
        else if(pDesc.mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_GPU_TO_CPU)
        {
            alloc_desc.HeapType = D3D12_HEAP_TYPE_READBACK;
            desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
        else
        {
            alloc_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        }
        
        // for commit resource
        if(pDesc.mFlags & RHI_BCF_OWN_MEMORY_BIT)
            alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;

        if(alloc_desc.HeapType != D3D12_HEAP_TYPE_DEFAULT && (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            heapProps.VisibleNodeMask = RHI_SINGLE_GPU_NODE_MASK;
            heapProps.CreationNodeMask = RHI_SINGLE_GPU_NODE_MASK;
            CHECK_HRESULT(DxDevice->pDeviceImpl->CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, &desc, res_states, NULL, IID_ARGS(&pBuffer->pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Committed Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", pDesc.pName ? pDesc.pName : "", allocationSize, pDesc.mFormat);
        }
        else
        {
            CHECK_HRESULT(DxDevice->pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, NULL, &pBuffer->pDxAllocation, IID_ARGS(&pBuffer->pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", pDesc.pName ? pDesc.pName : "", allocationSize, pDesc.mFormat);
        }
        
        
        return pBuffer;
    }
}