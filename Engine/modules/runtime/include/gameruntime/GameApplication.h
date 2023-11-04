#pragma once
#include "cyber_game.config.h"
#include "core/Core.h"
#include "core/Application.h"
#include "core/Window.h"
#include "rhi/rhi.h"
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

    protected:
        ///-------------------------------------
        static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
        static const uint32_t BACK_BUFFER_COUNT = 3;
        RHIDevice* device = nullptr;
        RHIInstance* instance = nullptr;
        RHIAdapter* adapter = nullptr;
        RHIFence* present_fence = nullptr;
        RHIQueue* queue = nullptr;
        RHICommandPool* pool = nullptr;
        RHICommandBuffer* cmd = nullptr;
        RHISwapChain* swap_chain = nullptr;
        RHISurface* surface = nullptr;
        RHIFence* present_swmaphore = nullptr; 
        uint32_t backbuffer_index = 0;
    };
}
