#pragma once
#include <cstdint>

/**
 * Abstract base for all events.
 *
 * Provides an interface to obtain the event's runtime type and a mutable
 * `handled` flag that handlers may set to indicate the event was processed.
 */

/**
 * Return the event's runtime type.
 *
 * @returns The EventType value representing this event's type.
 */

/**
 * Event emitted when a window is requested to close.
 */

/**
 * Event emitted when a window is resized.
 *
 * @note `width` and `height` contain the new size in pixels.
 */

/**
 * Dispatcher that routes a stored Event to a matching handler.
 *
 * Holds a reference to an Event and attempts to invoke a handler when the
 * handler's target type matches the stored event's type.
 */

/**
 * Construct a dispatcher for the given event.
 *
 * @param e The event to be dispatched by this dispatcher.
 */

/**
 * Invoke `func` if the stored event's type matches `T`.
 *
 * @param func Callable invoked with the stored event cast to `T&`. The callable's
 *             boolean return value will be assigned to `m_Event.handled`.
 * @returns `true` if the event type matched and the handler was invoked, `false` otherwise.
 */
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
        static constexpr EventType Type = EventType::WindowClose;
        EventType GetType() const override { return Type; }
    };

    struct WindowResizeEvent : Event {
        int width = 0, height = 0;
        WindowResizeEvent() = default;
        WindowResizeEvent(int w, int h) : width(w), height(h) {}
        static constexpr EventType Type = EventType::WindowResize;
        EventType GetType() const override { return Type; }
    };

    struct KeyDownEvent : Event {
        int key = 0;
        int scancode = 0;
        int mods = 0;
        bool repeat = false;
        static constexpr EventType Type = EventType::KeyDown;
        EventType GetType() const override { return Type; };
    };

    struct KeyUpEvent : Event {
        int key = 0;
        int scancode = 0;
        int mods = 0;
        static constexpr EventType Type = EventType::KeyUp;
        EventType GetType() const override { return Type; };
    };

    struct CharEvent : Event {
        unsigned int codepoint = 0;
        static constexpr EventType Type = EventType::Char;
        EventType GetType() const override { return Type; };
    };

    struct MouseButtonDownEvent : Event {
        int button = 0;
        int mods = 0;
        static constexpr EventType Type = EventType::MouseButtonDown;
        EventType GetType() const override { return Type; };
    };

    struct MouseButtonUpEvent : Event {
        int button = 0;
        int mods = 0;
        static constexpr EventType Type = EventType::MouseButtonUp;
        EventType GetType() const override { return Type; };
    };

    struct MouseMoveEvent : Event {
        double x = 0.0, y = 0.0;
        static constexpr EventType Type = EventType::MouseMove;
        EventType GetType() const override { return Type; };
    };

    struct MouseScrollEvent : Event {
        double dx = 0.0, dy = 0.0;
        static constexpr EventType Type = EventType::MouseScroll;
        EventType GetType() const override { return Type; };
    };

    class EventDispatcher {
    public:
        explicit EventDispatcher(Event &e) : m_Event(e) {}

        template<typename T, typename F>
        bool Dispatch(const F &func) {
            if (m_Event.GetType() == T::Type) {
                m_Event.handled = func(static_cast<T &>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event &m_Event;
    };

} // namespace koala::core
