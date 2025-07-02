#include <iostream>
#include "CyberLog/Log.h"
#include "platform/memory.h"
#include "gameruntime/sampleapp.h"

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

    Cyber::WindowDesc desc;
    desc.title = L"Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 800;
    desc.hInstance = hInstance;
    desc.cmdShow = nCmdShow;
    Cyber::Core::Application* app = Cyber::Core::Application::create_application(desc);
    app->set_sample_app(Cyber::Samples::SampleApp::create_sample_app());
    app->initialize();
    app->run();

    FreeConsole();
    return 1;
}
 