#include "graphics/backend/d3d12/device_context_d3d12.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/texture_d3d12.h"
#include "graphics/backend/d3d12/texture_view_d3d12.h"
#include "graphics/backend/d3d12/buffer_d3d12.h"
#include "graphics/backend/d3d12/swap_chain_d3d12.h"
#include "graphics/backend/d3d12/fence_d3d12.h"
#include "graphics/backend/d3d12/adapter_d3d12.h"
#include "graphics/backend/d3d12/instance_d3d12.h"
#include "graphics/backend/d3d12/command_buffer_d3d12.h"
#include "graphics/backend/d3d12/frame_buffer_d3d12.h"
#include "graphics/backend/d3d12/query_pool_d3d12.h"
#include "graphics/backend/d3d12/command_queue_d3d12.h"
#include "graphics/backend/d3d12/adapter_d3d12.h"
#include "graphics/backend/d3d12/descriptor_set_d3d12.h"
#include "graphics/backend/d3d12/semaphore_d3d12.h"
#include "graphics/backend/d3d12/render_pipeline_d3d12.h"
#include "graphics/backend/d3d12/root_signature_d3d12.h"
#include "graphics/backend/d3d12/sampler_d3d12.h"
#include "graphics/backend/d3d12/shader_library_d3d12.h"
#include "graphics/backend/d3d12/shader_reflection_d3d12.h"
#include "graphics/backend/d3d12/render_pass_d3d12.h"
#include "graphics/backend/d3d12/command_context.h"
#include "graphics/backend/d3d12/descriptor_heap_d3d12.h"
#include "graphics/backend/d3d12/d3d12_utils.h"
#include "common/template.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

DeviceContext_D3D12_Impl::DeviceContext_D3D12_Impl(RenderDeviceImplType* device, const DeviceContextDesc& desc)
    : TDeviceContextBase{device, desc},
     m_dynamic_mem_mgr{*device, 1, 1024 * 1024}
{
    m_pDynamicHeap = cyber_new<Dynamic_Heap_D3D12>(m_dynamic_mem_mgr, "DynamicHeap", 1024 * 1024);

    request_command_context();
}

void DeviceContext_D3D12_Impl::cmd_begin()
{
    //request_command_context();
}

void DeviceContext_D3D12_Impl::cmd_end()
{
    ID3D12CommandAllocator* command_allocator = nullptr;
    //curr_command_context->close(command_allocator);

    //todo release command allocator
    //m_pBoundRootSignature = nullptr;

    // reset state cache
    state_cache.bound_root_signature = nullptr;
}

void DeviceContext_D3D12_Impl::cmd_resource_barrier(const ResourceBarrierDesc& barrierDesc)
{
    transition_resource_state(barrierDesc);
}

void DeviceContext_D3D12_Impl::set_render_target(uint32_t numRenderTargets, ITexture_View* renderTargets[], ITexture_View* depthTarget)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

    for(uint32_t i = 0; i < numRenderTargets; i++)
    {
        RenderObject::Texture_View_D3D12_Impl * rtv = static_cast<RenderObject::Texture_View_D3D12_Impl *>(renderTargets[i]);
        rtvHandles[i] = rtv->m_rtvDsvDescriptorHandle;
    }

    if(depthTarget)
    {
        RenderObject::Texture_View_D3D12_Impl * dsv = static_cast<RenderObject::Texture_View_D3D12_Impl *>(depthTarget);
        dsvHandle = dsv->m_rtvDsvDescriptorHandle;
    }

    curr_command_context->set_render_target(numRenderTargets, rtvHandles, false,depthTarget ? &dsvHandle : nullptr);
    ++state.num_command;
}


void DeviceContext_D3D12_Impl::cmd_begin_render_pass(const BeginRenderPassAttribs& beginRenderPassDesc)
{
    TDeviceContextBase::cmd_begin_render_pass( beginRenderPassDesc);

    commit_subpass_rendertargets();
}

void DeviceContext_D3D12_Impl::cmd_next_sub_pass()
{
    //curr_command_context->end_render_pass();
    ++state.num_command;
    TDeviceContextBase::cmd_next_sub_pass();

    if( m_pRenderPass == nullptr || m_pFrameBuffer == nullptr)
    {
        cyber_assert(false, "RenderPass or FrameBuffer is nullptr!");
    }
    commit_subpass_rendertargets();
}

void DeviceContext_D3D12_Impl::cmd_end_render_pass()
{
    curr_command_context->end_render_pass();
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_bind_descriptor_set(IDescriptorSet* descriptorSet)
{
    const RenderObject::DescriptorSet_D3D12_Impl* Set = static_cast<const RenderObject::DescriptorSet_D3D12_Impl*>(descriptorSet);
    RenderObject::RootSignature_D3D12_Impl* RS = static_cast<RenderObject::RootSignature_D3D12_Impl*>(Set->get_root_signature());

    cyber_check(RS);
    reset_root_signature(PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);

    
    if(Set->cbv_srv_uav_handle != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
    {
        curr_command_context->set_graphics_root_descriptor_table(Set->get_set_index(), {state_cache.bound_heap_start_handles[0].ptr + Set->cbv_srv_uav_handle});
        ++state.num_command;
    }
    else if(Set->sampler_handle != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
    {
        curr_command_context->set_graphics_root_descriptor_table(Set->get_set_index(), {state_cache.bound_heap_start_handles[0].ptr + Set->sampler_handle});
        ++state.num_command;
    }

    if(Set->root_constant_address != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
    {
        curr_command_context->set_graphics_root_constant_buffer_view(RS->root_parameter_index, Set->root_constant_address);
        ++state.num_command;
    }
}

void DeviceContext_D3D12_Impl::render_encoder_set_viewport(uint32_t num_viewport, const Viewport* vps)
{
    D3D12_VIEWPORT views[MAX_VIEWPORTS];

    for(uint32_t i = 0; i < num_viewport; ++i)
    {
        const Viewport& vp = vps[i];
        views[i].TopLeftX = vp.top_left_x;
        views[i].TopLeftY = vp.top_left_y;
        views[i].Width = vp.width;
        views[i].Height = vp.height;
        views[i].MinDepth = vp.min_depth;
        views[i].MaxDepth = vp.max_depth;
    }
    curr_command_context->set_viewport(num_viewport, views);
    ++state.num_command;

}

void DeviceContext_D3D12_Impl::render_encoder_set_scissor( uint32_t num_rects, const Rect* rect )
{
    D3D12_RECT rects[MAX_RECTS];
    for(uint32_t i = 0; i < num_rects; ++i)
    {
        const Rect& r = rect[i];
        rects[i].left = r.left;
        rects[i].top = r.top;
        rects[i].right = r.right;
        rects[i].bottom = r.bottom;
    }
    curr_command_context->set_scissor_rects(num_rects, rects);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_set_blend_factor(const float* blend_factor)
{
    curr_command_context->set_blend_factor(blend_factor);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_bind_pipeline( IRenderPipeline* pipeline)
{
    RenderObject::RenderPipeline_D3D12_Impl* Pipeline = static_cast<RenderObject::RenderPipeline_D3D12_Impl*>(pipeline);
    reset_root_signature(PIPELINE_TYPE_GRAPHICS, Pipeline->pDxRootSignature);
    curr_command_context->set_primitive_topology(Pipeline->mPrimitiveTopologyType);
    ++state.num_command;
    curr_command_context->set_pipeline_state(Pipeline->pDxPipelineState);
    ++state.num_command;

    // record shader data
    if(Pipeline->graphics_pipeline_data.desc.vertex_shader)
    {
        state_cache.set_new_shader_data(SHADER_STAGE::SHADER_STAGE_VERT, Pipeline->graphics_pipeline_data.desc.vertex_shader->m_library->get_entry_reflection(0)->get_shader_register_count());
    }
    
    if(Pipeline->graphics_pipeline_data.desc.fragment_shader)
    {
        state_cache.set_new_shader_data(SHADER_STAGE::SHADER_STAGE_FRAG, Pipeline->graphics_pipeline_data.desc.fragment_shader->m_library->get_entry_reflection(0)->get_shader_register_count());
    }
}

void DeviceContext_D3D12_Impl::render_encoder_bind_vertex_buffer(uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint64_t* offsets)
{
    DECLARE_ZERO(D3D12_VERTEX_BUFFER_VIEW, views[GRAPHICS_MAX_VERTEX_ATTRIBUTES]);
    for(uint32_t i = 0;i < buffer_count; ++i)
    {
        const RenderObject::Buffer_D3D12_Impl* Buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(buffers[i]);
        cyber_check(Buffer->get_gpu_address(0) != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN);

        views[i].BufferLocation = Buffer->get_gpu_address(0) + (offsets ? offsets[i] : 0);
        views[i].SizeInBytes = (UINT)(Buffer->get_size() - (offsets ? offsets[i] : 0));
        views[i].StrideInBytes = (UINT)strides[i];  
    }
    curr_command_context->set_vertex_buffers(0, buffer_count, views);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_bind_index_buffer(IBuffer* buffer, uint32_t index_stride, uint64_t offset)
{
    const RenderObject::Buffer_D3D12_Impl* Buffer = static_cast<RenderObject::Buffer_D3D12_Impl*>(buffer);

    DECLARE_ZERO(D3D12_INDEX_BUFFER_VIEW, view);
    view.BufferLocation = Buffer->get_gpu_address(0) + offset;
    view.SizeInBytes = (UINT)(Buffer->get_size() - offset);
    view.Format = index_stride == sizeof(uint16_t) ? DXGI_FORMAT_R16_UINT : ((index_stride == sizeof(uint8_t) ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32_UINT));
    curr_command_context->set_index_buffer(&view);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::reset_root_signature(PIPELINE_TYPE type, ID3D12RootSignature* rootSignature)
{
    if(state_cache.bound_root_signature != rootSignature)
    {
        state_cache.bound_root_signature = rootSignature;

        if(type == PIPELINE_TYPE_GRAPHICS)
            curr_command_context->set_graphics_root_signature(rootSignature);
        else
            curr_command_context->set_compute_root_signature(rootSignature);
        
        ++state.num_command;
    }
}

void DeviceContext_D3D12_Impl::render_encoder_push_constants(IRootSignature* rs, const char8_t* name, const void* data)
{
    RootSignature_D3D12_Impl* RS = static_cast<RootSignature_D3D12_Impl*>(rs);
    reset_root_signature(PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
    curr_command_context->set_graphics_root_constants(RS->root_parameter_index, RS->root_constant_parameter.Constants.Num32BitValues, data, 0);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_draw(uint32_t vertex_count, uint32_t first_vertex)
{
    curr_command_context->draw_instanced(vertex_count, 1, first_vertex, 0);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_draw_instanced(uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    curr_command_context->draw_instanced(vertex_count, instance_count, first_vertex, first_instance);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    curr_command_context->draw_indexed_instanced(index_count, 1, first_index, first_vertex, 0);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::render_encoder_draw_indexed_instanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    curr_command_context->draw_indexed_instanced(index_count, instance_count, first_index, first_vertex, first_instance);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::prepare_for_rendering(IRootSignature* root_signature)
{
    RootSignature_D3D12_Impl* RS = static_cast<RootSignature_D3D12_Impl*>(root_signature);
    reset_root_signature(PIPELINE_TYPE_GRAPHICS, RS->dxRootSignature);
    bool has_srvs = RS->has_srvs();
    bool has_cbvs = RS->has_cbvs();
    bool has_uavs = RS->has_uavs();
    bool has_samplers = RS->has_samplers();
    bool has_root_cbvs = RS->has_root_cbvs();

    commit_bound_heaps();
    // Apply Resources
    uint64_t current_shader_dirty_cbv_slot_mask[SHADER_STAGE_COUNT] = {0};
    uint64_t current_shader_dirty_srv_slot_mask[SHADER_STAGE_COUNT] = {0};
    uint64_t current_shader_dirty_uav_slot_mask[SHADER_STAGE_COUNT] = {0};
    uint16_t current_shader_dirty_root_cbv_slot_mask[SHADER_STAGE_COUNT] = {0};
    
    uint32_t num_cbvs[SHADER_STAGE_COUNT] = {};
    uint32_t num_srvs[SHADER_STAGE_COUNT] = {};
    uint32_t num_uavs[SHADER_STAGE_COUNT] = {};
    uint32_t num_views = 0;
    // consume descriptor handles
    for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
    {
        const auto current_shader_register_mask = Bitmask<uint64_t>(state_cache.current_shader_cbv_count[stage]);
        current_shader_dirty_cbv_slot_mask[stage] = current_shader_register_mask & state_cache.constant_buffer_view_cache.bind_slot_mask[stage];
        num_cbvs[stage] = state_cache.current_shader_cbv_count[stage];
        num_views += num_cbvs[stage];
    }

    for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
    {
        const auto current_shader_register_mask = Bitmask<uint64_t>(state_cache.current_shader_srv_count[stage]);
        current_shader_dirty_srv_slot_mask[stage] = current_shader_register_mask & state_cache.shader_resource_view_cache.bind_slot_mask[stage];
        num_srvs[stage] = state_cache.current_shader_srv_count[stage];
        num_views += num_srvs[stage];
    }

    for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
    {
        const auto current_shader_register_mask = Bitmask<uint64_t>(state_cache.current_shader_uav_count[stage]);
        current_shader_dirty_uav_slot_mask[stage] = current_shader_register_mask & state_cache.unordered_access_view_cache.bind_slot_mask[stage];
        num_uavs[stage] = state_cache.current_shader_uav_count[stage];
        num_views += num_uavs[stage];
    }

    for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
    {
        const uint16_t current_shader_cbv_register_mask = Bitmask<uint16_t>(state_cache.current_shader_cbv_count[stage]);
        current_shader_dirty_root_cbv_slot_mask[stage] = current_shader_cbv_register_mask & state_cache.constant_buffer_cache.bind_slot_mask[stage];
    }

    auto first_slot = render_device->GetCbvSrvUavHeaps(0)->reserve_slots(num_views);

    if(num_views > 0)
    {
        if(has_srvs)
        {
            auto& srv_cache = state_cache.shader_resource_view_cache;
            for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
            {
                if(num_srvs[stage] > 0)
                {
                    const D3D12_GPU_DESCRIPTOR_HANDLE bind_handle = render_device->build_srv_table((SHADER_STAGE)stage, RS, srv_cache, num_srvs[stage], first_slot);
                    set_srv_table((SHADER_STAGE)stage, RS, srv_cache, num_srvs[stage], bind_handle);
                }
            }
        }

        if(has_cbvs)
        {
            auto& cbv_cache = state_cache.constant_buffer_view_cache;
            for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
            {
                if(num_cbvs[stage] > 0)
                {
                    const D3D12_GPU_DESCRIPTOR_HANDLE bind_handle = render_device->build_cbv_table((SHADER_STAGE)stage, RS, cbv_cache, num_cbvs[stage], first_slot);
                    set_cbv_table((SHADER_STAGE)stage, RS, cbv_cache, num_cbvs[stage], bind_handle);
                }
            }
        }

        if(has_root_cbvs)
        {
            auto& root_cbv_cache = state_cache.constant_buffer_cache;
            for(uint32_t stage = 0; stage < SHADER_STAGE_COUNT; ++stage)
            {
                if(current_shader_dirty_root_cbv_slot_mask[stage])
                {
                    set_root_cbv((SHADER_STAGE)stage, RS, root_cbv_cache, current_shader_dirty_root_cbv_slot_mask[stage]);
                }
            }
        }
    }

}

void DeviceContext_D3D12_Impl::set_shader_resource_view(SHADER_STAGE stage, uint32_t binding, ITexture_View* textureView)
{
    auto& cache = state_cache.shader_resource_view_cache;
    auto& current_srv_cache = cache.views[stage];
    if(current_srv_cache[binding] != textureView)
    {
        if(textureView != nullptr)
        {
            cache.bind_slot_mask[stage] |= ((uint64_t)1 << binding);
        }
        else
        {
            cache.bind_slot_mask[stage] &= ~((uint64_t)1 << binding);
        }
        current_srv_cache[binding] = static_cast<Texture_View_D3D12_Impl*>(textureView);
    }
}

void DeviceContext_D3D12_Impl::set_constant_buffer_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer)
{
    auto& cache = state_cache.constant_buffer_view_cache;
    auto& current_cbv_cache = cache.views[stage];

}

void DeviceContext_D3D12_Impl::set_unordered_access_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer)
{
    state_cache.unordered_access_view_cache.bind_slot_mask[stage] |= ((uint64_t)1 << binding);
}

void DeviceContext_D3D12_Impl::set_root_constant_buffer_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer)
{
    auto& cache = state_cache.constant_buffer_cache;
    auto& current_cbv_cache = cache.current_gpu_virtual_address[stage];
    Buffer_D3D12_Impl* cbv_buffer = static_cast<Buffer_D3D12_Impl*>(buffer);
    
    if(cbv_buffer && cbv_buffer->get_gpu_address(0) != D3D12_GPU_VIRTUAL_ADDRESS_UNKONWN)
    {
        state_cache.constant_buffer_cache.bind_slot_mask[stage] |= ((uint64_t)1 << binding);
        current_cbv_cache[binding] = cbv_buffer->get_gpu_address(0);
    }
    else
    {
        state_cache.constant_buffer_cache.bind_slot_mask[stage] &= ~((uint64_t)1 << binding);
    }
}

void DeviceContext_D3D12_Impl::set_srv_table(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, ShaderResourceViewCache& srv_cache, uint32_t slots_need, const D3D12_GPU_DESCRIPTOR_HANDLE& bind_descriptor)
{
    auto table_bind_slot = rs->srv_root_descriptor_table_bind_slot(stage);

    curr_command_context->set_graphics_root_descriptor_table(table_bind_slot, bind_descriptor);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::set_cbv_table(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, ConstantBufferViewCache& cbv_cache, uint32_t slots_need, const D3D12_GPU_DESCRIPTOR_HANDLE& bind_descriptor)
{
    auto table_bind_slot = rs->cbv_root_descriptor_table_bind_slot(stage);

    curr_command_context->set_graphics_root_descriptor_table(table_bind_slot, bind_descriptor);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::set_uav_table(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, UnorderedAccessViewCache& uav_cache, uint32_t slots_need, const D3D12_GPU_DESCRIPTOR_HANDLE& bind_descriptor)
{
    auto table_bind_slot = rs->uav_root_descriptor_table_bind_slot(stage);

    curr_command_context->set_graphics_root_descriptor_table(table_bind_slot, bind_descriptor);
    ++state.num_command;
}

void DeviceContext_D3D12_Impl::set_root_cbv(SHADER_STAGE stage, RootSignature_D3D12_Impl* rs, ConstantBufferCache& cbv_cache, uint16_t slots_need)
{
    const uint16_t root_cbv_slots_needs_mask = slots_need;
    uint16_t& current_dirty_slot_mask = cbv_cache.bind_slot_mask[stage];
    auto base_index = rs->cbv_root_descriptor_bind_slot(stage);
    const uint32_t root_cbv_needed = log2(root_cbv_slots_needs_mask) + 1;
    for(uint32_t i = 0;i < root_cbv_needed; ++i)
    {
        if((root_cbv_slots_needs_mask & (1 << i)) != 0)
        {
            const D3D12_GPU_VIRTUAL_ADDRESS cbv_address = cbv_cache.current_gpu_virtual_address[stage][i];
            curr_command_context->set_graphics_root_constant_buffer_view(base_index + i, cbv_address);
        }
    }
}

IRenderPass* DeviceContext_D3D12_Impl::create_render_pass(const RenderPassDesc& renderPassDesc)
{
    RenderPass_D3D12_Impl* dxRenderPass = cyber_new<RenderPass_D3D12_Impl>(render_device, renderPassDesc);
    return dxRenderPass;
}

void DeviceContext_D3D12_Impl::commit_subpass_rendertargets()
{
   #ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
       DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValues[GRAPHICS_MAX_MRT_COUNT]);
       DECLARE_ZERO(D3D12_CLEAR_VALUE, clearDepth);
       DECLARE_ZERO(D3D12_CLEAR_VALUE, clearStencil);
       DECLARE_ZERO(D3D12_RENDER_PASS_RENDER_TARGET_DESC, renderPassRenderTargetDescs[GRAPHICS_MAX_MRT_COUNT]);
       DECLARE_ZERO(D3D12_RENDER_PASS_DEPTH_STENCIL_DESC, renderPassDepthStencilDesc);
       uint32_t colorTargetCount = 0;
       
       auto* RenderPass = static_cast<RenderPass_D3D12_Impl*>(m_pRenderPass);
       auto RenderPassDesc = RenderPass->get_create_desc();

       auto* Framebuffer = static_cast<FrameBuffer_D3D12_Impl*>(m_pFrameBuffer);
       auto& FramebufferDesc = Framebuffer->get_create_desc();
       cyber_assert(RenderPassDesc.m_subpassCount > m_subpassIndex, "Subpass index out of range!");
       auto SubPassDesc = RenderPassDesc.m_pSubpasses[m_subpassIndex];

       // color
       for(uint32_t i = 0; i < SubPassDesc.m_renderTargetCount; ++i)
       {
           auto attachmentRef = SubPassDesc.m_pRenderTargetAttachments[i];
           auto attachmentIndex = attachmentRef.m_attachmentIndex;
           auto view = Framebuffer->get_attachment(attachmentIndex);
           Texture_View_D3D12_Impl * tex_view = static_cast<Texture_View_D3D12_Impl *>(view);

           clearValues[i].Format = DXGIUtil_TranslatePixelFormat(tex_view->get_create_desc().format);
           clearValues[i].Color[0] = color_clear_values[i].r;
           clearValues[i].Color[1] = color_clear_values[i].g;
           clearValues[i].Color[2] = color_clear_values[i].b;
           clearValues[i].Color[3] = color_clear_values[i].a;
           
           D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE beginningAccess = gDx12PassBeginOpTranslator[attachmentRef.m_loadAction];
           Texture_View_D3D12_Impl * tex_view_resolve = static_cast<Texture_View_D3D12_Impl *>(FramebufferDesc.m_ppAttachments[attachmentIndex]);
           if(attachmentRef.m_sampleCount != SAMPLE_COUNT_1 && tex_view_resolve)
           {
               Texture_D3D12_Impl* tex = static_cast<Texture_D3D12_Impl*>(tex_view->get_create_desc().p_texture);
               Texture_D3D12_Impl* tex_resolve = static_cast<Texture_D3D12_Impl*>(tex_view_resolve->get_create_desc().p_texture);
               D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
               renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = tex_view->m_rtvDsvDescriptorHandle;
               renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, clearValues[i] };
               renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess , {} };
               auto& resolve = renderPassRenderTargetDescs[colorTargetCount].EndingAccess.Resolve;
               resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
               resolve.Format = clearValues[i].Format;
               resolve.pSrcResource = tex->get_d3d12_resource();
               resolve.pDstResource = tex_resolve->get_d3d12_resource();
               
               m_subResolveResource[i].SrcRect = { 0, 0, (LONG)tex->get_create_desc().m_width, (LONG)tex->get_create_desc().m_height };
               m_subResolveResource[i].DstX = 0;
               m_subResolveResource[i].DstY = 0;
               m_subResolveResource[i].SrcSubresource = 0;
               m_subResolveResource[i].DstSubresource = CALC_SUBRESOURCE_INDEX(0, 0, 0, tex_resolve->get_create_desc().m_mipLevels, tex_resolve->get_create_desc().m_arraySize + 1);
               resolve.PreserveResolveSource = false;
               resolve.SubresourceCount = 1;
               resolve.pSubresourceParameters = &m_subResolveResource[i];
           }
           else
           {
               // Load & Store action
               D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = gDx12PassEndOpTranslator[attachmentRef.m_storeAction];
               renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = tex_view->m_rtvDsvDescriptorHandle;
               renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, clearValues[i] };
               renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess , {} };
           }
            curr_command_context->clear_render_target(tex_view->m_rtvDsvDescriptorHandle, clearValues[i].Color, 0, nullptr);

           ++colorTargetCount;
       }
       // depth stencil
       D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pRenderPassDepthStencilDesc = nullptr;
       if(SubPassDesc.m_pDepthStencilAttachment != nullptr)
       {
           auto attachmentRef = SubPassDesc.m_pDepthStencilAttachment;
           auto depthStencilAttachIndex = SubPassDesc.m_pDepthStencilAttachment->m_attachmentIndex;
           auto attachDesc = RenderPassDesc.m_pAttachments[depthStencilAttachIndex];
           auto clearValue = depth_stencil_clear_value;
           Texture_View_D3D12_Impl * dt_view = static_cast<Texture_View_D3D12_Impl *>(Framebuffer->get_attachment(depthStencilAttachIndex));
           D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE depthBeginningAccess = gDx12PassBeginOpTranslator[attachmentRef->m_loadAction];
           D3D12_RENDER_PASS_ENDING_ACCESS_TYPE depthEndingAccess = gDx12PassEndOpTranslator[attachmentRef->m_storeAction];
           D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE stencilBeginningAccess = gDx12PassBeginOpTranslator[attachmentRef->m_stencilLoadAction];
           D3D12_RENDER_PASS_ENDING_ACCESS_TYPE stencilEndingAccess = gDx12PassEndOpTranslator[attachmentRef->m_stencilStoreAction];
           clearDepth.Format = DXGIUtil_TranslatePixelFormat(attachDesc.m_format);
           clearDepth.DepthStencil.Depth = clearValue.depth;
           clearStencil.Format = DXGIUtil_TranslatePixelFormat(attachDesc.m_format);;
           clearStencil.DepthStencil.Stencil = clearValue.stencil;
           renderPassDepthStencilDesc.cpuDescriptor = dt_view->m_rtvDsvDescriptorHandle;
           renderPassDepthStencilDesc.DepthBeginningAccess = { depthBeginningAccess, clearDepth };
           renderPassDepthStencilDesc.DepthEndingAccess = { depthEndingAccess, {} };
           renderPassDepthStencilDesc.StencilBeginningAccess = { stencilBeginningAccess, clearStencil };
           renderPassDepthStencilDesc.StencilEndingAccess = { stencilEndingAccess, {} };
           pRenderPassDepthStencilDesc = &renderPassDepthStencilDesc;
       }
       D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRenderPassRenderTargetDesc = renderPassRenderTargetDescs;

       curr_command_context->begin_render_pass( 
           colorTargetCount, pRenderPassRenderTargetDesc, pRenderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
        ++state.num_command;
   #else
       cyber_warn(false, "ID3D12GraphicsCommandList4 is not defined!");
   #endif
}

void DeviceContext_D3D12_Impl::transition_resource_state(const ResourceBarrierDesc& barrierDesc)
{
    for(uint32_t i = 0;i < barrierDesc.buffer_barrier_count; i++)
    {
        const BufferBarrier& buffer_barrier = barrierDesc.buffer_barriers[i];
        Buffer_D3D12_Impl* buffer = static_cast<Buffer_D3D12_Impl*>(buffer_barrier.buffer);
        curr_command_context->transition_resource(*buffer, buffer_barrier);
        ++state.num_command;
    }

    for(uint32_t i = 0;i < barrierDesc.texture_barrier_count; i++)
    {
        const TextureBarrier& texture_barrier = barrierDesc.texture_barriers[i];
        Texture_D3D12_Impl* texture = static_cast<Texture_D3D12_Impl*>(texture_barrier.texture);
        curr_command_context->transition_resource(*texture, texture_barrier);
        ++state.num_command;
    }
}

void DeviceContext_D3D12_Impl::request_command_context()
{
    curr_command_context = render_device->allocate_command_context(get_command_queue_id());
}

Dynamic_Allocation_D3D12 DeviceContext_D3D12_Impl::allocate_dynamic_memory(uint64_t size_in_bytes, uint64_t alignment)
{
    return m_pDynamicHeap->allocate(size_in_bytes, alignment);
}

void DeviceContext_D3D12_Impl::set_bound_heap(uint32_t index, DescriptorHeap_D3D12* heap)
{
    cyber_check_msg(index < 2, "Invalid heap index");

    state_cache.bound_heaps[index] = heap;
    state_cache.bound_heap_start_handles[index].ptr = heap->get_heap()->GetGPUDescriptorHandleForHeapStart().ptr;
}

void DeviceContext_D3D12_Impl::commit_bound_heaps()
{
    state_cache.bound_heaps[0] = render_device->GetCbvSrvUavHeaps();
    state_cache.bound_heaps[1] = render_device->GetSamplerHeaps();
    if(state_cache.bound_heaps[0] || state_cache.bound_heaps[1])
    {
        ID3D12DescriptorHeap* heaps[2] = { state_cache.bound_heaps[0]->get_heap(), state_cache.bound_heaps[1]->get_heap() };
        curr_command_context->set_descriptor_heaps(2, heaps);
        ++state.num_command;
    }
}

void DeviceContext_D3D12_Impl::flush()
{
    flush(true);
    //render_device->close_and_execute_command_context(COMMAND_QUEUE_TYPE_GRAPHICS, 1, &curr_command_context);
}

void DeviceContext_D3D12_Impl::finish_frame()
{
    m_pDynamicHeap->release_allocated_pages(0);
}

void DeviceContext_D3D12_Impl::flush(bool request_new_command_context, uint32_t num_command_lists, ICommandBuffer** command_lists)
{
    cyber_check_msg(!is_deferred_context() || num_command_lists == 0 && command_lists == nullptr, "Deferred contexts cannot execute command lists directly");

    eastl::vector<RenderDevice_D3D12_Impl::PooledCommandContext> contexts;
    contexts.reserve(num_command_lists + 1);

    if(curr_command_context)
    {
        if(state.num_command != 0)
        {
            contexts.emplace_back(eastl::move(curr_command_context));
        }
        else if(!request_new_command_context)
        {
            render_device;
        }
    }

    // for deferred context
    for(uint32_t i = 0; i < num_command_lists; ++i)
    {

    }

    if(!contexts.empty())
    {
        render_device->close_and_execute_command_context(get_command_queue_id(), (uint32_t)contexts.size(), contexts.data());
    }

    if(!curr_command_context && request_new_command_context)
    {
        request_command_context();
    }

    state.num_command = 0;
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