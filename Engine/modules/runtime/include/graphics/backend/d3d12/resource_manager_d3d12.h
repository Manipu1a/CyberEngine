#pragma once
#include "d3d12.config.h"
#include "render_device_d3d12.h"
#include "core/debug.h"
#include "eastl/vector.h"
#include "eastl/string.h"
#include "eastl/map.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class Resource_Manager_D3D12
{
public:
    Resource_Manager_D3D12(RenderDevice_D3D12_Impl& render_device_impl);

    void initialize();

    void safe_release_resources(ID3D12Resource* resource, ID3D12CommandQueue* command_queue);

    void update();

private:
    struct Pending_Resource
    {
        ID3D12Resource* resource;
        uint64_t fence_value;
    };

    RenderDevice_D3D12_Impl& render_device;
    
    ID3D12Fence* fence;
    uint64_t current_fence_value;
    eastl::vector<Pending_Resource> pending_resources;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
