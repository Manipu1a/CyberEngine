#include "backend/d3d12/descriptor_heap_d3d12.h"
#include "platform/memory.h"
#include "backend/d3d12/graphics_types_d3d12.h"
#include "CyberLog/Log.h"

namespace Cyber
{
    namespace RenderObject
    {
        DescriptorHandle DescriptorHeap_D3D12::consume_descriptor_handles(DescriptorHeap_D3D12* dst, uint32_t descriptorCount)
        {
            cyber_check(dst);

            return dst->consume_descriptor_handles(descriptorCount);
        }

        uint32_t DescriptorHeap_D3D12::reserve_slots(uint32_t num_requested_slots)
        {
            uint32_t first_requested_slot = m_usedDescriptors;
            uint32_t slot_after_request = m_usedDescriptors + num_requested_slots;

            if(slot_after_request >= m_heapDesc.NumDescriptors)
            {
                first_requested_slot = 0;
                slot_after_request = num_requested_slots;
            }   

            m_usedDescriptors = slot_after_request;

            return first_requested_slot;
        }

        DescriptorHandle DescriptorHeap_D3D12::consume_descriptor_handles(uint32_t descriptorCount)
        {
            if(m_usedDescriptors + descriptorCount > m_heapDesc.NumDescriptors)
            {
            #ifdef CYBER_THREAD_SAFETY
            #endif
                if((m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE))
                {
                    uint32_t currentOffset = m_usedDescriptors;
                    D3D12_DESCRIPTOR_HEAP_DESC desc = m_heapDesc;
                    while(m_usedDescriptors + descriptorCount > desc.NumDescriptors)
                    {
                        desc.NumDescriptors <<= 1;
                    }
                    SAFE_RELEASE(m_pCurrentHeap);
                    m_pDevice->CreateDescriptorHeap(&desc, IID_ARGS(&m_pCurrentHeap));
                    m_heapDesc = desc;
                    m_startHandle.mCpu = m_pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
                    m_startHandle.mGpu = m_pCurrentHeap->GetGPUDescriptorHandleForHeapStart();

                    uint32_t* rangeSized = (uint32_t*)cyber_malloc(m_usedDescriptors * sizeof(uint32_t));
                #ifdef CYBER_THREAD_SAFETY
                #else
                    uint32_t usedDescriptors = m_usedDescriptors;
                #endif

                    for(uint32_t i = 0;i < m_usedDescriptors; ++i)
                        rangeSized[i] = 1;
                    //copy new heap to pHeap
                    //TODO: copy shader-visible heap may slow
                    m_pDevice->CopyDescriptors(
                        1, &m_startHandle.mCpu, &usedDescriptors, m_usedDescriptors, m_pHandles, rangeSized, m_heapDesc.Type);
                    D3D12_CPU_DESCRIPTOR_HANDLE* pNewHandles = 
                        (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(m_heapDesc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                    memcpy(pNewHandles, m_pHandles, m_usedDescriptors * sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                    cyber_free(m_pHandles);
                    m_pHandles = pNewHandles;
                }
                else if(m_freeList.size() >= descriptorCount)
                {
                    if(descriptorCount == 1)
                    {
                        DescriptorHandle ret = m_freeList.back();
                        m_freeList.pop_back();
                        return ret;
                    }

                    // search for continuous free items in the list
                    uint32_t freeCount = 1;
                    for(size_t i = m_freeList.size() - 1; i > 0; --i)
                    {
                        size_t index = i - 1;
                        DescriptorHandle DescHandle = m_freeList[index];
                        if(DescHandle.mCpu.ptr + m_descriptorSize == m_freeList[i].mCpu.ptr)
                        {
                            ++freeCount;
                        }
                        else 
                        {
                            freeCount = 1;
                        }

                        if(freeCount == descriptorCount)
                        {
                            m_freeList.erase(m_freeList.begin() + index, m_freeList.begin() + index + descriptorCount);
                            return DescHandle;
                        }
                    }
                }
            }
            #ifdef CYBER_THREAD_SAFETY
            #else
                uint32_t usedDescriptors = m_usedDescriptors = m_usedDescriptors + descriptorCount;
            #endif
            cyber_check(usedDescriptors + descriptorCount <= m_heapDesc.NumDescriptors);
            DescriptorHandle ret = {
                {m_startHandle.mCpu.ptr + usedDescriptors * m_descriptorSize},
                {m_startHandle.mGpu.ptr + usedDescriptors * m_descriptorSize},
            };
            return ret;
        }

        DescriptorHandle DescriptorHeap_D3D12::get_slot_handle(uint32_t slot_index)
        {
            DescriptorHandle handle = {
                {m_startHandle.mCpu.ptr + slot_index * m_descriptorSize},
                {m_startHandle.mGpu.ptr + slot_index * m_descriptorSize}
            };
            return handle;
        }

        void DescriptorHeap_D3D12::free_descriptor_handles(const DescriptorHandle& handle, uint32_t descriptorCount)
        {
            cyber_assert((m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) == 0, "Descriptor Handle flag should not be shader visible!");

            for(uint32_t i = 0; i < descriptorCount; ++i)
            {
                DECLARE_ZERO(DescriptorHandle, free);
                
                free.mCpu = {handle.mCpu.ptr + i * m_descriptorSize};
                free.mGpu = { D3D12_GPU_VIRTUAL_ADDRESS_NULL};
                m_freeList.push_back(free);
            }
        }

        void DescriptorHeap_D3D12::copy_descriptor_handle(const D3D12_CPU_DESCRIPTOR_HANDLE& srcHandle, const uint64_t& dstHandle, uint32_t index)
        {
            // fill dest heap
            m_pHandles[(dstHandle / m_descriptorSize) + index] = srcHandle;
            // copy
            m_pDevice->CopyDescriptorsSimple(1, {m_startHandle.mCpu.ptr + dstHandle + (index * m_descriptorSize)}, srcHandle, m_type);
        }

        void DescriptorHeap_D3D12::create_descriptor_heap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc, struct DescriptorHeap_D3D12** destHeap)
        {
            uint32_t numDesciptors = desc.NumDescriptors;
            DescriptorHeap_D3D12* heap = (DescriptorHeap_D3D12*)cyber_calloc(1, sizeof(*heap));
            // TODO thread safety

            heap->m_pDevice = device;

            // Keep 32 aligned for easy remove
            //numDesciptors = cyber_round_up(numDesciptors, 32);

            D3D12_DESCRIPTOR_HEAP_DESC Desc = desc;
            Desc.NumDescriptors = numDesciptors;
            heap->m_heapDesc = Desc;

            if(!SUCCEEDED(device->CreateDescriptorHeap(&Desc, IID_ARGS(&heap->m_pCurrentHeap))))
            {
                cyber_assert(false, "DescriptorHeap Create Failed!");
            }
            
            heap->m_startHandle.mCpu = heap->m_pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
            if(heap->m_heapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            {
                heap->m_startHandle.mGpu = heap->m_pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
            }
            heap->m_descriptorSize = device->GetDescriptorHandleIncrementSize(heap->m_heapDesc.Type);
            if(Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            {
                heap->m_pHandles = (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
            }

            *destHeap = heap;
        }

        void DescriptorHeap_D3D12::free()
        {
            SAFE_RELEASE(m_pCurrentHeap);

            m_freeList.~vector();

            cyber_free(m_pHandles);
            cyber_free(this);
            delete this;
        }
    }
}