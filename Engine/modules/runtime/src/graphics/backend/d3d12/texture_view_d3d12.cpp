#include "graphics/backend/d3d12/texture_view_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        TextureView_D3D12_Impl::TextureView_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TTextureViewBase(device)
        {
        }

        TextureView_D3D12_Impl::~TextureView_D3D12_Impl()
        {
        }
    }
}