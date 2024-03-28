#pragma once

#include "graphics/interface/swap_chain.hpp"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;

        struct ISwapChain_D3D12 : public ISwapChain 
        {

        };

        class CYBER_GRAPHICS_API SwapChain_D3D12_Impl final : public SwapChainBase<EngineD3D12ImplTraits>
        {
        public:
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
            
            SwapChain_D3D12_Impl(class RenderDevice_D3D12_Impl* device);

            virtual ~SwapChain_D3D12_Impl()
            {

            }
            
            IDXGISwapChain3* get_dx_swap_chain() const { return m_pDxSwapChain; }
            void set_dx_swap_chain(IDXGISwapChain3* swapChain) { m_pDxSwapChain = swapChain; }

            uint32_t get_dx_sync_interval() const { return m_dxSyncInterval; }
            void set_dx_sync_interval(uint32_t syncInterval) { m_dxSyncInterval = syncInterval; }

            uint32_t get_flags() const { return m_flags; }
            void set_flags(uint32_t flags) { m_flags = flags; }

            uint32_t get_image_count() const { return m_imageCount; }
            void set_image_count(uint32_t count) { m_imageCount = count; }

            uint32_t get_enable_vsync() const { return m_enableVsync; }
            void set_enable_vsync(uint32_t enableVsync) { m_enableVsync = enableVsync; }

            virtual void free() override final;
        protected:
            IDXGISwapChain3* m_pDxSwapChain;
            uint32_t m_dxSyncInterval : 3;
            uint32_t m_flags : 10;
            uint32_t m_imageCount : 3;
            uint32_t m_enableVsync : 1;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}
