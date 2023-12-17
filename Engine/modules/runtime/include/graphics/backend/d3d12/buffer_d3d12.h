#pragma once

#include "graphics/interface/buffer.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CERenderDevice_D3D12;
    }

    namespace RenderObject
    {
        class CYBER_GRAPHICS_API Buffer_D3D12 : public Buffer
        {
        public:
            /// GPU Address - Cache to avoid calls to ID3D12Resource::GetGpuVirtualAddress
            D3D12_GPU_VIRTUAL_ADDRESS mDxGpuAddress;
            /// Descriptor handle of the CBV in a CPU visible descriptor heap (applicable to BUFFER_USAGE_UNIFORM)
            D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
            /// Offset from mDxDescriptors for srv descriptor handle
            uint8_t mSrvDescriptorOffset;
            /// Offset from mDxDescriptors for uav descriptor handle
            uint8_t mUavDescriptorOffset;
            /// Native handle of the underlying resource
            ID3D12Resource* pDxResource;
            /// Contains resource allocation info such as parent heap, offset in heap
            D3D12MA::Allocation* pDxAllocation;

        protected:
            friend class RenderObject::CERenderDevice_D3D12;
        };

    }
}
