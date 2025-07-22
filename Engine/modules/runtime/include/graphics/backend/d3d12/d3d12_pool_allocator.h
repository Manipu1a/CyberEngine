#pragma once

#include "graphics_types_d3d12.h"
#include "render_device_d3d12.h"
#include "d3d12.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

struct PoolCreateAttribs
{
    static PoolCreateAttribs create_default_pools()
    {
        PoolCreateAttribs attribs;
        attribs.heap_type = D3D12_HEAP_TYPE_DEFAULT;
        attribs.heap_flags = D3D12_HEAP_FLAG_NONE;
        attribs.resource_flags = D3D12_RESOURCE_FLAG_NONE;
        attribs.initial_state = D3D12_RESOURCE_STATE_COMMON;
        return attribs;
    }
    static PoolCreateAttribs create_upload_pools()
    {
      PoolCreateAttribs attribs;
      attribs.heap_type = D3D12_HEAP_TYPE_UPLOAD;
      attribs.heap_flags = D3D12_HEAP_FLAG_NONE;
      attribs.resource_flags = D3D12_RESOURCE_FLAG_NONE;
      attribs.initial_state = D3D12_RESOURCE_STATE_GENERIC_READ;
      return attribs;
    }
    static PoolCreateAttribs create_readback_pools()
    {
      PoolCreateAttribs attribs;
      attribs.heap_type = D3D12_HEAP_TYPE_READBACK;
      attribs.heap_flags = D3D12_HEAP_FLAG_NONE;
      attribs.resource_flags = D3D12_RESOURCE_FLAG_NONE;
      attribs.initial_state = D3D12_RESOURCE_STATE_COPY_DEST;
      return attribs;
    }


    bool operator==( const PoolCreateAttribs& other ) const
    {
        return heap_type == other.heap_type &&
               heap_flags == other.heap_flags &&
               resource_flags == other.resource_flags &&
               initial_state == other.initial_state;
    }
    D3D12_HEAP_TYPE heap_type;
    D3D12_HEAP_FLAGS heap_flags;
    D3D12_RESOURCE_FLAGS resource_flags;
    D3D12_RESOURCE_STATES initial_state;
};

struct PoolAllocationData
{
    ID3D12Heap* heap;
    ID3D12Resource* buffer;
    uint64_t offset;
    uint64_t size;
    void* cpu_address;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_address;
};

class D3D12PoolAllocator
{
public:
      D3D12PoolAllocator(RenderDevice_D3D12_Impl* device, PoolCreateAttribs attribs, uint64_t pool_size, uint32_t pool_alignment);
      ~D3D12PoolAllocator();
      
      void create_pool(const PoolCreateAttribs& attribs, uint64_t pool_size, uint32_t pool_alignment);
      bool support_allocation(D3D12_HEAP_TYPE heap_type, D3D12_RESOURCE_FLAGS resource_flags) const;
      bool try_allocate(uint32_t in_size_bytes, uint32_t in_alignment, PoolAllocationData& allocation_data);

      bool alloc_default_resource(D3D12_HEAP_TYPE heap_type, const D3D12_RESOURCE_DESC resource_desc, uint32_t alignment,
                        D3D12_RESOURCE_STATES initial_state, Buffer_D3D12_Impl* allocation_buffer);
      
      bool alloc_resouce(D3D12_HEAP_TYPE heap_type, const D3D12_RESOURCE_DESC resource_desc, uint64_t size, uint32_t alignment,
                        D3D12_RESOURCE_STATES initial_state, Buffer_D3D12_Impl* allocation_buffer);

private:
      ID3D12Resource* create_placed_resouce(D3D12_RESOURCE_DESC resource_desc, D3D12_RESOURCE_STATES initial_state);

      RenderDevice_D3D12_Impl* device;
      ID3D12Heap* resource_heap;
      uint64_t pool_size;
      uint64_t used_memory;
      uint32_t pool_alignment;
      PoolCreateAttribs attribs;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE