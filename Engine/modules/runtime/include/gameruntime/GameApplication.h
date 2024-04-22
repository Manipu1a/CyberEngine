#pragma once
#include "cyber_game.config.h"
#include "core/Core.h"
#include "core/Application.h"
#include "core/Window.h"
#include "graphics/interface/graphics_types.h"
#include "CyberEvents/ApplicationEvent.h"
#include "inputsystem/InputSystem.h"
#include <windows.h>
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/device_context.h"


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

        virtual void create_gfx_objects();
    private:
        void create_window(Cyber::WindowDesc& desc);
        bool onWindowClose(WindowCloseEvent& e);
        void create_render_device(GRAPHICS_BACKEND backend);
    protected:
        Ref<Window> mWindow;
        Ref<InputSystem> mInputSystem; 
        bool mRunning = true;
        bool mMinimized = false;
        float mLastFrameTime = 0.0f;

        ///-------------------------------------
        static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
        static const uint32_t BACK_BUFFER_COUNT = 3;
        RenderObject::IRenderDevice* m_pRenderDevice = nullptr;
        RenderObject::IInstance* m_pInstance = nullptr;
        RenderObject::IAdapter* m_pAdapter = nullptr;
        RenderObject::IFence* m_pPresentFence = nullptr;
        RenderObject::IQueue* m_pQueue = nullptr;
        RenderObject::ICommandPool* m_pPool = nullptr;
        RenderObject::ICommandBuffer* m_pCmd = nullptr;
        RenderObject::ISwapChain* m_pSwapChain = nullptr;
        RenderObject::IRenderPass* m_pRenderPass = nullptr;
        Surface* m_pSurface = nullptr;
        RenderObject::IFence* m_pPresentSwmaphore = nullptr; 
        uint32_t m_backBufferIndex = 0;
    };
}
