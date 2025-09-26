#pragma once
#include <memory>
#include "layer.h"
#include "layer_stack.h"

struct GLFWwindow;

/**
 * Construct an Application and initialize its internal state.
 */
 
/**
 * Destroy the Application and release owned resources.
 */
 
/**
 * Enter and run the application's main loop until termination.
 */
 
/**
 * Add a layer to the application's LayerStack.
 * @param layer Shared pointer to the Layer to push onto the stack.
 */
 
/**
 * Add an overlay to the application's LayerStack.
 * @param layer Shared pointer to the Layer to push as an overlay.
 */
 
/**
 * Retrieve the native GLFW window handle owned by the application.
 * @returns Pointer to the GLFWwindow used by the application, or `nullptr` if no window is created.
 */
 
/**
 * Perform application-specific initialization steps before the main loop runs.
 */
 
/**
 * Begin a new ImGui frame and prepare ImGui state for rendering.
 */
 
/**
 * End the current ImGui frame and submit ImGui draw data for rendering.
 */
 
/**
 * Poll platform events and dispatch them to the application and its layers.
 */
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
} // namespace koala::core
