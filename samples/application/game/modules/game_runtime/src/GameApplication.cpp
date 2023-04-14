#include "GameRuntime/GameApplication.h"
#include "core/Timestep.h"
#include "renderer/renderer.h"

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
        renderer = CreateScope<Renderer>();
        
        RendererDesc renderer_desc;
        renderer->initialize(this, renderer_desc);

        mInputSystem = CreateRef<InputSystem>();
        mInputSystem->initInputSystem();
    }

    void GameApplication::create_window(Cyber::WindowDesc& desc)
    {
        mWindow = Cyber::Window::createWindow(desc);
        mWindow->setEventCallback(CB_BIND_EVENT_FN(Application::onEvent));
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