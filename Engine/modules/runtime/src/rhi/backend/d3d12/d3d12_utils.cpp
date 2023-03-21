#include "d3d12_utils.h"
#include "../src/rhi/common/common_utils.h"
#include "platform/configure.h"
#include <winnt.h>
namespace Cyber
{
    void D3D12Util_InitializeEnvironment(Ref<RHIInstance> pInst)
    {

    }

    void D3D12Util_DeInitializeEnvironment(Ref<RHIInstance> pInst)
    {
        
    }

    void D3D12Util_Optionalenable_debug_layer(RHIInstance_D3D12* result, const RHIInstanceCreateDesc& instanceDesc)
    {
        if(instanceDesc.mEnableDebugLayer)
        {
            if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&result->pDXDebug))))
            {
                result->pDXDebug->EnableDebugLayer();
                if(instanceDesc.mEnableGpuBasedValidation)
                {
                    ID3D12Debug1* pDebug1 = nullptr;
                    if(SUCCEEDED(result->pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
                    {
                        pDebug1->SetEnableGPUBasedValidation(instanceDesc.mEnableGpuBasedValidation);
                        pDebug1->Release();
                    }
                }
            }
        }
        else if(instanceDesc.mEnableGpuBasedValidation)
        {
            cyber_warn("D3D12 GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
        }
    }

    void D3D12Util_QueryAllAdapters(RHIInstance_D3D12* pInstance, uint32_t& pCount, bool& pFoundSoftwareAdapter)
    {
        cyber_assert(pInstance->pAdapters == nullptr, "getProperGpuCount should be called only once!");
        cyber_assert(pInstance->mAdaptersCount == 0, "getProperGpuCount should be called only once!");

        IDXGIAdapter* _adapter = nullptr;
        eastl::vector<IDXGIAdapter4*> dxgi_adapters;
        eastl::vector<D3D_FEATURE_LEVEL> adapter_levels;
        Ref<RHIInstance_D3D12> instanceRef = CreateRef<RHIInstance_D3D12>(pInstance);

        for(UINT i = 0; pInstance->pDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter));i++)
        {
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
                            pInstance->mAdaptersCount++;
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

        pCount = pInstance->mAdaptersCount;
        pInstance->pAdapters = (RHIAdapter_D3D12*)cb_malloc(sizeof(RHIAdapter_D3D12) * pInstance->mAdaptersCount);
       
        for(uint32_t i = 0;i < pCount; i++)
        {
            auto& adapter = pInstance->pAdapters[i];
            // Device Objects
            adapter.pDxActiveGPU = dxgi_adapters[i];
            adapter.mFeatureLevel = adapter_levels[i];
            adapter.pInstance = instanceRef;
        }
    }

    void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& pDesc, struct RHIDescriptorHeap_D3D12** ppDescHeap)
    {
        uint32_t numDesciptors = pDesc.NumDescriptors;
        RHIDescriptorHeap_D3D12* pHeap = (RHIDescriptorHeap_D3D12*)cb_calloc(1, sizeof(*pHeap));
        // TODO thread safety

        pHeap->pDevice = pDevice;

        // Keep 32 aligned for easy remove
        numDesciptors = cyber_round_up(numDesciptors, 32);

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
            pHeap->pHandles = (D3D12_CPU_DESCRIPTOR_HANDLE*)cb_calloc(Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
        }

        *ppDescHeap = pHeap;
    }

    void D3D12Util_CreateDMAAllocator(RHIInstance_D3D12* pInstance, RHIAdapter_D3D12* pAdapter, RHIDevice_D3D12* pDevice)
    {
        D3D12MA::ALLOCATOR_DESC desc = {};
        desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
        desc.pDevice = pDevice->pDxDevice;
        desc.pAdapter = pAdapter->pDxActiveGPU;

        D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
        allocationCallbacks.pAllocate = [](size_t size, size_t alignment, void*){
            return cb_memalign(size, alignment);
        };
        allocationCallbacks.pFree = [](void* ptr, void*){
            cb_free(ptr);
        };
        desc.pAllocationCallbacks = &allocationCallbacks;
        desc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
        if(!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &pDevice->pResourceAllocator)))
        {
            cyber_assert(false, "DMA Allocator Create Failed!");
        }

    }

    void D3D12Util_InitializeShaderReflection(ID3D12Device* device, RHISHaderLibrary_D3D12* library, const RHIShaderLibraryCreateDesc& desc)
    {

    }
}