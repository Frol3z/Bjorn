#pragma once

#include <memory>
#include <string>
#include <atomic>

namespace Felina 
{
	class Window;
	class Scene;
	class Renderer;
	class Object;
	class UI;

	class Application 
	{
		public:	
			std::atomic<bool> isFramebufferResized = false;
		
			Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight);
			~Application();

			void Run();
			const std::string& GetName() const { return m_name; }

		private:
			void InitGlfw();
			void InitImGui();
			void InitScene();
			void MainLoop();
			void CleanUp();

			std::string m_name;
			std::unique_ptr<Window> m_window = nullptr;
			std::unique_ptr<UI> m_UI = nullptr;
			std::unique_ptr<Scene> m_scene = nullptr;
			std::unique_ptr<Renderer> m_renderer = nullptr;
	};
}