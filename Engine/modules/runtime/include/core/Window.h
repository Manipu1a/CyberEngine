#pragma once
#include "cyber_runtime.config.h"
#include "core.h"
#include "EASTL/vector.h"
#include "CyberEvents/Event.h"
#include <windows.h>

namespace Cyber
{
    struct RectDesc
    {
        int32_t left;
        int32_t top;
        int32_t right;
        int32_t bottom;
    };

    struct CYBER_RUNTIME_API WindowDesc
    {
        eastl::wstring title;
        HWND handle;
        HINSTANCE hInstance = nullptr;
        int cmdShow;
        RectDesc windowedRect;
        RectDesc fullscreenRect;
        RectDesc clientRect;
        
        bool fullScreen;
        bool cursorCaptured;
        bool hide;

        uint32_t mWndX;
        uint32_t mWndY;
        uint32_t mWndW;
        uint32_t mWndH;

        bool mCursorHidden;

        WindowDesc(eastl::wstring title,
                    uint32_t width = 1280, 
                    uint32_t height = 720)
                    :title(title), mWndW(width), mWndH(height)
                    {
                    }

        WindowDesc() {}
    };

    struct CYBER_RUNTIME_API PlatformRect
    {
        int32_t left;
        int32_t top;
        int32_t right;
        int32_t bottom;

        PlatformRect() {}
        PlatformRect(int32_t left, int32_t top, int32_t right, int32_t bottom)
            :left(left), top(top), right(right), bottom(bottom)
        {
        }

        bool operator==(const PlatformRect& other) const
        {
            return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
        }
    };

    struct CYBER_RUNTIME_API MonitorInfo
    {
        eastl::string name;
        eastl::string id;
        int32_t native_width;
        int32_t native_height;
        PlatformRect display_rect;
        PlatformRect work_area;
        bool is_primary;
        int32_t dpi;
        int32_t physical_width;
        int32_t physical_height;
    };

    struct CYBER_RUNTIME_API DisplayMetrics
    {
        int32_t primary_monitor_width;
        int32_t primary_monitor_height;
        eastl::vector<MonitorInfo> monitors;
        PlatformRect primary_monitor_display_rect;
        PlatformRect virtual_display_rect;
    };

    class CYBER_RUNTIME_API Window
    {
    public:
        using EventCallbackFn = eastl::function<void(Event&)>;
        Window() = default;
        virtual ~Window() = default;
        virtual void initialize_window(const Cyber::WindowDesc& desc) = 0;
        virtual void update(float deltaTime) = 0;
        virtual void close() = 0;
        
        virtual uint32_t get_width() const = 0;
        virtual uint32_t get_height() const = 0;
		virtual HWND get_native_window() const = 0;
        virtual void set_event_callback(const EventCallbackFn& callback) = 0;
        virtual void rebuild_display_metrics(DisplayMetrics& outMetrics) = 0;
    };
}
