#pragma once

#include "graphics/interface/texture.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CERenderDevice_D3D12;

        class CYBER_GRAPHICS_API Texture_D3D12 : public Texture
        {
        public:
            Texture_D3D12()
            {

            }

            virtual ~Texture_D3D12()
            {

            }

            virtual void* get_native_texture() const override
            {
                return pDxResource;
            }
            virtual Texture_View* get_default_texture_view() const override
            {
                return nullptr;
            }
            virtual Texture_View* create_texture_view(const TextureViewCreateDesc& desc) const override
            {
                return nullptr;
            }

        protected:

            ID3D12Resource* pDxResource;
            D3D12MA::Allocation* pDxAllocation;

            friend class RenderObject::CERenderDevice_D3D12;
        };
    }
}
