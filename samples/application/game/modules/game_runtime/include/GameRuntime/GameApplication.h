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
    class Renderer;
    
    class CYBER_GAME_API GameApplication : public Cyber::Application
    {
    public:
        GameApplication();
        virtual ~GameApplication();

        virtual void initialize(Cyber::WindowDesc& desc);
        virtual void Run() override;
        virtual void onEvent(Event& e);

        virtual Ref<Window> getWindow() override { return mWindow; }

    private:
        void create_window(Cyber::WindowDesc& desc);
        bool onWindowClose(WindowCloseEvent& e);
    private:
        Ref<Window> mWindow;
        Ref<InputSystem> mInputSystem;
        Scope<Renderer> renderer;
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;
    };
}
