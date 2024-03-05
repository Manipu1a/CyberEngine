#include "backend/d3d12/descriptor_heap_d3d12.h"
#include "platform/memory.h"
#include "backend/d3d12/graphics_types_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        DescriptorHandle DescriptorHeap_D3D12::consume_descriptor_handles(uint32_t descriptorCount)
        {
            if(m_UsedDescriptors + descriptorCount > m_Desc.NumDescriptors)
            {
            #ifdef CYBER_THREAD_SAFETY
            #endif
                if((m_Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE))
                {
                    uint32_t currentOffset = m_UsedDescriptors;
                    D3D12_DESCRIPTOR_HEAP_DESC desc = m_Desc;
                    while(m_UsedDescriptors + descriptorCount > desc.NumDescriptors)
                    {
                        desc.NumDescriptors <<= 1;
                    }
                    SAFE_RELEASE(m_pCurrentHeap);
                    m_pDevice->CreateDescriptorHeap(&desc, IID_ARGS(&m_pCurrentHeap));
                    m_Desc = desc;
                    m_StartHandle.mCpu = m_pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
                    m_StartHandle.mGpu = m_pCurrentHeap->GetGPUDescriptorHandleForHeapStart();

                    uint32_t* rangeSized = (uint32_t*)cyber_malloc(m_UsedDescriptors * sizeof(uint32_t));
                #ifdef CYBER_THREAD_SAFETY
                #else
                    uint32_t usedDescriptors = m_UsedDescriptors;
                #endif

                    for(uint32_t i = 0;i < m_UsedDescriptors; ++i)
                        rangeSized[i] = 1;
                    //copy new heap to pHeap
                    //TODO: copy shader-visible heap may slow
                    m_pDevice->CopyDescriptors(
                        1, &m_StartHandle.mCpu, &usedDescriptors, m_UsedDescriptors, m_pHandles, rangeSized, m_Desc.Type);
                    D3D12_CPU_DESCRIPTOR_HANDLE* pNewHandles = 
                        (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(m_Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                    memcpy(pNewHandles, m_pHandles, m_UsedDescriptors * sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
                    cyber_free(m_pHandles);
                    m_pHandles = pNewHandles;
                }
                else if(m_FreeList.size() >= descriptorCount)
                {
                    if(descriptorCount == 1)
                    {
                        DescriptorHandle ret = m_FreeList.back();
                        m_FreeList.pop_back();
                        return ret;
                    }

                    // search for continuous free items in the list
                    uint32_t freeCount = 1;
                    for(size_t i = m_FreeList.size() - 1; i > 0; --i)
                    {
                        size_t index = i - 1;
                        DescriptorHandle DescHandle = m_FreeList[index];
                        if(DescHandle.mCpu.ptr + m_DescriptorSize == m_FreeList[i].mCpu.ptr)
                        {
                            ++freeCount;
                        }
                        else 
                        {
                            freeCount = 1;
                        }

                        if(freeCount == descriptorCount)
                        {
                            m_FreeList.erase(m_FreeList.begin() + index, m_FreeList.begin() + index + descriptorCount);
                            return DescHandle;
                        }
                    }
                }
            }
            #ifdef CYBER_THREAD_SAFETY
            #else
                uint32_t usedDescriptors = m_UsedDescriptors = m_UsedDescriptors + descriptorCount;
            #endif
            cyber_check(usedDescriptors + descriptorCount <= m_Desc.NumDescriptors);
            DescriptorHandle ret = {
                {m_StartHandle.mCpu.ptr + usedDescriptors * m_DescriptorSize},
                {m_StartHandle.mGpu.ptr + usedDescriptors * m_DescriptorSize},
            };
            return ret;
        }

        void DescriptorHeap_D3D12::free_descriptor_handles(const DescriptorHandle& handle, uint32_t descriptorCount)
        {
            cyber_assert((m_Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) == 0, "Descriptor Handle flag should not be shader visible!");

            for(uint32_t i = 0; i < descriptorCount; ++i)
            {
                DECLARE_ZERO(DescriptorHandle, free);
                
                free.mCpu = {handle.mCpu.ptr + i * m_DescriptorSize};
                free.mGpu = { D3D12_GPU_VIRTUAL_ADDRESS_NULL};
                m_FreeList.push_back(free);
            }
        }

        void DescriptorHeap_D3D12::copy_descriptor_handle(const D3D12_CPU_DESCRIPTOR_HANDLE& srcHandle, const uint64_t& dstHandle, uint32_t index)
        {
            // fill dest heap
            m_pHandles[(dstHandle / m_DescriptorSize) + index] = srcHandle;
            // copy
            m_pDevice->CopyDescriptorsSimple(1, {m_StartHandle.mCpu.ptr + dstHandle + (index * m_DescriptorSize)}, srcHandle, m_Type);
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
            heap->m_Desc = Desc;

            if(!SUCCEEDED(device->CreateDescriptorHeap(&Desc, IID_ARGS(&heap->m_pCurrentHeap))))
            {
                cyber_assert(false, "DescriptorHeap Create Failed!");
            }
            
            heap->m_StartHandle.mCpu = heap->m_pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
            if(heap->m_Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            {
                heap->m_StartHandle.mGpu = heap->m_pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
            }
            heap->m_DescriptorSize = device->GetDescriptorHandleIncrementSize(heap->m_Desc.Type);
            if(Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            {
                heap->m_pHandles = (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
            }

            *destHeap = heap;
        }

        void DescriptorHeap_D3D12::free()
        {
            SAFE_RELEASE(m_pCurrentHeap);

            m_FreeList.~vector();

            cyber_free(m_pHandles);
            cyber_free(this);
            delete this;
        }
    }
}