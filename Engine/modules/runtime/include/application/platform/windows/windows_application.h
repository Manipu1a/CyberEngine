#pragma once
#include "application/application.h"
#include "cyber_runtime.config.h"

namespace Cyber
{
    namespace Platform
    {
        class CYBER_RUNTIME_API WindowsApplication : public Core::Application
        {
        public:
            WindowsApplication(const WindowDesc& desc);
            virtual ~WindowsApplication();

            virtual void initialize() override;
            virtual void run() override;
            virtual void update(float deltaTime)override;
            virtual void onEvent(Event& e)override;
            virtual void create_window(const Cyber::WindowDesc& desc) override;

            LRESULT CALLBACK handle_win32_message(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        };
        
        #if defined(CYBER_RUNTIME_PLATFORM_WINDOWS)
            #define PlatformApplication WindowsApplication
        #endif
    }

}
