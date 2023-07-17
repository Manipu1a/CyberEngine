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
        Application();
        virtual ~Application();

        static Application& getApp();
        virtual void run() {}
        virtual Ref<Window> getWindow() { return nullptr; }

        virtual void onEvent(Event& e) {}
    private:
        static Application* sInstance;
    };
}
