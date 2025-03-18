#include "backend/d3d12/dynamic_heap_d3d12.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

Dynamic_Page_D3D12::Dynamic_Page_D3D12(ID3D12Device* d3d12_device, uint64_t size)
{
    D3D12_HEAP_PROPERTIES heap_properties = {};
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
