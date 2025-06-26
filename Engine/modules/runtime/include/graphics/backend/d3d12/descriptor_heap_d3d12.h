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
            static DescriptorHandle consume_descriptor_handles(DescriptorHeap_D3D12* dst, uint32_t descriptorCount);

            uint32_t reserve_slots(uint32_t num_requested_slots);

            DescriptorHandle consume_descriptor_handles(uint32_t descriptorCount);

            DescriptorHandle get_slot_handle(uint32_t slot_index);
            
            void free_descriptor_handles(const DescriptorHandle& handle, uint32_t descriptorCount);

            void copy_descriptor_handle(const D3D12_CPU_DESCRIPTOR_HANDLE& srcHandle, const uint64_t& dstHandle, uint32_t index);

            static void create_descriptor_heap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, struct DescriptorHeap_D3D12** destHeap);

            void free();
            
            ID3D12DescriptorHeap* get_heap() const
            {
                return m_pCurrentHeap;
            }
            
            void set_heap(ID3D12DescriptorHeap* heap)
            {
                m_pCurrentHeap = heap;
            }
            
            CYBER_FORCE_INLINE uint32_t get_descriptor_size() const
            {
                return m_descriptorSize;
            }

            CYBER_FORCE_INLINE uint32_t get_used_descriptors() const
            {
                return m_usedDescriptors;
            }

            CYBER_FORCE_INLINE uint32_t get_num_descriptors() const
            {
                return m_numDescriptors;
            }

            CYBER_FORCE_INLINE D3D12_DESCRIPTOR_HEAP_TYPE get_type() const
            {
                return m_type;
            }

            CYBER_FORCE_INLINE DescriptorHandle get_start_handle() const
            {
                return m_startHandle;
            }
            
        protected:
            /// DX Heap
            ID3D12DescriptorHeap* m_pCurrentHeap;
            ID3D12Device* m_pDevice;
            D3D12_CPU_DESCRIPTOR_HANDLE* m_pHandles;
            /// Start position in the heap
            DescriptorHandle m_startHandle;
            /// Free List used for CPU only descriptor heaps
            eastl::vector<DescriptorHandle> m_freeList;
            // Bitmask to track free regions (set bit means occupied)
            uint32_t* m_pFlags;
            /// Description
            D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
            D3D12_DESCRIPTOR_HEAP_TYPE m_type;
            uint32_t m_numDescriptors;
            /// Descriptor Increment Size
            uint32_t m_descriptorSize;
            // Usage
            uint32_t m_usedDescriptors;
        };

        struct EmptyDescriptors_D3D12
        {
            D3D12_CPU_DESCRIPTOR_HANDLE Sampler;
            D3D12_CPU_DESCRIPTOR_HANDLE TextureSRV[(uint8_t)TEXTURE_DIMENSION::TEX_DIMENSION_COUNT];
            D3D12_CPU_DESCRIPTOR_HANDLE TextureUAV[(uint8_t)TEXTURE_DIMENSION::TEX_DIMENSION_COUNT];
            D3D12_CPU_DESCRIPTOR_HANDLE BufferSRV;
            D3D12_CPU_DESCRIPTOR_HANDLE BufferUAV;
            D3D12_CPU_DESCRIPTOR_HANDLE BufferCBV;
        };
    }
}

