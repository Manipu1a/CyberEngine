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
            DECLARE_ZERO(RenderObject::InstanceCreateDesc, instance_desc);
            instance_desc.m_enableDebugLayer = true;
            instance_desc.m_enableGpuBasedValidation = false;
            instance_desc.m_enableSetName = true;
            instance = device->create_instance(instance_desc);

            // Filter adapters
            uint32_t adapter_count = 0;
            device->enum_adapters(instance, nullptr, &adapter_count);
            RenderObject::IAdapter* adapters[64];
            device->enum_adapters(instance, adapters, &adapter_count);
            adapter = adapters[0];

            // Create device
            DECLARE_ZERO(QueueGroupDesc, queue_group_desc);
            queue_group_desc.m_queueCount = 1;
            queue_group_desc.m_queueType = QUEUE_TYPE_GRAPHICS;
            DECLARE_ZERO(RenderObject::RenderDeviceCreateDesc, device_desc);
            device_desc.m_queueGroupCount = 1;
            device_desc.m_queueGroups = { queue_group_desc };
            device->create_device(adapter, device_desc);
            queue = device->get_queue(QUEUE_TYPE_GRAPHICS, 0);
            present_fence = device->create_fence();

            immediate_context = cyber_new<RenderObject::CEDeviceContext>(device);

            // Create swapchain
        #if defined (_WIN32) || defined (_WIN64)
            surface = device->surface_from_hwnd(getWindow()->getNativeWindow());
        #elif defined(_APPLE_)
        #endif
            DECLARE_ZERO(RenderObject::SwapChainDesc, chain_desc);
            chain_desc.m_pSurface = surface;
            chain_desc.m_width = getWindow()->getWidth();
            chain_desc.m_height = getWindow()->getHeight();
            chain_desc.m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
            chain_desc.m_imageCount = 3;
            chain_desc.m_presentQueue = queue;
            chain_desc.m_presentQueueCount = 1;
            chain_desc.m_enableVsync = true;
            swap_chain = device->create_swap_chain(chain_desc);

            present_swmaphore = device->create_fence();
            pool = device->create_command_pool(queue, RenderObject::CommandPoolCreateDesc());

            RenderObject::CommandBufferCreateDesc cmd_buffer_desc = {.m_isSecondary = false};
            cmd = device->create_command_buffer(pool, cmd_buffer_desc);
        }
    }
}
