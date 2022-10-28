#include <iostream>

#include "Core/Core.h"
#include <Platform/WindowsWindow.h>
#include <Core/Application.h>
#include <Module/Log/Log.h>
#include <Module/Memory/Memory.h>


class ClassTest
{
    public:
    ClassTest(int t)
    {
        a = t;
    }

    int a;
};

int main(int argc, char** argv)
{
    Cyber::Log::initLog();

    ClassTest* test = new ClassTest(10);
    if(test)
    {
        CB_CORE_INFO("test{0}", test->a);
    }

    CB_CORE_INFO("Cyber Engine");
    Cyber::WindowDesc desc;
    desc.title = "Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 720;
    Cyber::Application app(desc);
    app.Run();


    
    return 0;
}