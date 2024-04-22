#include "GameRuntime/GameApplication.h"
#include "core/Timestep.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/instance_d3d12.h"

namespace Cyber
{
    GameApplication::GameApplication()
    {

    }

    GameApplication::~GameApplication()
    {

    }

    void GameApplication::initialize(Cyber::WindowDesc& desc)
    {
        create_window(desc);


        mInputSystem = CreateRef<InputSystem>();
        mInputSystem->initInputSystem();
    }

    void GameApplication::create_window(Cyber::WindowDesc& desc)
    {
        mWindow = Cyber::Window::createWindow(desc);
        mWindow->setEventCallback(CB_BIND_EVENT_FN(Application::onEvent));
    }

    void GameApplication::run()
    {
        while(mRunning)
        {
            float time = 0.0f;
            Timestep timestep = time - mLastFrameTime;
            mLastFrameTime = time;

            //spdlog::info("Time: {0}", timestep);

            mInputSystem->updateInputSystem(mWindow->getNativeWindow());
            mWindow->onUpdate(timestep);
            update(timestep);
        }
    }

    void GameApplication::update(float deltaTime)
    {

    }

    void GameApplication::onEvent(Event& e)
    {
        //EventDispatcher dispatcher(e);
        //dispatcher.Dispatch<WindowCloseEvent>(CB_BIND_EVENT_FN(Application::onWindowClose));
    }

    bool GameApplication::onWindowClose(WindowCloseEvent& e)
    {
        mRunning = false;
        return true;
    }

    void GameApplication::create_render_device(GRAPHICS_BACKEND backend)
    {
        // Create instance
        DECLARE_ZERO(RenderObject::InstanceCreateDesc, instance_desc);
        instance_desc.m_enableDebugLayer = true;
        instance_desc.m_enableGpuBasedValidation = false;
        instance_desc.m_enableSetName = true;

        if(backend == GRAPHICS_BACKEND_D3D12)
        {
            m_pInstance = cyber_new<RenderObject::Instance_D3D12_Impl>(instance_desc);
        }

    }

    void GameApplication::create_gfx_objects()
    {
        create_render_device(GRAPHICS_BACKEND_D3D12);
        // Filter adapters
        uint32_t adapter_count = 0;
        m_pInstance->enum_adapters(nullptr, &adapter_count);
        RenderObject::IAdapter* adapters[64];
        m_pInstance->enum_adapters(adapters, &adapter_count);
        m_pAdapter = adapters[0];

        // Create device
        DECLARE_ZERO(QueueGroupDesc, queue_group_desc);
        queue_group_desc.m_queueCount = 1;
        queue_group_desc.m_queueType = QUEUE_TYPE_GRAPHICS;
        DECLARE_ZERO(RenderObject::RenderDeviceCreateDesc, device_desc);
        device_desc.m_queueGroupCount = 1;
        device_desc.m_queueGroups = { queue_group_desc };
        m_pRenderDevice = m_pInstance->create_render_device(m_pAdapter, device_desc);
        m_pQueue = m_pRenderDevice->get_queue(QUEUE_TYPE_GRAPHICS, 0);

        // Create swapchain
    #if defined (_WIN32) || defined (_WIN64)
        m_pSurface = m_pRenderDevice->surface_from_hwnd(getWindow()->getNativeWindow());
    #elif defined(_APPLE_)
    #endif
        DECLARE_ZERO(RenderObject::SwapChainDesc, chain_desc);
        chain_desc.m_pSurface = m_pSurface;
        chain_desc.m_width = getWindow()->getWidth();
        chain_desc.m_height = getWindow()->getHeight();
        chain_desc.m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
        chain_desc.m_imageCount = 3;
        chain_desc.m_presentQueue = m_pQueue;
        chain_desc.m_presentQueueCount = 1;
        chain_desc.m_enableVsync = true;
        m_pSwapChain = m_pRenderDevice->create_swap_chain(chain_desc);

        m_pPresentFence = m_pRenderDevice->create_fence();
        m_pPool = m_pRenderDevice->create_command_pool(m_pQueue, RenderObject::CommandPoolCreateDesc());

        RenderObject::CommandBufferCreateDesc cmd_buffer_desc = {.m_isSecondary = false};
        m_pCmd = m_pRenderDevice->create_command_buffer(m_pPool, cmd_buffer_desc);
    }
}