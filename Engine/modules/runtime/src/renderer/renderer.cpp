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

namespace Cyber
{
    Renderer::Renderer(HINSTANCE hInstance)
        : mhAppInst(hInstance)
    {

    }

    Renderer::~Renderer()
    {

    }

    void Renderer::initialize(class Application* app, const RendererDesc& desc)
    {
        RHI::createRHI(desc.backend);
        create_gfx_objects(app);

        present_swmaphore = RHI::GetRHIContext().rhi_create_fence(device);
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            pools[i] = RHI::GetRHIContext().rhi_create_command_pool(queue, CommandPoolCreateDesc());
            CommandBufferCreateDesc cmd_buffer_desc = {.is_secondary = false};
            cmds[i] = RHI::GetRHIContext().rhi_create_command_buffer(pools[i], cmd_buffer_desc);
            exec_fences[i] = RHI::GetRHIContext().rhi_create_fence(device);
        }
        // Create views
        for(uint32_t i = 0; i < swap_chain->mBufferCount; ++i)
        {
            RHITextureViewCreateDesc view_desc = {
                .texture = swap_chain->mBackBuffers[i],
                .aspects = RHI_TVA_COLOR,
                .dimension = RHI_TEX_DIMENSION_2D,
                .format = (ERHIFormat)swap_chain->mBackBuffers[i]->mFormat,
                .usages = RHI_TVU_RTV_DSV,
                .array_layer_count = 1
            };
            views[i] = RHI::GetRHIContext().rhi_create_texture_view(device, view_desc);
        }
        create_render_pipeline();
    }

    void Renderer::create_gfx_objects(class Application* app)
    {
        // Create instance
        DECLARE_ZERO(RHIInstanceCreateDesc, instance_desc);
        instance_desc.enable_debug_layer = false;
        instance_desc.enable_gpu_based_validation = false;
        instance_desc.enable_set_name = true;
        instance = RHI::GetRHIContext().rhi_create_instance(instance_desc);

        // Filter adapters
        uint32_t adapter_count = 0;
        RHI::GetRHIContext().rhi_enum_adapters(instance, nullptr, &adapter_count);
        RHIAdapter adapters[64];
        RHI::GetRHIContext().rhi_enum_adapters(instance, adapters, &adapter_count);
        adapter = CreateRef<RHIAdapter>(adapters[0]);

        // Create device
        DECLARE_ZERO(RHIQueueGroupDesc, queue_group_desc);
        queue_group_desc.queue_count = 1;
        queue_group_desc.queue_type = RHI_QUEUE_TYPE_GRAPHICS;
        DECLARE_ZERO(RHIDeviceCreateDesc, device_desc);
        device_desc.queue_group_count = 1;
        device_desc.queue_groups = { queue_group_desc };
        device = RHI::GetRHIContext().rhi_create_device(adapter, device_desc);
        queue = RHI::GetRHIContext().rhi_get_queue(device, RHI_QUEUE_TYPE_GRAPHICS, 0);
        present_fence = RHI::GetRHIContext().rhi_create_fence(device);

        // Create swapchain
    #if defined (_WIN32) || defined (_WIN64)
        surface = RHI::GetRHIContext().rhi_surface_from_hwnd(device, app->getWindow()->getNativeWindow());
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
        swap_chain = RHI::GetRHIContext().rhi_create_swap_chain(device, chain_desc);
    }

    void Renderer::create_render_pipeline()
    {
        // create shader
        ResourceLoader::ShaderLoadDesc vs_load_desc = {};
        vs_load_desc.target = shader_target_6_0;
        vs_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
            .file_name = "",
            .stage = RHI_SHADER_STAGE_VERT,
            .entry_point_name = "main",
        };
        Ref<RHIShaderLibrary> vs_shader = ResourceLoader::add_shader(*this, vs_load_desc);

        ResourceLoader::ShaderLoadDesc ps_load_desc = {};
        ps_load_desc.target = shader_target_6_0;
        ps_load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
            .file_name = "",
            .stage = RHI_SHADER_STAGE_FRAG,
            .entry_point_name = "main",
        };
        Ref<RHIShaderLibrary> ps_shader = ResourceLoader::add_shader(*this, ps_load_desc);

        // create root signature
        Ref<RHIPipelineShaderCreateDesc> pipeline_shader_create_desc[2];
        pipeline_shader_create_desc[0]->stage = RHI_SHADER_STAGE_VERT;
        pipeline_shader_create_desc[0]->library = vs_shader;
        pipeline_shader_create_desc[0]->entry = "main";
        pipeline_shader_create_desc[1]->stage = RHI_SHADER_STAGE_FRAG;
        pipeline_shader_create_desc[1]->library = ps_shader;
        pipeline_shader_create_desc[1]->entry = "main";
        RHIRootSignatureCreateDesc root_signature_create_desc = {
          .shaders = pipeline_shader_create_desc,
          .shader_count = 2,
        };
        Ref<RHIRootSignature> root_signature = RHI::GetRHIContext().rhi_create_root_signature(device, root_signature_create_desc);
        // create descriptor set
        RHIVertexLayout vertex_layout = {.attribute_count = 0};
        RHIRenderPipelineCreateDesc rp_desc = 
        {
            .root_signature = root_signature,
            .prim_topology = RHI_PRIM_TOPO_TRIANGLE_LIST,
            .vertex_layout = &vertex_layout,
            .vertex_shader = pipeline_shader_create_desc[0],
            .fragment_shader = pipeline_shader_create_desc[1],
            .render_target_count = 1,
            .color_formats = &views[0]->create_info.format
        };
        Ref<RHIRenderPipeline> render_pipeline = RHI::GetRHIContext().rhi_create_render_pipeline(device, rp_desc);
        RHI::GetRHIContext().rhi_free_shader_library(vs_shader);
        RHI::GetRHIContext().rhi_free_shader_library(ps_shader);
    }

    void Renderer::raster_draw()
    {
        // sync & reset
        RHI::GetRHIContext().rhi_wait_fences(&present_fence, 1);
        RHIAcquireNextDesc acquire_desc = {
            .fence = present_fence
        };
        backbuffer_index = RHI::GetRHIContext().rhi_acquire_next_image(swap_chain, acquire_desc);
        const auto back_buffer = swap_chain->mBackBuffers[backbuffer_index];
        const auto back_buffer_view = views[backbuffer_index];
        RHI::GetRHIContext().rhi_reset_command_pool(pool);

    }
}