#pragma once

#include "Core/Window.h"
#include <GLFW/glfw3.h>
namespace Cyber
{
    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowDesc& desc);

        virtual void onUpdate(float deltaTime) override;
        virtual void onClose() override;
        virtual uint32_t getWidth() const override;
        virtual uint32_t getHeight() const override;

        inline virtual void setEventCallback(const EventCallbackFn& callback) override { mData.mEventCallback = callback; }

        void initWindow(const Cyber::WindowDesc& desc);
        void initRHI();
        
        inline virtual void* getNativeWindow() const {return mWindow;}
    private:
        void* mWindow;
        #if WINDOWS_WINDOW
        HINSTANCE mInstance = nullptr;
        #endif
        struct WindowData
        {
            eastl::string mTitle;
            uint32_t mWidth, mHeight;

            EventCallbackFn mEventCallback;
        };

        WindowData mData;
    };
}
