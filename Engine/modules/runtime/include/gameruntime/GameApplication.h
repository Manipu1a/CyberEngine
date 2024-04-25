#pragma once
#include "core/core.h"
#include "core/Application.h"
#include "graphics/interface/graphics_types.h"
#include "CyberEvents/ApplicationEvent.h"
#include "inputsystem/InputSystem.h"
#include "graphics/interface/render_device.hpp"
#include "cyber_runtime.config.h"
namespace Cyber
{
    class Renderer;
    
    class CYBER_RUNTIME_API GameApplication : public Cyber::Application
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
        
    protected:
        Ref<Window> mWindow;
        Ref<InputSystem> mInputSystem; 
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;
    };
    
}
