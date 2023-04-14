#include "core/Application.h"

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
        return *sInstance;
    }
}