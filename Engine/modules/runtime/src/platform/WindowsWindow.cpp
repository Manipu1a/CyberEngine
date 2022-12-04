#include "platform/WindowsWindow.h"
#include "CyberEvents/ApplicationEvent.h"
#include "CyberLog/Log.h"
#include "core/Application.h"

namespace Cyber
{
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
	    HDC hdc;

	    switch (msg)
	    {
		    case WM_DESTROY:
		    	PostQuitMessage(0);
		    	//mRunning = false;
		    	break;
		    default:
		    	return DefWindowProc(hwnd, msg, wParam, lParam);
		    	break;
	    }

	    return 0;
    }

    WindowsWindow::WindowsWindow(const Cyber::WindowDesc& desc)
    {
        initWindow(desc);
    }

    void WindowsWindow::initWindow(const Cyber::WindowDesc& desc)
    {
        mData.mWindowDesc = desc;

        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style			= CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc	= WndProc;
        wcex.cbClsExtra		= 0;
        wcex.cbWndExtra		= 0;
        wcex.hInstance		= mData.mWindowDesc.hInstance;
        wcex.hIcon          = 0;
        wcex.hIconSm		= 0;
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	    wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wcex.lpszClassName	= L"MainWnd";
        wcex.lpszMenuName   = NULL;

        if (!RegisterClassEx(&wcex))
        {
            CB_CORE_ERROR("Call to RegisterClassEx failed!");
            return;
        }

    	mData.mWindowDesc.hWnd = CreateWindow(
			L"MainWnd",
			L"win32app",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			mData.mWindowDesc.mWndW, mData.mWindowDesc.mWndH,
			0,
			0,
			mData.mWindowDesc.hInstance,
			0
			);

        if(!mData.mWindowDesc.hWnd)
        {
            HRESULT hr = HRESULT_FROM_WIN32( GetLastError() );
            CB_CORE_ASSERTS(false, "Call to CreateWindow failed!{0}", GetLastError());
        }

        ShowWindow(mData.mWindowDesc.hWnd, mData.mWindowDesc.cmdShow);
	    UpdateWindow(mData.mWindowDesc.hWnd);


    }
    
    void WindowsWindow::onUpdate(float deltaTime)
    {
        /*
        MSG msg;
        while (PeekMessage(&msg, mData.mWindowDesc.hWnd,  0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
        */
    }

    void WindowsWindow::onClose()
    {
        
    }

    uint32_t WindowsWindow::getWidth() const
    {
        return 0;
    }
    uint32_t WindowsWindow::getHeight() const
    {
        return 0;
    }
}