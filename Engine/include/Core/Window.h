#pragma once

#include "cepch.h"
#include "Core/Core.h"
#include "Events/Event.h"
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

    struct WindowDesc
    {
        eastl::wstring title;
        void* window;
        HWND hWnd;
        HINSTANCE hInstance = nullptr;
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


    class Window
    {
    public:
        using EventCallbackFn = eastl::function<void(Event&)>;
        virtual ~Window() = default;

        virtual void onUpdate(float deltaTime) = 0;
        virtual void onClose() = 0;
        
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

		virtual void* getNativeWindow() const = 0;
        
        virtual void setEventCallback(const EventCallbackFn& callback) = 0;

        static Scope<Cyber::Window> createWindow(const WindowDesc& desc);
    };
}