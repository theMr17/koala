#pragma once
#include <memory>
#include "layer.h"
#include "layer_stack.h"

struct GLFWwindow;

namespace koala::core {
    class Application {
    public:
        Application();
        virtual ~Application();
        void Run();
        void PushLayer(const std::shared_ptr<Layer> &layer) { m_LayerStack.PushLayer(layer); }
        void PushOverlay(const std::shared_ptr<Layer> &layer) { m_LayerStack.PushOverlay(layer); }
        GLFWwindow *GetWindowHandle() const { return m_Window; }

    protected:
        virtual void OnInit() {}

    private:
        void BeginImGuiFrame();
        void EndImGuiFrame();
        void PollAndDispatchEvents();
        GLFWwindow *m_Window = nullptr;
        LayerStack m_LayerStack;
        bool m_Running = true;
    };


    Application *CreateApplication();
} // namespace koala::core
