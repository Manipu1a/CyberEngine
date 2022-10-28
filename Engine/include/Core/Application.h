#pragma once
#include "Core/Core.h"
#include "Window.h"
#include "Events/ApplicationEvent.h"

namespace Cyber
{
    class Application
    {
    public:
        Application(const Cyber::WindowDesc& desc);
        virtual ~Application();

        inline static Application& getApp() { return *sInstance; }
        void Run();

        void onEvent(Event& e);

    private:
        bool onWindowClose(WindowCloseEvent& e);
    private:
        Scope<Window> mWindow;
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;
        static Application* sInstance;
    };
}
