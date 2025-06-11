#include "backend/d3d12/command_context.h"
#include "core/debug.h"
#include "backend/d3d12/d3d12_utils.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

CommandContext::CommandContext(CommandListManager& command_list_manager)
{
    command_allocator = nullptr;
    command_list = nullptr;

    command_list_manager.create_new_command_list(&command_list, &command_allocator, max_interface_version);
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

void CommandContext::reset()
{
    cyber_check(command_list != nullptr);
    cyber_check(command_allocator != nullptr);
    CHECK_HRESULT(command_allocator->Reset());
    CHECK_HRESULT(command_list->Reset(command_allocator, nullptr));
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

void CommandContext::set_descriptor_heaps(uint32_t num_descriptor_heaps, ID3D12DescriptorHeap* const *ppDescriptorHeaps)
{
    cyber_check_msg(num_descriptor_heaps > 0, "D3D12 ERROR: Number of descriptor heaps must be greater than 0!");
    cyber_check_msg(ppDescriptorHeaps != nullptr, "D3D12 ERROR: Descriptor heaps pointer is null!");

    command_list->SetDescriptorHeaps(num_descriptor_heaps, ppDescriptorHeaps);
}

void CommandContext::set_pipeline_state(ID3D12PipelineState* pipeline_state)
{
    command_list->SetPipelineState(pipeline_state);
}

void CommandContext::set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY primitive_topology)
{
    command_list->IASetPrimitiveTopology(primitive_topology);
}

void CommandContext::set_vertex_buffers(uint32_t start_slot, uint32_t buffer_count, const D3D12_VERTEX_BUFFER_VIEW* views)
{
    cyber_check_msg(start_slot < D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1, "D3D12 ERROR: Vertex buffer slot out of range!");

    command_list->IASetVertexBuffers(start_slot, buffer_count, views);
}

void CommandContext::set_index_buffer(const D3D12_INDEX_BUFFER_VIEW* view)
{
    cyber_check_msg(view != nullptr, "D3D12 ERROR: Index buffer view is null!");

    command_list->IASetIndexBuffer(view);
}

void CommandContext::draw_instanced(uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    command_list->DrawInstanced((UINT)vertex_count_per_instance, (UINT)instance_count, (UINT)first_vertex, (UINT)first_instance);
}

void CommandContext::draw_indexed_instanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t first_index, int32_t base_vertex, uint32_t first_instance)
{
    command_list->DrawIndexedInstanced((UINT)index_count_per_instance, (UINT)instance_count, (UINT)first_index, (INT)base_vertex, (UINT)first_instance);
}

void CommandContext::set_graphics_root_constants( uint32_t root_parameter_index, uint32_t num_32bit_values_to_set, const void *p_src_data, uint32_t DestOffsetIn32BitValues)
{
    command_list->SetGraphicsRoot32BitConstants(root_parameter_index, num_32bit_values_to_set, p_src_data, DestOffsetIn32BitValues);
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
        cyber_warn(false, "D3D12 WARNING: Texture Barrier with same src and dst state!");
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
    const auto& create_desc = buffer.get_create_desc();
    if(create_desc.usage == GRAPHICS_RESOURCE_USAGE_DEFAULT ||
        create_desc.usage == GRAPHICS_RESOURCE_USAGE_STAGING ||
        (create_desc.usage == GRAPHICS_RESOURCE_USAGE_DYNAMIC && create_desc.mode == BUFFER_MODE_RAW))
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

void CommandContext::set_scissor_rects(uint32_t num_scissor_rects, const D3D12_RECT* rects)
{
    command_list->RSSetScissorRects(num_scissor_rects, rects);
}

void CommandContext::set_blend_factor(const float* blend_factor)
{
    command_list->OMSetBlendFactor( blend_factor);
}

void CommandContext::set_viewport(uint32_t num_viewport, const D3D12_VIEWPORT* vps)
{
    command_list->RSSetViewports(num_viewport, vps);
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


uint64_t UpdateSubresource(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestResource, ID3D12Resource* pIntermediate,
                                    uint32_t firstSubresource, uint32_t numSubresources, uint32_t requiredSize,
                                    const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, const uint32_t* pNumRows, const uint64_t* pRowSizeInBytes,
                                    const D3D12_SUBRESOURCE_DATA* pSrcData)
{
    D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc();
    D3D12_RESOURCE_DESC DestDesc = pDestResource->GetDesc();
    if(IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || 
        IntermediateDesc.Width < requiredSize + pLayouts[0].Offset ||
        requiredSize > (size_t)-1 || 
        (DestDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && (firstSubresource != 0 || numSubresources != 1)))
    {
        return 0;
    }
    BYTE* pData;
    pIntermediate->Map(0, NULL, reinterpret_cast<void**>(&pData));
    for(uint32_t i = 0; i < numSubresources; ++i)
    {
        if(pRowSizeInBytes[i] > (size_t)-1)
        {
            return 0;
        }
        D3D12_MEMCPY_DEST DestData = {pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i]};
        MemcpySubresource(&DestData, &pSrcData[i], pRowSizeInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
    }
    pIntermediate->Unmap(0, NULL);
    if(DestDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        pCmdList->CopyBufferRegion(pDestResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
    }
    else
    {
        for(uint32_t i = 0; i < numSubresources; ++i)
        {
            D3D12_TEXTURE_COPY_LOCATION Dst = {pDestResource, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, firstSubresource + i};
            D3D12_TEXTURE_COPY_LOCATION Src = {pIntermediate, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, pLayouts[i]};
            pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        }
    }
    return requiredSize;
}

uint64_t CommandContext::update_sub_resource(ID3D12Resource* pDestResource, ID3D12Resource* pIntermediate, uint32_t firstSubresource, uint32_t numSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData)
{
    uint64_t RequiredSize = 0;
    uint64_t MemToAlloc = (sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(uint32_t)  + sizeof(uint64_t)) * numSubresources;
    
    void* pMem = cyber_malloc(MemToAlloc);
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
    uint32_t* pNumRows = reinterpret_cast<uint32_t*>(pLayouts + numSubresources);
    uint64_t* pRowSizeInBytes = reinterpret_cast<uint64_t*>(pNumRows + numSubresources);

    D3D12_RESOURCE_DESC Desc = pDestResource->GetDesc();
    ID3D12Device* pDevice = nullptr;
    pDestResource->GetDevice(IID_PPV_ARGS(&pDevice));
    pDevice->GetCopyableFootprints(&Desc, firstSubresource, numSubresources, 0, pLayouts, pNumRows, pRowSizeInBytes, &RequiredSize);

    uint64_t Res = UpdateSubresource(command_list, pDestResource, pIntermediate, firstSubresource, numSubresources, RequiredSize, pLayouts, pNumRows, pRowSizeInBytes, pSrcData);
    cyber_free(pMem);
    pDevice->Release();

    return Res;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE













