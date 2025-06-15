#include "application/platform/windows/windows_window.h"
#include "application/platform/windows/windows_application.h"

namespace Cyber
{
    namespace Platform
    {
        WindowsWindow::WindowsWindow(const Cyber::WindowDesc& desc)
        {
            mData.mWindowDesc = desc;
        }

        LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            PAINTSTRUCT ps;
            HDC hdc;

            if(Core::Application::getApp() != nullptr)
            {
                WindowsApplication* app = static_cast<WindowsApplication*>(Core::Application::getApp());
                if (app->handle_win32_message(hwnd, msg, wParam, lParam))
                    return true;
            }

            switch (msg)
            {
                case WM_DESTROY:
                    PostQuitMessage(0);
                    //mRunning = false;
                    return 0;
                    break;
                case WM_SIZE:
                    auto new_width = LOWORD(lParam); // width
                    auto new_height = HIWORD(lParam); // height
                    if (new_width != 0 && new_height != 0)
                    {
                        Core::Application::getApp()->resize_window(new_width, new_height);
                    }
                    return 0;
                    break;
            }

            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        void WindowsWindow::initialize_window()
        {
            //SetProcessDPIAware();

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

            uint32_t window_style = WS_OVERLAPPEDWINDOW;
            LONG window_width = mData.mWindowDesc.mWndW;
            LONG window_height = mData.mWindowDesc.mWndH;
            RECT rc           = {0, 0, window_width, window_height};
            AdjustWindowRect(&rc, window_style, FALSE);

            mData.mWindowDesc.handle = CreateWindowW(
                L"MainWnd",
                L"CyberEngine",
                window_style,
                CW_USEDEFAULT, CW_USEDEFAULT,
                rc.right - rc.left,
                rc.bottom - rc.top, 
                nullptr,
                nullptr,
                mData.mWindowDesc.hInstance,
                0
                );
            
            GetClientRect(mData.mWindowDesc.handle, &rc);

            mData.mWindowDesc.mWndW = rc.right - rc.left;
            mData.mWindowDesc.mWndH = rc.bottom - rc.top;
            
            cyber_log("Window created with size: {0}x{1}", mData.mWindowDesc.mWndW, mData.mWindowDesc.mWndH);

            if(!mData.mWindowDesc.handle)
            {
                HRESULT hr = HRESULT_FROM_WIN32( GetLastError() );
                cyber_core_assert(false, "Call to CreateWindow failed!{0}", GetLastError());
            }

            SetWindowPos(mData.mWindowDesc.handle, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

            //DisplayMetrics metrics;
            //rebuild_display_metrics(metrics);

            LONG_PTR currentStyle = GetWindowLongPtr(mData.mWindowDesc.handle, GWL_STYLE);
            Core::Application::getApp()->on_window_create(mData.mWindowDesc.handle, mData.mWindowDesc.mWndW, mData.mWindowDesc.mWndH);

            ShowWindow(mData.mWindowDesc.handle, mData.mWindowDesc.cmdShow);
            UpdateWindow(mData.mWindowDesc.handle);
        }

        void WindowsWindow::update(float deltaTime)
        {

        }
        void WindowsWindow::close()
        {

        }
            
        uint32_t WindowsWindow::get_width() const
        {
            return mData.mWindowDesc.mWndW;
        }
        uint32_t WindowsWindow::get_height() const
        {
            return mData.mWindowDesc.mWndH;
        }
        void WindowsWindow::set_event_callback(const EventCallbackFn& callback)
        {
            mData.mEventCallback = callback;
        }
        
        static BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC monitorDC, LPRECT rect, LPARAM userData)
        {
            MONITORINFOEX monitorInfoEX;
            monitorInfoEX.cbSize = sizeof(monitorInfoEX);
            GetMonitorInfo(monitor, &monitorInfoEX);

            MonitorInfo* Info = (MonitorInfo*)userData;
            DEVMODE dm;
            dm.dmSize = sizeof(DEVMODE);
            dm.dmDriverExtra = 0;
            EnumDisplaySettings(monitorInfoEX.szDevice, ENUM_CURRENT_SETTINGS, &dm);
            Info->native_width = dm.dmPelsWidth;
            Info->native_height = dm.dmPelsHeight;
            eastl::string name(eastl::string::CtorConvert(), eastl::basic_string<wchar_t>(monitorInfoEX.szDevice));
            if(Info->name == name)
            {
                if(monitorInfoEX.rcMonitor.right - monitorInfoEX.rcMonitor.left > Info->native_width ||
                    monitorInfoEX.rcMonitor.bottom - monitorInfoEX.rcMonitor.top > Info->native_height)
                {
                    CB_WARN("Monitor {0} is not the same size as the native resolution.", Info->name.c_str());

                    Info->display_rect.left = dm.dmPosition.x;
                    Info->display_rect.top = dm.dmPosition.y;
                    Info->display_rect.right = dm.dmPosition.x + dm.dmPelsWidth;
                    Info->display_rect.bottom = dm.dmPosition.y + dm.dmPelsHeight;

                    Info->work_area.left = Info->display_rect.left;
                    Info->work_area.top = Info->display_rect.top;
                    Info->work_area.right = Info->display_rect.right;
                    Info->work_area.bottom = Info->display_rect.bottom;
                }
                else 
                {
                    Info->display_rect.bottom = monitorInfoEX.rcMonitor.bottom;
                    Info->display_rect.left = monitorInfoEX.rcMonitor.left;
                    Info->display_rect.right = monitorInfoEX.rcMonitor.right;
                    Info->display_rect.top = monitorInfoEX.rcMonitor.top;

                    Info->work_area.bottom = monitorInfoEX.rcWork.bottom;
                    Info->work_area.left = monitorInfoEX.rcWork.left;
                    Info->work_area.right = monitorInfoEX.rcWork.right;
                    Info->work_area.top = monitorInfoEX.rcWork.top;
                }

                return FALSE;
            }

            return TRUE;
        }

        void getMonitorsInfo(eastl::vector<MonitorInfo>& outMonitorInfo)
        {
            DISPLAY_DEVICE displayDevice;
            displayDevice.cb = sizeof(DISPLAY_DEVICE);
            DWORD deviceIndex = 0;
            
            while(EnumDisplayDevices(0, deviceIndex, &displayDevice, 0))
            {
                if((displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) > 0)
                {
                    DISPLAY_DEVICE monitorDevice;
                    ZeroMemory(&monitorDevice, sizeof(DISPLAY_DEVICE));
                    monitorDevice.cb = sizeof(monitorDevice);
                    DWORD monitorIndex = 0;

                    while(EnumDisplayDevices(displayDevice.DeviceName, monitorIndex, &monitorDevice, 0))
                    {
                        if(monitorDevice.StateFlags & DISPLAY_DEVICE_ACTIVE && 
                            !(monitorDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
                        {
                            MonitorInfo monitorInfo;
                            monitorInfo.name = eastl::string(eastl::string::CtorConvert(), eastl::basic_string<wchar_t>(displayDevice.DeviceName));

                            EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&monitorInfo);
                            monitorInfo.id = eastl::string(eastl::string::CtorConvert(), eastl::basic_string<wchar_t>(monitorDevice.DeviceID));
                            monitorInfo.name = monitorInfo.id.substr(8, monitorInfo.id.find("\\", 9) - 8);

                            monitorInfo.is_primary = (monitorDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
                            
                            outMonitorInfo.push_back(monitorInfo);
                        }
                        ++monitorIndex;
                    }
                }
                ++deviceIndex;
            }
        }

        void WindowsWindow::rebuild_display_metrics(DisplayMetrics& outMetrics)
        {
            auto res = IsProcessDPIAware();

            outMetrics.primary_monitor_width = GetSystemMetrics(SM_CXSCREEN);
            outMetrics.primary_monitor_height = GetSystemMetrics(SM_CYSCREEN);

            RECT workAreaRect;
            if(!SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0))
            {
                workAreaRect.top = workAreaRect.bottom = workAreaRect.left = workAreaRect.right = 0;
            }

            outMetrics.primary_monitor_display_rect.left = workAreaRect.left;
            outMetrics.primary_monitor_display_rect.top = workAreaRect.top;
            outMetrics.primary_monitor_display_rect.right = workAreaRect.right;
            outMetrics.primary_monitor_display_rect.bottom = workAreaRect.bottom;

            getMonitorsInfo(outMetrics.monitors);
        }
    }
}