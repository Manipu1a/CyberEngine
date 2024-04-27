#include "core/Application.h"
#include "platform/windows/windows_application.h"
#include "platform/memory.h"
#include "inputsystem/InputSystem.h"
#include "core/Timestep.h"
#include "core/window.h"
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

        Application& Application::getApp()
        {
            if(sInstance == nullptr)
            {
                cyber_assert(false, "Application is not created!");
            }
            return *sInstance;
        }

        void Application::initialize()
        {
            m_pInputSystem = cyber_new<InputSystem>();
            m_pInputSystem->initInputSystem();
        }
        void Application::run()
        {
            while(mRunning)
            {
                float time = 0.0f;
                Timestep timestep = time - mLastFrameTime;
                mLastFrameTime = time;

                //spdlog::info("Time: {0}", timestep);

                m_pInputSystem->updateInputSystem(m_pWindow->get_native_window());
                m_pWindow->update(timestep);
                update(timestep);
            }
        }

        void Application::update(float deltaTime)
        {

        }

        Application& Application::create_application(const WindowDesc& desc)
        {
            sInstance = cyber_new<Platform::WindowsApplication>(desc);
            return *sInstance;
        }
    }
}