#pragma once
#include "core/Core.h"
#include <windows.h>
#include "CyberEvents/Event.h"
#include "core/window.h"
#include "renderer/renderer.h"
#include "editor/editor.h"
#include "cyber_runtime.config.h"

namespace Cyber
{
    class InputSystem;
    class Window;
    namespace Editor
    {
        class Editor;
    }
    namespace Samples
    {
        class SampleApp;
    }
    namespace Core
    {
        using EventCallbackFn = eastl::function<void(Event&)>;
        class CYBER_RUNTIME_API Application
        {
        public:
            Application(const WindowDesc& desc);
            virtual ~Application();

            static Application* getApp();
            static Application* create_application(const WindowDesc& desc);
            
            virtual void create_window(const Cyber::WindowDesc& desc) {}
            virtual void initialize();
            virtual void run();
            virtual void update(float deltaTime);
            virtual void onEvent(Event& e) {}

            void on_window_create(HWND hwnd, uint32_t width, uint32_t height);
            void resize_window(uint32_t width, uint32_t height);

            CYBER_FORCE_INLINE Window* get_window() const { return m_pWindow; }
            CYBER_FORCE_INLINE InputSystem* get_input_system() const { return m_pInputSystem; }
            CYBER_FORCE_INLINE Renderer::Renderer* get_renderer() const { return m_pRenderer; }
            CYBER_FORCE_INLINE Editor::Editor* get_editor() const { return m_pEditor; }
            CYBER_FORCE_INLINE Samples::SampleApp* get_sample_app() const { return m_pSampleApp; }
            CYBER_FORCE_INLINE void set_sample_app(Samples::SampleApp* sampleApp) { m_pSampleApp = sampleApp; }

            CYBER_FORCE_INLINE bool is_running() const { return m_running; }
            CYBER_FORCE_INLINE bool is_minimized() const { return m_minimized; }
            CYBER_FORCE_INLINE float get_last_frame_time() const { return m_last_frame_time; }
            CYBER_FORCE_INLINE void quit_application() { m_running = false; }
        protected:
            Window* m_pWindow;
            InputSystem* m_pInputSystem;
            Renderer::Renderer* m_pRenderer;
            Samples::SampleApp* m_pSampleApp;
            Editor::Editor* m_pEditor;
            bool m_running = true;
            bool m_minimized = false;
            float m_last_frame_time = 0.0f;
            static Application* sInstance;
        };
    }
}