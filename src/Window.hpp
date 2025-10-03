#pragma once

#include <string>

// Fwd declaration
struct GLFWwindow;

namespace Bjorn 
{

	// Fwd declaration
	class Application;

	class Window {
	public:
		Window(uint32_t width, uint32_t height, const std::string& title, Application* app);
		~Window();

		GLFWwindow* GetHandle() const { return m_window; }
	private:
		GLFWwindow* m_window = nullptr;
		Application* m_app;

		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
}