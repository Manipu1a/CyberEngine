#include "backend/d3d12/instance_d3d12.h"
#include "EASTL/vector.h"
#include "backend/d3d12/graphics_types_d3d12.h"
#include "backend/d3d12/adapter_d3d12.h"
#include "platform/memory.h"
#include "d3d12_utils.h"

namespace Cyber
{
    namespace RenderObject
    {
        void Instance_D3D12_Impl::initialize_environment()
        {

        }

        void Instance_D3D12_Impl::de_initialize_environment()
        {
            
        }

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
            cyber_assert(m_adaptersCount == 0, "getProperGpuCount should be called only once!");

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
                                m_adaptersCount++;
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

            count = m_adaptersCount;
            m_pAdapters = (RenderObject::IAdapter**)cyber_calloc(m_adaptersCount, sizeof(RenderObject::IAdapter*));

            for(uint32_t i = 0;i < count; i++)
            {
                RenderObject::Adapter_D3D12_Impl* pAdapter = cyber_new<RenderObject::Adapter_D3D12_Impl>();
                // Device Objects
                pAdapter->fill_adapter(RenderObject::AdapterDetail{}, adapter_levels[i], dxgi_adapters[i], false);
                m_pAdapters[i] = pAdapter;
            }
        }

        void Instance_D3D12_Impl::free()
        {
            de_initialize_environment();
            if(m_adaptersCount > 0)
            {
                for(uint32_t i = 0;i < m_adaptersCount; i++)
                {
                    m_pAdapters[i]->free();
                }
            }
            cyber_free(m_pAdapters);
            SAFE_RELEASE(m_pDXGIFactory);
            if(m_pDXDebug)
            {
                SAFE_RELEASE(m_pDXDebug);
            }

        #if !defined (XBOX) && defined (_WIN32)
            d3d12_util_unload_dxc_dll();
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

            TInstanceBase::free();

            delete this;
        }
    }
}