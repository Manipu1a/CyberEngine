#include "backend/d3d12/command_context.h"
#include "core/debug.h"
#include "backend/d3d12/d3d12_utils.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

CommandContext::CommandContext(CommandListManager& command_list_manager)
{
    command_list_manager.create_new_command_list(&command_list, &command_allocator, max_interface_version);

    m_pBoundHeaps[0] = nullptr;
    m_pBoundHeaps[1] = nullptr;
    m_boundHeapStartHandles[0].ptr = 0;
    m_boundHeapStartHandles[1].ptr = 0;
    m_pBoundRootSignature = nullptr;
    m_nodeIndex = 0;
}

CommandContext::~CommandContext()
{
    
}

ID3D12GraphicsCommandList* CommandContext::close(ID3D12CommandAllocator* out_command_allocator)
{

    cyber_assert(command_allocator != nullptr, "Command allocator is not initialized");
    auto hr = command_list->Close();
    cyber_assert(SUCCEEDED(hr), "Failed to close command list");

    out_command_allocator = eastl::move(command_allocator);
    return command_list;
}

void CommandContext::reset(CommandListManager& command_list_manager)
{
    cyber_check(command_list != nullptr);
    cyber_check(command_list->GetType() == command_list_manager.get_command_list_type());

    if(command_allocator != nullptr)
    {
        command_list_manager.request_allocator(&command_allocator);
        command_list->Reset(command_allocator, nullptr);
    }
}

void CommandContext::set_render_target(uint32_t num_render_targets, const D3D12_CPU_DESCRIPTOR_HANDLE *rt_descriptor,BOOL rt_single_handle_to_descriptor_range,
    const D3D12_CPU_DESCRIPTOR_HANDLE *depth_stencil_descriptor)
{
    command_list->OMSetRenderTargets(num_render_targets, rt_descriptor, rt_single_handle_to_descriptor_range, depth_stencil_descriptor);
}

void CommandContext::set_graphics_root_signature(ID3D12RootSignature* root_signature)
{
    command_list->SetGraphicsRootSignature(root_signature);
}

void CommandContext::set_compute_root_signature(ID3D12RootSignature* root_signature)
{
    command_list->SetComputeRootSignature(root_signature);
}

void CommandContext::set_graphics_root_descriptor_table(uint32_t root_parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor)
{
    command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
}

void CommandContext::transition_resource(Texture_D3D12_Impl& texture, GRAPHICS_RESOURCE_STATE new_state)
{
    transition_resource(texture, TextureBarrier{&texture, GRAPHICS_RESOURCE_STATE_UNKNOWN, new_state});
}

void CommandContext::transition_resource(Buffer_D3D12_Impl& buffer, GRAPHICS_RESOURCE_STATE new_state)
{
    transition_resource(buffer, BufferBarrier{&buffer, GRAPHICS_RESOURCE_STATE_UNKNOWN, new_state});
}

void CommandContext::transition_resource(Texture_D3D12_Impl& texture, const TextureBarrier& transition_barrier)
{
    DECLARE_ZERO(D3D12_RESOURCE_BARRIER, d3d_barrier);
    if(texture.get_new_state() == transition_barrier.dst_state)
    {
        cyber_error("D3D12 ERROR: Texture Barrier with same src and dst state!");
        return;
    }

    texture.set_old_state(texture.get_new_state());
    texture.set_new_state(transition_barrier.dst_state);

    if(transition_barrier.src_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS &&
        transition_barrier.dst_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        d3d_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        d3d_barrier.UAV.pResource = texture.get_d3d12_resource();
    }
    else
    {
        cyber_assert(transition_barrier.src_state != transition_barrier.dst_state, "D3D12 ERROR: Texture Barrier with same src and dst state!");

        d3d_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        if(transition_barrier.d3d12.begin_only)
        {
            d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
        }
        else if(transition_barrier.d3d12.end_only)
        {
            d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
        }
        d3d_barrier.Transition.pResource = texture.get_d3d12_resource();
        const auto& create_desc = texture.get_create_desc();
        d3d_barrier.Transition.Subresource = transition_barrier.subresource_barrier != 0 ?
                                         CALC_SUBRESOURCE_INDEX(transition_barrier.mip_level, transition_barrier.array_layer, 0, create_desc.m_mipLevels, create_desc.m_arraySize + 1)
                                        : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        if(transition_barrier.queue_acquire)
        {
            d3d_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        }
        else 
        {
            d3d_barrier.Transition.StateBefore = D3D12Util_TranslateResourceState(transition_barrier.src_state);
        }

        if(transition_barrier.queue_release)
        {
            d3d_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
        }
        else 
        {
            d3d_barrier.Transition.StateAfter = D3D12Util_TranslateResourceState(transition_barrier.dst_state);
        }

        if(d3d_barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON && d3d_barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_COMMON)
        {
            if(transition_barrier.src_state == GRAPHICS_RESOURCE_STATE_PRESENT || transition_barrier.dst_state == D3D12_RESOURCE_STATE_COMMON)
            {
                return;
            }
        }
    }

    command_list->ResourceBarrier(1, &d3d_barrier);
}

void CommandContext::transition_resource(Buffer_D3D12_Impl& buffer, const BufferBarrier& transition_barrier)
{
    DECLARE_ZERO(D3D12_RESOURCE_BARRIER, d3d_barrier);

    const auto& memory_usage = buffer.get_memory_usage();
    if(memory_usage == GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_ONLY ||
        memory_usage == GRAPHICS_RESOURCE_MEMORY_USAGE_GPU_TO_CPU ||
        (memory_usage == GRAPHICS_RESOURCE_MEMORY_USAGE_CPU_TO_GPU && memory_usage & GRAPHICS_RESOURCE_TYPE_RW_BUFFER))
        {
            if(transition_barrier.src_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS &&
                transition_barrier.dst_state == GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                d3d_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                d3d_barrier.UAV.pResource = buffer.get_dx_resource();
            }
            else 
            {
                cyber_assert(transition_barrier.src_state != transition_barrier.dst_state, "D3D12 ERROR: Buffer Barrier with same src and dst state!");

                d3d_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                if(transition_barrier.d3d12.begin_only)
                {
                    d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                }
                else if(transition_barrier.d3d12.end_only)
                {
                    d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                }
                else
                {
                    d3d_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                }
                d3d_barrier.Transition.pResource = buffer.get_dx_resource();
                d3d_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                if(transition_barrier.queue_acquire)
                {
                    d3d_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
                }
                else 
                {
                    d3d_barrier.Transition.StateBefore = D3D12Util_TranslateResourceState(transition_barrier.src_state);
                }

                if(transition_barrier.queue_release)
                {
                    d3d_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                }
                else 
                {
                    d3d_barrier.Transition.StateAfter = D3D12Util_TranslateResourceState(transition_barrier.dst_state);
                }

                cyber_assert(d3d_barrier.Transition.StateBefore != d3d_barrier.Transition.StateAfter, "D3D12 ERROR: Buffer Barrier with same src and dst state!");
            }
        }
        command_list->ResourceBarrier(1, &d3d_barrier);
}

void CommandContext::set_scissor_rects(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    D3D12_RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    command_list->RSSetScissorRects(1, &rect);
}

void CommandContext::set_viewport(float x, float y, float width, float height, float min_depth, float max_depth)
{
    D3D12_VIEWPORT viewport = { x, y, width, height, min_depth, max_depth };
    command_list->RSSetViewports(1, &viewport);
}

void CommandContext::begin_render_pass(uint32_t num_render_targets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC* render_targets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depth_stencil, D3D12_RENDER_PASS_FLAGS flags)
{
    static_cast<ID3D12GraphicsCommandList4*>(command_list)->BeginRenderPass(num_render_targets, render_targets, depth_stencil, flags);
}

void CommandContext::end_render_pass()
{
    #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
        static_cast<ID3D12GraphicsCommandList4*>(command_list)->EndRenderPass();
    #else
        cyber_warn(false, "ID3D12GraphicsCommandList4 is not defined!");
    #endif
}
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE













