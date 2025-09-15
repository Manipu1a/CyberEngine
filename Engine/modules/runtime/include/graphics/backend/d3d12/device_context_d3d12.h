#pragma once
#include "graphics/interface/device_context.h"
#include "command_context.h"
#include "dynamic_heap_d3d12.hpp"
#include "engine_impl_traits_d3d12.hpp"
#include "state_cache_d3d12.h"

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

    DeviceContext_D3D12_Impl(RenderDeviceImplType* device, const DeviceContextDesc& desc);

    CommandContext& get_command_context()
    {
        return *curr_command_context;
    }

    virtual void transition_resource_state(const ResourceBarrierDesc& barrierDesc) override final;

    virtual void cmd_begin() override;
    virtual void cmd_end() override;
    virtual void cmd_resource_barrier(ITexture* texture, GRAPHICS_RESOURCE_STATE srcState, GRAPHICS_RESOURCE_STATE dstState) override;
    virtual void cmd_resource_barrier(IBuffer* buffer, GRAPHICS_RESOURCE_STATE srcState, GRAPHICS_RESOURCE_STATE dstState) override;
    virtual void cmd_resource_barrier(const ResourceBarrierDesc& barrierDesc) override;

    virtual void flush() override;
    virtual void finish_frame() override;
    virtual void set_render_target(uint32_t numRenderTargets, ITexture_View* renderTargets[], ITexture_View* depthTarget) override;

    virtual void cmd_begin_render_pass(const BeginRenderPassAttribs& beginRenderPassDesc) override;
    virtual void cmd_next_sub_pass() override;
    virtual void cmd_end_render_pass() override;
    virtual void transition_subpass_attachments(uint32_t subpass_index) override;
    virtual void render_encoder_bind_descriptor_set(IDescriptorSet* descriptorSet) override;
    virtual void render_encoder_set_viewport(uint32_t num_viewport, const Viewport* vps) override;
    virtual void render_encoder_set_scissor(uint32_t num_rects, const Rect* rect) override;
    virtual void render_encoder_set_blend_factor(const float* blend_factor) override;
    virtual void render_encoder_bind_pipeline( IRenderPipeline* pipeline) override;
    virtual void render_encoder_bind_vertex_buffer(uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint64_t* offsets) override;
    virtual void render_encoder_bind_index_buffer(IBuffer* buffer, uint32_t index_stride, uint64_t offset) override;
    virtual void render_encoder_push_constants(IRootSignature* rs, const char8_t* name, const void* data) override;
    virtual void render_encoder_draw(uint32_t vertex_count, uint32_t first_vertex) override;
    virtual void render_encoder_draw_instanced(uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) override;
    virtual void render_encoder_draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex) override;
    virtual void render_encoder_draw_indexed_instanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) override;

    virtual void prepare_for_rendering() override;
    
    virtual void create_render_pass(const RenderPassDesc& renderPassDesc, IRenderPass** render_pass) override;
    
    virtual void set_shader_resource_view(SHADER_STAGE stage, uint32_t binding, ITexture_View* textureView) override;
    virtual void set_constant_buffer_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer) override;
    virtual void set_unordered_access_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer) override;
    virtual void set_root_constant_buffer_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer) override;

    void set_srv_table(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, ShaderResourceViewCache& srv_cache, uint32_t slots_need, const D3D12_GPU_DESCRIPTOR_HANDLE& bind_descriptor);
    void set_cbv_table(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, ConstantBufferViewCache& cbv_cache, uint32_t slots_need, const D3D12_GPU_DESCRIPTOR_HANDLE& bind_descriptor);
    void set_uav_table(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, UnorderedAccessViewCache& uav_cache, uint32_t slots_need, const D3D12_GPU_DESCRIPTOR_HANDLE& bind_descriptor);
    
    void set_root_cbv(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, ConstantBufferCache& cbv_cache, uint16_t slots_need);

    void commit_subpass_rendertargets();

    void request_command_context();

    //todo: move to device context
    Dynamic_Allocation_D3D12 allocate_dynamic_memory(uint64_t size_in_bytes, uint64_t alignment);
                
    //struct DescriptorHeap_D3D12* get_bound_heap(uint32_t index) const { return m_pBoundHeaps[index]; }
    StateCache_D3D12& get_state_cache() { return state_cache; }

    void set_bound_heap(uint32_t index, struct DescriptorHeap_D3D12* heap);

    void commit_bound_heaps();
private:
    struct State
    {
        size_t num_command = 0;

    } state;

    void flush(bool request_new_command_context, uint32_t num_command_lists = 0, ICommandBuffer** command_lists = nullptr);
    void update_buffer_region(Buffer_D3D12_Impl* buffer, Dynamic_Allocation_D3D12& allocation, uint64_t dst_offset, uint64_t num_bytes, GRAPHICS_RESOUCE_STATE_TRANSTION_MODE transition_mode);
    void transition_or_verify_buffer_state(CommandContext& cmd_ctx, Buffer_D3D12_Impl& buffer, GRAPHICS_RESOUCE_STATE_TRANSTION_MODE transition_mode, GRAPHICS_RESOURCE_STATE required_state);
    void reset_root_signature(PIPELINE_TYPE type, RootSignature_D3D12_Impl* rootSignature);
    
    eastl::unique_ptr<CommandContext> curr_command_context = nullptr;

    Dynamic_Memory_Manager_D3D12 m_dynamic_mem_mgr;
    Dynamic_Heap_D3D12* m_pDynamicHeap;
    
    // Cached state and shader parameters
    StateCache_D3D12 state_cache;

    uint32_t m_nodeIndex;
    D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS m_subResolveResource[GRAPHICS_MAX_MRT_COUNT];
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE