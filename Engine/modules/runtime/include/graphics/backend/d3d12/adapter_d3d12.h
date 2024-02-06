#pragma once

#include "graphics/interface/adapter.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IAdapter_D3D12 : public IAdapter
        {
            
        };

        class CYBER_GRAPHICS_API Adapter_D3D12_Impl : public AdapterBase<EngineD3D12ImplTraits>
        {
        public:
            using TAdapterBase = AdapterBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Adapter_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TAdapterBase(device) {}

            IDXGIAdapter4* get_native_adapter() const
            {
                return pDxActiveGPU;
            }

            void fill_adapter(const AdapterDetail& detail, D3D_FEATURE_LEVEL featureLevel, IDXGIAdapter4* adapter, bool enhanceBarrierSupported)
            {
                mAdapterDetail = detail;
                mFeatureLevel = featureLevel;
                pDxActiveGPU = adapter;
                mEnhanceBarrierSupported = enhanceBarrierSupported;
            }
        protected:
        #if defined(XBOX)
        #elif defined(_WINDOWS)
            struct IDXGIAdapter4* pDxActiveGPU;
        #endif
            D3D_FEATURE_LEVEL mFeatureLevel;
            AdapterDetail mAdapterDetail;
            bool mEnhanceBarrierSupported : 1;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
