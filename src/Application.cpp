#include "Application.hpp"

#include "GLFW/glfw3.h"

namespace Bjorn 
{
	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight)
	{
		Init();

		m_window = std::make_unique<Window>(windowWidth, windowHeight, name, this);
		m_renderer = std::make_unique<Renderer>(name, *m_window);
	}

	void Application::Run() {
		Init();
		MainLoop();
		CleanUp();
	}

	void Application::Init() {
		glfwInit();
	}

	void Application::MainLoop() {
		
	}

	void Application::CleanUp() {
		glfwTerminate();
	}
}