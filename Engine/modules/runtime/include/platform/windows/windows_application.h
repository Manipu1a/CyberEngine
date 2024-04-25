#pragma once
#include "cyber_runtime.config.h"
#include "core/application.h"

namespace Cyber
{
    class WindowsApplication : public Core::Application
    {
    public:
        virtual void run() override;
        virtual void update(float deltaTime)override;
        virtual void onEvent(Event& e)override;
    };
    
#if defined(CYBER_RUNTIME_PLATFORM_WINDOWS)
    #define PlatformApplication WindowsApplication
#endif
}
