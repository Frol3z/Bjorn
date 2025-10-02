#pragma once

#include "Window.hpp"

namespace Bjorn {
	class Application {
	public:	
		bool isFramebufferResized = false;
		
		Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight);

		void Run();
	private:
		Window m_window;

		void Init();
		void MainLoop();
		void CleanUp();
	};
}