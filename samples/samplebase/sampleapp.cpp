#include "sampleapp.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace Samples
    {
        SampleApp::SampleApp()
        {
           // RHI::createRHI(ERHIBackend::RHI_BACKEND_D3D12);

        }
        SampleApp::~SampleApp()
        {

        }

        void SampleApp::initialize(Cyber::WindowDesc& desc)
        {
        }

        void SampleApp::run()
        {
        }

        void SampleApp::update(float deltaTime)
        {
        }

        void SampleApp::create_gfx_objects()
        {
            // Create instance
            DECLARE_ZERO(RHIInstanceCreateDesc, instance_desc);
            instance_desc.enable_debug_layer = true;
            instance_desc.enable_gpu_based_validation = false;
            instance_desc.enable_set_name = true;
            instance = device->create_instance(instance_desc);

            // Filter adapters
            uint32_t adapter_count = 0;
            device->enum_adapters(instance, nullptr, &adapter_count);
            RHIAdapter* adapters[64];
            device->enum_adapters(instance, adapters, &adapter_count);
            adapter = adapters[0];

            // Create device
            DECLARE_ZERO(RHIQueueGroupDesc, queue_group_desc);
            queue_group_desc.queue_count = 1;
            queue_group_desc.queue_type = RHI_QUEUE_TYPE_GRAPHICS;
            DECLARE_ZERO(RenderObject::RenderDeviceCreateDesc, device_desc);
            device_desc.queue_group_count = 1;
            device_desc.queue_groups = { queue_group_desc };
            device->create_device(adapter, device_desc);
            queue = device->get_queue(RHI_QUEUE_TYPE_GRAPHICS, 0);
            present_fence = device->create_fence();

            immediate_context = cyber_new<RenderObject::CEDeviceContext>(device);

            // Create swapchain
        #if defined (_WIN32) || defined (_WIN64)
            surface = device->surface_from_hwnd(getWindow()->getNativeWindow());
        #elif defined(_APPLE_)
        #endif
            DECLARE_ZERO(RHISwapChainCreateDesc, chain_desc);
            chain_desc.surface = surface;
            chain_desc.mWidth = getWindow()->getWidth();
            chain_desc.mHeight = getWindow()->getHeight();
            chain_desc.mFormat = RHI_FORMAT_R8G8B8A8_UNORM;
            chain_desc.mImageCount = 3;
            chain_desc.mPresentQueue = queue;
            chain_desc.mPresentQueueCount = 1;
            chain_desc.mEnableVsync = true;
            swap_chain = device->create_swap_chain(chain_desc);

            present_swmaphore = device->create_fence();
            pool = device->create_command_pool(queue, CommandPoolCreateDesc());

            CommandBufferCreateDesc cmd_buffer_desc = {.is_secondary = false};
            cmd = device->create_command_buffer(pool, cmd_buffer_desc);
        }
    }
}
