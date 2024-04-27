#include "platform/windows/windows_application.h"
#include "CyberEvents/ApplicationEvent.h"
#include "CyberLog/Log.h"
#include <windef.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "platform/windows/windows_window.h"
#include "platform/memory.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Cyber
{
    namespace Platform
    {
        WindowsApplication::WindowsApplication(const Cyber::WindowDesc& desc) : Application(desc)
        {
            create_window(desc);
        }
        WindowsApplication::~WindowsApplication()
        {

        }

        LRESULT CALLBACK WindowsApplication::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            PAINTSTRUCT ps;
            HDC hdc;

            if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
                return true;

            switch (msg)
            {
                case WM_DESTROY:
                    PostQuitMessage(0);
                    //mRunning = false;
                    break;
                case WM_SIZE:

                    break;
                default:
                    return DefWindowProc(hwnd, msg, wParam, lParam);
                    break;
            }

            return 0;
        }

        void WindowsApplication::run()
        {

        }
        void WindowsApplication::update(float deltaTime)
        {

        }
        void WindowsApplication::onEvent(Event& e)
        {

        }

        void WindowsApplication::create_window(const Cyber::WindowDesc& desc)
        {
            m_pWindow = cyber_new<WindowsWindow>();
            m_pWindow->initialize_window(desc);
        }
    }
}
    