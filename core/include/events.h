#pragma once
#include <functional>

namespace koala::core {

    enum class EventType {
        None = 0,
        WindowClose,
        WindowResize,
        KeyDown,
        KeyUp,
        Char,
        MouseButtonDown,
        MouseButtonUp,
        MouseMove,
        MouseScroll
    };

    class Event {
    public:
        virtual ~Event() = default;
        virtual EventType GetType() const = 0;
        bool handled = false;
    };

    struct WindowCloseEvent : Event {
        EventType GetType() const override { return EventType::WindowClose; }
    };

    struct WindowResizeEvent : Event {
        int width, height;
        EventType GetType() const override { return EventType::WindowResize; }
    };


    class EventDispatcher {
    public:
        explicit EventDispatcher(Event &e) : m_Event(e) {}
        template<typename T, typename F>
        bool Dispatch(const F &func) {
            if (m_Event.GetType() == T{}.GetType()) {
                m_Event.handled = func(static_cast<T &>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event &m_Event;
    };

} // namespace koala::core
