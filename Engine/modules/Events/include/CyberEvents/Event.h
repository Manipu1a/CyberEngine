#pragma once
#include "core/core.h"
#include "platform/configure.h"

namespace Cyber
{
    enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication    = BIT(0),
		EventCategoryInput          = BIT(1),
		EventCategoryKeyboard       = BIT(2),
		EventCategoryMouse          = BIT(3),
		EventCategoryMouseButton    = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
                                virtual EventType getEventType() const override { return getStaticType(); }\
                                virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }

    class Event
    {
    public:
        bool Handled = false;

        virtual EventType getEventType() const = 0;
        virtual const char* getName() const = 0;
        virtual int getCategoryFlags() const = 0;
        virtual eastl::string toString() const { return getName(); }

        CYBER_FORCE_INLINE bool isInCategory(EventCategory category)
        {
            return getCategoryFlags() & category;
        }
    };

    class EventDispatcher
    {
    public:
        EventDispatcher(Event& event)
            :mEvent(event)
        {

        }

        template<typename T, typename F>
        bool Dispatch(const F& func)
        {
            if(mEvent.getEventType() == T::getStaticType())
            {
                mEvent.Handled = func(static_cast<T&>(mEvent));
                return true;
            }
        }
    private:
        Event& mEvent;
    };
}