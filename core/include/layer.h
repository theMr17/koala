#pragma once
#include <string>
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
