#pragma once

#include "graphics/interface/texture_view.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CERenderDevice_D3D12;

        class CYBER_GRAPHICS_API Texture_View_D3D12 : public Texture_View
        {
        public:
            Texture_View_D3D12() = default;
            virtual ~Texture_View_D3D12() = default;

        protected:
            D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
            /// Offset from mDxDescriptors for srv descriptor handle
            uint8_t mSrvDescriptorOffset;
            /// Offset from mDxDescriptors for uav descriptor handle
            uint8_t mUavDescriptorOffset;
            /// Offset from mDxDescriptors for rtv descriptor handle
            D3D12_CPU_DESCRIPTOR_HANDLE mRtvDsvDescriptorHandle;

            friend class RenderObject::CERenderDevice_D3D12;
        };
    }
}