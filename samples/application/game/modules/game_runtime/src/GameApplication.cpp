#include "GameRuntime/GameApplication.h"
#include "core/Timestep.h"

namespace Cyber
{

    GameApplication::GameApplication(Cyber::WindowDesc& desc) : Cyber::Application(desc)
    {
        mWindow = Cyber::Window::createWindow(desc);
        mWindow->setEventCallback(CB_BIND_EVENT_FN(Application::onEvent));
        mInputSystem = CreateScope<InputSystem>();
        mInputSystem->initInputSystem();
    }

    GameApplication::~GameApplication()
    {

    }

    void GameApplication::Run()
    {
        while(mRunning)
        {
            float time = 0.0f;
            Timestep timestep = time - mLastFrameTime;
            mLastFrameTime = time;

            //spdlog::info("Time: {0}", timestep);

            mInputSystem->updateInputSystem(mWindow->getNativeWindow());
            mWindow->onUpdate(timestep);
        }
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

}