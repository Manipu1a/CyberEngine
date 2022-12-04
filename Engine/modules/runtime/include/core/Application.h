#pragma once
#include "Core.h"
#include "Window.h"
#include "CyberEvents/ApplicationEvent.h"
#include <windows.h>
namespace Cyber
{
    class CYBER_RUNTIME_API Application
    {
    public:
        Application(Cyber::WindowDesc& desc);
        virtual ~Application();

        static Application& getApp();
        virtual void Run() {}

        virtual void onEvent(Event& e) {}
    private:
        static Application* sInstance;
    };
}
