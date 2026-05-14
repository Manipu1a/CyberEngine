#include <iostream>
#include "log/Log.h"
#include "platform/memory.h"
#include "gameruntime/sampleapp.h"
#include "gameruntime/sample_registry.h"
#include "sample_selector.h"

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR    lpCmdLine,
                     int       nCmdShow)
{
    //AllocConsole();
    SetConsoleTitle(L"Console");
    //std::cout << "test log" << std::endl;
    Cyber::Log::initLog();
    CB_CORE_INFO("initLog");

    const auto* entries = Cyber::Samples::sample_registry();
    int count = Cyber::Samples::sample_registry_count();
    int idx = show_sample_selector(hInstance, entries, count);
    if (idx < 0)
    {
        CB_CORE_INFO("Sample selector cancelled or no samples available, exiting.");
        FreeConsole();
        return 0;
    }

    Cyber::WindowDesc desc;
    desc.title = L"Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 800;
    desc.hInstance = hInstance;
    desc.cmdShow = nCmdShow;
    Cyber::Core::Application* app = Cyber::Core::Application::create_application(desc);
    app->set_sample_app(entries[idx].factory());
    app->initialize();
    app->run();

    FreeConsole();
    return 1;
}
