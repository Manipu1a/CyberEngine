#pragma once

#include "Event.h"

namespace Cyber
{
    class WindowResizeEvent : public Event 
    {
    public:
        WindowResizeEvent(uint32_t width, uint32_t height)
            : mWidth(width), mHeight(height) {}

        virtual eastl::string toString() const override
        {
            eastl::string str(typename eastl::string::CtorSprintf(), ("WindowResizeEvent: %d, %d"), mWidth, mHeight);
            return str;
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        uint32_t mWidth, mHeight;
    };
    

    class WindowCloseEvent : public Event 
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };


}