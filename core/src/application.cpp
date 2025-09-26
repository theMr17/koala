#include "application.h"

#include <iostream>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

namespace koala::core {
    /**
     * @brief Logs a GLFW error message to standard error.
     *
     * @param error GLFW error code.
     * @param description Human-readable description of the error.
     */
    static void glfwErrorCallback(const int error, const char *description) {
        std::cerr << "GLFW Error " << error << ": " << description << "\n";
    }

    /**
     * @brief Constructs the application and initializes the platform, OpenGL, and ImGui subsystems.
     *
     * Initializes GLFW, configures an OpenGL 4.6 core profile context, creates the main window,
     * loads OpenGL function pointers via GLAD, and initializes Dear ImGui with keyboard navigation
     * and docking enabled using the GLFW and OpenGL3 backends.
     *
     * @note On initialization failure (GLFW, window creation, or GLAD), the function logs an error,
     *       performs necessary GLFW cleanup, and calls exit(1) to terminate the process.
     */
    Application::Application() {
        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            exit(1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        m_Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Koala", nullptr, nullptr);
        if (!m_Window) {
            std::cerr << "Failed to create window\n";
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1);

        if (!gladLoadGL()) {
            std::cerr << "Failed to initialize GLAD\n";
            glfwDestroyWindow(m_Window);
            glfwTerminate();
            exit(1);
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    /**
     * @brief Clean up ImGui and GLFW resources used by the application.
     *
     * Shuts down ImGui OpenGL3 and GLFW backends, destroys the ImGui context,
     * destroys the GLFW window, and terminates the GLFW library.
     */
    Application::~Application() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    /**
     * @brief Runs the application's main loop until termination.
     *
     * Executes the primary event-update-render cycle: polls and dispatches window events, computes
     * the frame delta time and calls each layer's OnUpdate with that delta (in seconds), begins an
     * ImGui frame, calls each layer's OnRender, displays the ImGui demo window, and finalizes the
     * ImGui frame and buffer swap. The loop continues while the application remains running.
     *
     * @note The delta time passed to layers is a float representing seconds between frames.
     */
    void Application::Run() {
        double lastTime = glfwGetTime();

        while (m_Running) {
            PollAndDispatchEvents();

            double now = glfwGetTime();
            float dt = static_cast<float>(now - lastTime);
            lastTime = now;

            for (auto &layer: m_LayerStack) {
                layer->OnUpdate(dt);
            }

            BeginImGuiFrame();

            for (auto &layer: m_LayerStack) {
                layer->OnRender();
            }

            ImGui::ShowDemoWindow();

            EndImGuiFrame();
        }
    }

    /**
     * @brief Prepares a new Dear ImGui frame and sets up a dock space covering the main viewport.
     *
     * This begins a fresh ImGui frame for the configured backends and creates a dockable area spanning
     * the application's main viewport so subsequent ImGui windows can be docked.
     */
    void Application::BeginImGuiFrame() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetWindowDockID(), ImGui::GetMainViewport());
    }
    /**
     * @brief Finalizes and presents the current ImGui frame.
     *
     * Renders ImGui draw data to the OpenGL context, updates the OpenGL viewport to match the window framebuffer size,
     * clears the color buffer with a dark background, and swaps the GLFW buffers to present the frame.
     */
    void Application::EndImGuiFrame() {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_Window);
    }

    /**
     * @brief Polls the platform event queue and updates the application's running state.
     *
     * Processes pending GLFW events and sets the internal running flag to false when the window
     * has been requested to close.
     */
    void Application::PollAndDispatchEvents() {
        glfwPollEvents();
        m_Running = !glfwWindowShouldClose(m_Window);
    }

} // namespace koala::core
