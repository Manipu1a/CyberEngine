#include "graphics/backend/d3d12/texture_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        Texture_D3D12_Impl::Texture_D3D12_Impl(class CERenderDevice_D3D12* device) : TTextureBase(device)
        {
            
        }

        void* Texture_D3D12_Impl::get_native_texture() const
        {
            return native_resource;
        }

        TextureView_D3D12_Impl* Texture_D3D12_Impl::get_default_texture_view() const
        {

        }

        TextureView_D3D12_Impl* Texture_D3D12_Impl::create_view_internal(const TextureViewCreateDesc& desc) const
        {
            
        }
    }
}