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
    static void glfwErrorCallback(const int error, const char *description) {
        std::cerr << "GLFW Error " << error << ": " << description << "\n";
    }

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

    Application::~Application() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

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

    void Application::BeginImGuiFrame() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetWindowDockID(), ImGui::GetMainViewport());
    }
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

    void Application::PollAndDispatchEvents() {
        glfwPollEvents();
        m_Running = !glfwWindowShouldClose(m_Window);
    }

} // namespace koala::core
