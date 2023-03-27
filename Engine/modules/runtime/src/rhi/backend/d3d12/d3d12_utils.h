#pragma once
#include "rhi/rhi.h"
#include "core/Core.h"
#include "rhi/backend/d3d12/rhi_d3d12.h"
#include "CyberMemory/Memory.h"

namespace Cyber
{
    struct DescriptorHeapProperties
    {
        uint32_t mMaxDescriptors;
        D3D12_DESCRIPTOR_HEAP_FLAGS mFlags;
    };

    static const DescriptorHeapProperties gCpuDescriptorHeapProerties[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {
        {1024 * 256, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},  //CBV SRV UAV
        {2048, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},        // Sampler
        {1024*64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},     // Rtv
        {1024*64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE}      // Dsv
    };

    static DXGI_FORMAT DXGIUtil_TranslatePixelFormat(const ERHIFormat fmt, bool ShaderResource = false)
    {
        switch(fmt)
        {
            case RHI_FORMAT_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
            case RHI_FORMAT_R5G6B5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
            case RHI_FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
            case RHI_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
            case RHI_FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
            case RHI_FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
            case RHI_FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
            case RHI_FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
            case RHI_FORMAT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
            case RHI_FORMAT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
            case RHI_FORMAT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
            case RHI_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
            case RHI_FORMAT_B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM; 

            case RHI_FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case RHI_FORMAT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
            case RHI_FORMAT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
            case RHI_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
            case RHI_FORMAT_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            case RHI_FORMAT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case RHI_FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
            case RHI_FORMAT_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

            case RHI_FORMAT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
	        case RHI_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;

            case RHI_FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
            case RHI_FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
            case RHI_FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
            case RHI_FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
            case RHI_FORMAT_R16_SFLOAT: return DXGI_FORMAT_R16_FLOAT;
            case RHI_FORMAT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
            case RHI_FORMAT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
            case RHI_FORMAT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
            case RHI_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
            case RHI_FORMAT_R16G16_SFLOAT: return DXGI_FORMAT_R16G16_FLOAT;
            case RHI_FORMAT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case RHI_FORMAT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
            case RHI_FORMAT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
            case RHI_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
            case RHI_FORMAT_R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case RHI_FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
            case RHI_FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
            case RHI_FORMAT_R32_SFLOAT: return DXGI_FORMAT_R32_FLOAT;
            case RHI_FORMAT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
            case RHI_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
            case RHI_FORMAT_R32G32_SFLOAT: return DXGI_FORMAT_R32G32_FLOAT;
            case RHI_FORMAT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
            case RHI_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
            case RHI_FORMAT_R32G32B32_SFLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
            case RHI_FORMAT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
            case RHI_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
            case RHI_FORMAT_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case RHI_FORMAT_B10G11R11_UFLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
            case RHI_FORMAT_E5B9G9R9_UFLOAT: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case RHI_FORMAT_D16_UNORM: return ShaderResource ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_D16_UNORM;
            case RHI_FORMAT_X8_D24_UNORM: return ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
            case RHI_FORMAT_D32_SFLOAT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT :DXGI_FORMAT_D32_FLOAT;
            case RHI_FORMAT_D24_UNORM_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS :DXGI_FORMAT_D24_UNORM_S8_UINT;
            case RHI_FORMAT_D32_SFLOAT_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS :DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case RHI_FORMAT_DXBC1_RGB_UNORM: return DXGI_FORMAT_BC1_UNORM;
            case RHI_FORMAT_DXBC1_RGB_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
            case RHI_FORMAT_DXBC1_RGBA_UNORM: return DXGI_FORMAT_BC1_UNORM;
            case RHI_FORMAT_DXBC1_RGBA_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
            case RHI_FORMAT_DXBC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
            case RHI_FORMAT_DXBC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
            case RHI_FORMAT_DXBC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
            case RHI_FORMAT_DXBC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
            case RHI_FORMAT_DXBC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
            case RHI_FORMAT_DXBC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
            case RHI_FORMAT_DXBC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
            case RHI_FORMAT_DXBC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
            case RHI_FORMAT_DXBC6H_UFLOAT: return DXGI_FORMAT_BC6H_UF16;
            case RHI_FORMAT_DXBC6H_SFLOAT: return DXGI_FORMAT_BC6H_SF16;
            case RHI_FORMAT_DXBC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
            case RHI_FORMAT_DXBC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

            case RHI_FORMAT_D16_UNORM_S8_UINT:
            case RHI_FORMAT_R4G4_UNORM: 
            default: return DXGI_FORMAT_UNKNOWN;
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    static DXGI_FORMAT DXGIUtil_FormatToTypeless(DXGI_FORMAT fmt)
    {
        switch (fmt) {
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_UINT:
            case DXGI_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case DXGI_FORMAT_R32G32B32_FLOAT:
            case DXGI_FORMAT_R32G32B32_UINT:
            case DXGI_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_TYPELESS;

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R16G16B16A16_UNORM:
            case DXGI_FORMAT_R16G16B16A16_UINT:
            case DXGI_FORMAT_R16G16B16A16_SNORM:
            case DXGI_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;

            case DXGI_FORMAT_R32G32_FLOAT:
            case DXGI_FORMAT_R32G32_UINT:
            case DXGI_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_TYPELESS;

            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_TYPELESS;

            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case DXGI_FORMAT_R16G16_FLOAT:
            case DXGI_FORMAT_R16G16_UNORM:
            case DXGI_FORMAT_R16G16_UINT:
            case DXGI_FORMAT_R16G16_SNORM:
            case DXGI_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_TYPELESS;

            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R32_FLOAT:
            case DXGI_FORMAT_R32_UINT:
            case DXGI_FORMAT_R32_SINT: return DXGI_FORMAT_R32_TYPELESS;
            case DXGI_FORMAT_R8G8_UNORM:
            case DXGI_FORMAT_R8G8_UINT:
            case DXGI_FORMAT_R8G8_SNORM:
            case DXGI_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_TYPELESS;
            case DXGI_FORMAT_B4G4R4A4_UNORM: // just treats a 16 raw bits
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_R16_FLOAT:
            case DXGI_FORMAT_R16_UNORM:
            case DXGI_FORMAT_R16_UINT:
            case DXGI_FORMAT_R16_SNORM:
            case DXGI_FORMAT_R16_SINT: return DXGI_FORMAT_R16_TYPELESS;
            case DXGI_FORMAT_A8_UNORM:
            case DXGI_FORMAT_R8_UNORM:
            case DXGI_FORMAT_R8_UINT:
            case DXGI_FORMAT_R8_SNORM:
            case DXGI_FORMAT_R8_SINT: return DXGI_FORMAT_R8_TYPELESS;
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_TYPELESS;
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_TYPELESS;
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_TYPELESS;
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_TYPELESS;
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_TYPELESS;
            case DXGI_FORMAT_B5G6R5_UNORM:
            case DXGI_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_R16_TYPELESS;

            case DXGI_FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;

            case DXGI_FORMAT_B8G8R8X8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_TYPELESS;

            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;

            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_TYPELESS;

            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_TYPELESS;

            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
            case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;

                // typeless just return the input format
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            case DXGI_FORMAT_R32G32B32_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            case DXGI_FORMAT_R32G32_TYPELESS:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R16G16_TYPELESS:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R16_TYPELESS:
            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC7_TYPELESS: return fmt;

            case DXGI_FORMAT_R1_UNORM:
            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
            case DXGI_FORMAT_AYUV:
            case DXGI_FORMAT_Y410:
            case DXGI_FORMAT_Y416:
            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
            case DXGI_FORMAT_420_OPAQUE:
            case DXGI_FORMAT_YUY2:
            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
            case DXGI_FORMAT_NV11:
            case DXGI_FORMAT_AI44:
            case DXGI_FORMAT_IA44:
            case DXGI_FORMAT_P8:
            case DXGI_FORMAT_A8P8:
            case DXGI_FORMAT_P208:
            case DXGI_FORMAT_V208:
            case DXGI_FORMAT_V408:
            case DXGI_FORMAT_UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            }
            return DXGI_FORMAT_UNKNOWN;
    }

    static D3D12_RESOURCE_STATES D3D12Util_TranslateResourceState(RHIResourceState state)
    {
        D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

        // These states cannot be combined with other states so we just do an == check
        if (state == RHI_RESOURCE_STATE_GENERIC_READ)
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        if (state == RHI_RESOURCE_STATE_COMMON)
            return D3D12_RESOURCE_STATE_COMMON;
        if (state == RHI_RESOURCE_STATE_PRESENT)
            return D3D12_RESOURCE_STATE_PRESENT;

        if (state & RHI_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (state & RHI_RESOURCE_STATE_INDEX_BUFFER)
            ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (state & RHI_RESOURCE_STATE_RENDER_TARGET)
            ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (state & RHI_RESOURCE_STATE_UNORDERED_ACCESS)
            ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (state & RHI_RESOURCE_STATE_DEPTH_WRITE)
            ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (state & RHI_RESOURCE_STATE_DEPTH_READ)
            ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (state & RHI_RESOURCE_STATE_STREAM_OUT)
            ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
        if (state & RHI_RESOURCE_STATE_INDIRECT_ARGUMENT)
            ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (state & RHI_RESOURCE_STATE_COPY_DEST)
            ret |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (state & RHI_RESOURCE_STATE_COPY_SOURCE)
            ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (state & RHI_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
            ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (state & RHI_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
            ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    #ifdef D3D12_RAYTRACING_AVAILABLE
        if (state & RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
            ret |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    #endif

    #ifdef ENABLE_VRS
        if (state & RESOURCE_STATE_SHADING_RATE_SOURCE)
            ret |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
    #endif

        return ret;
    }

    #if !defined (XBOX) && defined (_WIN32)
    #include <dxcapi.h>

    void D3D12Util_LoadDxcDLL();
    void D3D12Util_UnloadDxcDLL();
    DxcCreateInstanceProc D3D12Util_GetDxcCreateInstanceProc();
    #endif

    static DescriptorHandle D3D12Util_ConsumeDescriptorHandles(RHIDescriptorHeap_D3D12* pHeap, uint32_t descriptorCount)
    {
        if(pHeap->mUsedDescriptors + descriptorCount > pHeap->mDesc.NumDescriptors)
        {
        #ifdef CYBER_THREAD_SAFETY
        #endif
            if((pHeap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE))
            {
                uint32_t currentOffset = pHeap->mUsedDescriptors;
                D3D12_DESCRIPTOR_HEAP_DESC desc = pHeap->mDesc;
                while(pHeap->mUsedDescriptors + descriptorCount > desc.NumDescriptors)
                {
                    desc.NumDescriptors <<= 1;
                }
                ID3D12Device* pDevice = pHeap->pDevice;
                SAFE_RELEASE(pHeap->pCurrentHeap);
                pDevice->CreateDescriptorHeap(&desc, IID_ARGS(&pHeap->pCurrentHeap));
                pHeap->mDesc = desc;
                pHeap->mStartHandle.mCpu = pHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
                pHeap->mStartHandle.mGpu = pHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();

                uint32_t* rangeSized = (uint32_t*)cb_malloc(pHeap->mUsedDescriptors * sizeof(uint32_t));
            #ifdef CYBER_THREAD_SAFETY
            #else
                uint32_t usedDescriptors = pHeap->mUsedDescriptors;
            #endif

                for(uint32_t i = 0;i < pHeap->mUsedDescriptors; ++i)
                    rangeSized[i] = 1;
                //copy new heap to pHeap
                //TODO: copy shader-visible heap may slow
                pDevice->CopyDescriptors(
                    1, &pHeap->mStartHandle.mCpu, &usedDescriptors, pHeap->mUsedDescriptors, pHeap->pHandles, rangeSized, pHeap->mDesc.Type);
                D3D12_CPU_DESCRIPTOR_HANDLE* pNewHandles = 
                    (D3D12_CPU_DESCRIPTOR_HANDLE*)cb_calloc(pHeap->mDesc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                memcpy(pNewHandles, pHeap->pHandles, pHeap->mUsedDescriptors * sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                cb_free(pHeap->pHandles);
                pHeap->pHandles = pNewHandles;
            }
            else if(pHeap->mFreeList.size() >= descriptorCount)
            {
                if(descriptorCount == 1)
                {
                    DescriptorHandle ret = pHeap->mFreeList.back();
                    pHeap->mFreeList.pop_back();
                    return ret;
                }

                // search for continuous free items in the list
                uint32_t freeCount = 1;
                for(size_t i = pHeap->mFreeList.size() - 1; i > 0; --i)
                {
                    size_t index = i - 1;
                    DescriptorHandle mDescHandle = pHeap->mFreeList[index];
                    if(mDescHandle.mCpu.ptr + pHeap->mDescriptorSize == pHeap->mFreeList[i].mCpu.ptr)
                    {
                        ++freeCount;
                    }
                    else 
                    {
                        freeCount = 1;
                    }

                    if(freeCount == descriptorCount)
                    {
                        pHeap->mFreeList.erase(pHeap->mFreeList.begin() + index, pHeap->mFreeList.begin() + index + descriptorCount);
                        return mDescHandle;
                    }
                }
            }
        }
        #ifdef CYBER_THREAD_SAFETY
        #else
            uint32_t usedDescriptors = pHeap->mUsedDescriptors = pHeap->mUsedDescriptors + descriptorCount;
        #endif
        cyber_check(usedDescriptors + descriptorCount <= pHeap->mDesc.NumDescriptors);
        DescriptorHandle ret = {
            {pHeap->mStartHandle.mCpu.ptr + usedDescriptors * pHeap->mDescriptorSize},
            {pHeap->mStartHandle.mGpu.ptr + usedDescriptors * pHeap->mDescriptorSize},
        };
        return ret;
    }


    void D3D12Util_InitializeEnvironment(RHIInstance* pInst);

    void D3D12Util_DeInitializeEnvironment(RHIInstance* pInst);

    void D3D12Util_Optionalenable_debug_layer(RHIInstance_D3D12* result, const RHIInstanceCreateDesc& instanceDesc);

    void D3D12Util_QueryAllAdapters(RHIInstance_D3D12* instance, uint32_t& count, bool& foundSoftwareAdapter);

    void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& pDesc, struct RHIDescriptorHeap_D3D12** ppDescHeap);

    void D3D12Util_CreateDMAAllocator(RHIInstance_D3D12* pInstance, RHIAdapter_D3D12* pAdapter, RHIDevice_D3D12* pDevice);

    void D3D12Util_InitializeShaderReflection(ID3D12Device* device, RHIShaderLibrary_D3D12* library, const RHIShaderLibraryCreateDesc& desc);
    
    static void D3D12Util_CreateCBV(RHIDevice_D3D12* pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pCbvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
        pDevice->pDxDevice->CreateConstantBufferView(pCbvDesc, *pHandle);
    }

    static void D3D12Util_CreateSRV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
        pDevice->pDxDevice->CreateShaderResourceView(pResource, pSrvDesc, *pHandle);
    }

    static void D3D12Util_CreateUAV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
        pDevice->pDxDevice->CreateUnorderedAccessView(pResource, pCounterResource, pUavDesc, *pHandle);
    }

    static void D3D12Util_CreateRTV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], 1).mCpu;
        pDevice->pDxDevice->CreateRenderTargetView(pResource, pRtvDesc, *pHandle);
    }

    static void D3D12Util_CreateDSV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], 1).mCpu;
        pDevice->pDxDevice->CreateDepthStencilView(pResource, pDsvDesc, *pHandle);
    }

}