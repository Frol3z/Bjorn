#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

class Application {
public:
    void Run() {
        Init();
        MainLoop();
        CleanUp();
    }
private:
    void Init() {
        InitWindow();
        InitVulkan();
    }

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Application", nullptr, nullptr);
    }

    void InitVulkan() {
        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName = "Application",
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1,0,0),
            .apiVersion = vk::ApiVersion14
        };
        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo
        };
        m_Instance = vk::raii::Instance(m_Context, createInfo);
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(m_Window)) {
            glfwPollEvents();
        }
    }

    void CleanUp() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    GLFWwindow* m_Window;
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
};

int main() {
    Application app;

    try {
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
