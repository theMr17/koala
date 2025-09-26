#pragma once
#include <string>
#include <utility>

/**
 * Represents a configurable application layer that provides lifecycle and
 * event hooks which derived classes can override.
 *
 * All hook methods have default empty implementations so derived layers may
 * override only the callbacks they need.
 */

/**
 * Constructs a Layer with the given name.
 * @param name Layer name to store (defaults to "Layer").
 */

/**
 * Called when the layer is attached to the layer stack.
 */

/**
 * Called when the layer is detached from the layer stack.
 */

/**
 * Called once per update with the elapsed time since the previous update.
 * @param dt Time delta in seconds since the last update.
 */

/**
 * Called when the layer should perform rendering.
 */

/**
 * Handles an incoming event dispatched to this layer.
 * @param e Reference to the event to handle.
 */

/**
 * Retrieve the layer's name.
 * @returns The layer's stored name.
 */
namespace koala::core {
    class Event;

    class Layer {
    public:
        explicit Layer(std::string name = "Layer") : m_Name(std::move(name)) {}
        virtual ~Layer() = default;
        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnRender() {}
        virtual void OnEvent(Event &e) {}
        const std::string &GetName() const { return m_Name; }

    private:
        std::string m_Name;
    };
} // namespace koala::core
