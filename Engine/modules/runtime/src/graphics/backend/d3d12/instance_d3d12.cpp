#include "backend/d3d12/instance_d3d12.h"
#include "EASTL/vector.h"
#include "backend/d3d12/graphics_types_d3d12.h"
#include "backend/d3d12/adapter_d3d12.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        void Instance_D3D12_Impl::optional_enable_debug_layer()
        {
            if(m_desc.m_enableDebugLayer)
            {
                if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDXDebug))))
                {
                    m_pDXDebug->EnableDebugLayer();
                    if(m_desc.m_enableGpuBasedValidation)
                    {
                        ID3D12Debug1* pDebug1 = nullptr;
                        if(SUCCEEDED(m_pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
                        {
                            pDebug1->SetEnableGPUBasedValidation(m_desc.m_enableGpuBasedValidation);
                            pDebug1->Release();
                        }
                    }
                }
            }
            else if(m_desc.m_enableGpuBasedValidation)
            {
                cyber_warn(false, "D3D12 GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
            }
        }

        void Instance_D3D12_Impl::query_all_adapters(uint32_t& count, bool& foundSoftwareAdapter)
        {
            cyber_assert(m_pAdapters == nullptr, "getProperGpuCount should be called only once!");
            cyber_assert(m_AdaptersCount == 0, "getProperGpuCount should be called only once!");

            IDXGIAdapter* _adapter = nullptr;
            eastl::vector<IDXGIAdapter4*> dxgi_adapters;
            eastl::vector<D3D_FEATURE_LEVEL> adapter_levels;

            for(UINT i = 0; i < 10;i++)
            {
                if(!SUCCEEDED(m_pDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter))))
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
                    uint32_t level_c = GRAPHICS_ARRAY_LEN(d3d_feature_levels);

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
                                m_AdaptersCount++;
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
                    foundSoftwareAdapter = true;
                    SAFE_RELEASE(adapter);
                }
            }

            count = m_AdaptersCount;
            m_pAdapters = cyber_new_n<RenderObject::Adapter_D3D12_Impl>(m_AdaptersCount);

            for(uint32_t i = 0;i < count; i++)
            {
                RenderObject::Adapter_D3D12_Impl* pAdapter = static_cast<RenderObject::Adapter_D3D12_Impl*>(&m_pAdapters[i]);
                // Device Objects
                pAdapter->fill_adapter(RenderObject::AdapterDetail{}, adapter_levels[i], dxgi_adapters[i], false);
            }
        }
    }
}