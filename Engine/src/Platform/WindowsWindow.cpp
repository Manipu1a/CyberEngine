#include "Platform/WindowsWindow.h"
#include "Events/ApplicationEvent.h"
#include <Module/Log/Log.h>

#include <windows.h>
namespace Cyber
{

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{

	}

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
	    PAINTSTRUCT ps;
	    HDC hdc;

	    switch (message)
	    {
		    case WM_DESTROY:
		    	PostQuitMessage(0);
		    	//doExit = true;
		    	break;
		    default:
		    	return DefWindowProc(hWnd, message, wParam, lParam);
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

        //mData.mTitle = eastl::move(desc.title);
       // mData.mWidth = desc.mWndW;
        //mData.mHeight = desc.mWndH;

        /*
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        mWindow = glfwCreateWindow(mData.mWidth, mData.mHeight,"Cyber Engine", NULL, NULL);
        GLFWwindow* glfwWindow = (GLFWwindow*)mWindow;

        glfwSetWindowUserPointer(glfwWindow, &mData);

        glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            Cyber::WindowCloseEvent event;
            data.mEventCallback(event);
        });

        glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);
        */

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

        ShowWindow(mData.mWindowDesc.hWnd, 1);
	    UpdateWindow(mData.mWindowDesc.hWnd);
    }
    
    void WindowsWindow::onUpdate(float deltaTime)
    {
		//glfwPollEvents();
    }

    void WindowsWindow::onClose()
    {
        //glfwDestroyWindow((GLFWwindow*)mWindow);
        //glfwTerminate();
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