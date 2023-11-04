#include "platform/WindowsWindow.h"
#include "CyberEvents/ApplicationEvent.h"
#include "CyberLog/Log.h"
#include "core/Application.h"
#include <windef.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Cyber
{

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        SetProcessDPIAware();
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

    	mData.mWindowDesc.handle = CreateWindow(
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

        if(!mData.mWindowDesc.handle)
        {
            HRESULT hr = HRESULT_FROM_WIN32( GetLastError() );
            cyber_core_assert(false, "Call to CreateWindow failed!{0}", GetLastError());
        }

        DisplayMetrics metrics;
        rebuildDisplayMetrics(metrics);

        ShowWindow(mData.mWindowDesc.handle, mData.mWindowDesc.cmdShow);
	    UpdateWindow(mData.mWindowDesc.handle);
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
        return mData.mWindowDesc.mWndW;
    }
    uint32_t WindowsWindow::getHeight() const
    {
        return mData.mWindowDesc.mWndH;
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

    void WindowsWindow::rebuildDisplayMetrics(DisplayMetrics& outMetrics)
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