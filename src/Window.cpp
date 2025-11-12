#include "Application.hpp"

#include <GLFW/glfw3.h>

namespace Bjorn 
{
	Window::Window(uint32_t width, uint32_t height, const std::string& title, Application& app)
        : m_app(app)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_window, this);

        // Set callback function when the window gets resized
        glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
	}

    Window::~Window() { glfwDestroyWindow(m_window); }

    int Window::ShouldClose() const { return glfwWindowShouldClose(m_window); }

    void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_app.isFramebufferResized.store(true);
    }
}