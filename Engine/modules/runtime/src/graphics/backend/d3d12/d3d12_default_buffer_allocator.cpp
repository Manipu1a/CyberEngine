#include "backend/d3d12/d3d12_default_buffer_allocator.h"

#if !defined (BUFFER_POOL_DEFAULT_SIZE)
#define BUFFER_POOL_DEFAULT_SIZE (32 * 1024 * 1024) // 32 MB
#endif


CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

D3D12DefaultBufferAllocator::D3D12DefaultBufferAllocator(RenderDevice_D3D12_Impl* indevice) : device(indevice)
{
      cyber_check(device != nullptr);
      
      // Create default pool allocators for different heap types
      pool_allocators.push_back(new D3D12PoolAllocator(device, PoolCreateAttribs::create_upload_pools(), BUFFER_POOL_DEFAULT_SIZE, 64));
}

D3D12DefaultBufferAllocator::~D3D12DefaultBufferAllocator()
{

}

void D3D12DefaultBufferAllocator::alloc_default_resource(D3D12_HEAP_TYPE heap_type, const D3D12_RESOURCE_DESC& resource_desc, D3D12_RESOURCE_STATES create_state, uint32_t alignment, Buffer_D3D12_Impl* buffer)
{
      D3D12PoolAllocator* pool_allocator = nullptr;
      for(auto& allocator : pool_allocators)
      {
            if(allocator->support_allocation(heap_type, resource_desc.Flags))
            {
                  pool_allocator = allocator;
                  break;
            }
      }

      if(pool_allocator == nullptr)
      {
            // create new one
            PoolCreateAttribs attribs;
            attribs.heap_type = heap_type;
            attribs.heap_flags = D3D12_HEAP_FLAG_NONE;
            attribs.resource_flags = resource_desc.Flags;
            attribs.initial_state = create_state;
            pool_allocators.push_back(new D3D12PoolAllocator(device, attribs, BUFFER_POOL_DEFAULT_SIZE, alignment));
      }

      pool_allocator->alloc_default_resource(heap_type, resource_desc, alignment, create_state, buffer);
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
