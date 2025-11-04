#include "Application.hpp"

#include "GLFW/glfw3.h"

namespace Bjorn 
{
	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight)
		: m_name(name)
	{
		Init();

		m_window = std::make_unique<Window>(windowWidth, windowHeight, m_name, *this);
		m_scene = std::make_unique<Scene>(windowWidth, windowHeight);
		m_renderer = std::make_unique<Renderer>(*this, *m_window, *m_scene);
		m_mesh = std::make_unique<Mesh>();

		BuildScene();
	}

	void Application::Run() 
	{
		MainLoop();
		CleanUp();
	}

	void Application::Init() 
	{
		glfwInit();
	}

	void Application::BuildScene()
	{
		// Set camera position
		m_scene->GetCamera().SetPosition(glm::vec3(2.0f, 2.0f, 2.0f));
		
		// Load mesh
		m_renderer->Load(*m_mesh);
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