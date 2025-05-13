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
    using TDeviceContextBase = DeviceContextBase<EngineD3D12ImplTraits>;
    using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

    DeviceContext_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const DeviceContextDesc& desc);

    CommandContext& get_command_context()
    {
        return *command_context;
    }

    virtual void transition_resource_state(const ResourceBarrierDesc& barrierDesc) override final;

    virtual void reset_command_pool(ICommandPool* pool) override;
    virtual void free_command_pool(ICommandPool* pool) override;
    virtual void free_command_buffer(ICommandBuffer* commandBuffer) override;

    virtual void cmd_begin(ICommandBuffer* commandBuffer) override;
    virtual void cmd_end(ICommandBuffer* commandBuffer) override;
    virtual void cmd_resource_barrier(const ResourceBarrierDesc& barrierDesc) override;

    virtual void cmd_begin_render_pass(const BeginRenderPassAttribs& beginRenderPassDesc) override;
    virtual void cmd_next_sub_pass() override;
    virtual void cmd_end_render_pass() override;
    virtual void render_encoder_bind_descriptor_set(IDescriptorSet* descriptorSet) override;
    virtual void render_encoder_set_viewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
    virtual void render_encoder_set_scissor( uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    virtual void render_encoder_bind_pipeline( IRenderPipeline* pipeline) override;
    virtual void render_encoder_bind_vertex_buffer(uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets) override;
    virtual void render_encoder_bind_index_buffer(IBuffer* buffer, uint32_t index_stride, uint64_t offset) override;
    virtual void render_encoder_push_constants(IRootSignature* rs, const char8_t* name, const void* data) override;
    virtual void render_encoder_draw(uint32_t vertex_count, uint32_t first_vertex) override;
    virtual void render_encoder_draw_instanced(uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) override;
    virtual void render_encoder_draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex) override;
    virtual void render_encoder_draw_indexed_instanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) override;

    void commit_subpass_rendertargets(RenderPassEncoder* encoder);
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