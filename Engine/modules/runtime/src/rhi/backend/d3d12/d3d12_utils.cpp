#include "d3d12_utils.h"
//#include "platform/configure.h"
//#include <winnt.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include "platform/memory.h"

//#include "cyber_runtime.config.h"
//#include "../../common/common_utils.h"

namespace Cyber
{
    #if !defined (XBOX) && defined (_WIN32)
    struct RHIUtil_DXCLoader
    {
        static void Load()
        {
            dxcLibrary = LoadLibrary(L"dxcompiler.dll");
            pDxcCreateInstance = (void*)::GetProcAddress((HMODULE)dxcLibrary, "DxcCreateInstance");
        }
        static void Unload()
        {
            pDxcCreateInstance = nullptr;
            ::FreeLibrary(dxcLibrary);
        }

        static DxcCreateInstanceProc Get()
        {
            return (DxcCreateInstanceProc)pDxcCreateInstance;
        }
        static HMODULE dxcLibrary;
        static void* pDxcCreateInstance;
    };

    void* RHIUtil_DXCLoader::pDxcCreateInstance = nullptr;
    HMODULE RHIUtil_DXCLoader::dxcLibrary = nullptr;

    void D3D12Util_LoadDxcDLL()
    {
        RHIUtil_DXCLoader::Load();
    }
    void D3D12Util_UnloadDxcDLL()
    {
        RHIUtil_DXCLoader::Unload();
    }
    DxcCreateInstanceProc D3D12Util_GetDxcCreateInstanceProc()
    {
        if(RHIUtil_DXCLoader::pDxcCreateInstance == nullptr)
        {
            D3D12Util_LoadDxcDLL();
        }
        return RHIUtil_DXCLoader::Get();
    }
    #endif

    DescriptorHandle D3D12Util_ConsumeDescriptorHandles(RHIDescriptorHeap_D3D12* pHeap, uint32_t descriptorCount)
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

                uint32_t* rangeSized = (uint32_t*)cyber_malloc(pHeap->mUsedDescriptors * sizeof(uint32_t));
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
                    (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(pHeap->mDesc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                memcpy(pNewHandles, pHeap->pHandles, pHeap->mUsedDescriptors * sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                cyber_free(pHeap->pHandles);
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
    void D3D12Util_ReturnDescriptorHandles(RHIDescriptorHeap_D3D12* heap, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t count)
    {
        cyber_assert((heap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) == 0, "Descriptor Handle flag should not be shader visible!");

        for(uint32_t i = 0; i < count; ++i)
        {
            DECLARE_ZERO(DescriptorHandle, free);
            free.mCpu = {handle.ptr + i * heap->mDescriptorSize};
            free.mGpu = { D3D12_GPU_VIRTUAL_ADDRESS_NULL};
            heap->mFreeList.push_back(free);
        }
    }
    void D3D12Util_CopyDescriptorHandle(RHIDescriptorHeap_D3D12* dstHeap, const D3D12_CPU_DESCRIPTOR_HANDLE& srcHandle, const uint64_t& dstHandle, uint32_t index)
    {
        // fill dest heap
        dstHeap->pHandles[(dstHandle / dstHeap->mDescriptorSize) + index] = srcHandle;
        // copy
        dstHeap->pDevice->CopyDescriptorsSimple(1, {dstHeap->mStartHandle.mCpu.ptr + dstHandle + (index * dstHeap->mDescriptorSize)}, srcHandle, dstHeap->mType);
    }

    void D3D12Util_InitializeEnvironment(RHIInstance* pInst)
    {
        
    }

    void D3D12Util_DeInitializeEnvironment(RHIInstance* pInst)
    {
        
    }

    void D3D12Util_Optionalenable_debug_layer(RHIInstance_D3D12* result, const RHIInstanceCreateDesc& instanceDesc)
    {
        if(instanceDesc.enable_debug_layer)
        {
            if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&result->pDXDebug))))
            {
                result->pDXDebug->EnableDebugLayer();
                if(instanceDesc.enable_gpu_based_validation)
                {
                    ID3D12Debug1* pDebug1 = nullptr;
                    if(SUCCEEDED(result->pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
                    {
                        pDebug1->SetEnableGPUBasedValidation(instanceDesc.enable_gpu_based_validation);
                        pDebug1->Release();
                    }
                }
            }
        }
        else if(instanceDesc.enable_gpu_based_validation)
        {
            cyber_warn(false, "D3D12 GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
        }
    }

    void D3D12Util_QueryAllAdapters(RHIInstance_D3D12* instance, uint32_t& pCount, bool& pFoundSoftwareAdapter)
    {
        cyber_assert(instance->pAdapters == nullptr, "getProperGpuCount should be called only once!");
        cyber_assert(instance->mAdaptersCount == 0, "getProperGpuCount should be called only once!");

        IDXGIAdapter* _adapter = nullptr;
        eastl::vector<IDXGIAdapter4*> dxgi_adapters;
        eastl::vector<D3D_FEATURE_LEVEL> adapter_levels;

        for(UINT i = 0; i < 10;i++)
        {
            if(!SUCCEEDED(instance->pDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter))))
            {
                break;
            }
            DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc)
            IDXGIAdapter4* adapter = nullptr;
            _adapter->QueryInterface(IID_PPV_ARGS(&adapter));
            adapter->GetDesc3(&desc);
            // Ignore Microsoft Driver
            if(!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
            {
                uint32_t level_c = RHI_ARRAY_LEN(d3d_feature_levels);

                for(uint32_t level = 0;level < level_c;++level)
                {
                    ID3D12Device* pDevice = nullptr;
                    if(SUCCEEDED(D3D12CreateDevice(adapter, d3d_feature_levels[level], IID_PPV_ARGS(&pDevice))))
                    {
                        struct IDXGIAdapter4* tempAdapter;
                        HRESULT hres = adapter->QueryInterface(IID_PPV_ARGS(&tempAdapter));
                        if(SUCCEEDED(hres))
                        {
                            SAFE_RELEASE(tempAdapter);
                            instance->mAdaptersCount++;
                            // Add ref
                            {
                                dxgi_adapters.push_back(adapter);
                                adapter_levels.push_back(d3d_feature_levels[level]);
                            }
                            break;
                        }
                    }
                }
            }
            else 
            {
                pFoundSoftwareAdapter = true;
                SAFE_RELEASE(adapter);
            }
        }

        pCount = instance->mAdaptersCount;
        instance->pAdapters = cyber_new_n<RHIAdapter_D3D12>(instance->mAdaptersCount);
        CB_CORE_INFO("test");

        for(uint32_t i = 0;i < pCount; i++)
        {
            auto& adapter = instance->pAdapters[i];
            // Device Objects
            adapter.pDxActiveGPU = dxgi_adapters[i];
            adapter.mFeatureLevel = adapter_levels[i];
            adapter.pInstance = instance;
        }
    }

    void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& pDesc, struct RHIDescriptorHeap_D3D12** ppDescHeap)
    {
        uint32_t numDesciptors = pDesc.NumDescriptors;
        RHIDescriptorHeap_D3D12* pHeap = (RHIDescriptorHeap_D3D12*)cyber_calloc(1, sizeof(*pHeap));
        // TODO thread safety

        pHeap->pDevice = pDevice;

        // Keep 32 aligned for easy remove
        //numDesciptors = cyber_round_up(numDesciptors, 32);

        D3D12_DESCRIPTOR_HEAP_DESC Desc = pDesc;
        Desc.NumDescriptors = numDesciptors;
        pHeap->mDesc = Desc;

        if(!SUCCEEDED(pDevice->CreateDescriptorHeap(&Desc, IID_ARGS(&pHeap->pCurrentHeap))))
        {
            cyber_assert(false, "DescriptorHeap Create Failed!");
        }
        
        pHeap->mStartHandle.mCpu = pHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
        if(pHeap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            pHeap->mStartHandle.mGpu = pHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
        }
        pHeap->mDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(pHeap->mDesc.Type);
        if(Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            pHeap->pHandles = (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
        }

        *ppDescHeap = pHeap;
    }

    void D3D12Util_FreeDescriptorHeap(struct RHIDescriptorHeap_D3D12* heap)
    {
        if(heap == nullptr) return;
        SAFE_RELEASE(heap->pCurrentHeap);

        heap->mFreeList.~vector();

        cyber_free(heap->pHandles);
        cyber_free(heap);
    }

    void D3D12Util_CreateDMAAllocator(RHIInstance_D3D12* pInstance, RHIAdapter_D3D12* pAdapter, RHIDevice_D3D12* pDevice)
    {
        D3D12MA::ALLOCATOR_DESC desc = {};
        desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
        desc.pDevice = pDevice->pDxDevice;
        desc.pAdapter = pAdapter->pDxActiveGPU;

        D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
        allocationCallbacks.pAllocate = [](size_t size, size_t alignment, void*){
            return cyber_memalign(size, alignment);
        };
        allocationCallbacks.pFree = [](void* ptr, void*){
            cyber_free(ptr);
        };
        desc.pAllocationCallbacks = &allocationCallbacks;
        desc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
        if(!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &pDevice->pResourceAllocator)))
        {
            cyber_assert(false, "DMA Allocator Create Failed!");
        }
    }

    // Shader Reflection
    const char8_t* D3DShaderEntryName = u8"D3D12";
    static ERHIResourceType gD3D12_TO_DESCRIPTOR[] = {
        RHI_RESOURCE_TYPE_UNIFORM_BUFFER,   // D3D_SIT_CBUFFER
        RHI_RESOURCE_TYPE_BUFFER,           // D3D_SIT_TBUFFER
        RHI_RESOURCE_TYPE_TEXTURE,          // D3D_SIT_TEXTURE
        RHI_RESOURCE_TYPE_SAMPLER,          // D3D_SIT_SAMPLER
        RHI_RESOURCE_TYPE_RW_TEXTURE,       // D3D_SIT_UAV_RWTYPED
        RHI_RESOURCE_TYPE_BUFFER,           // D3D_SIT_STRUCTURED
        RHI_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_RWSTRUCTURED
        RHI_RESOURCE_TYPE_BUFFER,           // D3D_SIT_BYTEADDRESS
        RHI_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_UAV_RWBYTEADDRESS
        RHI_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_UAV_APPEND_STRUCTURED
        RHI_RESOURCE_TYPE_RW_BUFFER,       // D3D_SIT_UAV_CONSUME_STRUCTURED
        RHI_RESOURCE_TYPE_RW_BUFFER,       // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
        RHI_RESOURCE_TYPE_RAY_TRACING,     // D3D_SIT_RTACCELERATIONSTRUCTURE
    };

    static ERHITextureDimension gD3D12_TO_TEXTURE_DIM[D3D_SRV_DIMENSION_BUFFEREX + 1] = {
        RHI_TEX_DIMENSION_UNDEFINED,        // D3D_SRV_DIMENSION_UNKNOWN
        RHI_TEX_DIMENSION_UNDEFINED,        // D3D_SRV_DIMENSION_BUFFER
        RHI_TEX_DIMENSION_1D,               // D3D_SRV_DIMENSION_TEXTURE1D
        RHI_TEX_DIMENSION_1D_ARRAY,         // D3D_SRV_DIMENSION_TEXTURE1DARRAY
        RHI_TEX_DIMENSION_2D,               // D3D_SRV_DIMENSION_TEXTURE2D
        RHI_TEX_DIMENSION_2D_ARRAY,         // D3D_SRV_DIMENSION_TEXTURE2DARRAY
        RHI_TEX_DIMENSION_2DMS,             // D3D_SRV_DIMENSION_TEXTURE2DMS
        RHI_TEX_DIMENSION_2DMS_ARRAY,       // D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
        RHI_TEX_DIMENSION_3D,               // D3D_SRV_DIMENSION_TEXTURE3D
        RHI_TEX_DIMENSION_CUBE,             // D3D_SRV_DIMENSION_TEXTURECUBE
        RHI_TEX_DIMENSION_CUBE_ARRAY,      // D3D_SRV_DIMENSION_TEXTURECUBEARRAY
        RHI_TEX_DIMENSION_UNDEFINED,       // D3D_SRV_DIMENSION_BUFFEREX
    };

    static ERHIFormat gD3D12_TO_VERTEX_FORMAT[] = {
        RHI_FORMAT_UNDEFINED,
        RHI_FORMAT_R32_UINT,
        RHI_FORMAT_R32_SINT,
        RHI_FORMAT_R32_SFLOAT,

        RHI_FORMAT_R32G32_UINT,
        RHI_FORMAT_R32G32_SINT,
        RHI_FORMAT_R32G32_SFLOAT,

        RHI_FORMAT_R32G32B32_UINT,
        RHI_FORMAT_R32G32B32_SINT,
        RHI_FORMAT_R32G32B32_SFLOAT,

        RHI_FORMAT_R32G32B32A32_UINT,
        RHI_FORMAT_R32G32B32A32_SINT,
        RHI_FORMAT_R32G32B32A32_SFLOAT,
    };

    CYBER_FORCE_INLINE void D3D12Util_ReflectionRecordShaderResource(ID3D12ShaderReflection* d3d12Reflection, ERHIShaderStage stage, const D3D12_SHADER_DESC& shaderDesc,RHIShaderLibrary_D3D12* library)
    {
        // Get the number of bound resources
        library->entry_count = 1;
        library->entry_reflections = (RHIShaderReflection*)cyber_calloc(library->entry_count, sizeof(RHIShaderReflection));
        RHIShaderReflection* reflection = library->entry_reflections;
        reflection->entry_name = D3DShaderEntryName;
        reflection->shader_resource_count = shaderDesc.BoundResources;
        reflection->shader_resources = (RHIShaderResource*)cyber_calloc(shaderDesc.BoundResources, sizeof(RHIShaderResource));

        // Count string sizes of the bound resources for the name pool
        for(UINT i = 0;i < shaderDesc.BoundResources; ++i)
        {
            D3D12_SHADER_INPUT_BIND_DESC bindDesc;
            d3d12Reflection->GetResourceBindingDesc(i, &bindDesc);
            const size_t source_len = strlen(bindDesc.Name);
            reflection->shader_resources[i].name = (char8_t*)cyber_malloc(sizeof(char8_t) * (source_len + 1));
            reflection->shader_resources[i].name_hash = rhi_name_hash(bindDesc.Name, strlen(bindDesc.Name + 1));
            
            // We are very sure it's windows platform
            strcpy_s((char*)reflection->shader_resources[i].name, source_len + 1, bindDesc.Name);
            reflection->shader_resources[i].type = gD3D12_TO_DESCRIPTOR[bindDesc.Type];
            reflection->shader_resources[i].set = bindDesc.Space;
            reflection->shader_resources[i].binding = bindDesc.BindPoint;
            reflection->shader_resources[i].size = bindDesc.BindCount;
            reflection->shader_resources[i].stages = stage;
            reflection->shader_resources[i].dimension = gD3D12_TO_TEXTURE_DIM[bindDesc.Dimension];
            if(shaderDesc.ConstantBuffers && bindDesc.Type == D3D_SIT_CBUFFER)
            {
                ID3D12ShaderReflectionConstantBuffer* buffer = d3d12Reflection->GetConstantBufferByName(bindDesc.Name);
                cyber_assert(buffer, "D3D12 reflection failed : CBV not found!");
                D3D12_SHADER_BUFFER_DESC bufferDesc;
                buffer->GetDesc(&bufferDesc);
                reflection->shader_resources[i].size = bufferDesc.Size;
            }
            // RWTyped is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for RWBuffer here
            if(bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
            {
                reflection->shader_resources[i].type = RHI_RESOURCE_TYPE_RW_BUFFER;
            }
            // Buffer<> is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for Buffer<> here
            if(bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
            {
                reflection->shader_resources[i].type = RHI_RESOURCE_TYPE_BUFFER;
            }
        }
    }

    CYBER_FORCE_INLINE void D3D12Util_CollectShaderReflectionData(ID3D12ShaderReflection* d3d12Reflection, ERHIShaderStage stage, RHIShaderLibrary_D3D12* library)
    {
        D3D12_SHADER_DESC shaderDesc;
        d3d12Reflection->GetDesc(&shaderDesc);
        D3D12Util_ReflectionRecordShaderResource(d3d12Reflection, stage, shaderDesc, library);
        RHIShaderReflection* reflection = library->entry_reflections;
        reflection->shader_stage = stage;

        // Collect vertex inputs
        if(stage == RHI_SHADER_STAGE_VERT)
        {
            reflection->vertex_input_count = shaderDesc.InputParameters;
            reflection->vertex_inputs = (RHIVertexInput*)cyber_calloc(reflection->vertex_input_count, sizeof(RHIVertexInput));
            // Count the string sizes of the vertex inputs for the name pool
            for(UINT i = 0; i < shaderDesc.InputParameters; ++i)
            {
                D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                d3d12Reflection->GetInputParameterDesc(i, &paramDesc);
                // Get the length of the semantic name
                bool hasParamIndex = paramDesc.SemanticIndex > 0 || !strcmp(paramDesc.SemanticName, "TEXCOORD");
                uint32_t source_len = (uint32_t)strlen(paramDesc.SemanticName) + (hasParamIndex ? 1 : 0);

                reflection->vertex_inputs[i].name = (char8_t*)cyber_malloc(sizeof(char8_t) * (source_len+1));
                if(hasParamIndex)
                    sprintf((char*)reflection->vertex_inputs[i].name, "%s%u", paramDesc.SemanticName, paramDesc.SemanticIndex);
                else
                    sprintf((char*)reflection->vertex_inputs[i].name, "%s", paramDesc.SemanticName);
                const uint32_t comps = (uint32_t)log2(paramDesc.Mask);
                reflection->vertex_inputs[i].format = gD3D12_TO_VERTEX_FORMAT[(paramDesc.ComponentType + 3 * comps)];
            }
        }
        else if(stage == RHI_SHADER_STAGE_COMPUTE)
        {
            d3d12Reflection->GetThreadGroupSize(
                &reflection->thread_group_sizes[0], 
                &reflection->thread_group_sizes[1], 
                &reflection->thread_group_sizes[2]);
        }
    }

    void D3D12Util_InitializeShaderReflection(ID3D12Device* device, RHIShaderLibrary_D3D12* library, const RHIShaderLibraryCreateDesc& desc)
    {
        ID3D12ShaderReflection* d3d12Reflection = nullptr;
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) \
        ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 | (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)

        IDxcContainerReflection* reflection;
        UINT32 shaderIdx;
        auto procDxcCreateInstance = D3D12Util_GetDxcCreateInstanceProc();
        cyber_assert(procDxcCreateInstance, "Failed to get dxc proc!");
        procDxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&reflection));
        HRESULT hr = S_OK;

        hr = reflection->Load(library->shader_blob);
        hr = reflection->FindFirstPartKind(DXIL_FOURCC('D','X','I','L'), &shaderIdx);

        CHECK_HRESULT(reflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&d3d12Reflection)));
        if(d3d12Reflection)
        {
            D3D12Util_CollectShaderReflectionData(d3d12Reflection, desc.stage, library);
        }

        reflection->Release();
        d3d12Reflection->Release();
    }

    void D3D12Util_FreeShaderReflection(RHIShaderLibrary_D3D12* library)
    {
        if(library->entry_reflections)
        {
            RHIShaderReflection* reflection = library->entry_reflections;
            if(reflection->vertex_inputs)
            {
                for(uint32_t i = 0; i < reflection->vertex_input_count; ++i)
                {
                    cyber_free((void*)reflection->vertex_inputs[i].name);
                }
                cyber_free(reflection->vertex_inputs);
            }
            if(reflection->shader_resources)
            {
                for(uint32_t i = 0; i < reflection->shader_resource_count; ++i)
                {
                    cyber_free((void*)reflection->shader_resources[i].name);
                }
                cyber_free(reflection->shader_resources);
            }
            cyber_free(reflection);
        }
    }

    void D3D12Util_SignalFence(RHIQueue_D3D12* queue, ID3D12Fence* fence, uint64_t fenceValue)
    {
        queue->pCommandQueue->Signal(fence, fenceValue);
    }

    void D3D12Util_CreateCBV(RHIDevice_D3D12* pDevice, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pCbvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
        pDevice->pDxDevice->CreateConstantBufferView(pCbvDesc, *pHandle);
    }

    void D3D12Util_CreateSRV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
        pDevice->pDxDevice->CreateShaderResourceView(pResource, pSrvDesc, *pHandle);
    }

    void D3D12Util_CreateUAV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
        pDevice->pDxDevice->CreateUnorderedAccessView(pResource, pCounterResource, pUavDesc, *pHandle);
    }

    void D3D12Util_CreateRTV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], 1).mCpu;
        pDevice->pDxDevice->CreateRenderTargetView(pResource, pRtvDesc, *pHandle);
    }

    void D3D12Util_CreateDSV(RHIDevice_D3D12* pDevice, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(pDevice->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], 1).mCpu;
        pDevice->pDxDevice->CreateDepthStencilView(pResource, pDsvDesc, *pHandle);
    }

    D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const RHIBlendStateCreateDesc* pDesc)
    {
        int blendDescIndex = 0;
        D3D12_BLEND_DESC ret = {};
        ret.AlphaToCoverageEnable = (BOOL)pDesc->alpha_to_coverage;
        ret.IndependentBlendEnable = TRUE;
        for(int i = 0; i < RHI_MAX_MRT_COUNT;++i)
        {
            BOOL blendEnable = (gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]] != D3D12_BLEND_ONE) || (gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]] != D3D12_BLEND_ZERO) ||
                                (gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]] != D3D12_BLEND_ONE) || (gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]] != D3D12_BLEND_ZERO);

            ret.RenderTarget[i].BlendEnable = blendEnable;
            ret.RenderTarget[i].RenderTargetWriteMask = (UINT8)pDesc->masks[blendDescIndex];
            ret.RenderTarget[i].BlendOp = gDx12BlendOpTranlator[pDesc->blend_modes[blendDescIndex]];
            ret.RenderTarget[i].SrcBlend = gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]];
            ret.RenderTarget[i].DestBlend = gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]];
            ret.RenderTarget[i].BlendOpAlpha = gDx12BlendOpTranlator[pDesc->blend_alpha_modes[blendDescIndex]];
            ret.RenderTarget[i].SrcBlendAlpha = gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]];
            ret.RenderTarget[i].DestBlendAlpha = gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]];

            if(pDesc->independent_blend)
            {
                ++blendDescIndex;
            }
        }
        return ret;
    }
    D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const RHIRasterizerStateCreateDesc* pDesc)
    {
        cyber_check(pDesc->fill_mode < RHI_FILL_MODE_COUNT);
        cyber_check(pDesc->cull_mode < RHI_CULL_MODE_COUNT);
        cyber_check(pDesc->front_face == RHI_FRONT_FACE_COUNTER_CLOCKWISE || pDesc->front_face == RHI_FRONT_FACE_CLOCKWISE);
        D3D12_RASTERIZER_DESC ret = {};
        ret.FillMode = gDx12FillModeTranslator[pDesc->fill_mode];
        ret.CullMode = gDx12CullModeTranslator[pDesc->cull_mode];
        ret.FrontCounterClockwise = (pDesc->front_face == RHI_FRONT_FACE_COUNTER_CLOCKWISE);
        ret.DepthBias = pDesc->depth_bias;
        ret.DepthBiasClamp = 0.0f;
        ret.SlopeScaledDepthBias = pDesc->slope_scaled_depth_bias;
        ret.DepthClipEnable = !pDesc->enable_depth_clip;
        ret.MultisampleEnable = pDesc->enable_multisample ? TRUE : FALSE;
        ret.AntialiasedLineEnable = false;
        ret.ForcedSampleCount = 0;
        ret.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        return ret;
    }

    D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDepthStencilState(const RHIDepthStateCreateDesc* pDesc)
    {
        cyber_check(pDesc->depth_func < RHI_CMP_COUNT);
        cyber_check(pDesc->stencil_front_func < RHI_CMP_COUNT);
        cyber_check(pDesc->stencil_front_fail_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_front_pass_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_front_depth_fail_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_fail_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_pass_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_depth_fail_op < RHI_STENCIL_OP_COUNT);

        D3D12_DEPTH_STENCIL_DESC ret = {};
        ret.DepthEnable = pDesc->depth_test ? TRUE : FALSE;
        ret.DepthWriteMask = pDesc->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        ret.DepthFunc = gDx12ComparisonFuncTranslator[pDesc->depth_func];
        ret.StencilEnable = pDesc->stencil_test ? TRUE : FALSE;
        ret.StencilReadMask = pDesc->stencil_read_mask;
        ret.StencilWriteMask = pDesc->stencil_write_mask;
        ret.FrontFace.StencilFunc = gDx12ComparisonFuncTranslator[pDesc->stencil_front_func];
        ret.FrontFace.StencilFailOp = gDx12StencilOpTranslator[pDesc->stencil_front_fail_op];
        ret.FrontFace.StencilDepthFailOp = gDx12StencilOpTranslator[pDesc->stencil_front_depth_fail_op];
        ret.FrontFace.StencilPassOp = gDx12StencilOpTranslator[pDesc->stencil_front_pass_op];
        ret.BackFace.StencilFunc = gDx12ComparisonFuncTranslator[pDesc->stencil_back_func];
        ret.BackFace.StencilFailOp = gDx12StencilOpTranslator[pDesc->stencil_back_fail_op];
        ret.BackFace.StencilDepthFailOp = gDx12StencilOpTranslator[pDesc->stencil_back_depth_fail_op];
        ret.BackFace.StencilPassOp = gDx12StencilOpTranslator[pDesc->stencil_back_pass_op];
        return ret;
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopologyType(ERHIPrimitiveTopology topology)
    {
        switch(topology)
        {
            case RHI_PRIM_TOPO_POINT_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case RHI_PRIM_TOPO_LINE_LIST:
            case RHI_PRIM_TOPO_LINE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case RHI_PRIM_TOPO_TRIANGLE_LIST:
            case RHI_PRIM_TOPO_TRIANGLE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case RHI_PRIM_TOPO_PATCH_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        }
        cyber_check(false);
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}