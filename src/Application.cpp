#include "Application.hpp"

namespace Bjorn {
	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight) 
		: m_window(windowWidth, windowHeight, name, this), m_renderer(name)
	{}

	void Application::Run() {
		Init();
		MainLoop();
		CleanUp();
	}

	void Application::Init() {

	}

	void Application::MainLoop() {

	}

	void Application::CleanUp() {

	}
}