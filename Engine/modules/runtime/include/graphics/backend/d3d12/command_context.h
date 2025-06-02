#pragma once
#include "platform/configure.h"
#include "command_list_manager.h"
#include "interface/graphics_types.h"
#include "graphics/backend/d3d12/texture_d3d12.h"
#include "graphics/backend/d3d12/buffer_d3d12.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class CommandContext
{
public:
    explicit CommandContext(CommandListManager& command_list_manager);

    CommandContext(const CommandContext&) = delete;
    CommandContext& operator=(const CommandContext&) = delete;
    CommandContext(CommandContext&&) = delete;
    CommandContext& operator=(CommandContext&&) = delete;

    ~CommandContext();

    ID3D12GraphicsCommandList* close(ID3D12CommandAllocator* out_command_allocator);
    void reset(CommandListManager& command_list_manager);
    
    ID3D12GraphicsCommandList* get_command_list() const { return command_list; }
    D3D12_COMMAND_LIST_TYPE get_command_list_type() const { return command_list->GetType(); }

public:
    void set_render_target(uint32_t num_render_targets,
        const D3D12_CPU_DESCRIPTOR_HANDLE *rt_descriptor,
        BOOL rt_single_handle_to_descriptor_range,
        const D3D12_CPU_DESCRIPTOR_HANDLE *depth_stencil_descriptor);

    // Root Signature
    void set_graphics_root_signature(ID3D12RootSignature* root_signature);
    void set_compute_root_signature(ID3D12RootSignature* root_signature);
    // Root Parameters
    void set_graphics_root_descriptor_table(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor);
    void set_graphics_root_constants( uint32_t root_parameter_index, uint32_t num_32bit_values_to_set, const void *p_src_data, uint32_t DestOffsetIn32BitValues);
    
    void set_descriptor_heaps(uint32_t num_descriptor_heaps, ID3D12DescriptorHeap* const *ppDescriptorHeaps);
    void set_pipeline_state(ID3D12PipelineState* pipeline_state);

    // Input Assembler
    void set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY primitive_topology);
    void set_vertex_buffers(uint32_t start_slot, uint32_t buffer_count, const D3D12_VERTEX_BUFFER_VIEW* views);
    void set_index_buffer(const D3D12_INDEX_BUFFER_VIEW* view);

    void draw_instanced(uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
    void draw_indexed_instanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t first_index, int32_t base_vertex, uint32_t first_instance);

    void transition_resource(Texture_D3D12_Impl& texture, GRAPHICS_RESOURCE_STATE new_state);
    void transition_resource(Buffer_D3D12_Impl& buffer, GRAPHICS_RESOURCE_STATE new_state);

    void transition_resource(Texture_D3D12_Impl& texture, const TextureBarrier& barrier);
    void transition_resource(Buffer_D3D12_Impl& buffer, const BufferBarrier& barrier);
    void set_scissor_rects(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void set_blend_factor(const float* blend_factor);
    void set_viewport(uint32_t num_viewport, const D3D12_VIEWPORT* vps);
    
    void begin_render_pass(uint32_t num_render_targets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC* render_targets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depth_stencil, D3D12_RENDER_PASS_FLAGS flags);
    void end_render_pass();

    uint64_t update_sub_resource(ID3D12Resource* pDestResource, ID3D12Resource* pIntermediate, uint32_t firstSubresource, uint32_t numSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData);
    
    void reset();
private:
    ID3D12GraphicsCommandList* command_list;
    ID3D12CommandAllocator* command_allocator;

    uint32_t max_interface_version;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
