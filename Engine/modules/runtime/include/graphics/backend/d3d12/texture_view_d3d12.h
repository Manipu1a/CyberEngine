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
            TextureView_D3D12_Impl();
            virtual ~TextureView_D3D12_Impl();

        protected:
            D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
            /// Offset from mDxDescriptors for srv descriptor handle
            uint8_t mSrvDescriptorOffset;
            /// Offset from mDxDescriptors for uav descriptor handle
            uint8_t mUavDescriptorOffset;
            /// Offset from mDxDescriptors for rtv descriptor handle
            D3D12_CPU_DESCRIPTOR_HANDLE mRtvDsvDescriptorHandle;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}