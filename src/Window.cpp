#include "Window.hpp"

#include "Application.hpp"

#include <GLFW/glfw3.h>

namespace Felina 
{
	Window::Window(uint32_t width, uint32_t height, const std::string& title, Application& app)
        : m_app(app), m_width(width), m_height(height)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(m_window, this);

        // Set callback function when the window gets resized
        glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
	}

    Window::~Window() 
    {
        glfwDestroyWindow(m_window); 
    }

    int Window::ShouldClose() const { return glfwWindowShouldClose(m_window); }

    void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->SetWidth(static_cast<uint32_t>(width));
        win->SetHeight(static_cast<uint32_t>(height));
        win->m_app.isFramebufferResized.store(true);
    }
}