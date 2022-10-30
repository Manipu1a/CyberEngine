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

/*
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
*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR    lpCmdLine,
                     int       nCmdShow)
{
    
    Cyber::Log::initLog();
    Cyber::WindowDesc desc;
    desc.title = L"Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 720;
    desc.hInstance = hInstance;
    Cyber::Application app(desc);
    app.Run();

    return 1;
}
