#include "core/Application.h"
#include "platform/windows_application.h"
#include "platform/memory.h"
namespace Cyber
{
    Application* Application::sInstance = nullptr;

    Application::Application()
    {
        if(sInstance == nullptr)
            sInstance = this;
    }

    Application::~Application()
    {
        
    }

    Application& Application::getApp()
    {
        if(sInstance == nullptr)
        {
            cyber_assert(false, "Application is not created!");
        }
        return *sInstance;
    }

    Application& Application::create_application(const WindowDesc& desc)
    {
        sInstance = cyber_new<WindowsApplication>(desc);
        return *sInstance;
    }
}