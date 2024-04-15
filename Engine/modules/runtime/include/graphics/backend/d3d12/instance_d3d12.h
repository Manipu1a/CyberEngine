#pragma once

#include "graphics/interface/instance.h"
#include "engine_impl_traits_d3d12.hpp"
#include "render_device_d3d12.h"

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

            Instance_D3D12_Impl(const InstanceCreateDesc& desc);

            virtual void initialize_environment() override final;
            virtual void de_initialize_environment() override final;
            virtual void optional_enable_debug_layer() override final;
            virtual void query_all_adapters(uint32_t& count, bool& foundSoftwareAdapter) override final;

            virtual IRenderDevice* create_render_device(IAdapter* adapter, const RenderDeviceCreateDesc& desc) override final;

            virtual void free() override final;

        #if defined(XBOX)
        #elif defined(_WINDOWS)
            struct IDXGIFactory6* get_dxgi_factory() const { return m_pDXGIFactory; }
            void set_dxgi_factory(struct IDXGIFactory6* factory) { m_pDXGIFactory = factory; }
        #endif  
            struct ID3D12Debug* get_dx_debug() const { return m_pDXDebug; }
            void set_dx_debug(struct ID3D12Debug* debug) { m_pDXDebug = debug; }

            struct IAdapter** get_adapters() const { return m_pAdapters; }
            void set_adapters(struct IAdapter** adapters) { m_pAdapters = adapters; }

            struct IAdapter* get_adapter(uint32_t index) const { return m_pAdapters[index]; }
            void set_adapter(uint32_t index, struct IAdapter* adapter) { m_pAdapters[index] = adapter; }
            
            uint32_t get_adapters_count() const { return m_adaptersCount; }
            void set_adapters_count(uint32_t count) { m_adaptersCount = count; }
        protected:
        #if defined(XBOX)
        #elif defined(_WINDOWS)
            struct IDXGIFactory6* m_pDXGIFactory;
        #endif
            struct ID3D12Debug* m_pDXDebug;
            struct IAdapter** m_pAdapters;
            uint32_t m_adaptersCount;

        protected:
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
