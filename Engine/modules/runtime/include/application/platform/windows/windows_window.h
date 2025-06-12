#pragma once
#include "core/window.h"
#include "cyber_runtime.config.h"

namespace Cyber
{
    namespace Platform
    {
        class WindowsWindow : public Window
        {
        public:
            WindowsWindow();

            virtual void initialize_window(const Cyber::WindowDesc& desc);
            virtual void update(float deltaTime);
            virtual void close();
            
            virtual uint32_t get_width() const;
            virtual uint32_t get_height() const;
            virtual void set_event_callback(const EventCallbackFn& callback);
            virtual void rebuild_display_metrics(DisplayMetrics& outMetrics);

            void initRHI();
            inline virtual HWND get_native_window() const override{return mData.mWindowDesc.handle;}
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

}
