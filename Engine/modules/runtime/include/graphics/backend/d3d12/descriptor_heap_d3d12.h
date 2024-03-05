#pragma once
#include "d3d12.config.h"
#include "EASTL/vector.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct DescriptorHandle
        {
            D3D12_CPU_DESCRIPTOR_HANDLE mCpu;
            D3D12_GPU_DESCRIPTOR_HANDLE mGpu;
        };

        /// CPU Visible Heap to store all the resources needing CPU read / write operations - Textures/Buffers/RTV
        class DescriptorHeap_D3D12
        {
        public:

            DescriptorHandle consume_descriptor_handles(uint32_t descriptorCount);
            
            void free_descriptor_handles(const DescriptorHandle& handle, uint32_t descriptorCount);

            void copy_descriptor_handle(const D3D12_CPU_DESCRIPTOR_HANDLE& srcHandle, const uint64_t& dstHandle, uint32_t index);

            static void create_descriptor_heap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, struct DescriptorHeap_D3D12** destHeap);

            void free();
            
        protected:
            /// DX Heap
            ID3D12DescriptorHeap* m_pCurrentHeap;
            ID3D12Device* m_pDevice;
            D3D12_CPU_DESCRIPTOR_HANDLE* m_pHandles;
            /// Start position in the heap
            DescriptorHandle m_StartHandle;
            /// Free List used for CPU only descriptor heaps
            eastl::vector<DescriptorHandle> m_FreeList;
            // Bitmask to track free regions (set bit means occupied)
            uint32_t* m_pFlags;
            /// Description
            D3D12_DESCRIPTOR_HEAP_DESC m_Desc;
            D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
            uint32_t m_NumDescriptors;
            /// Descriptor Increment Size
            uint32_t m_DescriptorSize;
            // Usage
            uint32_t m_UsedDescriptors;
        };

        struct EmptyDescriptors_D3D12
        {
            D3D12_CPU_DESCRIPTOR_HANDLE Sampler;
            D3D12_CPU_DESCRIPTOR_HANDLE TextureSRV[TEXTURE_DIMENSION::GRAPHCIS_TEX_DIMENSION_COUNT - 1];
            D3D12_CPU_DESCRIPTOR_HANDLE TextureUAV[TEXTURE_DIMENSION::GRAPHCIS_TEX_DIMENSION_COUNT - 1];
            D3D12_CPU_DESCRIPTOR_HANDLE BufferSRV;
            D3D12_CPU_DESCRIPTOR_HANDLE BufferUAV;
            D3D12_CPU_DESCRIPTOR_HANDLE BufferCBV;
        };
    }
}

