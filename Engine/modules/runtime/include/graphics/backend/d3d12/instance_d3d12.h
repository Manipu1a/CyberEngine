#pragma once

#include "graphics/interface/instance.h"
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
        struct CYBER_GRAPHICS_API IInstance_D3D12 : public IInstance
        {
            
        };

        class CYBER_GRAPHICS_API Instance_D3D12_Impl : public InstanceBase<EngineD3D12ImplTraits>
        {
        public:
            using TInstanceBase = InstanceBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Instance_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const InstanceCreateDesc& desc) : TInstanceBase(device, desc) {}

            void optional_enable_debug_layer();
            void query_all_adapters(uint32_t& count, bool& foundSoftwareAdapter);
        protected:
        #if defined(XBOX)
        #elif defined(_WINDOWS)
            struct IDXGIFactory6* m_pDXGIFactory;
        #endif
            struct ID3D12Debug* m_pDXDebug;
            struct IAdapter* m_pAdapters;
            uint32_t m_AdaptersCount;

        protected:
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
