#pragma once

#include "graphics/interface/buffer.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }


    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IBuffer_D3D12 : public IBuffer
        {
            
        };

        class CYBER_GRAPHICS_API Buffer_D3D12_Impl : public Buffer<EngineD3D12ImplTraits>
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
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
