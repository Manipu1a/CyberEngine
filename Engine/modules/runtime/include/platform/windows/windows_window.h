#pragma once
#include "core/window.h"
#include "cyber_runtime.config.h"

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
        virtual void rebuildDisplayMetrics(DisplayMetrics& outMetrics);
        
        inline virtual void setEventCallback(const EventCallbackFn& callback) override { mData.mEventCallback = callback; }

        void initWindow(const Cyber::WindowDesc& desc);
        void initRHI();
        
        inline virtual HWND getNativeWindow() const {return mData.mWindowDesc.handle;}
    private:
    
    private:
        struct WindowData
        {
            WindowDesc mWindowDesc;
            EventCallbackFn mEventCallback;
        };

        WindowData mData;

    };
}
