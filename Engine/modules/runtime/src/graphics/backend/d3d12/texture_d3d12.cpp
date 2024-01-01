#include "graphics/backend/d3d12/texture_d3d12.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/texture_view_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        Texture_D3D12_Impl::Texture_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TTextureBase(device)
        {
            
        }

        void* Texture_D3D12_Impl::get_native_texture() const
        {
            return native_resource;
        }

        ITextureView* Texture_D3D12_Impl::get_default_texture_view() const
        {
            return nullptr;
        }

        TextureView_D3D12_Impl* Texture_D3D12_Impl::create_view_internal(const TextureViewCreateDesc& desc) const
        {
            auto* device = get_device();

            auto texture_view = device->create_texture_view(desc);
            
            if(texture_view == nullptr)
            {
                
                return nullptr;
            }

            return static_cast<TextureView_D3D12_Impl*>(texture_view);
        }
    }
}