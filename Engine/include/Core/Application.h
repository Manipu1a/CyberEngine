#pragma once
#include "Core/Core.h"
#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "InputSystem/InputSystem.h"
#include <windows.h>
namespace Cyber
{
    class Application
    {
    public:
        Application(Cyber::WindowDesc& desc);
        virtual ~Application();

        inline static Application& getApp() { return *sInstance; }
        void Run();

        void onEvent(Event& e);

        LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        bool onWindowClose(WindowCloseEvent& e);
    private:
        Scope<Window> mWindow;
        Scope<InputSystem> mInputSystem;
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;
        static Application* sInstance;
    };
}
