#include "graphics/backend/d3d12/render_device_d3d12.h"
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
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <intsafe.h>
#include <stdint.h>
#include <synchapi.h>
#include "platform/memory.h"
#include "../../common/common_utils.h"
#include "graphics/interface/device_context.h"
#include "graphics/interface/render_device.h"
#include "graphics/backend/d3d12/texture_d3d12.h"
#include "graphics/backend/d3d12/texture_view_d3d12.h"
#include "graphics/backend/d3d12/buffer_d3d12.h"
#include "graphics/backend/d3d12/swap_chain_d3d12.h"
#include "graphics/backend/d3d12/fence_d3d12.h"
#include "graphics/backend/d3d12/adapter_d3d12.h"
#include "graphics/backend/d3d12/instance_d3d12.h"
#include "graphics/backend/d3d12/command_buffer_d3d12.h"
#include "graphics/backend/d3d12/command_pool_d3d12.h"
#include "graphics/backend/d3d12/frame_buffer_d3d12.h"
#include "graphics/backend/d3d12/query_pool_d3d12.h"
#include "graphics/backend/d3d12/queue_d3d12.h"
#include "graphics/backend/d3d12/adapter_d3d12.h"


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Cyber
{
    namespace RenderObject
    {
    #define DECLARE_ZERO(type, var) type var = {};

    RenderDevice_D3D12_Impl::~RenderDevice_D3D12_Impl()
    {
        free_device();
    }

    void RenderDevice_D3D12_Impl::create_device(RenderObject::IAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc)
    {
        RenderObject::Adapter_D3D12_Impl* dxAdapter = static_cast<RenderObject::Adapter_D3D12_Impl*>(adapter);
        Instance_D3D12_Impl* dxInstance = static_cast<Instance_D3D12_Impl*>(dxAdapter->m_pInstance);
        
        this->m_pAdapter = adapter;

        if(!SUCCEEDED(D3D12CreateDevice(dxAdapter->m_pDxActiveGPU, dxAdapter->m_FeatureLevel, IID_PPV_ARGS(&m_pDxDevice))))
        {
            cyber_assert(false, "[D3D12 Fatal]: Create D3D12Device Failed!");
        }

        // Create Requested Queues
        m_pNullDescriptors = (RHIEmptyDescriptors_D3D12*)cyber_calloc(1, sizeof(RHIEmptyDescriptors_D3D12));

        for(uint32_t i = 0u; i < deviceDesc.queue_group_count;i++)
        {
            const auto& queueGroup = deviceDesc.queue_groups[i];
            const auto type = queueGroup.queue_type;

            pCommandQueueCounts[type] = queueGroup.queue_count;
            ppCommandQueues[type] = (ID3D12CommandQueue**)cyber_malloc(sizeof(ID3D12CommandQueue*) * queueGroup.queue_count);

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
                queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

                if(!SUCCEEDED(pDxDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&ppCommandQueues[type][j]))))
                {
                    cyber_assert(false, "[D3D12 Fatal]: Create CommandQueue Failed!");
                }
            }
        }

        // Create D3D12MA Allocator
        D3D12Util_CreateDMAAllocator(dxInstance, dxAdapter, this);
        cyber_assert(pResourceAllocator, "DMA Allocator Must be Created!");

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
            mCPUDescriptorHeaps[i] = (RHIDescriptorHeap_D3D12*)cyber_malloc(sizeof(RHIDescriptorHeap_D3D12));
            D3D12Util_CreateDescriptorHeap(this, desc, &mCPUDescriptorHeaps[i]);
        }
        
        // One shader visible heap for each linked node
        for(uint32_t i = 0; i < RHI_SINGLE_GPU_NODE_COUNT; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = RHI_SINGLE_GPU_NODE_MASK;
            desc.NumDescriptors = 1 << 16;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

            mCbvSrvUavHeaps[i] = (RHIDescriptorHeap_D3D12*)cyber_malloc(sizeof(RHIDescriptorHeap_D3D12));
            D3D12Util_CreateDescriptorHeap(this, desc, &mCbvSrvUavHeaps[i]);

            desc.NumDescriptors = 1 << 11;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            mSamplerHeaps[i] = (RHIDescriptorHeap_D3D12*)cyber_malloc(sizeof(RHIDescriptorHeap_D3D12));
            D3D12Util_CreateDescriptorHeap(this, desc, &mSamplerHeaps[i]);
        }

        // Allocate NULL Descriptors
        {
            pNullDescriptors->Sampler = D3D12_DESCRIPTOR_ID_NONE;
            D3D12_SAMPLER_DESC samplerDesc = {};
            samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            pNullDescriptors->Sampler = D3D12Util_ConsumeDescriptorHandles(mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1).mCpu;
            pDxDevice->CreateSampler(&samplerDesc, pNullDescriptors->Sampler);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8_UINT;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R8_UINT;

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_1D]);
            D3D12Util_CreateUAV(this, NULL, NULL, &uavDesc, &pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_1D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2D]);
            D3D12Util_CreateUAV(this, NULL, NULL, &uavDesc, &pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_2D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2DMS]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_3D]);
            D3D12Util_CreateUAV(this, NULL, NULL, &uavDesc, &pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_3D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_CUBE]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_1D_ARRAY]);
            D3D12Util_CreateUAV(this, NULL, NULL, &uavDesc, &pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_1D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2D_ARRAY]);
            D3D12Util_CreateUAV(this, NULL, NULL, &uavDesc, &pNullDescriptors->TextureUAV[RHI_TEX_DIMENSION_2D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_2DMS_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->TextureSRV[RHI_TEX_DIMENSION_CUBE_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            D3D12Util_CreateSRV(this, NULL, &srvDesc, &pNullDescriptors->BufferSRV);
            D3D12Util_CreateUAV(this, NULL, NULL, &uavDesc, &pNullDescriptors->BufferUAV);
            D3D12Util_CreateCBV(this, NULL, &pNullDescriptors->BufferCBV);
        }

        // Pipeline cache
        D3D12_FEATURE_DATA_SHADER_CACHE feature = {};
        HRESULT result = pDxDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &feature, sizeof(feature));
        if(SUCCEEDED(result))
        {
            result = E_NOTIMPL;
            if(feature.SupportFlags & D3D12_SHADER_CACHE_SUPPORT_LIBRARY)
            {
                ID3D12Device1* device1 = NULL;
                result = pDxDevice->QueryInterface(IID_ARGS(&device1));
                if(SUCCEEDED(result))
                {
                    result = device1->CreatePipelineLibrary(pPSOCacheData, 0, IID_ARGS(&pPipelineLibrary));
                }
                SAFE_RELEASE(device1);
            }
        }
    }

    void RenderDevice_D3D12_Impl::free_device()
    {
        for(uint32_t i = 0; i < QUEUE_TYPE::QUEUE_TYPE_COUNT; ++i)
        {
            ERHIQueueType type;
            for(uint32_t j = 0; j < pCommandQueueCounts[i]; ++j)
            {
                SAFE_RELEASE(ppCommandQueues[i][j]);
            }
            cyber_free((ID3D12CommandQueue**)ppCommandQueues[i]);
        }
        // Free D3D12MA Allocator
        SAFE_RELEASE(pResourceAllocator);
        // Free Descriptor Heaps
        for(uint32_t i = 0;i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12Util_FreeDescriptorHeap(mCPUDescriptorHeaps[i]);
        }
        D3D12Util_FreeDescriptorHeap(mCbvSrvUavHeaps[0]);
        D3D12Util_FreeDescriptorHeap(mSamplerHeaps[0]);
        cyber_free_map(mCPUDescriptorHeaps);
        cyber_free_map(mCbvSrvUavHeaps);
        cyber_free_map(mSamplerHeaps);
        cyber_free(pNullDescriptors);
        // Release D3D12 Device
        SAFE_RELEASE(pDxDevice);
        SAFE_RELEASE(pPipelineLibrary);
        if(pPSOCacheData) cyber_free(pPSOCacheData);
    }

    RenderObject::ITextureView* RenderDevice_D3D12_Impl::create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc)
    {
        RenderObject::TextureView_D3D12_Impl* tex_view = cyber_new<RenderObject::TextureView_D3D12_Impl>(this);
        tex_view->create_desc = viewDesc;
        RenderObject::Texture_D3D12_Impl* tex = static_cast<RenderObject::Texture_D3D12_Impl*>(viewDesc.texture);

        // Consume handles
        const auto usages = viewDesc.usages;
        uint32_t handleCount = ((usages & RHI_TVU_SRV) ? 1 : 0) + ((usages & RHI_TVU_UAV) ? 1 : 0);

        if(handleCount > 0)
        {
            RHIDescriptorHeap_D3D12* heap = mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
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
                D3D12Util_CreateSRV(this, tex->native_resource, &srvDesc, &srv);
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
                D3D12Util_CreateUAV(this, tex->native_resource, nullptr, &uavDesc, &uav);
            }
        }

        // Create RTV
        if(usages & RHI_TVU_RTV_DSV)
        {
            const bool isDSV = FormatUtil_IsDepthStencilFormat(viewDesc.format);

            if(isDSV)
            {
                RHIDescriptorHeap_D3D12* dsv_heap = mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
                tex_view->mRtvDsvDescriptorHandle = D3D12Util_ConsumeDescriptorHandles(dsv_heap, 1).mCpu;
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.format, false);
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
                D3D12Util_CreateDSV(this, tex->native_resource, &dsvDesc, &tex_view->mRtvDsvDescriptorHandle);
            }
            else
            {
                RHIDescriptorHeap_D3D12* rtv_heap = mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
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
                D3D12Util_CreateRTV(this, tex->native_resource, &rtvDesc, &tex_view->mRtvDsvDescriptorHandle);
            }
        }
        return tex_view;
    }

    void RenderDevice_D3D12_Impl::free_texture_view(RenderObject::ITextureView* view)
    {
        RenderObject::TextureView_D3D12_Impl* tex_view = static_cast<RenderObject::TextureView_D3D12_Impl*>(view);
        const auto usages = tex_view->create_desc.usages;
        const bool isDSV = FormatUtil_IsDepthStencilFormat(tex_view->create_desc.format);
        if(tex_view->mDxDescriptorHandles.ptr != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
        {
            uint32_t handleCount = ((usages & RHI_TVU_SRV) ? 1 : 0) + ((usages & RHI_TVU_UAV) ? 1 : 0);
            
        }
    }

    RenderObject::ITexture* RenderDevice_D3D12_Impl::create_texture(const RenderObject::TextureCreateDesc& pDesc)
    {
        RenderObject::Texture_D3D12_Impl* pTexture = cyber_new<RenderObject::Texture_D3D12_Impl>(this);
        cyber_assert(pTexture != nullptr, "rhi texture create failed!");

        D3D12_RESOURCE_DESC desc = {};

        //TODO:
        DXGI_FORMAT dxFormat = DXGIUtil_TranslatePixelFormat(pDesc.format);
        
        ERHIDescriptorType descriptors = pDesc.descriptors;

        if(pDesc.native_handle == nullptr)
        {
            D3D12_RESOURCE_DIMENSION res_dim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
            if(pDesc.flags & RHI_TCF_FORCE_2D)
            {
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }
            else if(pDesc.flags & RHI_TCF_FORCE_3D)
            {
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            }
            else
            {
                if(pDesc.depth > 1)
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                else if(pDesc.height > 1)
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                else
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            }

            desc.Dimension = res_dim;
            desc.Alignment = (UINT)pDesc.sample_count > 1 ? D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT : 0;
            desc.Width = pDesc.width;
            desc.Height = pDesc.height;
            desc.DepthOrArraySize = (UINT)(pDesc.array_size != 1 ? pDesc.array_size : pDesc.depth);
            desc.MipLevels = (UINT)pDesc.mip_levels;
            desc.Format = DXGIUtil_FormatToTypeless(dxFormat);
            desc.SampleDesc.Count = (UINT)pDesc.sample_count;
            desc.SampleDesc.Quality = (UINT)pDesc.sample_quality;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS data;
            data.Format = desc.Format;
            data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
            data.SampleCount = desc.SampleDesc.Count;
            hook_CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(data));
            while(data.NumQualityLevels == 0 && data.SampleCount > 0)
            {
                CB_CORE_WARN("Sample Count [0] not supported. Trying a lower sample count [1]", data.SampleCount, data.SampleCount / 2);
                data.SampleCount = desc.SampleDesc.Count / 2;
                hook_CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(data));
            }

            desc.SampleDesc.Count = data.SampleCount;

            ERHIResourceState actualStartState = (ERHIResourceState)pDesc.start_state;

            // Decide UAV flags
            if(descriptors & RHI_DESCRIPTOR_TYPE_RW_TEXTURE)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            // Decide render target flags
            if(pDesc.start_state & RHI_RESOURCE_STATE_RENDER_TARGET)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                actualStartState = (ERHIResourceState)((pDesc.start_state > RHI_RESOURCE_STATE_RENDER_TARGET)
                                    ? (pDesc.start_state & (ERHIResourceState)~RHI_RESOURCE_STATE_RENDER_TARGET)
                                    : RHI_RESOURCE_STATE_RENDER_TARGET);
            }
            else if(pDesc.start_state & RHI_RESOURCE_STATE_DEPTH_WRITE)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                actualStartState = (ERHIResourceState)((pDesc.start_state > RHI_RESOURCE_STATE_DEPTH_WRITE)
                                    ? (pDesc.start_state & (ERHIResourceState)~RHI_RESOURCE_STATE_DEPTH_WRITE)
                                    : RHI_RESOURCE_STATE_DEPTH_WRITE);
            }

            // Decide sharing flags for multi adapter
            if(pDesc.flags & RHI_TCF_EXPORT_ADAPTER_BIT)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            }

            if(pDesc.flags & RHI_TCF_FORCE_ALLOW_DISPLAY_TARGET)
            {
                actualStartState = RHI_RESOURCE_STATE_PRESENT;
            }

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = dxFormat;
            if(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                clearValue.DepthStencil.Depth = pDesc.clear_value.depth;
                clearValue.DepthStencil.Stencil = (UINT8)pDesc.clear_value.stencil;
            }
            else
            {
                clearValue.Color[0] = pDesc.clear_value.r;
                clearValue.Color[1] = pDesc.clear_value.g;
                clearValue.Color[2] = pDesc.clear_value.b;
                clearValue.Color[3] = pDesc.clear_value.a;
            }

            D3D12_CLEAR_VALUE* pClearValue = nullptr;
            D3D12_RESOURCE_STATES res_states = D3D12Util_TranslateResourceState(actualStartState);

            if((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            {
                pClearValue = &clearValue;
            }

            D3D12MA::ALLOCATION_DESC alloc_desc = {};
            alloc_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            if(pDesc.flags & RHI_TCF_OWN_MEMORY_BIT)
                alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;

            // Create resource
            auto hRes = pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, pClearValue, &pTexture->allocation, IID_ARGS(&pTexture->native_resource));
            if(hRes != S_OK)
            {
                auto fallbackhRes = hRes;
                CB_CORE_ERROR("[D3D12] Create Texture Resource Failed With HRESULT {0}! \n\t With Name: {1} \n\t Size: {2}{3} \n\t Format: {4} \n\t Sample Count: {5}", 
                                hRes, (char*)pDesc.name ? (char*)pDesc.name : "", pDesc.width, pDesc.height,
                                pDesc.format, pDesc.sample_count);
                const bool use_fallback_commited = true;
                if(use_fallback_commited)
                {
                    D3D12_HEAP_PROPERTIES heapProps = {};
                    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
                    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    heapProps.CreationNodeMask = RHI_SINGLE_GPU_NODE_MASK;
                    heapProps.VisibleNodeMask = RHI_SINGLE_GPU_NODE_MASK;
                    fallbackhRes = hook_CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, &desc, res_states, pClearValue, IID_ARGS(&pTexture->native_resource));
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
                CB_CORE_TRACE("[D3D12] Create Texture Resource Succeed! \n\t With Name: {0}\n\t Size: {1}x{2} \n\t Format: {3} \n\t Sample Count: {4}", 
                                (char*)pDesc.name ? (char*)pDesc.name : "", pDesc.width, pDesc.height,
                                pDesc.format, pDesc.sample_count);
            }


        }

        return pTexture;
    }

    IInstance* RenderDevice_D3D12_Impl::create_instance(const InstanceCreateDesc& instanceDesc)
    {
        Instance_D3D12_Impl* instance = cyber_new<Instance_D3D12_Impl>();
        // Initialize driver
        D3D12Util_InitializeEnvironment(instance);
        // Enable Debug Layer
        D3D12Util_Optionalenable_debug_layer(instance, instanceDesc);

        UINT flags = 0;
        if(instanceDesc.enable_debug_layer)
            flags = DXGI_CREATE_FACTORY_DEBUG;
        
        if(SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&instance->pDXGIFactory))))
        {
            uint32_t gpuCount = 0;
            bool foundSoftwareAdapter = false;
            D3D12Util_QueryAllAdapters(instance, gpuCount, foundSoftwareAdapter);
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

        return instance;
    }

    void RenderDevice_D3D12_Impl::free_instance(IInstance* instance)
    {
        Instance_D3D12_Impl* dx_instance = static_cast<Instance_D3D12_Impl*>(instance);
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

    RHISurface* RenderDevice_D3D12_Impl::surface_from_hwnd(HWND window)
    {
        RHISurface* surface = cyber_new<RHISurface>();
        surface->handle = window;
        //surface = (RHISurface*)(&window);
        return surface;
    }

    IFence* RenderDevice_D3D12_Impl::create_fence()
    {
        Fence_D3D12_Impl* dxFence = cyber_new<Fence_D3D12_Impl>();
        cyber_assert(dxFence, "Fence create failed!");
        CHECK_HRESULT(pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dxFence->pDxFence)));
        dxFence->mFenceValue = 0;

        dxFence->pDxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        return dxFence;
    }

    ERHIFenceStatus RenderDevice_D3D12_Impl::query_fence_status(IFence* fence)
    {
        Fence_D3D12_Impl* dxFence = static_cast<Fence_D3D12_Impl*>(fence);
        uint64_t fenceValue = dxFence->pDxFence->GetCompletedValue();
        if(fenceValue < dxFence->mFenceValue)
            return RHI_FENCE_STATUS_INCOMPLETE;
        else 
            return RHI_FENCE_STATUS_COMPLETE;
    }

    void RenderDevice_D3D12_Impl::wait_fences(IFence** fences, uint32_t fenceCount)
    {
        for(uint32_t i = 0; i < fenceCount; ++i)
        {
            ERHIFenceStatus fence_status = query_fence_status(fences[i]);
            Fence_D3D12_Impl* dxFence = static_cast<Fence_D3D12_Impl*>(fences[i]);
            uint64_t fence_value = dxFence->mFenceValue;
            if(fence_status == RHI_FENCE_STATUS_INCOMPLETE)
            {
                dxFence->pDxFence->SetEventOnCompletion(fence_value, dxFence->pDxWaitIdleFenceEvent);
                WaitForSingleObject(dxFence->pDxWaitIdleFenceEvent, INFINITE);
            }
        }
    }

    void RenderDevice_D3D12_Impl::free_fence(IFence* fence)
    {
        Fence_D3D12_Impl* dxFence = static_cast<Fence_D3D12_Impl*>(fence);
        SAFE_RELEASE(dxFence->pDxFence);
        CloseHandle(dxFence->pDxWaitIdleFenceEvent);
        cyber_delete(fence);
    }

    IQueue* RenderDevice_D3D12_Impl::get_queue(QUEUE_TYPE type, uint32_t index)
    {
        Queue_D3D12_Impl* dxQueue = cyber_new<Queue_D3D12_Impl>(this);
        dxQueue->pCommandQueue = ppCommandQueues[type][index];
        dxQueue->pFence = create_fence();
        return dxQueue;
    }
    void RenderDevice_D3D12_Impl::submit_queue(IQueue* queue, const QueueSubmitDesc& submitDesc)
    {
        uint32_t cmd_count = submitDesc.mCmdsCount;
        Queue_D3D12_Impl* dx_queue = static_cast<Queue_D3D12_Impl*>(queue);
        RHIFence_D3D12* dx_fence = static_cast<RHIFence_D3D12*>(submitDesc.mSignalFence);

        cyber_check(submitDesc.mCmdsCount > 0);
        cyber_check(submitDesc.pCmds);

        ID3D12CommandList** cmds = (ID3D12CommandList**)cyber_malloc(sizeof(ID3D12CommandList*) * cmd_count);
        for(uint32_t i = 0; i < cmd_count; i++)
        {
            RHICommandBuffer_D3D12* dx_cmd = static_cast<RHICommandBuffer_D3D12*>(submitDesc.pCmds[i]);
            cmds[i] = dx_cmd->pDxCmdList;
        }
        // Wait semaphores
        for(uint32_t i = 0; i < submitDesc.mWaitSemaphoreCount; i++)
        {
            RHISemaphore_D3D12* dx_semaphore = static_cast<RHISemaphore_D3D12*>(submitDesc.pWaitSemaphores[i]);
            dx_queue->pCommandQueue->Wait(dx_semaphore->dx_fence, dx_semaphore->fence_value - 1);
        }
        // Execute
        dx_queue->pCommandQueue->ExecuteCommandLists(cmd_count, cmds);
        // Signal fences
        if(dx_fence)
        {
            D3D12Util_SignalFence(dx_queue, dx_fence->pDxFence, ++dx_fence->mFenceValue);
        }
        // Signal semaphores
        for(uint32_t i = 0; i < submitDesc.mSignalSemaphoreCount; i++)
        {
            RHISemaphore_D3D12* dx_semaphore = static_cast<RHISemaphore_D3D12*>(submitDesc.pSignalSemaphores[i]);
            D3D12Util_SignalFence(dx_queue, dx_semaphore->dx_fence, dx_semaphore->fence_value++);
        }
    }

    void RenderDevice_D3D12_Impl::present_queue(IQueue* queue, const QueuePresentDesc& presentDesc)
    {
        SawpChain_D3D12_Impl* dx_swapchain = static_cast<SawpChain_D3D12_Impl*>(presentDesc.swap_chain);
        
        HRESULT hr =  dx_swapchain->pDxSwapChain->Present(0, dx_swapchain->mFlags);

        if(FAILED(hr))
        {
            CB_ERROR("Present failed!");
            #if defined(_WIN32)

            #endif
        }
    }
    void RenderDevice_D3D12_Impl::wait_queue_idle(IQueue* queue)
    {
        RHIQueue_D3D12* Queue = static_cast<RHIQueue_D3D12*>(queue);
        RHIFence_D3D12* Fence = static_cast<RHIFence_D3D12*>(Queue->pFence);
        D3D12Util_SignalFence(Queue, Fence->pDxFence, Fence->mFenceValue++);

        uint64_t fence_value = Fence->mFenceValue - 1;
        if(Fence->pDxFence->GetCompletedValue() < fence_value)
        {
            Fence->pDxFence->SetEventOnCompletion(fence_value, Fence->pDxWaitIdleFenceEvent);
            WaitForSingleObject(Fence->pDxWaitIdleFenceEvent, INFINITE);
        }
    }

    // Command Objects
    void allocate_transient_command_allocator(ID3D12Device* d3d12_device, CommandPool_D3D12* commandPool, IQueue* queue)
    {
        D3D12_COMMAND_LIST_TYPE type = queue->mType == RHI_QUEUE_TYPE_TRANSFER ? D3D12_COMMAND_LIST_TYPE_COPY : 
                            (queue->mType == RHI_QUEUE_TYPE_COMPUTE ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT);

        bool res = SUCCEEDED(d3d12_device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandPool->pDxCmdAlloc)));
        if(!res)
        {
            cyber_assert(false, "command allocator create failed!");
        }
        commandPool->pQueue = queue;
    }

    RHICommandPool* RenderDevice_D3D12_Impl::create_command_pool(IQueue* queue, const CommandPoolCreateDesc& commandPoolDesc)
    {
        RHICommandPool_D3D12* dxCommandPool = cyber_new<RHICommandPool_D3D12>();
        allocate_transient_command_allocator(pDxDevice, dxCommandPool, queue);
        return dxCommandPool;
    }
    void RenderDevice_D3D12_Impl::reset_command_pool(ICommandPool* pool)
    {
        RHICommandPool_D3D12* dxPool = static_cast<RHICommandPool_D3D12*>(pool);
        dxPool->pDxCmdAlloc->Reset();
    }
    void RenderDevice_D3D12_Impl::free_command_pool(ICommandPool* pool)
    {
        RHICommandPool_D3D12* dxPool = static_cast<RHICommandPool_D3D12*>(pool);
        SAFE_RELEASE(dxPool->pDxCmdAlloc);
        cyber_delete(pool);
    }

    RHICommandBuffer* RenderDevice_D3D12_Impl::create_command_buffer(ICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc) 
    {
        RHICommandBuffer_D3D12* dxCommandBuffer = cyber_new<RHICommandBuffer_D3D12>();
        RHICommandPool_D3D12* dxPool = static_cast<RHICommandPool_D3D12*>(pool);
        RHIQueue_D3D12* dxQueue = static_cast<RHIQueue_D3D12*>(dxPool->pQueue);

        // set command pool of new command
        dxCommandBuffer->mNodeIndex = RHI_SINGLE_GPU_NODE_INDEX;
        dxCommandBuffer->mType = dxQueue->mType;
        dxCommandBuffer->pBoundHeaps[0] = mCbvSrvUavHeaps[dxCommandBuffer->mNodeIndex];
        dxCommandBuffer->pBoundHeaps[1] = mSamplerHeaps[dxCommandBuffer->mNodeIndex];
        dxCommandBuffer->pCmdPool = pool;

        uint32_t nodeMask = dxCommandBuffer->mNodeIndex;
        ID3D12PipelineState* initialState = nullptr;
        CHECK_HRESULT(pDxDevice->CreateCommandList(nodeMask,gDx12CmdTypeTranslator[dxCommandBuffer->mType] , 
                dxPool->pDxCmdAlloc, initialState, IID_PPV_ARGS(&dxCommandBuffer->pDxCmdList)));
        
        // Command lists are add in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        CHECK_HRESULT(dxCommandBuffer->pDxCmdList->Close());
        return dxCommandBuffer;
    }

    void RenderDevice_D3D12_Impl::free_command_buffer(ICommandBuffer* commandBuffer)
    {
        RHICommandBuffer_D3D12* dxCommandBuffer = static_cast<RHICommandBuffer_D3D12*>(commandBuffer);
        SAFE_RELEASE(dxCommandBuffer->pDxCmdList);
        cyber_delete(commandBuffer);
    }

    void RenderDevice_D3D12_Impl::cmd_begin(ICommandBuffer* commandBuffer)
    {
        RHICommandBuffer_D3D12* cmd = static_cast<RHICommandBuffer_D3D12*>(commandBuffer);
        RHICommandPool_D3D12* pool = static_cast<RHICommandPool_D3D12*>(cmd->pCmdPool);
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

    void RenderDevice_D3D12_Impl::cmd_end(ICommandBuffer* commandBuffer)
    {
        RHICommandBuffer_D3D12* cmd = static_cast<RHICommandBuffer_D3D12*>(commandBuffer);
        cyber_check(cmd->pDxCmdList);
        CHECK_HRESULT(cmd->pDxCmdList->Close());
    }

    void RenderDevice_D3D12_Impl::cmd_resource_barrier(ICommandBuffer* cmd, const ResourceBarrierDesc& barrierDesc)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(cmd);
        const uint32_t barriers_count = barrierDesc.buffer_barrier_count + barrierDesc.texture_barrier_count;
        D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*)alloca(sizeof(D3D12_RESOURCE_BARRIER) * barriers_count);
        uint32_t transition_count = 0;
        for(uint32_t i = 0; i < barrierDesc.buffer_barrier_count; ++i)
        {
            const RHIBufferBarrier* transition_barrier = &barrierDesc.buffer_barriers[i];
            D3D12_RESOURCE_BARRIER* barrier = &barriers[transition_count];
            RenderObject::Buffer_D3D12_Impl* buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(transition_barrier->buffer);
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
            RenderObject::Texture_D3D12_Impl* texture = static_cast<RenderObject::Texture_D3D12_Impl*>(transition_barrier->texture);
            if(transition_barrier->src_state == RHI_RESOURCE_STATE_UNORDERED_ACCESS &&
                transition_barrier->dst_state == RHI_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier->UAV.pResource = texture->native_resource;
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
                barrier->Transition.pResource = texture->native_resource;
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
                //cyber_assert(false, "D3D12 ERROR: Texture Barrier with same src and dst state!");
                ++transition_count;
            }
        }
        if(transition_count > 0)
        {
            Cmd->pDxCmdList->ResourceBarrier(transition_count, barriers);
        }
    }

    void reset_root_signature(CommandBuffer_D3D12_Impl* cmd, PIPELINE_TYPE type, ID3D12RootSignature* rootSignature)
    {
        if(cmd->pBoundRootSignature != rootSignature)
        {
            cmd->pBoundRootSignature = rootSignature;
            if(type == PIPELINE_TYPE_GRAPHICS)
                cmd->pDxCmdList->SetGraphicsRootSignature(rootSignature);
            else
                cmd->pDxCmdList->SetComputeRootSignature(rootSignature);
        }
    }

    RHIRenderPassEncoder* RenderDevice_D3D12_Impl::cmd_begin_render_pass(ICommandBuffer* cmd, const RenderPassDesc& beginRenderPassDesc)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(cmd);
    #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
        ID3D12GraphicsCommandList4* cmdList4 = (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList;
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValues[RHI_MAX_MRT_COUNT]);
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearDepth);
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearStencil);
        DECLARE_ZERO(D3D12_RENDER_PASS_RENDER_TARGET_DESC, renderPassRenderTargetDescs[RHI_MAX_MRT_COUNT]);
        DECLARE_ZERO(D3D12_RENDER_PASS_DEPTH_STENCIL_DESC, renderPassDepthStencilDesc);
        uint32_t colorTargetCount = 0;
        uint32_t subpassIndex = 0;
        
        // color
        /*
        for(uint32_t i = 0; i < beginRenderPassDesc.subpasses[0].render_target_count; ++i)
        {
            RHITextureView_D3D12* tex_view = static_cast<RHITextureView_D3D12*>(beginRenderPassDesc.subpasses[0].input_attachments[i].view);
            clearValues[i].Format = DXGIUtil_TranslatePixelFormat(tex_view->create_info.format);
            clearValues[i].Color[0] = beginRenderPassDesc.subpasses[0].input_attachments[i].clear_value.r;
            clearValues[i].Color[1] = beginRenderPassDesc.subpasses[0].input_attachments[i].clear_value.g;
            clearValues[i].Color[2] = beginRenderPassDesc.subpasses[0].input_attachments[i].clear_value.b;
            clearValues[i].Color[3] = beginRenderPassDesc.subpasses[0].input_attachments[i].clear_value.a;
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE beginningAccess = gDx12PassBeginOpTranslator[beginRenderPassDesc.subpasses[0].color_attachments[i].load_action];
            RHITextureView_D3D12* tex_view_resolve = static_cast<RHITextureView_D3D12*>(beginRenderPassDesc.subpasses[0].color_attachments[i].resolve_view);
            if(beginRenderPassDesc.subpasses[0].sample_count != RHI_SAMPLE_COUNT_1 && tex_view_resolve)
            {
                RHITexture_D3D12* tex = static_cast<RHITexture_D3D12*>(tex_view->create_info.texture);
                RHITexture_D3D12* tex_resolve = static_cast<RHITexture_D3D12*>(tex_view_resolve->create_info.texture);
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
                D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = gDx12PassEndOpTranslator[beginRenderPassDesc.subpasses[0].color_attachments[i].store_action];
                renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = tex_view->mRtvDsvDescriptorHandle;
                renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, clearValues[i] };
                renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess , {} };
            }
            ++colorTargetCount;
        }
        // depth stencil
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pRenderPassDepthStencilDesc = nullptr;
        if(beginRenderPassDesc.subpasses[0].depth_stencil_attachment != nullptr && beginRenderPassDesc.subpasses[0].depth_stencil_attachment->view != nullptr)
        {
            RHITextureView_D3D12* dt_view = static_cast<RHITextureView_D3D12*>(beginRenderPassDesc.subpasses[0].depth_stencil_attachment->view);
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE depthBeginningAccess = gDx12PassBeginOpTranslator[beginRenderPassDesc.subpasses[0].depth_stencil_attachment->depth_load_action];
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE depthEndingAccess = gDx12PassEndOpTranslator[beginRenderPassDesc.subpasses[0].depth_stencil_attachment->depth_store_action];
            D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE stencilBeginningAccess = gDx12PassBeginOpTranslator[beginRenderPassDesc.subpasses[0].depth_stencil_attachment->stencil_load_action];
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE stencilEndingAccess = gDx12PassEndOpTranslator[beginRenderPassDesc.subpasses[0].depth_stencil_attachment->stencil_store_action];
            clearDepth.Format = DXGIUtil_TranslatePixelFormat(beginRenderPassDesc.subpasses[0].depth_stencil_attachment->view->create_info.format);
            clearDepth.DepthStencil.Depth = beginRenderPassDesc.subpasses[0].depth_stencil_attachment->clear_depth;
            clearStencil.Format = DXGIUtil_TranslatePixelFormat(beginRenderPassDesc.subpasses[0].depth_stencil_attachment->view->create_info.format);
            clearStencil.DepthStencil.Stencil = beginRenderPassDesc.subpasses[0].depth_stencil_attachment->clear_stencil;
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
        cyber_warn(false, "ID3D12GraphicsCommandList4 is not defined!");
        */
        #endif
        return cmd;
    }

    void RenderDevice_D3D12_Impl::cmd_end_render_pass(ICommandBuffer* pCommandBuffer)
    {
        RHICommandBuffer_D3D12* cmd = static_cast<RHICommandBuffer_D3D12*>(pCommandBuffer);
        #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
            ID3D12GraphicsCommandList4* cmdList4 = (ID3D12GraphicsCommandList4*)cmd->pDxCmdList;
            cmdList4->EndRenderPass();
            return;
        #endif
        cyber_warn(false, "ID3D12GraphicsCommandList4 is not defined!");
    }
    
    void RenderDevice_D3D12_Impl::render_encoder_bind_descriptor_set(IRenderPassEncoder* encoder, IDescriptorSet* descriptorSet)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        const RHIDescriptorSet_D3D12* Set = static_cast<const RHIDescriptorSet_D3D12*>(descriptorSet);
        RHIRootSignature_D3D12* RS = static_cast<RHIRootSignature_D3D12*>(Set->root_signature);

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
    void RenderDevice_D3D12_Impl::render_encoder_set_viewport(IRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        D3D12_VIEWPORT viewport = { x, y, width, height, min_depth, max_depth };
        Cmd->pDxCmdList->RSSetViewports(1, &viewport);
    }
    void RenderDevice_D3D12_Impl::render_encoder_set_scissor(IRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        D3D12_RECT rect;
        rect.left = x;
        rect.top = y;
        rect.right = x + width;
        rect.bottom = y + height;
        Cmd->pDxCmdList->RSSetScissorRects(1, &rect);
    }
    void RenderDevice_D3D12_Impl::render_encoder_bind_pipeline(IRenderPassEncoder* encoder, IRenderPipeline* pipeline)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        RHIRenderPipeline_D3D12* Pipeline = static_cast<RHIRenderPipeline_D3D12*>(pipeline);
        reset_root_signature(Cmd, RHI_PIPELINE_TYPE_GRAPHICS, Pipeline->pDxRootSignature);
        Cmd->pDxCmdList->IASetPrimitiveTopology(Pipeline->mPrimitiveTopologyType);
        Cmd->pDxCmdList->SetPipelineState(Pipeline->pDxPipelineState);
    }
    void RenderDevice_D3D12_Impl::render_encoder_bind_vertex_buffer(IRenderPassEncoder* encoder, uint32_t buffer_count, RenderObject::IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);

        DECLARE_ZERO(D3D12_VERTEX_BUFFER_VIEW, views[RHI_MAX_VERTEX_ATTRIBUTES]);
        for(uint32_t i = 0;i < buffer_count; ++i)
        {
            const RenderObject::Buffer_D3D12_Impl* Buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(buffers[i]);
            cyber_check(Buffer->mDxGpuAddress != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN);

            views[i].BufferLocation = Buffer->mDxGpuAddress + (offsets ? offsets[i] : 0);
            views[i].SizeInBytes = (UINT)(Buffer->mSize - (offsets ? offsets[i] : 0));
            views[i].StrideInBytes = (UINT)strides[i];  
        }
        Cmd->pDxCmdList->IASetVertexBuffers(0, buffer_count, views);
    }
    void RenderDevice_D3D12_Impl::render_encoder_bind_index_buffer(IRenderPassEncoder* encoder, RenderObject::IBuffer* buffer, uint32_t index_stride, uint64_t offset)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        const RenderObject::Buffer_D3D12_Impl* Buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(buffer);

        DECLARE_ZERO(D3D12_INDEX_BUFFER_VIEW, view);
        view.BufferLocation = Buffer->mDxGpuAddress + offset;
        view.SizeInBytes = (UINT)(Buffer->mSize - offset);
        view.Format = index_stride == sizeof(uint16_t) ? DXGI_FORMAT_R16_UINT : ((index_stride == sizeof(uint8_t) ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32_UINT));
        Cmd->pDxCmdList->IASetIndexBuffer(&view);
    }
    void RenderDevice_D3D12_Impl::render_encoder_push_constants(IRenderPassEncoder* encoder, IRootSignature* rs, const char8_t* name, const void* data)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        RHIRootSignature_D3D12* RS = static_cast<RHIRootSignature_D3D12*>(rs);
        reset_root_signature(Cmd, RHI_PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
        Cmd->pDxCmdList->SetGraphicsRoot32BitConstants(RS->root_parameter_index, RS->root_constant_parameter.Constants.Num32BitValues, data, 0);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw(IRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        Cmd->pDxCmdList->DrawInstanced((UINT)vertex_count, (UINT)1, (UINT)first_vertex, (UINT)0);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw_instanced(IRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        Cmd->pDxCmdList->DrawInstanced((UINT)vertex_count, (UINT)instance_count, (UINT)first_vertex, (UINT)first_instance);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw_indexed(IRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        Cmd->pDxCmdList->DrawIndexedInstanced((UINT)index_count, (UINT)1, (UINT)first_index, (UINT)first_vertex, (UINT)0);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw_indexed_instanced(IRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
    {
        RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
        Cmd->pDxCmdList->DrawIndexedInstanced((UINT)index_count, (UINT)instance_count, (UINT)first_index, (UINT)first_vertex, (UINT)first_instance);
    }

    ISwapChain* RenderDevice_D3D12_Impl::create_swap_chain(const SwapChainDesc& desc)
    {
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(adapter->pInstance);
        const uint32_t buffer_count = desc.mImageCount;
        SawpChain_D3D12_Impl* dxSwapChain = (SawpChain_D3D12_Impl*)cyber_calloc(1, sizeof(RHISwapChain_D3D12));
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

        HWND hwnd = desc.surface->handle;

        RHIQueue_D3D12* queue = nullptr;
        if(desc.mPresentQueue)
        {
            queue = static_cast<RHIQueue_D3D12*>(desc.mPresentQueue);
        }
        else 
        {
            queue = static_cast<RHIQueue_D3D12*>(get_queue(RHI_QUEUE_TYPE_GRAPHICS, 0));
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

        dxSwapChain->mBackBufferSRVs = (RenderObject::ITexture**)cyber_malloc(buffer_count * sizeof(RenderObject::ITexture*));
        for(uint32_t i = 0; i < buffer_count; i++)
        {
            RenderObject::Texture_D3D12_Impl* Ts = cyber_new<RenderObject::Texture_D3D12_Impl>(this);
            Ts->native_resource = backbuffers[i];
            Ts->allocation = nullptr;
            Ts->mIsCube = false;
            Ts->mArraySize = 0;
            Ts->mFormat = desc.mFormat;
            Ts->mAspectMask = 1;
            Ts->mDepth = 1;
            Ts->mWidth = desc.mWidth;
            Ts->mHeight = desc.mHeight;
            Ts->mMipLevels = 1;
            Ts->mNodeIndex = RHI_SINGLE_GPU_NODE_INDEX;
            Ts->mOwnsImage = false;
            Ts->mNativeHandle = Ts->native_resource;
            dxSwapChain->mBackBufferSRVs[i] = Ts;
        }
        //dxSwapChain->mBackBuffers = Ts;
        dxSwapChain->mBufferSRVCount = buffer_count;

        // Create depth stencil view
        //dxSwapChain->mBackBufferDSV = (RHITexture*)cyber_malloc(sizeof(RHITexture));
        TextureCreateDesc depthStencilDesc = {};
        depthStencilDesc.height = desc.mHeight;
        depthStencilDesc.width = desc.mWidth;
        depthStencilDesc.depth = 1;
        depthStencilDesc.array_size = 1;
        depthStencilDesc.format = RHI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.mip_levels = 1;
        depthStencilDesc.sample_count = RHI_SAMPLE_COUNT_1;
        depthStencilDesc.descriptors = RHI_DESCRIPTOR_TYPE_UNDEFINED;
        depthStencilDesc.start_state = RHI_RESOURCE_STATE_DEPTH_WRITE;
        depthStencilDesc.name = u8"Main Depth Stencil";
        dxSwapChain->mBackBufferDSV = create_texture(depthStencilDesc);

        auto dsv = static_cast<RenderObject::Texture_D3D12_Impl*>(dxSwapChain->mBackBufferDSV);

        TextureViewCreateDesc depthStencilViewDesc = {};
        depthStencilViewDesc.texture = dxSwapChain->mBackBufferDSV;
        depthStencilViewDesc.dimension = RHI_TEX_DIMENSION_2D;
        depthStencilViewDesc.format = RHI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.usages = RHI_TVU_RTV_DSV;
        depthStencilViewDesc.aspects = RHI_TVA_DEPTH;
        depthStencilViewDesc.array_layer_count = 1;
        dxSwapChain->mBackBufferDSVView = create_texture_view(depthStencilViewDesc);

        dxSwapChain->pDxSwapChain->GetCurrentBackBufferIndex();
        return dxSwapChain;
    }

    void RenderDevice_D3D12_Impl::free_swap_chain(ISwapChain* swapchain)
    {
        SawpChain_D3D12_Impl* dxSwapChain = static_cast<SawpChain_D3D12_Impl*>(swapchain);
        for(uint32_t i = 0;i < dxSwapChain->mBufferSRVCount; ++i)
        {
            RenderObject::Texture_D3D12_Impl* dxTexture = static_cast<RenderObject::Texture_D3D12_Impl*>(dxSwapChain->mBackBufferSRVs[i]);
            SAFE_RELEASE(dxTexture->native_resource);
        }
        SAFE_RELEASE(dxSwapChain->pDxSwapChain);
        cyber_delete(swapchain);
    }

    void RenderDevice_D3D12_Impl::enum_adapters(IInstance* instance, IAdapter** adapters, uint32_t* adapterCount)
    {
        cyber_assert(instance, "fatal: Invalid instance!");
        RHIInstance_D3D12* dxInstance = static_cast<RHIInstance_D3D12*>(instance);
        *adapterCount = dxInstance->mAdaptersCount;
        if(!adapters)
        {
            return;
        }
        else
        {
            for(uint32_t i = 0; i < dxInstance->mAdaptersCount; ++i)
            {
                adapters[i] = &dxInstance->pAdapters[i];
            }
        }
    }

    uint32_t RenderDevice_D3D12_Impl::acquire_next_image(ISwapChain* swapchain, const AcquireNextDesc& acquireDesc)
    {
        RenderObject::SawpChain_D3D12_Impl* dxSwapChain = static_cast<RenderObject::SawpChain_D3D12_Impl*>(swapchain);
        // On PC AquireNext is always true
        return dxSwapChain->pDxSwapChain->GetCurrentBackBufferIndex();
    }

    IFrameBuffer* RenderDevice_D3D12_Impl::create_frame_buffer(const FrameBuffserDesc& frameBufferDesc)
    {
        return nullptr;
    }

    // for example 
    IRootSignature* RenderDevice_D3D12_Impl::create_root_signature(const RootSignatureCreateDesc& rootSigDesc)
    {
        RHIRootSignature_D3D12* dxRootSignature = cyber_new<RHIRootSignature_D3D12>();

        // Pick root parameters from desc data
        SHADER_STAGE shaderStages = 0;
        for(uint32_t i = 0; i < rootSigDesc.shader_count; ++i)
        {
            PipelineShaderCreateDesc* shader_desc = *(rootSigDesc.shaders + i);
            shaderStages |= shader_desc->stage;
        }

        // Pick shader reflection data
        rhi_util_init_root_signature_tables(dxRootSignature, rootSigDesc);
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
            SHADER_STAGE visStages = SHADER_STAGE::SHADER_STAGE_NONE;
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
        cyber_assert(dxRootSignature->push_constant_count <= 1, "Only support one push constant range");
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
                    if(strcmp((char*)rst_slot.name, (char*)rootSigDesc.static_sampler_names[j]) == 0)
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
        
        
        D3D12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = valid_root_tables + dxRootSignature->push_constant_count;
        rootSignatureDesc.pParameters = nullptr;
        rootSignatureDesc.NumStaticSamplers = staticSamplerCount;
        rootSignatureDesc.pStaticSamplers = staticSamplerDescs;
        rootSignatureDesc.Flags = rootSignatureFlags;
        
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {};
        versionedRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        versionedRootSignatureDesc.Desc_1_1 = rootSignatureDesc;
        ID3DBlob* signature = nullptr;
        ID3DBlob* error = nullptr;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, &signature, &error);
        if(SUCCEEDED(hr))
        {
            hr = pDxDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&dxRootSignature->dxRootSignature));
            if(FAILED(hr))
            {
                CB_ERROR("Failed to create root signature!");
                return nullptr;
            }
        }

        return dxRootSignature;
    }

    void RenderDevice_D3D12_Impl::free_root_signature(IRootSignature* pRootSignature)
    {
        cyber_delete(pRootSignature);
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

    RHIDescriptorSet* RenderDevice_D3D12_Impl::create_descriptor_set(const IDescriptorSetCreateDesc& dSetDesc)
    {
        RHIRootSignature_D3D12* root_signature = static_cast<RHIRootSignature_D3D12*>(dSetDesc.root_signature);
        RHIDescriptorSet_D3D12* descSet = cyber_new<RHIDescriptorSet_D3D12>();
        descSet->root_signature = dSetDesc.root_signature;
        descSet->set_index = dSetDesc.set_index;

        const uint32_t node_index = RHI_SINGLE_GPU_NODE_INDEX;
        auto& cbv_srv_uav_heap = mCbvSrvUavHeaps[node_index];
        auto& sampler_heap = mSamplerHeaps[node_index];
        RHIParameterTable* param_table = &root_signature->parameter_tables[dSetDesc.set_index];
        uint32_t cbv_srv_uav_count = 0;
        uint32_t sampler_count = 0;
        // collect descriptor counts
        if(root_signature->parameter_table_count > 0)
        {
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
                    case RHI_RESOURCE_TYPE_TEXTURE: src_handle = pNullDescriptors->TextureSRV[dimension]; break;
                    case RHI_RESOURCE_TYPE_BUFFER: src_handle = pNullDescriptors->BufferSRV; break;
                    case RHI_RESOURCE_TYPE_RW_BUFFER: src_handle = pNullDescriptors->BufferUAV; break;
                    case RHI_RESOURCE_TYPE_UNIFORM_BUFFER: src_handle = pNullDescriptors->BufferCBV; break;
                    case RHI_RESOURCE_TYPE_SAMPLER: src_sampler_handle = pNullDescriptors->Sampler; break;
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
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
    
    // Default pipeline state
    CD3D12_BLEND_DESC gDefaultBlendDesc(D3D12_DEFAULT);
    CD3D12_RASTERIZER_DESC gDefaultRasterizerDesc(D3D12_DEFAULT);

    RHIRenderPipeline* RenderDevice_D3D12_Impl::create_render_pipeline(const RHIRenderPipelineCreateDesc& pipelineDesc)
    {
        RHIRootSignature_D3D12* DxRootSignature = static_cast<RHIRootSignature_D3D12*>(pipelineDesc.root_signature);
        RHIRenderPipeline_D3D12* pPipeline = cyber_new<RHIRenderPipeline_D3D12>();
        // Input layout
        DECLARE_ZERO(D3D12_INPUT_ELEMENT_DESC, input_elements[RHI_MAX_VERTEX_ATTRIBUTES]);
        uint32_t input_element_count = 0;

        static const D3D12_INPUT_ELEMENT_DESC s_inputElementDesc[] =
        {
            { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        if(pipelineDesc.vertex_shader->library->entry_reflections->vertex_input_count > 0)
        {
            eastl::string_hash_map<uint32_t> semantic_index_map;
            for(uint32_t attrib_index = 0; attrib_index < pipelineDesc.vertex_shader->library->entry_reflections->vertex_input_count; ++attrib_index)
            {
                auto attribute = pipelineDesc.vertex_shader->library->entry_reflections->vertex_inputs[attrib_index];
                input_elements[input_element_count].SemanticName = (char*)attribute.semantics_name;
                input_elements[input_element_count].SemanticIndex = attribute.semantics_index;
                input_elements[input_element_count].Format = DXGIUtil_TranslatePixelFormat(attribute.format);
                input_elements[input_element_count].InputSlot = 0;
                input_elements[input_element_count].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
                input_elements[input_element_count].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                input_elements[input_element_count].InstanceDataStepRate = 0;
                input_element_count++;
            }
        }
        
        /*
        if(pipelineDesc.vertex_layout)
        {
            eastl::string_hash_map<uint32_t> semantic_index_map;
            for(uint32_t attrib_index = 0;attrib_index < pipelineDesc.vertex_layout->attribute_count; ++attrib_index)
            {
                const RHIVertexAttribute* attribute = &pipelineDesc.vertex_layout->attributes[attrib_index];
                for(uint32_t index = 0; index < attribute->array_size; ++index)
                {
                    input_elements[input_element_count].SemanticName = (char*)attribute->semantic_name;
                    if(semantic_index_map.find((char*)attribute->semantic_name) == semantic_index_map.end())
                    {
                        semantic_index_map[(char*)attribute->semantic_name] = 0;
                    }
                    else 
                    {
                        semantic_index_map[(char*)attribute->semantic_name]++;
                    }
                    input_elements[input_element_count].SemanticIndex = semantic_index_map[(char*)attribute->semantic_name];
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
        */
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
                        RHIShaderLibrary_D3D12* vert_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.vertex_shader->library;
                        vertex_shader.pShaderBytecode = vert_lib->shader_blob->GetBufferPointer();
                        vertex_shader.BytecodeLength = vert_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_FRAG:
                {
                    if(pipelineDesc.fragment_shader)
                    {
                        RHIShaderLibrary_D3D12* frag_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.fragment_shader->library;
                        pixel_shader.pShaderBytecode = frag_lib->shader_blob->GetBufferPointer();
                        pixel_shader.BytecodeLength = frag_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_TESC:
                {
                    if(pipelineDesc.tesc_shader)
                    {
                        RHIShaderLibrary_D3D12* domain_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.tesc_shader->library;
                        hull_shader.pShaderBytecode = domain_lib->shader_blob->GetBufferPointer();
                        hull_shader.BytecodeLength = domain_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_TESE:
                {
                    if(pipelineDesc.tese_shader)
                    {
                        RHIShaderLibrary_D3D12* hull_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.tese_shader->library;
                        domain_shader.pShaderBytecode = hull_lib->shader_blob->GetBufferPointer();
                        domain_shader.BytecodeLength = hull_lib->shader_blob->GetBufferSize();
                    }
                    break;
                }
                case RHI_SHADER_STAGE_GEOM:
                {
                    if(pipelineDesc.geometry_shader)
                    {
                        RHIShaderLibrary_D3D12* geom_lib = (RHIShaderLibrary_D3D12*)pipelineDesc.geometry_shader->library;
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
        pso_desc.BlendState = pipelineDesc.blend_state ? D3D12Util_TranslateBlendState(pipelineDesc.blend_state) : gDefaultBlendDesc;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.RasterizerState = pipelineDesc.rasterizer_state ? D3D12Util_TranslateRasterizerState(pipelineDesc.rasterizer_state) : gDefaultRasterizerDesc;
        // Depth stencil
        pso_desc.DepthStencilState = pipelineDesc.depth_stencil_state ? D3D12Util_TranslateDepthStencilState(pipelineDesc.depth_stencil_state) : gDefaultDepthStencilDesc;
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
        if(pPipelineLibrary)
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

            swprintf(pipelineName, PSO_NAME_LENGTH, L"GRAPHCISPSO_S%zuR%zu", psoShaderHash, psoRenderHash);
            result = pPipelineLibrary->LoadGraphicsPipeline(pipelineName, &pso_desc, IID_PPV_ARGS(&pPipeline->pDxPipelineState));
        }
        // Not find in cache
        if(!SUCCEEDED(result))
        {
            CHECK_HRESULT(pDxDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pPipeline->pDxPipelineState)));
            // Pipeline cache
            if(pPipelineLibrary)
            {
                CHECK_HRESULT(pPipelineLibrary->StorePipeline(pipelineName, pPipeline->pDxPipelineState));
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

    void RenderDevice_D3D12_Impl::free_render_pipeline(IRenderPipeline* pipeline)
    {
        RHIRenderPipeline_D3D12* pPipeline = static_cast<RHIRenderPipeline_D3D12*>(pipeline);
        SAFE_RELEASE(pPipeline->pDxPipelineState);
        cyber_free(pPipeline);
    }

    void RenderDevice_D3D12_Impl::update_descriptor_set(IDescriptorSet* set, const DescriptorData* updateData, uint32_t count)
    {
        RHIDescriptorSet_D3D12* dxSet = static_cast<RHIDescriptorSet_D3D12*>(set);
        const RHIRootSignature_D3D12* dxRootSignature = static_cast<const RHIRootSignature_D3D12*>(set->root_signature);
        RHIParameterTable* paramTable = &dxRootSignature->parameter_tables[set->set_index];
        const uint32_t nodeIndex = RHI_SINGLE_GPU_NODE_INDEX;
        RHIDescriptorHeap_D3D12* pCbvSrvUavHeap = mCbvSrvUavHeaps[nodeIndex];
        RHIDescriptorHeap_D3D12* pSamplerHeap = mSamplerHeaps[nodeIndex];
        for(uint32_t i = 0;i < count; i++)
        {
            // Descriptor Info
            const RHIDescriptorData* pParam = updateData + i;
            RHIShaderResource* resData = nullptr;
            uint32_t heapOffset = 0;
            if(pParam->name != nullptr)
            {
                size_t argNameHash = graphics_name_hash(pParam->name, strlen((char*)pParam->name));
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
            const uint32_t arrayCount = graphics_max(1u,pParam->count);
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
                    RenderObject::TextureView_D3D12_Impl** Textures = (RenderObject::TextureView_D3D12_Impl**)pParam->textures;
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
                    RenderObject::Buffer_D3D12_Impl** Buffers = (RenderObject::Buffer_D3D12_Impl**)pParam->buffers;
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
                    RenderObject::Buffer_D3D12_Impl** Buffers = (RenderObject::Buffer_D3D12_Impl**)pParam->buffers;
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
                    RenderObject::TextureView_D3D12_Impl** Textures = (RenderObject::TextureView_D3D12_Impl**)pParam->textures;
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
                    RenderObject::Buffer_D3D12_Impl** Buffers = (RenderObject::Buffer_D3D12_Impl**)pParam->buffers;
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

    RenderObject::IBuffer* RenderDevice_D3D12_Impl::create_buffer(const BufferCreateDesc& pDesc)
    {
        RHIAdapter_D3D12* DxAdapter = static_cast<RHIAdapter_D3D12*>(adapter);
        
        RenderObject::Buffer_D3D12_Impl* pBuffer = cyber_new<RenderObject::Buffer_D3D12_Impl>(this);

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
        pDxDevice->GetCopyableFootprints(&desc, 0, 1, 0, NULL, NULL, NULL, &padded_size);
        allocationSize = (uint64_t)padded_size;
        // Buffer is 1D
        desc.Width = padded_size;

        ERHIResourceState start_state = pDesc.mStartState;
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
            CHECK_HRESULT(pDxDevice->CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, &desc, res_states, NULL, IID_ARGS(&pBuffer->pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Committed Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", (char*)(pDesc.pName ? pDesc.pName : CYBER_UTF8("")), allocationSize, pDesc.mFormat);
        }
        else
        {
            CHECK_HRESULT(pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, NULL, &pBuffer->pDxAllocation, IID_ARGS(&pBuffer->pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", (char*)(pDesc.pName ? pDesc.pName : CYBER_UTF8("")), allocationSize, pDesc.mFormat);
        }
        
        if(pDesc.mMemoryUsage != RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY && pDesc.mFlags & RHI_BCF_PERSISTENT_MAP_BIT)
            pBuffer->pDxResource->Map(0, NULL, &pBuffer->pCpuMappedAddress);
        
        pBuffer->mDxGpuAddress = pBuffer->pDxResource->GetGPUVirtualAddress();
        
        // Create Descriptors
        if(!(pDesc.mFlags & RHI_BCF_NO_DESCRIPTOR_VIEW_CREATION))
        {
            RHIDescriptorHeap_D3D12* pHeap = mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
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
                D3D12Util_CreateCBV(this, &cbvDesc, &pBuffer->mDxDescriptorHandles);
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

                D3D12Util_CreateSRV(this, pBuffer->pDxResource, &srvDesc, &srv);
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
                    HRESULT hr = pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
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

                ID3D12Resource* pCounterResource = pDesc.pCounterBuffer ? static_cast<RenderObject::Buffer_D3D12_Impl*>(pDesc.pCounterBuffer)->pDxResource : nullptr;
                D3D12Util_CreateUAV(this, pBuffer->pDxResource, pCounterResource, &uavDesc, &uav);
            }
        }

        pBuffer->mSize = (uint32_t)pDesc.mSize;
        pBuffer->mMemoryUsage = pDesc.mMemoryUsage;
        pBuffer->mDescriptors = pDesc.mDescriptors;
        return pBuffer;
    }
    void RenderDevice_D3D12_Impl::free_buffer(RenderObject::IBuffer* buffer)
    {

    }
    void RenderDevice_D3D12_Impl::map_buffer(RenderObject::IBuffer* buffer, const BufferRange* range)
    {

    }
    void RenderDevice_D3D12_Impl::unmap_buffer(RenderObject::IBuffer* buffer)
    {

    }
    RHIShaderLibrary* RenderDevice_D3D12_Impl::create_shader_library(const struct ShaderLibraryCreateDesc& desc)
    {
        RHIShaderLibrary_D3D12* pLibrary = cyber_new<RHIShaderLibrary_D3D12>();

        bool bUseDXC = false;
        switch(desc.shader_compiler)
        {
            case SHADER_COMPILER_DEFAULT:
                bUseDXC = false;
                break;
            case SHADER_COMPILER_GLSLANG:
                bUseDXC = false;
                break;
            case SHADER_COMPILER_DXC:
                bUseDXC = true;
                break;
            case SHADER_COMPILER_FXC:
                bUseDXC = false;
                break;
            default:
                CB_CORE_ERROR("Invalid shader compiler");
                return nullptr;
        }

        HRESULT hr = S_OK;
        if(bUseDXC)
        {
            IDxcUtils* pDxcUtils;
            IDxcLibrary* pDxcLibrary;
            IDxcContainerReflection* pReflection;
            IDxcValidator* pValidator;

            auto procDxcCreateInstance = D3D12Util_GetDxcCreateInstanceProc();
            if(!procDxcCreateInstance)
            {
                CB_CORE_ERROR("Cannot find dxc.dll");
                return nullptr;
            }
            procDxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pDxcUtils));
            procDxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pDxcLibrary));
            procDxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));
            procDxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&pValidator));
            if(!pDxcLibrary)
            {
                CB_CORE_ERROR("Cannot create dxc library");
                return nullptr;
            }
            /*
            LPCWSTR* pszArgs = cyber_new_n<LPCWSTR>(12);
            pszArgs[0] = L"shaders.hlsl";
            pszArgs[1] = L"-E";
            pszArgs[2] = L"VSMain";
            pszArgs[3] = L"-T";
            pszArgs[4] = L"vs_6_0";
            pszArgs[5] = L"D";
            pszArgs[6] = L"MYDEFINE=1";
            pszArgs[7] = L"-Fo";
            pszArgs[8] = L"shaders.bin";
            pszArgs[9] = L"-Fd";
            pszArgs[10] = L"shaders.pdb";
            pszArgs[11] = L"-Qstrip_reflect";
            */

            eastl::wstring shaderName(eastl::wstring::CtorConvert(), desc.name);
            eastl::wstring entry_point(eastl::wstring::CtorConvert(), desc.entry_point);
            auto entry = entry_point.c_str();
            ShaderVersion shader_version(6 , 0);
            eastl::string profile = GetHLSLProfileString(desc.stage, shader_version);
            eastl::wstring profile_wstr(eastl::wstring::CtorConvert(),profile.c_str());
            eastl::vector<const wchar_t*> DxilArgs;

            DxilArgs.push_back(shaderName.c_str());
            DxilArgs.push_back(L"-E");
            DxilArgs.push_back(entry_point.c_str());
            DxilArgs.push_back(L"-T");
            DxilArgs.push_back(profile_wstr.c_str());
            if(desc.shader_macro_count > 0)
            {
                DxilArgs.push_back(L"-D");

                for(uint32_t i = 0; i < desc.shader_macro_count; ++i)
                {
                    eastl::string macro_name(eastl::string::CtorSprintf(), "%s=%s",desc.shader_macros[i].definition, desc.shader_macros[i].value);
                    CB_INFO("Shader Define: {0} ", macro_name.c_str());
                    eastl::wstring wmacro_name(eastl::wstring::CtorConvert(), macro_name.c_str());
                    DxilArgs.push_back(wmacro_name.c_str());
                }
            }

            // debug
            {
                DxilArgs.push_back(L"-Fo");
                eastl::wstring shader_bin(shaderName.substr(0, shaderName.find(L".")));
                shader_bin += L".bin";
                DxilArgs.push_back(shader_bin.c_str());

                DxilArgs.push_back(L"-Fd");
                eastl::wstring shader_pdb(shaderName.substr(0, shaderName.find(L".")));
                shader_pdb += L".pdb";
                DxilArgs.push_back(shader_pdb.c_str());
            }
            
            IDxcBlobEncoding* pBlobEncoding;
            CHECK_HRESULT(pDxcUtils->CreateBlob((LPCVOID)desc.code, desc.code_size, DXC_CP_ACP, &pBlobEncoding));
            //CHECK_HRESULT(pDxcLibrary->CreateBlobWithEncodingFromPinned((LPCVOID)TestShader, sizeof(TestShader), 0, &pBlobEncoding));
            
            //IDxcCompiler *pCompiler;
            IDxcCompiler3 *pCompiler3;
            CHECK_HRESULT(procDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler3)));
            DxcBuffer pSource;
            pSource.Ptr = pBlobEncoding->GetBufferPointer();
            pSource.Size = pBlobEncoding->GetBufferSize();
            pSource.Encoding = DXC_CP_ACP;
            IDxcIncludeHandler* pIncludeHandler;
            pDxcUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

            pCompiler3->Compile(&pSource, DxilArgs.data(), DxilArgs.size(), pIncludeHandler, IID_PPV_ARGS(&pLibrary->shader_result));

            IDxcBlobUtf8* pErrors = nullptr;
            pLibrary->shader_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
            // Note that d3dcompiler would return null if no errors or warnings are present.
            // IDxcCompiler3::Compile will always return an error buffer, but its length
            // will be zero if there are no warnings or errors.
            if (pErrors != nullptr && pErrors->GetStringLength() != 0)
                CB_WARN("Warnings and Errors:{0}", pErrors->GetStringPointer());
            
            pLibrary->shader_result->GetStatus(&hr);
            if(FAILED(hr))
            {
                return nullptr;
            }

            pLibrary->shader_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pLibrary->shader_blob), nullptr);

            pDxcUtils->Release();
            pDxcLibrary->Release();
        }
        else {
            //hr = (pReflection->Load(pBlobEncoding));
            ID3D12ShaderReflection *pReflection2;
            ID3DBlob *ppCode;
            ID3DBlob* ppErrorMsgs;
            D3D_SHADER_MACRO Macros[] = {{"D3DCOMPILER", ""}, {}};
            DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
            //desc.shader_target;
            ShaderVersion shader_version(5 , 1);
            eastl::string profile = GetHLSLProfileString(desc.stage, shader_version);
            eastl::string entry_point(eastl::string::CtorConvert(), desc.entry_point);
            hr = D3DCompile((LPCVOID)desc.code, desc.code_size, nullptr, Macros, nullptr, entry_point.c_str(), profile.c_str(), dwShaderFlags, 0, &pLibrary->shader_blob, &ppErrorMsgs);

            if(hr != S_OK)
            {
                CB_CORE_ERROR("Failed to compile shader: {0}", (char*)ppErrorMsgs->GetBufferPointer());
                return nullptr;
            }
        }

        constexpr char TestShader[] = R"(
            float4 main() : SV_Target0
            {
                return float4(0.0, 0.0, 0.0, 0.0);
            }
            )";

        // Reflect shader
        D3D12Util_InitializeShaderReflection(this, pLibrary, desc);

        return pLibrary;
    }

    void RenderDevice_D3D12_Impl::free_shader_library(IShaderLibrary* shaderLibrary)
    {
        RHIShaderLibrary_D3D12* dx_shader_library = static_cast<RHIShaderLibrary_D3D12*>(shaderLibrary);
        D3D12Util_FreeShaderReflection(dx_shader_library);
        if(dx_shader_library->shader_blob != nullptr)
        {
            dx_shader_library->shader_blob->Release();
        }
        cyber_delete(shaderLibrary);
    }
    
    void RenderDevice_D3D12_Impl::create_dma_allocallor(RenderObject::Adapter_D3D12_Impl* adapter)
    {
        D3D12MA::ALLOCATOR_DESC desc = {};
        desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
        desc.pDevice = GetD3D12Device();
        desc.pAdapter = adapter->get_native_adapter();

        D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
        allocationCallbacks.pAllocate = [](size_t size, size_t alignment, void*){
            return cyber_memalign(size, alignment);
        };
        allocationCallbacks.pFree = [](void* ptr, void*){
            cyber_free(ptr);
        };
        desc.pAllocationCallbacks = &allocationCallbacks;
        desc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
        if(!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &m_pResourceAllocator)))
        {
            cyber_assert(false, "DMA Allocator Create Failed!");
        }
    }

    HRESULT RenderDevice_D3D12_Impl::hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize)
    {
        return pDxDevice->CheckFeatureSupport(pFeature, pFeatureSupportData, pFeatureSupportDataSize);
    }

    HRESULT RenderDevice_D3D12_Impl::hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource)
    {
        return pDxDevice->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

    void RenderDevice_D3D12_Impl::create_constant_buffer_view(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateConstantBufferView(desc, *destHandle);
    }

    void RenderDevice_D3D12_Impl::create_shader_resource_view(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateShaderResourceView(resource, desc, destHandle);
    }

    void RenderDevice_D3D12_Impl::create_unordered_access_view(ID3D12Resource* resource, ID3D12Resource* counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateUnorderedAccessView(resource, counterResource, desc, destHandle);
    }

    void RenderDevice_D3D12_Impl::create_render_target_view(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateRenderTargetView(resource, desc, destHandle);
    }

    }
}