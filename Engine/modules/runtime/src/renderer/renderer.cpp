#include "renderer/renderer.h"
#include "DirectXColors.h"
#include "DirectXMath.h"
#include <D3Dcompiler.h>
#include <array>
#include <combaseapi.h>
#include <cstdint>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcommon.h>
#include <debugapi.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <dxgitype.h>
#include <handleapi.h>
#include <intsafe.h>
#include <memory>
#include <minwinbase.h>
#include <minwindef.h>
#include <stdlib.h>
#include <string>
#include <synchapi.h>
#include <vector>
#include <winbase.h>
#include <winerror.h>
#include <wingdi.h>
#include <winnt.h>
#include "resource/resource_loader.h"
#include "core/Application.h"
#include "rhi/backend/d3d12/rhi_d3d12.h"
namespace Cyber
{
    #define UTF8(str) u8##str

    void Renderer::initialize(class Application* app, const RendererDesc& desc)
    {
        RHI::createRHI(desc.backend);
        create_gfx_objects(app);

        // Create views
        for(uint32_t i = 0; i < swap_chain->mBufferCount; ++i)
        {
            RHITextureViewCreateDesc view_desc = {
                .texture = swap_chain->mBackBuffers[i],
                .format = (ERHIFormat)swap_chain->mBackBuffers[i]->mFormat,
                .usages = RHI_TVU_RTV_DSV,
                .aspects = RHI_TVA_COLOR,
                .dimension = RHI_TEX_DIMENSION_2D,
                .array_layer_count = 1
            };
            views[i] = rhi_create_texture_view(device, view_desc);
        }
        create_render_pipeline();
    }

    void Renderer::finalize()
    {
        rhi_wait_queue_idle(queue);
        rhi_wait_fences(&present_fence, 1);
        rhi_free_fence(present_fence);
        for(uint32_t i = 0;i < swap_chain->mBufferCount; ++i)
        {
            rhi_free_texture_view(views[i]);
        }
        rhi_free_swap_chain(swap_chain);
        rhi_free_surface(surface);
        rhi_free_command_buffer(cmd);
        rhi_free_command_pool(pool);
        rhi_free_render_pipeline(pipeline);
        rhi_free_root_signature(root_signature);
        rhi_free_queue(queue);
        rhi_free_device(device);
        rhi_free_instance(instance);
    }
    void Renderer::create_gfx_objects(class Application* app)
    {
        // Create instance
        DECLARE_ZERO(RHIInstanceCreateDesc, instance_desc);
        instance_desc.enable_debug_layer = false;
        instance_desc.enable_gpu_based_validation = false;
        instance_desc.enable_set_name = true;
        instance = rhi_create_instance(instance_desc);

        // Filter adapters
        uint32_t adapter_count = 0;
        rhi_enum_adapters(instance, nullptr, &adapter_count);
        RHIAdapter* adapters[64];
        rhi_enum_adapters(instance, adapters, &adapter_count);
        adapter.reset(adapters[0]);
        RHIAdapter_D3D12* adapter_d3d12 = static_cast<RHIAdapter_D3D12*>(adapters[0]);

        // Create device
        DECLARE_ZERO(RHIQueueGroupDesc, queue_group_desc);
        queue_group_desc.queue_count = 1;
        queue_group_desc.queue_type = RHI_QUEUE_TYPE_GRAPHICS;
        DECLARE_ZERO(RHIDeviceCreateDesc, device_desc);
        device_desc.queue_group_count = 1;
        device_desc.queue_groups = { queue_group_desc };
        device = rhi_create_device(adapter, device_desc);
        queue = rhi_get_queue(device, RHI_QUEUE_TYPE_GRAPHICS, 0);
        present_fence = rhi_create_fence(device);

        // Create swapchain
    #if defined (_WIN32) || defined (_WIN64)
        surface = rhi_surface_from_hwnd(device, app->getWindow()->getNativeWindow());
    #elif defined(_APPLE_)
    #endif
        DECLARE_ZERO(RHISwapChainCreateDesc, chain_desc);
        chain_desc.surface = surface;
        chain_desc.mWidth = app->getWindow()->getWidth();
        chain_desc.mHeight = app->getWindow()->getHeight();
        chain_desc.mFormat = RHI_FORMAT_R8G8B8A8_UNORM;
        chain_desc.mImageCount = 3;
        chain_desc.mPresentQueue = queue;
        chain_desc.mPresentQueueCount = 1;
        chain_desc.mEnableVsync = true;
        swap_chain = rhi_create_swap_chain(device, chain_desc);

        present_swmaphore = rhi_create_fence(device);
        pool = rhi_create_command_pool(queue, CommandPoolCreateDesc());
        CommandBufferCreateDesc cmd_buffer_desc = {.is_secondary = false};
        cmd =  rhi_create_command_buffer(pool, cmd_buffer_desc);
    }

    void Renderer::create_render_pipeline()
    {
        // create shader
        ResourceLoader::ShaderLoadDesc vs_load_desc = {};
        vs_load_desc.target = shader_target_6_0;
        vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
            .file_name = CYBER_UTF8(""),
            .stage = RHI_SHADER_STAGE_VERT,
            .entry_point_name = CYBER_UTF8("main")
        };
        Ref<RHIShaderLibrary> vs_shader = ResourceLoader::add_shader(*this, vs_load_desc);

        ResourceLoader::ShaderLoadDesc ps_load_desc = {};
        ps_load_desc.target = shader_target_6_0;
        ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
            .file_name = CYBER_UTF8(""),
            .stage = RHI_SHADER_STAGE_FRAG,
            .entry_point_name = CYBER_UTF8("main"),
        };
        Ref<RHIShaderLibrary> ps_shader = ResourceLoader::add_shader(*this, ps_load_desc);

        // create root signature
        Ref<RHIPipelineShaderCreateDesc> pipeline_shader_create_desc[2];
        pipeline_shader_create_desc[0]->stage = RHI_SHADER_STAGE_VERT;
        pipeline_shader_create_desc[0]->library = vs_shader;
        pipeline_shader_create_desc[0]->entry = CYBER_UTF8("main");
        pipeline_shader_create_desc[1]->stage = RHI_SHADER_STAGE_FRAG;
        pipeline_shader_create_desc[1]->library = ps_shader;
        pipeline_shader_create_desc[1]->entry = CYBER_UTF8("main");
        RHIRootSignatureCreateDesc root_signature_create_desc = {
          .shaders = pipeline_shader_create_desc,
          .shader_count = 2,
        };
        root_signature = rhi_create_root_signature(device, root_signature_create_desc);
        // create descriptor set
        RHIVertexLayout vertex_layout = {.attribute_count = 0};
        RHIRenderPipelineCreateDesc rp_desc = 
        {
            .root_signature = root_signature,
            .vertex_shader = pipeline_shader_create_desc[0],
            .fragment_shader = pipeline_shader_create_desc[1],
            .vertex_layout = &vertex_layout,
            .color_formats = &views[0]->create_info.format,
            .render_target_count = 1,
            .prim_topology = RHI_PRIM_TOPO_TRIANGLE_LIST,
        };
        pipeline = rhi_create_render_pipeline(device, rp_desc);
        rhi_free_shader_library(vs_shader);
        rhi_free_shader_library(ps_shader);
    }

    void Renderer::update(float DeltaTime)
    {
        raster_draw();
    }

    void Renderer::raster_draw()
    {
        // sync & reset
        rhi_wait_fences(&present_fence, 1);
        RHIAcquireNextDesc acquire_desc = {
            .fence = present_fence
        };
        backbuffer_index = rhi_acquire_next_image(swap_chain, acquire_desc);
        const auto back_buffer = swap_chain->mBackBuffers[backbuffer_index];
        const auto back_buffer_view = views[backbuffer_index];
        rhi_reset_command_pool(pool);
        // record
        rhi_cmd_begin(cmd);
        RHIColorAttachment screen_attachment = {
            .view = back_buffer_view,
            .load_action = RHI_LOAD_ACTION_CLEAR,
            .store_action = RHI_STORE_ACTION_STORE,
            .clear_value = fastclear_0000
        };
        RHIRenderPassDesc rp_desc = {
            .sample_count = RHI_SAMPLE_COUNT_1,
            .color_attachments = &screen_attachment,
            .depth_stencil_attachment = nullptr,
            .render_target_count = 1,
        };
        RHITextureBarrier draw_barrier = {
            .texture = back_buffer,
            .src_state = RHI_RESOURCE_STATE_UNDEFINED,
            .dst_state = RHI_RESOURCE_STATE_RENDER_TARGET
        };
        RHIResourceBarrierDesc barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barrier_count = 1 };
        rhi_cmd_resource_barrier(cmd, barrier_desc0);
        Ref<RHIRenderPassEncoder> rp_encoder = rhi_cmd_begin_render_pass(cmd, rp_desc);
        rhi_render_encoder_set_viewport(rp_encoder, 0, 0, back_buffer->mWidth, back_buffer->mHeight, 0.0f, 1.0f);
        rhi_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->mWidth, back_buffer->mHeight);
        rhi_render_encoder_bind_pipeline(rp_encoder, pipeline);
        rhi_render_encoder_draw(rp_encoder, 3, 0);

        RHITextureBarrier present_barrier = {
            .texture = back_buffer,
            .src_state = RHI_RESOURCE_STATE_RENDER_TARGET,
            .dst_state = RHI_RESOURCE_STATE_PRESENT
        };
        rhi_cmd_end_render_pass(cmd);
        RHIResourceBarrierDesc barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barrier_count = 1 };
        rhi_cmd_resource_barrier(cmd, barrier_desc1);
        rhi_cmd_end(cmd);

        // submit
        RHIQueueSubmitDesc submit_desc = {
            .pCmds = &cmd,
            .mCmdsCount = 1
        };
        rhi_submit_queue(queue, submit_desc);

        // present
        
        RHIQueuePresentDesc present_desc = {
            .swap_chain = swap_chain,
            .wait_semaphores = nullptr,
            .wait_semaphore_count = 0,
            .index = backbuffer_index,
        };
        rhi_present_queue(queue, present_desc);
    }
}