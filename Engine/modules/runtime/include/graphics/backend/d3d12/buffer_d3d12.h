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
            using TBufferBase = Buffer<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            Buffer_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TBufferBase(device) {}

            virtual ~Buffer_D3D12_Impl() = default;

            D3D12_GPU_VIRTUAL_ADDRESS get_dx_gpu_address() const { return m_dxGpuAddress; }
            void set_dx_gpu_address(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress) { m_dxGpuAddress = gpuAddress; }

            D3D12_CPU_DESCRIPTOR_HANDLE get_dx_descriptor_handles() const { return m_dxDescriptorHandles; }
            void set_dx_descriptor_handles(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandles) { m_dxDescriptorHandles = descriptorHandles; }

            uint8_t get_srv_descriptor_offset() const { return m_srvDescriptorOffset; }
            void set_srv_descriptor_offset(uint8_t offset) { m_srvDescriptorOffset = offset; }

            uint8_t get_uav_descriptor_offset() const { return m_uavDescriptorOffset; }
            void set_uav_descriptor_offset(uint8_t offset) { m_uavDescriptorOffset = offset; }

            ID3D12Resource* get_dx_resource() const { return m_pDxResource; }
            void set_dx_resource(ID3D12Resource* resource) { m_pDxResource = resource; }

            D3D12MA::Allocation* get_dx_allocation() const { return m_pDxAllocation; }
            void set_dx_allocation(D3D12MA::Allocation* allocation) { m_pDxAllocation = allocation; }
        protected:
            /// GPU Address - Cache to avoid calls to ID3D12Resource::GetGpuVirtualAddress
            D3D12_GPU_VIRTUAL_ADDRESS m_dxGpuAddress;
            /// Descriptor handle of the CBV in a CPU visible descriptor heap (applicable to BUFFER_USAGE_UNIFORM)
            D3D12_CPU_DESCRIPTOR_HANDLE m_dxDescriptorHandles;
            /// Offset from mDxDescriptors for srv descriptor handle
            uint8_t m_srvDescriptorOffset;
            /// Offset from mDxDescriptors for uav descriptor handle
            uint8_t m_uavDescriptorOffset;
            /// Native handle of the underlying resource
            ID3D12Resource* m_pDxResource;
            /// Contains resource allocation info such as parent heap, offset in heap
            D3D12MA::Allocation* m_pDxAllocation;
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
