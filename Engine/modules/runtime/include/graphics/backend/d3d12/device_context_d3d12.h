#pragma once
#include "graphics/interface/device_context.h"
#include "command_context.h"
#include "dynamic_heap_d3d12.hpp"
#include "engine_impl_traits_d3d12.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

struct IDeviceContext_D3D12 : public IDeviceContext 
{
    // Interface for D3D12 device context
};

class CYBER_GRAPHICS_API DeviceContext_D3D12_Impl final : public DeviceContextBase<EngineD3D12ImplTraits>
{
public:
    using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

    DeviceContext_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const DeviceContextDesc& desc);

    CommandContext& get_command_context()
    {
        return *command_context;
    }
    

    virtual void transition_resource_state(const ResourceBarrierDesc& barrierDesc) override final;

    void request_command_context();

    //todo: move to device context
    Dynamic_Allocation_D3D12 allocate_dynamic_memory(uint64_t size_in_bytes, uint64_t alignment);

    void update_buffer_region(Buffer_D3D12_Impl* buffer, Dynamic_Allocation_D3D12& allocation, uint64_t dst_offset, uint64_t num_bytes, GRAPHICS_RESOUCE_STATE_TRANSTION_MODE transition_mode);

    void transition_or_verify_buffer_state(CommandContext& cmd_ctx, Buffer_D3D12_Impl& buffer, GRAPHICS_RESOUCE_STATE_TRANSTION_MODE transition_mode, GRAPHICS_RESOURCE_STATE required_state);
private:
    CommandContext* command_context = nullptr;

    Dynamic_Memory_Manager_D3D12 m_dynamic_mem_mgr;
    Dynamic_Heap_D3D12* m_pDynamicHeap;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE