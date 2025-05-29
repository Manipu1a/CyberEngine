#include <iostream>
#include "CyberLog/Log.h"
#include "platform/memory.h"
#include "core/Application.h"
#include "triangle.h"

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
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CON", "r", stdin);
    freopen_s(&stream, "CON", "w", stdout);
    SetConsoleTitle(L"Console");
    //std::cout << "test log" << std::endl;
    Cyber::Log::initLog();
    CB_CORE_INFO("initLog");

    Cyber::WindowDesc desc;
    desc.title = L"Cyber";
    desc.mWndW = 1280;
    desc.mWndH = 720;
    desc.hInstance = hInstance;
    desc.cmdShow = nCmdShow;
    Cyber::Core::Application* app = Cyber::Core::Application::create_application(desc);
    app->set_sample_app(Cyber::create_sample_app());
    app->initialize();
    app->run();

    FreeConsole();
    return 1;
}
 