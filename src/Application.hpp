#pragma once

#include "Window.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"

#include <atomic>

namespace Bjorn 
{
	class Application 
	{
	public:	
		std::atomic<bool> isFramebufferResized = false;
		
		Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight);

		void Run();
		const std::string& GetName() const { return m_name; };

	private:
		std::string m_name;
		std::unique_ptr<Window> m_window;
		std::unique_ptr<Scene> m_scene;
		std::unique_ptr<Renderer> m_renderer;

		void Init();
		void BuildScene();
		void MainLoop();
		void CleanUp();
	};
}