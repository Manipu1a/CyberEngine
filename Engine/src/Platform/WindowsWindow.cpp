#include "Platform/WindowsWindow.h"
#include "Events/ApplicationEvent.h"

namespace Cyber
{
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{

	}

    WindowsWindow::WindowsWindow(const Cyber::WindowDesc& desc)
    {
        initWindow(desc);


    }

    void WindowsWindow::initWindow(const Cyber::WindowDesc& desc)
    {
        mData.mTitle = desc.title;
        mData.mWidth = desc.mWndW;
        mData.mHeight = desc.mWndH;
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        mWindow = glfwCreateWindow(640, 480,"Cyber Engine", NULL, NULL);
        GLFWwindow* glfwWindow = (GLFWwindow*)mWindow;

        glfwSetWindowUserPointer(glfwWindow, &mData);

        glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            Cyber::WindowCloseEvent event;
            data.mEventCallback(event);
        });

        glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);
    }
    
    void WindowsWindow::onUpdate(float deltaTime)
    {
		glfwPollEvents();
    }

    void WindowsWindow::onClose()
    {
        glfwDestroyWindow((GLFWwindow*)mWindow);
        glfwTerminate();
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