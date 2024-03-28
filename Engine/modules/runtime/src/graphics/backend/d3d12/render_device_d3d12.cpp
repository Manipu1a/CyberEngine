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
#include "../../common/graphics_utils.h"
#include "graphics/backend/d3d12/D3D12MemAlloc.h"
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
#include "graphics/backend/d3d12/descriptor_heap_d3d12.h"
#include "graphics/backend/d3d12/descriptor_set_d3d12.h"
#include "graphics/backend/d3d12/semaphore_d3d12.h"
#include "graphics/backend/d3d12/render_pipeline_d3d12.h"
#include "graphics/backend/d3d12/root_signature_d3d12.h"
#include "graphics/backend/d3d12/sampler_d3d12.h"
#include "graphics/backend/d3d12/shader_library_d3d12.h"
#include "platform/configure.h"


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
        Instance_D3D12_Impl* dxInstance = static_cast<Instance_D3D12_Impl*>(dxAdapter->get_instance());
        
        this->m_pAdapter = adapter;

        if(!SUCCEEDED(D3D12CreateDevice(dxAdapter->get_native_adapter(), dxAdapter->get_feature_level(), IID_PPV_ARGS(&m_pDxDevice))))
        {
            cyber_assert(false, "[D3D12 Fatal]: Create D3D12Device Failed!");
        }

        // Create Requested Queues
        m_pNullDescriptors = (RenderObject::EmptyDescriptors_D3D12*)cyber_calloc(1, sizeof(RenderObject::EmptyDescriptors_D3D12));

        for(uint32_t i = 0u; i < deviceDesc.m_queueGroupCount;i++)
        {
            auto& queueGroup = deviceDesc.m_queueGroups[i];
            auto type = queueGroup.m_queueType;

            m_commandQueueCounts[type] = queueGroup.m_queueCount;
            m_commandQueues[type] = (ID3D12CommandQueue**)cyber_malloc(sizeof(ID3D12CommandQueue*) * queueGroup.m_queueCount);

            for(uint32_t j = 0u; j < queueGroup.m_queueCount; j++)
            {
                DECLARE_ZERO(D3D12_COMMAND_QUEUE_DESC, queueDesc)
                switch(type)
                {
                    case QUEUE_TYPE_GRAPHICS:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                        break;
                    case QUEUE_TYPE_COMPUTE:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                        break;
                    case QUEUE_TYPE_TRANSFER:
                        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                        break;
                    default:
                        cyber_assert(false, "[D3D12 Fatal]: Create D3D12CommandQueue Failed!");
                        break;
                }
                queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

                if(!SUCCEEDED(m_pDxDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueues[type][j]))))
                {
                    cyber_assert(false, "[D3D12 Fatal]: Create CommandQueue Failed!");
                }
            }
        }

        // Create D3D12MA Allocator
        create_dma_allocallor(dxAdapter);
        cyber_assert(m_pResourceAllocator, "DMA Allocator Must be Created!");

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
            m_cpuDescriptorHeaps[i] = (DescriptorHeap_D3D12*)cyber_malloc(sizeof(DescriptorHeap_D3D12));
            DescriptorHeap_D3D12::create_descriptor_heap(m_pDxDevice, desc, &m_cpuDescriptorHeaps[i]);
        }
        
        // One shader visible heap for each linked node
        for(uint32_t i = 0; i < GRAPHICS_SINGLE_GPU_NODE_COUNT; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = GRAPHICS_SINGLE_GPU_NODE_MASK;
            desc.NumDescriptors = 1 << 16;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

            m_cbvSrvUavHeaps[i] = (DescriptorHeap_D3D12*)cyber_malloc(sizeof(DescriptorHeap_D3D12));
            DescriptorHeap_D3D12::create_descriptor_heap(m_pDxDevice, desc, &m_cbvSrvUavHeaps[i]);

            desc.NumDescriptors = 1 << 11;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            m_samplerHeaps[i] = (DescriptorHeap_D3D12*)cyber_malloc(sizeof(DescriptorHeap_D3D12));
            DescriptorHeap_D3D12::create_descriptor_heap(m_pDxDevice, desc, &m_samplerHeaps[i]);
        }

        // Allocate NULL Descriptors
        {
            m_pNullDescriptors->Sampler = D3D12_DESCRIPTOR_ID_NONE;
            D3D12_SAMPLER_DESC samplerDesc = {};
            samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            m_pNullDescriptors->Sampler = DescriptorHeap_D3D12::consume_descriptor_handles(m_cpuDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1).mCpu;
            m_pDxDevice->CreateSampler(&samplerDesc, m_pNullDescriptors->Sampler);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8_UINT;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R8_UINT;

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_1D]);
            create_unordered_access_view(NULL, NULL, &uavDesc, m_pNullDescriptors->TextureUAV[TEX_DIMENSION_1D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_2D]);
            create_unordered_access_view(NULL, NULL, &uavDesc, m_pNullDescriptors->TextureUAV[TEX_DIMENSION_2D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_2DMS]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_3D]);
            create_unordered_access_view(NULL, NULL, &uavDesc, m_pNullDescriptors->TextureUAV[TEX_DIMENSION_3D]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_CUBE]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_1D_ARRAY]);
            create_unordered_access_view(NULL, NULL, &uavDesc, m_pNullDescriptors->TextureUAV[TEX_DIMENSION_1D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_2D_ARRAY]);
            create_unordered_access_view(NULL, NULL, &uavDesc, m_pNullDescriptors->TextureUAV[TEX_DIMENSION_2D_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_2DMS_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->TextureSRV[TEX_DIMENSION_CUBE_ARRAY]);

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            create_shader_resource_view(NULL, &srvDesc, m_pNullDescriptors->BufferSRV);
            create_unordered_access_view(NULL, NULL, &uavDesc, m_pNullDescriptors->BufferUAV);
            create_constant_buffer_view(NULL, m_pNullDescriptors->BufferCBV);
        }

        // Pipeline cache
        D3D12_FEATURE_DATA_SHADER_CACHE feature = {};
        HRESULT result = m_pDxDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &feature, sizeof(feature));
        if(SUCCEEDED(result))
        {
            result = E_NOTIMPL;
            if(feature.SupportFlags & D3D12_SHADER_CACHE_SUPPORT_LIBRARY)
            {
                ID3D12Device1* device1 = NULL;
                result = m_pDxDevice->QueryInterface(IID_ARGS(&device1));
                if(SUCCEEDED(result))
                {
                    result = device1->CreatePipelineLibrary(m_pPSOCacheData, 0, IID_ARGS(&m_pPipelineLibrary));
                }
                SAFE_RELEASE(device1);
            }
        }
    }

    void RenderDevice_D3D12_Impl::free_device()
    {
        for(uint32_t i = 0; i < QUEUE_TYPE::QUEUE_TYPE_COUNT; ++i)
        {
            QUEUE_TYPE type;
            for(uint32_t j = 0; j < m_commandQueueCounts[i]; ++j)
            {
                SAFE_RELEASE(m_commandQueues[i][j]);
            }
            cyber_free((ID3D12CommandQueue**)m_commandQueues[i]);
        }
        // Free D3D12MA Allocator
        SAFE_RELEASE(m_pResourceAllocator);
        // Free Descriptor Heaps
        for(uint32_t i = 0;i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            m_cpuDescriptorHeaps[i]->free();
        }
        m_cbvSrvUavHeaps[0]->free();
        m_samplerHeaps[0]->free();
        cyber_free_map(m_cpuDescriptorHeaps);
        cyber_free_map(m_cbvSrvUavHeaps);
        cyber_free_map(m_samplerHeaps);
        cyber_free(m_pNullDescriptors);
        // Release D3D12 Device
        SAFE_RELEASE(m_pDxDevice);
        SAFE_RELEASE(m_pPipelineLibrary);
        if(m_pPSOCacheData) cyber_free(m_pPSOCacheData);
    }

    RenderObject::ITextureView* RenderDevice_D3D12_Impl::create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc)
    {
        RenderObject::TextureView_D3D12_Impl* tex_view = cyber_new<RenderObject::TextureView_D3D12_Impl>(this);
        tex_view->m_desc = viewDesc;
        RenderObject::Texture_D3D12_Impl* tex = static_cast<RenderObject::Texture_D3D12_Impl*>(viewDesc.m_pTexture);

        // Consume handles
        const auto usages = viewDesc.m_usages;
        uint32_t handleCount = ((usages & TVU_SRV) ? 1 : 0) + ((usages & TVU_UAV) ? 1 : 0);

        if(handleCount > 0)
        {
            DescriptorHeap_D3D12* heap = m_cpuDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
            tex_view->m_dxDescriptorHandles = DescriptorHeap_D3D12::consume_descriptor_handles(heap, 1).mCpu;
            tex_view->m_srvDescriptorOffset = 0;
            uint64_t current_offset_cursor = tex_view->m_srvDescriptorOffset;
            // Create SRV
            if(usages & TVU_SRV)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE srv = { tex_view->m_dxDescriptorHandles.ptr + tex_view->m_srvDescriptorOffset };
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.m_format, true);
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                switch (viewDesc.m_dimension)
                {
                    case TEX_DIMENSION_1D:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                        srvDesc.Texture1D.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.Texture1D.MostDetailedMip = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_1D_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                        srvDesc.Texture1DArray.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.Texture1DArray.MostDetailedMip = viewDesc.m_baseMipLevel;
                        srvDesc.Texture1DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        srvDesc.Texture1DArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_2D:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        srvDesc.Texture2D.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.Texture2D.MostDetailedMip = viewDesc.m_baseMipLevel;
                        srvDesc.Texture2D.PlaneSlice = 0;
                        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                    }
                    break;
                    case TEX_DIMENSION_2D_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        srvDesc.Texture2DArray.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.Texture2DArray.MostDetailedMip = viewDesc.m_baseMipLevel;
                        srvDesc.Texture2DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        srvDesc.Texture2DArray.ArraySize = viewDesc.m_arrayLayerCount;
                        srvDesc.Texture2DArray.PlaneSlice = 0;
                        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
                    break;
                    case TEX_DIMENSION_2DMS:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    }
                    break;
                    case TEX_DIMENSION_2DMS_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        srvDesc.Texture2DMSArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        srvDesc.Texture2DMSArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_3D:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                        srvDesc.Texture3D.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.Texture3D.MostDetailedMip = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_CUBE:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        srvDesc.TextureCube.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.TextureCube.MostDetailedMip = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_CUBE_ARRAY:
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                        srvDesc.TextureCubeArray.MipLevels = viewDesc.m_mipLevelCount;
                        srvDesc.TextureCubeArray.MostDetailedMip = viewDesc.m_baseMipLevel;
                        srvDesc.TextureCubeArray.First2DArrayFace = viewDesc.m_baseArrayLayer;
                        srvDesc.TextureCubeArray.NumCubes = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                    break;
                }
                create_shader_resource_view(tex->native_resource, &srvDesc, srv);
                current_offset_cursor += heap->get_descriptor_size() * 1;
            }
            // Create UAV
            if(usages & TVU_UAV)
            {
                tex_view->m_uavDescriptorOffset = current_offset_cursor;
                current_offset_cursor += heap->get_descriptor_size() * 1;
                D3D12_CPU_DESCRIPTOR_HANDLE uav = { tex_view->m_dxDescriptorHandles.ptr + tex_view->m_uavDescriptorOffset };
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.m_format, true);
                cyber_assert(viewDesc.m_mipLevelCount <= 1, "UAVs can only be created for a single mip level");
                switch(viewDesc.m_dimension)
                {
                    case TEX_DIMENSION_1D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                        uavDesc.Texture1D.MipSlice = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_1D_ARRAY:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                        uavDesc.Texture1DArray.MipSlice = viewDesc.m_baseMipLevel;
                        uavDesc.Texture1DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        uavDesc.Texture1DArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_2D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                        uavDesc.Texture2D.MipSlice = viewDesc.m_baseMipLevel;
                        uavDesc.Texture2D.PlaneSlice = 0;
                    }
                    break;
                    case TEX_DIMENSION_2D_ARRAY:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        uavDesc.Texture2DArray.MipSlice = viewDesc.m_baseMipLevel;
                        uavDesc.Texture2DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        uavDesc.Texture2DArray.ArraySize = viewDesc.m_arrayLayerCount;
                        uavDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    break;
                    case TEX_DIMENSION_3D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                        uavDesc.Texture3D.MipSlice = viewDesc.m_baseMipLevel;
                        uavDesc.Texture3D.FirstWSlice = 0;
                        uavDesc.Texture3D.WSize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                    break;
                }
                create_unordered_access_view(tex->native_resource, NULL, &uavDesc, uav);
            }
        }

        // Create RTV
        if(usages & TVU_RTV_DSV)
        {
            const bool isDSV = FormatUtil_IsDepthStencilFormat(viewDesc.m_format);

            if(isDSV)
            {
                DescriptorHeap_D3D12* dsv_heap = m_cpuDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
                tex_view->m_rtvDsvDescriptorHandle = DescriptorHeap_D3D12::consume_descriptor_handles(dsv_heap, 1).mCpu;
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.m_format, false);
                switch (viewDesc.m_dimension)
                {
                    case TEX_DIMENSION_1D:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                        dsvDesc.Texture1D.MipSlice = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_1D_ARRAY:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                        dsvDesc.Texture1DArray.MipSlice = viewDesc.m_baseMipLevel;
                        dsvDesc.Texture1DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        dsvDesc.Texture1DArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_2D:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                        dsvDesc.Texture2D.MipSlice = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_2D_ARRAY:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        dsvDesc.Texture2DArray.MipSlice = viewDesc.m_baseMipLevel;
                        dsvDesc.Texture2DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        dsvDesc.Texture2DArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_2DMS:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    }
                    break;
                    case TEX_DIMENSION_2DMS_ARRAY:
                    {
                        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        dsvDesc.Texture2DMSArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        dsvDesc.Texture2DMSArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                        break;
                }
                create_depth_stencil_view(tex->native_resource, &dsvDesc, tex_view->m_rtvDsvDescriptorHandle);
            }
            else
            {
                DescriptorHeap_D3D12* rtv_heap = m_cpuDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
                tex_view->m_rtvDsvDescriptorHandle = DescriptorHeap_D3D12::consume_descriptor_handles(rtv_heap, 1).mCpu;
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = DXGIUtil_TranslatePixelFormat(viewDesc.m_format, true);
                switch(viewDesc.m_dimension)
                {
                    case TEX_DIMENSION_1D:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                        rtvDesc.Texture1D.MipSlice = viewDesc.m_baseMipLevel;
                    }
                    break;
                    case TEX_DIMENSION_1D_ARRAY:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        rtvDesc.Texture1DArray.MipSlice = viewDesc.m_baseMipLevel;
                        rtvDesc.Texture1DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        rtvDesc.Texture1DArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_2D:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                        rtvDesc.Texture2D.MipSlice = viewDesc.m_baseMipLevel;
                        rtvDesc.Texture2D.PlaneSlice = 0;
                    }
                    break;
                    case TEX_DIMENSION_2DMS:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    }
                    break;
                    case TEX_DIMENSION_2D_ARRAY:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        rtvDesc.Texture2DArray.MipSlice = viewDesc.m_baseMipLevel;
                        rtvDesc.Texture2DArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        rtvDesc.Texture2DArray.ArraySize = viewDesc.m_arrayLayerCount;
                        rtvDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    break;
                    case TEX_DIMENSION_2DMS_ARRAY:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        rtvDesc.Texture2DMSArray.FirstArraySlice = viewDesc.m_baseArrayLayer;
                        rtvDesc.Texture2DMSArray.ArraySize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    case TEX_DIMENSION_3D:
                    {
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                        rtvDesc.Texture3D.MipSlice = viewDesc.m_baseMipLevel;
                        rtvDesc.Texture3D.FirstWSlice = viewDesc.m_baseArrayLayer;
                        rtvDesc.Texture3D.WSize = viewDesc.m_arrayLayerCount;
                    }
                    break;
                    default:
                        cyber_assert(false, "Invalid texture dimension");
                        break;
                }
                create_render_target_view(tex->native_resource, &rtvDesc, tex_view->m_rtvDsvDescriptorHandle);
            }
        }
        return tex_view;
    }

    void RenderDevice_D3D12_Impl::free_texture_view(RenderObject::ITextureView* view)
    {
        RenderObject::TextureView_D3D12_Impl* tex_view = static_cast<RenderObject::TextureView_D3D12_Impl*>(view);
        const auto usages = tex_view->m_desc.m_usages;
        const bool isDSV = FormatUtil_IsDepthStencilFormat(tex_view->m_desc.m_format);
        if(tex_view->m_dxDescriptorHandles.ptr != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
        {
            uint32_t handleCount = ((usages & TVU_SRV) ? 1 : 0) + ((usages & TVU_UAV) ? 1 : 0);
            
        }
    }

    RenderObject::ITexture* RenderDevice_D3D12_Impl::create_texture(const RenderObject::TextureCreateDesc& pDesc)
    {
        RenderObject::Texture_D3D12_Impl* pTexture = cyber_new<RenderObject::Texture_D3D12_Impl>(this);
        cyber_assert(pTexture != nullptr, "rhi texture create failed!");

        D3D12_RESOURCE_DESC desc = {};

        //TODO:
        DXGI_FORMAT dxFormat = DXGIUtil_TranslatePixelFormat(pDesc.m_format);
        
        DESCRIPTOR_TYPE descriptors = pDesc.m_descriptors;

        if(pDesc.m_pNativeHandle == nullptr)
        {
            D3D12_RESOURCE_DIMENSION res_dim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
            if(pDesc.m_flags & TCF_FORCE_2D)
            {
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }
            else if(pDesc.m_flags & TCF_FORCE_3D)
            {
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            }
            else
            {
                if(pDesc.m_depth > 1)
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                else if(pDesc.m_height > 1)
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                else
                    res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            }

            desc.Dimension = res_dim;
            desc.Alignment = (UINT)pDesc.m_sampleCount > 1 ? D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT : 0;
            desc.Width = pDesc.m_width;
            desc.Height = pDesc.m_height;
            desc.DepthOrArraySize = (UINT)(pDesc.m_arraySize != 1 ? pDesc.m_arraySize : pDesc.m_depth);
            desc.MipLevels = (UINT)pDesc.m_mipLevels;
            desc.Format = DXGIUtil_FormatToTypeless(dxFormat);
            desc.SampleDesc.Count = (UINT)pDesc.m_sampleCount;
            desc.SampleDesc.Quality = (UINT)pDesc.m_sampleQuality;
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

            GRAPHICS_RESOURCE_STATE actualStartState = (GRAPHICS_RESOURCE_STATE)pDesc.m_startState;

            // Decide UAV flags
            if(descriptors & DESCRIPTOR_TYPE_RW_TEXTURE)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            // Decide render target flags
            if(pDesc.m_startState & GRAPHICS_RESOURCE_STATE_RENDER_TARGET)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                actualStartState = (GRAPHICS_RESOURCE_STATE)((pDesc.m_startState > GRAPHICS_RESOURCE_STATE_RENDER_TARGET)
                                    ? (pDesc.m_startState & (GRAPHICS_RESOURCE_STATE)~GRAPHICS_RESOURCE_STATE_RENDER_TARGET)
                                    : GRAPHICS_RESOURCE_STATE_RENDER_TARGET);
            }
            else if(pDesc.m_startState & GRAPHICS_RESOURCE_STATE_DEPTH_WRITE)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                actualStartState = (GRAPHICS_RESOURCE_STATE)((pDesc.m_startState > GRAPHICS_RESOURCE_STATE_DEPTH_WRITE)
                                    ? (pDesc.m_startState & (GRAPHICS_RESOURCE_STATE)~GRAPHICS_RESOURCE_STATE_DEPTH_WRITE)
                                    : GRAPHICS_RESOURCE_STATE_DEPTH_WRITE);
            }

            // Decide sharing flags for multi adapter
            if(pDesc.m_flags & TCF_EXPORT_ADAPTER_BIT)
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            }

            if(pDesc.m_flags & TCF_FORCE_ALLOW_DISPLAY_TARGET)
            {
                actualStartState = GRAPHICS_RESOURCE_STATE_PRESENT;
            }

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = dxFormat;
            if(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                clearValue.DepthStencil.Depth = pDesc.m_clearValue.depth;
                clearValue.DepthStencil.Stencil = (UINT8)pDesc.m_clearValue.stencil;
            }
            else
            {
                clearValue.Color[0] = pDesc.m_clearValue.r;
                clearValue.Color[1] = pDesc.m_clearValue.g;
                clearValue.Color[2] = pDesc.m_clearValue.b;
                clearValue.Color[3] = pDesc.m_clearValue.a;
            }

            D3D12_CLEAR_VALUE* pClearValue = nullptr;
            D3D12_RESOURCE_STATES res_states = D3D12Util_TranslateResourceState(actualStartState);

            if((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            {
                pClearValue = &clearValue;
            }

            D3D12MA::ALLOCATION_DESC alloc_desc = {};
            alloc_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            if(pDesc.m_flags & TCF_OWN_MEMORY_BIT)
                alloc_desc.Flags = (D3D12MA::ALLOCATION_FLAGS)(alloc_desc.Flags | D3D12MA::ALLOCATION_FLAG_COMMITTED);

            // Create resource
            auto hRes = m_pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, pClearValue, &pTexture->allocation, IID_ARGS(&pTexture->native_resource));
            if(hRes != S_OK)
            {
                auto fallbackhRes = hRes;
                CB_CORE_ERROR("[D3D12] Create Texture Resource Failed With HRESULT {0}! \n\t With Name: {1} \n\t Size: {2}{3} \n\t Format: {4} \n\t Sample Count: {5}", 
                                hRes, (char*)pDesc.m_name ? (char*)pDesc.m_name : "", pDesc.m_width, pDesc.m_height,
                                pDesc.m_format, pDesc.m_sampleCount);
                const bool use_fallback_commited = true;
                if(use_fallback_commited)
                {
                    D3D12_HEAP_PROPERTIES heapProps = {};
                    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
                    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    heapProps.CreationNodeMask = GRAPHICS_SINGLE_GPU_NODE_MASK;
                    heapProps.VisibleNodeMask = GRAPHICS_SINGLE_GPU_NODE_MASK;
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
                                (char*)pDesc.m_name ? (char*)pDesc.m_name : "", pDesc.m_width, pDesc.m_height,
                                pDesc.m_format, pDesc.m_sampleCount);
            }


        }

        return pTexture;
    }

    IInstance* RenderDevice_D3D12_Impl::create_instance(const InstanceCreateDesc& instanceDesc)
    {
        Instance_D3D12_Impl* instance = cyber_new<Instance_D3D12_Impl>(this, instanceDesc);
        // Initialize driver
        instance->initialize_environment();
        // Enable Debug Layer
        instance->optional_enable_debug_layer();

        UINT flags = 0;
        if(instanceDesc.m_enableDebugLayer)
            flags = DXGI_CREATE_FACTORY_DEBUG;
        
        if(SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&instance->m_pDXGIFactory))))
        {
            uint32_t gpuCount = 0;
            bool foundSoftwareAdapter = false;
            instance->query_all_adapters(gpuCount, foundSoftwareAdapter);
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

    }

    Surface* RenderDevice_D3D12_Impl::surface_from_hwnd(HWND window)
    {
        Surface* surface = cyber_new<Surface>();
        surface->handle = window;
        //surface = (RHISurface*)(&window);
        return surface;
    }

    IFence* RenderDevice_D3D12_Impl::create_fence()
    {
        Fence_D3D12_Impl* dxFence = cyber_new<Fence_D3D12_Impl>();
        cyber_assert(dxFence, "Fence create failed!");
        CHECK_HRESULT(m_pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dxFence->m_pDxFence)));
        dxFence->m_fenceValue = 0;

        dxFence->m_dxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        return dxFence;
    }

    FENCE_STATUS RenderDevice_D3D12_Impl::query_fence_status(IFence* fence)
    {
        Fence_D3D12_Impl* dxFence = static_cast<Fence_D3D12_Impl*>(fence);
        uint64_t fenceValue = dxFence->m_pDxFence->GetCompletedValue();
        if(fenceValue < dxFence->m_fenceValue)
            return FENCE_STATUS_INCOMPLETE;
        else 
            return FENCE_STATUS_COMPLETE;
    }

    void RenderDevice_D3D12_Impl::wait_fences(IFence** fences, uint32_t fenceCount)
    {
        for(uint32_t i = 0; i < fenceCount; ++i)
        {
            FENCE_STATUS fence_status = query_fence_status(fences[i]);
            Fence_D3D12_Impl* dxFence = static_cast<Fence_D3D12_Impl*>(fences[i]);
            uint64_t fence_value = dxFence->m_fenceValue;
            if(fence_status == FENCE_STATUS_INCOMPLETE)
            {
                dxFence->m_pDxFence->SetEventOnCompletion(fence_value, dxFence->m_dxWaitIdleFenceEvent);
                WaitForSingleObject(dxFence->m_dxWaitIdleFenceEvent, INFINITE);
            }
        }
    }

    void RenderDevice_D3D12_Impl::free_fence(IFence* fence)
    {
        Fence_D3D12_Impl* dxFence = static_cast<Fence_D3D12_Impl*>(fence);
        SAFE_RELEASE(dxFence->m_pDxFence);
        CloseHandle(dxFence->m_dxWaitIdleFenceEvent);
        cyber_delete(fence);
    }

    IQueue* RenderDevice_D3D12_Impl::get_queue(QUEUE_TYPE type, uint32_t index)
    {
        Queue_D3D12_Impl* dxQueue = cyber_new<Queue_D3D12_Impl>(this);
        dxQueue->m_pCommandQueue = m_commandQueues[type][index];
        dxQueue->m_pFence = create_fence();
        return dxQueue;
    }
    void RenderDevice_D3D12_Impl::submit_queue(IQueue* queue, const QueueSubmitDesc& submitDesc)
    {
        uint32_t cmd_count = submitDesc.m_cmdsCount;
        Queue_D3D12_Impl* dx_queue = static_cast<Queue_D3D12_Impl*>(queue);
        Fence_D3D12_Impl* dx_fence = static_cast<Fence_D3D12_Impl*>(submitDesc.m_pSignalFence);

        cyber_check(submitDesc.m_cmdsCount > 0);
        cyber_check(submitDesc.m_ppCmds);

        ID3D12CommandList** cmds = (ID3D12CommandList**)cyber_malloc(sizeof(ID3D12CommandList*) * cmd_count);
        for(uint32_t i = 0; i < cmd_count; i++)
        {
            CommandBuffer_D3D12_Impl* dx_cmd = static_cast<CommandBuffer_D3D12_Impl*>(submitDesc.m_ppCmds[i]);
            cmds[i] = dx_cmd->get_dx_cmd_list();
        }
        // Wait semaphores
        for(uint32_t i = 0; i < submitDesc.m_waitSemaphoreCount; i++)
        {
            Semaphore_D3D12_Impl* dx_semaphore = static_cast<Semaphore_D3D12_Impl*>(submitDesc.m_ppWaitSemaphores[i]);
            dx_queue->get_native_queue()->Wait(dx_semaphore->dx_fence, dx_semaphore->fence_value - 1);
        }
        // Execute
        dx_queue->get_native_queue()->ExecuteCommandLists(cmd_count, cmds);
        // Signal fences
        if(dx_fence)
        {
            dx_queue->signal_fence(dx_fence, ++dx_fence->m_fenceValue);
        }
        // Signal semaphores
        for(uint32_t i = 0; i < submitDesc.m_signalSemaphoreCount; i++)
        {
            Semaphore_D3D12_Impl* dx_semaphore = static_cast<Semaphore_D3D12_Impl*>(submitDesc.m_ppSignalSemaphores[i]);
            dx_queue->get_native_queue()->Signal(dx_semaphore->dx_fence, dx_semaphore->fence_value++);
        }
    }

    void RenderDevice_D3D12_Impl::present_queue(IQueue* queue, const QueuePresentDesc& presentDesc)
    {
        SwapChain_D3D12_Impl* dx_swapchain = static_cast<SwapChain_D3D12_Impl*>(presentDesc.m_pSwapChain);
        
        HRESULT hr =  dx_swapchain->get_dx_swap_chain()->Present(0, dx_swapchain->get_flags());

        if(FAILED(hr))
        {
            CB_ERROR("Present failed!");
            #if defined(_WIN32)
            #endif
        }
    }
    void RenderDevice_D3D12_Impl::wait_queue_idle(IQueue* queue)
    {
        Queue_D3D12_Impl* Queue = static_cast<Queue_D3D12_Impl*>(queue);
        Fence_D3D12_Impl* Fence = static_cast<Fence_D3D12_Impl*>(Queue->m_pFence);
        Queue->signal_fence(Fence, Fence->m_fenceValue);
        uint64_t fence_value = Fence->m_fenceValue - 1;
        if(Fence->m_pDxFence->GetCompletedValue() < fence_value)
        {
            Fence->m_pDxFence->SetEventOnCompletion(fence_value, Fence->m_dxWaitIdleFenceEvent);
            WaitForSingleObject(Fence->m_dxWaitIdleFenceEvent, INFINITE);
        }
    }

    // Command Objects
    void allocate_transient_command_allocator(ID3D12Device* d3d12_device, CommandPool_D3D12_Impl* commandPool, IQueue* queue)
    {
        D3D12_COMMAND_LIST_TYPE type = queue->get_type() == QUEUE_TYPE_TRANSFER ? D3D12_COMMAND_LIST_TYPE_COPY : 
                            (queue->get_type() == QUEUE_TYPE_COMPUTE ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT);
        
        auto command_allocator = commandPool->get_native_command_allocator();
        bool res = SUCCEEDED(d3d12_device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator)));
        if(!res)
        {
            cyber_assert(false, "command allocator create failed!");
        }
        commandPool->set_queue(queue);
    }

    ICommandPool* RenderDevice_D3D12_Impl::create_command_pool(IQueue* queue, const CommandPoolCreateDesc& commandPoolDesc)
    {
        CommandPool_D3D12_Impl* dxCommandPool = cyber_new<CommandPool_D3D12_Impl>();
        allocate_transient_command_allocator(m_pDxDevice, dxCommandPool, queue);
        return dxCommandPool;
    }
    void RenderDevice_D3D12_Impl::reset_command_pool(ICommandPool* pool)
    {
        CommandPool_D3D12_Impl* dxPool = static_cast<CommandPool_D3D12_Impl*>(pool);
        dxPool->m_pDxCmdAlloc->Reset();
    }
    void RenderDevice_D3D12_Impl::free_command_pool(ICommandPool* pool)
    {
        CommandPool_D3D12_Impl* dxPool = static_cast<CommandPool_D3D12_Impl*>(pool);
        SAFE_RELEASE(dxPool->m_pDxCmdAlloc);
        cyber_delete(pool);
    }

    ICommandBuffer* RenderDevice_D3D12_Impl::create_command_buffer(ICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc) 
    {
        CommandBuffer_D3D12_Impl* dxCommandBuffer = cyber_new<CommandBuffer_D3D12_Impl>();
        CommandPool_D3D12_Impl* dxPool = static_cast<CommandPool_D3D12_Impl*>(pool);
        Queue_D3D12_Impl* dxQueue = static_cast<Queue_D3D12_Impl*>(dxPool->get_queue());

        // set command pool of new command
        dxCommandBuffer->set_node_index(GRAPHICS_SINGLE_GPU_NODE_INDEX);
        dxCommandBuffer->set_type(dxQueue->get_type());
        dxCommandBuffer->set_bound_heap(0, m_cbvSrvUavHeaps[dxCommandBuffer->m_nodeIndex]);
        dxCommandBuffer->set_bound_heap(1, m_samplerHeaps[dxCommandBuffer->m_nodeIndex]);

        dxCommandBuffer->m_pCmdPool = pool;
        
        uint32_t nodeMask = dxCommandBuffer->m_nodeIndex;
        ID3D12PipelineState* initialState = nullptr;
        auto cmd_list = dxCommandBuffer->get_dx_cmd_list();
        CHECK_HRESULT(m_pDxDevice->CreateCommandList(nodeMask,gDx12CmdTypeTranslator[dxCommandBuffer->m_type] , 
                dxPool->m_pDxCmdAlloc, initialState, IID_PPV_ARGS(&cmd_list)));
        
        // Command lists are add in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        CHECK_HRESULT(dxCommandBuffer->get_dx_cmd_list()->Close());
        return dxCommandBuffer;
    }

    void RenderDevice_D3D12_Impl::free_command_buffer(ICommandBuffer* commandBuffer)
    {
        CommandBuffer_D3D12_Impl* dxCommandBuffer = static_cast<CommandBuffer_D3D12_Impl*>(commandBuffer);
        dxCommandBuffer->free();
    }

    void RenderDevice_D3D12_Impl::cmd_begin(ICommandBuffer* commandBuffer)
    {
        CommandBuffer_D3D12_Impl* cmd = static_cast<CommandBuffer_D3D12_Impl*>(commandBuffer);
        CommandPool_D3D12_Impl* pool = static_cast<CommandPool_D3D12_Impl*>(cmd->m_pCmdPool);
        CHECK_HRESULT(cmd->get_dx_cmd_list()->Reset(pool->m_pDxCmdAlloc, nullptr));

        // Reset the descriptor heaps
        if(cmd->m_type != QUEUE_TYPE_TRANSFER)
        {
            ID3D12DescriptorHeap* heaps[] = {
                cmd->m_pBoundHeaps[0]->get_heap(),
                cmd->m_pBoundHeaps[1]->get_heap()
            };
            cmd->get_dx_cmd_list()->SetDescriptorHeaps(2, heaps);

            cmd->m_boundHeapStartHandles[0] = cmd->m_pBoundHeaps[0]->get_heap()->GetGPUDescriptorHandleForHeapStart();
            cmd->m_boundHeapStartHandles[1] = cmd->m_pBoundHeaps[1]->get_heap()->GetGPUDescriptorHandleForHeapStart();
        }
        // Reset CPU side data
        cmd->m_pBoundRootSignature = nullptr;
    }

    void RenderDevice_D3D12_Impl::cmd_end(ICommandBuffer* commandBuffer)
    {
        CommandBuffer_D3D12_Impl* cmd = static_cast<CommandBuffer_D3D12_Impl*>(commandBuffer);
        cyber_check(cmd->get_dx_cmd_list());
        CHECK_HRESULT(cmd->get_dx_cmd_list()->Close());
    }

    void RenderDevice_D3D12_Impl::cmd_resource_barrier(ICommandBuffer* cmd, const ResourceBarrierDesc& barrierDesc)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(cmd);
        const uint32_t barriers_count = barrierDesc.buffer_barrier_count + barrierDesc.texture_barrier_count;
        D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*)alloca(sizeof(D3D12_RESOURCE_BARRIER) * barriers_count);
        uint32_t transition_count = 0;
        for(uint32_t i = 0; i < barrierDesc.buffer_barrier_count; ++i)
        {
            const BufferBarrier* transition_barrier = &barrierDesc.buffer_barriers[i];
            D3D12_RESOURCE_BARRIER* barrier = &barriers[transition_count];
            RenderObject::Buffer_D3D12_Impl* buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(transition_barrier->buffer);
            if(buffer->m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_ONLY ||
                buffer->m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_TO_CPU ||
                (buffer->m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_TO_GPU && buffer->m_descriptors & GRAPHICS_RESOURCE_TYPE_RW_BUFFER))
                {
                    if(transition_barrier->src_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS &&
                        transition_barrier->dst_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS)
                    {
                            barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                            barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            barrier->UAV.pResource = buffer->m_pDxResource;
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
                            barrier->Transition.pResource = buffer->get_dx_resource();
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
            const TextureBarrier* transition_barrier = &barrierDesc.texture_barriers[i];
            D3D12_RESOURCE_BARRIER* barrier = &barriers[transition_count];
            RenderObject::Texture_D3D12_Impl* texture = static_cast<RenderObject::Texture_D3D12_Impl*>(transition_barrier->texture);
            if(transition_barrier->src_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS &&
                transition_barrier->dst_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier->UAV.pResource = texture->get_d3d12_resource();
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
                barrier->Transition.pResource = texture->get_d3d12_resource();
                barrier->Transition.Subresource = transition_barrier->subresource_barrier ?
                                                 CALC_SUBRESOURCE_INDEX(transition_barrier->mip_level, transition_barrier->array_layer, 0, texture->m_mipLevels, texture->m_arraySize + 1)
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
                    if(transition_barrier->src_state == GRAPHICS_RESOURCE_STATE_PRESENT || transition_barrier->dst_state == D3D12_RESOURCE_STATE_COMMON)
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
            Cmd->get_dx_cmd_list()->ResourceBarrier(transition_count, barriers);
        }
    }

    void reset_root_signature(CommandBuffer_D3D12_Impl* cmd, PIPELINE_TYPE type, ID3D12RootSignature* rootSignature)
    {
        if(cmd->get_bound_root_signature() != rootSignature)
        {
            cmd->set_bound_root_signature(rootSignature);
            if(type == PIPELINE_TYPE_GRAPHICS)
                cmd->get_dx_cmd_list()->SetGraphicsRootSignature(rootSignature);
            else
                cmd->get_dx_cmd_list()->SetComputeRootSignature(rootSignature);
        }
    }

    RenderPassEncoder* RenderDevice_D3D12_Impl::cmd_begin_render_pass(ICommandBuffer* cmd, const RenderPassDesc& beginRenderPassDesc)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(cmd);
    #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
        ID3D12GraphicsCommandList4* cmdList4 = (ID3D12GraphicsCommandList4*)Cmd->get_dx_cmd_list();
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValues[GRAPHICS_MAX_MRT_COUNT]);
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearDepth);
        DECLARE_ZERO(D3D12_CLEAR_VALUE, clearStencil);
        DECLARE_ZERO(D3D12_RENDER_PASS_RENDER_TARGET_DESC, renderPassRenderTargetDescs[GRAPHICS_MAX_MRT_COUNT]);
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
        CommandBuffer_D3D12_Impl* cmd = static_cast<CommandBuffer_D3D12_Impl*>(pCommandBuffer);
        #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
            ID3D12GraphicsCommandList4* cmdList4 = (ID3D12GraphicsCommandList4*)cmd->get_dx_cmd_list();
            cmdList4->EndRenderPass();
            return;
        #endif
        cyber_warn(false, "ID3D12GraphicsCommandList4 is not defined!");
    }
    
    void RenderDevice_D3D12_Impl::render_encoder_bind_descriptor_set(RenderPassEncoder* encoder, RenderObject::IDescriptorSet* descriptorSet)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        const RenderObject::DescriptorSet_D3D12_Impl* Set = static_cast<const RenderObject::DescriptorSet_D3D12_Impl*>(descriptorSet);
        RenderObject::RootSignature_D3D12_Impl* RS = static_cast<RenderObject::RootSignature_D3D12_Impl*>(Set->get_root_signature());

        cyber_check(RS);
        reset_root_signature(Cmd, PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
        if(Set->cbv_srv_uav_handle != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
        {
            Cmd->get_dx_cmd_list()->SetGraphicsRootDescriptorTable(Set->get_set_index(), {Cmd->m_boundHeapStartHandles[0].ptr + Set->cbv_srv_uav_handle});
        }
        else if(Set->sampler_handle != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
        {
            Cmd->get_dx_cmd_list()->SetGraphicsRootDescriptorTable(Set->get_set_index(), {Cmd->m_boundHeapStartHandles[1].ptr + Set->sampler_handle});
        }
    }
    void RenderDevice_D3D12_Impl::render_encoder_set_viewport(RenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        D3D12_VIEWPORT viewport = { x, y, width, height, min_depth, max_depth };
        Cmd->get_dx_cmd_list()->RSSetViewports(1, &viewport);
    }
    void RenderDevice_D3D12_Impl::render_encoder_set_scissor(RenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        D3D12_RECT rect;
        rect.left = x;
        rect.top = y;
        rect.right = x + width;
        rect.bottom = y + height;
        Cmd->get_dx_cmd_list()->RSSetScissorRects(1, &rect);
    }
    void RenderDevice_D3D12_Impl::render_encoder_bind_pipeline(RenderPassEncoder* encoder, RenderObject::IRenderPipeline* pipeline)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        RenderObject::RenderPipeline_D3D12_Impl* Pipeline = static_cast<RenderObject::RenderPipeline_D3D12_Impl*>(pipeline);
        reset_root_signature(Cmd, PIPELINE_TYPE_GRAPHICS, Pipeline->pDxRootSignature);
        Cmd->get_dx_cmd_list()->IASetPrimitiveTopology(Pipeline->mPrimitiveTopologyType);
        Cmd->get_dx_cmd_list()->SetPipelineState(Pipeline->pDxPipelineState);
    }
    void RenderDevice_D3D12_Impl::render_encoder_bind_vertex_buffer(RenderPassEncoder* encoder, uint32_t buffer_count, RenderObject::IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);

        DECLARE_ZERO(D3D12_VERTEX_BUFFER_VIEW, views[GRAPHICS_MAX_VERTEX_ATTRIBUTES]);
        for(uint32_t i = 0;i < buffer_count; ++i)
        {
            const RenderObject::Buffer_D3D12_Impl* Buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(buffers[i]);
            cyber_check(Buffer->get_dx_gpu_address() != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN);

            views[i].BufferLocation = Buffer->get_dx_gpu_address() + (offsets ? offsets[i] : 0);
            views[i].SizeInBytes = (UINT)(Buffer->get_size() - (offsets ? offsets[i] : 0));
            views[i].StrideInBytes = (UINT)strides[i];  
        }
        Cmd->get_dx_cmd_list()->IASetVertexBuffers(0, buffer_count, views);
    }
    void RenderDevice_D3D12_Impl::render_encoder_bind_index_buffer(RenderPassEncoder* encoder, RenderObject::IBuffer* buffer, uint32_t index_stride, uint64_t offset)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        const RenderObject::Buffer_D3D12_Impl* Buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(buffer);

        DECLARE_ZERO(D3D12_INDEX_BUFFER_VIEW, view);
        view.BufferLocation = Buffer->get_dx_gpu_address() + offset;
        view.SizeInBytes = (UINT)(Buffer->get_size() - offset);
        view.Format = index_stride == sizeof(uint16_t) ? DXGI_FORMAT_R16_UINT : ((index_stride == sizeof(uint8_t) ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32_UINT));
        Cmd->get_dx_cmd_list()->IASetIndexBuffer(&view);
    }
    void RenderDevice_D3D12_Impl::render_encoder_push_constants(RenderPassEncoder* encoder, RenderObject::IRootSignature* rs, const char8_t* name, const void* data)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        RootSignature_D3D12_Impl* RS = static_cast<RootSignature_D3D12_Impl*>(rs);
        reset_root_signature(Cmd, PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
        Cmd->get_dx_cmd_list()->SetGraphicsRoot32BitConstants(RS->root_parameter_index, RS->root_constant_parameter.Constants.Num32BitValues, data, 0);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw(RenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        Cmd->get_dx_cmd_list()->DrawInstanced((UINT)vertex_count, (UINT)1, (UINT)first_vertex, (UINT)0);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw_instanced(RenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        Cmd->get_dx_cmd_list()->DrawInstanced((UINT)vertex_count, (UINT)instance_count, (UINT)first_vertex, (UINT)first_instance);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw_indexed(RenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        Cmd->get_dx_cmd_list()->DrawIndexedInstanced((UINT)index_count, (UINT)1, (UINT)first_index, (UINT)first_vertex, (UINT)0);
    }
    void RenderDevice_D3D12_Impl::render_encoder_draw_indexed_instanced(RenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
    {
        CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
        Cmd->get_dx_cmd_list()->DrawIndexedInstanced((UINT)index_count, (UINT)instance_count, (UINT)first_index, (UINT)first_vertex, (UINT)first_instance);
    }

    ISwapChain* RenderDevice_D3D12_Impl::create_swap_chain(const SwapChainDesc& desc)
    {
        Instance_D3D12_Impl* dxInstance = static_cast<Instance_D3D12_Impl*>(m_pAdapter->get_instance());
        const uint32_t buffer_count = desc.m_imageCount;
        SwapChain_D3D12_Impl* dxSwapChain = (SwapChain_D3D12_Impl*)cyber_calloc(1, sizeof(SwapChain_D3D12_Impl));
        dxSwapChain->set_dx_sync_interval(desc.m_enableVsync ? 1 : 0);

        DECLARE_ZERO(DXGI_SWAP_CHAIN_DESC1, chinDesc);
        chinDesc.Width = desc.m_width;
        chinDesc.Height = desc.m_height;
        chinDesc.Format = DXGIUtil_TranslatePixelFormat(desc.m_format);
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
        dxInstance->get_dxgi_factory()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        chinDesc.Flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        auto flag = dxSwapChain->get_flags() | (!desc.m_enableVsync && allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;
        dxSwapChain->set_flags(flag);    
        IDXGISwapChain1* swapchain;

        HWND hwnd = desc.m_pSurface->handle;

        Queue_D3D12_Impl* queue = nullptr;
        if(desc.m_presentQueue)
        {
            queue = static_cast<Queue_D3D12_Impl*>(desc.m_presentQueue);
        }
        else 
        {
            queue = static_cast<Queue_D3D12_Impl*>(get_queue(QUEUE_TYPE_GRAPHICS, 0));
        }

        auto bCreated = SUCCEEDED(dxInstance->get_dxgi_factory()->CreateSwapChainForHwnd(m_pDxDevice, hwnd, &chinDesc, NULL, NULL, &swapchain));
        cyber_assert(bCreated, "Failed to try to create swapchain! An existed swapchain might be destroyed!");

        bCreated = SUCCEEDED(dxInstance->get_dxgi_factory()->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
        cyber_assert(bCreated, "Failed to try to associate swapchain with window!");

        auto swap_chain = dxSwapChain->get_dx_swap_chain();
        auto bQueryChain3 = SUCCEEDED(swapchain->QueryInterface(IID_PPV_ARGS(&swap_chain)));
        cyber_assert(bQueryChain3, "Failed to query IDXGISwapChain3 from created swapchain!");

        SAFE_RELEASE(swapchain);
        // Get swapchain images
        ID3D12Resource** backbuffers = (ID3D12Resource**)alloca(buffer_count * sizeof(ID3D12Resource*));
        for(uint32_t i = 0; i < buffer_count; ++i)
        {
            CHECK_HRESULT(dxSwapChain->get_dx_swap_chain()->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i])));
        }

        dxSwapChain->m_ppBackBufferSRVs = (RenderObject::ITexture**)cyber_malloc(buffer_count * sizeof(RenderObject::ITexture*));
        for(uint32_t i = 0; i < buffer_count; i++)
        {
            RenderObject::Texture_D3D12_Impl* Ts = cyber_new<RenderObject::Texture_D3D12_Impl>(this);
            Ts->native_resource = backbuffers[i];
            Ts->allocation = nullptr;
            Ts->m_isCube = false;
            Ts->m_arraySize = 0;
            Ts->m_format = desc.m_format;
            Ts->m_aspectMask = 1;
            Ts->m_depth = 1;
            Ts->m_width = desc.m_width;
            Ts->m_height = desc.m_height;
            Ts->m_mipLevels = 1;
            Ts->m_nodeIndex = GRAPHICS_SINGLE_GPU_NODE_INDEX;
            Ts->m_ownsImage = false;
            Ts->m_pNativeHandle = Ts->native_resource;
            dxSwapChain->m_ppBackBufferSRVs[i] = Ts;
        }
        //dxSwapChain->mBackBuffers = Ts;
        dxSwapChain->m_bufferSRVCount = buffer_count;

        // Create depth stencil view
        //dxSwapChain->mBackBufferDSV = (RHITexture*)cyber_malloc(sizeof(RHITexture));
        TextureCreateDesc depthStencilDesc = {};
        depthStencilDesc.m_height = desc.m_height;
        depthStencilDesc.m_width = desc.m_width;
        depthStencilDesc.m_depth = 1;
        depthStencilDesc.m_arraySize = 1;
        depthStencilDesc.m_format = TEXTURE_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.m_mipLevels = 1;
        depthStencilDesc.m_sampleCount = SAMPLE_COUNT_1;
        depthStencilDesc.m_descriptors = DESCRIPTOR_TYPE_UNDEFINED;
        depthStencilDesc.m_startState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
        depthStencilDesc.m_name = u8"Main Depth Stencil";
        dxSwapChain->m_pBackBufferDSV = create_texture(depthStencilDesc);

        auto dsv = static_cast<RenderObject::Texture_D3D12_Impl*>(dxSwapChain->m_pBackBufferDSV);

        TextureViewCreateDesc depthStencilViewDesc = {};
        depthStencilViewDesc.m_pTexture = dxSwapChain->m_pBackBufferDSV;
        depthStencilViewDesc.m_dimension = TEX_DIMENSION_2D;
        depthStencilViewDesc.m_format = TEXTURE_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.m_usages = TVU_RTV_DSV;
        depthStencilViewDesc.m_aspects = TVA_DEPTH;
        depthStencilViewDesc.m_arrayLayerCount = 1;
        dxSwapChain->m_pBackBufferDSVView = create_texture_view(depthStencilViewDesc);

        dxSwapChain->get_dx_swap_chain()->GetCurrentBackBufferIndex();
        return dxSwapChain;
    }

    void RenderDevice_D3D12_Impl::free_swap_chain(ISwapChain* swapchain)
    {
        swapchain->free();
    }

    void RenderDevice_D3D12_Impl::enum_adapters(IInstance* instance, IAdapter** adapters, uint32_t* adapterCount)
    {
        cyber_assert(instance, "fatal: Invalid instance!");
        Instance_D3D12_Impl* dxInstance = static_cast<Instance_D3D12_Impl*>(instance);
        *adapterCount = dxInstance->get_adapters_count();
        if(!adapters)
        {
            return;
        }
        else
        {
            for(uint32_t i = 0; i < dxInstance->get_adapters_count(); ++i)
            {
                adapters[i] = dxInstance->get_adapter(i);
            }
        }
    }

    uint32_t RenderDevice_D3D12_Impl::acquire_next_image(ISwapChain* swapchain, const AcquireNextDesc& acquireDesc)
    {
        RenderObject::SwapChain_D3D12_Impl* dxSwapChain = static_cast<RenderObject::SwapChain_D3D12_Impl*>(swapchain);
        // On PC AquireNext is always true
        return dxSwapChain->get_dx_swap_chain()->GetCurrentBackBufferIndex();
    }

    IFrameBuffer* RenderDevice_D3D12_Impl::create_frame_buffer(const FrameBuffserDesc& frameBufferDesc)
    {
        return nullptr;
    }

    // for example 
    IRootSignature* RenderDevice_D3D12_Impl::create_root_signature(const RenderObject::RootSignatureCreateDesc& rootSigDesc)
    {
        RootSignature_D3D12_Impl* dxRootSignature = cyber_new<RootSignature_D3D12_Impl>();

        // Pick root parameters from desc data
        SHADER_STAGE shaderStages = SHADER_STAGE_NONE;
        for(uint32_t i = 0; i < rootSigDesc.m_shaderCount; ++i)
        {
            PipelineShaderCreateDesc* shader_desc = rootSigDesc.m_ppShaders[i];
            shaderStages |= shader_desc->m_stage;
        }

        // Pick shader reflection data
        graphics_util_init_root_signature_tables(dxRootSignature, rootSigDesc);
        // rs pool allocation
        
        // Fill resource slots
        const uint32_t tableCount = dxRootSignature->get_parameter_table_count();
        uint32_t descRangeCount = 0;
        for(uint32_t i = 0;i < tableCount; ++i)
        {
            descRangeCount += dxRootSignature->get_parameter_table(i)->m_resourceCount;
        }
        D3D12_ROOT_PARAMETER1* rootParams = (D3D12_ROOT_PARAMETER1*)cyber_calloc(tableCount + dxRootSignature->get_parameter_table_count(), sizeof(D3D12_ROOT_PARAMETER1));
        D3D12_DESCRIPTOR_RANGE1* descRanges = (D3D12_DESCRIPTOR_RANGE1*)cyber_calloc(descRangeCount, sizeof(D3D12_DESCRIPTOR_RANGE1));
        // Create descriptor table parameter
        uint32_t valid_root_tables = 0;
        for(uint32_t i_set = 0; i_set < tableCount; ++i_set)
        {
            RootSignatureParameterTable* paramTable = dxRootSignature->get_parameter_table(i_set);
            D3D12_ROOT_PARAMETER1 rootParam = rootParams[valid_root_tables];
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            SHADER_STAGE visStages = SHADER_STAGE::SHADER_STAGE_NONE;
            uint32_t i_range = 0;
            const D3D12_DESCRIPTOR_RANGE1* descRange = &descRanges[i_range];
            for(uint32_t i_register = 0; i_register < paramTable->m_resourceCount; ++i_register)
            {
                IShaderResource* resourceSlot = paramTable->m_ppResources[i_register];
                visStages |= resourceSlot->get_stages();
                D3D12_DESCRIPTOR_RANGE1* descRange = &descRanges[i_range];
                descRange->RangeType = D3D12Util_ResourceTypeToDescriptorRangeType(resourceSlot->get_type());
                descRange->NumDescriptors = (resourceSlot->get_type() != GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER) ? resourceSlot->get_size() : 1;
                descRange->BaseShaderRegister = resourceSlot->get_binding();
                descRange->RegisterSpace = resourceSlot->get_set();
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
        cyber_assert(dxRootSignature->get_push_constant_count() <= 1, "Only support one push constant range");
        if(dxRootSignature->get_push_constant_count() > 0)
        {
            auto pushConstant = dxRootSignature->get_push_constant(0);
            dxRootSignature->root_constant_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            dxRootSignature->root_constant_parameter.ShaderVisibility = D3D12Util_TranslateShaderStage(pushConstant->get_stages());
            dxRootSignature->root_constant_parameter.Constants.Num32BitValues = pushConstant->get_size() / sizeof(uint32_t);
            dxRootSignature->root_constant_parameter.Constants.ShaderRegister = pushConstant->get_binding();
            dxRootSignature->root_constant_parameter.Constants.RegisterSpace = pushConstant->get_set();
        }
        // Create static sampler parameter
        uint32_t staticSamplerCount = rootSigDesc.m_staticSamplerCount;
        D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs = nullptr;
        if(staticSamplerCount > 0)
        {
            staticSamplerDescs = (D3D12_STATIC_SAMPLER_DESC*)cyber_calloc(staticSamplerCount, sizeof(D3D12_STATIC_SAMPLER_DESC));
            for(uint32_t i = 0;i < dxRootSignature->m_staticSamplerCount; ++i)
            {
                auto& rst_slot = dxRootSignature->m_pStaticSamplers[i];
                for(uint32_t j = 0; j < rootSigDesc.m_staticSamplerCount; ++j)
                {
                    auto input_slot = (Sampler_D3D12_Impl*)rootSigDesc.m_staticSamplers[i];
                    if(strcmp((char*)rst_slot->get_name(), (char*)rootSigDesc.m_staticSamplerNames[j]) == 0)
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

                        IShaderResource* samplerResource = rst_slot;
                        staticSamplerDescs[i].ShaderRegister = samplerResource->get_binding();
                        staticSamplerDescs[i].RegisterSpace = samplerResource->get_set();
                        staticSamplerDescs[i].ShaderVisibility = D3D12Util_TranslateShaderStage(samplerResource->get_stages());
                    }
                }
            }
        }
        bool useInputLayout = shaderStages & SHADER_STAGE_VERT; // VertexStage uses input layout
        // Fill RS flags
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        if(useInputLayout)
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        }
        if(!(shaderStages & SHADER_STAGE_VERT))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & SHADER_STAGE_HULL))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & SHADER_STAGE_DOMAIN))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & SHADER_STAGE_GEOM))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        }
        if(!(shaderStages & SHADER_STAGE_FRAG))
        {
            rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
        }
        
        
        D3D12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = valid_root_tables + dxRootSignature->get_push_constant_count();
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
            hr = m_pDxDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&dxRootSignature->dxRootSignature));
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

    uint32_t descriptor_count_needed(IShaderResource* resource)
    {
        if(resource->get_dimension() == TEX_DIMENSION_1D_ARRAY ||
            resource->get_dimension() == TEX_DIMENSION_2D_ARRAY ||
            resource->get_dimension() == TEX_DIMENSION_2DMS_ARRAY ||
            resource->get_dimension() == TEX_DIMENSION_CUBE_ARRAY)
        {
            return resource->get_size();
        }
        else 
        {
            return 1;
        }
    }

    IDescriptorSet* RenderDevice_D3D12_Impl::create_descriptor_set(const DescriptorSetCreateDesc& dSetDesc)
    {
        RootSignature_D3D12_Impl* root_signature = static_cast<RootSignature_D3D12_Impl*>(dSetDesc.root_signature);
        DescriptorSet_D3D12_Impl* descSet = cyber_new<DescriptorSet_D3D12_Impl>();
        
        descSet->set_root_signature(dSetDesc.root_signature);
        descSet->set_set_index(dSetDesc.set_index);

        const uint32_t node_index = GRAPHICS_SINGLE_GPU_NODE_INDEX;
        auto& cbv_srv_uav_heap = m_cbvSrvUavHeaps[node_index];
        auto& sampler_heap = m_samplerHeaps[node_index];
        RenderObject::RootSignatureParameterTable* param_table = root_signature->get_parameter_table(dSetDesc.set_index);
        uint32_t cbv_srv_uav_count = 0;
        uint32_t sampler_count = 0;
        // collect descriptor counts
        if(root_signature->get_parameter_table_count() > 0)
        {
            for(uint32_t i = 0; i < param_table->m_resourceCount; ++i)
            {
                if(param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_SAMPLER)
                {
                    sampler_count++;
                }
                else if(param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_TEXTURE || 
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_RW_TEXTURE ||
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_BUFFER ||
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_RW_BUFFER ||
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_BUFFER_RAW ||
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_RW_BUFFER_RAW ||
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_TEXTURE_CUBE ||
                        param_table->m_ppResources[i]->get_type() == GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER 
                        )
                {
                    cbv_srv_uav_count += descriptor_count_needed(param_table->m_ppResources[i]);
                }
            }
        }

        // cbv/srv/uav heap
        descSet->cbv_srv_uav_handle = D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN;
        descSet->sampler_handle = D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN;
        if(cbv_srv_uav_count)
        {
            auto startHandle = DescriptorHeap_D3D12::consume_descriptor_handles(cbv_srv_uav_heap, cbv_srv_uav_count);
            descSet->cbv_srv_uav_handle = startHandle.mGpu.ptr - cbv_srv_uav_heap->get_start_handle().mGpu.ptr;
            descSet->cbv_srv_uav_stride = cbv_srv_uav_count * cbv_srv_uav_heap->get_descriptor_size();
        }
        if(sampler_count)
        {
            auto startHandle = DescriptorHeap_D3D12::consume_descriptor_handles(sampler_heap, sampler_count);
            descSet->sampler_handle = startHandle.mGpu.ptr - sampler_heap->get_start_handle().mGpu.ptr;
            descSet->sampler_stride = sampler_count * sampler_heap->get_descriptor_size();
        }
        // bind null handles on creation
        if(cbv_srv_uav_count || sampler_count)
        {
            uint32_t cbv_srv_uav_offset = 0;
            uint32_t sampler_offset = 0;
            for(uint32_t i = 0; i < param_table->m_resourceCount; ++i)
            {
                const auto dimension = param_table->m_ppResources[i]->get_dimension();
                auto src_handle = D3D12_DESCRIPTOR_ID_NONE;
                auto src_sampler_handle = D3D12_DESCRIPTOR_ID_NONE;
                switch (param_table->m_ppResources[i]->get_type())
                {
                    case GRAPHICS_RESOURCE_TYPE_TEXTURE: src_handle = m_pNullDescriptors->TextureSRV[dimension]; break;
                    case GRAPHICS_RESOURCE_TYPE_BUFFER: src_handle = m_pNullDescriptors->BufferSRV; break;
                    case GRAPHICS_RESOURCE_TYPE_RW_BUFFER: src_handle = m_pNullDescriptors->BufferUAV; break;
                    case GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER: src_handle = m_pNullDescriptors->BufferCBV; break;
                    case GRAPHICS_RESOURCE_TYPE_SAMPLER: src_sampler_handle = m_pNullDescriptors->Sampler; break;
                    default: break;
                }

                if(src_handle.ptr != D3D12_DESCRIPTOR_ID_NONE.ptr)
                {
                    for(uint32_t j = 0; j < param_table->m_ppResources[i]->get_size(); ++j)
                    {
                        cbv_srv_uav_heap->copy_descriptor_handle(src_handle, descSet->cbv_srv_uav_handle, cbv_srv_uav_offset);
                        cbv_srv_uav_offset++;
                    }
                }
                if(src_sampler_handle.ptr != D3D12_DESCRIPTOR_ID_NONE.ptr)
                {
                    for(uint32_t j = 0; j < param_table->m_ppResources[i]->get_size(); ++j)
                    {
                        sampler_heap->copy_descriptor_handle(src_sampler_handle, descSet->sampler_handle, sampler_offset);
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

    IRenderPipeline* RenderDevice_D3D12_Impl::create_render_pipeline(const RenderPipelineCreateDesc& pipelineDesc)
    {
        RootSignature_D3D12_Impl* DxRootSignature = static_cast<RootSignature_D3D12_Impl*>(pipelineDesc.root_signature);
        RenderPipeline_D3D12_Impl* pPipeline = cyber_new<RenderPipeline_D3D12_Impl>();
        // Input layout
        DECLARE_ZERO(D3D12_INPUT_ELEMENT_DESC, input_elements[GRAPHICS_MAX_VERTEX_ATTRIBUTES]);
        uint32_t input_element_count = 0;

        static const D3D12_INPUT_ELEMENT_DESC s_inputElementDesc[] =
        {
            { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        if(pipelineDesc.vertex_shader->m_library->get_entry_reflection(0)->get_vertex_input_count() > 0)
        {
            eastl::string_hash_map<uint32_t> semantic_index_map;
            for(uint32_t attrib_index = 0; attrib_index < pipelineDesc.vertex_shader->m_library->get_entry_reflection(0)->get_vertex_input_count(); ++attrib_index)
            {
                auto attribute = pipelineDesc.vertex_shader->m_library->get_entry_reflection(0)->get_vertex_input(attrib_index);
                input_elements[input_element_count].SemanticName = (char*)attribute->get_semantics_name();
                input_elements[input_element_count].SemanticIndex = attribute->get_semantics_index();
                input_elements[input_element_count].Format = DXGIUtil_TranslatePixelFormat(attribute->get_format());
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
            SHADER_STAGE stage_mask = (SHADER_STAGE)(1 << i);
            switch (stage_mask)
            {
                case SHADER_STAGE_VERT:
                {
                    if(pipelineDesc.vertex_shader)
                    {
                        ShaderLibrary_D3D12_Impl* vert_lib = (ShaderLibrary_D3D12_Impl*)pipelineDesc.vertex_shader->m_library;
                        vertex_shader.pShaderBytecode = vert_lib->m_pShaderBlob->GetBufferPointer();
                        vertex_shader.BytecodeLength = vert_lib->m_pShaderBlob->GetBufferSize();
                    }
                    break;
                }
                case SHADER_STAGE_FRAG:
                {
                    if(pipelineDesc.fragment_shader)
                    {
                        ShaderLibrary_D3D12_Impl* frag_lib = (ShaderLibrary_D3D12_Impl*)pipelineDesc.fragment_shader->m_library;
                        pixel_shader.pShaderBytecode = frag_lib->get_shader_blob()->GetBufferPointer();
                        pixel_shader.BytecodeLength = frag_lib->get_shader_blob()->GetBufferSize();
                    }
                    break;
                }
                case SHADER_STAGE_TESC:
                {
                    if(pipelineDesc.tesc_shader)
                    {
                        ShaderLibrary_D3D12_Impl* domain_lib = (ShaderLibrary_D3D12_Impl*)pipelineDesc.tesc_shader->m_library;
                        hull_shader.pShaderBytecode = domain_lib->get_shader_blob()->GetBufferPointer();
                        hull_shader.BytecodeLength = domain_lib->get_shader_blob()->GetBufferSize();
                    }
                    break;
                }
                case SHADER_STAGE_TESE:
                {
                    if(pipelineDesc.tese_shader)
                    {
                        ShaderLibrary_D3D12_Impl* hull_lib = (ShaderLibrary_D3D12_Impl*)pipelineDesc.tese_shader->m_library;
                        domain_shader.pShaderBytecode = hull_lib->get_shader_blob()->GetBufferPointer();
                        domain_shader.BytecodeLength = hull_lib->get_shader_blob()->GetBufferSize();
                    }
                    break;
                }
                case SHADER_STAGE_GEOM:
                {
                    if(pipelineDesc.geometry_shader)
                    {
                        ShaderLibrary_D3D12_Impl* geom_lib = (ShaderLibrary_D3D12_Impl*)pipelineDesc.geometry_shader->m_library;
                        geometry_shader.pShaderBytecode = geom_lib->get_shader_blob()->GetBufferPointer();
                        geometry_shader.BytecodeLength = geom_lib->get_shader_blob()->GetBufferSize();
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
        pso_desc.NodeMask = GRAPHICS_SINGLE_GPU_NODE_MASK;
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
        if(m_pPipelineLibrary)
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
            result = m_pPipelineLibrary->LoadGraphicsPipeline(pipelineName, &pso_desc, IID_PPV_ARGS(&pPipeline->pDxPipelineState));
        }
        // Not find in cache
        if(!SUCCEEDED(result))
        {
            CHECK_HRESULT(m_pDxDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pPipeline->pDxPipelineState)));
            // Pipeline cache
            if(m_pPipelineLibrary)
            {
                CHECK_HRESULT(m_pPipelineLibrary->StorePipeline(pipelineName, pPipeline->pDxPipelineState));
            }
        }
        D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        switch(pipelineDesc.prim_topology)
        {
            case PRIM_TOPO_POINT_LIST:
                topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case PRIM_TOPO_LINE_LIST:
                topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case PRIM_TOPO_LINE_STRIP:
                topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;
            case PRIM_TOPO_TRIANGLE_LIST:
                topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;
            case PRIM_TOPO_TRIANGLE_STRIP:
                topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;
            case PRIM_TOPO_PATCH_LIST:
            {
                cyber_assert(false, "PRIM_TOPO_PATCH_LIST not supported");
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
        RenderPipeline_D3D12_Impl* pPipeline = static_cast<RenderPipeline_D3D12_Impl*>(pipeline);
        SAFE_RELEASE(pPipeline->pDxPipelineState);
        cyber_free(pPipeline);
    }

    void RenderDevice_D3D12_Impl::update_descriptor_set(IDescriptorSet* set, const DescriptorData* updateData, uint32_t count)
    {
        DescriptorSet_D3D12_Impl* dxSet = static_cast<DescriptorSet_D3D12_Impl*>(set);
        const RootSignature_D3D12_Impl* dxRootSignature = static_cast<const RootSignature_D3D12_Impl*>(set->get_root_signature());
        RootSignatureParameterTable* paramTable = dxRootSignature->m_ppParameterTables[set->get_set_index()];
        const uint32_t nodeIndex = GRAPHICS_SINGLE_GPU_NODE_INDEX;
        DescriptorHeap_D3D12* pCbvSrvUavHeap = m_cbvSrvUavHeaps[nodeIndex];
        DescriptorHeap_D3D12* pSamplerHeap = m_samplerHeaps[nodeIndex];
        for(uint32_t i = 0;i < count; i++)
        {
            // Descriptor Info
            const DescriptorData* pParam = updateData + i;
            IShaderResource* resData = nullptr;
            uint32_t heapOffset = 0;
            if(pParam->name != nullptr)
            {
                size_t argNameHash = graphics_name_hash(pParam->name, strlen((char*)pParam->name));
                for(uint32_t j = 0;j < paramTable->m_resourceCount; ++j)
                {
                    if(paramTable->m_ppResources[j]->get_name_hash() == argNameHash)
                    {
                        resData = paramTable->m_ppResources[j];
                        break;
                    }
                    heapOffset += descriptor_count_needed(paramTable->m_ppResources[j]);
                }
            }
            else
            {
                for(uint32_t j = 0; j < paramTable->m_resourceCount; ++j)
                {
                    if(paramTable->m_ppResources[j]->get_type() == pParam->binding_type && 
                        paramTable->m_ppResources[j]->get_binding() == pParam->binding)
                    {
                        resData = paramTable->m_ppResources[j];
                        break;
                    }
                    heapOffset += descriptor_count_needed(paramTable->m_ppResources[j]);
                }
            }
            // Update info
            const uint32_t arrayCount = graphics_max(1u,pParam->count);
            switch(resData->get_type())
            {
                case GRAPHICS_RESOURCE_TYPE_SAMPLER:
                {
                    cyber_assert(pParam->samplers, "Binding Null Sampler");
                    RenderObject::Sampler_D3D12_Impl** Samplers = (RenderObject::Sampler_D3D12_Impl**)pParam->samplers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->samplers[arr], "Binding Null Sampler");

                        pSamplerHeap->copy_descriptor_handle({Samplers[arr]->mDxHandle.ptr}, dxSet->sampler_handle, arr + heapOffset);
                    }
                }
                break;
                case GRAPHICS_RESOURCE_TYPE_TEXTURE:
                case GRAPHICS_RESOURCE_TYPE_TEXTURE_CUBE:
                {
                    cyber_assert(pParam->textures, "Binding Null Texture");
                    RenderObject::TextureView_D3D12_Impl** Textures = (RenderObject::TextureView_D3D12_Impl**)pParam->textures;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->textures[arr], "Binding Null Texture");
                        pCbvSrvUavHeap->copy_descriptor_handle({Textures[arr]->m_dxDescriptorHandles.ptr + Textures[arr]->m_srvDescriptorOffset}, 
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case GRAPHICS_RESOURCE_TYPE_BUFFER:
                case GRAPHICS_RESOURCE_TYPE_BUFFER_RAW:
                {
                    cyber_assert(pParam->buffers, "Binding Null Buffer");
                    RenderObject::Buffer_D3D12_Impl** Buffers = (RenderObject::Buffer_D3D12_Impl**)pParam->buffers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->buffers[arr], "Binding Null Buffer");
                        pCbvSrvUavHeap->copy_descriptor_handle({Buffers[arr]->m_dxDescriptorHandles.ptr + Buffers[arr]->m_srvDescriptorOffset},
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER:
                {
                    cyber_assert(pParam->buffers, "Binding Null Buffer");
                    RenderObject::Buffer_D3D12_Impl** Buffers = (RenderObject::Buffer_D3D12_Impl**)pParam->buffers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->buffers[arr], "Binding Null Buffer");
                        pCbvSrvUavHeap->copy_descriptor_handle({Buffers[arr]->m_dxDescriptorHandles.ptr},
                        dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case GRAPHICS_RESOURCE_TYPE_RW_TEXTURE:
                {
                    cyber_assert(pParam->textures, "Binding Null Texture");
                    RenderObject::TextureView_D3D12_Impl** Textures = (RenderObject::TextureView_D3D12_Impl**)pParam->textures;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->textures[arr], "Binding Null Texture");
                        pCbvSrvUavHeap->copy_descriptor_handle({Textures[arr]->m_dxDescriptorHandles.ptr + Textures[arr]->m_uavDescriptorOffset}, dxSet->cbv_srv_uav_handle, arr + heapOffset);
                    }
                }
                break;
                case GRAPHICS_RESOURCE_TYPE_RW_BUFFER:
                case GRAPHICS_RESOURCE_TYPE_RW_BUFFER_RAW:
                {
                    cyber_assert(pParam->buffers, "Binding Null Buffer");
                    RenderObject::Buffer_D3D12_Impl** Buffers = (RenderObject::Buffer_D3D12_Impl**)pParam->buffers;
                    for(uint32_t arr = 0; arr < arrayCount; ++arr)
                    {
                        cyber_assert(pParam->buffers[arr], "Binding Null Buffer");
                        pCbvSrvUavHeap->copy_descriptor_handle({Buffers[arr]->m_dxDescriptorHandles.ptr + Buffers[arr]->m_uavDescriptorOffset},
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
        Adapter_D3D12_Impl* DxAdapter = static_cast<Adapter_D3D12_Impl*>(m_pAdapter);
        
        RenderObject::Buffer_D3D12_Impl* pBuffer = cyber_new<RenderObject::Buffer_D3D12_Impl>(this);

        uint64_t allocationSize = pDesc.m_size;
        // Align the buffer size to multiples of the dynamic uniform buffer minimum size
        if(pDesc.m_descriptors & DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            allocationSize = round_up_64(allocationSize, DxAdapter->m_adapterDetail.m_uniformBufferAlignment);
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

        if(pDesc.m_descriptors & DESCRIPTOR_TYPE_RW_BUFFER)
        {
            // UAV
            desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // Adjust for padding
        uint64_t padded_size = 0;
        m_pDxDevice->GetCopyableFootprints(&desc, 0, 1, 0, NULL, NULL, NULL, &padded_size);
        allocationSize = (uint64_t)padded_size;
        // Buffer is 1D
        desc.Width = padded_size;

        GRAPHICS_RESOURCE_STATE start_state = pDesc.m_startState;
        if(pDesc.m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_TO_GPU || pDesc.m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_ONLY)
        {
            // Your application should generally avoid transitioning to D3D12_RESOURCE_STATE_GENERIC_READ when possible, 
            // since that can result in premature cache flushes, or resource layout changes (for example, compress/decompress),
            // causing unnecessary pipeline stalls.
            start_state = GRAPHICS_RESOURCE_STATE_GENERIC_READ;
        }
        else if(pDesc.m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_TO_CPU)
        {
            start_state = GRAPHICS_RESOURCE_STATE_COPY_DEST;
        }

        D3D12_RESOURCE_STATES res_states = D3D12Util_TranslateResourceState(start_state);

        D3D12MA::ALLOCATION_DESC alloc_desc = {};

        if(pDesc.m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_ONLY || pDesc.m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_TO_GPU)
        {
            alloc_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        }
        else if(pDesc.m_memoryUsage == GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_TO_CPU)
        {
            alloc_desc.HeapType = D3D12_HEAP_TYPE_READBACK;
            desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
        else
        {
            alloc_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        }
        
        // for commit resource
        if(pDesc.m_flags & BCF_OWN_MEMORY_BIT)
            alloc_desc.Flags = (D3D12MA::ALLOCATION_FLAGS)(alloc_desc.Flags | D3D12MA::ALLOCATION_FLAG_COMMITTED);

        if(alloc_desc.HeapType != D3D12_HEAP_TYPE_DEFAULT && (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            heapProps.VisibleNodeMask = GRAPHICS_SINGLE_GPU_NODE_MASK;
            heapProps.CreationNodeMask = GRAPHICS_SINGLE_GPU_NODE_MASK;
            CHECK_HRESULT(m_pDxDevice->CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, &desc, res_states, NULL, IID_ARGS(&pBuffer->m_pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Committed Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", (char*)(pDesc.m_pName ? pDesc.m_pName : CYBER_UTF8("")), allocationSize, pDesc.m_format);
        }
        else
        {
            CHECK_HRESULT(m_pResourceAllocator->CreateResource(&alloc_desc, &desc, res_states, NULL, &pBuffer->m_pDxAllocation, IID_ARGS(&pBuffer->m_pDxResource)));
            CB_CORE_TRACE("[D3D12] Create Buffer Resource Succeed! \n\t With Name: [0]\n\t Size: [1] \n\t Format: [2]", (char*)(pDesc.m_pName ? pDesc.m_pName : CYBER_UTF8("")), allocationSize, pDesc.m_format);
        }
        
        if(pDesc.m_memoryUsage != GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_ONLY && pDesc.m_flags & BCF_PERSISTENT_MAP_BIT)
            pBuffer->m_pDxResource->Map(0, NULL, &pBuffer->m_pCpuMappedAddress);
        
        pBuffer->m_dxGpuAddress = pBuffer->m_pDxResource->GetGPUVirtualAddress();
        
        // Create Descriptors
        if(!(pDesc.m_flags & BCF_NO_DESCRIPTOR_VIEW_CREATION))
        {
            DescriptorHeap_D3D12* pHeap = m_cpuDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
            uint32_t handleCount = ((pDesc.m_descriptors & DESCRIPTOR_TYPE_UNIFORM_BUFFER) ? 1 : 0) + 
                                    ((pDesc.m_descriptors & DESCRIPTOR_TYPE_BUFFER) ? 1 : 0) +
                                    ((pDesc.m_descriptors & DESCRIPTOR_TYPE_RW_BUFFER) ? 1 : 0);
            pBuffer->m_dxDescriptorHandles = DescriptorHeap_D3D12::consume_descriptor_handles(pHeap, handleCount).mCpu;
        
            // Create CBV
            if(pDesc.m_descriptors & DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                pBuffer->m_srvDescriptorOffset = 1;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.BufferLocation = pBuffer->m_dxGpuAddress;
                cbvDesc.SizeInBytes = (UINT)allocationSize;
                create_constant_buffer_view( &cbvDesc, pBuffer->m_dxDescriptorHandles);
            }

            // Create SRV
            if(pDesc.m_descriptors & DESCRIPTOR_TYPE_BUFFER)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE srv = {pBuffer->m_dxDescriptorHandles.ptr + pBuffer->m_srvDescriptorOffset};
                pBuffer->m_uavDescriptorOffset = pBuffer->m_srvDescriptorOffset + pHeap->get_descriptor_size() * 1;

                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Buffer.FirstElement = pDesc.m_firstElement;
                srvDesc.Buffer.NumElements = (UINT)pDesc.m_elementCount;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
                srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(pDesc.m_format);
                if(DESCRIPTOR_TYPE_BUFFER_RAW == (pDesc.m_descriptors & DESCRIPTOR_TYPE_BUFFER_RAW))
                {
                    if(pDesc.m_format != TEXTURE_FORMAT_UNDEFINED)
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
                create_shader_resource_view(pBuffer->m_pDxResource, &srvDesc, srv);
            }

            // Create UAV
            if(pDesc.m_descriptors & DESCRIPTOR_TYPE_RW_BUFFER)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE uav = {pBuffer->m_dxDescriptorHandles.ptr + pBuffer->m_uavDescriptorOffset};

                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.FirstElement = pDesc.m_firstElement;
                uavDesc.Buffer.NumElements = (UINT)pDesc.m_elementCount;
                uavDesc.Buffer.StructureByteStride = (UINT)pDesc.m_structStride;
                uavDesc.Buffer.CounterOffsetInBytes = 0;
                uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
                if(DESCRIPTOR_TYPE_RW_BUFFER_RAW == (pDesc.m_descriptors & DESCRIPTOR_TYPE_RW_BUFFER_RAW))
                {
                    if(pDesc.m_format != TEXTURE_FORMAT_UNDEFINED)
                        CB_CORE_WARN("Raw buffer use R32 typeless format. Format will be ignored");
                    uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                    uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                }
                else if(pDesc.m_format != TEXTURE_FORMAT_UNDEFINED)
                {
                    uavDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(pDesc.m_format);
                    D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = {uavDesc.Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE};
                    HRESULT hr = m_pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
                    if(!SUCCEEDED(hr) || !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) || 
                        !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE))
                    {
                        CB_CORE_WARN("Cannot use Typed UAV for buffer format [0]", (uint32_t)pDesc.m_format);
                        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                    }
                }
                // Cannot create a typed RWStructuredBuffer
                if(uavDesc.Format != DXGI_FORMAT_UNKNOWN)
                {
                    uavDesc.Buffer.StructureByteStride = 0;
                }

                ID3D12Resource* pCounterResource = pDesc.m_pCounterBuffer ? static_cast<RenderObject::Buffer_D3D12_Impl*>(pDesc.m_pCounterBuffer)->m_pDxResource : nullptr;
                create_unordered_access_view(pBuffer->m_pDxResource, pCounterResource, &uavDesc, uav);
            }
        }

        pBuffer->m_size = (uint32_t)pDesc.m_size;
        pBuffer->m_memoryUsage = pDesc.m_memoryUsage;
        pBuffer->m_descriptors = pDesc.m_descriptors;
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
    IShaderLibrary* RenderDevice_D3D12_Impl::create_shader_library(const struct ShaderLibraryCreateDesc& desc)
    {
        ShaderLibrary_D3D12_Impl* pLibrary = cyber_new<ShaderLibrary_D3D12_Impl>();

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

            auto procDxcCreateInstance = d3d12_util_get_dxc_create_instance_proc();
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

            pCompiler3->Compile(&pSource, DxilArgs.data(), DxilArgs.size(), pIncludeHandler, IID_PPV_ARGS(&pLibrary->m_pShaderResult));

            IDxcBlobUtf8* pErrors = nullptr;
            pLibrary->m_pShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
            // Note that d3dcompiler would return null if no errors or warnings are present.
            // IDxcCompiler3::Compile will always return an error buffer, but its length
            // will be zero if there are no warnings or errors.
            if (pErrors != nullptr && pErrors->GetStringLength() != 0)
                CB_WARN("Warnings and Errors:{0}", pErrors->GetStringPointer());
            
            pLibrary->m_pShaderResult->GetStatus(&hr);
            if(FAILED(hr))
            {
                return nullptr;
            }

            pLibrary->m_pShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pLibrary->m_pShaderBlob), nullptr);

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
            hr = D3DCompile((LPCVOID)desc.code, desc.code_size, nullptr, Macros, nullptr, entry_point.c_str(), profile.c_str(), dwShaderFlags, 0, &pLibrary->m_pShaderBlob, &ppErrorMsgs);

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
        pLibrary->Initialize_shader_reflection(desc);

        return pLibrary;
    }

    void RenderDevice_D3D12_Impl::free_shader_library(IShaderLibrary* shaderLibrary)
    {
        ShaderLibrary_D3D12_Impl* dx_shader_library = static_cast<ShaderLibrary_D3D12_Impl*>(shaderLibrary);
        dx_shader_library->free_reflection();
        if(dx_shader_library->m_pShaderBlob != nullptr)
        {
            dx_shader_library->m_pShaderBlob->Release();
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
        desc.Flags = D3D12MA::ALLOCATOR_FLAGS(desc.Flags | D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED);
        if(!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &m_pResourceAllocator)))
        {
            cyber_assert(false, "DMA Allocator Create Failed!");
        }
    }

    HRESULT RenderDevice_D3D12_Impl::hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize)
    {
        return m_pDxDevice->CheckFeatureSupport(pFeature, pFeatureSupportData, pFeatureSupportDataSize);
    }

    HRESULT RenderDevice_D3D12_Impl::hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource)
    {
        return m_pDxDevice->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

    void RenderDevice_D3D12_Impl::create_constant_buffer_view(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateConstantBufferView(desc, destHandle);
    }

    void RenderDevice_D3D12_Impl::create_shader_resource_view(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateShaderResourceView(resource, desc, destHandle);
    }

    void RenderDevice_D3D12_Impl::create_depth_stencil_view(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateDepthStencilView(resource, desc, destHandle);
    }

    void RenderDevice_D3D12_Impl::create_unordered_access_view(ID3D12Resource* resource, ID3D12Resource* counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateUnorderedAccessView(resource, counterResource, desc, destHandle);
    }

    void RenderDevice_D3D12_Impl::create_render_target_view(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle)
    {
        if(destHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            destHandle = GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->consume_descriptor_handles(1).mCpu;
        GetD3D12Device()->CreateRenderTargetView(resource, desc, destHandle);
    }

    }
}