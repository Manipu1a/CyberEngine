#include "graphics/backend/d3d12/device_context_d3d12.h"
#include "graphics/backend/d3d12//render_device_d3d12.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

DeviceContext_D3D12_Impl::DeviceContext_D3D12_Impl(RenderDevice_D3D12_Impl* device, const DeviceContextDesc& desc)
    : DeviceContextBase<EngineD3D12ImplTraits>(device, desc), m_dynamic_mem_mgr{*device, 1, 1024 * 1024}
{
    m_pDynamicHeap = cyber_new<Dynamic_Heap_D3D12>(m_dynamic_mem_mgr, "DynamicHeap", 1024 * 1024);

    request_command_context();
}

void DeviceContext_D3D12_Impl::transition_resource_state(const ResourceBarrierDesc& barrierDesc)
{
    for(uint32_t i = 0;i < barrierDesc.buffer_barrier_count; i++)
    {
        const BufferBarrier& buffer_barrier = barrierDesc.buffer_barriers[i];
        Buffer_D3D12_Impl* buffer = static_cast<Buffer_D3D12_Impl*>(buffer_barrier.buffer);
        command_context->transition_resource(*buffer, buffer_barrier);
    }

    for(uint32_t i = 0;i < barrierDesc.texture_barrier_count; i++)
    {
        const TextureBarrier& texture_barrier = barrierDesc.texture_barriers[i];
        Texture_D3D12_Impl* texture = static_cast<Texture_D3D12_Impl*>(texture_barrier.texture);
        command_context->transition_resource(*texture, texture_barrier);
    }
}

void DeviceContext_D3D12_Impl::request_command_context()
{
    command_context = render_device->allocate_command_context(get_command_queue_id());
}


Dynamic_Allocation_D3D12 DeviceContext_D3D12_Impl::allocate_dynamic_memory(uint64_t size_in_bytes, uint64_t alignment)
{
    return m_pDynamicHeap->allocate(size_in_bytes, alignment);
}

void DeviceContext_D3D12_Impl::transition_or_verify_buffer_state(CommandContext& cmd_ctx, Buffer_D3D12_Impl& buffer, GRAPHICS_RESOUCE_STATE_TRANSTION_MODE transition_mode, GRAPHICS_RESOURCE_STATE required_state)
{
    if(transition_mode == GRAPH_RESOURCE_STATE_TRANSITION_MODE_TRANSITION)
    {
        if(buffer.is_known_state())
        {
            cmd_ctx.transition_resource(buffer, required_state);
        }
    }
    else if(transition_mode == GRAPH_RESOURCE_STATE_TRANSITION_MODE_VERIFY)
    {
        
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE