#pragma once

#include "render_device_d3d12.h"
#include "d3d12_pool_allocator.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class D3D12DefaultBufferAllocator
{
public:
    D3D12DefaultBufferAllocator(RenderDevice_D3D12_Impl* device);
    virtual ~D3D12DefaultBufferAllocator();

    
    bool alloc_default_resource(D3D12_HEAP_TYPE heap_type, const D3D12_RESOURCE_DESC& resource_desc, D3D12_RESOURCE_STATES create_state, uint32_t alignment, Buffer_D3D12_Impl* buffer);
private:
    RenderDevice_D3D12_Impl* device;

    eastl::vector<D3D12PoolAllocator*> pool_allocators;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE

