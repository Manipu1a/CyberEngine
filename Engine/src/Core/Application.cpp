#include "Core/Application.h"
#include "Core/Timestep.h"

#include <glfw/glfw3.h>
namespace Cyber
{
    Application::Application(Cyber::WindowDesc& desc)
    {
        mWindow = Cyber::Window::createWindow(desc);
        mWindow->setEventCallback(CB_BIND_EVENT_FN(Application::onEvent));
        mInputSystem = CreateScope<InputSystem>();
        mInputSystem->initInputSystem(mWindow->getNativeWindow());
    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        while(mRunning)
        {
            float time = (float)glfwGetTime();
            Timestep timestep = time - mLastFrameTime;
            mLastFrameTime = time;

            //spdlog::info("Time: {0}", timestep);

            mInputSystem->updateInputSystem(mWindow->getNativeWindow());
            mWindow->onUpdate(timestep);
        }
    }

    void Application::onEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(CB_BIND_EVENT_FN(Application::onWindowClose));
    }

    bool Application::onWindowClose(WindowCloseEvent& e)
    {
        mRunning = false;
        return true;
    }
}