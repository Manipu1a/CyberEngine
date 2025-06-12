#pragma once
#include "cyber_core.config.h"
#include "core/application.h"

namespace Cyber
{
    namespace Platform
    {
        class WindowsApplication : public Core::Application
        {
        public:
            WindowsApplication(const WindowDesc& desc);
            virtual ~WindowsApplication();

            virtual void run() override;
            virtual void update(float deltaTime)override;
            virtual void onEvent(Event& e)override;
            virtual void create_window(const Cyber::WindowDesc& desc) override;

            static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        };
        
        #if defined(CYBER_RUNTIME_PLATFORM_WINDOWS)
            #define PlatformApplication WindowsApplication
        #endif
    }

}
