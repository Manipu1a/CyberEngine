#include "backend/d3d12/resource_manager_d3d12.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

Resource_Manager_D3D12::Resource_Manager_D3D12(RenderDevice_D3D12_Impl& render_device_impl)
    : render_device(render_device_impl)
{
    
}

void Resource_Manager_D3D12::initialize()
{
    auto d3d12_device = render_device.GetD3D12Device();
    CHECK_HRESULT(d3d12_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    current_fence_value = 0;
}

void Resource_Manager_D3D12::safe_release_resources(ID3D12Resource* resource, ID3D12CommandQueue* command_queue)
{
    uint64_t fence_value = current_fence_value + 1;
    command_queue->Signal(fence, fence_value);

    pending_resources.emplace_back(resource, fence_value);
}

void Resource_Manager_D3D12::update()
{
    uint64_t completed_fence_value = fence->GetCompletedValue();

    auto it = pending_resources.begin();
    while(it != pending_resources.end())
    {
        if(it->fence_value <= completed_fence_value)
        {
            it->resource->Release();
            it = pending_resources.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE

