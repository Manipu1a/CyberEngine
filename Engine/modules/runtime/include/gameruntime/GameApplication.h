#pragma once
#include "cyber_game.config.h"
#include "core/Core.h"
#include "core/Application.h"
#include "core/Window.h"
#include "graphics/rhi/rhi.h"
#include "CyberEvents/ApplicationEvent.h"
#include "inputsystem/InputSystem.h"
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
        virtual void run() override;
        virtual void update(float deltaTime) override;
        virtual void onEvent(Event& e);
        virtual Ref<Window> getWindow() override { return mWindow; }

    private:
        void create_window(Cyber::WindowDesc& desc);
        bool onWindowClose(WindowCloseEvent& e);
    private:
        Ref<Window> mWindow;
        Ref<InputSystem> mInputSystem; 
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;

    };
}
