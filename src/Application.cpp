#include "Application.hpp"

#include "GLFW/glfw3.h"

namespace Bjorn 
{
	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight)
		: m_name(name)
	{
		Init();

		m_window = std::make_unique<Window>(windowWidth, windowHeight, m_name, *this);
		m_scene = std::make_unique<Scene>();
		m_renderer = std::make_unique<Renderer>(*this, *m_window, *m_scene);
	}

	void Application::Run() 
	{
		Init();
		MainLoop();
		CleanUp();
	}

	void Application::Init() 
	{
		glfwInit();
	}

	void Application::MainLoop() 
	{
		while (!m_window->ShouldClose()) {
			glfwPollEvents();
			m_renderer->DrawFrame();
		}
		m_renderer->WaitIdle(); // Wait for pending GPU operations to finish
	}

	void Application::CleanUp() 
	{
		glfwTerminate();
	}
}