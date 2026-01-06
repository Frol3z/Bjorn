#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <utility>

struct GLFWwindow;

namespace Felina 
{
	class Window;
	class Scene;
	class Renderer;
	class Object;
	class UI;
	class Input;

	class Application
	{
		public:			
			Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight);
			~Application();

			void Init();
			void Run();
			void CleanUp();

			const std::string& GetName() const { return m_name; }
			inline Window& GetWindow() { return *m_window; }
			inline Input& GetInput() { return *m_input; }
			inline Scene& GetScene() { return *m_scene; }
			
			// NOTES: 
			// It "consumes" the value when called (see Renderer.cpp).
			inline bool IsFramebufferResized() { return std::exchange(m_isFramebufferResized, false); }
			inline void SignalFramebufferResized() { m_isFramebufferResized = true; }

		private:
			static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
			static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

			void InitGlfw();
			void InitImGui();
			void InitScene();

			void UpdateCamera();

			std::string m_name;
			bool m_isFramebufferResized;
			uint32_t m_startupWindowWidth; // Won't be updated when window gets resized
			uint32_t m_startupWindowHeight;
			
			std::unique_ptr<Window> m_window = nullptr;
			std::unique_ptr<Input> m_input = nullptr;
			std::unique_ptr<UI> m_UI = nullptr;
			std::unique_ptr<Scene> m_scene = nullptr;
			std::unique_ptr<Renderer> m_renderer = nullptr;
	};
}