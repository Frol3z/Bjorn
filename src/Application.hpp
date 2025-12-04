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

			void ProcessInput();

			std::string m_name;
			std::unique_ptr<Window> m_window = nullptr;
			std::unique_ptr<UI> m_UI = nullptr;
			std::unique_ptr<Scene> m_scene = nullptr;
			std::unique_ptr<Renderer> m_renderer = nullptr;

			// Normalized [0,1] mouse input
			double m_mouseX{ 0.0 };
			double m_mouseY{ 0.0 };
			float m_sensitivity{ 100.0f };
			double m_mouseDeltaX{ 0.0 };
			double m_mouseDeltaY{ 0.0 };
	};
}