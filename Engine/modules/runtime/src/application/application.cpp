#include "application/application.h"
#include "application/platform/windows/windows_application.h"
#include "platform/memory.h"
#include "core/Timestep.h"
#include "core/window.h"
#include "gameruntime/sampleapp.h"
#include "editor/editor_impl_win32.h"
#include "inputsystem/core/input_manager.h"

namespace Cyber
{
    namespace Core
    {
        Application* Application::sInstance = nullptr;

        Application::Application(const WindowDesc& desc) : m_pNewInputManager(nullptr), m_pRenderer(nullptr), m_pSampleApp(nullptr), m_pEditor(nullptr)
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

        }

        void Application::on_window_create(HWND hwnd, uint32_t width, uint32_t height)
        {
            CB_CORE_INFO("Application :: on_window_create() - width: {0}, height: {1}", width, height);

            m_pRenderer = cyber_new<Renderer::Renderer>();
            m_pRenderer->create_gfx_objects();
            //m_pInputSystem = cyber_new<InputSystem>();
            //m_pInputSystem->initInputSystem();
            
            // Initialize new input system
            m_pNewInputManager = Input::InputManager::get_instance();
            m_pNewInputManager->initialize(hwnd);
            m_pNewInputManager->set_window_size(width, height);
            Editor::EditorCreateInfo createInfo = {};
            createInfo.application = this;
            createInfo.pDevice = m_pRenderer->get_render_device();
            createInfo.swap_chain = m_pRenderer->get_swap_chain();
            createInfo.BackBufferFmt = TEX_FORMAT_RGB32_FLOAT;
            createInfo.DepthBufferFmt = TEX_FORMAT_D32_FLOAT;
            createInfo.Hwnd = hwnd;
            m_pEditor = Editor::Editor_Impl_Win32::create(createInfo);
            m_pEditor->initialize(createInfo.pDevice, createInfo.Hwnd);
            
            // Only initialize sample app if it exists
            if(m_pSampleApp) {
                m_pSampleApp->initialize();
            }
        }

        void Application::resize_window(uint32_t width, uint32_t height)
        {
            CB_CORE_INFO("Application :: resize_window() - width: {0}, height: {1}", width, height);
            if(m_pWindow)
            {
                m_pRenderer->get_swap_chain()->resize(width, height);
            }
            else
            {
                CB_CORE_ERROR("Application :: resize_window() - Window is not created!");
            }
        }

        void Application::run()
        {
            CB_CORE_INFO("Application :: run()");
            CB_CORE_INFO("Application :: run() - finished");
        }

        void Application::update(float deltaTime)
        {
            float time = 0.0f;
            // Update new input system
            if (m_pNewInputManager) {
                m_pNewInputManager->update(deltaTime);
            }
            m_pWindow->update(deltaTime);
            m_pEditor->new_frame( m_pWindow->get_width(), m_pWindow->get_height());

            m_pRenderer->update(deltaTime);
            m_pSampleApp->update(deltaTime);

            m_pEditor->update(deltaTime);
            m_pSampleApp->draw_ui(m_pEditor->get_imgui_context());
            m_pEditor->render(m_pRenderer->get_device_context(), m_pRenderer->get_render_device());
            //m_pEditor->end_frame();

            m_pSampleApp->present();
        }
        
        Application* Application::create_application(const WindowDesc& desc)
        {
            sInstance = cyber_new<Platform::WindowsApplication>(desc);
            return sInstance;
        }
    }
}