#include "rhi/backend/d3d12/rhi_d3d12.h"
#include "EASTL/vector.h"
#include <EASTL/hash_map.h>
#include <EASTL/string_hash_map.h>
#include "d3d12_utils.h"
#include "CyberLog/Log.h"
#include "dxcapi.h"
#include "math/common.h"
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <intsafe.h>
#include <stdint.h>
#include <synchapi.h>
#include "platform/memory.h"
#include "../../common/common_utils.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Cyber
{
    #define DECLARE_ZERO(type, var) type var = {};

    HRESULT RHIDevice_D3D12::hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize)
    {
        return pDxDevice->CheckFeatureSupport(pFeature, pFeatureSupportData, pFeatureSupportDataSize);
    }

    HRESULT RHIDevice_D3D12::hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource)
    {
        return pDxDevice->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

    RHI_D3D12::RHI_D3D12()
    {
        
    }

    RHI_D3D12::~RHI_D3D12()
    {

    }

    Ref<RHIDevice> RHI_D3D12::rhi_create_device(Ref<RHIAdapter> pAdapter, const RHIDeviceCreateDesc& deviceDesc)
    {
        RHIAdapter_D3D12* dxAdapter = static_cast<RHIAdapter_D3D12*>(pAdapter.get());
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(pAdapter->pInstance.get());
        Ref<RHIDevice_D3D12> dx_device_ref = CreateRef<RHIDevice_D3D12>();
        
        dx_device_ref->pAdapter = pAdapter;

        if(!SUCCEEDED(D3D12CreateDevice(dxAdapter->pDxActiveGPU, dxAdapter->mFeatureLevel, IID_PPV_ARGS(&dx_device_ref->pDxDevice))))
        {
            cyber_assert(false, "[D3D12 Fatal]: Create D3D12Device Failed!");
        }

        // Create Requested Queues
        dx_device_ref->pNullDescriptors = (RHIEmptyDescriptors_D3D12*)cyber_calloc(1, sizeof(RHIEmptyDescriptors_D3D12));

        for(uint32_t i = 0u; i < deviceDesc.queue_group_count;i++)
        {
            const auto& queueGroup = deviceDesc.queue_groups[i];
            const auto type = queueGroup.queue_type;

            dx_device_ref->pCommandQueueCounts[type] = queueGroup.queue_count;
            dx_device_ref->ppCommandQueues[type] = (ID3D12CommandQueue**)cyber_malloc(sizeof(ID3D12CommandQueue*) * queueGroup.queue_count);

            for(uint32_t j = 0u; j < queueGroup.queue_count; j++)
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
        D3D12Util_CreateDMAAllocator(dxInstance, dxAdapter, dx_device_ref.get());
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
            dx_device_ref->mCPUDescriptorHeaps[i] = (RHIDescriptorHeap_D3D12*)cyber_malloc(sizeof(RHIDescriptorHeap_D3D12));
            D3D12Util_CreateDescriptorHeap(dx_device_ref->pDxDevice, desc, &dx_device_ref->mCPUDescriptorHeaps[i]);
        }

        // Allocate NULL Descriptors
        {
            dx_device_ref->pNullDescriptors->Sampler = D3D12_DESCRIPTOR_ID_NONE;
            D3D12_SAMPLER_DESC samplerDesc = {};
            samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            dx_device_ref->pNullDescriptors->Sampler = D3D12Util_ConsumeDescriptorHandles(dx_device_ref->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1).mCpu;
            dx_device_ref->pDxDevice->CreateSampler(&samplerDesc, dx_device_ref->pNullDescriptors->Sampler);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8_UINT;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R8_UINT;

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_1D]);
            D3D12Util_CreateUAV(dx_device_ref.get(), NULL, NULL, &uavDesc, &dx_device_ref->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_1D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2D]);
            D3D12Util_CreateUAV(dx_device_ref.get(), NULL, NULL, &uavDesc, &dx_device_ref->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_2D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2DMS]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_3D]);
            D3D12Util_CreateUAV(dx_device_ref.get(), NULL, NULL, &uavDesc, &dx_device_ref->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_3D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_CUBE]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_1D_ARRAY]);
            D3D12Util_CreateUAV(dx_device_ref.get(), NULL, NULL, &uavDesc, &dx_device_ref->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_1D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2D_ARRAY]);
            D3D12Util_CreateUAV(dx_device_ref.get(), NULL, NULL, &uavDesc, &dx_device_ref->pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_2D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2DMS_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_CUBE_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            D3D12Util_CreateSRV(dx_device_ref.get(), NULL, &srvDesc, &dx_device_ref->pNullDescriptors->BufferSRV);
            D3D12Util_CreateUAV(dx_device_ref.get(), NULL, NULL, &uavDesc, &dx_device_ref->pNullDescriptors->BufferUAV);
            D3D12Util_CreateCBV(dx_device_ref.get(), NULL, &dx_device_ref->pNullDescriptors->BufferCBV);
        }

        // Pipeline cache
        D3D12_FEATURE_DATA_SHADER_CACHE feature = {};
        HRESULT result = dx_device_ref->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &feature, sizeof(feature));
        if(SUCCEEDED(result))
        {
            result = E_NOTIMPL;
            if(feature.SupportFlags & D3D12_SHADER_CACHE_SUPPORT_LIBRARY)
            {
                ID3D12Device1* device1 = NULL;
                result = dx_device_ref->pDxDevice->QueryInterface(IID_ARGS(&device1));
                if(SUCCEEDED(result))
                {
                    result = device1->CreatePipelineLibrary(dx_device_ref->pPSOCacheData, 0, IID_ARGS(&dx_device_ref->pPipelineLibrary));
                }
                SAFE_RELEASE(device1);
            }
        }

        return dx_device_ref;
    }

    void RHI_D3D12::rhi_free_device(Ref<RHIDevice> device)
    {
        RHIDevice_D3D12* dx_device = static_cast<RHIDevice_D3D12*>(device.get());
        for(uint32_t i = 0; i < RHI_QUEUE_TYPE_COUNT; ++i)
        {
            ERHIQueueType type;
            for(uint32_t j = 0; j < dx_device->pCommandQueueCounts[i]; ++j)
            {
                SAFE_RELEASE(dx_device->ppCommandQueues[i][j]);
            }
            cyber_free((ID3D12CommandQueue**)dx_device->ppCommandQueues[i]);
        }
        // Free D3D12MA Allocator
        SAFE_RELEASE(dx_device->pResourceAllocator);
        // Free Descriptor Heaps
        for(uint32_t i = 0;i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12Util_FreeDescriptorHeap(dx_device->mCPUDescriptorHeaps[i]);
        }
        D3D12Util_FreeDescriptorHeap(dx_device->mCbvSrvUavHeaps[0]);
        D3D12Util_FreeDescriptorHeap(dx_device->mSamplerHeaps[0]);
        cyber_free_map(dx_device->mCPUDescriptorHeaps);
        cyber_free_map(dx_device->mCbvSrvUavHeaps);
        cyber_free_map(dx_device->mSamplerHeaps);
        cyber_free(dx_device->pNullDescriptors);
        // Release D3D12 Device
        SAFE_RELEASE(dx_device->pDxDevice);
        SAFE_RELEASE(dx_device->pPipelineLibrary);
        if(dx_device->pPSOCacheData) cyber_free(dx_device->pPSOCacheData);
        cyber_delete(dx_device);
        device.reset();
    }

    Ref<RHITextureView> RHI_D3D12::rhi_create_texture_view(Ref<RHIDevice> device, const RHITextureViewCreateDesc& viewDesc)
    {
        Ref<RHITextureView_D3D12> tex_view = CreateRef<RHITextureView_D3D12>();
        RHITexture_D3D12* tex = static_cast<RHITexture_D3D12*>(viewDesc.texture.get());
        RHIDevice_D3D12* dx_device = static_cast<RHIDevice_D3D12*>(device.get());

        // Consume handles
        const auto usages = viewDesc.usages;
        uint32_t handleCount = ((usages & RHI_TVU_SRV) ? 1 : 0) + ((usages & RHI_TVU_UAV) ? 1 : 0);

        if(handleCount > 0)
        {
            RHIDescriptorHeap_D3D12* heap = dx_device->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
            tex_view->mDxDescriptorHandles = D3D12Util_ConsumeDescriptorHandles(heap, handleCount).mCpu;
            tex_view->mSrvDescriptorOffset = 0;
            uint64_t current_offset_cursor = tex_view->mSrvDescriptorOffset;
            // Create SRV
            if(usages & RHI_TVU_SRV)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE srv = { tex_view->mDxDescriptorHandles.ptr + tex_view->mSrvDescriptorOffset };
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.format, true);
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                switch (viewDesc.dimension)
                {
                    case RHI_TEX_DIMENSION_1D:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                        srvDesc.Texture1D.MipLevels = viewDesc.mip_level_count;
                        srvDesc.Texture1D.MostDetailedMip = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_1D_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                        srvDesc.Texture1DArray.MipLevels = viewDesc.mip_level_count;
                        srvDesc.Texture1DArray.MostDetailedMip = viewDesc.base_mip_level;
                        srvDesc.Texture1DArray.FirstArraySlice = viewDesc.base_array_layer;
                        srvDesc.Texture1DArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        srvDesc.Texture2D.MipLevels = viewDesc.mip_level_count;
                        srvDesc.Texture2D.MostDetailedMip = viewDesc.base_mip_level;
                        srvDesc.Texture2D.PlaneSlice = 0;
                        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        srvDesc.Texture2DArray.MipLevels = viewDesc.mip_level_count;
                        srvDesc.Texture2DArray.MostDetailedMip = viewDesc.base_mip_level;
                        srvDesc.Texture2DArray.FirstArraySlice = viewDesc.base_array_layer;
                        srvDesc.Texture2DArray.ArraySize = viewDesc.array_layer_count;
                        srvDesc.Texture2DArray.PlaneSlice = 0;
                        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2DMS:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2DMS_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        srvDesc.Texture2DMSArray.FirstArraySlice = viewDesc.base_array_layer;
                        srvDesc.Texture2DMSArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_3D:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                        srvDesc.Texture3D.MipLevels = viewDesc.mip_level_count;
                        srvDesc.Texture3D.MostDetailedMip = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_CUBE:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        srvDesc.TextureCube.MipLevels = viewDesc.mip_level_count;
                        srvDesc.TextureCube.MostDetailedMip = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_CUBE_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                        srvDesc.TextureCubeArray.MipLevels = viewDesc.mip_level_count;
                        srvDesc.TextureCubeArray.MostDetailedMip = viewDesc.base_mip_level;
                        srvDesc.TextureCubeArray.First2DArrayFace = viewDesc.base_array_layer;
                        srvDesc.TextureCubeArray.NumCubes = viewDesc.array_layer_count;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                    break;
                }
                D3D12Util_CreateSRV(dx_device, tex->pDxResource, &srvDesc, &srv);
                current_offset_cursor += heap->mDescriptorSize * 1;
            }
            // Create UAV
            if(usages & RHI_TVU_UAV)
            {
                tex_view->mUavDescriptorOffset = current_offset_cursor;
                current_offset_cursor += heap->mDescriptorSize * 1;
                D3D12_CPU_DESCRIPTOR_HANDLE uav = { tex_view->mDxDescriptorHandles.ptr + tex_view->mUavDescriptorOffset };
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.format, true);
                cyber_assert(viewDesc.mip_level_count <= 1, "UAVs can only be created for a single mip level");
                switch(viewDesc.dimension)
                {
                    case RHI_TEX_DIMENSION_1D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                        uavDesc.Texture1D.MipSlice = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_1D_ARRAY:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                        uavDesc.Texture1DArray.MipSlice = viewDesc.base_mip_level;
                        uavDesc.Texture1DArray.FirstArraySlice = viewDesc.base_array_layer;
                        uavDesc.Texture1DArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                        uavDesc.Texture2D.MipSlice = viewDesc.base_mip_level;
                        uavDesc.Texture2D.PlaneSlice = 0;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D_ARRAY:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        uavDesc.Texture2DArray.MipSlice = viewDesc.base_mip_level;
                        uavDesc.Texture2DArray.FirstArraySlice = viewDesc.base_array_layer;
                        uavDesc.Texture2DArray.ArraySize = viewDesc.array_layer_count;
                        uavDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    break;
                    case RHI_TEX_DIMENSION_3D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                        uavDesc.Texture3D.MipSlice = viewDesc.base_mip_level;
                        uavDesc.Texture3D.FirstWSlice = 0;
                        uavDesc.Texture3D.WSize = viewDesc.array_layer_count;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                    break;
                }
                D3D12Util_CreateUAV(dx_device, tex->pDxResource, nullptr, &uavDesc, &uav);
            }
        }

        // Create RTV
        if(usages & RHI_TVU_RTV_DSV)
        {
            const bool isDSV = FormatUtil_IsDepthStencilFormat(viewDesc.format);

            if(isDSV)
            {
                RHIDescriptorHeap_D3D12* dsv_heap = dx_device->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
                tex_view->mRtvDsvDescriptorHandle = D3D12Util_ConsumeDescriptorHandles(dsv_heap, 1).mCpu;
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.format, true);
                switch (viewDesc.dimension)
                {
                    case RHI_TEX_DIMENSION_1D:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                        dsvDesc.Texture1D.MipSlice = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_1D_ARRAY:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                        dsvDesc.Texture1DArray.MipSlice = viewDesc.base_mip_level;
                        dsvDesc.Texture1DArray.FirstArraySlice = viewDesc.base_array_layer;
                        dsvDesc.Texture1DArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                        dsvDesc.Texture2D.MipSlice = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D_ARRAY:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        dsvDesc.Texture2DArray.MipSlice = viewDesc.base_mip_level;
                        dsvDesc.Texture2DArray.FirstArraySlice = viewDesc.base_array_layer;
                        dsvDesc.Texture2DArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2DMS:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2DMS_ARRAY:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        dsvDesc.Texture2DMSArray.FirstArraySlice = viewDesc.base_array_layer;
                        dsvDesc.Texture2DMSArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                        break;
                }
                D3D12Util_CreateDSV(dx_device, tex->pDxResource, &dsvDesc, &tex_view->mRtvDsvDescriptorHandle);
            }
            else
            {
                RHIDescriptorHeap_D3D12* rtv_heap = dx_device->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
                tex_view->mRtvDsvDescriptorHandle = D3D12Util_ConsumeDescriptorHandles(rtv_heap, 1).mCpu;
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.format, true);
                switch(viewDesc.dimension)
                {
                    case RHI_TEX_DIMENSION_1D:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                        rtvDesc.Texture1D.MipSlice = viewDesc.base_mip_level;
                    }
                    break;
                    case RHI_TEX_DIMENSION_1D_ARRAY:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        rtvDesc.Texture1DArray.MipSlice = viewDesc.base_mip_level;
                        rtvDesc.Texture1DArray.FirstArraySlice = viewDesc.base_array_layer;
                        rtvDesc.Texture1DArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                        rtvDesc.Texture2D.MipSlice = viewDesc.base_mip_level;
                        rtvDesc.Texture2D.PlaneSlice = 0;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2DMS:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2D_ARRAY:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        rtvDesc.Texture2DArray.MipSlice = viewDesc.base_mip_level;
                        rtvDesc.Texture2DArray.FirstArraySlice = viewDesc.base_array_layer;
                        rtvDesc.Texture2DArray.ArraySize = viewDesc.array_layer_count;
                        rtvDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    break;
                    case RHI_TEX_DIMENSION_2DMS_ARRAY:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        rtvDesc.Texture2DMSArray.FirstArraySlice = viewDesc.base_array_layer;
                        rtvDesc.Texture2DMSArray.ArraySize = viewDesc.array_layer_count;
                    }
                    break;
                    case RHI_TEX_DIMENSION_3D:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                        rtvDesc.Texture3D.MipSlice = viewDesc.base_mip_level;
                        rtvDesc.Texture3D.FirstWSlice = viewDesc.base_array_layer;
                        rtvDesc.Texture3D.WSize = viewDesc.array_layer_count;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                        break;
                }
                D3D12Util_CreateRTV(dx_device, tex->pDxResource, &rtvDesc, &tex_view->mDxDescriptorHandles);
            }
        }
        return tex_view;
    }

    void RHI_D3D12::rhi_free_texture_view(Ref<RHITextureView> view)
    {
        RHITextureView_D3D12* tex_view = static_cast<RHITextureView_D3D12*>(view.get());
        RHIDevice_D3D12* dx_device = static_cast<RHIDevice_D3D12*>(tex_view->device.get());
        const auto usages = tex_view->create_info.usages;
        const bool isDSV = FormatUtil_IsDepthStencilFormat(tex_view->create_info.format);
        if(tex_view->mDxDescriptorHandles.ptr != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
        {
            uint32_t handleCount = ((usages & RHI_TVU_SRV) ? 1 : 0) + ((usages & RHI_TVU_UAV) ? 1 : 0);
            
        }
    }

    Texture2DRHIRef RHI_D3D12::rhi_create_texture(Ref<RHIDevice> pDevice, const TextureCreationDesc& pDesc)
    {
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

    InstanceRHIRef RHI_D3D12::rhi_create_instance(const RHIInstanceCreateDesc& instanceDesc)
    {
        Ref<RHIInstance_D3D12> instanceRef = CreateRef<RHIInstance_D3D12>();
        // Initialize driver
        D3D12Util_InitializeEnvironment(instanceRef.get());
        // Enable Debug Layer
        D3D12Util_Optionalenable_debug_layer(instanceRef.get(), instanceDesc);

        UINT flags = 0;
        if(instanceDesc.enable_debug_layer)
            flags = DXGI_CREATE_FACTORY_DEBUG;
        
        if(SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&instanceRef->pDXGIFactory))))
        {
            uint32_t gpuCount = 0;
            bool foundSoftwareAdapter = false;
            D3D12Util_QueryAllAdapters(instanceRef, gpuCount, foundSoftwareAdapter);
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

        return instanceRef;
    }

    void RHI_D3D12::rhi_free_instance(Ref<RHIInstance> instance)
    {
        RHIInstance_D3D12* dx_instance = static_cast<RHIInstance_D3D12*>(instance.get());
        D3D12Util_DeInitializeEnvironment(dx_instance);
        if(dx_instance->mAdaptersCount > 0)
        {
            for(uint32_t i = 0;i < dx_instance->mAdaptersCount; i++)
            {
                SAFE_RELEASE(dx_instance->pAdapters[i].pDxActiveGPU);
            }
        }
        cyber_free(dx_instance->pAdapters);
        SAFE_RELEASE(dx_instance->pDXGIFactory);
        if(dx_instance->pDXDebug)
        {
            SAFE_RELEASE(dx_instance->pDXDebug);
        }
        cyber_delete(dx_instance);
        instance.reset();

    #if !defined (XBOX) && defined (_WIN32)
        D3D12Util_UnloadDxcDLL();
    #endif

    #ifdef _DEBUG
        {
            IDXGIDebug1* dxgiDebug = nullptr;
            if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            }
            SAFE_RELEASE(dxgiDebug);
        }
    #endif
    }

    Ref<RHISurface> RHI_D3D12::rhi_surface_from_hwnd(Ref<RHIDevice> pDevice, HWND window)
    {
        Ref<RHISurface> surface = CreateRef<RHISurface>();
        surface.reset((RHISurface*)(&window));
        return surface;
    }

    FenceRHIRef RHI_D3D12::rhi_create_fence(Ref<RHIDevice> pDevice)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIFence_D3D12* dxFence = cyber_new<RHIFence_D3D12>();
        cyber_assert(dxFence, "Fence create failed!");
        
        CHECK_HRESULT(dxDevice->pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dxFence->pDxFence)));
        dxFence->mFenceValue = 1;

        dxFence->pDxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        return CreateRef<RHIFence_D3D12>(*dxFence);
    }

    ERHIFenceStatus RHI_D3D12::rhi_query_fence_status(Ref<RHIFence> fence)
    {
        RHIFence_D3D12* dxFence = static_cast<RHIFence_D3D12*>(fence.get());
        uint64_t fenceValue = dxFence->pDxFence->GetCompletedValue();
        if(fenceValue < dxFence->mFenceValue)
            return RHI_FENCE_STATUS_INCOMPLETE;
        else 
            return RHI_FENCE_STATUS_COMPLETE;
    }

    void RHI_D3D12::rhi_wait_fences(const Ref<RHIFence>* fences, uint32_t fenceCount)
    {
        for(uint32_t i = 0; i < fenceCount; ++i)
        {
            ERHIFenceStatus fence_status = rhi_query_fence_status(fences[i]);
            RHIFence_D3D12* dxFence = static_cast<RHIFence_D3D12*>(fences[i].get());
            uint64_t fence_value = dxFence->mFenceValue;
            if(fence_status == RHI_FENCE_STATUS_INCOMPLETE)
            {
                dxFence->pDxFence->SetEventOnCompletion(fence_value, dxFence->pDxWaitIdleFenceEvent);
                WaitForSingleObject(dxFence->pDxWaitIdleFenceEvent, INFINITE);
            }
        }
    }

    void RHI_D3D12::rhi_free_fence(Ref<RHIFence> fence)
    {
        RHIFence_D3D12* dxFence = static_cast<RHIFence_D3D12*>(fence.get());
        SAFE_RELEASE(dxFence->pDxFence);
        CloseHandle(dxFence->pDxWaitIdleFenceEvent);
        cyber_assert(fence.use_count() == 1, "Fence free failed!");
        fence.reset();
    }

    QueueRHIRef RHI_D3D12::rhi_get_queue(Ref<RHIDevice> pDevice, ERHIQueueType type, uint32_t index)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIQueue_D3D12* dxQueue = cyber_new<RHIQueue_D3D12>();
        dxQueue->pCommandQueue = dxDevice->ppCommandQueues[type][index];
        dxQueue->pFence = rhi_create_fence(pDevice);
        return CreateRef<RHIQueue_D3D12>(*dxQueue);
    }
    void RHI_D3D12::rhi_submit_queue(Ref<RHIQueue> queue, const RHIQueueSubmitDesc& submitDesc)
    {
        uint32_t cmd_count = submitDesc.mCmdsCount;
        RHIQueue_D3D12* dx_queue = static_cast<RHIQueue_D3D12*>(queue.get());
        RHIFence_D3D12* dx_fence = static_cast<RHIFence_D3D12*>(submitDesc.mSignalFence.get());

        cyber_check(submitDesc.mCmdsCounht > 0);
        cyber_check(submitDesc.pCmds);

        ID3D12CommandList** cmds = (ID3D12CommandList**)cyber_malloc(sizeof(ID3D12CommandList*) * cmd_count);
        for(uint32_t i = 0; i < cmd_count; i++)
        {
            RHICommandBuffer_D3D12* dx_cmd = static_cast<RHICommandBuffer_D3D12*>(submitDesc.pCmds[i].get());
            cmds[i] = dx_cmd->pDxCmdList;
        }
        // Wait semaphores
        for(uint32_t i = 0; i < submitDesc.mWaitSemaphoreCount; i++)
        {
            RHISemaphore_D3D12* dx_semaphore = static_cast<RHISemaphore_D3D12*>(submitDesc.pWaitSemaphores[i].get());
            dx_queue->pCommandQueue->Wait(dx_semaphore->dx_fence, dx_semaphore->fence_value - 1);
        }
        // Execute
        dx_queue->pCommandQueue->ExecuteCommandLists(cmd_count, cmds);
        // Signal fences
        if(dx_fence)
        {
            D3D12Util_SignalFence(dx_queue, dx_fence->pDxFence, dx_fence->mFenceValue++);
        }
        // Signal semaphores
        for(uint32_t i = 0; i < submitDesc.mSignalSemaphoreCount; i++)
        {
            RHISemaphore_D3D12* dx_semaphore = static_cast<RHISemaphore_D3D12*>(submitDesc.pSignalSemaphores[i].get());
            D3D12Util_SignalFence(dx_queue, dx_semaphore->dx_fence, dx_semaphore->fence_value++);
        }
    }

    void RHI_D3D12::rhi_present_queue(Ref<RHIQueue> queue, const RHIQueuePresentDesc& presentDesc)
    {
        RHISwapChain_D3D12* dx_swapchain = static_cast<RHISwapChain_D3D12*>(presentDesc.swap_chain.get());
        HRESULT hr =  dx_swapchain->pDxSwapChain->Present(dx_swapchain->mDxSyncInterval, dx_swapchain->mFlags);

        if(FAILED(hr))
        {
            CB_ERROR("Present failed!");
            #if defined(_WIN32)

            #endif
        }
    }
    void RHI_D3D12::rhi_wait_queue_idle(Ref<RHIQueue> queue)
    {
        RHIQueue_D3D12* Queue = static_cast<RHIQueue_D3D12*>(queue.get());
        RHIFence_D3D12* Fence = static_cast<RHIFence_D3D12*>(Queue->pFence.get());
        D3D12Util_SignalFence(Queue, Fence->pDxFence, Fence->mFenceValue++);

        uint64_t fence_value = Fence->mFenceValue - 1;
        if(Fence->pDxFence->GetCompletedValue() < fence_value)
        {
            Fence->pDxFence->SetEventOnCompletion(fence_value, Fence->pDxWaitIdleFenceEvent);
            WaitForSingleObject(Fence->pDxWaitIdleFenceEvent, INFINITE);
        }
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
        return CreateRef<RHICommandPool_D3D12>(*dxCommandPool);
    }
    void RHI_D3D12::rhi_reset_command_pool(Ref<RHICommandPool> pPool)
    {
        RHICommandPool_D3D12* dxPool = static_cast<RHICommandPool_D3D12*>(pPool.get());
        dxPool->pDxCmdAlloc->Reset();
    }
    void RHI_D3D12::rhi_free_command_pool(Ref<RHICommandPool> pPool)
    {
        RHICommandPool_D3D12* dxPool = static_cast<RHICommandPool_D3D12*>(pPool.get());
        SAFE_RELEASE(dxPool->pDxCmdAlloc);
        cyber_assert(pPool.use_count() == 1, "Command pool free failed!");
        pPool.reset();
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
        return CreateRef<RHICommandBuffer_D3D12>(*dxCommandBuffer);
    }

    void RHI_D3D12::rhi_free_command_buffer(Ref<RHICommandBuffer> pCommandBuffer)
    {
        RHICommandBuffer_D3D12* dxCommandBuffer = static_cast<RHICommandBuffer_D3D12*>(pCommandBuffer.get());
        SAFE_RELEASE(dxCommandBuffer->pDxCmdList);
        cyber_assert(pCommandBuffer.use_count() == 1, "Command buffer free failed!");
        pCommandBuffer.reset();
    }

    void RHI_D3D12::rhi_cmd_begin(Ref<RHICommandBuffer> pCommandBuffer)
    {
        RHICommandBuffer_D3D12* cmd = static_cast<RHICommandBuffer_D3D12*>(pCommandBuffer.get());
        RHICommandPool_D3D12* pool = static_cast<RHICommandPool_D3D12*>(cmd->pCmdPool.get());
        CHECK_HRESULT(cmd->pDxCmdList->Reset(pool->pDxCmdAlloc, nullptr));

        // Reset the descriptor heaps
        if(cmd->mType != RHI_QUEUE_TYPE_TRANSFER)
        {
            ID3D12DescriptorHeap* heaps[] = {
                cmd->pBoundHeaps[0]->pCurrentHeap,
                cmd->pBoundHeaps[1]->pCurrentHeap
            };
            cmd->pDxCmdList->SetDescriptorHeaps(2, heaps);

            cmd->mBoundHeapStartHandles[0] = cmd->pBoundHeaps[0]->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
            cmd->mBoundHeapStartHandles[1] = cmd->pBoundHeaps[1]->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
        }
        // Reset CPU side data
        cmd->pBoundRootSignature = nullptr;
    }

    void RHI_D3D12::rhi_cmd_end(Ref<RHICommandBuffer> pCommandBuffer)
    {
        RHICommandBuffer_D3D12* cmd = static_cast<RHICommandBuffer_D3D12*>(pCommandBuffer.get());
        cyber_check(cmd.pDxCmdList);
        CHECK_HRESULT(cmd->pDxCmdList->Close());
    }

    void RHI_D3D12::rhi_cmd_resource_barrier(Ref<RHICommandBuffer> cmd, const RHIResourceBarrierDesc& barrierDesc)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(cmd.get());
        const uint32_t barriers_count = barrierDesc.buffer_barrier_count + barrierDesc.texture_barrier_count;
        D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*)alloca(sizeof(D3D12_RESOURCE_BARRIER) * barriers_count);
        uint32_t transition_count = 0;
        for(uint32_t i = 0; i < barrierDesc.buffer_barrier_count; ++i)
        {
            const RHIBufferBarrier* transition_barrier = &barrierDesc.buffer_barriers[i];
            D3D12_RESOURCE_BARRIER* barrier = &barriers[transition_count];
            RHIBuffer_D3D12* buffer = static_cast<RHIBuffer_D3D12*>(transition_barrier->buffer.get());
            if(buffer->mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY ||
                buffer->mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_GPU_TO_CPU ||
                (buffer->mMemoryUsage == RHI_RESOURCE_MEMORY_USAGE_CPU_TO_GPU && buffer->mDescriptors & RHI_RESOURCE_TYPE_RW_BUFFER))
                {
                    if(transition_barrier->src_state == RHI_RESOURCE_STATE_UNORDERED_ACCESS &&
                        transition_barrier->dst_state == RHI_RESOURCE_STATE_UNORDERED_ACCESS)
                    {
                            barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                            barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            barrier->UAV.pResource = buffer->pDxResource;
                            ++transition_count;
                    }
                    else 
                    {
                            cyber_assert(transition_barrier->src_state != transition_barrier->dst_state, "D3D12 ERROR: Buffer Barrier with same src and dst state!");

                            barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                            if(transition_barrier->d3d12.begin_only)
                            {
                                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                            }
                            else if(transition_barrier->d3d12.end_only)
                            {
                                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                            }
                            else
                            {
                                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            }
                            barrier->Transition.pResource = buffer->pDxResource;
                            barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                            if(transition_barrier->queue_acquire)
                            {
                                barrier->Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
                            }
                            else 
                            {
                                barrier->Transition.StateBefore = D3D12Util_TranslateResourceState(transition_barrier->src_state);
                            }

                            if(transition_barrier->queue_release)
                            {
                                barrier->Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                            }
                            else 
                            {
                                barrier->Transition.StateAfter = D3D12Util_TranslateResourceState(transition_barrier->dst_state);
                            }

                            cyber_assert(barrier->Transition.StateBefore != barrier->Transition.StateAfter, "D3D12 ERROR: Buffer Barrier with same src and dst state!");
                            ++transition_count;
                    }
                }
        }

        for(uint32_t i = 0; i < barrierDesc.texture_barrier_count; ++i)
        {
            const RHITextureBarrier* transition_barrier = &barrierDesc.texture_barriers[i];
            D3D12_RESOURCE_BARRIER* barrier = &barriers[transition_count];
            RHITexture_D3D12* texture = static_cast<RHITexture_D3D12*>(transition_barrier->texture.get());
            if(transition_barrier->src_state == RHI_RESOURCE_STATE_UNORDERED_ACCESS &&
                transition_barrier->dst_state == RHI_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier->UAV.pResource = texture->pDxResource;
                ++transition_count;
            }
            else
            {
                cyber_assert(transition_barrier->src_state != transition_barrier->dst_state, "D3D12 ERROR: Texture Barrier with same src and dst state!");

                barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                if(transition_barrier->d3d12.begin_only)
                {
                    barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                }
                else if(transition_barrier->d3d12.end_only)
                {
                    barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                }
                barrier->Transition.pResource = texture->pDxResource;
                barrier->Transition.Subresource = transition_barrier->subresource_barrier ?
                                                 CALC_SUBRESOURCE_INDEX(transition_barrier->mip_level, transition_barrier->array_layer, 0, texture->mMipLevels, texture->mArraySize + 1)
                                                : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                if(transition_barrier->queue_acquire)
                {
                    barrier->Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
                }
                else 
                {
                    barrier->Transition.StateBefore = D3D12Util_TranslateResourceState(transition_barrier->src_state);
                }

                if(transition_barrier->queue_release)
                {
                    barrier->Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                }
                else 
                {
                    barrier->Transition.StateAfter = D3D12Util_TranslateResourceState(transition_barrier->dst_state);
                }

                if(barrier->Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON && barrier->Transition.StateAfter == D3D12_RESOURCE_STATE_COMMON)
                {
                    if(transition_barrier->src_state == RHI_RESOURCE_STATE_PRESENT || transition_barrier->dst_state == D3D12_RESOURCE_STATE_COMMON)
                    {
                        continue;
                    }
                }
                cyber_assert(false, "D3D12 ERROR: Texture Barrier with same src and dst state!");
                ++transition_count;
            }
        }
        if(transition_count > 0)
        {
            Cmd->pDxCmdList->ResourceBarrier(transition_count, barriers);
        }
    }

    void reset_root_signature(RHICommandBuffer_D3D12* cmd, ERHIPipelineType type, ID3D12RootSignature* rootSignature)
    {
        if(cmd->pBoundRootSignature != rootSignature)
        {
            cmd->pBoundRootSignature = rootSignature;
            if(type == RHI_PIPELINE_TYPE_GRAPHICS)
                cmd->pDxCmdList->SetGraphicsRootSignature(nullptr);
            else
                cmd->pDxCmdList->SetComputeRootSignature(nullptr);
        }
    }

    Ref<RHIRenderPassEncoder> RHI_D3D12::rhi_cmd_begin_render_pass(Ref<RHICommandBuffer> cmd, const RHIRenderPassDesc& beginRenderPassDesc)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(cmd.get());
    #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
        ID3D12GraphicsCommandList4* cmdList4 = (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList;
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValues[RHI_MAX_MRT_COUNT]);
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearDepth);
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearStencil);
        DECLARE_ZERO(D3D12_RENDER_PASS_RENDER_TARGET_DESC, renderPassRenderTargetDescs[RHI_MAX_MRT_COUNT]);
        DECLARE_ZERO(D3D12_RENDER_PASS_DEPTH_STENCIL_DESC, renderPassDepthStencilDesc);
        uint32_t colorTargetCount = 0;
        // color
        for(uint32_t i = 0; i < beginRenderPassDesc.render_target_count; ++i)
        {
            RHITextureView_D3D12* tex_view = static_cast<RHITextureView_D3D12*>(beginRenderPassDesc.color_attachments[i].view.get());
            clearValues[i].Format = DXGIUtil_TranslatePixelFormat(tex_view->create_info.format);
            clearValues[i].Color[0] = beginRenderPassDesc.color_attachments[i].clear_value.r;
            clearValues[i].Color[1] = beginRenderPassDesc.color_attachments[i].clear_value.g;
            clearValues[i].Color[2] = beginRenderPassDesc.color_attachments[i].clear_value.b;
            clearValues[i].Color[3] = beginRenderPassDesc.color_attachments[i].clear_value.a;
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE beginningAccess = gDx12PassBeginOpTranslator[beginRenderPassDesc.color_attachments[i].load_action];
            RHITextureView_D3D12* tex_view_resolve = static_cast<RHITextureView_D3D12*>(beginRenderPassDesc.color_attachments[i].resolve_view.get());
            if(beginRenderPassDesc.sample_count != RHI_SAMPLE_COUNT_1 && tex_view_resolve)
            {
                RHITexture_D3D12* tex = static_cast<RHITexture_D3D12*>(tex_view->create_info.texture.get());
                RHITexture_D3D12* tex_resolve = static_cast<RHITexture_D3D12*>(tex_view_resolve->create_info.texture.get());
                D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
                renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = tex_view->mRtvDsvDescriptorHandle;
                renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, clearValues[i] };
                renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess , {} };
                auto& resolve = renderPassRenderTargetDescs[colorTargetCount].EndingAccess.Resolve;
                resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
                resolve.Format = clearValues[i].Format;
                resolve.pSrcResource = tex->pDxResource;
                resolve.pDstResource = tex_resolve->pDxResource;
                Cmd->mSubResolveResource[i].SrcRect = { 0, 0, (LONG)tex->mWidth, (LONG)tex->mHeight };
                Cmd->mSubResolveResource[i].DstX = 0;
                Cmd->mSubResolveResource[i].DstY = 0;
                Cmd->mSubResolveResource[i].SrcSubresource = 0;
                Cmd->mSubResolveResource[i].DstSubresource = CALC_SUBRESOURCE_INDEX(0, 0, 0, tex_resolve->mMipLevels, tex_resolve->mArraySize + 1);
                resolve.PreserveResolveSource = false;
                resolve.SubresourceCount = 1;
                resolve.pSubresourceParameters = &Cmd->mSubResolveResource[i];
            }
            else
            {
                // Load & Store action
                D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = gDx12PassEndOpTranslator[beginRenderPassDesc.color_attachments[i].store_action];
                renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = tex_view->mRtvDsvDescriptorHandle;
                renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, clearValues[i] };
                renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess , {} };
            }
            ++colorTargetCount;
        }
        // depth stencil
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pRenderPassDepthStencilDesc = nullptr;
        if(beginRenderPassDesc.depth_stencil_attachment != nullptr && beginRenderPassDesc.depth_stencil_attachment->view != nullptr)
        {
            RHITextureView_D3D12* dt_view = static_cast<RHITextureView_D3D12*>(beginRenderPassDesc.depth_stencil_attachment->view.get());
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE depthBeginningAccess = gDx12PassBeginOpTranslator[beginRenderPassDesc.depth_stencil_attachment->depth_load_action];
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE depthEndingAccess = gDx12PassEndOpTranslator[beginRenderPassDesc.depth_stencil_attachment->depth_store_action];
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE stencilBeginningAccess = gDx12PassBeginOpTranslator[beginRenderPassDesc.depth_stencil_attachment->stencil_load_action];
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE stencilEndingAccess = gDx12PassEndOpTranslator[beginRenderPassDesc.depth_stencil_attachment->stencil_store_action];
            clearDepth.Format = DXGIUtil_TranslatePixelFormat(beginRenderPassDesc.depth_stencil_attachment->view->create_info.format);
            clearDepth.DepthStencil.Depth = beginRenderPassDesc.depth_stencil_attachment->clear_depth;
            clearStencil.Format = DXGIUtil_TranslatePixelFormat(beginRenderPassDesc.depth_stencil_attachment->view->create_info.format);
            clearStencil.DepthStencil.Stencil = beginRenderPassDesc.depth_stencil_attachment->clear_stencil;
            renderPassDepthStencilDesc.cpuDescriptor = dt_view->mRtvDsvDescriptorHandle;
            renderPassDepthStencilDesc.DepthBeginningAccess = { depthBeginningAccess, clearDepth };
            renderPassDepthStencilDesc.DepthEndingAccess = { depthEndingAccess, {} };
            renderPassDepthStencilDesc.StencilBeginningAccess = { stencilBeginningAccess, clearStencil };
            renderPassDepthStencilDesc.StencilEndingAccess = { stencilEndingAccess, {} };
            pRenderPassDepthStencilDesc = &renderPassDepthStencilDesc;
        }
        D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRenderPassRenderTargetDesc = renderPassRenderTargetDescs;
        cmdList4->BeginRenderPass(colorTargetCount, pRenderPassRenderTargetDesc, pRenderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
        return cmd;
    #endif
        cyber_warn("ID3D12GraphicsCommandList4 is not defined!");
        return cmd;
    }

    void RHI_D3D12::rhi_cmd_end_render_pass(Ref<RHICommandBuffer> pCommandBuffer)
    {
        RHICommandBuffer_D3D12* cmd = static_cast<RHICommandBuffer_D3D12*>(pCommandBuffer.get());
        #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
            ID3D12GraphicsCommandList4* cmdList4 = (ID3D12GraphicsCommandList4*)cmd->pDxCmdList;
            cmdList4->EndRenderPass();
            return;
        #endif
        cyber_warn("ID3D12GraphicsCommandList4 is not defined!");
    }
    
    void RHI_D3D12::rhi_render_encoder_bind_descriptor_set(Ref<RHIRenderPassEncoder> encoder, Ref<RHIDescriptorSet> descriptorSet)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        const RHIDescriptorSet_D3D12* Set = static_cast<const RHIDescriptorSet_D3D12*>(descriptorSet.get());
        RHIRootSignature_D3D12* RS = static_cast<RHIRootSignature_D3D12*>(Set->root_signature.get());

        cyber_check(RS);
        reset_root_signature(Cmd, RHI_PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
        if(Set->cbv_srv_uav_handle != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
        {
            Cmd->pDxCmdList->SetGraphicsRootDescriptorTable(Set->set_index, {Cmd->mBoundHeapStartHandles[0].ptr + Set->cbv_srv_uav_handle});
        }
        else if(Set->sampler_handle != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
        {
            Cmd->pDxCmdList->SetGraphicsRootDescriptorTable(Set->set_index, {Cmd->mBoundHeapStartHandles[1].ptr + Set->sampler_handle});
        }
    }
    void RHI_D3D12::rhi_render_encoder_set_viewport(Ref<RHIRenderPassEncoder> encoder, float x, float y, float width, float height, float min_depth, float max_depth)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        D3D12_VIEWPORT viewport = { x, y, width, height, min_depth, max_depth };
        Cmd->pDxCmdList->RSSetViewports(1, &viewport);
    }
    void RHI_D3D12::rhi_render_encoder_set_scissor(Ref<RHIRenderPassEncoder> encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        D3D12_RECT rect;
        rect.left = x;
        rect.top = y;
        rect.right = x + width;
        rect.bottom = y + height;
        Cmd->pDxCmdList->RSSetScissorRects(1, &rect);
    }
    void RHI_D3D12::rhi_render_encoder_bind_pipeline(Ref<RHIRenderPassEncoder> encoder, Ref<RHIRenderPipeline> pipeline)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        RHIRenderPipeline_D3D12* Pipeline = static_cast<RHIRenderPipeline_D3D12*>(pipeline.get());
        reset_root_signature(Cmd, RHI_PIPELINE_TYPE_GRAPHICS, Pipeline->pDxRootSignature);
        Cmd->pDxCmdList->IASetPrimitiveTopology(Pipeline->mPrimitiveTopologyType);
        Cmd->pDxCmdList->SetPipelineState(Pipeline->pDxPipelineState);
    }
    void RHI_D3D12::rhi_render_encoder_bind_vertex_buffer(Ref<RHIRenderPassEncoder> encoder, uint32_t buffer_count, Ref<RHIBuffer>* buffers,const uint32_t* strides, const uint32_t* offsets)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());

        DECLARE_ZERO(D3D12_VERTEX_BUFFER_VIEW, views[RHI_MAX_VERTEX_ATTRIBUTES]);
        for(uint32_t i = 0;i < buffer_count; ++i)
        {
            const RHIBuffer_D3D12* Buffer = static_cast<RHIBuffer_D3D12*>(buffers[i].get());
            cyber_check(Buffer->mDxGpuAddress != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN);

            views[i].BufferLocation = Buffer->mDxGpuAddress + (offsets ? offsets[i] : 0);
            views[i].SizeInBytes = (UINT)(Buffer->mSize - (offsets ? offsets[i] : 0));
            views[i].StrideInBytes = (UINT)strides[i];  
        }
        Cmd->pDxCmdList->IASetVertexBuffers(0, buffer_count, views);
    }
    void RHI_D3D12::rhi_render_encoder_bind_index_buffer(Ref<RHIRenderPassEncoder> encoder, Ref<RHIBuffer> buffer, uint32_t index_stride, uint64_t offset)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        const RHIBuffer_D3D12* Buffer = static_cast<RHIBuffer_D3D12*>(buffer.get());

        DECLARE_ZERO(D3D12_INDEX_BUFFER_VIEW, view);
        view.BufferLocation = Buffer->mDxGpuAddress + offset;
        view.SizeInBytes = (UINT)(Buffer->mSize - offset);
        view.Format = index_stride == sizeof(uint16_t) ? DXGI_FORMAT_R16_UINT : ((index_stride == sizeof(uint8_t) ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32_UINT));
        Cmd->pDxCmdList->IASetIndexBuffer(&view);
    }
    void RHI_D3D12::rhi_render_encoder_push_constants(Ref<RHIRenderPassEncoder> encoder, Ref<RHIRootSignature> rs, const char8_t* name, const void* data)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        RHIRootSignature_D3D12* RS = static_cast<RHIRootSignature_D3D12*>(rs.get());
        reset_root_signature(Cmd, RHI_PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
        Cmd->pDxCmdList->SetGraphicsRoot32BitConstants(RS->root_parameter_index, RS->root_constant_parameter.Constants.Num32BitValues, data, 0);
    }
    void RHI_D3D12::rhi_render_encoder_draw(Ref<RHIRenderPassEncoder> encoder, uint32_t vertex_count, uint32_t first_vertex)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        Cmd->pDxCmdList->DrawInstanced((UINT)vertex_count, (UINT)0, (UINT)first_vertex, (UINT)0);
    }
    void RHI_D3D12::rhi_render_encoder_draw_instanced(Ref<RHIRenderPassEncoder> encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        Cmd->pDxCmdList->DrawInstanced((UINT)vertex_count, (UINT)instance_count, (UINT)first_vertex, (UINT)first_instance);
    }
    void RHI_D3D12::rhi_render_encoder_draw_indexed(Ref<RHIRenderPassEncoder> encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        Cmd->pDxCmdList->DrawIndexedInstanced((UINT)index_count, (UINT)1, (UINT)first_index, (UINT)first_vertex, (UINT)0);
    }
    void RHI_D3D12::rhi_render_encoder_draw_indexed_instanced(Ref<RHIRenderPassEncoder> encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder.get());
        Cmd->pDxCmdList->DrawIndexedInstanced((UINT)index_count, (UINT)instance_count, (UINT)first_index, (UINT)first_vertex, (UINT)first_instance);
    }

    SwapChainRef RHI_D3D12::rhi_create_swap_chain(Ref<RHIDevice> pDevice, const RHISwapChainCreateDesc& desc)
    {
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(pDevice->pAdapter->pInstance.get());
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        const uint32_t buffer_count = desc.mImageCount;
        RHISwapChain_D3D12* dxSwapChain = (RHISwapChain_D3D12*)cyber_calloc(1, sizeof(RHISwapChain_D3D12) + desc.mImageCount * sizeof(RHITexture));
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

        HWND hwnd = *(HWND*)desc.surface.get();

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
        dxSwapChain->mBackBuffers = (Ref<RHITexture>*)cyber_calloc(buffer_count, sizeof(RHITexture));
        dxSwapChain->mBackBuffers[0] = CreateRef<RHITexture>(*Ts);

        dxSwapChain->mBufferCount = buffer_count;
        return CreateRef<RHISwapChain>(*dxSwapChain);
    }
    void RHI_D3D12::rhi_free_swap_chain(Ref<RHISwapChain> swapchain)
    {
        RHISwapChain_D3D12* dxSwapChain = static_cast<RHISwapChain_D3D12*>(swapchain.get());
        for(uint32_t i = 0;i < dxSwapChain->mBufferCount; ++i)
        {
            RHITexture_D3D12* dxTexture = static_cast<RHITexture_D3D12*>(dxSwapChain->mBackBuffers[i].get());
            SAFE_RELEASE(dxTexture->pDxResource);
        }
        SAFE_RELEASE(dxSwapChain->pDxSwapChain);
        cyber_free(dxSwapChain);
        swapchain.reset();
    }

    void RHI_D3D12::rhi_enum_adapters(Ref<RHIInstance> instance, RHIAdapter* const adapters, uint32_t* adapterCount)
    {
        cyber_assert(instance, "fatal: Invalid instance!");
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(instance.get());
        *adapterCount = dxInstance->mAdaptersCount;
        if(!adapters)
        {
            return;
        }
        else
        {
            for(uint32_t i = 0; i < dxInstance->mAdaptersCount; ++i)
            {
                adapters[i] = dxInstance->pAdapters[i];
            }
        }
    }

    uint32_t RHI_D3D12::rhi_acquire_next_image(Ref<RHISwapChain> pSwapChain, const RHIAcquireNextDesc& acquireDesc)
    {
        RHISwapChain_D3D12* dxSwapChain = static_cast<RHISwapChain_D3D12*>(pSwapChain.get());
        // On PC AquireNext is always true
        return dxSwapChain->pDxSwapChain->GetCurrentBackBufferIndex();
    }

    // for example 
    RootSignatureRHIRef RHI_D3D12::rhi_create_root_signature(Ref<RHIDevice> pDevice, const RHIRootSignatureCreateDesc& rootSigDesc)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        Ref<RHIRootSignature_D3D12> dxRootSignature = CreateRef<RHIRootSignature_D3D12>();

        // Pick root parameters from desc data
        ERHIShaderStages shaderStages = 0;
        for(uint32_t i = 0; i < rootSigDesc.shader_count; ++i)
        {
            RHIPipelineShaderCreateDesc* shader_desc = (rootSigDesc.shaders + i)->get();
            shaderStages |= shader_desc->stage;
        }

        // Pick shader reflection data
        rhi_util_init_root_signature_tables(dxRootSignature.get(), rootSigDesc);
        // rs pool allocation
        
        // Fill resource slots
        const uint32_t tableCount = dxRootSignature->parameter_table_count;
        uint32_t descRangeCount = 0;
        for(uint32_t i = 0;i < tableCount; ++i)
        {
            descRangeCount += dxRootSignature->parameter_tables[i].resource_count;
        }
        D3D12_ROOT_PARAMETER1* rootParams = (D3D12_ROOT_PARAMETER1*)cyber_calloc(tableCount + dxRootSignature->push_constant_count, sizeof(D3D12_ROOT_PARAMETER1));
        D3D12_DESCRIPTOR_RANGE1* descRanges = (D3D12_DESCRIPTOR_RANGE1*)cyber_calloc(descRangeCount, sizeof(D3D12_DESCRIPTOR_RANGE1));
        // Create descriptor table parameter
        uint32_t valid_root_tables = 0;
        for(uint32_t i_set = 0; i_set < tableCount; ++i_set)
        {
            RHIParameterTable* paramTable = dxRootSignature->parameter_tables + i_set;
            D3D12_ROOT_PARAMETER1 rootParam = rootParams[valid_root_tables];
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            ERHIShaderStages visStages = RHI_SHADER_STAGE_NONE;
            uint32_t i_range = 0;
            const D3D12_DESCRIPTOR_RANGE1* descRange = &descRanges[i_range];
            for(uint32_t i_register = 0; i_register < paramTable->resource_count; ++i_register)
            {
                RHIShaderResource* resourceSlot = paramTable->resources + i_register;
                visStages |= resourceSlot->stages;
                D3D12_DESCRIPTOR_RANGE1* descRange = &descRanges[i_range];
                descRange->RangeType = D3D12Util_ResourceTypeToDescriptorRangeType(resourceSlot->type);
                descRange->NumDescriptors = (resourceSlot->type != RHI_RESOURCE_TYPE_UNIFORM_BUFFER) ? resourceSlot->size : 1;
                descRange->BaseShaderRegister = resourceSlot->binding;
                descRange->RegisterSpace = resourceSlot->set;
                descRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descRange->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                rootParam.DescriptorTable.NumDescriptorRanges++;
                i_range++;
            }
            if(visStages != 0)
            {
                rootParam.ShaderVisibility = D3D12Util_TranslateShaderStage(visStages);
                rootParam.DescriptorTable.pDescriptorRanges = descRange;
                valid_root_tables++;
            }
        }
        // Create push constant parameter
        cyber_assert(dxRootSignature.push_constant_count <= 1, "Only support one push constant range");
        if(dxRootSignature->push_constant_count > 0)
        {
            auto& pushConstant = dxRootSignature->push_constants;
            dxRootSignature->root_constant_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            dxRootSignature->root_constant_parameter.ShaderVisibility = D3D12Util_TranslateShaderStage(pushConstant->stages);
            dxRootSignature->root_constant_parameter.Constants.Num32BitValues = pushConstant->size / sizeof(uint32_t);
            dxRootSignature->root_constant_parameter.Constants.ShaderRegister = pushConstant->binding;
            dxRootSignature->root_constant_parameter.Constants.RegisterSpace = pushConstant->set;
        }
        // Create static sampler parameter
        uint32_t staticSamplerCount = rootSigDesc.static_sampler_count;
        D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs = nullptr;
        if(staticSamplerCount > 0)
        {
            staticSamplerDescs = (D3D12_STATIC_SAMPLER_DESC*)cyber_calloc(staticSamplerCount, sizeof(D3D12_STATIC_SAMPLER_DESC));
            for(uint32_t i = 0;i < dxRootSignature->static_sampler_count; ++i)
            {
                auto& rst_slot = dxRootSignature->static_samplers[i];
                for(uint32_t j = 0; j < rootSigDesc.static_sampler_count; ++j)
                {
                    auto input_slot = (RHISampler_D3D12*)rootSigDesc.static_samplers[i];
                    if(strcmp(rst_slot.name, rootSigDesc.static_sampler_names[j]) == 0)
                    {
                        D3D12_SAMPLER_DESC& dxSamplerDesc = input_slot->dxSamplerDesc;
                        staticSamplerDescs[i].Filter = dxSamplerDesc.Filter;
                        staticSamplerDescs[i].AddressU = dxSamplerDesc.AddressU;
                        staticSamplerDescs[i].AddressV = dxSamplerDesc.AddressV;
                        staticSamplerDescs[i].AddressW = dxSamplerDesc.AddressW;
                        staticSamplerDescs[i].MipLODBias = dxSamplerDesc.MipLODBias;
                        staticSamplerDescs[i].MaxAnisotropy = dxSamplerDesc.MaxAnisotropy;
                        staticSamplerDescs[i].ComparisonFunc = dxSamplerDesc.ComparisonFunc;
                        staticSamplerDescs[i].MinLOD = dxSamplerDesc.MinLOD;
                        staticSamplerDescs[i].MaxLOD = dxSamplerDesc.MaxLOD;
                        staticSamplerDescs[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

                        RHIShaderResource* samplerResource = &rst_slot;
                        staticSamplerDescs[i].ShaderRegister = samplerResource->binding;
                        staticSamplerDescs[i].RegisterSpace = samplerResource->set;
                        staticSamplerDescs[i].ShaderVisibility = D3D12Util_TranslateShaderStage(samplerResource->stages);
                    }
                }
            }
        }
        bool useInputLayout = shaderStages & RHI_SHADER_STAGE_VERT; // VertexStage uses input layout
        // Fill RS flags
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        if(useInputLayout)
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        }
        if(!(shaderStages & RHI_SHADER_STAGE_VERT))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & RHI_SHADER_STAGE_HULL))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & RHI_SHADER_STAGE_DOMAIN))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & RHI_SHADER_STAGE_GEOM))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & RHI_SHADER_STAGE_FRAG))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
        }
        
        return dxRootSignature;
    }

    void RHI_D3D12::rhi_free_root_signature(Ref<RHIRootSignature> pRootSignature)
    {

    }

    uint32_t descriptor_count_needed(RHIShaderResource* resource)
    {
        if(resource->dimension == RHI_TEX_DIMENSION_1D_ARRAY ||
            resource->dimension == RHI_TEX_DIMENSION_2D_ARRAY ||
            resource->dimension == RHI_TEX_DIMENSION_2DMS_ARRAY ||
            resource->dimension == RHI_TEX_DIMENSION_CUBE_ARRAY)
        {
            return resource->size;
        }
        else 
        {
            return 1;
        }
    }

    DescriptorSetRHIRef RHI_D3D12::rhi_create_descriptor_set(Ref<RHIDevice> device, const RHIDescriptorSetCreateDesc& dSetDesc)
    {
        RHIRootSignature_D3D12* root_signature = static_cast<RHIRootSignature_D3D12*>(dSetDesc.root_signature.get());
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(device.get());
        Ref<RHIDescriptorSet_D3D12> descSet = CreateRef<RHIDescriptorSet_D3D12>();
        descSet->root_signature = dSetDesc.root_signature;
        descSet->set_index = dSetDesc.set_index;

        const uint32_t node_index = RHI_SINGLE_GPU_NODE_INDEX;
        RHIDescriptorHeap_D3D12* cbv_srv_uav_heap = dxDevice->mCbvSrvUavHeaps[node_index];
        RHIDescriptorHeap_D3D12* sampler_heap = dxDevice->mSamplerHeaps[node_index];
        RHIParameterTable* param_table = &root_signature->parameter_tables[dSetDesc.set_index];
        uint32_t cbv_srv_uav_count = 0;
        uint32_t sampler_count = 0;
        // collect descriptor counts
        for(uint32_t i = 0; i < param_table->resource_count; ++i)
        {
            if(param_table->resources[i].type == RHI_RESOURCE_TYPE_SAMPLER)
            {
                sampler_count++;
            }
            else if(param_table->resources[i].type == RHI_RESOURCE_TYPE_TEXTURE || 
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_RW_TEXTURE ||
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_BUFFER ||
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_RW_BUFFER ||
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_BUFFER_RAW ||
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_RW_BUFFER_RAW ||
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_TEXTURE_CUBE ||
                    param_table->resources[i].type == RHI_RESOURCE_TYPE_UNIFORM_BUFFER 
                    )
            {
                cbv_srv_uav_count += descriptor_count_needed(&param_table->resources[i]);
            }
        }

        // cbv/srv/uav heap
        descSet->cbv_srv_uav_handle = D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN;
        descSet->sampler_handle = D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN;
        if(cbv_srv_uav_count)
        {
            auto startHandle = D3D12Util_ConsumeDescriptorHandles(cbv_srv_uav_heap, cbv_srv_uav_count);
            descSet->cbv_srv_uav_handle = startHandle.mGpu.ptr - cbv_srv_uav_heap->mStartHandle.mGpu.ptr;
            descSet->cbv_srv_uav_stride = cbv_srv_uav_count * cbv_srv_uav_heap->mDescriptorSize;
        }
        if(sampler_count)
        {
            auto startHandle = D3D12Util_ConsumeDescriptorHandles(sampler_heap, sampler_count);
            descSet->sampler_handle = startHandle.mGpu.ptr - sampler_heap->mStartHandle.mGpu.ptr;
            descSet->sampler_stride = sampler_count * sampler_heap->mDescriptorSize;
        }
        // bind null handles on creation
        if(cbv_srv_uav_count || sampler_count)
        {
            uint32_t cbv_srv_uav_offset = 0;
            uint32_t sampler_offset = 0;
            for(uint32_t i = 0; i < param_table->resource_count; ++i)
            {
                const auto dimension = param_table->resources[i].dimension;
                auto src_handle = D3D12_DESCRIPTOR_ID_NONE;
                auto src_sampler_handle = D3D12_DESCRIPTOR_ID_NONE;
                switch (param_table->resources[i].type)
                {
                    case RHI_RESOURCE_TYPE_TEXTURE: src_handle = dxDevice->pNullDescriptors->TextureSRV[dimension]; break;
                    case RHI_RESOURCE_TYPE_BUFFER: src_handle = dxDevice->pNullDescriptors->BufferSRV; break;
                    case RHI_RESOURCE_TYPE_RW_BUFFER: src_handle = dxDevice->pNullDescriptors->BufferUAV; break;
                    case RHI_RESOURCE_TYPE_UNIFORM_BUFFER: src_handle = dxDevice->pNullDescriptors->BufferCBV; break;
                    case RHI_RESOURCE_TYPE_SAMPLER: src_sampler_handle = dxDevice->pNullDescriptors->Sampler; break;
                    default: break;
                }

                if(src_handle.ptr != D3D12_DESCRIPTOR_ID_NONE.ptr)
                {
                    for(uint32_t j = 0; j < param_table->resources[i].size; ++j)
                    {
                        D3D12Util_CopyDescriptorHandle(cbv_srv_uav_heap, src_handle, descSet->cbv_srv_uav_handle, cbv_srv_uav_offset);
                        cbv_srv_uav_offset++;
                    }
                }
                if(src_sampler_handle.ptr != D3D12_DESCRIPTOR_ID_NONE.ptr)
                {
                    for(uint32_t j = 0; j < param_table->resources[i].size; ++j)
                    {
                        D3D12Util_CopyDescriptorHandle(sampler_heap, src_sampler_handle, descSet->sampler_handle, sampler_offset);
                        sampler_offset++;
                    }
                }
            }
        }

        return descSet;
    }
    D3D12_DEPTH_STENCIL_DESC gDefaultDepthStencilDesc = {};
    D3D12_BLEND_DESC gDefaultBlendDesc = {};
    D3D12_RASTERIZER_DESC gDefaultRasterizerDesc = {};
    Ref<RHIRenderPipeline> RHI_D3D12::rhi_create_render_pipeline(Ref<RHIDevice> pDevice, const RHIRenderPipelineCreateDesc& pipelineDesc)
    {
        RHIDevice_D3D12* DxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIRootSignature_D3D12* DxRootSignature = static_cast<RHIRootSignature_D3D12*>(pipelineDesc.root_signature.get());
        Ref<RHIRenderPipeline_D3D12> pPipeline = CreateRef<RHIRenderPipeline_D3D12>();

        // Input layout
        DECLARE_ZERO(D3D12_INPUT_ELEMENT_DESC, input_elements[RHI_MAX_VERTEX_ATTRIBUTES]);
        uint32_t input_element_count = 0;
        if(pipelineDesc.vertex_layout)
        {
            eastl::string_hash_map<uint32_t> semantic_index_map;
            for(uint32_t attrib_index = 0;attrib_index < pipelineDesc.vertex_layout->attribute_count; ++attrib_index)
            {
                const RHIVertexAttribute* attribute = &pipelineDesc.vertex_layout->attributes[attrib_index];
                for(uint32_t index = 0; index < attribute->array_size; ++index)
                {
                    input_elements[input_element_count].SemanticName = attribute->semantic_name;
                    if(semantic_index_map.find(attribute->semantic_name) == semantic_index_map.end())
                    {
                        semantic_index_map[attribute->semantic_name] = 0;
                    }
                    else 
                    {
                        semantic_index_map[attribute->semantic_name]++;
                    }
                    input_elements[input_element_count].SemanticIndex = semantic_index_map[attribute->semantic_name];
                    input_elements[input_element_count].Format = DXGIUtil_TranslatePixelFormat(attribute->format);
                    input_elements[input_element_count].InputSlot = attribute->binding;
                    input_elements[input_element_count].AlignedByteOffset = attribute->offset + index * FormatUtil_BitSizeOfBlock(attribute->format) / 8;
                    if(attribute->input_rate == RHI_INPUT_RATE_INSTANCE)
                    {
                        input_elements[input_element_count].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                        input_elements[input_element_count].InstanceDataStepRate = 1;
                    }
                    else
                    {
                        input_elements[input_element_count].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                        input_elements[input_element_count].InstanceDataStepRate = 0;
                    }
                    input_element_count++;
                }
            }
        }
        DECLARE_ZERO(D3D12_INPUT_LAYOUT_DESC, input_layout_desc);
        input_layout_desc.pInputElementDescs = input_element_count ? input_elements : nullptr;
        input_layout_desc.NumElements = input_element_count;

        // Shader stages
        DECLARE_ZERO(D3D12_SHADER_BYTECODE, vertex_shader);
        DECLARE_ZERO(D3D12_SHADER_BYTECODE, pixel_shader);
        DECLARE_ZERO(D3D12_SHADER_BYTECODE, domain_shader);
        DECLARE_ZERO(D3D12_SHADER_BYTECODE, hull_shader);
        DECLARE_ZERO(D3D12_SHADER_BYTECODE, geometry_shader);
        for(uint32_t i = 0; i < 5;++i)
        {
            ERHIShaderStage stage_mask = (ERHIShaderStage)(1 << i);
            switch (stage_mask)
            {
                case RHI_SHADER_STAGE_VERT:
                {
                    if(pipelineDesc.vertex_shader)
                    {
                        RHIShaderLibrary_D3D12* vert_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.vertex_shader->library.get();
                        vertex_shader.pShaderBytecode = vert_lib->shader_blob->GetBufferPointer();
                        vertex_shader.BytecodeLength = vert_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_FRAG:
                {
                    if(pipelineDesc.fragment_shader)
                    {
                        RHIShaderLibrary_D3D12* frag_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.fragment_shader->library.get();
                        pixel_shader.pShaderBytecode = frag_lib->shader_blob->GetBufferPointer();
                        pixel_shader.BytecodeLength = frag_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_TESC:
                {
                    if(pipelineDesc.tesc_shader)
                    {
                        RHIShaderLibrary_D3D12* domain_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.tesc_shader->library.get();
                        hull_shader.pShaderBytecode = domain_lib->shader_blob->GetBufferPointer();
                        hull_shader.BytecodeLength = domain_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_TESE:
                {
                    if(pipelineDesc.tese_shader)
                    {
                        RHIShaderLibrary_D3D12* hull_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.tese_shader->library.get();
                        domain_shader.pShaderBytecode = hull_lib->shader_blob->GetBufferPointer();
                        domain_shader.BytecodeLength = hull_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_GEOM:
                {
                    if(pipelineDesc.geometry_shader)
                    {
                        RHIShaderLibrary_D3D12* geom_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.geometry_shader->library.get();
                        geometry_shader.pShaderBytecode = geom_lib->shader_blob->GetBufferPointer();
                        geometry_shader.BytecodeLength = geom_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                default:
                    cyber_assert(false, "Invalid shader stage");
                    break;
            }
        }
        // Stream out
        DECLARE_ZERO(D3D12_STREAM_OUTPUT_DESC, stream_output_desc);
        stream_output_desc.pSODeclaration = nullptr;
        stream_output_desc.NumEntries = 0;
        stream_output_desc.pBufferStrides = nullptr;
        stream_output_desc.NumStrides = 0;
        stream_output_desc.RasterizedStream = 0;
        // Sample
        DECLARE_ZERO(DXGI_SAMPLE_DESC, sample_desc);
        sample_desc.Count = (uint32_t)(pipelineDesc.sample_count ? pipelineDesc.sample_count : 1);
        sample_desc.Quality = (uint32_t)(pipelineDesc.sample_quality);
        DECLARE_ZERO(D3D12_CACHED_PIPELINE_STATE, cached_pso_desc);
        cached_pso_desc.pCachedBlob = nullptr;
        cached_pso_desc.CachedBlobSizeInBytes = 0;
        // Fill pipeline object desc
        DECLARE_ZERO(D3D12_GRAPHICS_PIPELINE_STATE_DESC, pso_desc);
        pso_desc.pRootSignature = DxRootSignature->dxRootSignature;
        // Single GPU
        pso_desc.NodeMask = RHI_SINGLE_GPU_NODE_MASK;
        pso_desc.VS = vertex_shader;
        pso_desc.PS = pixel_shader;
        pso_desc.DS = domain_shader;
        pso_desc.HS = hull_shader;
        pso_desc.GS = geometry_shader;
        pso_desc.StreamOutput = stream_output_desc;
        pso_desc.BlendState = pipelineDesc.blend_state ? D3D12Util_TranslateBlendState(pipelineDesc.blend_state.get()) : gDefaultBlendDesc;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.RasterizerState = pipelineDesc.rasterizer_state ? D3D12Util_TranslateRasterizerState(pipelineDesc.rasterizer_state.get()) : gDefaultRasterizerDesc;
        // Depth stencil
        pso_desc.DepthStencilState = pipelineDesc.depth_stencil_state ? D3D12Util_TranslateDepthStencilState(pipelineDesc.depth_stencil_state.get()) : gDefaultDepthStencilDesc;
        pso_desc.InputLayout = input_layout_desc;
        pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        pso_desc.PrimitiveTopologyType = D3D12Util_TranslatePrimitiveTopologyType(pipelineDesc.prim_topology);
        pso_desc.NumRenderTargets = (UINT)pipelineDesc.render_target_count;
        pso_desc.DSVFormat = DXGIUtil_TranslatePixelFormat(pipelineDesc.depth_stencil_format);
        pso_desc.SampleDesc = sample_desc;
        pso_desc.CachedPSO = cached_pso_desc;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        for(uint32_t i = 0; i < pso_desc.NumRenderTargets;++i)
        {
            pso_desc.RTVFormats[i] = DXGIUtil_TranslatePixelFormat(pipelineDesc.color_formats[i]);
        }
        // Create pipeline object
        HRESULT result = E_FAIL;
        wchar_t pipelineName[PSO_NAME_LENGTH] = {};
        // Cached PSO
        size_t psoShaderHash = 0;
        size_t psoRenderHash = 0;
        if(DxDevice->pPipelineLibrary)
        {
            // Calculate graphics pso shader hash
            if(vertex_shader.BytecodeLength)
                psoShaderHash = cyber_hash(vertex_shader.pShaderBytecode, vertex_shader.BytecodeLength, psoShaderHash);
            if(pixel_shader.BytecodeLength)
                psoShaderHash = cyber_hash(pixel_shader.pShaderBytecode, pixel_shader.BytecodeLength, psoShaderHash);
            if(domain_shader.BytecodeLength)
                psoShaderHash = cyber_hash(domain_shader.pShaderBytecode, domain_shader.BytecodeLength, psoShaderHash);
            if(hull_shader.BytecodeLength)
                psoShaderHash = cyber_hash(hull_shader.pShaderBytecode, hull_shader.BytecodeLength, psoShaderHash);
            if(geometry_shader.BytecodeLength)
                psoShaderHash = cyber_hash(geometry_shader.pShaderBytecode, geometry_shader.BytecodeLength, psoShaderHash);
            // Calculate graphics pso render state hash
            psoRenderHash = cyber_hash(&pso_desc.BlendState, sizeof(D3D12_BLEND_DESC), psoRenderHash);
            psoRenderHash = cyber_hash(&pso_desc.RasterizerState, sizeof(D3D12_RASTERIZER_DESC), psoRenderHash);
            psoRenderHash = cyber_hash(&pso_desc.DepthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC), psoRenderHash);
            psoRenderHash = cyber_hash(&pso_desc.SampleDesc, sizeof(DXGI_SAMPLE_DESC), psoRenderHash);
            psoRenderHash = cyber_hash(&pso_desc.DSVFormat, sizeof(DXGI_FORMAT), psoRenderHash);
            psoRenderHash = cyber_hash(pso_desc.RTVFormats, sizeof(DXGI_FORMAT) * pso_desc.NumRenderTargets, psoRenderHash);
            psoRenderHash = cyber_hash(&pso_desc.IBStripCutValue, sizeof(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE), psoRenderHash);
            psoRenderHash = cyber_hash(&pso_desc.Flags, sizeof(D3D12_PIPELINE_STATE_FLAGS), psoRenderHash);
            for(uint32_t i = 0; i < pso_desc.InputLayout.NumElements; ++i)
            {
                psoRenderHash = cyber_hash(&pso_desc.InputLayout.pInputElementDescs[i], sizeof(D3D12_INPUT_ELEMENT_DESC), psoRenderHash);
            }

            swprintf(pipelineName, PSO_NAME_LENGTH, L"%s_S%zuR%zu", "GRAPHCISPSO", psoShaderHash, psoRenderHash);
            result = DxDevice->pPipelineLibrary->LoadGraphicsPipeline(pipelineName, &pso_desc, IID_PPV_ARGS(&pPipeline->pDxPipelineState));
        }
        // Not find in cache
        if(!SUCCEEDED(result))
        {
            CHECK_HRESULT(DxDevice->pDxDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pPipeline->pDxPipelineState)));
            // Pipeline cache
            if(DxDevice->pPipelineLibrary)
            {
                CHECK_HRESULT(DxDevice->pPipelineLibrary->StorePipeline(pipelineName, pPipeline->pDxPipelineState));
            }
        }
        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        switch(pipelineDesc.prim_topology)
        {
            case RHI_PRIM_TOPO_POINT_LIST:
                topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case RHI_PRIM_TOPO_LINE_LIST:
                topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case RHI_PRIM_TOPO_LINE_STRIP:
                topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;
            case RHI_PRIM_TOPO_TRIANGLE_LIST:
                topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;
            case RHI_PRIM_TOPO_TRIANGLE_STRIP:
                topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;
            case RHI_PRIM_TOPO_PATCH_LIST:
            {
                cyber_assert(false, "RHI_PRIM_TOPO_PATCH_LIST not supported");
                break;
            }
            default:
                cyber_assert(false, "Invalid primitive topology");
                break;
        }
        pPipeline->mPrimitiveTopologyType = topology;
        pPipeline->pDxRootSignature = DxRootSignature->dxRootSignature;
        return pPipeline;
    }

    void RHI_D3D12::rhi_free_render_pipeline(Ref<RHIRenderPipeline> pipeline)
    {
        RHIRenderPipeline_D3D12* pPipeline = static_cast<RHIRenderPipeline_D3D12*>(pipeline.get());
        SAFE_RELEASE(pPipeline->pDxPipelineState);
        cyber_free(pPipeline);
    }

    void RHI_D3D12::rhi_update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateData, uint32_t count)
    {
        RHIDescriptorSet_D3D12* dxSet = static_cast<RHIDescriptorSet_D3D12*>(set);
        const RHIRootSignature_D3D12* dxRootSignature = static_cast<const RHIRootSignature_D3D12*>(set->root_signature.get());
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(set->root_signature->device.get());
        RHIParameterTable* paramTable = &dxRootSignature->parameter_tables[set->set_index];
        const uint32_t nodeIndex = RHI_SINGLE_GPU_NODE_INDEX;
        RHIDescriptorHeap_D3D12* pCbvSrvUavHeap = dxDevice->mCbvSrvUavHeaps[nodeIndex];
        RHIDescriptorHeap_D3D12* pSamplerHeap = dxDevice->mSamplerHeaps[nodeIndex];
        for(uint32_t i = 0;i < count; i++)
        {
            // Descriptor Info
            const RHIDescriptorData* pParam = updateData + i;
            RHIShaderResource* resData = nullptr;
            uint32_t heapOffset = 0;
            if(pParam->name != nullptr)
            {
                size_t argNameHash = rhi_name_hash(pParam->name, strlen(pParam->name));
                for(uint32_t j = 0;j < paramTable->resource_count; ++j)
                {
                    if(paramTable->resources[j].name_hash == argNameHash)
                    {
                        resData = &paramTable->resources[j];
                        break;
                    }
                    heapOffset += descriptor_count_needed(&paramTable->resources[j]);
                }
            }
            else
            {
                for(uint32_t j = 0; j < paramTable->resource_count; ++j)
                {
                    if(paramTable->resources[j].type == pParam->binding_type && 
                        paramTable->resources[j].binding == pParam->binding)
                    {
                        resData = &paramTable->resources[j];
                        break;
                    }
                    heapOffset += descriptor_count_needed(&paramTable->resources[j]);
                }
            }
            // Update info
            const uint32_t arrayCount = rhi_max(1u,pParam->count);
            switch(resData->type)
            {
                case RHI_RESOURCE_TYPE_SAMPLER:
                {
                    cyber_assert(pParam->samplers, "Binding Null Sampler");
                    RHISampler_D3D12** Samplers = (RHISampler_D3D12**)pParam->samplers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->samplers[arr], "Binding Null Sampler");
                        D3D12Util_CopyDescriptorHandle(pSamplerHeap, {Samplers[arr]->mDxHandle.ptr}, 
                        dxSet->sampler_handle, arr + heapOffset);
                    }
                }
                break;
                case RHI_RESOURCE_TYPE_TEXTURE:
                case RHI_RESOURCE_TYPE_TEXTURE_CUBE:
                {
                    cyber_assert(pParam->textures, "Binding Null Texture");
                    RHITextureView_D3D12** Textures = (RHITextureView_D3D12**)pParam->textures;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->textures[arr], "Binding Null Texture");
                        D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, {Textures[arr]->mDxDescriptorHandles.ptr + Textures[arr]->mSrvDescriptorOffset}, 
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case RHI_RESOURCE_TYPE_BUFFER:
                case RHI_RESOURCE_TYPE_BUFFER_RAW:
                {
                    cyber_assert(pParam->buffers, "Binding Null Buffer");
                    RHIBuffer_D3D12** Buffers = (RHIBuffer_D3D12**)pParam->buffers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->buffers[arr], "Binding Null Buffer");
                        D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, {Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mSrvDescriptorOffset}, 
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case RHI_RESOURCE_TYPE_UNIFORM_BUFFER:
                {
                    cyber_assert(pParam->buffers, "Binding Null Buffer");
                    RHIBuffer_D3D12** Buffers = (RHIBuffer_D3D12**)pParam->buffers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->buffers[arr], "Binding Null Buffer");
                        D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, {Buffers[arr]->mDxDescriptorHandles.ptr}, 
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case RHI_RESOURCE_TYPE_RW_TEXTURE:
                {
                    cyber_assert(pParam->textures, "Binding Null Texture");
                    RHITextureView_D3D12** Textures = (RHITextureView_D3D12**)pParam->textures;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->textures[arr], "Binding Null Texture");
                        D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, {Textures[arr]->mDxDescriptorHandles.ptr + Textures[arr]->mUavDescriptorOffset}, 
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case RHI_RESOURCE_TYPE_RW_BUFFER:
                case RHI_RESOURCE_TYPE_RW_BUFFER_RAW:
                {
                    cyber_assert(pParam->buffers, "Binding Null Buffer");
                    RHIBuffer_D3D12** Buffers = (RHIBuffer_D3D12**)pParam->buffers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->buffers[arr], "Binding Null Buffer");
                        D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, {Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mUavDescriptorOffset}, 
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                default:
                {
                    cyber_assert(false, "Invalid Resource Type");
                    break;
                }
            }
        }
    }

    BufferRHIRef RHI_D3D12::rhi_create_buffer(Ref<RHIDevice> pDevice, const BufferCreateDesc& pDesc)
    {
        RHIDevice_D3D12* DxDevice = static_cast<RHIDevice_D3D12*>(pDevice.get());
        RHIAdapter_D3D12* DxAdapter = static_cast<RHIAdapter_D3D12*>(pDevice->pAdapter.get());
        
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
    void RHI_D3D12::rhi_free_buffer(Ref<RHIBuffer> buffer)
    {

    }
    void RHI_D3D12::rhi_map_buffer(Ref<RHIBuffer> buffer, const RHIBufferRange* range)
    {

    }
    void RHI_D3D12::rhi_unmap_buffer(Ref<RHIBuffer> buffer)
    {

    }
    ShaderLibraryRHIRef RHI_D3D12::rhi_create_shader_library(Ref<RHIDevice> device, const struct RHIShaderLibraryCreateDesc& desc)
    {
        RHIDevice_D3D12* dxDevice = static_cast<RHIDevice_D3D12*>(device.get());
        RHIShaderLibrary_D3D12* pLibrary = cyber_new<RHIShaderLibrary_D3D12>();
        IDxcLibrary* pDxcLibrary;
        auto procDxcCreateInstance = D3D12Util_GetDxcCreateInstanceProc();
        if(!procDxcCreateInstance)
        {
            CB_CORE_ERROR("Cannot find dxc.dll");
            return nullptr;
        }

        procDxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pDxcLibrary));
        if(!pDxcLibrary)
        {
            CB_CORE_ERROR("Cannot create dxc library");
            return nullptr;
        }
        pDxcLibrary->CreateBlobWithEncodingOnHeapCopy(desc.code, (uint32_t)desc.code_size, DXC_CP_ACP, &pLibrary->shader_blob);
        pLibrary->pDevice = device;
        // Reflect shader
        D3D12Util_InitializeShaderReflection(dxDevice->pDxDevice, pLibrary, desc);

        pDxcLibrary->Release();
        return CreateRef<RHIShaderLibrary>(*pLibrary);
    }

    void RHI_D3D12::rhi_free_shader_library(Ref<RHIShaderLibrary> shaderLibrary)
    {
        RHIShaderLibrary_D3D12* dx_shader_library = static_cast<RHIShaderLibrary_D3D12*>(shaderLibrary.get());
        D3D12Util_FreeShaderReflection(dx_shader_library);
        if(dx_shader_library->shader_blob != nullptr)
        {
            dx_shader_library->shader_blob->Release();
        }
        cyber_delete(dx_shader_library);
        // ?
        shaderLibrary.reset();
    }

}