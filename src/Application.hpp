#pragma once

#include "Window.hpp"
#include "Renderer.hpp"

namespace Bjorn {
	class Application {
	public:	
		bool isFramebufferResized = false;
		
		Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight);

		void Run();
	private:
		Window m_window;
		Renderer m_renderer;

		void Init();
		void MainLoop();
		void CleanUp();
	};
}