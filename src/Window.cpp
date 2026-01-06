#include "Window.hpp"

#include <GLFW/glfw3.h>

namespace Felina 
{
	Window::Window(uint32_t width, uint32_t height, const std::string& title)
        : m_width(width), m_height(height)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        // Additional configurations
        //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

    Window::~Window() 
    {
        glfwDestroyWindow(m_window); 
    }

    bool Window::ShouldClose() const { return glfwWindowShouldClose(m_window) != 0; }
}