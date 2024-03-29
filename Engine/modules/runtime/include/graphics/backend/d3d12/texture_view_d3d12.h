#pragma once

#include "graphics/interface/texture_view.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;

        struct ITextureView_D3D12 : public ITextureView
        {

        };

        class TextureView_D3D12_Impl : public Texture_View<EngineD3D12ImplTraits>
        {
        public:
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
            using TTextureViewBase = Texture_View<EngineD3D12ImplTraits>;

            TextureView_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const TextureViewCreateDesc& desc) : TTextureViewBase(device, desc) {  }
            virtual ~TextureView_D3D12_Impl();

        protected:
            D3D12_CPU_DESCRIPTOR_HANDLE m_dxDescriptorHandles;
            /// Offset from mDxDescriptors for srv descriptor handle
            uint8_t m_srvDescriptorOffset;
            /// Offset from mDxDescriptors for uav descriptor handle
            uint8_t m_uavDescriptorOffset;
            /// Offset from mDxDescriptors for rtv descriptor handle
            D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDsvDescriptorHandle;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}