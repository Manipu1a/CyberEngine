#include "graphics/backend/d3d12/d3d12_pool_allocator.h"
#include "backend/d3d12/buffer_d3d12.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

D3D12PoolAllocator::D3D12PoolAllocator(RenderDevice_D3D12_Impl* device, PoolCreateAttribs attribs, uint64_t pool_size, uint32_t pool_alignment)
      : device(device), resource_heap(nullptr), pool_size(pool_size), used_memory(0), pool_alignment(pool_alignment), attribs(attribs)
{
      create_pool(attribs, pool_size, pool_alignment);
}

void D3D12PoolAllocator::create_pool(const PoolCreateAttribs& attribs, uint64_t pool_size, uint32_t pool_alignment)
{
      // Create the D3D12 heap for the pool
      D3D12_HEAP_DESC heap_desc = {};
      heap_desc.SizeInBytes = pool_size;
      heap_desc.Properties.Type = attribs.heap_type;
      heap_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      heap_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      heap_desc.Alignment = 0;
      heap_desc.Flags = attribs.heap_flags;
      
      HRESULT hr = device->GetD3D12Device()->CreateHeap(
          &heap_desc, IID_PPV_ARGS(&resource_heap));
      cyber_assert(SUCCEEDED(hr), "Failed to create D3D12 heap for pool allocator");
}

bool D3D12PoolAllocator::support_allocation(D3D12_HEAP_TYPE heap_type, D3D12_RESOURCE_FLAGS resource_flags) const
{
      // Check if the heap type and flags are supported by this pool allocator
      return (heap_type == attribs.heap_type);
}

bool D3D12PoolAllocator::try_allocate(uint32_t in_size_bytes, uint32_t in_alignment, PoolAllocationData& allocation_data)
{
    uint32_t aligned_size = cyber_round_up(in_size_bytes, in_alignment);
    if(pool_size - used_memory > aligned_size)
    {
            uint64_t start_offset = used_memory;
            used_memory += aligned_size;
            allocation_data.offset = start_offset;
            allocation_data.size = aligned_size;
            allocation_data.heap = resource_heap;
            return true;
    }
    return false;
}

bool D3D12PoolAllocator::alloc_default_resource(D3D12_HEAP_TYPE heap_type, const D3D12_RESOURCE_DESC resource_desc, uint32_t alignment,
                        D3D12_RESOURCE_STATES initial_state, Buffer_D3D12_Impl* allocation_buffer)
{
      return alloc_resouce(heap_type, resource_desc, resource_desc.Width, alignment, initial_state, allocation_buffer);
}

bool D3D12PoolAllocator::alloc_resouce(D3D12_HEAP_TYPE heap_type, const D3D12_RESOURCE_DESC resource_desc, uint64_t size, uint32_t in_alignment,
                        D3D12_RESOURCE_STATES initial_state, Buffer_D3D12_Impl* allocation_buffer)
{
      bool use_placed = size < pool_size && in_alignment <= pool_alignment;

      if(use_placed)
      {
            uint32_t alignment = pool_alignment;
            D3D12_RESOURCE_DESC placed_resource_desc = resource_desc;
            placed_resource_desc.Alignment = alignment;
            ID3D12Resource* resource = create_placed_resouce(placed_resource_desc, initial_state);
            if(resource)
            {
                  allocation_buffer->set_dx_resource(resource);
                  return true;
            }
      }

      return false;
}

ID3D12Resource* D3D12PoolAllocator::create_placed_resouce(D3D12_RESOURCE_DESC resource_desc, D3D12_RESOURCE_STATES initial_state)
{
      PoolAllocationData allocation_data;
      if(!try_allocate(resource_desc.Width, pool_alignment, allocation_data))
      {
            cyber_check(false);
      }
      if(allocation_data.heap)
      {
            ID3D12Resource* resource = nullptr;
            HRESULT hr = device->GetD3D12Device()->CreatePlacedResource(
                  allocation_data.heap, allocation_data.offset, &resource_desc, initial_state, nullptr,
                  IID_PPV_ARGS(&resource));
            cyber_assert(SUCCEEDED(hr), "Failed to create placed resource");
            return resource;
      }
      
      cyber_error("Failed to allocate resource in D3D12 pool allocator");
      return nullptr;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE