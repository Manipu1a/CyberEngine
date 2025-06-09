#include "core/Application.h"
#include "platform/windows/windows_application.h"
#include "platform/memory.h"
#include "inputsystem/InputSystem.h"
#include "core/Timestep.h"
#include "core/window.h"
#include "gameruntime/sampleapp.h"
#include "editor/editor_impl_win32.h"

namespace Cyber
{
    namespace Core
    {
        Application* Application::sInstance = nullptr;

        Application::Application(const WindowDesc& desc)
        {
            if(sInstance == nullptr)
                sInstance = this;
        }

        Application::~Application()
        {
            
        }

        Application* Application::getApp()
        {
            if(sInstance == nullptr)
            {
                cyber_assert(false, "Application is not created!");
            }
            return sInstance;
        }

        void Application::initialize()
        {
            CB_CORE_INFO("Application :: initialize()");
            m_pRenderer = cyber_new<Renderer::Renderer>();
            m_pRenderer->create_gfx_objects();
            m_pInputSystem = cyber_new<InputSystem>();
            m_pInputSystem->initInputSystem();
            Editor::EditorCreateInfo createInfo = {};
            createInfo.application = this;
            createInfo.pDevice = m_pRenderer->get_render_device();
            createInfo.swap_chain = m_pRenderer->get_swap_chain();
            createInfo.BackBufferFmt = TEX_FORMAT_RGB32_FLOAT;
            createInfo.DepthBufferFmt = TEX_FORMAT_D32_FLOAT;
            createInfo.Hwnd = m_pWindow->get_native_window();
            m_pEditor = Editor::Editor_Impl_Win32::create(createInfo);
            m_pEditor->initialize(createInfo.pDevice, createInfo.Hwnd);
            m_pSampleApp->initialize();
        }

        void Application::run()
        {
            CB_CORE_INFO("Application :: run()");
            while(m_running)
            {
                update(0.0f);
            }

            CB_CORE_INFO("Application :: run() - finished");
        }

        void Application::update(float deltaTime)
        {
            float time = 0.0f;
            Timestep timestep = time - m_last_frame_time;
            m_last_frame_time = time;
            //spdlog::info("Time: {0}", timestep);
            m_pInputSystem->updateInputSystem(m_pWindow->get_native_window());
            m_pWindow->update(timestep);
            m_pRenderer->update(timestep);
            m_pSampleApp->update(timestep);
            auto back_buffer_index = m_pSampleApp->get_back_buffer_index();
            auto swap_chain = m_pRenderer->get_swap_chain();
            auto back_buffer_view = swap_chain->get_back_buffer_srv_view(back_buffer_index);
            auto render_device = m_pRenderer->get_render_device();

            m_pEditor->new_frame( m_pWindow->get_width(), m_pWindow->get_height());
            m_pEditor->update(timestep);
            m_pEditor->render(m_pRenderer->get_device_context(), m_pRenderer->get_render_device());
            m_pEditor->end_frame();

            m_pSampleApp->present();
        }
        
        Application* Application::create_application(const WindowDesc& desc)
        {
            sInstance = cyber_new<Platform::WindowsApplication>(desc);
            return sInstance;
        }
    }
}