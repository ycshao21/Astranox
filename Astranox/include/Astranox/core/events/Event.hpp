#pragma once

namespace Astranox
{
    enum class EventType: uint8_t
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory: uint8_t
    {
        EventCategoryNone           = 0,
        EventCategoryApplication    = BIT(0),
        EventCategoryInput          = BIT(1),
        EventCategoryKeyboard       = BIT(2),
        EventCategoryMouse          = BIT(3),
        EventCategoryMouseButton    = BIT(4)
    };

// In each subclass of Event, these member functions must be defined.
// So, we define them as macros to avoid redundancy.
#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::##type; }\
                                virtual EventType getEventType() const override { return getStaticType(); }\
                                virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual uint8_t getCategoryFlags() const override { return category; }

    class Event
    {
    public:
        virtual EventType getEventType() const = 0;
        virtual const char* getName() const = 0;
        virtual std::string toString() const { return getName(); }

        virtual uint8_t getCategoryFlags() const = 0;

        inline virtual bool isInCategory(EventCategory category) const final { return getCategoryFlags() & category; }

        bool handled = false;
    };

    class EventDispatcher final
    {
    public:
        EventDispatcher(Event& evnt) : m_Event(evnt) {}

        template<typename T, typename F>
        bool dispatch(const F& func)
        {
            if (m_Event.getEventType() == T::getStaticType())
            {
                m_Event.handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

    ///**
    // * Overload for fmt::formatter to format Event objects.
    // * This fixes C2079 error which reports that `Event` is unformattable
    // */
    //template<typename T>
    //struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Event, T>::value, char>>
    //    : fmt::formatter<std::string>
    //{
    //    auto format(const T& evt, fmt::format_context& ctx) {
    //        return fmt::format_to(ctx.out(), "{}", evt.toString());
    //    }
    //};
}

