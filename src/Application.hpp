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

			void Init();
			void Run();
			void CleanUp();
			const std::string& GetName() const { return m_name; }

		private:
			void InitGlfw();
			void InitImGui();
			void InitScene();

			void ProcessInput();

			std::string m_name;
			uint32_t m_startupWindowWidth; // Won't be updated when window gets resized
			uint32_t m_startupWindowHeight;
			
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