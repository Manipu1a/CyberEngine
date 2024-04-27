#pragma once
#include "Core.h"
#include <windows.h>
#include "CyberEvents/Event.h"
#include "window.h"
#include "renderer/renderer.h"
#include "cyber_runtime.config.h"

namespace Cyber
{
    class InputSystem;
    class Window;

    namespace Core
    {
        using EventCallbackFn = eastl::function<void(Event&)>;
        class CYBER_RUNTIME_API Application
        {
        public:
            Application(const WindowDesc& desc);
            virtual ~Application();

            static Application& getApp();
            static Application& create_application(const WindowDesc& desc);
            
            virtual void create_window(const Cyber::WindowDesc& desc) {}
            virtual void initialize();
            virtual void run();
            virtual void update(float deltaTime);
            virtual void onEvent(Event& e) {}

            CYBER_FORCE_INLINE Window* get_window() const { return m_pWindow; }
            CYBER_FORCE_INLINE InputSystem* get_input_system() const { return m_pInputSystem; }
            CYBER_FORCE_INLINE Renderer::Renderer* get_renderer() const { return m_pRenderer; }
        protected:
            Window* m_pWindow;
            InputSystem* m_pInputSystem;
            Renderer::Renderer* m_pRenderer;
            bool mRunning = true;
            bool mMinimized = false;
            float mLastFrameTime = 0.0f;
            static Application* sInstance;
        };
    }
}