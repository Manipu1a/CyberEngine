#pragma once
#include "cyber_game.config.h"
#include "core/Core.h"
#include "core/Application.h"
#include "core/Window.h"
#include "CyberEvents/ApplicationEvent.h"
#include "CyberInputSystem/InputSystem.h"
#include <windows.h>

namespace Cyber
{
    class CYBER_GAME_API GameApplication : public Cyber::Application
    {
    public:
        GameApplication(Cyber::WindowDesc& desc);
        virtual ~GameApplication();

        virtual void Run();

        virtual void onEvent(Event& e);
    private:
        bool onWindowClose(WindowCloseEvent& e);
    private:
        Scope<Window> mWindow;
        Scope<InputSystem> mInputSystem;
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;
    };
}
