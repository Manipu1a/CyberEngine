#include "rhi/backend/d3d12/rhi_d3d12.h"
#include "EASTL/EABase/eabase.h"
#include "EASTL/vector.h"
#include "d3d12_utils.h"
#include "CyberLog/Log.h"
#include "math/common.h"
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <intsafe.h>
#include <malloc.h>
#include <stdint.h>
#include <synchapi.h>

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

    void rhi_create_device(Ref<RHIDevice> pDevice, Ref<RHIAdapter> pAdapter, const DeviceCreateDesc& deviceDesc)
    {
        RHIAdapter_D3D12* dxAdapter = static_cast<RHIAdapter_D3D12*>(pAdapter.get());
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(pAdapter->pInstance.get());
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        
        dxDevice->pAdapter = pAdapter;

        if(!SUCCEEDED(D3D12CreateDevice(dxAdapter->pDxActiveGPU, dxAdapter->mFeatureLevel, IID_PPV_ARGS(&dxDevice->pDxDevice))))
        {
            cyber_assert(false, "[D3D12 Fatal]: Create D3D12Device Failed!");
        }

        // Create Requested Queues
        dxDevice->pNullDescriptors = (RHIEmptyDescriptors_D3D12*)cb_calloc(1, sizeof(RHIEmptyDescriptors_D3D12));

        for(uint32_t i = 0u; i < deviceDesc.mQueueGroupCount;i++)
        {
            const auto& queueGroup = deviceDesc.mQueueGroupsDesc[i];
            const auto type = queueGroup.mQueueType;

            dxDevice->pCommandQueueCounts[type] = queueGroup.mQueueCount;
            dxDevice->ppCommandQueues[type] = (ID3D12CommandQueue*)cb_malloc(sizeof(ID3D12CommandQueue*) * queueGroup.mQueueCount);

            for(uint32_t j = 0u; j < queueGroup.mQueueCount; j++)
            {
                DECLARE_ZERO(D3D12_COMMAND_QUEUE_DESC, queueDesc)
                switch(type)
                {
                    case RHI_QUEUE_TYPE_GRAPHICS:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                        break;
                    case RHI_QUEUE_TYPE_COMPUTE:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                        break;
                    case RHI_QUEUE_TYPE_TRANSFER:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                        break;
                    default:
                        cyber_assert(false, "[D3D12 Fatal]: Create D3D12CommandQueue Failed!");
                        break;
                }
            }
        }

        // Create D3D12MA Allocator
        D3D12Util_CreateDMAAllocator(dxInstance, dxAdapter, dxDevice);
        cyber_assert(dxDevice->pResourceAllocator == nullptr, "DMA Allocator Must be Created!");
        // Create Descriptor Heaps
        for(uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Flags = gCpuDescriptorHeapProerties[i].mFlags;
            desc.NodeMask = 0;
            desc.NumDescriptors = gCpuDescriptorHeapProerties[i].mMaxDescriptors;
            desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
#ifdef _DEBUG

#endif
            dxDevice->mCPUDescriptorHeaps[i] = (RHIDescriptorHeap_D3D12*)cb_malloc(sizeof(RHIDescriptorHeap_D3D12));
            D3D12Util_CreateDescriptorHeap(dxDevice->pDxDevice, desc, &dxDevice->mCPUDescriptorHeaps[i]);
        }

        // Allocate NULL Descriptors
        {
            dxDevice->pNullDescriptors->Sampler = D3D12_DESCRIPTOR_ID_NONE;
            D3D12_SAMPLER_DESC samplerDesc = {};
            samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            dxDevice->pNullDescriptors->Sampler = D3D12Util_ConsumeDescriptorHandles(dxDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1).mCpu;
            dxDevice->pDxDevice->CreateSampler(&samplerDesc, dxDevice->pNullDescriptors->Sampler);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8_UINT;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R8_UINT;

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_1D]);
            D3D12Util_CreateUAV(dxDevice, NULL, NULL, &uavDesc, &dxDevice->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_1D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2D]);
            D3D12Util_CreateUAV(dxDevice, NULL, NULL, &uavDesc, &dxDevice->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_2D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2DMS]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_3D]);
            D3D12Util_CreateUAV(dxDevice, NULL, NULL, &uavDesc, &dxDevice->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_3D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_CUBE]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_1D_ARRAY]);
            D3D12Util_CreateUAV(dxDevice, NULL, NULL, &uavDesc, &dxDevice->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_1D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2D_ARRAY]);
            D3D12Util_CreateUAV(dxDevice, NULL, NULL, &uavDesc, &dxDevice->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_2D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2DMS_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_CUBE_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            D3D12Util_CreateSRV(dxDevice, NULL, &srvDesc, &dxDevice->pNullDescriptors->BufferSRV);
            D3D12Util_CreateUAV(dxDevice, NULL, NULL, &uavDesc, &dxDevice->pNullDescriptors->BufferUAV);
            D3D12Util_CreateCBV(dxDevice, NULL, &dxDevice->pNullDescriptors->BufferCBV);
        }

        // Pipeline cache
        D3D12_FEATURE_DATA_SHADER_CACHE feature = {};
        HRESULT result = dxDevice->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &feature, sizeof(feature));
        if(SUCCEEDED(result))
        {
            result = E_NOTIMPL;
            if(feature.SupportFlags & D3D12_SHADER_CACHE_SUPPORT_LIBRARY)
            {
                ID3D12Device1* device1 = NULL;
                result = dxDevice->pDxDevice->QueryInterface(IID_ARGS(&device1));
                if(SUCCEEDED(result))
                {
                    result = device1->CreatePipelineLibrary(dxDevice->pPSOCacheData, 0, IID_ARGS(&dxDevice->pPipelineLibrary));
                }
                SAFE_RELEASE(device1);
            }
        }
    }

    Texture2DRHIRef RHI_D3D12::rhi_create_texture(Ref<RHIDevice> pDevice, const TextureCreationDesc& pDesc)
    {
        //RHID3D12Texture* pTexture = (RHID3D12Texture*)cb_calloc_memalign(1, alignof(RHID3D12Texture), sizeof(RHID3D12Texture));
        RHIDevice_D3D12* DxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());

        Ref<RHITexture2D_D3D12> pTexture = CreateRef<RHITexture2D_D3D12>();
        cyber_assert(pTexture != nullptr, "rhi texture create failed!");

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

    InstanceRHIRef RHI_D3D12::rhi_create_instance(Ref<RHIDevice> pDevice, const RHIInstanceCreateDesc& instanceDesc)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());

        RHIInstance_D3D12* dxInstance = cyber_new<RHIInstance_D3D12>();
        // Initialize driver
        D3D12Util_InitializeEnvironment(dxInstance);
        // Enable Debug Layer
        D3D12Util_Optionalenable_debug_layer(dxInstance, instanceDesc);

        UINT flags = 0;
        if(instanceDesc.mEnableDebugLayer)
            flags = DXGI_CREATE_FACTORY_DEBUG;
        
        if(SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxDevice->pDXGIFactory))))
        {
            uint32_t gpuCount = 0;
            bool foundSoftwareAdapter = false;
            D3D12Util_QueryAllAdapters(dxInstance, gpuCount, foundSoftwareAdapter);
            // If the only adapter we found is a software adapter, log error message for QA 
            if(!gpuCount && foundSoftwareAdapter)
            {
                cyber_assert(false, "The only avaliable GPU has DXGI_ADAPTER_FLAG_SOFTWARE. Early exiting");
                return nullptr;
            }
        }
        else 
        {
            cyber_assert(false, "[D3D12 Fatal]: Create DXGIFactory2 Failed!]");
        }

        return CreateRef<RHIInstance_D3D12>(dxInstance);
    }

    FenceRHIRef RHI_D3D12::rhi_create_fence(Ref<RHIDevice> pDevice)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIFence_D3D12* dxFence = cyber_new<RHIFence_D3D12>();
        cyber_assert(dxFence, "Fence create failed!");
        
        CHECK_HRESULT(dxDevice->pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dxFence->pDxFence)));
        dxFence->mFenceValue = 1;

        dxFence->pDxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        return CreateRef<RHIFence_D3D12>(dxFence);
    }

    QueueRHIRef RHI_D3D12::rhi_get_queue(Ref<RHIDevice> pDevice, ERHIQueueType type, uint32_t index)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIQueue_D3D12* dxQueue = cyber_new<RHIQueue_D3D12>();
        dxQueue->pCommandQueue = dxDevice->ppCommandQueues[type];
        dxQueue->pFence = rhi_create_fence(pDevice);
        return CreateRef<RHIQueue_D3D12>(dxQueue);
    }

    // Command Objects
    void allocate_transient_command_allocator(RHICommandPool_D3D12* commandPool, Ref<RHIQueue> queue)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(queue->pDevice.get());

        D3D12_COMMAND_LIST_TYPE type = queue->mType == RHI_QUEUE_TYPE_TRANSFER ? D3D12_COMMAND_LIST_TYPE_COPY : 
                            (queue->mType == RHI_QUEUE_TYPE_COMPUTE ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT);

        bool res = SUCCEEDED(dxDevice->pDxDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandPool->pDxCmdAlloc)));
        if(!res)
        {
            cyber_assert(false, "command allocator create failed!");
        }
    }

    CommandPoolRef RHI_D3D12::rhi_create_command_pool(Ref<RHIQueue> queue, const CommandPoolCreateDesc& commandPoolDesc)
    {
        RHICommandPool_D3D12* dxCommandPool = cyber_new<RHICommandPool_D3D12>();
        allocate_transient_command_allocator(dxCommandPool, queue);
        return CreateRef<RHICommandPool_D3D12>(dxCommandPool);
    }

    CommandBufferRef RHI_D3D12::rhi_create_command_buffer(Ref<RHICommandPool> pPool, const CommandBufferCreateDesc& commandBufferDesc) 
    {
        RHICommandBuffer_D3D12* dxCommandBuffer = cyber_new<RHICommandBuffer_D3D12>();
        RHICommandPool_D3D12* dxPool = static_cast<RHICommandPool_D3D12*>(pPool.get());
        RHIQueue_D3D12* dxQueue = static_cast<RHIQueue_D3D12*>(dxPool->pQueue.get());
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(dxQueue->pDevice.get());

        // set command pool of new command
        dxCommandBuffer->mNodeIndex = RHI_SINGLE_GPU_NODE_INDEX;
        dxCommandBuffer->mType = dxQueue->mType;

        dxCommandBuffer->pBoundHeaps[0] = dxDevice->mCbvSrvUavHeaps[dxCommandBuffer->mNodeIndex];
        dxCommandBuffer->pBoundHeaps[1] = dxDevice->mSamplerHeaps[dxCommandBuffer->mNodeIndex];
        dxCommandBuffer->pCmdPool = pPool;

        uint32_t nodeMask = dxCommandBuffer->mNodeIndex;
        ID3D12PipelineState* initialState = nullptr;
        CHECK_HRESULT(dxDevice->pDxDevice->CreateCommandList(nodeMask,gDx12CmdTypeTranslator[dxCommandBuffer->mType] , 
                dxPool->pDxCmdAlloc, initialState, IID_PPV_ARGS(&dxCommandBuffer->pDxCmdList)));
        
        // Command lists are add in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        CHECK_HRESULT(dxCommandBuffer->pDxCmdList->Close());
        return CreateRef<RHICommandBuffer_D3D12>(dxCommandBuffer);
    }
    
    SwapChainRef RHI_D3D12::rhi_create_swap_chain(Ref<RHIDevice> pDevice, const RHISwapChainCreateDesc& desc)
    {
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(pDevice->pAdapter->pInstance.get());
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        const uint32_t buffer_count = desc.mImageCount;
        RHISwapChain_D3D12* dxSwapChain = (RHISwapChain_D3D12*)cb_calloc(1, sizeof(RHISwapChain_D3D12) + desc.mImageCount * sizeof(RHITexture));
        dxSwapChain->mDxSyncInterval = desc.mEnableVsync ? 1 : 0;

        DECLARE_ZERO(DXGI_SWAP_CHAIN_DESC1, chinDesc);
        chinDesc.Width = desc.mWidth;
        chinDesc.Height = desc.mHeight;
        chinDesc.Format = DXGIUtil_TranslatePixelFormat(desc.mFormat);
        chinDesc.Stereo = false;
        chinDesc.SampleDesc.Count = 1;
        chinDesc.SampleDesc.Quality = 0;
        chinDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        chinDesc.BufferCount = buffer_count;
        chinDesc.Scaling = DXGI_SCALING_STRETCH;
        chinDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        chinDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        chinDesc.Flags = 0;
        BOOL allowTearing = FALSE;
        dxInstance->pDXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        chinDesc.Flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        dxSwapChain->mFlags |= (!desc.mEnableVsync && allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

        IDXGISwapChain1* swapchain;

        HWND hwnd = (HWND)desc.mWindowHandle.window;

        RHIQueue_D3D12* queue = nullptr;
        if(desc.mPresentQueue)
        {
            queue = static_cast<RHIQueue_D3D12*>(desc.mPresentQueue.get());
        }
        else 
        {
            queue = static_cast<RHIQueue_D3D12*>(rhi_get_queue(pDevice, RHI_QUEUE_TYPE_GRAPHICS, 0).get());
        }

        auto bCreated = SUCCEEDED(dxInstance->pDXGIFactory->CreateSwapChainForHwnd(queue->pCommandQueue, hwnd, &chinDesc, NULL, NULL, &swapchain));
        cyber_assert(bCreated, "Failed to try to create swapchain! An existed swapchain might be destroyed!");

        bCreated = SUCCEEDED(dxInstance->pDXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
        cyber_assert(bCreated, "Failed to try to associate swapchain with window!");

        auto bQueryChain3 = SUCCEEDED(swapchain->QueryInterface(IID_PPV_ARGS(&dxSwapChain->pDxSwapChain)));
        cyber_assert(bQueryChain3, "Failed to query IDXGISwapChain3 from created swapchain!");

        SAFE_RELEASE(swapchain);
        // Get swapchain images
        ID3D12Resource** backbuffers = (ID3D12Resource**)alloca(buffer_count * sizeof(ID3D12Resource*));
        for(uint32_t i = 0; i < buffer_count; ++i)
        {
            CHECK_HRESULT(dxSwapChain->pDxSwapChain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i])));
        }

        RHITexture_D3D12* Ts = (RHITexture_D3D12*)(dxSwapChain + 1);
        for(uint32_t i = 0; i < buffer_count; i++)
        {
            Ts[i].pDxResource = backbuffers[i];
            Ts[i].pDxAllocation = nullptr;
            Ts[i].mIsCube = false;
            Ts[i].mArraySize = 0;
            Ts[i].mFormat = desc.mFormat;
            Ts[i].mAspectMask = 1;
            Ts[i].mDepth = 1;
            Ts[i].mWidth = desc.mWidth;
            Ts[i].mHeight = desc.mHeight;
            Ts[i].mMipLevels = 1;
            Ts[i].mNodeIndex = RHI_SINGLE_GPU_NODE_INDEX;
            Ts[i].mOwnsImage = false;
            Ts[i].mNativeHandle = Ts[i].pDxResource;
        }
        dxSwapChain->mBackBuffers = CreateRef<RHITexture*>(Ts);
        dxSwapChain->mBufferCount = buffer_count;
        return CreateRef<RHISwapChain>(dxSwapChain);
    }
    
    // for example 
    RootSignatureRHIRef RHI_D3D12::rhi_create_root_signature(Ref<RHIDevice> pDevice, const RHIRootSignatureCreateDesc& rootSigDesc)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIRootSignature_D3D12* dxRootSignature = cyber_new<RHIRootSignature_D3D12>();

        // Pick root parameters from desc data
        ERHIShaderStages shaderStages = 0;
        for(uint32_t i = 0; i < rootSigDesc.mShaderCount; ++i)
        {
            RHIPipelineShaderCreateDesc* shader_desc = rootSigDesc.pShaderDescs + i;
            shaderStages |= shader_desc->mStage;
        }

        // Pick shader reflection data
        
    }

    BufferRHIRef RHI_D3D12::rhi_create_buffer(Ref<RHIDevice> pDevice, const BufferCreateDesc& pDesc)
    {
        RHIDevice_D3D12* DxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIAdapter_D3D12* DxAdapter = static_cast<RHIAdapter_D3D12*>(pDevice->pAdapter.get());
        //RHIRenderer_D3D12* DxRenderer = static_cast<RHIRenderer_D3D12*>(pRenderer.get());
        
        Ref<RHIBuffer_D3D12> pBuffer = CreateRef<RHIBuffer_D3D12>();

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
        DxDevice->pDxDevice->GetCopyableFootprints(&desc, 0, 1, 0, NULL, NULL, NULL, &padded_size);
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
            CHECK_HRESULT(DxDevice->pDxDevice->CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, &desc, res_states, NULL, IID_ARGS(&pBuffer->pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Committed Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", pDesc.pName ? pDesc.pName : "", allocationSize, pDesc.mFormat);
        }
        else
        {
            CHECK_HRESULT(DxDevice->pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, NULL, &pBuffer->pDxAllocation, IID_ARGS(&pBuffer->pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", pDesc.pName ? pDesc.pName : "", allocationSize, pDesc.mFormat);
        }
        
        if(pDesc.mMemoryUsage != RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY && pDesc.mFlags & RHI_BCF_PERSISTENT_MAP_BIT)
            pBuffer->pDxResource->Map(0, NULL, &pBuffer->pCpuMappedAddress);
        
        pBuffer->mDxGpuAddress = pBuffer->pDxResource->GetGPUVirtualAddress();
        
        // Create Descriptors
        if(!(pDesc.mFlags & RHI_BCF_NO_DESCRIPTOR_VIEW_CREATION))
        {
            RHIDescriptorHeap_D3D12* pHeap = DxDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
            uint32_t handleCount = ((pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ? 1 : 0) + 
                                    ((pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_BUFFER) ? 1 : 0) +
                                    ((pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_RW_BUFFER) ? 1 : 0);
            pBuffer->mDxDescriptorHandles = D3D12Util_ConsumeDescriptorHandles(pHeap, handleCount).mCpu;

            // Create CBV
            if(pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                pBuffer->mSrvDescriptorOffset = 1;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.BufferLocation = pBuffer->mDxGpuAddress;
                cbvDesc.SizeInBytes = (UINT)allocationSize;
                D3D12Util_CreateCBV(DxDevice, &cbvDesc, &pBuffer->mDxDescriptorHandles);
            }

            // Create SRV
            if(pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_BUFFER)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE srv = {pBuffer->mDxDescriptorHandles.ptr + pBuffer->mSrvDescriptorOffset};
                pBuffer->mUavDescriptorOffset = pBuffer->mSrvDescriptorOffset + pHeap->mDescriptorSize * 1;

                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Buffer.FirstElement = pDesc.mFirstElement;
                srvDesc.Buffer.NumElements = (UINT)pDesc.mElementCount;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
                srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(pDesc.mFormat);
                if(RHI_DESCRIPTOR_TYPE_BUFFER_RAW == (pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_BUFFER_RAW))
                {
                    if(pDesc.mFormat != RHI_FORMAT_UNDEFINED)
                    {
                        CB_CORE_WARN("Raw buffer use R32 typeless format. Format will be ignored");
                    }
                    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                    srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
                }
                // Cannot create a typed StructuredBuffer
                if(srvDesc.Format != DXGI_FORMAT_UNKNOWN)
                {
                    srvDesc.Buffer.StructureByteStride = 0;
                }

                D3D12Util_CreateSRV(DxDevice, pBuffer->pDxResource, &srvDesc, &srv);
            }

            // Create UAV
            if(pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_RW_BUFFER)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE uav = {pBuffer->mDxDescriptorHandles.ptr + pBuffer->mUavDescriptorOffset};

                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.FirstElement = pDesc.mFirstElement;
                uavDesc.Buffer.NumElements = (UINT)pDesc.mElementCount;
                uavDesc.Buffer.StructureByteStride = (UINT)pDesc.mStructStride;
                uavDesc.Buffer.CounterOffsetInBytes = 0;
                uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
                if(RHI_DESCRIPTOR_TYPE_RW_BUFFER_RAW == (pDesc.mDescriptors & RHI_DESCRIPTOR_TYPE_RW_BUFFER_RAW))
                {
                    if(pDesc.mFormat != RHI_FORMAT_UNDEFINED)
                        CB_CORE_WARN("Raw buffer use R32 typeless format. Format will be ignored");
                    uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                    uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                }
                else if(pDesc.mFormat != RHI_FORMAT_UNDEFINED)
                {
                    uavDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(pDesc.mFormat);
                    D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = {uavDesc.Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE};
                    HRESULT hr = DxDevice->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
                    if(!SUCCEEDED(hr) || !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) || 
                        !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE))
                    {
                        CB_CORE_WARN("Cannot use Typed UAV for buffer format [0]", (uint32_t)pDesc.mFormat);
                        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                    }
                }
                // Cannot create a typed RWStructuredBuffer
                if(uavDesc.Format != DXGI_FORMAT_UNKNOWN)
                {
                    uavDesc.Buffer.StructureByteStride = 0;
                }

                ID3D12Resource* pCounterResource = pDesc.pCounterBuffer ? static_cast<RHIBuffer_D3D12*>(pDesc.pCounterBuffer)->pDxResource : nullptr;
                D3D12Util_CreateUAV(DxDevice, pBuffer->pDxResource, pCounterResource, &uavDesc, &uav);
            }
        }

        pBuffer->mSize = (uint32_t)pDesc.mSize;
        pBuffer->mMemoryUsage = pDesc.mMemoryUsage;
        pBuffer->mDescriptors = pDesc.mDescriptors;
        return pBuffer;
    }
}