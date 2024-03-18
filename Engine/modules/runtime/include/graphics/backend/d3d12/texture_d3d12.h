#pragma once

#include "graphics/interface/texture.hpp"
#include "engine_impl_traits_d3d12.hpp"
#include "texture_view_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;

        struct ITexture_D3D12 : public ITexture 
        {
            virtual ID3D12Resource* get_d3d12_resource() const = 0;
        };

        class CYBER_GRAPHICS_API Texture_D3D12_Impl : public Texture<EngineD3D12ImplTraits>
        {
        public:
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
            using TextureViewImplType = EngineD3D12ImplTraits::TextureViewImplType;
            using TTextureBase = Texture<EngineD3D12ImplTraits>;
            
            Texture_D3D12_Impl(class RenderDevice_D3D12_Impl* device) :TTextureBase(device) {}

            virtual ~Texture_D3D12_Impl()
            {

            }
            
            virtual ID3D12Resource* get_d3d12_resource() const override
            {
                return native_resource;
            }

            virtual void* get_native_texture() const override;
        protected:
            virtual TextureView_D3D12_Impl* create_view_internal(const TextureViewCreateDesc& desc) const override;
        protected:
            ID3D12Resource* native_resource;
            D3D12MA::Allocation* allocation;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}
